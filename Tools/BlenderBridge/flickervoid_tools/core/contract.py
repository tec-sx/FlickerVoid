"""The character contract: side pairs from the game project's config, the other rules from the add-on preferences."""

import os
from dataclasses import dataclass

from .ini import parse_struct, read_ini
from .sides import SideNames

ANIMATION_SETTINGS_SECTION = "/Script/FVAnimationSystem.FVAnimationSettings"
DEFAULT_SIDE_PAIRS = (("_l", "_r"),)

MANNEQUIN_BODY_CHAIN = (
    "root", "pelvis",
    "spine_01", "spine_02", "spine_03", "spine_04", "spine_05",
    "neck_01", "neck_02", "head",
    "clavicle_l", "upperarm_l", "lowerarm_l", "hand_l",
    "clavicle_r", "upperarm_r", "lowerarm_r", "hand_r",
    "thigh_l", "calf_l", "foot_l", "ball_l",
    "thigh_r", "calf_r", "foot_r", "ball_r",
)


@dataclass(frozen=True)
class Contract:
    side_pairs: tuple
    required_bones: tuple
    bone_budget: int = 300
    max_influences: int = 8
    unit_scale: float = 0.01
    side_pairs_source: str = "default"

    @property
    def sides(self):
        return SideNames(self.side_pairs)


def load_side_pairs(project_dir):
    """Returns (pairs, source). Falls back to the engine default when the project or its setting is missing."""
    if not project_dir:
        return DEFAULT_SIDE_PAIRS, "default (no Unreal project set)"
    path = os.path.join(project_dir, "Config", "DefaultGame.ini")
    if not os.path.isfile(path):
        return DEFAULT_SIDE_PAIRS, f"default ({path} not found)"

    values = read_ini(path).get(ANIMATION_SETTINGS_SECTION, {}).get("SidePairs")
    if values is None:
        return DEFAULT_SIDE_PAIRS, f"default (not set in {path})"

    pairs = []
    for value in values:
        fields = parse_struct(value)
        pairs.append((fields.get("Left", DEFAULT_SIDE_PAIRS[0][0]), fields.get("Right", DEFAULT_SIDE_PAIRS[0][1])))
    return tuple(pairs), path


def parse_bone_list(text):
    return tuple(name.strip() for name in text.replace("\n", ",").split(",") if name.strip())
