# Glyph tooling

Generates `src/BoostGlyphs.cpp` (preset glyphs per class+spec + the `ApplyGlyphs`
runtime) from editable build files, resolved against `GlyphProperties.dbc`/`Spell.dbc`.

## Files
- `json/<class>.json` — `{"0": {"major": [3 names], "minor": [3 names]}, "1": ..., "2": ...}`.
  Glyph names are exact in-game names ("Glyph of ...").
- `glyphs.py` — resolves glyph names to GlyphProperties ids (major TypeFlags 0 / minor 1).
- `gen.py` — validates (names resolve, 3 major + 3 minor per spec) and generates the C++.

## Usage
`WOW_DBC_DIR` overrides the client DBC path.
```sh
python3 gen.py validate
python3 gen.py generate > ../../src/BoostGlyphs.cpp
```
At boost time `ApplyGlyphs` places each glyph into the first free slot whose
`GlyphSlot.TypeFlags` matches the glyph's `GlyphProperties.TypeFlags`.
