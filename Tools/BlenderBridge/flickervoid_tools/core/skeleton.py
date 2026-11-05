"""Skeleton checks against the library's Reference character, on plain data so they run and test without Blender.

Both skeletons are their deform bones as Unreal sees them, each parented to its nearest deform ancestor. Positions
aren't compared, since every character has its own proportions and rest pose.
"""

import math
from dataclasses import dataclass

from .rules import BONES, ERROR, WARNING, Issue, preview

ROLL_TOLERANCE = math.radians(0.5)


@dataclass
class BoneInfo:
    parent: object
    roll: float


def roll_difference(a, b):
    return abs((a - b + math.pi) % (2.0 * math.pi) - math.pi)


def compare(name, reference, current, roll_tolerance=ROLL_TOLERANCE):
    """Issues for an armature's bones against the reference's, both as {bone: BoneInfo}."""
    common = [bone for bone in reference if bone in current]
    parents = sorted(bone for bone in common if current[bone].parent != reference[bone].parent)
    missing = [bone for bone in reference if bone not in current]
    rolls = sorted(bone for bone in common if roll_difference(current[bone].roll, reference[bone].roll) > roll_tolerance)
    extra = sorted(bone for bone in current if bone not in reference)

    issues = []
    if parents:
        issues.append(Issue(ERROR, f"{len(parents)} bones have another parent than in the reference: {preview(parents)}", name, BONES, parents))
    if missing:
        issues.append(Issue(WARNING, f"{len(missing)} reference bones are missing, Sync Skeleton adds them: {preview(missing)}", name))
    if rolls:
        issues.append(Issue(WARNING, f"{len(rolls)} bones have another roll than in the reference: {preview(rolls)}", name, BONES, rolls))
    if extra:
        issues.append(Issue(WARNING, f"{len(extra)} bones aren't in the reference: {preview(extra)}", name, BONES, extra))
    return issues


def sync_order(reference, current):
    """The reference bones missing from current, parents before their children."""
    have = set(current)
    pending = [bone for bone in reference if bone not in have]
    order = []
    while pending:
        rest = [bone for bone in pending if reference[bone].parent is not None and reference[bone].parent not in have]
        ready = [bone for bone in pending if bone not in rest]
        if not ready:
            break
        order += ready
        have.update(ready)
        pending = rest
    return order
