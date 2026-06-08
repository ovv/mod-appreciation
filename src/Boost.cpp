#include "Boost.h"

#include "Config.h"
#include "ObjectMgr.h"
#include "BoostFlightPaths.h"
#include "BoostSkills.h"
#include "BoostSpells.h"
#include "BoostEquipment.h"
#include "BoostTalents.h"
#include "BoostGlyphs.h"
#include "BoostGear.h"

// Boost options map to a (targetLevel, equipmentTier) pair, indexed by the option encoded
// in the gossip action: 0 = level 60, 1 = level 70, 2 = level 80 fresh,
// 3 = level 80 pre-raid, 4 = level 80 PVP.
int levels[5] = {60, 70, 80, 80, 80};
int equipmentTiers[5] = {0, 0, 0, 1, 2};

void Boost::BoostPlayer(Player *player, uint8 action)
{
  // Decode the gossip action into specialisation and boost option.
  uint8 specialization = action / 10;
  uint8 option = action % 10;
  uint32 targetLevel = levels[option];
  uint8 equipmentTier = equipmentTiers[option];

  // Log
  LOG_INFO("module.appreciation", "Applying boost on character '{}' from account #{}.", player->GetName(), player->GetSession()->GetAccountId());

  // Boost
  Boost::GiveLevel(player, targetLevel);
  Boost::GiveGold(player);
  // Talents before equipment: Titan's Grip / Dual Wield must be active to equip off-hands.
  Boost::GiveTalents(player, specialization);
  Boost::GiveGlyphs(player, specialization);
  Boost::GiveEquipment(player, specialization, targetLevel, equipmentTier);
  Boost::GiveEnchants(player, specialization);
  Boost::GiveGems(player, specialization);
  Boost::GiveAmmo(player);
  Boost::GiveBags(player, targetLevel);
  Boost::GiveConsumables(player, specialization);
  Boost::GiveProficiencies(player);
  Boost::GiveRiding(player, targetLevel);
  Boost::GiveMounts(player, targetLevel);
  Boost::GiveSpells(player, targetLevel);
  Boost::GiveDualSpec(player);
  Boost::UnlockFlightPaths(player, targetLevel);
  Boost::GiveReputation(player, targetLevel);
  Boost::BindHeartstone(player, targetLevel);
  Boost::Teleport(player, targetLevel);
}

void Boost::GiveLevel(Player *player, uint32 targetLevel)
{
  if (player->GetLevel() >= targetLevel)
  {
    return;
  }

  player->GiveLevel(targetLevel);
  player->SetUInt32Value(PLAYER_XP, 0);
}

void Boost::GiveGold(Player *player)
{
  uint32 gold = sConfigMgr->GetOption<uint32>("Boost.Gold", 250);

  if (gold <= 0)
  {
    return;
  }

  player->ModifyMoney(gold * 10000);
}

void Boost::GiveEquipment(Player *player, uint8 specialization, uint32 targetLevel, uint8 equipmentTier)
{
  uint32 giveEquipment = sConfigMgr->GetOption<uint32>("Boost.Equipment", 1);

  if (giveEquipment <= 0)
  {
    return;
  }
  BoostEquipment::EquipPlayer(player, specialization, targetLevel, equipmentTier);
}

void Boost::GiveEnchants(Player *player, uint8 specialization)
{
  uint32 giveEnchants = sConfigMgr->GetOption<uint32>("Boost.Enchants", 1);

  if (giveEnchants <= 0)
  {
    return;
  }
  BoostGear::ApplyEnchants(player, specialization);
}

void Boost::GiveGems(Player *player, uint8 specialization)
{
  uint32 giveGems = sConfigMgr->GetOption<uint32>("Boost.Gems", 1);

  if (giveGems <= 0)
  {
    return;
  }
  BoostGear::ApplyGems(player, specialization);
}

void Boost::GiveAmmo(Player *player)
{
  uint32 giveAmmo = sConfigMgr->GetOption<uint32>("Boost.Ammo", 1);

  if (giveAmmo <= 0)
  {
    return;
  }

  Item *ranged = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
  if (!ranged)
  {
    return;
  }

  // Bows and crossbows fire arrows (held in a quiver); guns fire bullets (held in an ammo
  // pouch). Thrown/relics/wands need no ammo.
  uint32 ammoId = 0;
  uint32 containerId = 0;
  switch (ranged->GetTemplate()->SubClass)
  {
  case ITEM_SUBCLASS_WEAPON_BOW:
  case ITEM_SUBCLASS_WEAPON_CROSSBOW:
    ammoId = 41165;      /* Iceblade Arrow */
    containerId = 18714; /* quiver */
    break;
  case ITEM_SUBCLASS_WEAPON_GUN:
    ammoId = 41164;      /* Shatter Rounds */
    containerId = 8218;  /* ammo pouch */
    break;
  default:
    return;
  }

  // A character may equip only one quiver/ammo pouch (ITEM_CLASS_QUIVER). Reuse an existing
  // one of the right type; otherwise destroy the wrong one (e.g. the starter pouch) and
  // equip the matching container, then store the ammo inside it.
  uint32 neededSubClass = (ammoId == 41165) ? ITEM_SUBCLASS_QUIVER : ITEM_SUBCLASS_AMMO_POUCH;
  ItemTemplate const *ourProto = sObjectMgr->GetItemTemplate(containerId);
  uint32 ourSlots = ourProto ? ourProto->ContainerSlots : 0;
  uint8 ammoBag = INVENTORY_SLOT_BAG_0;
  uint32 bagSlots = 0;
  bool haveContainer = false;
  uint8 freeSlot = NULL_SLOT;

  for (uint8 slot = INVENTORY_SLOT_BAG_START; slot < INVENTORY_SLOT_BAG_END; ++slot)
  {
    Item *bag = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
    if (!bag)
    {
      if (freeSlot == NULL_SLOT)
      {
        freeSlot = slot;
      }
      continue;
    }

    ItemTemplate const *bagProto = bag->GetTemplate();
    if (bagProto->Class == ITEM_CLASS_QUIVER)
    {
      // Keep it only if it is the right type and at least as large as the one we give.
      if (bagProto->SubClass == neededSubClass && bagProto->ContainerSlots >= ourSlots)
      {
        ammoBag = slot;
        bagSlots = bagProto->ContainerSlots;
        haveContainer = true;
        break;
      }
      // Wrong type, or a smaller same-type container: remove it (it also blocks equipping
      // ours, since only one quiver/ammo pouch may be equipped).
      player->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
      if (freeSlot == NULL_SLOT)
      {
        freeSlot = slot;
      }
    }
  }

  if (!haveContainer && freeSlot != NULL_SLOT)
  {
    uint16 eqDest;
    if (player->CanEquipNewItem(freeSlot, eqDest, containerId, false) == EQUIP_ERR_OK)
    {
      if (Item *container = player->EquipNewItem(eqDest, containerId, true))
      {
        player->SendNewItem(container, 1, true, false);
        ammoBag = freeSlot;
        bagSlots = ourSlots;
        haveContainer = true;
      }
    }
  }

  // No bag slot available: hand the player the container as an item so they can equip it.
  if (!haveContainer)
  {
    ItemPosCountVec bagDest;
    if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, bagDest, containerId, 1) == EQUIP_ERR_OK)
    {
      if (Item *container = player->StoreNewItem(bagDest, containerId, true))
      {
        player->SendNewItem(container, 1, true, false);
      }
    }
  }

  // Fill the container to capacity (one max stack per slot); in the backpack fallback just
  // give a single stack so it does not swallow inventory space.
  ItemTemplate const *ammoProto = sObjectMgr->GetItemTemplate(ammoId);
  uint32 stack = ammoProto ? ammoProto->GetMaxStackSize() : 1000;
  uint32 stacks = (haveContainer && bagSlots) ? bagSlots : 1;

  for (uint32 i = 0; i < stacks; ++i)
  {
    ItemPosCountVec dest;
    if (player->CanStoreNewItem(ammoBag, NULL_SLOT, dest, ammoId, stack) != EQUIP_ERR_OK)
    {
      break;
    }
    if (Item *ammo = player->StoreNewItem(dest, ammoId, true))
    {
      player->SendNewItem(ammo, stack, true, false);
    }
  }

  player->SetAmmo(ammoId);
}

void Boost::GiveBags(Player *player, uint32 targetLevel)
{
  uint32 giveBags = sConfigMgr->GetOption<uint32>("Boost.Bags", 1);
  if (giveBags <= 0)
  {
    return;
  }

  uint32 bagId = 41599; /* Frostweave Bag */
  if (targetLevel == 60) {
    bagId = 14046; /* Runecloth Bag */
  } else if (targetLevel == 70) {
    bagId = 21841; /* Netherweave Bag */
  }

  if (!sObjectMgr->GetItemTemplate(bagId))
  {
    return;
  }

  // Equip a bag into each empty bag slot; mail one for any slot already taken (existing bag
  // or the quiver/ammo pouch) so the player always receives four bags.
  uint8 mailCount = 0;
  for (uint8 slot = INVENTORY_SLOT_BAG_START; slot < INVENTORY_SLOT_BAG_END; ++slot)
  {
    if (!player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
    {
      uint16 eqDest;
      if (player->CanEquipNewItem(slot, eqDest, bagId, false) == EQUIP_ERR_OK)
      {
        if (Item *bag = player->EquipNewItem(eqDest, bagId, true))
        {
          player->SendNewItem(bag, 1, true, false);
          continue;
        }
      }
    }
    ++mailCount;
  }

  if (mailCount == 0)
  {
    return;
  }

  uint32 guid = player->GetGUID().GetCounter();
  CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
  MailDraft *mail = new MailDraft("Some bags to help you", "Here are some bags to help you with your character boost.");

  for (uint8 i = 0; i < mailCount; ++i)
  {
    if (Item *mailItem = Item::CreateItem(bagId, 1))
    {
      mailItem->SaveToDB(trans);
      mail->AddItem(mailItem);
    }
  }

  mail->SendMailTo(trans, player ? player : MailReceiver(guid), MailSender(MAIL_NORMAL, 0, MAIL_STATIONERY_GM), MAIL_CHECK_MASK_RETURNED);
  delete mail;
  CharacterDatabase.CommitTransaction(trans);
}

namespace
{
  enum BoostRole { ROLE_STR, ROLE_TANK, ROLE_AGI, ROLE_CASTER, ROLE_HEALER };

  BoostRole RoleOf(uint8 classId, uint8 spec)
  {
    switch (classId)
    {
    case CLASS_WARRIOR: return spec == 2 ? ROLE_TANK : ROLE_STR;
    case CLASS_PALADIN: return spec == 0 ? ROLE_HEALER : (spec == 1 ? ROLE_TANK : ROLE_STR);
    case CLASS_HUNTER: return ROLE_AGI;
    case CLASS_ROGUE: return ROLE_AGI;
    case CLASS_PRIEST: return spec == 2 ? ROLE_CASTER : ROLE_HEALER;
    case CLASS_DEATH_KNIGHT: return ROLE_STR;
    case CLASS_SHAMAN: return spec == 0 ? ROLE_CASTER : (spec == 1 ? ROLE_AGI : ROLE_HEALER);
    case CLASS_MAGE: return ROLE_CASTER;
    case CLASS_WARLOCK: return ROLE_CASTER;
    case CLASS_DRUID: return spec == 0 ? ROLE_CASTER : (spec == 1 ? ROLE_AGI : ROLE_HEALER);
    }
    return ROLE_STR;
  }

  // Place an item stack in the player's bags, or mail it if there is no room.
  void GiveItem(Player *player, uint32 itemId, uint32 count)
  {
    ItemPosCountVec dest;
    if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count) == EQUIP_ERR_OK)
    {
      if (Item *item = player->StoreNewItem(dest, itemId, true))
      {
        player->SendNewItem(item, count, true, false);
      }
      return;
    }

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
    MailDraft *mail = new MailDraft("Some consumables to help you", "Here are some consumables to help you with your character boost.");
    if (Item *item = Item::CreateItem(itemId, count))
    {
      item->SaveToDB(trans);
      mail->AddItem(item);
    }
    mail->SendMailTo(trans, player, MailSender(MAIL_NORMAL, 0, MAIL_STATIONERY_GM), MAIL_CHECK_MASK_RETURNED);
    delete mail;
    CharacterDatabase.CommitTransaction(trans);
  }
}

void Boost::GiveConsumables(Player *player, uint8 specialization)
{
  uint32 giveConsumables = sConfigMgr->GetOption<uint32>("Boost.Consumables", 1);

  if (giveConsumables <= 0)
  {
    return;
  }

  uint8 classId = player->getClass();
  BoostRole role = RoleOf(classId, specialization);
  bool manaUser = classId != CLASS_WARRIOR && classId != CLASS_ROGUE && classId != CLASS_DEATH_KNIGHT;

  // Well-fed food by role.
  uint32 food = (role == ROLE_CASTER || role == ROLE_HEALER) ? 34767 /* Firecracker Salmon (spell power) */
              : role == ROLE_AGI ? 42999                              /* Blackened Dragonfin (agility) */
              : 43000;                                                /* Dragonfin Filet (strength) */
  GiveItem(player, food, 20);

  // Flask by role.
  uint32 flask = role == ROLE_CASTER ? 46376 /* Flask of the Frost Wyrm */
               : role == ROLE_HEALER ? 46378 /* Flask of Pure Mojo */
               : role == ROLE_TANK ? 46379   /* Flask of Stoneblood */
               : 46377;                       /* Flask of Endless Rage */
  GiveItem(player, flask, 5);

  GiveItem(player, 33447, 5);  /* Runic Healing Potion */
  GiveItem(player, 34722, 20); /* Heavy Frostweave Bandage */

  if (manaUser)
  {
    GiveItem(player, 33445, 20); /* Honeymint Tea */
    GiveItem(player, 33448, 5);  /* Runic Mana Potion */
  }

  // Class-specific reagents/consumables.
  switch (classId)
  {
  case CLASS_ROGUE:
    GiveItem(player, 43231, 5); /* Instant Poison IX */
    GiveItem(player, 43233, 5); /* Deadly Poison IX */
    GiveItem(player, 43235, 5); /* Wound Poison VII */
    break;
  case CLASS_MAGE:
    GiveItem(player, 17031, 5); /* Rune of Teleportation */
    GiveItem(player, 17032, 5); /* Rune of Portals */
    break;
  case CLASS_WARLOCK:
    GiveItem(player, 6265, 5); /* Soul Shard */
    break;
  case CLASS_SHAMAN:
    GiveItem(player, 17030, 5); /* Ankh */
    break;
  default:
    break;
  }
}

void Boost::GiveTalents(Player *player, uint8 specialization)
{
  uint32 giveTalents = sConfigMgr->GetOption<uint32>("Boost.Talents", 1);

  if (giveTalents <= 0)
  {
    // Legacy behavior: wipe talents and let the player spend their own.
    Boost::ResetTalents(player);
    return;
  }
  BoostTalents::ApplyTalents(player, specialization);
}

void Boost::GiveGlyphs(Player *player, uint8 specialization)
{
  uint32 giveGlyphs = sConfigMgr->GetOption<uint32>("Boost.Glyphs", 1);

  if (giveGlyphs <= 0)
  {
    return;
  }
  BoostGlyphs::ApplyGlyphs(player, specialization);
}

void Boost::ResetTalents(Player *player)
{
  // Player talents
  player->resetTalents(true);
  player->SendTalentsInfoData(false);

  // Client-side popup
  WorldPacket data(SMSG_TALENTS_INVOLUNTARILY_RESET, 1);
  data << uint8(0);
  player->SendMessageToSet(&data, true);
}

void Boost::GiveProficiencies(Player *player)
{
  uint32 giveProficiencies = sConfigMgr->GetOption<uint32>("Boost.Proficiencies", 1);

  if (giveProficiencies <= 0)
  {
    return;
  }
  BoostSkills::UpdateSkills(player);
}

void Boost::GiveSpells(Player *player, uint32 targetLevel)
{
  uint32 giveSpells = sConfigMgr->GetOption<uint32>("Boost.Spells", 1);

  if (giveSpells <= 0)
  {
    return;
  }
  BoostSpells::LearnSpells(player, targetLevel);
}

void Boost::GiveDualSpec(Player *player)
{
  uint32 giveDualSpec = sConfigMgr->GetOption<uint32>("Boost.DualSpec", 1);

  if (giveDualSpec <= 0)
  {
    return;
  }
  player->CastSpell(player, 63680, true, nullptr, nullptr, player->GetGUID());
  player->CastSpell(player, 63624, true, nullptr, nullptr, player->GetGUID());
  player->UpdateSpecCount(2);
}

void Boost::GiveRiding(Player *player, uint32 targetLevel)
{
  uint32 ridingLevel = sConfigMgr->GetOption<uint32>("Boost.Riding", 3);

  if (ridingLevel <= 0)
  {
    return;
  }
  if (ridingLevel >= 1)
  {
    player->learnSpell(33388); // Apprentice Riding (60% ground)
  }
  if (ridingLevel >= 2)
  {
    player->learnSpell(33391); // Journeyman Riding (100% ground)
  }
  if (ridingLevel >= 3 && targetLevel >= 70)
  {
    player->learnSpell(34090); // Expert Riding (150% fly)
  }
  if (ridingLevel >= 4 && targetLevel >= 80)
  {
    player->learnSpell(34091); // Artisan Riding (280/310% fly)
  }

  uint32 coldFlying = sConfigMgr->GetOption<uint32>("Boost.ColdFlying", 1);
  if (coldFlying >= 1 && targetLevel >= 80)
  {
    player->learnSpell(54197); // Northend flying
  }
}

void Boost::GiveMounts(Player *player, uint32 targetLevel)
{
  uint32 mountHordeGround = sConfigMgr->GetOption<uint32>("Boost.Mounts.Horde.Ground", 23251);
  uint32 mountHordeFly = sConfigMgr->GetOption<uint32>("Boost.Mounts.Horde.Fly", 32243);
  uint32 mountHordeFlyFast = sConfigMgr->GetOption<uint32>("Boost.Mounts.Horde.FlyFast", 61230);
  uint32 mountAllianceGround = sConfigMgr->GetOption<uint32>("Boost.Mounts.Alliance.Ground", 23229);
  uint32 mountAllianceFly = sConfigMgr->GetOption<uint32>("Boost.Mounts.Alliance.Fly", 32240);
  uint32 mountAllianceFlyFast = sConfigMgr->GetOption<uint32>("Boost.Mounts.Alliance.FlyFast", 61229);


  if (player->GetTeamId() == TEAM_HORDE)
  {
    if (mountHordeGround > 0)
    {
      player->learnSpell(mountHordeGround);
    }
    if (mountHordeFly > 0 && targetLevel >= 70)
    {
      player->learnSpell(mountHordeFly);
    }
    if (mountHordeFlyFast > 0 && targetLevel >= 80)
    {
      player->learnSpell(mountHordeFlyFast);
    }
  }
  else
  {
    if (mountAllianceGround > 0)
    {
      player->learnSpell(mountAllianceGround);
    }
    if (mountAllianceFly > 0 && targetLevel >= 70)
    {
      player->learnSpell(mountAllianceFly);
    }
    if (mountAllianceFlyFast > 0 && targetLevel >= 80)
    {
      player->learnSpell(mountAllianceFlyFast);
    }
  }
}

void Boost::UnlockFlightPaths(Player *player, uint32 targetLevel)
{
  uint32 flightEasternKingdoms = sConfigMgr->GetOption<uint32>("Boost.FlightPaths.EasternKingdoms", 1);
  uint32 flightKalimdor = sConfigMgr->GetOption<uint32>("Boost.FlightPaths.Kalimdor", 1);
  uint32 flightOutland = sConfigMgr->GetOption<uint32>("Boost.FlightPaths.Outland", 1);
  uint32 flightNorthend = sConfigMgr->GetOption<uint32>("Boost.FlightPaths.Northend", 1);

  if (flightEasternKingdoms > 0)
  {
    BoostFlightPaths::UnlockFlightPaths(player, BOOST_FLIGHTPATHS_EASTERN_KINGDOMS);
  }
  if (flightKalimdor > 0)
  {
    BoostFlightPaths::UnlockFlightPaths(player, BOOST_FLIGHTPATHS_KALIMDOR);
  }
  if (flightOutland > 0 && targetLevel >= 70)
  {
    BoostFlightPaths::UnlockFlightPaths(player, BOOST_FLIGHTPATHS_OUTLAND);
  }
  if (flightNorthend > 0 && targetLevel >= 80)
  {
    BoostFlightPaths::UnlockFlightPaths(player, BOOST_FLIGHTPATHS_NORTHEND);
  }
}

void Boost::GiveReputation(Player *player, uint32 targetLevel)
{
  if (player->GetReputation(FACTION_KIRIN_TOR) < 3000 && targetLevel >= 80)
  {
    player->SetReputation(FACTION_KIRIN_TOR, 3000);
  }
}

void Boost::BindHeartstone(Player *player, uint32 targetLevel)
{
  uint32 bind = sConfigMgr->GetOption<uint32>("Boost.Heartstone", 1);

  if (bind <= 0)
  {
    return;
  }

  WorldLocation homebinding;
  int areaID;
  if (player->GetTeamId() == TEAM_HORDE)
  {
    if (targetLevel >= 80) {
      /* Dalaran */
      areaID = 4395;
      homebinding = WorldLocation(571, 5887.78, 508.266, 641.57, 4.9057107);
    } else {
      /* Orgrimmar */
      areaID = 1637;
      homebinding = WorldLocation(1, 1632.58, -4439.76, 15.7678, 3.69762);
    }
  }
  else
  {
    if (targetLevel >= 80) {
      /* Dalaran - A Hero's Welcome inn, Silver Enclave (Alliance) */
      areaID = 4395;
      homebinding = WorldLocation(571, 5671.43, 724.478, 653.412, 2.08769);
    } else {
      /* Stormwind */
      areaID = 1519;
      homebinding = WorldLocation(0, -8865.86, 672.69, 97.9034, 1.06535);
    }
  }

  // Bind on server
  player->SetHomebind(homebinding, areaID);

  // Notify client
  WorldPacket data(SMSG_BINDPOINTUPDATE, 4 + 4 + 4 + 4 + 4);
  data << float(homebinding.GetPositionX());
  data << float(homebinding.GetPositionY());
  data << float(homebinding.GetPositionZ());
  data << uint32(homebinding.GetMapId());
  data << uint32(areaID);
  player->SendDirectMessage(&data);

  // ???
  data.Initialize(SMSG_PLAYERBOUND, 8 + 4);
  data << player->GetGUID();
  data << uint32(areaID);
  player->SendDirectMessage(&data);
}

void Boost::Teleport(Player *player, uint32 targetLevel)
{
  uint32 teleport = sConfigMgr->GetOption<uint32>("Boost.Teleport", 1);

  if (teleport <= 0)
  {
    return;
  }

  if (targetLevel >= 80) {
    if (player->GetTeamId() == TEAM_HORDE) {
      /* Dalaran - Sunreaver's Sanctuary inn (Horde) */
      player->TeleportTo(571, 5888.69, 507.898, 641.653, 1.8675);
    } else {
      /* Dalaran - A Hero's Welcome inn, Silver Enclave (Alliance) */
      player->TeleportTo(571, 5671.43, 724.478, 653.412, 2.08769);
    }
  } else if (player->GetTeamId() == TEAM_HORDE) {
    /* Orgrimmar */
    player->TeleportTo(1, 1629.85, -4373.64, 31.5573, 3.69762);
  } else {
    /* Stormwind */
    player->TeleportTo(0, -8833.38, 628.628, 94.0066, 1.06535);
  }
}