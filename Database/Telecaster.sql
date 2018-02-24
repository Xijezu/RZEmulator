-- --------------------------------------------------------
-- Host:                         192.168.0.12
-- Server Version:               10.1.23-MariaDB-9+deb9u1 - Raspbian 9.0
-- Server Betriebssystem:        debian-linux-gnueabihf
-- HeidiSQL Version:             9.5.0.5196
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


-- Exportiere Datenbank Struktur für Telecaster
DROP DATABASE IF EXISTS `Telecaster`;
CREATE DATABASE IF NOT EXISTS `Telecaster` /*!40100 DEFAULT CHARACTER SET utf8mb4 */;
USE `Telecaster`;

-- Exportiere Struktur von Tabelle Telecaster.Alliance
DROP TABLE IF EXISTS `Alliance`;
CREATE TABLE IF NOT EXISTS `Alliance` (
  `sid` int(11) NOT NULL DEFAULT '0',
  `lead_guild_id` int(11) NOT NULL DEFAULT '0',
  `name` varchar(128) NOT NULL,
  `max_alliance_cnt` int(11) NOT NULL DEFAULT '0',
  `name_changed` int(11) NOT NULL DEFAULT '1',
  PRIMARY KEY (`sid`),
  UNIQUE KEY `UQ__Alliance__72E12F1B5ECA0095` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.AllowedCommandsForPermission
DROP TABLE IF EXISTS `AllowedCommandsForPermission`;
CREATE TABLE IF NOT EXISTS `AllowedCommandsForPermission` (
  `sid` int(11) NOT NULL AUTO_INCREMENT,
  `permission` int(11) NOT NULL DEFAULT '0',
  `command` varchar(32) NOT NULL,
  `parameter` varchar(32) NOT NULL,
  PRIMARY KEY (`sid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.Auction
DROP TABLE IF EXISTS `Auction`;
CREATE TABLE IF NOT EXISTS `Auction` (
  `sid` int(11) NOT NULL DEFAULT '0',
  `item_id` bigint(20) NOT NULL,
  `seller_id` int(11) NOT NULL DEFAULT '0',
  `seller_name` varchar(31) NOT NULL,
  `is_secroute_only` char(1) NOT NULL,
  `end_time` datetime NOT NULL,
  `instant_purchase_price` bigint(20) NOT NULL,
  `registration_tax` bigint(20) NOT NULL,
  `bidder_list` varchar(1152) NOT NULL,
  `highest_bidding_price` bigint(20) NOT NULL,
  `highest_bidder_id` int(11) NOT NULL DEFAULT '0',
  `highest_bidder_name` varchar(31) NOT NULL,
  PRIMARY KEY (`sid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.AutoAuctionRegistrationInfo
DROP TABLE IF EXISTS `AutoAuctionRegistrationInfo`;
CREATE TABLE IF NOT EXISTS `AutoAuctionRegistrationInfo` (
  `sid` int(11) NOT NULL AUTO_INCREMENT,
  `auto_auction_id` int(11) NOT NULL DEFAULT '0',
  `scheduled_register_time` datetime NOT NULL,
  `registered_time` datetime NOT NULL,
  PRIMARY KEY (`sid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.Character
DROP TABLE IF EXISTS `Character`;
CREATE TABLE IF NOT EXISTS `Character` (
  `sid` int(11) NOT NULL DEFAULT '0',
  `name` varchar(61) NOT NULL,
  `account` varchar(60) NOT NULL,
  `account_id` int(11) NOT NULL DEFAULT '0',
  `slot` int(11) NOT NULL DEFAULT '0',
  `x` int(11) NOT NULL DEFAULT '0',
  `y` int(11) NOT NULL DEFAULT '0',
  `z` int(11) NOT NULL DEFAULT '0',
  `layer` int(11) NOT NULL DEFAULT '0',
  `race` int(11) NOT NULL DEFAULT '0',
  `sex` int(11) NOT NULL DEFAULT '0',
  `exp` bigint(20) NOT NULL,
  `last_decreased_exp` bigint(20) NOT NULL DEFAULT '0',
  `lv` int(11) NOT NULL DEFAULT '0',
  `hp` int(11) NOT NULL DEFAULT '0',
  `mp` int(11) NOT NULL DEFAULT '0',
  `stamina` int(11) NOT NULL DEFAULT '0',
  `havoc` int(11) NOT NULL DEFAULT '0',
  `jlv` int(11) NOT NULL DEFAULT '0',
  `jp` int(11) NOT NULL DEFAULT '0',
  `total_jp` int(11) DEFAULT NULL,
  `job_0` int(11) DEFAULT NULL,
  `job_1` int(11) DEFAULT NULL,
  `job_2` int(11) DEFAULT NULL,
  `jlv_0` int(11) DEFAULT NULL,
  `jlv_1` int(11) DEFAULT NULL,
  `jlv_2` int(11) DEFAULT NULL,
  `immoral_point` decimal(18,4) NOT NULL,
  `cha` int(11) NOT NULL DEFAULT '0',
  `pkc` int(11) NOT NULL DEFAULT '0',
  `dkc` int(11) NOT NULL DEFAULT '0',
  `huntaholic_point` int(11) NOT NULL DEFAULT '0',
  `create_time` datetime NOT NULL,
  `delete_time` datetime DEFAULT NULL,
  `login_time` datetime NOT NULL,
  `logout_time` datetime NOT NULL,
  `login_count` int(11) NOT NULL DEFAULT '0',
  `play_time` int(11) NOT NULL DEFAULT '0',
  `belt_00` bigint(20) NOT NULL,
  `belt_01` bigint(20) NOT NULL,
  `belt_02` bigint(20) NOT NULL,
  `belt_03` bigint(20) NOT NULL,
  `belt_04` bigint(20) NOT NULL,
  `belt_05` bigint(20) NOT NULL,
  `permission` int(11) NOT NULL DEFAULT '0',
  `skin_color` int(11) NOT NULL DEFAULT '0',
  `model_00` int(11) NOT NULL DEFAULT '0',
  `model_01` int(11) NOT NULL DEFAULT '0',
  `model_02` int(11) NOT NULL DEFAULT '0',
  `model_03` int(11) NOT NULL DEFAULT '0',
  `model_04` int(11) NOT NULL DEFAULT '0',
  `job` int(11) NOT NULL DEFAULT '0',
  `gold` bigint(20) NOT NULL,
  `party_id` int(11) NOT NULL DEFAULT '0',
  `guild_id` int(11) NOT NULL DEFAULT '0',
  `prev_guild_id` int(11) NOT NULL DEFAULT '0',
  `flag_list` varchar(1000) NOT NULL,
  `client_info` varchar(4096) NOT NULL,
  `job_depth` tinyint(3) unsigned NOT NULL,
  `summon_0` int(11) NOT NULL DEFAULT '0',
  `summon_1` int(11) NOT NULL DEFAULT '0',
  `summon_2` int(11) NOT NULL DEFAULT '0',
  `summon_3` int(11) NOT NULL DEFAULT '0',
  `summon_4` int(11) NOT NULL DEFAULT '0',
  `summon_5` int(11) NOT NULL DEFAULT '0',
  `main_summon` int(11) NOT NULL DEFAULT '0',
  `sub_summon` int(11) NOT NULL DEFAULT '0',
  `remain_summon_time` int(11) NOT NULL DEFAULT '0',
  `pet` int(11) NOT NULL DEFAULT '0',
  `chaos` int(11) NOT NULL DEFAULT '0',
  `adv_chat_count` int(11) NOT NULL DEFAULT '0',
  `name_changed` int(11) NOT NULL DEFAULT '0',
  `auto_used` int(11) NOT NULL DEFAULT '0',
  `guild_block_time` bigint(20) NOT NULL,
  `pkmode` tinyint(3) unsigned NOT NULL,
  `otp_value` int(11) NOT NULL DEFAULT '0',
  `otp_date` datetime NOT NULL,
  `chat_block_time` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`sid`),
  UNIQUE KEY `UQ__Character__0AD2A005` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='utf8mb4_general_ci';

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.Check_Aplname_Proc
DROP TABLE IF EXISTS `Check_Aplname_Proc`;
CREATE TABLE IF NOT EXISTS `Check_Aplname_Proc` (
  `log_date` datetime NOT NULL,
  `SESSION` varchar(30) NOT NULL,
  `SYS_USER` varchar(30) NOT NULL,
  `APL_NAME` varchar(60) NOT NULL,
  `HOST_NAME` varchar(30) NOT NULL,
  `HOST_ID` varchar(30) NOT NULL,
  `sid` bigint(20) NOT NULL,
  `gold` bigint(20) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.Denials
DROP TABLE IF EXISTS `Denials`;
CREATE TABLE IF NOT EXISTS `Denials` (
  `sid` int(11) NOT NULL AUTO_INCREMENT,
  `owner_id` varchar(50) NOT NULL,
  `denial_id` varchar(50) NOT NULL,
  PRIMARY KEY (`sid`),
  KEY `IDX_Denials_Friend_Id` (`denial_id`,`owner_id`),
  KEY `IDX_Denials_Owner_Id` (`owner_id`,`denial_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.Dungeon
DROP TABLE IF EXISTS `Dungeon`;
CREATE TABLE IF NOT EXISTS `Dungeon` (
  `dungeon_id` int(11) NOT NULL DEFAULT '0',
  `owner_guild_id` int(11) NOT NULL DEFAULT '0',
  `raid_guild_id` int(11) NOT NULL DEFAULT '0',
  `best_raid_time` int(11) NOT NULL DEFAULT '0',
  `last_dungeon_siege_finish_time` int(11) NOT NULL DEFAULT '0',
  `last_dungeon_raid_wrap_up_time` int(11) NOT NULL DEFAULT '0',
  `tax_rate` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`dungeon_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.DungeonRaidRecord
DROP TABLE IF EXISTS `DungeonRaidRecord`;
CREATE TABLE IF NOT EXISTS `DungeonRaidRecord` (
  `dungeon_id` int(11) NOT NULL DEFAULT '0',
  `guild_id` int(11) NOT NULL DEFAULT '0',
  `raid_time` int(11) NOT NULL DEFAULT '0',
  `record` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`dungeon_id`,`guild_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.DungeonRaidRecord_Log
DROP TABLE IF EXISTS `DungeonRaidRecord_Log`;
CREATE TABLE IF NOT EXISTS `DungeonRaidRecord_Log` (
  `log_date` datetime NOT NULL,
  `dungeon_id` int(11) NOT NULL DEFAULT '0',
  `guild_id` int(11) NOT NULL DEFAULT '0',
  `raid_time` int(11) NOT NULL DEFAULT '0',
  `record` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.EventAreaEnterCount
DROP TABLE IF EXISTS `EventAreaEnterCount`;
CREATE TABLE IF NOT EXISTS `EventAreaEnterCount` (
  `player_id` int(11) NOT NULL DEFAULT '0',
  `event_area_id` int(11) NOT NULL DEFAULT '0',
  `enter_count` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`player_id`,`event_area_id`),
  KEY `IDX_EventAreaEnterCount_Player_Id` (`player_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.EventItemDropInfo
DROP TABLE IF EXISTS `EventItemDropInfo`;
CREATE TABLE IF NOT EXISTS `EventItemDropInfo` (
  `sid` int(11) NOT NULL DEFAULT '0',
  `code` int(11) NOT NULL DEFAULT '0',
  `remain_time` int(11) NOT NULL DEFAULT '0',
  `duration` int(11) NOT NULL DEFAULT '0',
  `count` int(11) NOT NULL DEFAULT '0',
  `total_count` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`sid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.EventItemSupplyInfo
DROP TABLE IF EXISTS `EventItemSupplyInfo`;
CREATE TABLE IF NOT EXISTS `EventItemSupplyInfo` (
  `sid` int(11) NOT NULL DEFAULT '0',
  `code` int(11) NOT NULL DEFAULT '0',
  `min_count` int(11) NOT NULL DEFAULT '0',
  `max_count` int(11) NOT NULL DEFAULT '0',
  `flag` int(11) NOT NULL DEFAULT '0',
  `start_time` datetime NOT NULL,
  `end_time` datetime NOT NULL,
  `left_count` int(11) NOT NULL DEFAULT '0',
  `total_count` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`sid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.Favor
DROP TABLE IF EXISTS `Favor`;
CREATE TABLE IF NOT EXISTS `Favor` (
  `owner_id` int(11) NOT NULL DEFAULT '0',
  `favor_id` int(11) NOT NULL DEFAULT '0',
  `favor` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`owner_id`,`favor_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.Friends
DROP TABLE IF EXISTS `Friends`;
CREATE TABLE IF NOT EXISTS `Friends` (
  `sid` int(11) NOT NULL AUTO_INCREMENT,
  `owner_id` varchar(50) NOT NULL,
  `friend_id` varchar(50) NOT NULL,
  PRIMARY KEY (`sid`),
  KEY `IDX_Friends_Friend_Id` (`friend_id`,`owner_id`),
  KEY `IDX_Friends_Owner_Id` (`owner_id`,`friend_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.GlobalVariable
DROP TABLE IF EXISTS `GlobalVariable`;
CREATE TABLE IF NOT EXISTS `GlobalVariable` (
  `name` varchar(128) NOT NULL,
  `value` varchar(1024) NOT NULL,
  PRIMARY KEY (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.Guild
DROP TABLE IF EXISTS `Guild`;
CREATE TABLE IF NOT EXISTS `Guild` (
  `sid` int(11) NOT NULL DEFAULT '0',
  `name` varchar(128) NOT NULL,
  `leader_id` int(11) NOT NULL DEFAULT '0',
  `raid_leader_id` int(11) NOT NULL DEFAULT '0',
  `notice` varchar(255) NOT NULL,
  `icon` varchar(255) NOT NULL,
  `icon_size` int(11) NOT NULL DEFAULT '0',
  `name_changed` int(11) NOT NULL DEFAULT '0',
  `dungeon_id` int(11) NOT NULL DEFAULT '0',
  `dungeon_block_time` bigint(20) DEFAULT '0',
  `gold` bigint(20) NOT NULL,
  `chaos` int(11) NOT NULL DEFAULT '0',
  `alliance_id` int(11) NOT NULL DEFAULT '0',
  `alliance_block_time` bigint(20) NOT NULL,
  `donation_point` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`sid`),
  UNIQUE KEY `UQ__Guild__6B24EA82` (`leader_id`),
  UNIQUE KEY `UQ__Guild__6A30C649` (`name`),
  UNIQUE KEY `IDX_Guild_GuildName` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.HuntaholicInfo
DROP TABLE IF EXISTS `HuntaholicInfo`;
CREATE TABLE IF NOT EXISTS `HuntaholicInfo` (
  `huntaholic_id` int(11) DEFAULT NULL,
  `next_settling_time` datetime DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.HuntaholicRanking
DROP TABLE IF EXISTS `HuntaholicRanking`;
CREATE TABLE IF NOT EXISTS `HuntaholicRanking` (
  `huntaholic_id` int(11) DEFAULT NULL,
  `owner_id` int(11) DEFAULT NULL,
  `LAST_MONTH_SCORE` bigint(20) DEFAULT NULL,
  `this_month_score` bigint(20) DEFAULT NULL,
  `total_score` bigint(20) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.Item
DROP TABLE IF EXISTS `Item`;
CREATE TABLE IF NOT EXISTS `Item` (
  `sid` bigint(20) NOT NULL,
  `owner_id` int(11) NOT NULL DEFAULT '0',
  `account_id` int(11) NOT NULL DEFAULT '0',
  `summon_id` int(11) NOT NULL DEFAULT '0',
  `auction_id` int(11) NOT NULL DEFAULT '0',
  `keeping_id` int(11) NOT NULL DEFAULT '0',
  `idx` int(11) NOT NULL DEFAULT '0',
  `code` int(11) NOT NULL DEFAULT '0',
  `cnt` bigint(20) NOT NULL,
  `level` int(11) NOT NULL DEFAULT '0',
  `enhance` int(11) NOT NULL DEFAULT '0',
  `endurance` int(11) NOT NULL DEFAULT '0',
  `flag` int(11) NOT NULL DEFAULT '0',
  `gcode` int(11) NOT NULL DEFAULT '0',
  `wear_info` int(11) DEFAULT NULL,
  `socket_0` int(11) NOT NULL DEFAULT '0',
  `socket_1` int(11) NOT NULL DEFAULT '0',
  `socket_2` int(11) NOT NULL DEFAULT '0',
  `socket_3` int(11) NOT NULL DEFAULT '0',
  `socket_4` int(11) NOT NULL DEFAULT '0',
  `socket_5` int(11) NOT NULL DEFAULT '0',
  `remain_time` int(11) NOT NULL DEFAULT '0',
  `elemental_effect_type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `elemental_effect_expire_time` datetime NOT NULL,
  `elemental_effect_attack_point` int(11) NOT NULL DEFAULT '0',
  `elemental_effect_magic_point` int(11) NOT NULL DEFAULT '0',
  `create_time` datetime NOT NULL,
  `update_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`sid`),
  KEY `IDX_Item_Account_Id_Owner_Id_Auction_Id_Keeping_Id` (`account_id`,`owner_id`,`auction_id`,`keeping_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.ItemCoolTime
DROP TABLE IF EXISTS `ItemCoolTime`;
CREATE TABLE IF NOT EXISTS `ItemCoolTime` (
  `owner_id` int(11) NOT NULL DEFAULT '0',
  `cool_time_00` int(11) NOT NULL DEFAULT '0',
  `cool_time_01` int(11) NOT NULL DEFAULT '0',
  `cool_time_02` int(11) NOT NULL DEFAULT '0',
  `cool_time_03` int(11) NOT NULL DEFAULT '0',
  `cool_time_04` int(11) NOT NULL DEFAULT '0',
  `cool_time_05` int(11) NOT NULL DEFAULT '0',
  `cool_time_06` int(11) NOT NULL DEFAULT '0',
  `cool_time_07` int(11) NOT NULL DEFAULT '0',
  `cool_time_08` int(11) NOT NULL DEFAULT '0',
  `cool_time_09` int(11) NOT NULL DEFAULT '0',
  `cool_time_10` int(11) NOT NULL DEFAULT '0',
  `cool_time_11` int(11) NOT NULL DEFAULT '0',
  `cool_time_12` int(11) NOT NULL DEFAULT '0',
  `cool_time_13` int(11) NOT NULL DEFAULT '0',
  `cool_time_14` int(11) NOT NULL DEFAULT '0',
  `cool_time_15` int(11) NOT NULL DEFAULT '0',
  `cool_time_16` int(11) NOT NULL DEFAULT '0',
  `cool_time_17` int(11) NOT NULL DEFAULT '0',
  `cool_time_18` int(11) NOT NULL DEFAULT '0',
  `cool_time_19` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`owner_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.ItemKeeping
DROP TABLE IF EXISTS `ItemKeeping`;
CREATE TABLE IF NOT EXISTS `ItemKeeping` (
  `sid` int(11) NOT NULL DEFAULT '0',
  `item_id` bigint(20) NOT NULL,
  `owner_id` int(11) NOT NULL DEFAULT '0',
  `expiration_time` datetime NOT NULL,
  `keeping_type` int(11) NOT NULL DEFAULT '0',
  `related_auction_id` int(11) NOT NULL DEFAULT '0',
  `related_item_code` int(11) NOT NULL DEFAULT '0',
  `related_item_enhance` int(11) NOT NULL DEFAULT '0',
  `related_item_level` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`sid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.PaidItem
DROP TABLE IF EXISTS `PaidItem`;
CREATE TABLE IF NOT EXISTS `PaidItem` (
  `sid` int(11) NOT NULL AUTO_INCREMENT,
  `account_id` int(11) DEFAULT NULL,
  `avatar_id` int(11) DEFAULT NULL,
  `avatar_name` varchar(61) DEFAULT NULL,
  `item_code` int(11) DEFAULT NULL,
  `item_count` int(11) DEFAULT NULL,
  `rest_item_count` int(11) DEFAULT NULL,
  `bought_time` datetime DEFAULT NULL,
  `valid_time` datetime DEFAULT NULL,
  `server_name` varchar(30) DEFAULT NULL,
  `taken_avatar_id` int(11) DEFAULT NULL,
  `taken_avatar_name` varchar(61) DEFAULT NULL,
  `taken_server_name` varchar(30) DEFAULT NULL,
  `taken_time` datetime DEFAULT NULL,
  `taken_account_id` int(11) DEFAULT NULL,
  `confirmed` int(11) DEFAULT NULL,
  `confirmed_time` date DEFAULT NULL,
  `isCancel` tinyint(3) unsigned DEFAULT NULL,
  PRIMARY KEY (`sid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.Party
DROP TABLE IF EXISTS `Party`;
CREATE TABLE IF NOT EXISTS `Party` (
  `sid` int(11) NOT NULL DEFAULT '0',
  `name` varchar(128) NOT NULL,
  `leader_id` int(11) NOT NULL DEFAULT '0',
  `share_mode` int(11) NOT NULL DEFAULT '0',
  `attack_team` int(11) NOT NULL DEFAULT '0',
  `lead_party_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`sid`),
  UNIQUE KEY `UQ__Party__6FE99F9F` (`leader_id`),
  UNIQUE KEY `UQ__Party__6EF57B66` (`name`),
  UNIQUE KEY `IDX_Party_PartyName` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.Pet
DROP TABLE IF EXISTS `Pet`;
CREATE TABLE IF NOT EXISTS `Pet` (
  `sid` int(11) NOT NULL DEFAULT '0',
  `account_id` int(11) NOT NULL DEFAULT '0',
  `owner_id` int(11) NOT NULL DEFAULT '0',
  `cage_uid` bigint(20) NOT NULL,
  `code` int(11) NOT NULL DEFAULT '0',
  `name` varchar(18) NOT NULL,
  `name_changed` int(11) NOT NULL DEFAULT '0',
  `cool_time_01` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`sid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.Quest
DROP TABLE IF EXISTS `Quest`;
CREATE TABLE IF NOT EXISTS `Quest` (
  `owner_id` int(11) NOT NULL DEFAULT '0',
  `id` int(11) NOT NULL DEFAULT '0',
  `code` int(11) NOT NULL DEFAULT '0',
  `start_id` int(11) DEFAULT NULL,
  `status1` int(11) NOT NULL DEFAULT '0',
  `status2` int(11) NOT NULL DEFAULT '0',
  `status3` int(11) NOT NULL DEFAULT '0',
  `progress` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`owner_id`,`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.RandomQuestInfo
DROP TABLE IF EXISTS `RandomQuestInfo`;
CREATE TABLE IF NOT EXISTS `RandomQuestInfo` (
  `owner_id` int(11) NOT NULL DEFAULT '0',
  `code` int(11) NOT NULL DEFAULT '0',
  `key1` int(11) NOT NULL DEFAULT '0',
  `key2` int(11) NOT NULL DEFAULT '0',
  `key3` int(11) NOT NULL DEFAULT '0',
  `value1` int(11) NOT NULL DEFAULT '0',
  `value2` int(11) NOT NULL DEFAULT '0',
  `value3` int(11) NOT NULL DEFAULT '0',
  `is_dropped` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY (`owner_id`,`code`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.RankingInfo
DROP TABLE IF EXISTS `RankingInfo`;
CREATE TABLE IF NOT EXISTS `RankingInfo` (
  `ranking_id` int(11) NOT NULL DEFAULT '0',
  `next_settling_time` datetime NOT NULL,
  PRIMARY KEY (`ranking_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.RankingScore
DROP TABLE IF EXISTS `RankingScore`;
CREATE TABLE IF NOT EXISTS `RankingScore` (
  `ranking_id` int(11) NOT NULL DEFAULT '0',
  `owner_id` int(11) NOT NULL DEFAULT '0',
  `score` decimal(18,4) NOT NULL,
  `is_valid` tinyint(1) NOT NULL,
  PRIMARY KEY (`ranking_id`,`owner_id`),
  KEY `IDX_RankingScore_Ranking_Id` (`ranking_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.ScheduledCommand
DROP TABLE IF EXISTS `ScheduledCommand`;
CREATE TABLE IF NOT EXISTS `ScheduledCommand` (
  `sid` int(11) NOT NULL DEFAULT '0',
  `scheduled_time` datetime NOT NULL,
  `command` varchar(255) NOT NULL,
  `is_launched` char(1) NOT NULL,
  `launched_time` datetime NOT NULL DEFAULT '9999-12-31 00:00:00',
  PRIMARY KEY (`sid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.SecurityNo
DROP TABLE IF EXISTS `SecurityNo`;
CREATE TABLE IF NOT EXISTS `SecurityNo` (
  `account_id` int(11) NOT NULL DEFAULT '0',
  `security_no` varchar(31) NOT NULL,
  KEY `IDX_SecurityNo_Account_Id` (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.Skill
DROP TABLE IF EXISTS `Skill`;
CREATE TABLE IF NOT EXISTS `Skill` (
  `sid` int(11) NOT NULL DEFAULT '0',
  `owner_id` int(11) NOT NULL DEFAULT '0',
  `summon_id` int(11) NOT NULL DEFAULT '0',
  `skill_id` int(11) NOT NULL DEFAULT '0',
  `skill_level` int(11) NOT NULL DEFAULT '0',
  `cool_time` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`sid`),
  KEY `IDX_Skill_Owner_Id_Summon_Id` (`owner_id`,`summon_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.State
DROP TABLE IF EXISTS `State`;
CREATE TABLE IF NOT EXISTS `State` (
  `sid` int(11) NOT NULL AUTO_INCREMENT,
  `owner_id` int(11) NOT NULL DEFAULT '0',
  `summon_id` int(11) NOT NULL DEFAULT '0',
  `code` int(11) NOT NULL DEFAULT '0',
  `level_1` smallint(6) NOT NULL,
  `level_2` smallint(6) NOT NULL,
  `level_3` smallint(6) NOT NULL,
  `duration_1` int(11) NOT NULL DEFAULT '0',
  `duration_2` int(11) NOT NULL DEFAULT '0',
  `duration_3` int(11) NOT NULL DEFAULT '0',
  `remain_time_1` int(11) NOT NULL DEFAULT '0',
  `remain_time_2` int(11) NOT NULL DEFAULT '0',
  `remain_time_3` int(11) NOT NULL DEFAULT '0',
  `base_damage_1` int(11) NOT NULL DEFAULT '0',
  `base_damage_2` int(11) NOT NULL DEFAULT '0',
  `base_damage_3` int(11) NOT NULL DEFAULT '0',
  `remain_fire_time` int(11) NOT NULL DEFAULT '0',
  `state_value` int(11) NOT NULL DEFAULT '0',
  `state_string_value` varchar(32) NOT NULL,
  `enable` int(11) NOT NULL DEFAULT '1',
  PRIMARY KEY (`sid`),
  KEY `IX_State` (`owner_id`,`summon_id`)
) ENGINE=InnoDB AUTO_INCREMENT=16 DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.Summon
DROP TABLE IF EXISTS `Summon`;
CREATE TABLE IF NOT EXISTS `Summon` (
  `sid` int(11) NOT NULL DEFAULT '0',
  `account_id` int(11) NOT NULL DEFAULT '0',
  `owner_id` int(11) NOT NULL DEFAULT '0',
  `code` int(11) NOT NULL DEFAULT '0',
  `card_uid` bigint(20) NOT NULL,
  `exp` bigint(20) NOT NULL,
  `jp` int(11) NOT NULL DEFAULT '0',
  `last_decreased_exp` bigint(20) NOT NULL DEFAULT '0',
  `name` varchar(64) NOT NULL,
  `transform` int(11) NOT NULL DEFAULT '0',
  `lv` int(11) NOT NULL DEFAULT '0',
  `jlv` int(11) NOT NULL DEFAULT '0',
  `max_level` int(11) NOT NULL DEFAULT '0',
  `fp` int(11) NOT NULL DEFAULT '0',
  `prev_level_01` int(11) NOT NULL DEFAULT '0',
  `prev_level_02` int(11) NOT NULL DEFAULT '0',
  `prev_id_01` int(11) NOT NULL DEFAULT '0',
  `prev_id_02` int(11) NOT NULL DEFAULT '0',
  `sp` int(11) NOT NULL DEFAULT '0',
  `hp` int(11) NOT NULL DEFAULT '0',
  `mp` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`sid`),
  KEY `IDX_Summon_Account_Id` (`account_id`),
  KEY `IDX_Summon_Card_Uid` (`card_uid`),
  KEY `IDX_Summon_Owner_Id` (`owner_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.TBLSTATS
DROP TABLE IF EXISTS `TBLSTATS`;
CREATE TABLE IF NOT EXISTS `TBLSTATS` (
  `OwnerName` varchar(160) NOT NULL,
  `TableName` varchar(160) NOT NULL,
  `IndexName` varchar(160) NOT NULL DEFAULT 'ALL',
  `CurrentValue` tinyint(3) unsigned DEFAULT NULL,
  `OldValue` tinyint(3) unsigned DEFAULT '0',
  `LastModDate` timestamp NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.tb_character_job_lv_log
DROP TABLE IF EXISTS `tb_character_job_lv_log`;
CREATE TABLE IF NOT EXISTS `tb_character_job_lv_log` (
  `log_date` varchar(10) NOT NULL,
  `job` int(11) NOT NULL DEFAULT '0',
  `lv` int(11) NOT NULL DEFAULT '0',
  `total_cnt` int(11) NOT NULL DEFAULT '0',
  `cnt` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.tb_event_333
DROP TABLE IF EXISTS `tb_event_333`;
CREATE TABLE IF NOT EXISTS `tb_event_333` (
  `sid` int(11) NOT NULL AUTO_INCREMENT,
  `code` int(11) NOT NULL DEFAULT '0',
  `character_id` int(11) DEFAULT NULL,
  `name` varchar(50) DEFAULT NULL,
  `account_id` int(11) DEFAULT NULL,
  `account` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`sid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.tb_temp_0504
DROP TABLE IF EXISTS `tb_temp_0504`;
CREATE TABLE IF NOT EXISTS `tb_temp_0504` (
  `account_id` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.TRIG_Character_BankGold
DROP TABLE IF EXISTS `TRIG_Character_BankGold`;
CREATE TABLE IF NOT EXISTS `TRIG_Character_BankGold` (
  `log_date` datetime NOT NULL,
  `SESSION` varchar(30) NOT NULL,
  `SYS_USER` varchar(30) NOT NULL,
  `APL_NAME` varchar(60) NOT NULL,
  `HOST_NAME` varchar(30) DEFAULT NULL,
  `HOST_ID` varchar(30) NOT NULL,
  `account_id` int(11) NOT NULL DEFAULT '0',
  `Prev_Bank_gold` bigint(20) NOT NULL,
  `Bank_gold` bigint(20) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Telecaster.TRIG_Character_Gold
DROP TABLE IF EXISTS `TRIG_Character_Gold`;
CREATE TABLE IF NOT EXISTS `TRIG_Character_Gold` (
  `log_date` datetime NOT NULL,
  `SESSION` varchar(30) NOT NULL,
  `SYS_USER` varchar(30) NOT NULL,
  `APL_NAME` varchar(60) NOT NULL,
  `HOST_NAME` varchar(30) NOT NULL,
  `HOST_ID` varchar(30) DEFAULT NULL,
  `account_id` int(11) NOT NULL DEFAULT '0',
  `account` varchar(50) NOT NULL,
  `name` varchar(50) NOT NULL,
  `Prev_gold` bigint(20) NOT NULL,
  `gold` bigint(20) NOT NULL,
  `Login_time` datetime NOT NULL,
  `Logout_time` datetime NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
