# Talent build tooling

Generates `src/BoostTalents.cpp` (a preset talent build per class+spec, plus the
`ApplyTalents` runtime) from editable per-class build files, validated against the 3.3.5a
client `Talent.dbc`.

## Files

- `json/<class>.json` — `{"0": [["Talent Name", rank], ...], "1": ..., "2": ...}` for the
  three specs. Talent names are matched (case-insensitive) within the class's trees.
- `talents.py` — parses `Talent.dbc` / `TalentTab.dbc` (and `Spell.dbc` for names), exposes
  the per-class trees, name resolution, and a build validator.
- `gen.py` — loads `json/*.json`, resolves names → talent ids, validates, generates the C++.

## Usage

`Talent.dbc` location defaults to `~/projects/git.ovv.dev/ovv/wow-client-data/data/dbc`;
override with `WOW_DBC_DIR`.

```sh
# Inspect a class's trees (talentId, row/col, max rank, name):
python3 talents.py dump WARRIOR

# Validate every build (names resolve, ranks <= max, prereqs, tier minimums, <= 71 points):
python3 gen.py validate

# Regenerate the C++ after editing json/:
python3 gen.py generate > ../../src/BoostTalents.cpp
```

Builds are applied at boost time via `player->LearnTalent` in tier order, so the validator
enforces the same tier/prerequisite rules the core does. Specs that dual-wield but get no
Dual Wield talent (Warrior Fury, DK Frost, Enhancement Shaman) also learn spell 674 — see
`EXTRA_SPELLS` in `gen.py`.
