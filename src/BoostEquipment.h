#ifndef BOOST_EQUIPMENT_H_
#define BOOST_EQUIPMENT_H_

#include "Player.h"

enum
{
  BOOST_SPEC_1 = 0,
  BOOST_SPEC_2 = 1,
  BOOST_SPEC_3 = 2,
};

enum
{
  BOOST_EQUIPMENT_TIER_FRESH = 0,
  BOOST_EQUIPMENT_TIER_PRERAID = 1,
  BOOST_EQUIPMENT_TIER_PVP = 2,
};

class BoostEquipment
{
public:
  static void EquipPlayer(Player *player, uint8 specialization, uint32 targetLevel, uint8 equipmentTier);

private:
  static std::vector<std::vector<int>> GetItemList(uint32 classId, uint8 specialization, uint32 TargetLevel, uint8 equipmentTier);
  static std::vector<std::vector<int>> GetLevel80ItemList(uint32 classId, uint8 specialization, uint8 equipmentTier);
};

#endif
