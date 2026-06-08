#ifndef BOOST_GLYPHS_H_
#define BOOST_GLYPHS_H_

#include "Player.h"

#include <vector>

class BoostGlyphs
{
public:
  // Inscribe the preset glyphs for this spec into the matching glyph slots.
  static void ApplyGlyphs(Player *player, uint8 specialization);

private:
  // GlyphProperties ids (3 major + 3 minor) for the class/spec; empty if none.
  static std::vector<uint32> GetGlyphs(uint32 classId, uint8 spec);
};

#endif
