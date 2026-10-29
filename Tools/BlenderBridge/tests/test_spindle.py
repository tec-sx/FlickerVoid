import unittest

import numpy as np

import _path  # noqa: F401
from meatbag_factory import spindle


class SpindleTest(unittest.TestCase):
    def test_profile_peaks_at_belly_and_keeps_tendons(self):
        t = np.linspace(0.0, 1.0, 201)
        radius = spindle.radius_profile(t, belly=0.3, tendon=0.2)
        self.assertAlmostEqual(float(t[np.argmax(radius)]), 0.3, delta=0.01)
        self.assertAlmostEqual(float(radius.max()), 1.0, places=3)
        self.assertAlmostEqual(float(radius[0]), 0.2)
        self.assertAlmostEqual(float(radius[-1]), 0.2)

    def test_mesh_spans_length_and_belly_size(self):
        vertices, faces = spindle.spindle(40.0, 5.0, 3.0, belly=0.5, tendon=0.1, rings=21, segments=16)
        points = np.array(vertices)
        self.assertEqual(len(points), 2 + 19 * 16)
        self.assertAlmostEqual(points[:, 1].min(), 0.0)
        self.assertAlmostEqual(points[:, 1].max(), 40.0)
        self.assertAlmostEqual(np.abs(points[:, 0]).max(), 5.0, places=2)
        self.assertAlmostEqual(np.abs(points[:, 2]).max(), 3.0, places=1)
        self.assertTrue(all(max(face) < len(points) for face in faces))
        edges = {}
        for face in faces:
            for a, b in zip(face, face[1:] + face[:1]):
                key = (min(a, b), max(a, b))
                edges[key] = edges.get(key, 0) + 1
        self.assertTrue(all(count == 2 for count in edges.values()), "the spindle must be closed")

    def test_fit_scales_each_ring_vertex(self):
        plain, _ = spindle.spindle(10.0, 2.0, 2.0)
        fit = np.ones((10, 12))
        fit[4, 3] = 0.5
        fitted, _ = spindle.spindle(10.0, 2.0, 2.0, fit=fit)
        index = 1 + 4 * 12 + 3
        self.assertAlmostEqual(np.hypot(fitted[index][0], fitted[index][2]), 0.5 * np.hypot(plain[index][0], plain[index][2]))
        self.assertEqual(fitted[index + 1], plain[index + 1])


if __name__ == "__main__":
    unittest.main()
