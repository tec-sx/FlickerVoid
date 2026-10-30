import unittest

import numpy as np

import _path  # noqa: F401
from meatbag_factory import solver


def rotation(axis, angle):
    axis = np.asarray(axis, float) / np.linalg.norm(axis)
    k = np.array([[0, -axis[2], axis[1]], [axis[2], 0, -axis[0]], [-axis[1], axis[0], 0]])
    return np.eye(3) + np.sin(angle) * k + (1 - np.cos(angle)) * k @ k


def affine(rot, translation, scale=1.0):
    m = np.zeros((3, 4))
    m[:, :3] = scale * rot
    m[:, 3] = translation
    return m


def limb(frames=4, seed=0, helper_scale=1.0):
    """A cylinder-ish point cloud on two main bones, with a helper bone over the joint."""
    rng = np.random.default_rng(seed)
    n = 400
    z = rng.uniform(0, 2, n)
    angle = rng.uniform(0, 2 * np.pi, n)
    rest = np.stack([0.3 * np.cos(angle), 0.3 * np.sin(angle), z], axis=1)
    upper = np.clip(1.5 - z, 0, 1)
    lower = 1 - upper
    helper = np.exp(-((z - 1.0) ** 2) / 0.05) * 0.5
    weights = np.stack([upper * (1 - helper), lower * (1 - helper), helper], axis=1)
    weights /= weights.sum(axis=1, keepdims=True)

    true = np.zeros((frames, 3, 3, 4))
    start = np.zeros((frames, 3, 3, 4))
    for f in range(frames):
        bend = rotation([1, 0, 0], 0.3 * (f + 1))
        pivot = np.array([0, 0, 1.0])
        lower_m = affine(bend, pivot - bend @ pivot)
        true[f, 0] = start[f, 0] = affine(np.eye(3), np.zeros(3))
        true[f, 1] = start[f, 1] = lower_m
        bulge = rotation([0, 1, 0.3], 0.2 * (f + 1))
        true[f, 2] = affine(bulge, rng.normal(0, 0.05, 3), helper_scale ** (f + 1))
        start[f, 2] = true[f, 0]
    targets = solver.skin_points(rest, weights, true)
    return rest, weights, start, targets, true


class SolverTests(unittest.TestCase):
    def test_skin_identity(self):
        rest = np.random.default_rng(1).normal(size=(10, 3))
        skin = np.zeros((1, 2, 3, 4))
        skin[0, :, :, :3] = np.eye(3)
        weights = np.full((10, 2), 0.5)
        np.testing.assert_allclose(solver.skin_points(rest, weights, skin)[0], rest)

    def test_fit_affine_recovers_transform(self):
        rng = np.random.default_rng(2)
        points = rng.normal(size=(50, 3))
        truth = affine(rotation([1, 2, 3], 0.7), np.array([1.0, -2.0, 0.5]), 1.3)
        targets = points @ truth[:, :3].T + truth[:, 3]
        np.testing.assert_allclose(solver.fit_affine(points, targets, np.ones(50), True), truth, atol=1e-9)
        rigid = solver.fit_affine(points, targets, np.ones(50), False)
        self.assertAlmostEqual(abs(np.linalg.det(rigid[:, :3])), 1.0, places=9)

    def test_nnls(self):
        a = np.array([[1.0, 0.0], [0.0, 1.0], [1.0, 1.0]])
        np.testing.assert_allclose(solver.nnls(a, np.array([1.0, -1.0, 0.0])), [0.5, 0.0], atol=1e-9)
        np.testing.assert_allclose(solver.nnls(a, a @ np.array([0.3, 0.7])), [0.3, 0.7], atol=1e-9)

    def test_recovers_helper_transforms(self):
        rest, weights, start, targets, true = limb()
        problem = solver.Problem(rest, weights, start, targets, np.array([False, False, True]))
        result = solver.solve(problem, solver.Options(iterations=5))
        self.assertGreater(result.rms(result.initial_error), 0.01)
        self.assertLess(result.rms(), 1e-8)
        np.testing.assert_array_equal(result.skin[:, :2], start[:, :2])

    def test_recovers_scale_only_when_allowed(self):
        rest, weights, start, targets, _ = limb(helper_scale=1.15)
        problem = solver.Problem(rest, weights, start, targets, np.array([False, False, True]))
        rigid = solver.solve(problem, solver.Options(iterations=5))
        scaled = solver.solve(problem, solver.Options(iterations=5, allow_scale=True))
        self.assertLess(scaled.rms(), 1e-8)
        self.assertGreater(rigid.rms(), scaled.rms())

    def test_weights_improve_fit(self):
        rest, weights, start, targets, _ = limb()
        weak = weights.copy()
        weak[:, 2] *= 0.3
        weak /= weak.sum(axis=1, keepdims=True)
        problem = solver.Problem(rest, weak, start, targets, np.array([False, False, True]))
        fixed = solver.solve(problem, solver.Options(iterations=10))
        candidates = [np.array([2])] * len(rest)
        solved = solver.solve(problem, solver.Options(iterations=3, solve_weights=True, weight_rounds=10, candidates=candidates))
        self.assertLess(solved.rms(), fixed.rms() * 0.7)
        np.testing.assert_allclose(solved.weights.sum(axis=1), 1.0, atol=1e-9)
        self.assertTrue((solved.weights >= 0).all())

    def test_limit_influences(self):
        row = solver.limit_influences(np.array([0.5, 0.25, 0.15, 0.1]), 2)
        np.testing.assert_allclose(row, [0.5 / 0.75, 0.25 / 0.75, 0, 0])


if __name__ == "__main__":
    unittest.main()
