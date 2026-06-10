#!/usr/bin/env python3
"""Driver: load json/<class>.json into sets.DATA, then validate or generate."""
import json, os, sys
sys.path.insert(0, os.path.dirname(__file__))
import sets

HERE = os.path.dirname(__file__)
STEMS = {
    "dk": "DK", "druid": "DRUID", "hunter": "HUNTER", "mage": "MAGE", "paladin": "PALADIN",
    "priest": "PRIEST", "rogue": "ROGUE", "shaman": "SHAMAN", "warlock": "WARLOCK", "warrior": "WARRIOR",
}

for stem, cls in STEMS.items():
    with open(os.path.join(HERE, "json", f"{stem}.json"), encoding="utf-8") as f:
        sets.from_json(cls, json.load(f))

cmd = sys.argv[1] if len(sys.argv) > 1 else "validate"
if cmd == "validate":
    errs = sets.validate()
    if errs:
        print("\n".join(errs))
        print(f"\n{len(errs)} ERRORS")
        sys.exit(1)
    n = sum(1 for c in sets.DATA.values() for t in c.values() for sp in t if sp)
    print(f"all {n} sets valid ({len(sets.DATA)} classes)")
elif cmd == "generate":
    print(sets.generate())
