#include "BoostSkills.h"

void BoostSkills::UpdateSkills(Player *player)
{
  switch (player->getClass())
  {
  case CLASS_PALADIN:
    player->learnSpell(196); // Axes
    player->learnSpell(750); // Plate Mail
    player->learnSpell(200); // PoleArms
    player->learnSpell(197); // 2H Axe
    player->learnSpell(199); // 2H Mace
    break;

  case CLASS_SHAMAN:
    player->learnSpell(196);   // Axes
    player->learnSpell(1180);  // Daggers
    player->learnSpell(15590); // Fists
    player->learnSpell(8737);  // Mail
    player->learnSpell(197);   // 2H Axe
    player->learnSpell(199);   // 2H Mace
    break;

  case CLASS_WARRIOR:
    player->learnSpell(264);   // Bows
    player->learnSpell(5011);  // Crossbow
    player->learnSpell(15590); // Fists
    player->learnSpell(266);   // Guns
    player->learnSpell(750);   // Plate Mail
    player->learnSpell(200);   // PoleArms
    player->learnSpell(199);   // 2H Mace
    player->learnSpell(227);   // Staves
    break;

  case CLASS_HUNTER:
    player->learnSpell(264);   // Bows
    player->learnSpell(5011);  // Crossbow
    player->learnSpell(15590); // Fists
    player->learnSpell(266);   // Guns
    player->learnSpell(8737);  // Mail
    player->learnSpell(200);   // PoleArms
    player->learnSpell(227);   // Staves
    player->learnSpell(2567);  // Thrown
    player->learnSpell(202);   // 2H Sword
    break;

  case CLASS_ROGUE:
    player->learnSpell(264);   // Bows
    player->learnSpell(5011);  // Crossbow
    player->learnSpell(15590); // Fists
    player->learnSpell(266);   // Guns
    player->learnSpell(198);   // Maces
    player->learnSpell(201);   // Swords
    break;

  case CLASS_DRUID:
    player->learnSpell(1180);  // Daggers
    player->learnSpell(15590); // Fists
    player->learnSpell(199);   // 2H Mace
    break;

  case CLASS_MAGE:
    player->learnSpell(1180); // Daggers
    player->learnSpell(201);  // Swords
    break;

  case CLASS_WARLOCK:
    player->learnSpell(201); // Swords
    break;

  case CLASS_PRIEST:
    player->learnSpell(1180); // Daggers
    break;

  case CLASS_DEATH_KNIGHT:
    player->learnSpell(198); // Maces
    player->learnSpell(199); // 2H Mace
    break;

  default:
    break;
  }

  // Up to max
  player->UpdateSkillsToMaxSkillsForLevel();
}