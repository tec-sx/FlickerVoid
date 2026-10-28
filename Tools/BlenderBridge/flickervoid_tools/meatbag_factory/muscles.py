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
    vertices, faces = spindle.spindle(settings.length, settings.width, settings.thickness, settings.belly, settings.tendon)
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


def create(context, armature, skin, name, origin_bone, insertion_bone, origin, insertion, width=None, thickness=None):
    """A muscle from armature-space rest points on the two bones, pushing the skin (the sculpt target)."""
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
    muscle.display_type = "WIRE"
    muscle.hide_render = True
    settings = muscle.meatbag_muscle
    settings["skin"] = skin
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
    _add_skin(skin, muscle, skin_mask(armature, skin, (origin_bone, insertion_bone), origin, insertion))
    return muscle


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
    for obj in [muscle, *anchors(muscle)]:
        if obj is not None:
            bpy.data.objects.remove(obj)
    if data.users == 0:
        bpy.data.meshes.remove(data)


def mirror(context, muscle, sides):
    """The muscle's counterpart on the other side, from the side names of its bones and name."""
    settings = muscle.meatbag_muscle
    origin_anchor, insertion_anchor = anchors(muscle)
    armature = origin_anchor.parent
    origin_bone = sides.mirror_bone(settings.origin_bone)
    insertion_bone = sides.mirror_bone(settings.insertion_bone)
    name = sides.mirror_name(muscle.name)
    if not origin_bone or not insertion_bone or not name:
        raise MuscleError(f"{muscle.name}: its name and both bones need a side token to mirror")

    def flipped(anchor):
        point = anchor_point(anchor)
        return Vector((-point.x, point.y, point.z))

    existing = bpy.data.objects.get(name)
    if existing is not None and existing.meatbag_muscle.skin is not None:
        remove(existing)
    created = create(context, armature, settings.skin, name, origin_bone, insertion_bone,
                     flipped(origin_anchor), flipped(insertion_anchor), settings.width, settings.thickness)
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
