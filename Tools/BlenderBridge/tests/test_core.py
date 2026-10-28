import os
import tempfile
import unittest

import _path  # noqa: F401
from core import rules
from core.contract import Contract, load_side_pairs
from core.ini import parse_ini, parse_struct
from core.sides import CENTRE, LEFT, RIGHT, SideNames


class IniTests(unittest.TestCase):
    def test_array_operators(self):
        ini = parse_ini("[S]\n!Pairs=ClearArray\n+Pairs=a\n+Pairs=b\n+Pairs=a\n.Pairs=a\n-Pairs=b\nKey=1\nKey=2\n")
        self.assertEqual(ini["S"]["Pairs"], ["a", "a"])
        self.assertEqual(ini["S"]["Key"], ["2"])

    def test_cleared_key_is_present(self):
        self.assertEqual(parse_ini("[S]\n!Pairs=ClearArray\n")["S"]["Pairs"], [])

    def test_struct(self):
        self.assertEqual(parse_struct('(Left="_l",Right="_r")'), {"Left": "_l", "Right": "_r"})
        self.assertEqual(parse_struct("(Left=.L, Right=.R)"), {"Left": ".L", "Right": ".R"})


class SidePairTests(unittest.TestCase):
    def _project(self, text):
        folder = tempfile.mkdtemp()
        os.makedirs(os.path.join(folder, "Config"))
        with open(os.path.join(folder, "Config", "DefaultGame.ini"), "w") as file:
            file.write(text)
        return folder

    def test_reads_project_setting(self):
        folder = self._project('[/Script/FVAnimationSystem.FVAnimationSettings]\n!SidePairs=ClearArray\n+SidePairs=(Left="_l",Right="_r")\n+SidePairs=(Left="Left",Right="Right")\n')
        pairs, source = load_side_pairs(folder)
        self.assertEqual(pairs, (("_l", "_r"), ("Left", "Right")))
        self.assertTrue(source.endswith("DefaultGame.ini"))

    def test_default_when_missing(self):
        self.assertEqual(load_side_pairs(self._project("[Other]\nA=1\n"))[0], (("_l", "_r"),))
        self.assertEqual(load_side_pairs("")[0], (("_l", "_r"),))


class SideNameTests(unittest.TestCase):
    sides = SideNames([("_l", "_r")])

    def test_bone_suffix(self):
        self.assertEqual(self.sides.bone_side("upperarm_l"), LEFT)
        self.assertEqual(self.sides.bone_side("thumb_01_R"), RIGHT)
        self.assertEqual(self.sides.bone_side("spine_03"), CENTRE)
        self.assertEqual(self.sides.mirror_bone("upperarm_twist_01_l"), "upperarm_twist_01_r")
        self.assertEqual(self.sides.mirror_bone("hand_L"), "hand_R")
        self.assertIsNone(self.sides.mirror_bone("head"))
        self.assertIsNone(self.sides.mirror_bone("_l"))

    def test_name_tokens(self):
        self.assertEqual(self.sides.name_side("upperarm_l_up_90"), LEFT)
        self.assertEqual(self.sides.mirror_name("CR_thigh_l_raise"), "CR_thigh_r_raise")
        self.assertEqual(self.sides.name_side("hand_l_hand_r"), CENTRE)
        self.assertIsNone(self.sides.mirror_name("ArmRaise"))
        self.assertEqual(self.sides.name_side("leg_lower"), CENTRE)


class RuleTests(unittest.TestCase):
    contract = Contract(side_pairs=(("_l", "_r"),), required_bones=("root", "pelvis"), bone_budget=4, max_influences=2)

    def test_armature(self):
        bones = [("root", None), ("pelvis", "root"), ("thigh_l", "pelvis"), ("thigh_r", "pelvis"), ("calf_l", "thigh_l")]
        messages = [issue.message for issue in rules.check_armature("Arm", bones, self.contract)]
        self.assertTrue(any("calf_l" in message for message in messages))
        self.assertTrue(any("budget" in message for message in messages))
        self.assertFalse(any("top bone" in message for message in messages))

    def test_two_top_bones(self):
        issues = rules.check_armature("Arm", [("root", None), ("pelvis", None)], self.contract)
        self.assertEqual(len([issue for issue in issues if "top bone" in issue.message]), 1)

    def test_weights(self):
        issues = rules.check_weights("Body", [[1.0], [0.5, 0.3, 0.2], [0.5, 0.2], []], 2)
        by_fix = {issue.fix: issue.items for issue in issues}
        self.assertEqual(by_fix[rules.FIX_LIMIT_INFLUENCES], [1])
        self.assertEqual(by_fix[rules.FIX_NORMALIZE], [2])
        self.assertEqual(by_fix[""], [3])

    def test_limit_and_normalize(self):
        result = rules.limit_and_normalize({"a": 0.6, "b": 0.3, "c": 0.1}, 2)
        self.assertEqual(set(result), {"a", "b"})
        self.assertAlmostEqual(sum(result.values()), 1.0)


if __name__ == "__main__":
    unittest.main()
