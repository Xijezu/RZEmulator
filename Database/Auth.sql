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

-- Exportiere Daten aus Tabelle Auth.Accounts: ~2 rows (ungefähr)
/*!40000 ALTER TABLE `Accounts` DISABLE KEYS */;
INSERT INTO `Accounts` (`account_id`, `login_name`, `password`, `block`, `withdraw_remain_time`, `age`, `auth_ok`, `pcbang`, `last_login_server_idx`, `event_code`, `server_list_mask`, `result`, `ip`, `game_code`) VALUES
	(1, 'xijezu', 'c9c8b57bdc080a7847f1cffa3fd6e9a0', 0, 0, 0, 1, 0, 0, 0, '0', 0, 0, '0'),
	(1337, 'xijtest', 'c9c8b57bdc080a7847f1cffa3fd6e9a0', 0, 0, 23, 1, 0, 0, 0, '0', 0, 0, '0');
/*!40000 ALTER TABLE `Accounts` ENABLE KEYS */;

-- Exportiere Daten aus Tabelle Auth.IPLog: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `IPLog` DISABLE KEYS */;
/*!40000 ALTER TABLE `IPLog` ENABLE KEYS */;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
