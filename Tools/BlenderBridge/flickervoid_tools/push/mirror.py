"""Mirror on push: shape keys with a side token reach Unreal on their own side, with their counterpart made by reflection.

A mesh with a Mirror modifier is exported as a temporary full copy: Blender's FBX export drops every shape key once a
modifier changes the vertex count, and a shape key on the modelled half would show on both sides anyway. The copy has
the Mirror applied, and each shape key whose name has a side token (CR_upperarm_l_up) moves only that side; its
counterpart (CR_upperarm_r_up) is made by reflecting it, unless the mesh has one already. Keys without a side token
stay symmetric. A body processed from a template is already full and stores each vertex's mirror partner, so its copy
only adds the missing counterparts. Left is +X, as on the UE Mannequin.
"""

from contextlib import contextmanager

import numpy as np

import bpy
from mathutils import kdtree

from ..core.sides import CENTRE, LEFT, RIGHT

CENTRE_TOLERANCE = 1e-4
FLIP = np.array([-1.0, 1.0, 1.0])
PARTNER_ATTRIBUTE = "mf_mirror"


class MirrorError(RuntimeError):
    pass


def mirror_modifier(mesh):
    """The mesh's enabled Mirror modifier, or None."""
    return next((m for m in mesh.modifiers if m.type == "MIRROR" and m.show_viewport), None)


def partners(mesh):
    """Each vertex's mirror partner on a processed body (itself on the centre line), or None."""
    attribute = mesh.data.attributes.get(PARTNER_ATTRIBUTE)
    if attribute is None or attribute.domain != "POINT" or attribute.data_type != "INT":
        return None
    values = np.empty(len(attribute.data), dtype=np.int32)
    attribute.data.foreach_get("value", values)
    return values


def _missing_counterparts(mesh, sides):
    keys = mesh.data.shape_keys.key_blocks
    names = [sides.mirror_name(key.name) for key in keys if sides.name_side(key.name) != CENTRE]
    return [name for name in names if name and name not in keys]


def needs_copy(mesh, sides):
    if mesh.data.shape_keys is None:
        return False
    if mirror_modifier(mesh) is not None:
        return True
    return partners(mesh) is not None and bool(_missing_counterparts(mesh, sides))


@contextmanager
def full_meshes(context, meshes, sides):
    """The meshes to export: each mirrored mesh with shape keys is swapped for a full copy, removed afterwards."""
    swaps = []
    try:
        result = []
        for mesh in meshes:
            if needs_copy(mesh, sides):
                copy = _full_copy(context, mesh, sides) if mirror_modifier(mesh) else _counterpart_copy(context, mesh, sides)
                swaps.append((mesh, copy, mesh.name))
                mesh.name = mesh.name + "_mf_half"
                copy.name = swaps[-1][2]
                result.append(copy)
            else:
                result.append(mesh)
        yield result
    finally:
        for mesh, copy, name in swaps:
            data = copy.data
            bpy.data.objects.remove(copy)
            bpy.data.meshes.remove(data)
            mesh.name = name


def _evaluated_points(context, mesh):
    evaluated = mesh.evaluated_get(context.evaluated_depsgraph_get())
    data = evaluated.to_mesh()
    try:
        co = np.empty(len(data.vertices) * 3)
        data.vertices.foreach_get("co", co)
    finally:
        evaluated.to_mesh_clear()
    return co.reshape(-1, 3)


@contextmanager
def _key_view(mesh):
    """Shows one shape key at a time without armature deformation; restores the mesh afterwards."""
    armatures = [(m, m.show_viewport) for m in mesh.modifiers if m.type == "ARMATURE"]
    show_only, active = mesh.show_only_shape_key, mesh.active_shape_key_index
    try:
        for modifier, _ in armatures:
            modifier.show_viewport = False
        mesh.show_only_shape_key = True
        yield
    finally:
        for modifier, shown in armatures:
            modifier.show_viewport = shown
        mesh.show_only_shape_key, mesh.active_shape_key_index = show_only, active


def _check_modifier(mesh, modifier):
    if modifier.mirror_object is not None or list(modifier.use_axis) != [True, False, False]:
        raise MirrorError(f"{mesh.name}: the Mirror modifier must mirror X only, without a mirror object, for its shape keys to reach Unreal")


def _full_copy(context, mesh, sides):
    modifier = mirror_modifier(mesh)
    _check_modifier(mesh, modifier)
    keys = mesh.data.shape_keys.key_blocks
    half = len(mesh.data.vertices)

    shapes = []
    with _key_view(mesh):
        for index in range(len(keys)):
            mesh.active_shape_key_index = index
            context.view_layer.update()
            shapes.append(_evaluated_points(context, mesh))
        mesh.active_shape_key_index = 0
        context.view_layer.update()
        evaluated = mesh.evaluated_get(context.evaluated_depsgraph_get())
        data = bpy.data.meshes.new_from_object(evaluated, preserve_all_data_layers=True, depsgraph=context.evaluated_depsgraph_get())

    basis = shapes[0]
    if any(len(shape) != len(basis) for shape in shapes) or not np.allclose(basis[:half], _rest(mesh)):
        bpy.data.meshes.remove(data)
        raise MirrorError(f"{mesh.name}: modifiers before or after the Mirror change the vertices; only the Mirror may")

    source = mirror_sources(basis, half)
    original_side = LEFT if basis[:half, 0].mean() >= 0.0 else RIGHT
    centre = np.abs(basis[:, 0]) < CENTRE_TOLERANCE

    deltas = {}
    for key, shape in zip(keys[1:], shapes[1:]):
        side = sides.name_side(key.name)
        delta = shape - basis
        if side == CENTRE:
            deltas[key.name] = delta
            continue
        deltas[key.name] = _place(delta, source, half, centre, side == original_side)
        counterpart = sides.mirror_name(key.name)
        if counterpart and counterpart not in keys:
            deltas[counterpart] = _place(delta, source, half, centre, side != original_side)

    copy = mesh.copy()
    copy.data = data
    for other in list(copy.modifiers):
        if other.type != "ARMATURE":
            copy.modifiers.remove(other)
    context.scene.collection.objects.link(copy)
    copy.shape_key_clear()
    copy.shape_key_add(name=keys[0].name, from_mix=False)
    for name, delta in deltas.items():
        key = copy.shape_key_add(name=name, from_mix=False)
        key.data.foreach_set("co", (basis + delta).ravel())
        key.value = 0.0
    return copy


def _counterpart_copy(context, mesh, sides):
    """A copy of a processed body with each side key's missing counterpart, reflected through the stored partners."""
    partner = partners(mesh)
    if len(partner) != len(mesh.data.vertices):
        raise MirrorError(f"{mesh.name}: its mirror partners are out of date; press Process in the Meatbag Factory")
    keys = mesh.data.shape_keys
    basis = key_points(keys.reference_key)
    copy = mesh.copy()
    copy.data = mesh.data.copy()
    context.scene.collection.objects.link(copy)
    for key in list(keys.key_blocks):
        counterpart = sides.mirror_name(key.name)
        if key == keys.reference_key or sides.name_side(key.name) == CENTRE or not counterpart or counterpart in copy.data.shape_keys.key_blocks:
            continue
        delta = key_points(key) - basis
        added = copy.shape_key_add(name=counterpart, from_mix=False)
        added.data.foreach_set("co", (basis + counterpart_delta(delta, partner)).ravel())
        added.value = 0.0
    return copy


def key_points(key):
    co = np.empty(len(key.data) * 3)
    key.data.foreach_get("co", co)
    return co.reshape(-1, 3)


def counterpart_delta(delta, partner):
    """The other side's version of a full mesh's shape key delta."""
    return delta[partner] * FLIP


def place_half(delta, side, sources, original_side, centre):
    """A half mesh's shape key delta on its full mesh: centre keys on both sides, side keys on their own side only.

    sources: for each vertex the Mirror added, the half vertex it reflects. centre: the half vertices the Mirror merged."""
    half = len(delta)
    full = np.zeros((half + len(sources), 3))
    reflected = delta[sources] * FLIP
    if side == CENTRE or side == original_side:
        full[:half] = delta
    if side == CENTRE or side != original_side:
        full[half:] = reflected
    if side != CENTRE and side != original_side:
        full[:half][centre] = delta[centre] * FLIP
    return full


def _rest(mesh):
    co = np.empty(len(mesh.data.vertices) * 3)
    mesh.data.vertices.foreach_get("co", co)
    return co.reshape(-1, 3)


def mirror_sources(basis, half):
    """For each vertex the Mirror added (index half and up), the modelled vertex it reflects."""
    tree = kdtree.KDTree(half)
    for index, co in enumerate(basis[:half]):
        tree.insert(co, index)
    tree.balance()
    sources = np.empty(len(basis) - half, dtype=int)
    for offset, co in enumerate(basis[half:]):
        _, index, _ = tree.find((-co[0], co[1], co[2]))
        sources[offset] = index
    return sources


def _place(delta, sources, half, centre, on_modelled_side):
    """The modelled half's change put on one side: as modelled, or reflected onto the added half. Centre vertices keep it."""
    placed = np.zeros_like(delta)
    modelled = delta[:half]
    if on_modelled_side:
        placed[:half] = modelled
    else:
        reflected = modelled[sources] * FLIP
        placed[half:] = reflected
    placed[centre] = delta[centre]
    return placed
