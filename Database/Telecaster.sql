-- --------------------------------------------------------
-- Host:                         192.168.0.12
-- Server Version:               10.1.23-MariaDB-9+deb9u1 - Raspbian 9.0
-- Server Betriebssystem:        debian-linux-gnueabihf
-- HeidiSQL Version:             9.4.0.5174
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- Exportiere Daten aus Tabelle Telecaster.Alliance: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `Alliance` DISABLE KEYS */;
/*!40000 ALTER TABLE `Alliance` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.AllowedCommandsForPermission: ~198 rows (ungefähr)
/*!40000 ALTER TABLE `AllowedCommandsForPermission` DISABLE KEYS */;
INSERT INTO `AllowedCommandsForPermission` (`sid`, `permission`, `command`, `parameter`) VALUES
	(1, 1, 'suicide', ''),
	(2, 1, 'set_huntaholic_point', ''),
	(3, 1, 'show_huntaholic_lobby_window', ''),
	(4, 1, 'exit_huntaholic_lobby', ''),
	(5, 1, 'warp_to_huntaholic_lobby', ''),
	(6, 1, 'show_auction_window', ''),
	(7, 1, 'reset_summon_skill', ''),
	(8, 1, 'creature_name_change_box', ''),
	(9, 1, 'get_creature_name_id', ''),
	(10, 1, 'set_continuous_play_time', ''),
	(11, 1, 'set_pcbang_user', ''),
	(12, 1, 'donation_reward', ''),
	(13, 1, 'show_donation_prop', ''),
	(14, 1, 'update_gold_chaos', ''),
	(15, 1, 'get_local_info', ''),
	(16, 1, 'open_popup_and_set_size', ''),
	(17, 1, 'open_popup', ''),
	(18, 1, 'open_url', ''),
	(19, 1, 'clear_auto_account', ''),
	(20, 1, 'set_auto_account', ''),
	(21, 1, 'set_auto_user', ''),
	(22, 1, 'find_npc', ''),
	(23, 1, 'get_server_category', ''),
	(24, 1, 'warp_to_revive_position', ''),
	(25, 1, 'recall_feather', ''),
	(26, 1, 'set_way_point_speed', ''),
	(27, 1, 'set_way_point_type', ''),
	(28, 1, 'add_way_point', ''),
	(29, 1, 'draw_tax_chaos', ''),
	(30, 1, 'draw_tax', ''),
	(31, 1, 'get_tax_chaos_amount', ''),
	(32, 1, 'get_tax_amount', ''),
	(33, 1, 'set_tax_rate', ''),
	(34, 1, 'get_tax_rate', ''),
	(35, 1, 'get_dungeon_relation', ''),
	(36, 1, 'get_own_guild_name', ''),
	(37, 1, 'get_guild_block_time', ''),
	(38, 1, 'set_guild_block_time', ''),
	(39, 1, 'is_guild_leader', ''),
	(40, 1, 'update_guild_info', ''),
	(41, 1, 'show_channel_set', ''),
	(42, 1, 'equip_summon_card', ''),
	(43, 1, 'get_summon_name_id', ''),
	(44, 1, 'get_layer_of_channel', ''),
	(45, 1, 'get_user_count_in_channel', ''),
	(46, 1, 'get_max_channel_num', ''),
	(47, 1, 'get_min_channel_num', ''),
	(48, 1, 'get_proper_channel_num', ''),
	(49, 1, 'creature_learn_skill', ''),
	(50, 1, 'call_lc_In', ''),
	(51, 1, 'creature_evolution', ''),
	(52, 1, 'get_npc_handle', ''),
	(53, 1, 'get_npc_id', ''),
	(54, 1, 'check_valid_alliance_name', ''),
	(55, 1, 'destroy_alliance', ''),
	(56, 1, 'create_alliance', ''),
	(57, 1, 'check_valid_guild_name', ''),
	(58, 1, 'force_promote_guild_leader', ''),
	(59, 1, 'force_change_guild_name', ''),
	(60, 1, 'show_alliance_create', ''),
	(61, 1, 'show_guild_create', ''),
	(62, 1, 'destroy_guild', ''),
	(63, 1, 'create_guild', ''),
	(64, 1, 'get_user_count_near', ''),
	(65, 1, 'open_storage', ''),
	(66, 1, 'get_creature_handle', ''),
	(67, 1, 'get_string', ''),
	(68, 1, 'get_env', ''),
	(69, 1, 'set_env', ''),
	(70, 1, 'remove_cstate', ''),
	(71, 1, 'add_cstate', ''),
	(72, 1, 'remove_state', ''),
	(73, 1, 'get_state_level', ''),
	(74, 1, 'add_state', ''),
	(75, 1, 'is_changeable_job', ''),
	(76, 1, 'message', ''),
	(77, 1, 'sconv', ''),
	(78, 1, 'show_creature_dialog', ''),
	(79, 1, 'get_base_skill_level', ''),
	(80, 1, 'learn_creature_all_skill', ''),
	(81, 1, 'learn_all_skill', ''),
	(82, 1, 'learn_skill', ''),
	(83, 1, 'refresh', ''),
	(84, 1, 'gametime', ''),
	(85, 1, 'open_market', ''),
	(86, 1, 'get_monster_id', ''),
	(87, 1, 'set_next_attackable_time', ''),
	(88, 1, 'monster_skill_cast', ''),
	(89, 1, 'respawn_near_monster', ''),
	(90, 1, 'save', ''),
	(91, 1, 'delete_block_account', ''),
	(92, 1, 'shutdown', ''),
	(93, 1, 'saveall', ''),
	(94, 1, 'setspeed', ''),
	(95, 1, 'get_siege_dungeon_id', ''),
	(96, 1, 'get_own_dungeon_id', ''),
	(97, 1, 'clear_dungeon_core_guardian', ''),
	(98, 1, 'change_dungeon_owner', ''),
	(99, 1, 'drop_dungeon_owner_ship', ''),
	(100, 1, 'enter_dungeon', ''),
	(101, 1, 'request_dungeon_raid', ''),
	(102, 1, 'change_tactical_position_owner', ''),
	(103, 1, 'show_dungeon_stone', ''),
	(104, 1, 'respawn_guardian_object', ''),
	(105, 1, 'is_in_siege_dungeon', ''),
	(106, 1, 'recall_player', ''),
	(107, 1, 'whisper', ''),
	(108, 1, 'kill_target', ''),
	(109, 1, 'kill', ''),
	(110, 1, 'raid_respawn_rare_mob', ''),
	(111, 1, 'raid_respawn', ''),
	(112, 1, 'respawn_roaming_mob', ''),
	(113, 1, 'respawn_rare_mob', ''),
	(114, 1, 'respawn_guardian', ''),
	(115, 1, 'respawn', ''),
	(116, 1, 'add_npc_to_world', ''),
	(117, 1, 'add_npc', ''),
	(118, 1, 'env', ''),
	(119, 1, 'force_unregister_account', ''),
	(120, 1, 'kick', ''),
	(121, 1, 'get_last_accept_quest', ''),
	(122, 1, 'get_quest_progress', ''),
	(123, 1, 'quest_info', ''),
	(124, 1, 'end_quest', ''),
	(125, 1, 'force_start_quest', ''),
	(126, 1, 'start_quest', ''),
	(127, 1, 'dlg_text_without_quest_menu', ''),
	(128, 1, 'quest_text_without_quest_menu', ''),
	(129, 1, 'dlg_show', ''),
	(130, 1, 'dlg_menu', ''),
	(131, 1, 'dlg_text', ''),
	(132, 1, 'dlg_title', ''),
	(133, 1, 'get_max_alliance_member_count', ''),
	(135, 1, 'is_alliance_leader', ''),
	(136, 1, 'set_pk_mode', ''),
	(137, 1, 'del_flag', ''),
	(138, 1, 'set_flag', ''),
	(139, 1, 'get_flag', ''),
	(140, 1, 'emcv', ''),
	(141, 1, 'gmcv', ''),
	(142, 1, 'gcv', ''),
	(143, 1, 'smcv', ''),
	(144, 1, 'scv', ''),
	(145, 1, 'change_creature_name', ''),
	(146, 1, 'get_creature_value', ''),
	(147, 1, 'set_creature_value', ''),
	(148, 1, 'get_all_value', ''),
	(149, 1, 'ev', ''),
	(150, 1, 'sv', ''),
	(151, 1, 'av', ''),
	(152, 1, 'gv', ''),
	(153, 1, 'echo_value', ''),
	(154, 1, 'set_value', ''),
	(155, 1, 'add_value', ''),
	(156, 1, 'get_value', ''),
	(157, 1, 'supply_event_item', ''),
	(158, 1, 'stop_event_supply', ''),
	(159, 1, 'start_event_supply', ''),
	(160, 1, 'refresh_event_supply', ''),
	(161, 1, 'event_supply', ''),
	(162, 1, 'stop_event_drop', ''),
	(163, 1, 'start_event_drop', ''),
	(164, 1, 'refresh_event_drop', ''),
	(165, 1, 'event_drop', ''),
	(166, 1, 'show_soulstone_repair_window', ''),
	(167, 1, 'get_max_item_soulstone_endurance', ''),
	(168, 1, 'get_item_soulstone_endurance', ''),
	(169, 1, 'show_soulstone_craft_window', ''),
	(170, 1, 'get_wear_item_handle', ''),
	(171, 1, 'get_item_name_id', ''),
	(172, 1, 'get_item_name_by_code', ''),
	(173, 1, 'set_socket_info', ''),
	(174, 1, 'get_socket_info', ''),
	(175, 1, 'set_item_endurance', ''),
	(176, 1, 'get_item_endurance', ''),
	(177, 1, 'get_item_price', ''),
	(178, 1, 'get_item_rank', ''),
	(179, 1, 'get_item_enhance', ''),
	(180, 1, 'set_item_enhance', ''),
	(181, 1, 'set_item_level', ''),
	(182, 1, 'get_item_level', ''),
	(183, 1, 'get_item_code', ''),
	(184, 1, 'get_item_name', ''),
	(185, 1, 'get_item_handle', ''),
	(186, 1, 'has_item', ''),
	(187, 1, 'insert_gold', ''),
	(188, 1, 'drop_gold', ''),
	(189, 1, 'drop_item', ''),
	(190, 1, 'find_item', ''),
	(191, 1, 'is_erasable_item', ''),
	(192, 1, 'delete_item', ''),
	(193, 1, 'insert_item', ''),
	(194, 1, 'notice', ''),
	(195, 1, 'warp', ''),
	(196, 1, 'invisible', ''),
	(197, 1, 'add_state', ''),
	(199, 1, 'regenerate', ''),
	(200, 1, 'block', '');
/*!40000 ALTER TABLE `AllowedCommandsForPermission` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.Auction: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `Auction` DISABLE KEYS */;
/*!40000 ALTER TABLE `Auction` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.AutoAuctionRegistrationInfo: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `AutoAuctionRegistrationInfo` DISABLE KEYS */;
/*!40000 ALTER TABLE `AutoAuctionRegistrationInfo` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.Character: ~29 rows (ungefähr)
/*!40000 ALTER TABLE `Character` DISABLE KEYS */;
/*!40000 ALTER TABLE `Character` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.Check_Aplname_Proc: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `Check_Aplname_Proc` DISABLE KEYS */;
/*!40000 ALTER TABLE `Check_Aplname_Proc` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.Denials: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `Denials` DISABLE KEYS */;
/*!40000 ALTER TABLE `Denials` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.Dungeon: ~11 rows (ungefähr)
/*!40000 ALTER TABLE `Dungeon` DISABLE KEYS */;
INSERT INTO `Dungeon` (`dungeon_id`, `owner_guild_id`, `raid_guild_id`, `best_raid_time`, `last_dungeon_siege_finish_time`, `last_dungeon_raid_wrap_up_time`, `tax_rate`) VALUES
	(121000, 0, 0, -1, 1367204403, 1369433809, 5),
	(122000, 0, 0, -1, 1367204404, 1369433809, 5),
	(123000, 0, 0, -1, 1367204405, 1369433809, 5),
	(130000, 0, 0, -1, 0, 1369433809, 5),
	(130300, 0, 0, -1, 0, 1369433809, 5),
	(130400, 0, 0, -1, 0, 1369433809, 5),
	(130500, 0, 0, -1, 1367208001, 1369433809, 5),
	(130600, 0, 0, -1, 1367204402, 1369433809, 5),
	(130700, 0, 0, -1, 1367200801, 1369433809, 5),
	(130800, 0, 0, -1, 0, 1369433809, 5),
	(130900, 0, 0, -1, 1367200802, 1369433809, 5);
/*!40000 ALTER TABLE `Dungeon` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.DungeonRaidRecord: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `DungeonRaidRecord` DISABLE KEYS */;
/*!40000 ALTER TABLE `DungeonRaidRecord` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.DungeonRaidRecord_Log: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `DungeonRaidRecord_Log` DISABLE KEYS */;
/*!40000 ALTER TABLE `DungeonRaidRecord_Log` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.EventAreaEnterCount: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `EventAreaEnterCount` DISABLE KEYS */;
/*!40000 ALTER TABLE `EventAreaEnterCount` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.EventItemDropInfo: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `EventItemDropInfo` DISABLE KEYS */;
/*!40000 ALTER TABLE `EventItemDropInfo` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.EventItemSupplyInfo: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `EventItemSupplyInfo` DISABLE KEYS */;
/*!40000 ALTER TABLE `EventItemSupplyInfo` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.Favor: ~11 rows (ungefähr)
/*!40000 ALTER TABLE `Favor` DISABLE KEYS */;
/*!40000 ALTER TABLE `Favor` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.Friends: ~8 rows (ungefähr)
/*!40000 ALTER TABLE `Friends` DISABLE KEYS */;
/*!40000 ALTER TABLE `Friends` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.GlobalVariable: ~1 rows (ungefähr)
/*!40000 ALTER TABLE `GlobalVariable` DISABLE KEYS */;
/*!40000 ALTER TABLE `GlobalVariable` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.Guild: ~1 rows (ungefähr)
/*!40000 ALTER TABLE `Guild` DISABLE KEYS */;
/*!40000 ALTER TABLE `Guild` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.HuntaholicInfo: ~1 rows (ungefähr)
/*!40000 ALTER TABLE `HuntaholicInfo` DISABLE KEYS */;
/*!40000 ALTER TABLE `HuntaholicInfo` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.HuntaholicRanking: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `HuntaholicRanking` DISABLE KEYS */;
/*!40000 ALTER TABLE `HuntaholicRanking` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.Item: ~1.804 rows (ungefähr)
/*!40000 ALTER TABLE `Item` DISABLE KEYS */;
/*!40000 ALTER TABLE `Item` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.ItemCoolTime: ~16 rows (ungefähr)
/*!40000 ALTER TABLE `ItemCoolTime` DISABLE KEYS */;
/*!40000 ALTER TABLE `ItemCoolTime` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.ItemKeeping: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `ItemKeeping` DISABLE KEYS */;
/*!40000 ALTER TABLE `ItemKeeping` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.PaidItem: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `PaidItem` DISABLE KEYS */;
/*!40000 ALTER TABLE `PaidItem` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.Party: ~2 rows (ungefähr)
/*!40000 ALTER TABLE `Party` DISABLE KEYS */;
/*!40000 ALTER TABLE `Party` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.Pet: ~4 rows (ungefähr)
/*!40000 ALTER TABLE `Pet` DISABLE KEYS */;
/*!40000 ALTER TABLE `Pet` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.Quest: ~12 rows (ungefähr)
/*!40000 ALTER TABLE `Quest` DISABLE KEYS */;
/*!40000 ALTER TABLE `Quest` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.RandomQuestInfo: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `RandomQuestInfo` DISABLE KEYS */;
/*!40000 ALTER TABLE `RandomQuestInfo` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.RankingInfo: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `RankingInfo` DISABLE KEYS */;
/*!40000 ALTER TABLE `RankingInfo` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.RankingScore: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `RankingScore` DISABLE KEYS */;
/*!40000 ALTER TABLE `RankingScore` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.ScheduledCommand: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `ScheduledCommand` DISABLE KEYS */;
/*!40000 ALTER TABLE `ScheduledCommand` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.SecurityNo: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `SecurityNo` DISABLE KEYS */;
/*!40000 ALTER TABLE `SecurityNo` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.Skill: ~428 rows (ungefähr)
/*!40000 ALTER TABLE `Skill` DISABLE KEYS */;
/*!40000 ALTER TABLE `Skill` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.State: ~71 rows (ungefähr)
/*!40000 ALTER TABLE `State` DISABLE KEYS */;
/*!40000 ALTER TABLE `State` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.Summon: ~17 rows (ungefähr)
/*!40000 ALTER TABLE `Summon` DISABLE KEYS */;
/*!40000 ALTER TABLE `Summon` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.TBLSTATS: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `TBLSTATS` DISABLE KEYS */;
/*!40000 ALTER TABLE `TBLSTATS` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.tb_character_job_lv_log: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `tb_character_job_lv_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `tb_character_job_lv_log` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.tb_event_333: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `tb_event_333` DISABLE KEYS */;
/*!40000 ALTER TABLE `tb_event_333` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.tb_temp_0504: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `tb_temp_0504` DISABLE KEYS */;
/*!40000 ALTER TABLE `tb_temp_0504` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.TRIG_Character_BankGold: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `TRIG_Character_BankGold` DISABLE KEYS */;
/*!40000 ALTER TABLE `TRIG_Character_BankGold` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Telecaster.TRIG_Character_Gold: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `TRIG_Character_Gold` DISABLE KEYS */;
/*!40000 ALTER TABLE `TRIG_Character_Gold` ENABLE KEYS */;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
