UPDATE MixResource
SET mix_value_02 = 1,
	mix_value_03 = 1
WHERE mix_type = 101;

UPDATE EnhanceResource
SET fail_result = 1,
	max_enhance = 20
WHERE enhance_id IN  (SELECT mix_value_01 FROM MixResource WHERE main_type_01 = 1 AND main_value_01 < 9);

UPDATE EnhanceResource
SET enhance_type = 1,
	fail_result = 2,
	max_enhance = 10
WHERE enhance_id IN  (SELECT mix_value_01 FROM MixResource WHERE mix_type = 102);

UPDATE EnhanceResource
SET fail_result = 3,
	max_enhance = 10
WHERE enhance_id IN  (SELECT mix_value_01 FROM MixResource WHERE main_type_01 = 1 AND main_value_01 = 9);