"""The muscle mesh: a spindle along +Y from 0 to its length, thin tendons at the ends and a belly between. No bpy."""

import math

import numpy as np


def radius_profile(t, belly, tendon):
    """Relative radius (0 to 1) at t in [0, 1]; the widest point sits at belly, the ends at tendon."""
    t = np.clip(np.asarray(t, dtype=float), 0.0, 1.0)
    belly = min(max(belly, 0.05), 0.95)
    warped = t ** (math.log(0.5) / math.log(belly))  # moves belly to 0.5
    bump = np.sin(math.pi * warped) ** 0.75
    return tendon + (1.0 - tendon) * bump


def spindle(length, width, thickness, belly=0.45, tendon=0.15, rings=12, segments=12, fit=None):
    """(vertices, faces): rings of segments around +Y, closed by one vertex at each end. `fit` scales each ring
    vertex's radius, (rings - 2) x segments, to follow the room around the muscle."""
    rings = max(rings, 3)
    segments = max(segments, 3)
    t = np.linspace(0.0, 1.0, rings)[1:-1]
    radius = radius_profile(t, belly, tendon)
    angle = np.linspace(0.0, 2.0 * math.pi, segments, endpoint=False)

    scale = np.ones((len(t), segments)) if fit is None else np.asarray(fit, dtype=float).reshape(len(t), segments)

    vertices = [(0.0, 0.0, 0.0)]
    for ring, (ring_t, ring_radius) in enumerate(zip(t, radius)):
        for segment, a in enumerate(angle):
            r = ring_radius * scale[ring, segment]
            vertices.append((math.cos(a) * width * r, ring_t * length, math.sin(a) * thickness * r))
    vertices.append((0.0, length, 0.0))

    inner = len(t)
    last = len(vertices) - 1
    faces = []
    for s in range(segments):
        n = (s + 1) % segments
        faces.append((0, 1 + n, 1 + s))
        top = 1 + (inner - 1) * segments
        faces.append((last, top + s, top + n))
    for r in range(inner - 1):
        a = 1 + r * segments
        b = a + segments
        for s in range(segments):
            n = (s + 1) % segments
            faces.append((a + s, a + n, b + n, b + s))
    return vertices, faces
