//
// Created by xijezu on 17.12.17.
//

#ifndef PROJECT_DATABASETEMPLATES_H
#define PROJECT_DATABASETEMPLATES_H

#define MAX_DROP_GROUP 10

struct SummonLevelBonus
{
    int summon_id;
    float strength;
    float vital;
    float dexterity;
    float agility;
    float intelligence;
    float mentality;
    float luck;
};

struct DropGroup {
    int uid;
    int drop_item_id[MAX_DROP_GROUP];
    float drop_percentage[MAX_DROP_GROUP];
};


struct LevelResourceTemplate {
    int level{};
    int64 normal_exp{};
    int jlv[4]{};
};

struct ItemTemplate {
    int32       id;
    int32       name_id;
    int32       type;
    int32       group;
    int32       iclass;
    int32       wear_type;
    int32       set_id;
    int32       set_part_flag;
    int32       rank;
    int32       level;
    int32       enhance;
    int32       socket;
    int32       status_flag;
    int32       limit_deva;
    int32       limit_asura;
    int32       limit_gaia;
    int32       limit_fighter;
    int32       limit_hunter;
    int32       limit_magician;
    int32       limit_summoner;
    int32       use_min_level;
    int32       use_max_level;
    int32       target_min_level;
    int32       target_max_level;
    float       range;
    float       weight;
    uint32      price;
    int32       endurance;
    int32       material;
    int32       summon_id;
    int8_t      flaglist[19]{0};
    int32       available_period;
    int16       decrease_type;
    float       throw_range;
    int8_t      distribute_type;
    int16       base_type[4]{0};
    float       base_var[4][2]{0};
    int16       opt_type[4]{0};
    float       opt_var[4][2]{0};
    int16       enhance_id[2]{0};
    float       _enhance[2][4]{0};
    int32       skill_id;
    int32       state_id;
    int32       state_level;
    int32       state_time;
    int32       state_type;
    int32       cool_time;
    int16       cool_time_group;
    std::string script_text;
};

struct JobLevelBonusTemplate {
    int   job_id;
    float strength[4];
    float vital[4];
    float dexterity[4];
    float agility[4];
    float intelligence[4];
    float mentality[4];
    float luck[4];
};

struct JobResourceTemplate {
    int id;
    uint stat_id;
    int job_class;
    uint job_depth;
    int up_lv;
    int up_jlv;
    int available_job[4];
};

struct MarketInfo {
    int         sort_id;
    std::string name;
    uint         code;
    float       price_ratio;
    float       huntaholic_ratio;
};
struct SummonResourceTemplate {
    int   id;
    int   type;
    int   magic_type;
    int   rate;
    int   stat_id;
    int   standard_walk_speed;
    int   standard_run_speed;
    int   walk_speed;
    int   run_speed;
    bool  is_riding_only;
    float attack_range;
    int   material;
    int   weapon_type;
    int   form;
    int   evolve_target;
    int   card_id;
};
#endif //PROJECT_DATABASETEMPLATES_H
