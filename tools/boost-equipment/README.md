# Level-80 boost equipment tooling

Generates `src/BoostEquipmentLevel80.cpp` from curated item sets, validating every item
ID against the 3.3.5a client `Item.dbc` (existence, slot/inventory type, armor class, and
ranged/relic type per class) and, when the world DB dump is present, against
`item_template` for class equippability and tier item-level ordering.

## Files

- `json/<class>.json` — the curated sets: `{tier: {"0|1|2": {slot: [itemId, "name"]}}}`
  for tiers `fresh`, `preraid`, `pvp` and the three specs of each class.
- `dbc.py` — minimal `Item.dbc` / `ItemSet.dbc` parser.
- `itemdb.py` — `item_template.sql` parser (item level + AllowableClass).
- `sets.py` — slot/type rules, validator, and C++ generator.
- `gen.py` — loads all `json/*.json`, then validates or generates.

## Usage

`Item.dbc` location defaults to `~/projects/git.ovv.dev/ovv/wow-client-data/data/dbc`;
override with `WOW_DBC_DIR`. The world DB dump (`item_template.sql`) defaults to the
superproject's `data/sql/base/db_world/item_template.sql`; override with `AC_ITEM_TEMPLATE`.

```sh
# Validate every set (existence + slot + armor/relic type; plus equippability and
# fresh<=preraid item level when item_template is available):
python3 gen.py validate

# Regenerate the C++ after editing json/:
python3 gen.py generate > ../../src/BoostEquipmentLevel80.cpp
```

Checks enforced:

- Each ID exists in 3.3.5a and fits its slot (inventory type), the class armor type, and the
  ranged/relic type for the class.
- Each item is equippable by its class (`AllowableClass`) — catches e.g. a class-restricted
  weapon placed on the wrong class, which the client `Item.dbc` cannot detect.
- A fresh-tier piece never exceeds the item level of its pre-raid counterpart in the same slot.

Equippability and item-level checks are skipped (with a note) if `item_template` is not found.
Validation does not judge stat optimality; adjust individual IDs in `json/` and regenerate.
