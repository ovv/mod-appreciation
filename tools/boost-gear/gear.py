#!/usr/bin/env python3
"""Verify enchant/gem SpellItemEnchantment ids against the client DBC and hold the
spec->archetype mapping for generating src/BoostGear.cpp.
"""
import struct, os, sys
sys.path.insert(0, os.path.expanduser("~/projects/github.com/ovv/mod-appreciation/tools/boost-talents"))
import talents  # DBC dir

DBC = talents.DBC

CLASS_CONST = {
    "WARRIOR": "CLASS_WARRIOR", "PALADIN": "CLASS_PALADIN", "HUNTER": "CLASS_HUNTER",
    "ROGUE": "CLASS_ROGUE", "PRIEST": "CLASS_PRIEST", "DK": "CLASS_DEATH_KNIGHT",
    "SHAMAN": "CLASS_SHAMAN", "MAGE": "CLASS_MAGE", "WARLOCK": "CLASS_WARLOCK",
    "DRUID": "CLASS_DRUID",
}

# Each (class, spec index) -> stat archetype.
ARCHETYPE = {
    ("WARRIOR", 0): "STR_MELEE", ("WARRIOR", 1): "STR_MELEE", ("WARRIOR", 2): "TANK",
    ("PALADIN", 0): "HEALER", ("PALADIN", 1): "TANK", ("PALADIN", 2): "STR_MELEE",
    ("HUNTER", 0): "AGI_RANGED", ("HUNTER", 1): "AGI_RANGED", ("HUNTER", 2): "AGI_RANGED",
    ("ROGUE", 0): "AGI_MELEE", ("ROGUE", 1): "AGI_MELEE", ("ROGUE", 2): "AGI_MELEE",
    ("PRIEST", 0): "HEALER", ("PRIEST", 1): "HEALER", ("PRIEST", 2): "SP_CASTER",
    ("DK", 0): "STR_MELEE", ("DK", 1): "STR_MELEE", ("DK", 2): "STR_MELEE",
    ("SHAMAN", 0): "SP_CASTER", ("SHAMAN", 1): "AGI_MELEE", ("SHAMAN", 2): "HEALER",
    ("MAGE", 0): "SP_CASTER", ("MAGE", 1): "SP_CASTER", ("MAGE", 2): "SP_CASTER",
    ("WARLOCK", 0): "SP_CASTER", ("WARLOCK", 1): "SP_CASTER", ("WARLOCK", 2): "SP_CASTER",
    ("DRUID", 0): "SP_CASTER", ("DRUID", 1): "AGI_MELEE", ("DRUID", 2): "HEALER",
}

# Enchantable slots -> EQUIPMENT_SLOT index.
ENCHANT_SLOTS = {
    "head": 0, "shoulder": 2, "chest": 4, "wrist": 8, "hands": 9, "legs": 6,
    "feet": 7, "back": 14, "mainhand": 15, "offhand": 16, "ranged": 17,
}
SOCKET_COLORS = {"meta": 1, "red": 2, "yellow": 4, "blue": 8}


def enchant_ids():
    d = open(os.path.join(DBC, "SpellItemEnchantment.dbc"), "rb").read()
    _, n, nf, rs, ss = struct.unpack_from("<4sIIII", d, 0)
    return {struct.unpack_from("<I", d, 20 + i * rs)[0] for i in range(n)}


ENCHANTS = enchant_ids()


def verify(value):
    return value == 0 or value in ENCHANTS
