"""Process: builds the full body from its half template.

The template has a Mirror modifier on X and half UVs, one half per UDIM tile. Process applies the Mirror with Mirror U
flipping each tile around its own centre, so both sides get their own UVs, and writes the result into the body. The
body keeps what needs both sides: the pose blend shapes and the sculpt target. When the topology is unchanged, every
shape key keeps its delta vertex by vertex; when it changed, the deltas are projected from the old body, which is kept
as <body>_MF_Previous, and the moved vertices are marked in MF_Changed. Each body vertex stores the template vertex it
came from and its mirror partner.
"""

import hashlib
from contextlib import contextmanager
from dataclasses import dataclass

import numpy as np

import bpy

from . import projection
from .. import sculpt
from ... import scene_data
from ...core.sides import LEFT, RIGHT
from ...push import mirror

TEMPLATE_TAG = "mf_template"
PREVIOUS_TAG = "mf_previous"
FINGERPRINT = "mf_template_fingerprint"
TEMPLATE_INDEX = "mf_template_index"
MIRROR_INDEX = mirror.PARTNER_ATTRIBUTE
CHANGED_GROUP = "MF_Changed"
TEMPLATE_SUFFIX = "_Template"
PREVIOUS_SUFFIX = "_MF_Previous"
REFLECT_TOLERANCE = 1e-4

UP_TO_DATE = "UP_TO_DATE"
CHANGED = "CHANGED"
TOPOLOGY = "TOPOLOGY"
EDITING = "EDITING"

# Mirror U flips each UDIM tile around its own centre, so a half in U 0 to 0.5 of any tile fills the other half.
_MIRROR_OPTIONS = {
    "use_mirror_u": True,
    "use_mirror_v": False,
    "use_mirror_udim": True,
    "use_mirror_vertex_groups": True,
    "mirror_offset_u": 0.0,
    "mirror_offset_v": 0.0,
    "offset_u": 0.0,
    "offset_v": 0.0,
}


class TemplateError(RuntimeError):
    pass


@dataclass
class Report:
    keys: int = 0
    projected: int = 0
    groups_added: tuple = ()
    warnings: tuple = ()
    target: str = ""

    def message(self, body):
        if self.projected:
            text = (f"{body.name}: topology changed, {self.keys} shape keys projected; {self.projected} vertices "
                    f"interpolated, marked in {CHANGED_GROUP}. The old body is kept as {body.name}{PREVIOUS_SUFFIX}")
        else:
            text = f"{body.name} is up to date with its template, {self.keys} shape keys kept"
        if self.groups_added:
            text += f". Added to the template so the Mirror can flip them: {', '.join(self.groups_added)}"
        if self.target:
            text += f". {self.target}"
        if self.warnings:
            text += ". " + "; ".join(self.warnings)
        return text


def template_of(body):
    return body.meatbag_template if body is not None and body.type == "MESH" else None


def previous_of(body):
    previous = bpy.data.objects.get(body.name + PREVIOUS_SUFFIX)
    return previous if previous is not None and previous.get(PREVIOUS_TAG) else None


def editing(template):
    return template.visible_get()


def mirror_modifier(obj):
    return mirror.mirror_modifier(obj)


def problems(obj):
    """(errors, warnings) for a template, or a half mesh about to become one."""
    modifier = mirror_modifier(obj)
    if modifier is None:
        return [f"{obj.name} has no enabled Mirror modifier"], []
    errors, warnings = [], []
    if modifier.mirror_object is not None or list(modifier.use_axis) != [True, False, False]:
        errors.append(f"{obj.name}: the Mirror must mirror X only, without a mirror object")
    if not modifier.use_mirror_merge:
        errors.append(f"{obj.name}: turn on Merge in the Mirror, or the centre line stays open")
    for layer in obj.data.uv_layers:
        uv = np.empty(len(layer.data) * 2)
        layer.data.foreach_get("uv", uv)
        tile = uv[0::2] - np.floor(uv[0::2])
        past = int((tile > 0.5 + 1e-5).sum())
        on_edge = int((tile < 1e-7).sum())
        if past:
            warnings.append(f"{layer.name}: {past} UV corners lie past the middle of their tile and overlap the mirrored side")
        if on_edge:
            warnings.append(f"{layer.name}: {on_edge} UV corners sit on a tile's left edge, where the Mirror can't flip them")
    return errors, warnings


def add_counterpart_groups(template, sides):
    """Adds an empty counterpart for every side group: the Mirror only flips weights into groups that exist."""
    names = {group.name for group in template.vertex_groups}
    added = []
    for name in sorted(names):
        other = sides.mirror_bone(name)
        if other and other not in names:
            template.vertex_groups.new(name=other)
            names.add(other)
            added.append(other)
    return tuple(added)


# Fingerprints

def _digest(*chunks):
    sha = hashlib.sha1()
    for chunk in chunks:
        sha.update(chunk if isinstance(chunk, bytes) else repr(chunk).encode())
    return sha.hexdigest()[:16]


def _array(collection, key, width, dtype=np.float64):
    values = np.empty(len(collection) * width, dtype=dtype)
    collection.foreach_get(key, values)
    return values


def _attribute_values(attribute):
    data = attribute.data
    if len(data) == 0:
        return np.empty(0)
    for key in ("value", "vector", "color"):
        sample = getattr(data[0], key, None)
        if sample is None or isinstance(sample, str):
            continue
        width = len(sample) if hasattr(sample, "__len__") else 1
        values = [0.0] * (len(data) * width)
        try:
            data.foreach_get(key, values)
        except (TypeError, RuntimeError):
            return None
        return np.asarray(values, dtype=np.float64)
    return None


def _content_chunks(obj):
    mesh = obj.data
    chunks = [np.round(_array(mesh.vertices, "co", 3), 6).tobytes()]
    for attribute in sorted(mesh.attributes, key=lambda item: item.name):
        if attribute.name.startswith(".") or attribute.name == "position":
            continue
        chunks.append((attribute.name, attribute.domain, attribute.data_type))
        values = _attribute_values(attribute)
        if values is not None:
            chunks.append(np.round(values, 6).tobytes())
    weights = [(vertex.index, element.group, round(element.weight, 6)) for vertex in mesh.vertices for element in vertex.groups]
    chunks.append(([group.name for group in obj.vertex_groups], weights))
    chunks.append([material.name if material else "" for material in mesh.materials])
    return chunks


def fingerprint(template):
    """(topology, content) digests of the template and its Mirror settings."""
    mesh = template.data
    modifier = mirror_modifier(template)
    settings = None
    if modifier:
        settings = (round(modifier.merge_threshold, 7), modifier.use_mirror_merge, tuple(modifier.use_bisect_axis),
                    tuple(modifier.use_bisect_flip_axis))
    topology = _digest(len(mesh.vertices), _array(mesh.edges, "vertices", 2, np.int32).tobytes(),
                       _array(mesh.polygons, "loop_total", 1, np.int32).tobytes(),
                       _array(mesh.loops, "vertex_index", 1, np.int32).tobytes(), settings)
    return topology, _digest(*_content_chunks(template))


def _stored(body):
    topology, _, content = body.get(FINGERPRINT, "").partition(":")
    return topology, content


def state(body):
    template = template_of(body)
    if template is None:
        return None
    if editing(template):
        return EDITING
    topology, content = fingerprint(template)
    stored_topology, stored_content = _stored(body)
    if (topology, content) == (stored_topology, stored_content):
        return UP_TO_DATE
    return CHANGED if topology == stored_topology else TOPOLOGY


def ensure_current(context, body, sides):
    """Processes a template change that keeps the topology; returns a Report, or None when nothing changed.

    Raises TemplateError while the template is being edited or after a topology change, which waits for Process."""
    current = state(body)
    template = template_of(body)
    if current == EDITING:
        raise TemplateError(f"{template.name} is being edited: press Done first")
    if current == TOPOLOGY:
        raise TemplateError(f"{template.name}'s topology changed: press Process to carry the shape keys over, then check {CHANGED_GROUP}")
    if current == CHANGED:
        return process(context, body, sides)
    return None


# Mesh helpers

def points(mesh):
    return _array(mesh.vertices, "co", 3).reshape(-1, 3)


def read_int(mesh, name):
    attribute = mesh.attributes.get(name)
    if attribute is None or attribute.domain != "POINT" or len(attribute.data) != len(mesh.vertices):
        return None
    values = np.empty(len(attribute.data), dtype=np.int32)
    attribute.data.foreach_get("value", values)
    return values


def _write_int(mesh, name, values):
    attribute = mesh.attributes.get(name)
    if attribute is not None:
        mesh.attributes.remove(attribute)
    attribute = mesh.attributes.new(name, "INT", "POINT")
    attribute.data.foreach_set("value", np.asarray(values, dtype=np.int32))


@contextmanager
def _mirror_only(template):
    """Shows only the Mirror, set to flip UVs per tile and side groups, on the template's basis shape."""
    modifier = mirror_modifier(template)
    shown = [(item, item.show_viewport) for item in template.modifiers]
    options = {name: getattr(modifier, name) for name in _MIRROR_OPTIONS}
    view = template.show_only_shape_key, template.active_shape_key_index
    try:
        for item, _ in shown:
            item.show_viewport = item == modifier
        for name, value in _MIRROR_OPTIONS.items():
            setattr(modifier, name, value)
        template.show_only_shape_key = True
        template.active_shape_key_index = 0
        yield
    finally:
        for item, visible in shown:
            item.show_viewport = visible
        for name, value in options.items():
            setattr(modifier, name, value)
        template.show_only_shape_key, template.active_shape_key_index = view


def evaluate(context, template):
    """The template with only its Mirror applied, as a new mesh, and its vertices' template index and mirror partner."""
    with _mirror_only(template):
        context.view_layer.update()
        depsgraph = context.evaluated_depsgraph_get()
        data = bpy.data.meshes.new_from_object(template.evaluated_get(depsgraph), preserve_all_data_layers=True, depsgraph=depsgraph)
    context.view_layer.update()
    try:
        half = len(template.data.vertices)
        basis = points(data)
        if len(basis) < half or not np.allclose(basis[:half], points(template.data), atol=1e-6):
            raise TemplateError(f"{template.name}: the Mirror's result doesn't start with the template's own vertices")
        sources = mirror.mirror_sources(basis, half)
        if len(sources) and np.abs(basis[half:] * mirror.FLIP - basis[sources]).max() > REFLECT_TOLERANCE:
            raise TemplateError(f"{template.name}: the mirrored side doesn't reflect the template on X")
        if len(np.unique(sources)) != len(sources):
            raise TemplateError(f"{template.name}: several mirrored vertices land on one template vertex; check Merge and the centre line")
    except TemplateError:
        bpy.data.meshes.remove(data)
        raise
    template_index = np.concatenate([np.arange(half), sources])
    partner = np.arange(len(basis))
    partner[half:] = sources
    partner[sources] = np.arange(half, len(basis))
    _write_int(data, TEMPLATE_INDEX, template_index)
    _write_int(data, MIRROR_INDEX, partner)
    return data, sources


# Shape keys

@dataclass
class _Key:
    name: str
    delta: np.ndarray
    value: float
    mute: bool
    slider_min: float
    slider_max: float
    vertex_group: str
    relative_key: str
    interpolation: str


def _snapshot(obj):
    """(reference key name, basis points, [_Key]) of an object's shape keys."""
    keys = obj.data.shape_keys
    if keys is None:
        return None, points(obj.data), []
    reference = keys.reference_key
    basis = mirror.key_points(reference)
    return reference.name, basis, [
        _Key(key.name, mirror.key_points(key) - basis, key.value, key.mute, key.slider_min, key.slider_max,
             key.vertex_group, key.relative_key.name, key.interpolation)
        for key in keys.key_blocks if key != reference]


def _restore(obj, basis, reference, keys, deltas, drivers=None):
    if reference is None:
        return
    obj.shape_key_add(name=reference, from_mix=False)
    for item, delta in zip(keys, deltas):
        key = obj.shape_key_add(name=item.name, from_mix=False)
        key.data.foreach_set("co", (basis + delta).ravel())
        key.slider_max = max(item.slider_max, key.slider_min)
        key.slider_min = item.slider_min
        key.slider_max = item.slider_max
        key.value, key.mute = item.value, item.mute
        key.vertex_group, key.interpolation = item.vertex_group, item.interpolation
    blocks = obj.data.shape_keys.key_blocks
    for item in keys:
        relative = blocks.get(item.relative_key)
        if relative is not None:
            blocks[item.name].relative_key = relative
    if drivers is not None and drivers.drivers:
        target = obj.data.shape_keys
        target.animation_data_create()
        for driver in drivers.drivers:
            target.animation_data.drivers.from_existing(src_driver=driver)


def _group_weights(obj, name):
    group = obj.vertex_groups.get(name)
    if group is None:
        return {}
    return {vertex.index: element.weight for vertex in obj.data.vertices for element in vertex.groups if element.group == group.index}


def _set_group(obj, name, weights):
    group = obj.vertex_groups.get(name) or obj.vertex_groups.new(name=name)
    for index, weight in weights.items():
        group.add([index], weight, "REPLACE")


def _carried_attributes(old, new):
    """The body's own point attributes (such as the solve's residual), to copy when the topology is unchanged."""
    carried = []
    for attribute in old.attributes:
        name = attribute.name
        if name.startswith(".") or attribute.domain != "POINT" or name in new.attributes:
            continue
        values = _attribute_values(attribute)
        if values is not None:
            carried.append((name, attribute.data_type, values))
    return carried


def _write_attribute(mesh, name, data_type, values):
    attribute = mesh.attributes.new(name, data_type, "POINT")
    key = next(key for key in ("value", "vector", "color") if hasattr(attribute.data[0], key)) if len(attribute.data) else None
    if key:
        attribute.data.foreach_set(key, values.tolist())


# Process

def _keep_previous(body, old):
    remove_previous(body)
    previous = body.copy()
    previous.data = old
    scene_data.clear_properties(previous)
    previous.name = old.name = body.name + PREVIOUS_SUFFIX
    for collection in body.users_collection:
        collection.objects.link(previous)
    previous[PREVIOUS_TAG] = True
    previous.hide_render = True
    previous.hide_set(True)


def remove_previous(body):
    previous = previous_of(body)
    if previous is None:
        return False
    data = previous.data
    bpy.data.objects.remove(previous)
    if data.users == 0:
        bpy.data.meshes.remove(data)
    return True


def _rebuild_target(body, data, new_basis, old_body, body_mapping):
    """Gives the sculpt target the new body mesh, its pose shapes carried like the body's shape keys."""
    target = body.meatbag_target
    if target is None:
        bpy.data.meshes.remove(data)
        return
    old = target.data
    reference, old_basis, keys = _snapshot(target)
    if len(old.vertices) == len(old_body.vertices) and projection.same_topology(old, old_body):
        mapping = body_mapping
    else:
        mapping = projection.mapping(old, old_basis, new_basis)
    deltas = [item.delta if mapping is None else mapping.apply(item.delta) for item in keys]
    target.data = data
    target.shape_key_clear()
    _restore(target, new_basis, reference or "Basis", keys, deltas)
    name = old.name
    if old.users == 0:
        bpy.data.meshes.remove(old)
    data.name = name


def process(context, body, sides):
    """Rebuilds the body from its template; returns a Report. Raises TemplateError when the template can't be processed."""
    template = template_of(body)
    if template is None:
        raise TemplateError(f"{body.name} has no template")
    errors, warnings = problems(template)
    if errors:
        raise TemplateError("; ".join(errors))
    added = add_counterpart_groups(template, sides)
    new, _ = evaluate(context, template)
    new_basis = points(new)

    old = body.data
    reference, old_basis, keys = _snapshot(body)
    same = projection.same_topology(old, new)
    body_mapping = None if same else projection.mapping(old, old_basis, new_basis)
    deltas = [item.delta if same else body_mapping.apply(item.delta) for item in keys]
    carried = _carried_attributes(old, new) if same else []
    changed = _group_weights(body, CHANGED_GROUP) if same else {}
    target_data = new.copy()
    drivers = old.shape_keys.animation_data if old.shape_keys else None

    name = old.name
    body.data = new
    _restore(body, new_basis, reference, keys, deltas, drivers)
    for attribute, data_type, values in carried:
        _write_attribute(new, attribute, data_type, values)
    if not same:
        changed = {int(index): 1.0 for index in np.flatnonzero(~body_mapping.exact)}
    if changed:
        _set_group(body, CHANGED_GROUP, changed)
    _rebuild_target(body, target_data, new_basis, old, body_mapping)
    if not same:
        _keep_previous(body, old)
    elif old.users == 0:
        bpy.data.meshes.remove(old)
    new.name = name

    topology, content = fingerprint(template)
    body[FINGERPRINT] = f"{topology}:{content}"
    return Report(keys=len(keys), projected=0 if same else int((~body_mapping.exact).sum()), groups_added=added,
                  warnings=tuple(warnings))


def make_template(context, body, sides):
    """Turns a half mesh with a Mirror into a template, a hidden copy that keeps the Mirror, and makes the body full.

    Shape keys and sculpt target poses with a side token move to their own side; the others stay symmetric."""
    if template_of(body) is not None:
        raise TemplateError(f"{body.name} already has a template")
    errors, warnings = problems(body)
    if errors:
        raise TemplateError("; ".join(errors))

    template = body.copy()
    for collection in body.users_collection:
        collection.objects.link(template)
    scene_data.clear_properties(template)
    template[TEMPLATE_TAG] = True
    template.name = body.name + TEMPLATE_SUFFIX
    half = body.data
    added = add_counterpart_groups(template, sides)

    reference, half_basis, keys = _snapshot(template)
    target = body.meatbag_target
    target_note = ""
    target_keys = None
    if target is not None:
        if len(target.data.vertices) == len(half.vertices):
            target_reference, _, target_keys = _snapshot(target)
        else:
            target_note = f"Removed {target.name}: it no longer matched the mesh"
    try:
        new, sources = evaluate(context, template)
    except TemplateError:
        bpy.data.objects.remove(template)
        raise
    new_basis = points(new)
    centre = np.ones(len(half.vertices), dtype=bool)
    centre[sources] = False
    original_side = LEFT if half_basis[:, 0].mean() >= 0.0 else RIGHT

    def placed(items):
        return [mirror.place_half(item.delta, sides.name_side(item.name), sources, original_side, centre) for item in items]

    target_data = new.copy()
    drivers = half.shape_keys.animation_data if half.shape_keys else None
    body.data = new
    for modifier in [item for item in body.modifiers if item.type == "MIRROR"]:
        body.modifiers.remove(modifier)
    _restore(body, new_basis, reference, keys, placed(keys), drivers)
    template.shape_key_clear()
    name = half.name
    half.name = template.name
    new.name = name

    if target is not None and target_keys is not None:
        old_target = target.data
        target.data = target_data
        target.shape_key_clear()
        _restore(target, new_basis, target_reference or "Basis", target_keys, placed(target_keys))
        target_data.name = old_target.name
        if old_target.users == 0:
            bpy.data.meshes.remove(old_target)
        target_note = f"{target.name} converted, {len(target_keys)} poses kept"
    else:
        bpy.data.meshes.remove(target_data)
        if target is not None:
            sculpt.remove_target(body)

    body.meatbag_template = template
    template.hide_render = True
    template.hide_set(True)
    topology, content = fingerprint(template)
    body[FINGERPRINT] = f"{topology}:{content}"
    return Report(keys=len(keys), groups_added=added, warnings=tuple(warnings), target=target_note)


def write_back_weights(body, bones, weights, before, changed, sides):
    """Writes solved weights into the template's vertices; the mirrored side only where its partner didn't change.

    weights and before are (vertices, bones) arrays over the body; returns how many template vertices changed."""
    template = template_of(body)
    index = read_int(body.data, TEMPLATE_INDEX)
    if template is None or index is None:
        return 0
    half = len(template.data.vertices)
    changed = [int(vertex) for vertex in changed]
    changed_set = set(changed)
    groups = {}
    written = 0
    for vertex in changed:
        source = int(index[vertex])
        mirrored = vertex >= half
        if mirrored and source in changed_set:
            continue
        for bone in np.flatnonzero((weights[vertex] > 0) | (before[vertex] > 0)):
            name = bones[bone]
            if mirrored:
                name = sides.mirror_bone(name) or name
            group = groups.get(name) or template.vertex_groups.get(name) or template.vertex_groups.new(name=name)
            groups[name] = group
            if weights[vertex, bone] > 0:
                group.add([source], float(weights[vertex, bone]), "REPLACE")
            else:
                group.remove([source])
        written += 1
    return written
