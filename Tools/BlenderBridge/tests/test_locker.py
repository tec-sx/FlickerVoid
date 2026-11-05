import math
import os
import tempfile
import unittest

import _path  # noqa: F401
from core import locker, rules
from core.skeleton import BoneInfo, compare, roll_difference, sync_order


class LibraryTests(unittest.TestCase):
    def setUp(self):
        self.library = tempfile.mkdtemp()
        for name in ("Bartender", "Aria", "_Trash", ".git", "NoFile"):
            os.makedirs(os.path.join(self.library, name))
        for name in ("Bartender", "Aria"):
            open(locker.blend_path(self.library, name), "w").close()
        open(os.path.join(self.library, "_Trash", "_Trash.blend"), "w").close()

    def test_characters(self):
        self.assertEqual([item.name for item in locker.characters(self.library)], ["Aria", "Bartender"])
        self.assertEqual(locker.characters(os.path.join(self.library, "missing")), [])

    def test_character_of(self):
        path = locker.blend_path(self.library, "Aria")
        self.assertEqual(locker.character_of(self.library, path), "Aria")
        self.assertEqual(locker.character_of(self.library, os.path.join(self.library, "Aria", "Other.blend")), "")
        self.assertEqual(locker.character_of(self.library, os.path.join(self.library, "Aria.blend")), "")
        self.assertEqual(locker.character_of("", path), "")

    def test_names(self):
        self.assertEqual(locker.name_problem(self.library, "Hero 2"), "")
        self.assertIn("already exists", locker.name_problem(self.library, "Aria"))
        self.assertTrue(locker.name_problem(self.library, "_Hidden"))
        self.assertTrue(locker.name_problem(self.library, "a/b"))
        self.assertTrue(locker.name_problem(self.library, ""))

    def test_trash_keeps_both(self):
        first = locker.trash(self.library, "Aria")
        os.makedirs(os.path.join(self.library, "Aria"))
        second = locker.trash(self.library, "Aria")
        self.assertNotEqual(first, second)
        self.assertTrue(os.path.isdir(first) and os.path.isdir(second))
        self.assertEqual([item.name for item in locker.characters(self.library)], ["Bartender"])

    def test_reference(self):
        self.assertEqual(locker.reference(self.library), "")
        locker.set_reference(self.library, "Bartender")
        self.assertEqual(locker.reference(self.library), "Bartender")

    def test_metadata(self):
        self.assertEqual(locker.parse_metadata("not json"), {"kind": locker.BASE, "base": "", "armature": ""})
        text = locker.format_metadata({"kind": locker.HERO, "base": "Bartender", "armature": "Rig", "extra": 1})
        self.assertEqual(locker.parse_metadata(text), {"kind": locker.HERO, "base": "Bartender", "armature": "Rig"})
        self.assertEqual(locker.parse_metadata('{"kind": "VILLAIN"}')["kind"], locker.BASE)

    def test_catalogs(self):
        self.assertTrue(locker.ensure_catalogs(self.library))
        self.assertFalse(locker.ensure_catalogs(self.library))
        with open(os.path.join(self.library, locker.CATALOGS)) as file:
            text = file.read()
        self.assertIn("VERSION 1", text)
        for kind in locker.KINDS:
            self.assertEqual(text.count(locker.CATALOG_IDS[kind]), 1)

    def test_catalogs_keep_others(self):
        path = os.path.join(self.library, locker.CATALOGS)
        with open(path, "w") as file:
            file.write("VERSION 1\n\n11111111-2222-3333-4444-555555555555:Props:Props\n")
        locker.ensure_catalogs(self.library)
        with open(path) as file:
            text = file.read()
        self.assertIn("Props:Props", text)
        self.assertIn(locker.CATALOG_PATHS[locker.HERO], text)


class SkeletonTests(unittest.TestCase):
    REFERENCE = {
        "root": BoneInfo(None, 0.0),
        "spine": BoneInfo("root", 0.0),
        "upperarm_l": BoneInfo("spine", 0.5),
        "deltoid_l": BoneInfo("upperarm_l", 0.0),
        "deltoid_tip_l": BoneInfo("deltoid_l", 0.0),
    }

    def test_matching(self):
        self.assertEqual(compare("Rig", self.REFERENCE, dict(self.REFERENCE)), [])

    def test_differences(self):
        current = {
            "root": BoneInfo(None, 0.0),
            "spine": BoneInfo("root", 2 * math.pi),
            "upperarm_l": BoneInfo("root", 0.6),
            "tail": BoneInfo("root", 0.0),
        }
        issues = compare("Rig", self.REFERENCE, current)
        parents = next(issue for issue in issues if "parent" in issue.message)
        self.assertEqual((parents.level, parents.items), (rules.ERROR, ["upperarm_l"]))
        rolls = next(issue for issue in issues if "roll" in issue.message)
        self.assertEqual(rolls.items, ["upperarm_l"])
        missing = next(issue for issue in issues if "missing" in issue.message)
        self.assertIn("deltoid_l", missing.message)
        extra = next(issue for issue in issues if "aren't in the reference" in issue.message)
        self.assertEqual(extra.items, ["tail"])
        self.assertEqual(len(issues), 4)

    def test_roll_wraps(self):
        self.assertAlmostEqual(roll_difference(math.pi - 0.01, -math.pi + 0.01), 0.02)

    def test_sync_order_puts_parents_first(self):
        current = {"root": BoneInfo(None, 0.0), "spine": BoneInfo("root", 0.0)}
        reference = dict(reversed(list(self.REFERENCE.items())))
        self.assertEqual(sync_order(reference, current), ["upperarm_l", "deltoid_l", "deltoid_tip_l"])


if __name__ == "__main__":
    unittest.main()
