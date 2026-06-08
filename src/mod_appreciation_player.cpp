#include "mod_appreciation.h"

#include "Player.h"
#include "BoostSpells.h"

class AppreciationPlayer : public PlayerScript
{
public:
  AppreciationPlayer() : PlayerScript("AppreciationPlayer") {}

  void OnPlayerLearnTalents(Player *player, uint32 /*talentId*/, uint32 /*talentRank*/, uint32 /*spellid*/)
  {
    BoostSpells::LearnTalentRanks(player);
  }
};

void AddSC_appreciation_player()
{
  new AppreciationPlayer();
}