#ifndef PROJECT_SUMMONBASE_H
#define PROJECT_SUMMONBASE_H

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

struct SummonResourceTemplate {
    int   id;
    int   type;
    int   magic_type;
    int   rate;
    int   stat_id;
    float size;
    float scale;
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

#endif // PROJECT_SUMMONBASE_H
