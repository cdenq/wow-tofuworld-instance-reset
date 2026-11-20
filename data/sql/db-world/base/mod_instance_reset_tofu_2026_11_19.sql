-- Tofu Worldview: greeting text for the instance reset NPC (Cromi, entry 300000).
-- Schema: acore_world. Applied by the worldserver's module updater at start (state MODULE); no reload needed for new players, but a client that already saw Cromi must delete its Cache folder.
-- Text ID 900000 is registered in claude/server-state.md (npc_text 900000 to 900009). The script sends it with SendGossipMenuFor.
-- Re-runnable: DELETE then INSERT of one fixed row.
-- Check: SELECT ID, text0_0, Probability0 FROM acore_world.npc_text WHERE ID = 900000; -- expected 1 row, Probability0 1

DELETE FROM `npc_text` WHERE `ID` = 900000;
INSERT INTO `npc_text` (`ID`, `text0_0`, `text0_1`, `Probability0`) VALUES
(900000, 'Hello, $N. I can restore the fortitude of your soul so that you may brave the challenges of Azeroth''s toughest dungeons and raids yet again. Normally I would charge, but the guild Tofu Worldview has paid for all your future services indefinitely.', 'Hello, $N. I can restore the fortitude of your soul so that you may brave the challenges of Azeroth''s toughest dungeons and raids yet again. Normally I would charge, but the guild Tofu Worldview has paid for all your future services indefinitely.', 1);
