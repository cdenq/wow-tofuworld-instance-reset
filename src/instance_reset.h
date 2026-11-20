#ifndef MOD_INSTANCE_RESET_H
#define MOD_INSTANCE_RESET_H

#include "Chat.h"
#include "Configuration/Config.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"

class InstanceResetAnnouncer : public PlayerScript
{
public:
    InstanceResetAnnouncer() : PlayerScript("InstanceResetAnnouncer", {
        PLAYERHOOK_ON_LOGIN
    }) {}

    void OnPlayerLogin(Player* player) override;
};

class InstanceReset : public CreatureScript
{
public:
    InstanceReset() : CreatureScript("instance_reset") {}

    bool OnGossipHello(Player* player, Creature* creature) override;
    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override;
};

class InstanceResetWorldConfig : public WorldScript
{
public:
    InstanceResetWorldConfig() : WorldScript("InstanceResetWorldConfig", {
        WORLDHOOK_ON_BEFORE_CONFIG_LOAD
    }) { }

    void OnBeforeConfigLoad(bool /*reload*/) override;
};

#endif
