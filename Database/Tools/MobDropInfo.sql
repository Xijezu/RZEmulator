SET @MobID = 9110015;

-- ************************************************************************
-- NO TOUCHY
-- ************************************************************************

SELECT MobDropInfo.id,
		 MobDropInfo.monster_group,
		 (SELECT `value` FROM StringResource WHERE `code` = MobDropInfo.name_id) AS `name`,
		 (SELECT `value` FROM StringResource WHERE `code` = MobDropInfo.location_id) AS `location`,
		 MobDropInfo.model,
		 MobDropInfo.`level`,
		 MobDropInfo.grp,
		 MobDropInfo.race,
		 MobDropInfo.drop_id,
		 (SELECT `value` FROM StringResource WHERE `code` = item.name_id) AS `item`,
		 MobDropInfo.drop_chance,
		 MobDropInfo.item_min_count,
		 MobDropInfo.item_max_count,
		 MobDropInfo.item_min_level,
		 MobDropInfo.item_max_level
FROM (
	SELECT Mob.id, Mob.monster_group, Mob.name_id, Mob.location_id, Mob.model, Mob.`level`, Mob.grp, Mob.race, Mob.drop_item_id_00 AS drop_id, DropGroup.drop_item_id AS item_id, Mob.drop_percentage_00 AS drop_chance, Mob.drop_min_count_00 AS item_min_count, Mob.drop_max_count_00 AS item_max_count, Mob.drop_min_level_00 AS item_min_level, Mob.drop_max_level_00 AS item_max_level
	FROM MonsterResource AS Mob
	INNER JOIN
	(
		SELECT id, drop_item_id FROM DropGroupResource_NORM WHERE drop_item_id > 0 AND id NOT IN (SELECT drop_item_id FROM DropGroupResource_NORM)
		UNION ALL
		SELECT d1.id, d2.drop_item_id FROM DropGroupResource_NORM d1
		INNER JOIN DropGroupResource_NORM d2 ON d2.id = d1.drop_item_id
	) AS DropGroup ON DropGroup.id = Mob.drop_item_id_00
	UNION ALL
	SELECT Mob.id, Mob.monster_group, Mob.name_id, Mob.location_id, Mob.model, Mob.`level`, Mob.grp, Mob.race, Mob.drop_item_id_01 AS drop_id, DropGroup.drop_item_id AS item_id, Mob.drop_percentage_01 AS drop_chance, Mob.drop_min_count_01 AS item_min_count, Mob.drop_max_count_01 AS item_max_count, Mob.drop_min_level_01 AS item_min_level, Mob.drop_max_level_01 AS item_max_level
	FROM MonsterResource AS Mob
	INNER JOIN
	(
		SELECT id, drop_item_id FROM DropGroupResource_NORM WHERE drop_item_id > 0 AND id NOT IN (SELECT drop_item_id FROM DropGroupResource_NORM)
		UNION ALL
		SELECT d1.id, d2.drop_item_id FROM DropGroupResource_NORM d1
		INNER JOIN DropGroupResource_NORM d2 ON d2.id = d1.drop_item_id
	) AS DropGroup ON DropGroup.id = Mob.drop_item_id_01
	UNION ALL
	SELECT Mob.id, Mob.monster_group, Mob.name_id, Mob.location_id, Mob.model, Mob.`level`, Mob.grp, Mob.race, Mob.drop_item_id_02 AS drop_id, DropGroup.drop_item_id AS item_id, Mob.drop_percentage_02 AS drop_chance, Mob.drop_min_count_02 AS item_min_count, Mob.drop_max_count_02 AS item_max_count, Mob.drop_min_level_02 AS item_min_level, Mob.drop_max_level_02 AS item_max_level
	FROM MonsterResource AS Mob
	INNER JOIN
	(
		SELECT id, drop_item_id FROM DropGroupResource_NORM WHERE drop_item_id > 0 AND id NOT IN (SELECT drop_item_id FROM DropGroupResource_NORM)
		UNION ALL
		SELECT d1.id, d2.drop_item_id FROM DropGroupResource_NORM d1
		INNER JOIN DropGroupResource_NORM d2 ON d2.id = d1.drop_item_id
	) AS DropGroup ON DropGroup.id = Mob.drop_item_id_02
	UNION ALL
	SELECT Mob.id, Mob.monster_group, Mob.name_id, Mob.location_id, Mob.model, Mob.`level`, Mob.grp, Mob.race, Mob.drop_item_id_03 AS drop_id, DropGroup.drop_item_id AS item_id, Mob.drop_percentage_03 AS drop_chance, Mob.drop_min_count_03 AS item_min_count, Mob.drop_max_count_03 AS item_max_count, Mob.drop_min_level_03 AS item_min_level, Mob.drop_max_level_03 AS item_max_level
	FROM MonsterResource AS Mob
	INNER JOIN
	(
		SELECT id, drop_item_id FROM DropGroupResource_NORM WHERE drop_item_id > 0 AND id NOT IN (SELECT drop_item_id FROM DropGroupResource_NORM)
		UNION ALL
		SELECT d1.id, d2.drop_item_id FROM DropGroupResource_NORM d1
		INNER JOIN DropGroupResource_NORM d2 ON d2.id = d1.drop_item_id
	) AS DropGroup ON DropGroup.id = Mob.drop_item_id_03
	UNION ALL
	SELECT Mob.id, Mob.monster_group, Mob.name_id, Mob.location_id, Mob.model, Mob.`level`, Mob.grp, Mob.race, Mob.drop_item_id_04 AS drop_id, DropGroup.drop_item_id AS item_id, Mob.drop_percentage_04 AS drop_chance, Mob.drop_min_count_04 AS item_min_count, Mob.drop_max_count_04 AS item_max_count, Mob.drop_min_level_04 AS item_min_level, Mob.drop_max_level_04 AS item_max_level
	FROM MonsterResource AS Mob
	INNER JOIN
	(
		SELECT id, drop_item_id FROM DropGroupResource_NORM WHERE drop_item_id > 0 AND id NOT IN (SELECT drop_item_id FROM DropGroupResource_NORM)
		UNION ALL
		SELECT d1.id, d2.drop_item_id FROM DropGroupResource_NORM d1
		INNER JOIN DropGroupResource_NORM d2 ON d2.id = d1.drop_item_id
	) AS DropGroup ON DropGroup.id = Mob.drop_item_id_04
	UNION ALL
	SELECT Mob.id, Mob.monster_group, Mob.name_id, Mob.location_id, Mob.model, Mob.`level`, Mob.grp, Mob.race, Mob.drop_item_id_05 AS drop_id, DropGroup.drop_item_id AS item_id, Mob.drop_percentage_05 AS drop_chance, Mob.drop_min_count_05 AS item_min_count, Mob.drop_max_count_05 AS item_max_count, Mob.drop_min_level_05 AS item_min_level, Mob.drop_max_level_05 AS item_max_level
	FROM MonsterResource AS Mob
	INNER JOIN
	(
		SELECT id, drop_item_id FROM DropGroupResource_NORM WHERE drop_item_id > 0 AND id NOT IN (SELECT drop_item_id FROM DropGroupResource_NORM)
		UNION ALL
		SELECT d1.id, d2.drop_item_id FROM DropGroupResource_NORM d1
		INNER JOIN DropGroupResource_NORM d2 ON d2.id = d1.drop_item_id
	) AS DropGroup ON DropGroup.id = Mob.drop_item_id_05
	UNION ALL
	SELECT Mob.id, Mob.monster_group, Mob.name_id, Mob.location_id, Mob.model, Mob.`level`, Mob.grp, Mob.race, Mob.drop_item_id_06 AS drop_id, DropGroup.drop_item_id AS item_id, Mob.drop_percentage_06 AS drop_chance, Mob.drop_min_count_06 AS item_min_count, Mob.drop_max_count_06 AS item_max_count, Mob.drop_min_level_06 AS item_min_level, Mob.drop_max_level_06 AS item_max_level
	FROM MonsterResource AS Mob
	INNER JOIN
	(
		SELECT id, drop_item_id FROM DropGroupResource_NORM WHERE drop_item_id > 0 AND id NOT IN (SELECT drop_item_id FROM DropGroupResource_NORM)
		UNION ALL
		SELECT d1.id, d2.drop_item_id FROM DropGroupResource_NORM d1
		INNER JOIN DropGroupResource_NORM d2 ON d2.id = d1.drop_item_id
	) AS DropGroup ON DropGroup.id = Mob.drop_item_id_06
	UNION ALL
	SELECT Mob.id, Mob.monster_group, Mob.name_id, Mob.location_id, Mob.model, Mob.`level`, Mob.grp, Mob.race, Mob.drop_item_id_07 AS drop_id, DropGroup.drop_item_id AS item_id, Mob.drop_percentage_07 AS drop_chance, Mob.drop_min_count_07 AS item_min_count, Mob.drop_max_count_07 AS item_max_count, Mob.drop_min_level_07 AS item_min_level, Mob.drop_max_level_07 AS item_max_level
	FROM MonsterResource AS Mob
	INNER JOIN
	(
		SELECT id, drop_item_id FROM DropGroupResource_NORM WHERE drop_item_id > 0 AND id NOT IN (SELECT drop_item_id FROM DropGroupResource_NORM)
		UNION ALL
		SELECT d1.id, d2.drop_item_id FROM DropGroupResource_NORM d1
		INNER JOIN DropGroupResource_NORM d2 ON d2.id = d1.drop_item_id
	) AS DropGroup ON DropGroup.id = Mob.drop_item_id_07
	UNION ALL
	SELECT Mob.id, Mob.monster_group, Mob.name_id, Mob.location_id, Mob.model, Mob.`level`, Mob.grp, Mob.race, Mob.drop_item_id_08 AS drop_id, DropGroup.drop_item_id AS item_id, Mob.drop_percentage_08 AS drop_chance, Mob.drop_min_count_08 AS item_min_count, Mob.drop_max_count_08 AS item_max_count, Mob.drop_min_level_08 AS item_min_level, Mob.drop_max_level_08 AS item_max_level
	FROM MonsterResource AS Mob
	INNER JOIN
	(
		SELECT id, drop_item_id FROM DropGroupResource_NORM WHERE drop_item_id > 0 AND id NOT IN (SELECT drop_item_id FROM DropGroupResource_NORM)
		UNION ALL
		SELECT d1.id, d2.drop_item_id FROM DropGroupResource_NORM d1
		INNER JOIN DropGroupResource_NORM d2 ON d2.id = d1.drop_item_id
	) AS DropGroup ON DropGroup.id = Mob.drop_item_id_08
	UNION ALL
	SELECT Mob.id, Mob.monster_group, Mob.name_id, Mob.location_id, Mob.model, Mob.`level`, Mob.grp, Mob.race, Mob.drop_item_id_09 AS drop_id, DropGroup.drop_item_id AS item_id, Mob.drop_percentage_09 AS drop_chance, Mob.drop_min_count_09 AS item_min_count, Mob.drop_max_count_09 AS item_max_count, Mob.drop_min_level_09 AS item_min_level, Mob.drop_max_level_09 AS item_max_level
	FROM MonsterResource AS Mob
	INNER JOIN
	(
		SELECT id, drop_item_id FROM DropGroupResource_NORM WHERE drop_item_id > 0 AND id NOT IN (SELECT drop_item_id FROM DropGroupResource_NORM)
		UNION ALL
		SELECT d1.id, d2.drop_item_id FROM DropGroupResource_NORM d1
		INNER JOIN DropGroupResource_NORM d2 ON d2.id = d1.drop_item_id
	) AS DropGroup ON DropGroup.id = Mob.drop_item_id_09
) AS MobDropInfo
INNER JOIN ItemResource AS item ON item_id = item.id
WHERE MobDropInfo.id = @MobID;