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


-- Exportiere Datenbank Struktur für Auth
DROP DATABASE IF EXISTS `Auth`;
CREATE DATABASE IF NOT EXISTS `Auth` /*!40100 DEFAULT CHARACTER SET utf8mb4 */;
USE `Auth`;

-- Exportiere Struktur von Tabelle Auth.Accounts
DROP TABLE IF EXISTS `Accounts`;
CREATE TABLE IF NOT EXISTS `Accounts` (
  `account_id` int(11) NOT NULL,
  `login_name` varchar(31) DEFAULT NULL,
  `password` varchar(32) DEFAULT NULL,
  `block` int(11) DEFAULT NULL,
  `withdraw_remain_time` int(11) DEFAULT NULL,
  `age` int(11) DEFAULT NULL,
  `auth_ok` int(11) DEFAULT NULL,
  `pcbang` int(11) DEFAULT NULL,
  `last_login_server_idx` int(11) DEFAULT NULL,
  `event_code` int(11) DEFAULT NULL,
  `server_list_mask` varchar(31) DEFAULT NULL,
  `result` int(11) DEFAULT NULL,
  `ip` int(11) DEFAULT NULL,
  `game_code` varchar(50) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
-- Exportiere Struktur von Tabelle Auth.IPLog
DROP TABLE IF EXISTS `IPLog`;
CREATE TABLE IF NOT EXISTS `IPLog` (
  `account_id` int(11) DEFAULT NULL,
  `account_name` varchar(32) NOT NULL,
  `ip` varchar(50) NOT NULL,
  `login_date` date NOT NULL,
  `login_time` time NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Daten Export vom Benutzer nicht ausgewählt
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
