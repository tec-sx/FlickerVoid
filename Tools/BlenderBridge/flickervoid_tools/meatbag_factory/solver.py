"""Helper bone fitting by Smooth Skinning Decomposition with Rigid Bones (SSDR).

A numpy port of the parts of Dem Bones (Le and Deng; Electronic Arts, BSD-3) that the Meatbag Factory
needs: main bones are locked to their posed transforms, and only helper bones are solved, so that linear
blend skinning reproduces target meshes (a muscle simulation, a sculpt) in each example pose.

Shapes: n vertices, B bones, F example poses. Skin matrices are 3x4 affine maps from rest to posed space
(posed bone matrix times inverse bind matrix), all in armature space.
"""

from dataclasses import dataclass, field

import numpy as np

WEIGHT_EPSILON = 1e-4


@dataclass
class Problem:
    rest: np.ndarray        # (n, 3)
    weights: np.ndarray     # (n, B), rows sum to 1 where skinned
    skin: np.ndarray        # (F, B, 3, 4), helpers at their initial transforms
    targets: np.ndarray     # (F, n, 3)
    helpers: np.ndarray     # (B,) bool, the bones whose transforms are solved


@dataclass
class Options:
    iterations: int = 5
    allow_scale: bool = False
    solve_weights: bool = False
    weight_rounds: int = 10
    max_influences: int = 8
    candidates: list = None     # per vertex, helper bone indices that may receive weight; None means its current helpers
    weight_vertices: np.ndarray = None  # (n,) bool, vertices whose weights may change; None means all
    tolerance: float = 1e-6


@dataclass
class Result:
    skin: np.ndarray
    weights: np.ndarray
    error: np.ndarray       # (F, n) distance to the target
    initial_error: np.ndarray
    history: list = field(default_factory=list)  # RMS error after each round

    def rms(self, error=None):
        error = self.error if error is None else error
        return float(np.sqrt(np.mean(error ** 2))) if error.size else 0.0


def homogeneous(points):
    return np.concatenate([points, np.ones((len(points), 1))], axis=1)


def skin_points(rest, weights, skin):
    """Linear blend skinning: (F, n, 3)."""
    rest_h = homogeneous(rest)
    per_bone = np.einsum("fbij,nj->fbni", skin, rest_h)
    return np.einsum("nb,fbni->fni", weights, per_bone)


def fit_affine(points, targets, weights, allow_scale):
    """Weighted Procrustes (Umeyama): the rotation, optional uniform scale and translation taking points to targets."""
    total = weights.sum()
    if total <= 0.0:
        return None
    mean_p = weights @ points / total
    mean_q = weights @ targets / total
    centered_p = points - mean_p
    centered_q = targets - mean_q
    covariance = (centered_q * weights[:, None]).T @ centered_p
    u, s, vt = np.linalg.svd(covariance)
    d = np.ones(3)
    d[2] = np.sign(np.linalg.det(u @ vt)) or 1.0
    rotation = u @ np.diag(d) @ vt
    scale = 1.0
    if allow_scale:
        variance = np.sum(weights * np.sum(centered_p ** 2, axis=1))
        if variance > 0.0:
            scale = float(np.sum(s * d) / variance)
    matrix = np.zeros((3, 4))
    matrix[:, :3] = scale * rotation
    matrix[:, 3] = mean_q - scale * rotation @ mean_p
    return matrix


def solve_transforms(problem, skin, weights, options):
    """One Gauss-Seidel sweep over the helper bones, every pose: each helper is fitted with the others fixed."""
    rest_h = homogeneous(problem.rest)
    posed = skin_points(problem.rest, weights, skin)
    for bone in np.flatnonzero(problem.helpers):
        column = weights[:, bone]
        vertices = np.flatnonzero(column > WEIGHT_EPSILON)
        if len(vertices) < 3:
            continue
        w = column[vertices]
        points = problem.rest[vertices]
        for frame in range(len(skin)):
            own = w[:, None] * (rest_h[vertices] @ skin[frame, bone].T)
            others = posed[frame, vertices] - own
            goal = (problem.targets[frame, vertices] - others) / w[:, None]
            matrix = fit_affine(points, goal, w ** 2, options.allow_scale)
            if matrix is None:
                continue
            skin[frame, bone] = matrix
            posed[frame, vertices] = others + w[:, None] * (rest_h[vertices] @ matrix.T)
    return skin


def nnls(a, b, max_iterations=None):
    """Lawson-Hanson non-negative least squares: argmin |a x - b| with x >= 0."""
    rows, cols = a.shape
    x = np.zeros(cols)
    passive = np.zeros(cols, dtype=bool)
    gradient = a.T @ (b - a @ x)
    tolerance = 1e-10 * max(1.0, np.abs(a).max())
    for _ in range(max_iterations or 3 * cols):
        candidates = (~passive) & (gradient > tolerance)
        if not candidates.any():
            break
        passive[np.argmax(np.where(candidates, gradient, -np.inf))] = True
        while True:
            z = np.zeros(cols)
            z[passive] = np.linalg.lstsq(a[:, passive], b, rcond=None)[0]
            if (z[passive] > 0).all():
                x = z
                break
            blocking = passive & (z <= 0)
            step = x[blocking] - z[blocking]
            ratios = np.where(step > 0, x[blocking] / np.where(step > 0, step, 1.0), np.inf)
            alpha = float(np.min(ratios)) if np.isfinite(ratios).any() else 0.0
            x = x + alpha * (z - x)
            passive &= x > tolerance
            x[~passive] = 0.0
            if not passive.any():
                break
        gradient = a.T @ (b - a @ x)
    return x


def solve_helper_weights(problem, skin, weights, options):
    """Gives helpers their share of each vertex; the main bones keep their painted proportions in the rest.

    Posed position = main_pos + sum_k h_k (helper_k pos - main_pos) with h_k >= 0 and sum h_k <= 1,
    which is linear in h, so each vertex is one small non-negative least squares problem.
    """
    rest_h = homogeneous(problem.rest)
    helpers = problem.helpers
    weights = weights.copy()
    vertices = range(len(problem.rest)) if options.weight_vertices is None else np.flatnonzero(options.weight_vertices)
    for vertex in vertices:
        main = weights[vertex] * ~helpers
        main_total = main.sum()
        if main_total <= 0.0:
            continue
        candidates = options.candidates[vertex] if options.candidates is not None else np.flatnonzero(helpers & (weights[vertex] > 0))
        if len(candidates) == 0:
            continue
        main = main / main_total
        per_bone = skin[:, :, :, :] @ rest_h[vertex]                     # (F, B, 3)
        main_pos = np.einsum("b,fbi->fi", main, per_bone)                 # (F, 3)
        columns = (per_bone[:, candidates] - main_pos[:, None, :])        # (F, K, 3)
        a = columns.transpose(0, 2, 1).reshape(-1, len(candidates))
        b = (problem.targets[:, vertex] - main_pos).reshape(-1)
        h = nnls(a, b)
        total = h.sum()
        if total > 1.0:
            h /= total
            total = 1.0
        row = main * (1.0 - total)
        row[candidates] = h
        weights[vertex] = limit_influences(row, options.max_influences)
    return weights


def limit_influences(row, max_influences):
    nonzero = np.flatnonzero(row > WEIGHT_EPSILON)
    if len(nonzero) > max_influences:
        drop = nonzero[np.argsort(row[nonzero])[:-max_influences]]
        row = row.copy()
        row[drop] = 0.0
    row = np.where(row > WEIGHT_EPSILON, row, 0.0)
    total = row.sum()
    return row / total if total > 0.0 else row


def distances(problem, skin, weights):
    return np.linalg.norm(skin_points(problem.rest, weights, skin) - problem.targets, axis=2)


def solve(problem, options=None):
    options = options or Options()
    skin = problem.skin.copy()
    weights = problem.weights.copy()
    initial = distances(problem, skin, weights)
    history = [float(np.sqrt(np.mean(initial ** 2)))]

    rounds = options.weight_rounds if options.solve_weights else 1
    for round_index in range(rounds):
        previous = history[-1]
        for _ in range(options.iterations):
            skin = solve_transforms(problem, skin, weights, options)
            error = float(np.sqrt(np.mean(distances(problem, skin, weights) ** 2)))
            history.append(error)
            if previous - error <= options.tolerance * max(previous, 1.0):
                break
            previous = error
        if options.solve_weights:
            weights = solve_helper_weights(problem, skin, weights, options)
            if round_index == rounds - 1:
                for _ in range(options.iterations):
                    skin = solve_transforms(problem, skin, weights, options)
                history.append(float(np.sqrt(np.mean(distances(problem, skin, weights) ** 2))))

    return Result(skin=skin, weights=weights, error=distances(problem, skin, weights), initial_error=initial, history=history)
