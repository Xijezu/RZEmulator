START TRANSACTION;

-- SELECT * FROM DropGroupResource
UPDATE DropGroupResource
SET drop_item_id_00 = 0
WHERE drop_item_id_00 IN (
	SELECT id FROM (SELECT * FROM DropGroupResource) AS DropGroup
	WHERE drop_item_id_00 = 0
	AND drop_item_id_01 = 0
	AND drop_item_id_02 = 0
	AND drop_item_id_03 = 0
	AND drop_item_id_04 = 0
	AND drop_item_id_05 = 0
	AND drop_item_id_06 = 0
	AND drop_item_id_07 = 0
	AND drop_item_id_08 = 0
	AND drop_item_id_09 = 0
);

-- SELECT * FROM DropGroupResource
UPDATE DropGroupResource
SET drop_item_id_01 = 0
WHERE drop_item_id_01 IN (
	SELECT id FROM (SELECT * FROM DropGroupResource) AS DropGroup
	WHERE drop_item_id_00 = 0
	AND drop_item_id_01 = 0
	AND drop_item_id_02 = 0
	AND drop_item_id_03 = 0
	AND drop_item_id_04 = 0
	AND drop_item_id_05 = 0
	AND drop_item_id_06 = 0
	AND drop_item_id_07 = 0
	AND drop_item_id_08 = 0
	AND drop_item_id_09 = 0
);

-- SELECT * FROM DropGroupResource
UPDATE DropGroupResource
SET drop_item_id_02 = 0
WHERE drop_item_id_02 IN (
	SELECT id FROM (SELECT * FROM DropGroupResource) AS DropGroup
	WHERE drop_item_id_00 = 0
	AND drop_item_id_01 = 0
	AND drop_item_id_02 = 0
	AND drop_item_id_03 = 0
	AND drop_item_id_04 = 0
	AND drop_item_id_05 = 0
	AND drop_item_id_06 = 0
	AND drop_item_id_07 = 0
	AND drop_item_id_08 = 0
	AND drop_item_id_09 = 0
);

-- SELECT * FROM DropGroupResource
UPDATE DropGroupResource
SET drop_item_id_03 = 0
WHERE drop_item_id_03 IN (
	SELECT id FROM (SELECT * FROM DropGroupResource) AS DropGroup
	WHERE drop_item_id_00 = 0
	AND drop_item_id_01 = 0
	AND drop_item_id_02 = 0
	AND drop_item_id_03 = 0
	AND drop_item_id_04 = 0
	AND drop_item_id_05 = 0
	AND drop_item_id_06 = 0
	AND drop_item_id_07 = 0
	AND drop_item_id_08 = 0
	AND drop_item_id_09 = 0
);

-- SELECT * FROM DropGroupResource
UPDATE DropGroupResource
SET drop_item_id_04 = 0
WHERE drop_item_id_04 IN (
	SELECT id FROM (SELECT * FROM DropGroupResource) AS DropGroup
	WHERE drop_item_id_00 = 0
	AND drop_item_id_01 = 0
	AND drop_item_id_02 = 0
	AND drop_item_id_03 = 0
	AND drop_item_id_04 = 0
	AND drop_item_id_05 = 0
	AND drop_item_id_06 = 0
	AND drop_item_id_07 = 0
	AND drop_item_id_08 = 0
	AND drop_item_id_09 = 0
);

-- SELECT * FROM DropGroupResource
UPDATE DropGroupResource
SET drop_item_id_05 = 0
WHERE drop_item_id_05 IN (
	SELECT id FROM (SELECT * FROM DropGroupResource) AS DropGroup
	WHERE drop_item_id_00 = 0
	AND drop_item_id_01 = 0
	AND drop_item_id_02 = 0
	AND drop_item_id_03 = 0
	AND drop_item_id_04 = 0
	AND drop_item_id_05 = 0
	AND drop_item_id_06 = 0
	AND drop_item_id_07 = 0
	AND drop_item_id_08 = 0
	AND drop_item_id_09 = 0
);

-- SELECT * FROM DropGroupResource
UPDATE DropGroupResource
SET drop_item_id_06 = 0
WHERE drop_item_id_06 IN (
	SELECT id FROM (SELECT * FROM DropGroupResource) AS DropGroup
	WHERE drop_item_id_00 = 0
	AND drop_item_id_01 = 0
	AND drop_item_id_02 = 0
	AND drop_item_id_03 = 0
	AND drop_item_id_04 = 0
	AND drop_item_id_05 = 0
	AND drop_item_id_06 = 0
	AND drop_item_id_07 = 0
	AND drop_item_id_08 = 0
	AND drop_item_id_09 = 0
);

-- SELECT * FROM DropGroupResource
UPDATE DropGroupResource
SET drop_item_id_07 = 0
WHERE drop_item_id_07 IN (
	SELECT id FROM (SELECT * FROM DropGroupResource) AS DropGroup
	WHERE drop_item_id_00 = 0
	AND drop_item_id_01 = 0
	AND drop_item_id_02 = 0
	AND drop_item_id_03 = 0
	AND drop_item_id_04 = 0
	AND drop_item_id_05 = 0
	AND drop_item_id_06 = 0
	AND drop_item_id_07 = 0
	AND drop_item_id_08 = 0
	AND drop_item_id_09 = 0
);

-- SELECT * FROM DropGroupResource
UPDATE DropGroupResource
SET drop_item_id_08 = 0
WHERE drop_item_id_08 IN (
	SELECT id FROM (SELECT * FROM DropGroupResource) AS DropGroup
	WHERE drop_item_id_00 = 0
	AND drop_item_id_01 = 0
	AND drop_item_id_02 = 0
	AND drop_item_id_03 = 0
	AND drop_item_id_04 = 0
	AND drop_item_id_05 = 0
	AND drop_item_id_06 = 0
	AND drop_item_id_07 = 0
	AND drop_item_id_08 = 0
	AND drop_item_id_09 = 0
);

-- SELECT * FROM DropGroupResource
UPDATE DropGroupResource
SET drop_item_id_09 = 0
WHERE drop_item_id_09 IN (
	SELECT id FROM (SELECT * FROM DropGroupResource) AS DropGroup
	WHERE drop_item_id_00 = 0
	AND drop_item_id_01 = 0
	AND drop_item_id_02 = 0
	AND drop_item_id_03 = 0
	AND drop_item_id_04 = 0
	AND drop_item_id_05 = 0
	AND drop_item_id_06 = 0
	AND drop_item_id_07 = 0
	AND drop_item_id_08 = 0
	AND drop_item_id_09 = 0
);

-- COMMIT;