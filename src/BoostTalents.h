#ifndef BOOST_TALENTS_H_
#define BOOST_TALENTS_H_

#include "Player.h"

#include <utility>
#include <vector>

class BoostTalents
{
public:
  // Reset the player's talents and apply the preset build for this spec.
  static void ApplyTalents(Player *player, uint8 specialization);

private:
  // [{talentId, rank}, ...] in tier order; empty if no build for class/spec.
  static std::vector<std::pair<uint32, uint32>> GetBuild(uint32 classId, uint8 spec);
  // Baseline weapon abilities a build assumes but the class is not taught (e.g. Dual Wield).
  static std::vector<uint32> GetExtraSpells(uint32 classId, uint8 spec);
};

#endif
