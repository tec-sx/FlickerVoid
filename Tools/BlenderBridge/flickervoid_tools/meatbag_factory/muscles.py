"""Muscles on the sculpt target: a spindle mesh between two bone anchors pushes the skin out as it bulges.

Each muscle is three objects: an origin anchor (an empty on the origin bone), an insertion anchor (an empty on the
insertion bone) and the spindle mesh, parented to the origin anchor and stretched to the insertion anchor with
volume kept, so it thickens as it shortens. The sculpt target gets one Shrinkwrap per muscle (skin inside the
muscle moves to its surface), masked by a copy of the origin bone's weights, and one Corrective Smooth after them.
It all follows the pose at once, with no frames, so the pose library and the solve drive it directly.
"""

import bmesh
import bpy
import numpy as np
from mathutils import Matrix, Vector

from . import spindle

COLLECTION = "MF_Muscles"
PREFIX = "MF_"
SMOOTH = "MF_Smooth"
STRETCH = "MF_Stretch"
ANCHOR_SIZE = 2.0
FIT = "mf_fit"
FIT_SIZE = 10 * 12  # inner rings x segments of the spindle


class MuscleError(RuntimeError):
    pass


def muscles_of(scene, target):
    return [obj for obj in scene.objects if obj.type == "MESH" and target is not None and obj.meatbag_muscle.skin == target]


def bone_point(armature, bone_name, along):
    """Armature-space rest point at fraction `along` from the bone's head to its tail."""
    bone = armature.data.bones.get(bone_name)
    if bone is None:
        raise MuscleError(f"{armature.name} has no bone {bone_name}")
    return bone.head_local.lerp(bone.tail_local, along)


def _bone_space(bone):
    # Objects parented to a bone sit in the space of the bone's tail.
    return bone.matrix_local @ Matrix.Translation((0.0, bone.length, 0.0))


def anchor_point(anchor):
    """The anchor's armature-space rest point."""
    bone = anchor.parent.data.bones[anchor.parent_bone]
    return _bone_space(bone) @ anchor.matrix_basis.translation


def fitted_radius(armature, mesh, bone_name, start, end):
    """A radius inside the skin: 80% of the median distance from the muscle line of the bone's main vertices."""
    group = mesh.vertex_groups.get(bone_name)
    if group is None:
        return 3.0
    to_armature = armature.matrix_world.inverted() @ mesh.matrix_world
    axis = end - start
    length = max(axis.length, 1e-6)
    axis = axis / length
    distances = []
    for vertex in mesh.data.vertices:
        if any(element.group == group.index and element.weight > 0.5 for element in vertex.groups):
            point = to_armature @ vertex.co - start
            along = min(max(point.dot(axis), 0.0), length)
            distances.append((point - axis * along).length)
    return 0.8 * float(np.median(distances)) if distances else 3.0


def _collection(skin):
    collection = bpy.data.collections.get(COLLECTION)
    if collection is None:
        collection = bpy.data.collections.new(COLLECTION)
        parent = skin.users_collection[0] if skin.users_collection else bpy.context.scene.collection
        parent.children.link(collection)
    return collection


def _anchor(name, armature, bone_name, point, collection):
    anchor = bpy.data.objects.new(name, None)
    anchor.empty_display_type = "PLAIN_AXES"
    anchor.empty_display_size = ANCHOR_SIZE
    collection.objects.link(anchor)
    anchor.parent = armature
    anchor.parent_type = "BONE"
    anchor.parent_bone = bone_name
    anchor.matrix_parent_inverse = Matrix.Identity(4)
    anchor.location = _bone_space(armature.data.bones[bone_name]).inverted() @ point
    return anchor


def build_mesh(muscle):
    """Regenerates the spindle from the muscle's settings."""
    settings = muscle.meatbag_muscle
    fit = muscle.get(FIT)
    vertices, faces = spindle.spindle(settings.length, settings.width, settings.thickness, settings.belly, settings.tendon,
                                      fit=list(fit) if fit is not None and len(fit) == FIT_SIZE else None)
    data = muscle.data
    data.clear_geometry()
    data.from_pydata(vertices, [], faces)
    work = bmesh.new()
    work.from_mesh(data)
    bmesh.ops.recalc_face_normals(work, faces=work.faces)
    work.to_mesh(data)
    work.free()
    for polygon in data.polygons:
        polygon.use_smooth = True
    data.update()


def create(context, armature, skin, name, origin_bone, insertion_bone, origin, insertion, width=None, thickness=None,
           group="", hooks=(), color=None):
    """A muscle from armature-space rest points on the two bones, pushing the skin (the sculpt target). `hooks` are
    (bone, point) pairs the muscle also bends with; `group` names the muscle the strand belongs to."""
    if origin_bone == insertion_bone:
        raise MuscleError("Pick two different bones")
    if (insertion - origin).length < 1e-4:
        raise MuscleError("The origin and insertion points are at the same place")
    name = PREFIX + name if not name.startswith(PREFIX) else name
    if bpy.data.objects.get(name):
        raise MuscleError(f"{name} already exists")

    collection = _collection(skin)
    origin_anchor = _anchor(name + "_origin", armature, origin_bone, origin, collection)
    insertion_anchor = _anchor(name + "_insertion", armature, insertion_bone, insertion, collection)

    radius = fitted_radius(armature, skin, origin_bone, origin, insertion)
    muscle = bpy.data.objects.new(name, bpy.data.meshes.new(name))
    collection.objects.link(muscle)
    muscle.parent = origin_anchor
    muscle.matrix_parent_inverse = Matrix.Identity(4)
    muscle.hide_render = True
    settings = muscle.meatbag_muscle
    settings["skin"] = skin
    settings["group"] = group or name
    settings["origin_bone"], settings["insertion_bone"] = origin_bone, insertion_bone
    settings["length"] = (insertion - origin).length
    settings["width"] = width or radius
    settings["thickness"] = thickness or radius
    build_mesh(muscle)

    stretch = muscle.constraints.new("STRETCH_TO")
    stretch.name = STRETCH
    stretch.target = insertion_anchor
    stretch.rest_length = settings.length * armature.matrix_world.to_scale().y
    stretch.volume = "VOLUME_XZX"
    stretch.bulge = settings.volume
    muscle.data.materials.append(_material(settings.group, color, context.scene))
    show(muscle, context.scene)
    for index, (bone, point) in enumerate(hooks):
        _add_hook(context, muscle, _anchor(f"{name}_hook{index + 1}", armature, bone, point, collection))
    bones = (origin_bone, insertion_bone, *(bone for bone, _ in hooks))
    _add_skin(skin, muscle, skin_mask(armature, skin, bones, origin, insertion))
    return muscle


def _add_hook(context, muscle, anchor):
    """Bends the muscle near `anchor` with the anchor's bone, on top of the stretch between its ends."""
    context.view_layer.update()
    hook = muscle.modifiers.new(anchor.name, "HOOK")
    hook.object = anchor
    to_local = muscle.matrix_world.inverted()
    hook.center = to_local @ anchor.matrix_world.translation
    hook.matrix_inverse = (to_local @ anchor.matrix_world).inverted()
    hook.vertex_indices_set(range(len(muscle.data.vertices)))  # without indices or a group a hook moves nothing
    hook.falloff_type = "SMOOTH"
    hook.falloff_radius = max(muscle.meatbag_muscle.length * 0.6, 1e-3)
    hook.show_expanded = False


def hooks_of(muscle):
    """(bone, armature-space rest point) of each hook, for mirroring."""
    return [(modifier.object.parent_bone, anchor_point(modifier.object)) for modifier in muscle.modifiers
            if modifier.type == "HOOK" and modifier.object is not None]


def _material(group, color, scene):
    base = group.rsplit("_", 1)[0]
    name = f"{PREFIX}Muscle_{base}"
    material = bpy.data.materials.get(name)
    if material is None:
        material = bpy.data.materials.new(name)
        color = color or (0.75, 0.2, 0.2)
        material.diffuse_color = (*color, 1.0)
        material.surface_render_method = "BLENDED"
    material.diffuse_color[3] = scene.meatbag_muscles.opacity
    return material


def show(muscle, scene):
    """Applies the scene's muscle view settings to one muscle."""
    view = scene.meatbag_muscles
    muscle.display_type = "SOLID" if view.solid else "WIRE"
    muscle.show_in_front = view.in_front
    muscle.hide_viewport = not view.visible
    for material in muscle.data.materials:
        if material is not None:
            material.diffuse_color[3] = view.opacity


def show_all(scene):
    for obj in scene.objects:
        if obj.type == "MESH" and obj.meatbag_muscle.skin is not None:
            show(obj, scene)


def body_frame(armature):
    """(front, left, up) unit vectors of the body in armature space, from the skeleton itself."""
    bones = armature.data.bones
    up = (bones["head"].head_local - bones["pelvis"].head_local).normalized()
    left = bones["clavicle_l"].tail_local - bones["clavicle_r"].tail_local
    left = (left - up * left.dot(up)).normalized()
    return left.cross(up), left, up


class Fitter:
    """Places template attachments under the skin's rest surface and sizes muscles to the room they have."""

    def __init__(self, armature, skin):
        from mathutils.bvhtree import BVHTree
        self.armature = armature
        self.bone_names = set(armature.data.bones.keys())
        to_armature = armature.matrix_world.inverted() @ skin.matrix_world
        vertices = [to_armature @ vertex.co for vertex in skin.data.vertices]
        self.tree = BVHTree.FromPolygons(vertices, [polygon.vertices for polygon in skin.data.polygons])
        self.front, self.left, self.up = body_frame(armature)

    def bone(self, name):
        from . import anatomy
        bone = anatomy.resolve_bone(name, self.bone_names)
        if bone is None:
            raise MuscleError(f"{self.armature.name} has no bone for {name}")
        return bone

    def point(self, attachment):
        """(bone, armature-space point) of a template attachment."""
        bone = self.bone(attachment.bone)
        start = bone_point(self.armature, bone, attachment.along)
        front, left, up = attachment.direction
        direction = self.front * front + self.left * left + self.up * up
        if direction.length < 1e-6:
            return bone, start
        hit, _, _, distance = self.tree.ray_cast(start, direction.normalized())
        if hit is None:
            return bone, start
        return bone, start.lerp(hit, 1.0 - attachment.depth)

    def clearance(self, point):
        """Distance from a point to the skin."""
        nearest = self.tree.find_nearest(point)
        return nearest[3] if nearest[0] is not None else 3.0


def fit_to_skin(context, muscle, fitter, room=0.85, grow=1.6, shrink=0.2):
    """Gives every ring vertex its own radius: `room` of the distance to the skin straight out from the muscle's
    line, so it flattens under flat skin and fills round limbs. Growth is capped, so a muscle doesn't fill the
    body cavity behind it. Kept on the muscle and reapplied when its shape settings change."""
    settings = muscle.meatbag_muscle
    muscle.pop(FIT, None)
    build_mesh(muscle)
    context.view_layer.update()
    to_armature = fitter.armature.matrix_world.inverted() @ muscle.matrix_world
    to_local = to_armature.inverted()
    vertices = muscle.data.vertices
    scale = np.ones(FIT_SIZE)
    for index in range(FIT_SIZE):
        local = vertices[index + 1].co  # vertex 0 is the origin tip
        centre = Vector((0.0, local.y, 0.0))
        start = to_armature @ centre
        direction = (to_armature @ local - start)
        if direction.length < 1e-9:
            continue
        hit, _, _, _ = fitter.tree.ray_cast(start, direction.normalized())
        if hit is None:
            continue
        room_local = ((to_local @ hit) - centre).length * room
        current = (local - centre).length
        taper = spindle.radius_profile(local.y / settings.length, settings.belly, settings.tendon)
        scale[index] = room_local * float(taper) / current  # the belly fills the room, the tendons keep their taper
    # Smoothed around each ring to avoid spikes from single rays, never past the room a ray found.
    raw = scale.reshape(-1, 12)
    rings = np.minimum(raw, (raw + np.roll(raw, 1, axis=1) + np.roll(raw, -1, axis=1)) / 3.0)
    rings = np.clip(rings, shrink, grow)
    muscle[FIT] = rings.ravel().tolist()
    build_mesh(muscle)


def fit_all(context, armature, skin):
    fitter = Fitter(armature, skin)
    fitted = muscles_of(context.scene, skin)
    for muscle in fitted:
        fit_to_skin(context, muscle, fitter)
    return fitted


GENERATED = "mf_generated"


def generate(context, armature, skin, sides, template):
    """Builds every template muscle on the left, mirrors it to the right, and replaces earlier generated muscles."""
    from . import anatomy
    for obj in [obj for obj in context.scene.objects
                if obj.type == "MESH" and obj.meatbag_muscle.skin == skin and obj.get(GENERATED)]:
        remove(obj)
    fitter = Fitter(armature, skin)
    created = []
    for muscle in template:
        for index, (origin, insertion) in enumerate(muscle.strands):
            origin_bone, origin_point = fitter.point(origin)
            insertion_bone, insertion_point = fitter.point(insertion)
            radius = fitter.clearance(origin_point.lerp(insertion_point, muscle.belly))
            hooks = [fitter.point(hook) for hook in muscle.hooks]
            strand = create(context, armature, skin, anatomy.strand_name(muscle, index), origin_bone, insertion_bone,
                            origin_point, insertion_point, radius * muscle.width, radius * muscle.thickness,
                            group=muscle.name, hooks=hooks, color=muscle.color)
            settings = strand.meatbag_muscle
            settings["belly"], settings["tendon"] = muscle.belly, muscle.tendon
            build_mesh(strand)
            strand[GENERATED] = True
            created.append(strand)
            mirrored = mirror(context, strand, sides)
            mirrored[GENERATED] = True
            created.append(mirrored)
    for strand in created:
        fit_to_skin(context, strand, fitter)
    return created


def skin_mask(armature, skin, bones, origin, insertion, fade=0.1):
    """Per skin vertex: its weight on the muscle's bones, faded out beyond the muscle's ends, so only skin along
    the muscle can be pushed (skin past an end would otherwise snap to the tip)."""
    count = len(skin.data.vertices)
    weights = np.zeros(count)
    indices = {skin.vertex_groups[name].index for name in bones if name in skin.vertex_groups}
    for vertex in skin.data.vertices:
        for element in vertex.groups:
            if element.group in indices:
                weights[vertex.index] = max(weights[vertex.index], element.weight)

    co = np.empty(count * 3)
    skin.data.vertices.foreach_get("co", co)
    to_armature = np.array(armature.matrix_world.inverted() @ skin.matrix_world)
    points = co.reshape(-1, 3) @ to_armature[:3, :3].T + to_armature[:3, 3]
    axis = np.array(insertion - origin)
    along = (points - np.array(origin)) @ axis / max(axis @ axis, 1e-12)
    ramp = np.clip(np.minimum(along, 1.0 - along) / fade, 0.0, 1.0)
    return weights * ramp * ramp * (3.0 - 2.0 * ramp)


def _add_skin(skin, muscle, mask):
    group = skin.vertex_groups.get(muscle.name) or skin.vertex_groups.new(name=muscle.name)
    group.remove(list(range(len(mask))))
    for index in np.flatnonzero(mask > 1e-4):
        group.add([int(index)], float(mask[index]), "REPLACE")

    wrap = skin.modifiers.new(muscle.name, "SHRINKWRAP")
    wrap.target = muscle
    wrap.wrap_method = "NEAREST_SURFACEPOINT"
    wrap.wrap_mode = "OUTSIDE"
    wrap.offset = muscle.meatbag_muscle.offset
    wrap.vertex_group = group.name
    wrap.show_expanded = False
    _update_smooth(skin)


def _update_smooth(skin):
    """One Corrective Smooth after every muscle, limited to the skin the muscles touch."""
    muscles = [modifier for modifier in skin.modifiers if modifier.type == "SHRINKWRAP" and modifier.name.startswith(PREFIX)]
    smooth = skin.modifiers.get(SMOOTH)
    if not muscles:
        if smooth:
            skin.modifiers.remove(smooth)
        group = skin.vertex_groups.get(SMOOTH)
        if group:
            skin.vertex_groups.remove(group)
        return

    group = skin.vertex_groups.get(SMOOTH) or skin.vertex_groups.new(name=SMOOTH)
    weights = np.zeros(len(skin.data.vertices))
    indices = {skin.vertex_groups[modifier.vertex_group].index for modifier in muscles if modifier.vertex_group in skin.vertex_groups}
    for vertex in skin.data.vertices:
        for element in vertex.groups:
            if element.group in indices:
                weights[vertex.index] = max(weights[vertex.index], element.weight)
    group.remove(list(range(len(weights))))
    for index in np.flatnonzero(weights > 0.0):
        group.add([int(index)], float(weights[index]), "REPLACE")

    if smooth is None:
        smooth = skin.modifiers.new(SMOOTH, "CORRECTIVE_SMOOTH")
        smooth.rest_source = "ORCO"
        smooth.smooth_type = "LENGTH_WEIGHTED"
        smooth.factor = 0.5
        smooth.iterations = 5
        smooth.show_expanded = False
    smooth.vertex_group = group.name
    _move_last(skin, smooth)


def _move_last(obj, modifier):
    index = list(obj.modifiers).index(modifier)
    last = len(obj.modifiers) - 1
    if index != last:
        obj.modifiers.move(index, last)


def anchors(muscle):
    origin = muscle.parent
    insertion = muscle.constraints[STRETCH].target if STRETCH in muscle.constraints else None
    return origin, insertion


def remove(muscle):
    skin = muscle.meatbag_muscle.skin
    if skin is not None:
        modifier = skin.modifiers.get(muscle.name)
        if modifier:
            skin.modifiers.remove(modifier)
        group = skin.vertex_groups.get(muscle.name)
        if group:
            skin.vertex_groups.remove(group)
        _update_smooth(skin)
    data = muscle.data
    hook_anchors = [modifier.object for modifier in muscle.modifiers if modifier.type == "HOOK"]
    for obj in [muscle, *anchors(muscle), *hook_anchors]:
        if obj is not None:
            bpy.data.objects.remove(obj)
    if data.users == 0:
        bpy.data.meshes.remove(data)


def mirror(context, muscle, sides):
    """The muscle's counterpart on the other side, from the side names of its bones and name."""
    settings = muscle.meatbag_muscle
    origin_anchor, insertion_anchor = anchors(muscle)
    armature = origin_anchor.parent
    origin_bone = sides.mirror_bone(settings.origin_bone) or settings.origin_bone
    insertion_bone = sides.mirror_bone(settings.insertion_bone) or settings.insertion_bone
    name = sides.mirror_name(muscle.name)
    if not name:
        raise MuscleError(f"{muscle.name}: its name needs a side token to mirror")

    def flipped(anchor):
        point = anchor_point(anchor)
        return Vector((-point.x, point.y, point.z))

    existing = bpy.data.objects.get(name)
    if existing is not None and existing.meatbag_muscle.skin is not None:
        remove(existing)
    hooks = []
    for bone, point in hooks_of(muscle):
        hooks.append((sides.mirror_bone(bone) or bone, Vector((-point.x, point.y, point.z))))
    created = create(context, armature, settings.skin, name, origin_bone, insertion_bone,
                     flipped(origin_anchor), flipped(insertion_anchor), settings.width, settings.thickness,
                     group=sides.mirror_name(settings.group) or settings.group, hooks=hooks,
                     color=tuple(muscle.data.materials[0].diffuse_color[:3]) if muscle.data.materials else None)
    other = created.meatbag_muscle
    for prop in ("belly", "tendon", "volume", "offset"):
        other[prop] = getattr(settings, prop)
    build_mesh(created)
    apply_settings(created)
    return created


def apply_settings(muscle):
    """Pushes volume and offset onto the constraint and the skin modifier."""
    settings = muscle.meatbag_muscle
    if STRETCH in muscle.constraints:
        muscle.constraints[STRETCH].bulge = settings.volume
    skin = settings.skin
    modifier = skin.modifiers.get(muscle.name) if skin else None
    if modifier:
        modifier.offset = settings.offset
