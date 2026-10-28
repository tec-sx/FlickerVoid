"""Contract checks on plain data, so they run and test without Blender."""

from dataclasses import dataclass, field

ERROR = "ERROR"
WARNING = "WARNING"

BONES = "BONES"
VERTICES = "VERTICES"
NONE = "NONE"

FIX_LIMIT_INFLUENCES = "LIMIT_INFLUENCES"
FIX_NORMALIZE = "NORMALIZE"

WEIGHT_SUM_TOLERANCE = 1e-3


@dataclass
class Issue:
    level: str
    message: str
    object_name: str = ""
    item_kind: str = NONE
    items: list = field(default_factory=list)
    fix: str = ""


def check_unit_scale(scale, expected):
    if abs(scale - expected) > 1e-6:
        return [Issue(ERROR, f"Scene unit scale is {scale:g}, the contract needs {expected:g} (centimetres)")]
    return []


def check_armature(name, bones, contract):
    """bones: list of (bone name, parent name or None)."""
    issues = []
    names = {bone for bone, _ in bones}
    sides = contract.sides

    unpaired = sorted(bone for bone in names if (mirror := sides.mirror_bone(bone)) and mirror not in names)
    if unpaired:
        issues.append(Issue(ERROR, f"{len(unpaired)} side bones have no counterpart: {_preview(unpaired)}", name, BONES, unpaired))

    missing = [bone for bone in contract.required_bones if bone not in names]
    if missing:
        issues.append(Issue(ERROR, f"Missing required bones: {_preview(missing)}", name))

    if len(bones) > contract.bone_budget:
        issues.append(Issue(ERROR, f"{len(bones)} bones, the budget is {contract.bone_budget}", name))

    tops = sorted(bone for bone, parent in bones if parent is None)
    if len(tops) != 1:
        issues.append(Issue(ERROR, f"Needs exactly one top bone, found {len(tops)}: {_preview(tops)}", name, BONES, tops))
    return issues


def check_vertex_groups(name, group_names, bone_names):
    orphans = sorted(group for group in group_names if group not in bone_names)
    if orphans:
        return [Issue(WARNING, f"{len(orphans)} vertex groups match no deform bone: {_preview(orphans)}", name)]
    return []


def check_weights(name, vertex_weights, max_influences):
    """vertex_weights: per vertex, the list of deform weights above zero."""
    over, unnormalized, unweighted = [], [], []
    for index, weights in enumerate(vertex_weights):
        total = sum(weights)
        if total <= 0.0:
            unweighted.append(index)
            continue
        if len(weights) > max_influences:
            over.append(index)
        if abs(total - 1.0) > WEIGHT_SUM_TOLERANCE:
            unnormalized.append(index)

    issues = []
    if unweighted:
        issues.append(Issue(ERROR, f"{len(unweighted)} vertices have no deform weight", name, VERTICES, unweighted))
    if over:
        issues.append(Issue(WARNING, f"{len(over)} vertices have more than {max_influences} influences", name, VERTICES, over, FIX_LIMIT_INFLUENCES))
    if unnormalized:
        issues.append(Issue(WARNING, f"{len(unnormalized)} vertices have weights that don't sum to 1", name, VERTICES, unnormalized, FIX_NORMALIZE))
    return issues


def limit_and_normalize(weights, max_influences):
    """UE's pruning of one vertex's {bone: weight}: keep the largest influences, then normalize."""
    kept = sorted(weights.items(), key=lambda item: item[1], reverse=True)[:max_influences]
    total = sum(weight for _, weight in kept)
    if total <= 0.0:
        return {}
    return {bone: weight / total for bone, weight in kept}


def _preview(items, count=5):
    items = list(items)
    shown = ", ".join(str(item) for item in items[:count])
    return shown + (f" and {len(items) - count} more" if len(items) > count else "")
