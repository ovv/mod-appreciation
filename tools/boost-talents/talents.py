#!/usr/bin/env python3
"""Parse 3.3.5a Talent.dbc / TalentTab.dbc and decode Wowhead-style talent
calculator strings into {talentId, rank} builds, verified against the client DBCs.

A Wowhead WotLK talent string is three tree segments separated by '-', each a run
of digits — one digit per talent in (Row, Col) order — giving the rank put into
that talent (trailing zeros omitted). Tree order is tab page 0,1,2.
"""
import struct, os, sys

DBC = os.environ.get(
    "WOW_DBC_DIR",
    os.path.expanduser("~/projects/git.ovv.dev/ovv/wow-client-data/data/dbc"),
)

# classId -> class bit in ClassMask
CLASSES = {
    "WARRIOR": 1, "PALADIN": 2, "HUNTER": 3, "ROGUE": 4, "PRIEST": 5,
    "DK": 6, "SHAMAN": 7, "MAGE": 8, "WARLOCK": 9, "DRUID": 11,
}
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
TALENT_BUDGET = 71  # talent points available at level 80


def _read(name):
    with open(os.path.join(DBC, name), "rb") as f:
        data = f.read()
    _, n, nf, rs, ss = struct.unpack_from("<4sIIII", data, 0)
    rows = [struct.unpack_from(f"<{nf}I", data, 20 + i * rs) for i in range(n)]
    return rows, nf


_SPELL_NAMES = None


def spell_name(spell_id):
    """Lazy id->name from Spell.dbc (enUS Name_Lang at field 136)."""
    global _SPELL_NAMES
    if _SPELL_NAMES is None:
        with open(os.path.join(DBC, "Spell.dbc"), "rb") as f:
            data = f.read()
        _, n, nf, rs, ss = struct.unpack_from("<4sIIII", data, 0)
        str_off = 20 + n * rs
        block = data[str_off:str_off + ss]
        _SPELL_NAMES = {}
        for i in range(n):
            rec = struct.unpack_from(f"<{nf}I", data, 20 + i * rs)
            o = rec[136]
            e = block.find(b"\x00", o)
            _SPELL_NAMES[rec[0]] = block[o:e].decode("utf-8", "replace")
    return _SPELL_NAMES.get(spell_id, "")


def load():
    """Return {classId: [tree0, tree1, tree2]} where each tree is a list of talents
    in (row,col) order: dicts with id,row,col,maxrank,ranks,dep,deprank."""
    tabs, _ = _read("TalentTab.dbc")
    # TalentTabEntry: 0 id, 1..17 name(loc), 18 spellIcon, 19 raceMask, 20 classMask,
    # 21 petMask, 22 tabPage
    tab_info = {}
    for r in tabs:
        tab_id, class_mask, tab_page = r[0], r[20], r[22]
        if class_mask and (class_mask & (class_mask - 1)) == 0:  # single-class tree
            class_id = class_mask.bit_length()
            tab_info[tab_id] = (class_id, tab_page)

    tals, _ = _read("Talent.dbc")
    # TalentEntry: 0 id, 1 tab, 2 row, 3 col, 4..8 rankSpell[5], 13 dependsOn, 16 dependsOnRank
    by_class = {cid: {0: [], 1: [], 2: []} for cid in CLASSES.values()}
    for r in tals:
        tid, tab, row, col = r[0], r[1], r[2], r[3]
        if tab not in tab_info:
            continue
        class_id, tab_page = tab_info[tab]
        ranks = [r[4 + i] for i in range(5) if r[4 + i]]
        by_class[class_id][tab_page].append({
            "id": tid, "row": row, "col": col, "maxrank": len(ranks),
            "ranks": ranks, "dep": r[13], "deprank": r[16],
            "name": spell_name(ranks[0]) if ranks else "",
        })
    for cid, trees in by_class.items():
        for tp in trees:
            trees[tp].sort(key=lambda t: (t["row"], t["col"]))
    return by_class


TREES = load()


def resolve(class_key, name):
    """Find the talentId for a talent by (case-insensitive) name within a class.
    Returns (talentId, tree). Raises if not found or ambiguous."""
    cid = CLASSES[class_key]
    hits = [(t["id"], tp) for tp in range(3) for t in TREES[cid][tp]
            if t["name"].lower() == name.lower()]
    if not hits:
        raise KeyError(f"{class_key}: no talent named {name!r}")
    if len(hits) > 1:
        raise KeyError(f"{class_key}: talent name {name!r} ambiguous across trees {[h[1] for h in hits]}")
    return hits[0]


def build_from_names(class_key, named_picks):
    """named_picks: list of (name, rank). Returns [(talentId, rank), ...]."""
    return [(resolve(class_key, name)[0], rank) for name, rank in named_picks]


def decode(class_key, strings):
    """Decode 3 Wowhead tree strings into [(talentId, rank), ...] for a class."""
    cid = CLASSES[class_key]
    out = []
    for tp, s in enumerate(strings):
        tree = TREES[cid][tp]
        for i, ch in enumerate(s):
            rank = int(ch)
            if rank == 0:
                continue
            if i >= len(tree):
                raise ValueError(f"{class_key} tree{tp}: digit {i} beyond {len(tree)} talents")
            out.append((tree[i]["id"], rank))
    return out


def validate_build(class_key, picks):
    """picks: list of (talentId, rank). Returns list of error strings."""
    cid = CLASSES[class_key]
    flat = {t["id"]: (tp, t) for tp in range(3) for t in TREES[cid][tp]}
    errs = []
    total = 0
    per_tree_points = {0: 0, 1: 0, 2: 0}
    chosen = {}
    for tid, rank in picks:
        if tid not in flat:
            errs.append(f"{class_key}: talent {tid} not in class trees")
            continue
        tp, t = flat[tid]
        if rank > t["maxrank"]:
            errs.append(f"{class_key}: talent {tid} rank {rank} > max {t['maxrank']}")
        chosen[tid] = (tp, t, rank)
        total += rank
        per_tree_points[tp] += rank
    # tier requirement + prereqs need cumulative points per tree in row order
    for tid, (tp, t, rank) in chosen.items():
        # tier: need row*5 points spent in this tree among lower-or-equal rows... approximate
        spent_below = sum(r for (otp, ot, r) in chosen.values()
                          if otp == tp and ot["row"] < t["row"])
        if spent_below < t["row"] * 5:
            errs.append(f"{class_key}: talent {tid} at row {t['row']} needs {t['row']*5} pts below, has {spent_below}")
        # Some talents carry a stale DependsOn pointing at a talentId that does not exist in
        # the class trees (a DBC artifact); only enforce real prerequisites.
        if t["dep"] and t["dep"] in flat:
            if t["dep"] not in chosen or chosen[t["dep"]][2] <= t["deprank"]:
                errs.append(f"{class_key}: talent {tid} prereq {t['dep']} rank>{t['deprank']} not met")
    if total > TALENT_BUDGET:
        errs.append(f"{class_key}: {total} points > {TALENT_BUDGET}")
    return errs, total, per_tree_points


if __name__ == "__main__":
    cmd = sys.argv[1] if len(sys.argv) > 1 else "trees"
    if cmd == "trees":
        for k, cid in CLASSES.items():
            sizes = [len(TREES[cid][tp]) for tp in range(3)]
            print(f"{k}: trees sizes {sizes}")
    elif cmd == "dump":
        k = sys.argv[2]
        cid = CLASSES[k]
        for tp in range(3):
            print(f"--- {k} tree {tp} ---")
            for i, t in enumerate(TREES[cid][tp]):
                print(f"  r{t['row']}c{t['col']} id={t['id']} max={t['maxrank']}  {t['name']}")
