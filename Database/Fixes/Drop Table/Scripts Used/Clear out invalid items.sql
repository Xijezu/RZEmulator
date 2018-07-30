-- START TRANSACTION;

SELECT * FROM DropGroupResource
-- UPDATE DropGroupResource
-- SET drop_item_id_00 = 0
WHERE drop_item_id_00 NOT IN (SELECT id FROM ItemResource)
AND drop_item_id_00 > 0;

SELECT * FROM DropGroupResource
-- UPDATE DropGroupResource
-- SET drop_item_id_01 = 0
WHERE drop_item_id_01 NOT IN (SELECT id FROM ItemResource)
AND drop_item_id_01 > 0;

SELECT * FROM DropGroupResource
-- UPDATE DropGroupResource
-- SET drop_item_id_02 = 0
WHERE drop_item_id_02 NOT IN (SELECT id FROM ItemResource)
AND drop_item_id_02 > 0;

SELECT * FROM DropGroupResource
-- UPDATE DropGroupResource
-- SET drop_item_id_03 = 0
WHERE drop_item_id_03 NOT IN (SELECT id FROM ItemResource)
AND drop_item_id_03 > 0;

SELECT * FROM DropGroupResource
-- UPDATE DropGroupResource
-- SET drop_item_id_04 = 0
WHERE drop_item_id_04 NOT IN (SELECT id FROM ItemResource)
AND drop_item_id_04 > 0;

SELECT * FROM DropGroupResource
-- UPDATE DropGroupResource
-- SET drop_item_id_05 = 0
WHERE drop_item_id_05 NOT IN (SELECT id FROM ItemResource)
AND drop_item_id_05 > 0;

SELECT * FROM DropGroupResource
-- UPDATE DropGroupResource
-- SET drop_item_id_06 = 0
WHERE drop_item_id_06 NOT IN (SELECT id FROM ItemResource)
AND drop_item_id_06 > 0;

SELECT * FROM DropGroupResource
-- UPDATE DropGroupResource
-- SET drop_item_id_07 = 0
WHERE drop_item_id_07 NOT IN (SELECT id FROM ItemResource)
AND drop_item_id_07 > 0;

SELECT * FROM DropGroupResource
-- UPDATE DropGroupResource
-- SET drop_item_id_08 = 0
WHERE drop_item_id_08 NOT IN (SELECT id FROM ItemResource)
AND drop_item_id_08 > 0;

SELECT * FROM DropGroupResource
-- UPDATE DropGroupResource
-- SET drop_item_id_09 = 0
WHERE drop_item_id_09 NOT IN (SELECT id FROM ItemResource)
AND drop_item_id_09 > 0;

-- COMMIT;