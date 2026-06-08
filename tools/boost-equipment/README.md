# Level-80 boost equipment tooling

Generates `src/BoostEquipmentLevel80.cpp` from curated item sets, validating every item
ID against the 3.3.5a client `Item.dbc` (existence, slot/inventory type, armor class, and
ranged/relic type per class).

## Files

- `json/<class>.json` — the curated sets: `{tier: {"0|1|2": {slot: [itemId, "name"]}}}`
  for tiers `fresh`, `preraid`, `pvp` and the three specs of each class.
- `dbc.py` — minimal `Item.dbc` / `ItemSet.dbc` parser.
- `sets.py` — slot/type rules, validator, and C++ generator.
- `gen.py` — loads all `json/*.json`, then validates or generates.

## Usage

`Item.dbc` location defaults to `~/projects/git.ovv.dev/ovv/wow-client-data/data/dbc`;
override with `WOW_DBC_DIR`.

```sh
# Validate every set (existence + correct slot + armor/relic type):
python3 gen.py validate

# Regenerate the C++ after editing json/:
python3 gen.py generate > ../../src/BoostEquipmentLevel80.cpp
```

Validation guarantees each ID exists in 3.3.5a and fits its slot; it does not judge stat
optimality. Adjust individual IDs in `json/` and regenerate.
