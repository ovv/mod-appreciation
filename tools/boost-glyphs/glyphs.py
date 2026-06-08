#!/usr/bin/env python3
"""Parse GlyphProperties.dbc (+ Spell.dbc names) and resolve glyph names to
GlyphProperties ids for a per-spec build. Major glyphs have TypeFlags 0, minor 1.
"""
import struct, os, sys
sys.path.insert(0, os.path.expanduser("~/projects/github.com/ovv/mod-appreciation/tools/boost-talents"))
import talents  # reuse spell_name() and DBC dir

DBC = talents.DBC

CLASS_CONST = {
    "WARRIOR": "CLASS_WARRIOR", "PALADIN": "CLASS_PALADIN", "HUNTER": "CLASS_HUNTER",
    "ROGUE": "CLASS_ROGUE", "PRIEST": "CLASS_PRIEST", "DK": "CLASS_DEATH_KNIGHT",
    "SHAMAN": "CLASS_SHAMAN", "MAGE": "CLASS_MAGE", "WARLOCK": "CLASS_WARLOCK",
    "DRUID": "CLASS_DRUID",
}
SPEC_NAMES = talents.SPEC_NAMES


def load():
    d = open(os.path.join(DBC, "GlyphProperties.dbc"), "rb").read()
    _, n, nf, rs, ss = struct.unpack_from("<4sIIII", d, 0)
    # name -> {0: majorId, 1: minorId}
    by_name = {}
    for i in range(n):
        gid, spellid, typeflags = struct.unpack_from("<3I", d, 20 + i * rs)[:3]
        if typeflags not in (0, 1):
            continue
        name = talents.spell_name(spellid)
        if not name.lower().startswith("glyph of"):
            continue
        by_name.setdefault(name.lower(), {}).setdefault(typeflags, gid)
    return by_name


GLYPHS = load()


def resolve(name, major):
    key = name.lower()
    want = 0 if major else 1
    if key not in GLYPHS or want not in GLYPHS[key]:
        raise KeyError(f"glyph {name!r} ({'major' if major else 'minor'}) not found")
    return GLYPHS[key][want]


def build_ids(spec_obj):
    """spec_obj: {"major": [names], "minor": [names]} -> ([ids], errors)."""
    ids, errs = [], []
    for name in spec_obj.get("major", []):
        try:
            ids.append(resolve(name, True))
        except KeyError as e:
            errs.append(str(e))
    for name in spec_obj.get("minor", []):
        try:
            ids.append(resolve(name, False))
        except KeyError as e:
            errs.append(str(e))
    nmaj, nmin = len(spec_obj.get("major", [])), len(spec_obj.get("minor", []))
    if nmaj != 3 or nmin != 3:
        errs.append(f"expected 3 major + 3 minor, got {nmaj} major + {nmin} minor")
    return ids, errs


if __name__ == "__main__":
    print(f"{len(GLYPHS)} glyph names loaded")
