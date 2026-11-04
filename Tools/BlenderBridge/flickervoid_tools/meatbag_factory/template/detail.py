"""The Detail object: a hidden multires copy of the full body for sculpted detail, bake meshes and multires bakes.

It shares the body's topology and UVs. While the body keeps its topology, Process moves the Detail's base with it and
the multires detail follows. After a topology change it stays on the old topology, which still bakes, until Transfer
Detail shrinkwraps the sculpt onto a new Detail and reshapes its multires from it.
"""

import os
from contextlib import contextmanager

import numpy as np

import bpy

from . import projection
from ...push import export

DETAIL_TAG = "mf_detail"
DETAIL_SUFFIX = "_MF_Detail"
TOPOLOGY = "mf_detail_topology"
EXPORT_FOLDER = "Export"

BAKE_TYPES = (
    ("NORMALS", "Normals", "Tangent-space normal map of the sculpted detail"),
    ("DISPLACEMENT", "Displacement", "Height of the sculpted detail above the base surface"),
    ("VECTOR_DISPLACEMENT", "Vector Displacement", "Offset of the sculpted detail from the base surface, in tangent space"),
)
_BAKE_FILES = {"NORMALS": ("normal", "PNG", "png"), "DISPLACEMENT": ("displacement", "OPEN_EXR", "exr"),
               "VECTOR_DISPLACEMENT": ("vector_displacement", "OPEN_EXR", "exr")}


class DetailError(RuntimeError):
    pass


def detail_of(body):
    return body.meatbag_detail if body is not None and body.type == "MESH" else None


def multires(obj):
    return next((modifier for modifier in obj.modifiers if modifier.type == "MULTIRES"), None)


def follows(body):
    """True while the Detail shares the body's topology, so Process moves it along."""
    detail = detail_of(body)
    return detail is not None and detail.get(TOPOLOGY) == projection.topology(body.data)


def levels(detail):
    modifier = multires(detail)
    return modifier.total_levels if modifier else 0


def _subdivide(detail, count):
    modifier = multires(detail) or detail.modifiers.new("Multires", "MULTIRES")
    with bpy.context.temp_override(object=detail, active_object=detail):
        for _ in range(count):
            bpy.ops.object.multires_subdivide(modifier=modifier.name, mode="CATMULL_CLARK")
    return modifier


def add(body, count):
    """Makes the body's Detail: a hidden copy of its base mesh, without shape keys, with count multires levels."""
    if detail_of(body) is not None:
        raise DetailError(f"{body.name} already has {body.meatbag_detail.name}")
    if any(modifier.type == "MIRROR" for modifier in body.modifiers):
        raise DetailError(f"{body.name} is a half mesh: press Make Template first, the Detail needs the full body")
    detail = bpy.data.objects.new(body.name + DETAIL_SUFFIX, body.data.copy())
    detail.data.name = detail.name
    for collection in body.users_collection:
        collection.objects.link(detail)
    detail.matrix_world = body.matrix_world.copy()
    detail.shape_key_clear()
    detail[DETAIL_TAG] = True
    detail[TOPOLOGY] = projection.topology(body.data)
    detail.hide_render = True
    _subdivide(detail, count)
    detail.hide_set(True)
    body.meatbag_detail = detail
    return detail


def remove(body):
    detail = detail_of(body)
    if detail is None:
        return
    body.meatbag_detail = None
    data = detail.data
    bpy.data.objects.remove(detail)
    if data.users == 0:
        bpy.data.meshes.remove(data)


def follow(body, mesh):
    """Moves the Detail's base and UVs to the processed body mesh; returns False when it is on another topology."""
    detail = detail_of(body)
    if detail is None or detail.get(TOPOLOGY) != projection.topology(mesh):
        return False
    co = np.empty(len(mesh.vertices) * 3)
    mesh.vertices.foreach_get("co", co)
    detail.data.vertices.foreach_set("co", co)
    for layer in mesh.uv_layers:
        target = detail.data.uv_layers.get(layer.name) or detail.data.uv_layers.new(name=layer.name)
        uv = np.empty(len(layer.data) * 2)
        layer.data.foreach_get("uv", uv)
        target.data.foreach_set("uv", uv)
    detail.data.update()
    return True


@contextmanager
def _shown(context, objects):
    """Shows the objects and selects only them, the first one active; restores visibility and selection after."""
    view_layer = context.view_layer
    hidden = [(obj, obj.hide_get()) for obj in objects]
    selection, active = list(context.selected_objects), view_layer.objects.active
    try:
        for obj, _ in hidden:
            obj.hide_set(False)
        for obj in selection:
            obj.select_set(False)
        for obj in objects:
            obj.select_set(True)
        view_layer.objects.active = objects[0]
        yield
    finally:
        for obj in context.selected_objects:
            obj.select_set(False)
        for obj, was_hidden in hidden:
            obj.hide_set(was_hidden)
        for obj in selection:
            if obj.name in view_layer.objects:
                obj.select_set(True)
        view_layer.objects.active = active


def _evaluated_mesh(context, obj):
    context.view_layer.update()
    depsgraph = context.evaluated_depsgraph_get()
    return bpy.data.meshes.new_from_object(obj.evaluated_get(depsgraph), depsgraph=depsgraph)


@contextmanager
def _top_level(*objects):
    """Shows every object's multires at its highest level while inside."""
    saved = [(modifier, modifier.levels) for modifier in (multires(obj) for obj in objects) if modifier]
    try:
        for modifier, _ in saved:
            modifier.levels = modifier.total_levels
        yield
    finally:
        for modifier, value in saved:
            modifier.levels = value


def transfer(context, body):
    """Replaces a Detail left on an older topology with one on the body's, its sculpt moved over; returns the new Detail."""
    old = detail_of(body)
    if old is None:
        raise DetailError(f"{body.name} has no Detail")
    if follows(body):
        raise DetailError(f"{old.name} already matches {body.name}")
    count = levels(old)
    name = old.name
    old.name = name + "_old"
    body.meatbag_detail = None
    try:
        new = add(body, count)
    except Exception:
        old.name = name
        body.meatbag_detail = old
        raise
    if count:
        _reshape_from(context, new, old)
    data = old.data
    bpy.data.objects.remove(old)
    if data.users == 0:
        bpy.data.meshes.remove(data)
    new.name = new.data.name = name
    return new


def _reshape_from(context, new, old):
    """Projects new's top level onto old's sculpt along its normals and reshapes new's multires to it."""
    with _shown(context, [new, old]), _top_level(new, old):
        high = _evaluated_mesh(context, new)
        wrapped = bpy.data.objects.new("MF_Transfer", high)
        context.scene.collection.objects.link(wrapped)
        wrapped.matrix_world = new.matrix_world.copy()
        try:
            wrap = wrapped.modifiers.new("Shrinkwrap", "SHRINKWRAP")
            wrap.target = old
            wrap.wrap_method = "PROJECT"
            wrap.use_negative_direction = True
            wrap.use_positive_direction = True
            projected = _evaluated_mesh(context, wrapped)
            wrapped.modifiers.remove(wrap)
            wrapped.data = projected
            bpy.data.meshes.remove(high)
            # Reshape copies from the other selected object, so only the projected copy may be selected with new.
            old.select_set(False)
            wrapped.select_set(True)
            context.view_layer.objects.active = new
            bpy.ops.object.multires_reshape(modifier=multires(new).name)
        finally:
            data = wrapped.data
            bpy.data.objects.remove(wrapped)
            bpy.data.meshes.remove(data)


def export_folder():
    if not bpy.data.filepath:
        raise DetailError("Save the file first: bake meshes and maps go into an Export folder next to it")
    folder = os.path.join(os.path.dirname(bpy.data.filepath), EXPORT_FOLDER)
    os.makedirs(folder, exist_ok=True)
    return folder


def export_meshes(context, body):
    """Writes <body>_low.fbx (the body's base) and <body>_high.fbx (the Detail at its top level); returns their paths."""
    detail = detail_of(body)
    if detail is None:
        raise DetailError(f"{body.name} has no Detail")
    folder = export_folder()
    low = bpy.data.objects.new(body.name + "_low", body.data.copy())
    low.shape_key_clear()
    with _top_level(detail):
        high = bpy.data.objects.new(body.name + "_high", _evaluated_mesh(context, detail))
    paths = []
    try:
        for obj in (low, high):
            context.scene.collection.objects.link(obj)
            obj.matrix_world = body.matrix_world.copy()
            path = os.path.join(folder, obj.name + ".fbx")
            export.export_meshes_fbx(context, [obj], path)
            paths.append(path)
    finally:
        for obj in (low, high):
            data = obj.data
            bpy.data.objects.remove(obj)
            bpy.data.meshes.remove(data)
    return paths


def uv_tiles(mesh):
    """The UDIM tiles the mesh's faces use in its active UV map, by face centre."""
    layer = mesh.uv_layers.active
    if layer is None or not len(mesh.polygons):
        return []
    uv = np.empty(len(layer.data) * 2)
    layer.data.foreach_get("uv", uv)
    starts = np.empty(len(mesh.polygons), dtype=np.int64)
    mesh.polygons.foreach_get("loop_start", starts)
    totals = np.empty(len(mesh.polygons), dtype=np.int64)
    mesh.polygons.foreach_get("loop_total", totals)
    centres = np.add.reduceat(uv.reshape(-1, 2), starts) / totals[:, None]
    tiles = 1001 + np.floor(centres[:, 0]).astype(int) + 10 * np.floor(centres[:, 1]).astype(int)
    return sorted({int(tile) for tile in tiles if 1001 <= tile <= 1100})


def _image(name, size, tiles, floating):
    """A tiled image with a filled tile for each UDIM tile."""
    image = bpy.data.images.new(name, size, size, alpha=False, float_buffer=floating, tiled=True)
    image.colorspace_settings.name = "Non-Color"
    first = image.tiles[0]
    if first.number != tiles[0]:
        first.number = tiles[0]
    for number in tiles[1:]:
        image.tiles.active = image.tiles.new(tile_number=number)
        with bpy.context.temp_override(edit_image=image):
            bpy.ops.image.tile_fill(color=(0.0, 0.0, 0.0, 1.0), generated_type="BLANK", width=size, height=size,
                                    float=floating, alpha=False)
    image.tiles.active = image.tiles[0]
    return image


@contextmanager
def _bake_material(obj, image):
    """Gives every material slot a temporary material whose active node is the image, as the multires bake needs."""
    material = bpy.data.materials.new("MF_Bake")
    if material.node_tree is None:
        material.use_nodes = True
    node = material.node_tree.nodes.new("ShaderNodeTexImage")
    node.image = image
    material.node_tree.nodes.active = node
    added = not obj.material_slots
    if added:
        obj.data.materials.append(None)
    slots = [(slot, slot.link, slot.material if slot.link == "OBJECT" else None) for slot in obj.material_slots]
    try:
        for slot, _, _ in slots:
            slot.link = "OBJECT"
            slot.material = material
        yield
    finally:
        for slot, link, kept in slots:
            slot.material = kept
            slot.link = link
        if added:
            obj.data.materials.pop()
        bpy.data.materials.remove(material)


@contextmanager
def _bake_settings(scene, bake_type):
    bake = scene.render.bake
    saved = bake.use_multires, bake.type, bake.margin
    try:
        bake.use_multires = True
        bake.type = bake_type
        bake.margin = 16
        yield
    finally:
        bake.use_multires, bake.type, bake.margin = saved


def bake(context, body, bake_type, size):
    """Bakes the Detail's sculpt from its multires into <body>_<map>.<UDIM> files in the Export folder; returns them."""
    detail = detail_of(body)
    if detail is None:
        raise DetailError(f"{body.name} has no Detail")
    modifier = multires(detail)
    if modifier is None or modifier.total_levels == 0:
        raise DetailError(f"{detail.name} has no multires levels to bake from")
    tiles = uv_tiles(detail.data)
    if not tiles:
        raise DetailError(f"{detail.name} has no UVs in UDIM tiles 1001 to 1100")
    folder = export_folder()
    suffix, file_format, extension = _BAKE_FILES[bake_type]
    image = _image(f"{body.name}_{suffix}", size, tiles, file_format == "OPEN_EXR")
    view = modifier.levels, modifier.render_levels
    try:
        modifier.levels, modifier.render_levels = 0, modifier.total_levels
        if context.mode != "OBJECT":
            bpy.ops.object.mode_set(mode="OBJECT")
        with _shown(context, [detail]), _bake_material(detail, image), _bake_settings(context.scene, bake_type):
            bpy.ops.object.bake_image()
        image.filepath_raw = os.path.join(folder, f"{body.name}_{suffix}.<UDIM>.{extension}")
        image.file_format = file_format
        image.save()
    finally:
        modifier.levels, modifier.render_levels = view
        bpy.data.images.remove(image)
    return [os.path.join(folder, f"{body.name}_{suffix}.{tile}.{extension}") for tile in tiles]
