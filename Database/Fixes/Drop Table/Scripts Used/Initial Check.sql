SELECT * FROM DropGroupResource_NORM
WHERE drop_item_id NOT IN (SELECT id FROM ItemResource)
AND drop_item_id > 0