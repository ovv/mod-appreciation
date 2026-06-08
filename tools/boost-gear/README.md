# Enchant + gem tooling

Generates `src/BoostGear.cpp` (per-archetype enchants and gems + the `ApplyEnchants` /
`ApplyGems` runtimes), verified against `SpellItemEnchantment.dbc`.

## Model
Each (class, spec) maps to one stat archetype (see `ARCHETYPE` in `gear.py`):
STR_MELEE, TANK, AGI_MELEE, AGI_RANGED, SP_CASTER, HEALER.

`archetypes.json` holds, per archetype:
- `enchants`: slot -> SpellItemEnchantment id (slots: head, shoulder, chest, wrist, hands,
  legs, feet, back, mainhand, offhand, ranged; 0/omitted = none).
- `gems`: socket colour (meta/red/yellow/blue) -> the SpellItemEnchantment id the gem applies.

## Usage
`WOW_DBC_DIR` overrides the client DBC path.
```sh
python3 gen.py validate    # every enchant/gem id must exist in SpellItemEnchantment.dbc
python3 gen.py generate > ../../src/BoostGear.cpp
```
At boost time (after equipment): `ApplyEnchants` sets the per-slot enchant on each equipped
item; `ApplyGems` reads each item's `ItemTemplate::Socket[]` colours and fills them.
