#!/usr/bin/env python3
"""Parse 3.3.5a Item.dbc / ItemSet.dbc to verify item IDs and look up sets."""
import struct, sys, os

DBC = os.environ.get(
    "WOW_DBC_DIR",
    os.path.expanduser("~/projects/git.ovv.dev/ovv/wow-client-data/data/dbc"),
)

INV_TYPE = {
    0: "non-equip", 1: "head", 2: "neck", 3: "shoulder", 4: "shirt", 5: "chest",
    6: "waist", 7: "legs", 8: "feet", 9: "wrist", 10: "hands", 11: "finger",
    12: "trinket", 13: "1h", 14: "shield", 15: "ranged(bow)", 16: "back",
    17: "2h", 18: "bag", 19: "tabard", 20: "robe(chest)", 21: "mainhand",
    22: "offhand", 23: "holdable", 24: "ammo", 25: "thrown", 26: "ranged-r(gun/wand)",
    28: "relic",
}
ARMOR_SUB = {0: "misc", 1: "cloth", 2: "leather", 3: "mail", 4: "plate",
             5: "buckler", 6: "shield", 7: "libram", 8: "idol", 9: "totem",
             10: "sigil"}
WEAPON_SUB = {0: "axe1h", 1: "axe2h", 2: "bow", 3: "gun", 4: "mace1h", 5: "mace2h",
              6: "polearm", 7: "sword1h", 8: "sword2h", 10: "staff", 13: "fist",
              15: "dagger", 16: "thrown", 18: "crossbow", 19: "wand", 20: "fishing"}


def load_items():
    with open(os.path.join(DBC, "Item.dbc"), "rb") as f:
        data = f.read()
    magic, n, nf, rs, ss = struct.unpack_from("<4sIIII", data, 0)
    assert magic == b"WDBC" and rs == 32 and nf == 8, (magic, nf, rs)
    items = {}
    off = 20
    for i in range(n):
        rec = struct.unpack_from("<8I", data, off + i * rs)
        # 0 id, 1 classId, 2 subclassId, 3 sound_override, 4 material, 5 display, 6 invtype, 7 sheathe
        items[rec[0]] = (rec[1], rec[2], rec[6])
    return items


def typestr(classId, sub, inv):
    if classId == 4:
        a = ARMOR_SUB.get(sub, f"armorsub{sub}")
    elif classId == 2:
        a = WEAPON_SUB.get(sub, f"wpnsub{sub}")
    else:
        a = f"class{classId}.{sub}"
    return f"{a}/{INV_TYPE.get(inv, 'inv'+str(inv))}"


def load_sets():
    with open(os.path.join(DBC, "ItemSet.dbc"), "rb") as f:
        data = f.read()
    magic, n, nf, rs, ss = struct.unpack_from("<4sIIII", data, 0)
    assert magic == b"WDBC", magic
    str_off = 20 + n * rs
    strblock = data[str_off:str_off + ss]

    def getstr(o):
        e = strblock.find(b"\x00", o)
        return strblock[o:e].decode("utf-8", "replace")
    sets = []
    for i in range(n):
        base = 20 + i * rs
        rec = struct.unpack_from(f"<{nf}I", data, base)
        sid = rec[0]
        name = getstr(rec[1])  # enUS
        item_ids = [x for x in rec[18:35] if x]  # ItemID[17]
        sets.append((sid, name, item_ids))
    return sets


if __name__ == "__main__":
    cmd = sys.argv[1]
    if cmd == "ids":
        items = load_items()
        for a in sys.argv[2:]:
            i = int(a)
            if i in items:
                c, s, inv = items[i]
                print(f"{i}\tOK\t{typestr(c, s, inv)}")
            else:
                print(f"{i}\tMISSING")
    elif cmd == "set":
        q = " ".join(sys.argv[2:]).lower()
        for sid, name, ids in load_sets():
            if q in name.lower():
                print(f"[{sid}] {name}: {ids}")
