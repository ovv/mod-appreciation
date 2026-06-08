#include "mod_appreciation.h"

#include "Config.h"
#include "CreatureScript.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "Boost.h"

class AppreciationCreature : public CreatureScript
{
public:
    AppreciationCreature() : CreatureScript("npc_appreciation") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ClearGossipMenuFor(player);

        // Check if enable
        if (sConfigMgr->GetOption<uint32>("Boost.Enabled", 1) <= 0)
        {
            SendGossipMenuFor(player, GOSSIP_BOOST_TEXT_DISABLE, creature->GetGUID());
            return true;
        }
        
        uint32 playerLevel = player->GetLevel();
        // Check player is below level
        bool boostAvailable = false;
        if (sConfigMgr->GetOption<uint32>("Boost.60.Enabled", 1) && playerLevel < 60) {
            boostAvailable = true;
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want a character boost to level 60", GOSSIP_SENDER_MAIN, GOSSIP_MENU_CHOOSE_SPECIALIZATION_START + 0);
        }
        if (sConfigMgr->GetOption<uint32>("Boost.70.Enabled", 1) && playerLevel < 70) {
            boostAvailable = true;
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want a character boost to level 70", GOSSIP_SENDER_MAIN, GOSSIP_MENU_CHOOSE_SPECIALIZATION_START + 1);
        }
        if (playerLevel < 80) {
            if (sConfigMgr->GetOption<uint32>("Boost.80.Fresh.Enabled", 1)) {
                boostAvailable = true;
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want a character boost to level 80 with fresh dungeon gear", GOSSIP_SENDER_MAIN, GOSSIP_MENU_CHOOSE_SPECIALIZATION_START + 2);
            }
            if (sConfigMgr->GetOption<uint32>("Boost.80.PreRaid.Enabled", 1)) {
                boostAvailable = true;
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want a character boost to level 80 with pre-Naxxramas gear", GOSSIP_SENDER_MAIN, GOSSIP_MENU_CHOOSE_SPECIALIZATION_START + 3);
            }
            if (sConfigMgr->GetOption<uint32>("Boost.80.PVP.Enabled", 1)) {
                boostAvailable = true;
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want a character boost to level 80 with PVP gear", GOSSIP_SENDER_MAIN, GOSSIP_MENU_CHOOSE_SPECIALIZATION_START + 4);
            }
        }

        if (boostAvailable) {
            SendGossipMenuFor(player, GOSSIP_BOOST_TEXT_DEFAULT, creature->GetGUID());
        } else {
            SendGossipMenuFor(player, GOSSIP_BOOST_TEXT_LEVEL_TOO_HIGH, creature->GetGUID());
        }

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        if (sender != GOSSIP_SENDER_MAIN)
        {
            return false;
        }

        if (action >= GOSSIP_MENU_CHOOSE_SPECIALIZATION_START && action <= GOSSIP_MENU_CHOOSE_SPECIALIZATION_END)
        {
            ClearGossipMenuFor(player);
            uint32 option = action - GOSSIP_MENU_CHOOSE_SPECIALIZATION_START;

            switch (player->getClass())
            {
            case CLASS_WARRIOR:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be an Arms Warrior", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_1 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Fury Warrior", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_2 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Protection Warrior", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_3 + option);
                break;
            case CLASS_PALADIN:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Holy Paladin", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_1 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Protection Paladin", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_2 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Retribution Paladin", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_3 + option);
                break;
            case CLASS_HUNTER:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Beast Mastery Hunter", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_1 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Marksmanship Hunter", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_2 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Survival Hunter", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_3 + option);
                break;
            case CLASS_ROGUE:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be an Assassination Rogue", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_1 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Combat Rogue", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_2 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Subtlety Rogue", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_3 + option);
                break;
            case CLASS_PRIEST:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Discipline Priest", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_1 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Holy Priest", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_2 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Shadow Priest", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_3 + option);
                break;
            case CLASS_DEATH_KNIGHT:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Blood Death Knight", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_1 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Frost Death Knight", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_2 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Unholy Death Knight", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_3 + option);
                break;
            case CLASS_SHAMAN:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be an Elemental Shaman", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_1 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be an Enhancement Shaman", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_2 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Restoration Shaman", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_3 + option);
                break;
            case CLASS_MAGE:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be an Arcane Mage", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_1 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Fire Mage", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_2 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Frost Mage", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_3 + option);
                break;
            case CLASS_WARLOCK:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be an Affliction Warlock", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_1 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Demonology Warlock", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_2 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Destruction Warlock", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_3 + option);
                break;
            case CLASS_DRUID:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Balance Druid", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_1 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Feral Druid", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_2 + option);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want to be a Restoration Druid", GOSSIP_SENDER_MAIN, GOSSIP_MENU_SPECIALIZATION_3 + option);
                break;
            }

            SendGossipMenuFor(player, GOSSIP_BOOST_TEXT_CHOOSE_SPECIALIZATION, creature->GetGUID());
        }
        else if (action >= GOSSIP_MENU_SPECIALIZATION_START && action <= GOSSIP_MENU_SPECIALIZATION_END)
        {
            ClearGossipMenuFor(player);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I'm sure!", GOSSIP_SENDER_MAIN, action + 100);
            SendGossipMenuFor(player, GOSSIP_BOOST_TEXT_CONFIRM_SPECIALIZATION, creature->GetGUID());
        }
        else if (action >= GOSSIP_MENU_SPECIALIZATION_CONFIRM_START && action <= GOSSIP_MENU_SPECIALIZATION_CONFIRM_END)
        {
            CloseGossipMenuFor(player);
            Boost::BoostPlayer(player, action - 300);
        }

        return true;
    } 
};

void AddSC_appreciation_creature()
{
    new AppreciationCreature();
}