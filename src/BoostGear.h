#ifndef BOOST_GEAR_H_
#define BOOST_GEAR_H_

#include "Player.h"

class BoostGear
{
public:
  // Apply standard enchants to the equipped gear for this spec's archetype.
  static void ApplyEnchants(Player *player, uint8 specialization);
  // Fill the equipped gear's sockets with gems matching the spec's archetype.
  static void ApplyGems(Player *player, uint8 specialization);
};

#endif
