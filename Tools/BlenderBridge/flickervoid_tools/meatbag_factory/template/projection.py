"""Moves per-vertex values from an old body onto new topology: copied where a vertex kept its place, otherwise
blended from the corners of the nearest point on the old surface."""

from dataclasses import dataclass

import numpy as np
from mathutils import Vector, kdtree
from mathutils.bvhtree import BVHTree
from mathutils.interpolate import poly_3d_calc

SAME_PLACE = 1e-5


@dataclass
class Mapping:
    indices: np.ndarray  # (new count, corners) old vertices per new vertex, padded with 0
    weights: np.ndarray  # (new count, corners) their weights, padded with 0
    exact: np.ndarray    # (new count,) True where an old vertex sat at the same place

    def apply(self, values):
        """values: (old count, ...) per old vertex; returns (new count, ...)."""
        return np.einsum("nk,nk...->n...", self.weights, values[self.indices])


def same_topology(old, new):
    if len(old.vertices) != len(new.vertices) or len(old.polygons) != len(new.polygons) or len(old.loops) != len(new.loops):
        return False
    return np.array_equal(_loop_vertices(old), _loop_vertices(new))


def _loop_vertices(mesh):
    values = np.empty(len(mesh.loops), dtype=np.int32)
    mesh.loops.foreach_get("vertex_index", values)
    return values


def mapping(old_mesh, old_points, new_points):
    polygons = [list(polygon.vertices) for polygon in old_mesh.polygons]
    corners = [Vector(point) for point in old_points]
    tree = kdtree.KDTree(len(corners))
    for index, point in enumerate(corners):
        tree.insert(point, index)
    tree.balance()
    surface = BVHTree.FromPolygons(corners, polygons)

    rows = []
    exact = np.zeros(len(new_points), dtype=bool)
    for index, point in enumerate(new_points):
        _, nearest, distance = tree.find(point)
        if distance <= SAME_PLACE:
            rows.append(([nearest], [1.0]))
            exact[index] = True
            continue
        location, _, face, _ = surface.find_nearest(point)
        if face is None:
            rows.append(([nearest], [1.0]))
            continue
        polygon = polygons[face]
        rows.append((polygon, poly_3d_calc([corners[corner] for corner in polygon], location)))

    width = max(len(row[0]) for row in rows) if rows else 1
    indices = np.zeros((len(rows), width), dtype=np.int64)
    weights = np.zeros((len(rows), width))
    for index, (sources, factors) in enumerate(rows):
        indices[index, :len(sources)] = sources
        weights[index, :len(factors)] = factors
    return Mapping(indices, weights, exact)
