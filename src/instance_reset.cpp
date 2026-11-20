#include "instance_reset.h"

#include "Creature.h"
#include "Group.h"
#include "GroupReference.h"
#include "InstanceSaveMgr.h"
#include "PlayerbotAI.h"

#include <string>
#include <vector>

namespace
{
    // Greeting row added by data/sql/db-world/base/mod_instance_reset_tofu_2026_11_19.sql
    constexpr uint32 NPC_TEXT_TOFU_GREETING = 900000;
    constexpr uint32 ACTION_RESET = GOSSIP_ACTION_INFO_DEF + 1;
    char const* const OPTION_TEXT = "Thank you. Please remove my instance saves.";

    bool cfg_enable = true;
    bool cfg_normalModeOnly = true;
    uint32 cfg_transactionType = 0;
    uint32 cfg_token = 49426;
    uint32 cfg_count = 26;
    uint32 cfg_money = 10000000;

    // True when the player meets the configured price.
    bool CanPay(Player* player)
    {
        switch (cfg_transactionType)
        {
            case 1:
                return player->HasItemCount(cfg_token, cfg_count, true);
            case 2:
                return player->GetMoney() >= cfg_money;
            case 3:
                return player->HasItemCount(cfg_token, cfg_count, true) && player->GetMoney() >= cfg_money;
            default:
                return true;
        }
    }

    char const* PriceRefusal()
    {
        switch (cfg_transactionType)
        {
            case 1:
                return "You do not have the required items or token.";
            case 2:
                return "You don't have enough money.";
            default:
                return "The reset requires a token and money.";
        }
    }

    void Charge(Player* player)
    {
        if (cfg_transactionType == 1 || cfg_transactionType == 3)
            player->DestroyItemCount(cfg_token, cfg_count, true);

        if (cfg_transactionType == 2 || cfg_transactionType == 3)
            player->ModifyMoney(-int32(cfg_money));
    }

    // Removes every instance bind of target, except the map it is standing in.
    // Map ids are collected first because PlayerUnbindInstance erases from the map being read.
    void UnbindAll(Player* target)
    {
        uint8 const difficulties = cfg_normalModeOnly ? 2 : uint8(MAX_DIFFICULTY);

        for (uint8 i = 0; i < difficulties; ++i)
        {
            Difficulty const difficulty = Difficulty(i);
            std::vector<uint32> mapIds;

            for (auto const& bind : sInstanceSaveMgr->PlayerGetBoundInstances(target->GetGUID(), difficulty))
                if (bind.first != target->GetMapId())
                    mapIds.push_back(bind.first);

            for (uint32 mapId : mapIds)
                sInstanceSaveMgr->PlayerUnbindInstance(target->GetGUID(), mapId, difficulty, true, target);
        }
    }

    // Unbinds the playerbots in the player's group. Real players and SelfBots (a real player's character on autopilot) are left alone.
    uint32 UnbindGroupBots(Player* player)
    {
        Group* group = player->GetGroup();
        if (!group)
            return 0;

        uint32 botsReset = 0;
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || member == player || !member->IsInWorld())
                continue;

            if (IsRealPlayer(member) || IsSelfBot(member))
                continue;

            UnbindAll(member);
            ++botsReset;
        }

        return botsReset;
    }
}

void InstanceResetAnnouncer::OnPlayerLogin(Player* player)
{
    if (sConfigMgr->GetOption<bool>("instanceReset.Announcer", true))
        ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Instance Reset |rmodule.");
}

bool InstanceReset::OnGossipHello(Player* player, Creature* creature)
{
    ClearGossipMenuFor(player);

    if (cfg_enable)
    {
        if (CanPay(player))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, OPTION_TEXT, GOSSIP_SENDER_MAIN, ACTION_RESET);
        else
            creature->Whisper(PriceRefusal(), LANG_UNIVERSAL, player);
    }

    SendGossipMenuFor(player, NPC_TEXT_TOFU_GREETING, creature->GetGUID());
    return true;
}

bool InstanceReset::OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
{
    ClearGossipMenuFor(player);

    if (action != ACTION_RESET || !cfg_enable)
    {
        CloseGossipMenuFor(player);
        return true;
    }

    // Re-check the price: the player may have spent the money or token since the menu opened.
    if (!CanPay(player))
    {
        creature->Whisper(PriceRefusal(), LANG_UNIVERSAL, player);
        CloseGossipMenuFor(player);
        return true;
    }

    UnbindAll(player);
    uint32 const botsReset = UnbindGroupBots(player);
    Charge(player);

    std::string message = "Your instances have been reset.";
    if (botsReset == 1)
        message += " So have those of the 1 bot in your group.";
    else if (botsReset > 1)
        message += " So have those of the " + std::to_string(botsReset) + " bots in your group.";

    creature->Whisper(message, LANG_UNIVERSAL, player);
    CloseGossipMenuFor(player);
    return true;
}

void InstanceResetWorldConfig::OnBeforeConfigLoad(bool /*reload*/)
{
    cfg_enable = sConfigMgr->GetOption<bool>("instanceReset.Enable", true);
    cfg_normalModeOnly = sConfigMgr->GetOption<bool>("instanceReset.NormalModeOnly", true);
    cfg_transactionType = sConfigMgr->GetOption<uint32>("instanceReset.TransactionType", 0);
    cfg_token = sConfigMgr->GetOption<uint32>("instanceReset.TokenID", 49426);
    cfg_count = sConfigMgr->GetOption<uint32>("instanceReset.TokenCount", 26);
    cfg_money = sConfigMgr->GetOption<uint32>("instanceReset.MoneyCount", 10000000);
}

void AddInstanceResetScripts()
{
    new InstanceReset();
    new InstanceResetAnnouncer();
    new InstanceResetWorldConfig();
}
