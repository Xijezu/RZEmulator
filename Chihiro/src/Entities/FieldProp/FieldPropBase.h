#ifndef PROJECT_FIELDPROPBASE_H
#define PROJECT_FIELDPROPBASE_H

#include "Common.h"

 enum FP_LimitFlag : int
 {
     FPLF_Deva     = 0x4,
     FPLF_Asura    = 0x8,
     FPLF_Gaia     = 0x10,
     FPLF_Fighter  = 0x400,
     FPLF_Hunter   = 0x800,
     FPLF_Magician = 0x1000,
     FPLF_Summoner = 0x2000,
 };

enum ActivateCheckType : int
{
    FP_Item                = 1,
    FP_Quest               = 2,
    FP_Skill               = 3,
    FP_Wear                = 4,
    FP_Summon              = 5,
    FP_Prop                = 6,
    FP_Member              = 7,
    FP_ExistNearMob        = 11,
    FP_NonExistNearMob     = 12,
    FP_OwnTacticalPosition = 13,
};

struct DropItemInfo
{
    int code;                                        // 0x0
    int ratio;                                       // 0x4
    int min_count;                                   // 0x8
    int max_count;                                   // 0xC
    int min_level;                                   // 0x10
    int max_level;                                   // 0x14
};

struct FieldPropRespawnInfo
{
    FieldPropRespawnInfo() = default;
    FieldPropRespawnInfo(const FieldPropRespawnInfo& _src)
    {
     nPropID     = _src.nPropID;
     x           = _src.x;
     y           = _src.y;
     layer       = _src.layer;
     fZOffset    = _src.fZOffset;
     fRotateX    = _src.fRotateX;
     fRotateY    = _src.fRotateY;
     fRotateZ    = _src.fRotateZ;
     fScaleX     = _src.fScaleX;
     fScaleY     = _src.fScaleY;
     fScaleZ     = _src.fScaleZ;
     bLockHeight = _src.bLockHeight;
     fLockHeight = _src.fLockHeight;
     bOnce       = _src.bOnce;
    }

    int   nPropID;
    float x;
    float y;
    uint8 layer;
    float fZOffset;
    float fRotateX;
    float fRotateY;
    float fRotateZ;
    float fScaleX;
    float fScaleY;
    float fScaleZ;
    bool  bLockHeight;
    float fLockHeight;
    bool  bOnce;
};

struct FieldPropTemplate
{
    uint         nPropID;
    int          nPropTextID;
    int          nType;
    int          nLocalFlag;
    uint         nCastingTime;
    int          nUseCount;
    uint         nRegenTime;
    uint         nLifeTime;
    int          nMinLevel;
    int          nMaxLevel;
    int          nLimit;
    int          nLimitJobID;
    int          nActivateID[2];
    int          nActivateValue[2][2];
    int          nActivateSkillID;
    DropItemInfo drop_info[2];
    std::string  strScript;
};

#endif // PROJECT_FIELDPROPBASE_H
