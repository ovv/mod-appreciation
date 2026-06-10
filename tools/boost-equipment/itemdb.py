#!/usr/bin/env python3
"""Parse AzerothCore `item_template.sql` for ItemLevel and AllowableClass.

The client `Item.dbc` carries neither item level nor class restrictions, so those
checks read the server-side world DB dump instead. `INSERT ... VALUES` is positional;
the column order comes from the CREATE TABLE: 0 entry, 6 Quality, 12 InventoryType,
13 AllowableClass, 15 ItemLevel.

Path defaults to the superproject's world DB dump; override with `AC_ITEM_TEMPLATE`.
When the file is absent (e.g. the module checked out on its own), `available()` is
False and the caller skips the item-level / equippability checks.
"""
import os

SQL = os.environ.get(
    "AC_ITEM_TEMPLATE",
    os.path.expanduser(
        "~/projects/git.ovv.dev/ovv/azerothcore-custom/data/sql/base/db_world/item_template.sql"
    ),
)

# class name (as used in sets.DATA) -> classId; AllowableClass bit is 1 << (classId-1).
CLASS_ID = {
    "WARRIOR": 1, "PALADIN": 2, "HUNTER": 3, "ROGUE": 4, "PRIEST": 5,
    "DK": 6, "SHAMAN": 7, "MAGE": 8, "WARLOCK": 9, "DRUID": 11,
}


def available(path=None):
    return os.path.exists(path or SQL)


def load(path=None):
    """entry -> {q, inv, ac, ilvl} for every row in the dump."""
    path = path or SQL
    txt = open(path, encoding="utf-8", errors="replace").read()
    items = {}
    i, n = 0, len(txt)
    while True:
        v = txt.find("VALUES", i)
        if v < 0:
            break
        i = v + 6
        while i < n:
            while i < n and txt[i] in " \t\r\n,":
                i += 1
            if i >= n or txt[i] == ";":
                break
            if txt[i] != "(":
                break
            i += 1
            fields, cur, instr, esc, depth = [], [], False, False, 0
            while i < n:
                ch = txt[i]
                if instr:
                    if esc:
                        cur.append(ch); esc = False
                    elif ch == "\\":
                        esc = True
                    elif ch == "'":
                        instr = False
                    else:
                        cur.append(ch)
                    i += 1
                    continue
                if ch == "'":
                    instr = True; i += 1; continue
                if ch == "," and depth == 0:
                    fields.append("".join(cur)); cur = []; i += 1
                    if len(fields) >= 16:  # everything we read lives in the first 16 columns
                        d2, s2, e2 = 0, False, False
                        while i < n:
                            c2 = txt[i]
                            if s2:
                                if e2: e2 = False
                                elif c2 == "\\": e2 = True
                                elif c2 == "'": s2 = False
                            elif c2 == "'": s2 = True
                            elif c2 == "(": d2 += 1
                            elif c2 == ")":
                                if d2 == 0: break
                                d2 -= 1
                            i += 1
                        break
                    continue
                if ch == "(":
                    depth += 1; cur.append(ch); i += 1; continue
                if ch == ")":
                    if depth == 0:
                        fields.append("".join(cur)); cur = []; break
                    depth -= 1; cur.append(ch); i += 1; continue
                cur.append(ch); i += 1
            while i < n and txt[i] != ")":
                i += 1
            if i < n:
                i += 1
            if len(fields) >= 16:
                try:
                    items[int(fields[0])] = dict(
                        q=int(fields[6]), inv=int(fields[12]),
                        ac=int(fields[13]), ilvl=int(fields[15]),
                    )
                except ValueError:
                    pass
    return items


def equippable(allowable_class, cls):
    """True if AllowableClass (a class bitmask, or -1 for all) permits class `cls`."""
    if allowable_class == -1:
        return True
    return bool(allowable_class & (1 << (CLASS_ID[cls] - 1)))
