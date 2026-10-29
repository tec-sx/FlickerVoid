"""The human muscle template for Unreal Mannequin skeletons (UE4 and UE5 bone names). No bpy.

Every muscle is written for the left side and mirrored. A muscle is a group of strands: one strand for a simple
muscle, a fan of strands for a muscle with several attachments (pectoral, deltoid, trapezius). A pad is a flatter
strand that also follows extra bones through hooks, for broad shapes that several joints bend (gluteus).

An attachment is (bone, along, direction, depth): the point `along` the bone (0 head, 1 tail), pushed toward the
skin in `direction` (front, left, up of the body) and kept `depth` of the way back from the skin, so attachments
sit under the surface on any body shape.
"""

from dataclasses import dataclass, field

STRAND = "STRAND"
PAD = "PAD"

# Template names for bones that differ between Mannequin generations: the first existing bone is used.
ALIASES = {
    "chest": ("spine_05", "spine_03"),
    "midspine": ("spine_03", "spine_02"),
    "neck": ("neck_01",),
}

REQUIRED = ("pelvis", "spine_01", "spine_02", "spine_03", "neck_01", "head", "clavicle_l", "upperarm_l", "lowerarm_l",
            "hand_l", "thigh_l", "calf_l", "foot_l", "clavicle_r", "upperarm_r", "thigh_r")


@dataclass(frozen=True)
class Attachment:
    bone: str
    along: float
    direction: tuple
    depth: float = 0.4


@dataclass(frozen=True)
class Muscle:
    name: str
    strands: tuple
    kind: str = STRAND
    hooks: tuple = ()
    width: float = 0.85
    thickness: float = 0.85
    belly: float = 0.45
    tendon: float = 0.15
    color: tuple = (0.75, 0.2, 0.2)


def _a(bone, along, front=0.0, left=0.0, up=0.0, depth=0.4):
    return Attachment(bone, along, (front, left, up), depth)


def _fan(origins, insertion):
    return tuple((origin, insertion) for origin in origins)


RED = (0.75, 0.2, 0.2)
DARK = (0.55, 0.12, 0.15)
PINK = (0.85, 0.4, 0.4)

TEMPLATE = (
    # Neck and shoulders
    Muscle("Sternocleidomastoid_l", ((_a("head", 0.0, -0.3, 1, -0.5), _a("clavicle_l", 0.05, 1, 0.2, 0.3)),),
           width=0.6, thickness=0.6, color=PINK),
    Muscle("Trapezius_l", _fan((_a("neck", 0.3, -1, 0.2), _a("chest", 0.6, -1, 0.15), _a("midspine", 0.3, -1, 0.1)),
                               _a("clavicle_l", 0.8, -0.5, 0, 1)),
           width=0.7, thickness=0.5, tendon=0.3, color=DARK),
    Muscle("Deltoid_l", ((_a("clavicle_l", 0.8, 1, 0, 0.5), _a("upperarm_l", 0.4, 0.3, 0, 1)),
                         (_a("clavicle_l", 1.0, 0, 0, 1), _a("upperarm_l", 0.4, 0, 0, 1)),
                         (_a("clavicle_l", 0.8, -1, 0, 0.5), _a("upperarm_l", 0.4, -0.3, 0, 1))),
           width=0.8, thickness=0.8, belly=0.35, color=RED),
    # Chest and back
    Muscle("Pectoral_l", _fan((_a("clavicle_l", 0.4, 1, 0, 0.2), _a("chest", 0.4, 1, 0.15), _a("midspine", 0.6, 1, 0.2)),
                              _a("upperarm_l", 0.12, 1, 0.3, depth=0.5)),
           width=0.7, thickness=0.5, tendon=0.3, color=RED),
    Muscle("Latissimus_l", _fan((_a("midspine", 0.0, -1, 0.1), _a("spine_01", 0.5, -1, 0.2)),
                                _a("upperarm_l", 0.12, -1, -0.3, depth=0.5)),
           width=0.7, thickness=0.5, tendon=0.3, color=DARK),
    Muscle("ErectorSpinae_l", ((_a("pelvis", 0.5, -1, 0.25), _a("chest", 0.5, -1, 0.2)),),
           width=0.5, thickness=0.6, belly=0.4, color=DARK),
    # Abdomen
    Muscle("RectusAbdominis_l", ((_a("pelvis", 0.2, 1, 0.25), _a("chest", 0.0, 1, 0.3)),),
           width=0.5, thickness=0.4, tendon=0.5, color=PINK),
    Muscle("Oblique_l", ((_a("pelvis", 0.8, 0.3, 1), _a("midspine", 0.5, 0.6, 1)),),
           width=0.6, thickness=0.5, tendon=0.4, color=PINK),
    # Arms
    Muscle("Biceps_l", ((_a("upperarm_l", 0.15, 1), _a("lowerarm_l", 0.1, 1)),), width=0.8, thickness=0.8),
    Muscle("Triceps_l", ((_a("upperarm_l", 0.1, -1), _a("lowerarm_l", 0.0, -1)),), width=0.8, thickness=0.8, color=DARK),
    Muscle("ForearmFlexors_l", ((_a("lowerarm_l", 0.05, 0.3, 0, -1), _a("hand_l", 0.0, 0, 0, -1)),),
           width=0.8, thickness=0.7, belly=0.3, color=PINK),
    Muscle("ForearmExtensors_l", ((_a("lowerarm_l", 0.05, 0.3, 0, 1), _a("hand_l", 0.0, 0, 0, 1)),),
           width=0.8, thickness=0.7, belly=0.3, color=PINK),
    # Hips and legs
    Muscle("Gluteus_l", ((_a("pelvis", 0.5, -1, 0.5), _a("thigh_l", 0.2, -1, 0.5)),), kind=PAD,
           hooks=(_a("spine_01", 0.0, -1, 0.3), _a("thigh_l", 0.05, 0, 1)),
           width=0.85, thickness=0.75, belly=0.5, tendon=0.5),
    Muscle("Quadriceps_l", _fan((_a("thigh_l", 0.05, 1), _a("thigh_l", 0.15, 0.6, 1), _a("thigh_l", 0.2, 0.6, -1)),
                                _a("calf_l", 0.0, 1, depth=0.5)),
           width=0.7, thickness=0.7, belly=0.55),
    Muscle("Hamstrings_l", _fan((_a("pelvis", 0.0, -1, 0.5), _a("pelvis", 0.0, -1, 0.3)),
                                _a("calf_l", 0.05, -1, depth=0.5)),
           width=0.7, thickness=0.7, color=DARK),
    Muscle("Adductors_l", ((_a("pelvis", 0.0, 0.5, 0.3, -1), _a("thigh_l", 0.6, 0, -1)),), width=0.7, thickness=0.6, color=PINK),
    Muscle("Gastrocnemius_l", ((_a("calf_l", 0.05, -1, 0.4), _a("foot_l", 0.0, -1)),
                               (_a("calf_l", 0.05, -1, -0.4), _a("foot_l", 0.0, -1))),
           width=0.75, thickness=0.75, belly=0.3),
    Muscle("Tibialis_l", ((_a("calf_l", 0.1, 1, 0.3), _a("foot_l", 0.0, 1)),), width=0.5, thickness=0.5, color=PINK),
)


@dataclass(frozen=True)
class Joint:
    """A hinge the skin folds around. The half bone sits at the joint and turns halfway with the child bone, holding
    volume in the fold; the cap bone rides on the child bone at the bony point outside the bend (kneecap, elbow
    tip), keeping it hard as the skin stretches over it."""
    name: str
    parent: str
    child: str
    cap: tuple  # direction of the bony point, front/left/up of the body
    half: float = 0.5
    band: float = 0.18  # half bone weight reach either side of the joint, relative to the mean bone length
    cap_size: float = 0.12  # cap weight radius, relative to the child bone length


JOINTS = (
    Joint("Elbow_l", "upperarm_l", "lowerarm_l", (-1.0, 0.0, 0.0)),
    Joint("Knee_l", "thigh_l", "calf_l", (1.0, 0.0, 0.0)),
)


def resolve_bone(name, bone_names):
    """The skeleton's bone for a template bone name, or None."""
    for candidate in ALIASES.get(name, (name,)):
        if candidate in bone_names:
            return candidate
    return None


def missing_bones(bone_names):
    """The Mannequin bones the skeleton lacks; empty when the template fits."""
    bone_names = set(bone_names)
    missing = [name for name in REQUIRED if name not in bone_names]
    missing += [name for name in ALIASES if resolve_bone(name, bone_names) is None and name not in missing]
    return missing


def strand_name(muscle, index):
    """`Pectoral2_l` for a fan's second strand, `Biceps_l` for a single strand."""
    if len(muscle.strands) == 1:
        return muscle.name
    base, _, side = muscle.name.rpartition("_")
    return f"{base}{index + 1}_{side}"
