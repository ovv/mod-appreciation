#ifndef BOOST_H_
#define BOOST_H_

#include "Player.h"

class Boost
{
public:
  static void BoostPlayer(Player *player, uint8 action);

private:
  static void GiveLevel(Player *player, uint32 targetLevel);
  static void GiveGold(Player *player);
  static void GiveEquipment(Player *player, uint8 specialization, uint32 targetLevel, uint8 equipmentTier);
  static void GiveAmmo(Player *player);
  static void GiveEnchants(Player *player, uint8 specialization);
  static void GiveGems(Player *player, uint8 specialization);
  static void GiveBags(Player *player, uint32 targetLevel);
  static void GiveConsumables(Player *player, uint8 specialization);
  static void ResetTalents(Player *player);
  static void GiveTalents(Player *player, uint8 specialization);
  static void GiveGlyphs(Player *player, uint8 specialization);
  static void GiveProficiencies(Player *player);
  static void GiveSpells(Player *player, uint32 targetLevel);
  static void GiveDualSpec(Player *player);
  static void GiveRiding(Player *player, uint32 targetLevel);
  static void GiveMounts(Player *player, uint32 targetLevel);
  static void UnlockFlightPaths(Player *player, uint32 targetLevel);
  static void GiveReputation(Player *player, uint32 targetLevel);
  static void BindHeartstone(Player *player, uint32 targetLevel);
  static void Teleport(Player *player, uint32 targetLevel);
};

enum {
  FACTION_KIRIN_TOR = 1090
};

#endif