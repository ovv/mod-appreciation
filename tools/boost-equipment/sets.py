#!/usr/bin/env python3
"""Structured level-80 boost equipment data + validator + C++ generator.

Each set is a dict slot->(itemId, randomProp). Missing slots default to (0,0).
Slots are validated against Item.dbc (existence + inventory type) and the
expected armor class per spec.
"""
import sys
import dbc  # /tmp/dbc.py

ITEMS = dbc.load_items()  # id -> (classId, subclassId, invType)

# Equipment slot order (index 0..17), matching EQUIPMENT_SLOT_* in AzerothCore.
SLOTS = ["head", "neck", "shoulder", "shirt", "chest", "waist", "legs", "feet",
         "wrist", "hands", "finger1", "finger2", "trinket1", "trinket2", "back",
         "mainhand", "offhand", "ranged"]

# Acceptable inventory types per slot (from Item.dbc invType field).
SLOT_INV = {
    "head": {1}, "neck": {2}, "shoulder": {3}, "shirt": {4},
    "chest": {5, 20}, "waist": {6}, "legs": {7}, "feet": {8}, "wrist": {9},
    "hands": {10}, "finger1": {11}, "finger2": {11}, "trinket1": {12},
    "trinket2": {12}, "back": {16},
    "mainhand": {13, 17, 21},          # 1h, 2h, mainhand-only
    "offhand": {13, 14, 17, 22, 23},   # 1h(dual), shield, 2h(TG), offhand, holdable
    "ranged": {15, 25, 26, 28},        # bow, thrown, gun/wand/crossbow, relic
}

# expected armor subclass per class (1 cloth, 2 leather, 3 mail, 4 plate)
CLASS_ARMOR = {
    "WARRIOR": 4, "PALADIN": 4, "DK": 4,
    "HUNTER": 3, "SHAMAN": 3,
    "ROGUE": 2, "DRUID": 2,
    "PRIEST": 1, "MAGE": 1, "WARLOCK": 1,
}
ARMOR_SLOTS = {"head", "shoulder", "chest", "waist", "legs", "feet", "wrist", "hands"}

# Allowed (classId, subclass) for the ranged slot per class.
# weapon class=2: bow2 gun3 crossbow18 thrown16 wand19; armor class=4: libram7 idol8 totem9 sigil10
RANGED_OK = {
    "WARRIOR": {(2, 2), (2, 3), (2, 18), (2, 16)},
    "ROGUE": {(2, 2), (2, 3), (2, 18), (2, 16)},
    "HUNTER": {(2, 2), (2, 3), (2, 18)},
    "PRIEST": {(2, 19)}, "MAGE": {(2, 19)}, "WARLOCK": {(2, 19)},
    "PALADIN": {(4, 7)}, "DRUID": {(4, 8)}, "SHAMAN": {(4, 9)}, "DK": {(4, 10)},
}

# class constants used by AzerothCore
CLASS_CONST = {
    "WARRIOR": "CLASS_WARRIOR", "PALADIN": "CLASS_PALADIN", "HUNTER": "CLASS_HUNTER",
    "ROGUE": "CLASS_ROGUE", "PRIEST": "CLASS_PRIEST", "DK": "CLASS_DEATH_KNIGHT",
    "SHAMAN": "CLASS_SHAMAN", "MAGE": "CLASS_MAGE", "WARLOCK": "CLASS_WARLOCK",
    "DRUID": "CLASS_DRUID",
}

TIERS = ["fresh", "preraid", "pvp"]

# DATA[class][tier][spec 0..2] = {slot: (id, rand)}
DATA = {}


def from_json(cls, j):
    """Load a class from the agent JSON shape: {tier: {"0|1|2": {slot: [id, name]}}}."""
    out = {}
    for tier in TIERS:
        specs = []
        for si in ("0", "1", "2"):
            slots = {}
            for slot, val in j[tier][si].items():
                iid = val[0] if isinstance(val, (list, tuple)) else val
                if iid:
                    slots[slot] = (int(iid), 0)
            specs.append(slots)
        out[tier] = specs
    DATA[cls] = out


def s(**kw):
    """Build a slot dict, ignoring slots left unset (default (0,0))."""
    out = {}
    for k, v in kw.items():
        out[k] = v if isinstance(v, tuple) else (v, 0)
    return out


# ---- data is filled by per-class modules importing this and assigning DATA ----

def validate():
    errors = []
    for cls, tiers in DATA.items():
        for tier, specs in tiers.items():
            for spec_i, slots in enumerate(specs):
                if slots is None:
                    continue
                for slot, (iid, rnd) in slots.items():
                    if iid == 0:
                        continue
                    if iid not in ITEMS:
                        errors.append(f"{cls}/{tier}/spec{spec_i}/{slot}: id {iid} MISSING from Item.dbc")
                        continue
                    c, sub, inv = ITEMS[iid]
                    if inv not in SLOT_INV[slot]:
                        errors.append(f"{cls}/{tier}/spec{spec_i}/{slot}: id {iid} invType {inv} ({dbc.typestr(c,sub,inv)}) not valid for slot {slot}")
                    elif slot in ARMOR_SLOTS and c == 4 and sub != CLASS_ARMOR[cls]:
                        errors.append(f"{cls}/{tier}/spec{spec_i}/{slot}: id {iid} armor {dbc.ARMOR_SUB.get(sub,sub)} != expected for {cls}")
                    elif slot == "ranged" and (c, sub) not in RANGED_OK[cls]:
                        errors.append(f"{cls}/{tier}/spec{spec_i}/ranged: id {iid} ({dbc.typestr(c,sub,inv)}) wrong ranged type for {cls}")
    return errors


def cpp_set(slots):
    pairs = []
    for slot in SLOTS:
        iid, rnd = slots.get(slot, (0, 0)) if slots else (0, 0)
        pairs.append(f"{{{iid}, {rnd}}}")
    return "{" + ", ".join(pairs) + "}"


SPEC_NAMES = {
    "WARRIOR": ["Arms", "Fury", "Protection"],
    "PALADIN": ["Holy", "Protection", "Retribution"],
    "HUNTER": ["Beast Mastery", "Marksmanship", "Survival"],
    "ROGUE": ["Assassination", "Combat", "Subtlety"],
    "PRIEST": ["Discipline", "Holy", "Shadow"],
    "DK": ["Blood", "Frost", "Unholy"],
    "SHAMAN": ["Elemental", "Enhancement", "Restoration"],
    "MAGE": ["Arcane", "Fire", "Frost"],
    "WARLOCK": ["Affliction", "Demonology", "Destruction"],
    "DRUID": ["Balance", "Feral", "Restoration"],
}


def generate():
    out = []
    out.append("// Generated by tools/boost-equipment/gen.py - do not edit by hand.")
    out.append("// Level-80 boost equipment: curated tier sets verified against 3.3.5a Item.dbc.")
    out.append('#include "BoostEquipment.h"')
    out.append("")
    out.append("namespace")
    out.append("{")
    out.append("  // 18 {itemId, randomProperty} slots in EQUIPMENT_SLOT order:")
    out.append("  // head, neck, shoulder, shirt, chest, waist, legs, feet, wrist, hands,")
    out.append("  // finger1, finger2, trinket1, trinket2, back, mainhand, offhand, ranged.")
    out.append("  struct BoostSet")
    out.append("  {")
    out.append("    int item[18][2];")
    out.append("  };")
    out.append("")
    out.append("  // [equipmentTier][specialization]; tier 0 fresh, 1 pre-raid, 2 PVP.")
    for cls in CLASS_CONST:
        if cls not in DATA:
            continue
        arr = f"{cls.lower()}Sets"
        out.append(f"  const BoostSet {arr}[3][3] =")
        out.append("  {")
        for tier in TIERS:
            specs = DATA[cls].get(tier, [None, None, None])
            out.append(f"    {{ // {tier}")
            for spec_i in range(3):
                slots = specs[spec_i] if specs and spec_i < len(specs) else None
                nm = SPEC_NAMES[cls][spec_i]
                out.append(f"      {{{cpp_set(slots)}}}, // {nm}")
            out.append("    },")
        out.append("  };")
        out.append("")
    out.append("}")
    out.append("")
    out.append("std::vector<std::vector<int>> BoostEquipment::GetLevel80ItemList(uint32 classId, uint8 specialization, uint8 equipmentTier)")
    out.append("{")
    out.append("  if (specialization > 2)")
    out.append("  {")
    out.append("    specialization = 0;")
    out.append("  }")
    out.append("  if (equipmentTier > 2)")
    out.append("  {")
    out.append("    equipmentTier = 0;")
    out.append("  }")
    out.append("")
    out.append("  const BoostSet *set = nullptr;")
    out.append("  switch (classId)")
    out.append("  {")
    for cls in CLASS_CONST:
        if cls not in DATA:
            continue
        out.append(f"  case {CLASS_CONST[cls]}:")
        out.append(f"    set = &{cls.lower()}Sets[equipmentTier][specialization];")
        out.append("    break;")
    out.append("  default:")
    out.append("    break;")
    out.append("  }")
    out.append("")
    out.append("  std::vector<std::vector<int>> itemList;")
    out.append("  if (!set)")
    out.append("  {")
    out.append("    return itemList;")
    out.append("  }")
    out.append("")
    out.append("  for (int i = 0; i < 18; ++i)")
    out.append("  {")
    out.append("    itemList.push_back({set->item[i][0], set->item[i][1]});")
    out.append("  }")
    out.append("  return itemList;")
    out.append("}")
    out.append("")
    return "\n".join(out)


if __name__ == "__main__":
    import importlib
    # import all class data modules listed on argv (filenames without .py)
    for mod in sys.argv[2:]:
        importlib.import_module(mod)
    cmd = sys.argv[1] if len(sys.argv) > 1 else "validate"
    if cmd == "validate":
        errs = validate()
        if errs:
            print("\n".join(errs))
            print(f"\n{len(errs)} ERRORS")
            sys.exit(1)
        print("all sets valid")
    elif cmd == "generate":
        print(generate())
