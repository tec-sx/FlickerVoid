import unittest

import _path  # noqa: F401
from meatbag_factory import anatomy

UE5 = {"root", "pelvis", "spine_01", "spine_02", "spine_03", "spine_04", "spine_05", "neck_01", "neck_02", "head",
       *(f"{bone}_{side}" for side in "lr" for bone in ("clavicle", "upperarm", "lowerarm", "hand", "thigh", "calf", "foot", "ball"))}
UE4 = UE5 - {"spine_04", "spine_05", "neck_02"}


class AnatomyTest(unittest.TestCase):
    def test_both_mannequins_fit(self):
        self.assertEqual(anatomy.missing_bones(UE5), [])
        self.assertEqual(anatomy.missing_bones(UE4), [])
        self.assertEqual(anatomy.resolve_bone("chest", UE5), "spine_05")
        self.assertEqual(anatomy.resolve_bone("chest", UE4), "spine_03")

    def test_other_skeleton_is_rejected(self):
        self.assertIn("pelvis", anatomy.missing_bones({"hips", "spine", "chest"}))

    def test_template_uses_only_known_bones_on_the_left(self):
        for muscle in anatomy.TEMPLATE:
            self.assertTrue(muscle.name.endswith("_l"), muscle.name)
            for origin, insertion in anatomy.segments(muscle):
                for attachment in (origin, insertion):
                    self.assertIsNotNone(anatomy.resolve_bone(attachment.bone, UE4), (muscle.name, attachment.bone))
                    self.assertFalse(attachment.bone.endswith("_r"), muscle.name)
                self.assertNotEqual(origin.bone, insertion.bone, muscle.name)

    def test_spine_paths_step_through_every_spine_bone(self):
        erector = next(muscle for muscle in anatomy.TEMPLATE if muscle.name == "ErectorSpinae_l")
        steps = anatomy.segments(erector)
        self.assertEqual([origin.bone for origin, _ in steps], ["pelvis", "spine_01", "spine_02", "midspine"])
        self.assertEqual(anatomy.strand_name(erector, 3), "ErectorSpinae4_l")

    def test_joint_pivots_move_with_both_bones(self):
        self.assertEqual(anatomy.skin_bones("@Knee_l"), ("thigh_l", "calf_l"))
        self.assertEqual(anatomy.skin_bones("calf_l"), ("calf_l",))
        self.assertEqual(anatomy.resolve_bone("@Elbow_r", UE5), "@Elbow_r")
        self.assertIsNone(anatomy.resolve_bone("@Wrist_l", UE5))

    def test_fan_strands_are_numbered_before_the_side(self):
        pectoral = next(muscle for muscle in anatomy.TEMPLATE if muscle.name == "Pectoral_l")
        self.assertEqual([anatomy.strand_name(pectoral, i) for i in range(3)], ["Pectoral1_l", "Pectoral2_l", "Pectoral3_l"])
        biceps = next(muscle for muscle in anatomy.TEMPLATE if muscle.name == "Biceps_l")
        self.assertEqual(anatomy.strand_name(biceps, 0), "Biceps_l")


if __name__ == "__main__":
    unittest.main()
