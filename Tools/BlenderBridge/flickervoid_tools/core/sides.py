"""Left/right naming, matching the game's mirror name rule (bone suffixes and whole name tokens)."""

LEFT = "LEFT"
RIGHT = "RIGHT"
CENTRE = "CENTRE"


def _match_case(replacement, matched):
    upper = matched.upper() == matched and matched.lower() != matched
    return replacement.upper() if upper else replacement


class SideNames:
    def __init__(self, pairs):
        self.pairs = [(left, right) for left, right in pairs if left and right]

    def _options(self):
        for left, right in self.pairs:
            yield LEFT, left, right
            yield RIGHT, right, left

    def _match_suffix(self, name):
        lowered = name.lower()
        for side, suffix, other in self._options():
            if len(name) > len(suffix) and lowered.endswith(suffix.lower()):
                return side, len(suffix), _match_case(other, name[-len(suffix):])
        return None

    def _match_token(self, name, index):
        for side, token, other in self._options():
            end = index + len(token)
            starts = token.startswith("_") or index == 0 or name[index - 1] == "_"
            ends = end == len(name) or (end < len(name) and name[end] == "_")
            if end <= len(name) and starts and ends and name[index:end].lower() == token.lower():
                return side, len(token), _match_case(other, name[index:end])
        return None

    def bone_side(self, bone):
        match = self._match_suffix(bone)
        return match[0] if match else CENTRE

    def mirror_bone(self, bone):
        """The counterpart bone name, or None for a centre bone."""
        match = self._match_suffix(bone)
        return bone[:-match[1]] + match[2] if match else None

    def name_side(self, name):
        left = right = False
        index = 0
        while index < len(name):
            match = self._match_token(name, index)
            if match:
                left, right = (True, right) if match[0] == LEFT else (left, True)
                index += match[1]
            else:
                index += 1
        if left == right:
            return CENTRE
        return LEFT if left else RIGHT

    def mirror_name(self, name):
        """The name with every side token swapped, or None when it has none."""
        result = []
        swapped = False
        index = 0
        while index < len(name):
            match = self._match_token(name, index)
            if match:
                result.append(match[2])
                index += match[1]
                swapped = True
            else:
                result.append(name[index])
                index += 1
        return "".join(result) if swapped else None
