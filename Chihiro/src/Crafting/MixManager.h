#ifndef PROJECT_MIXMANAGER_H
#define PROJECT_MIXMANAGER_H

#include "Common.h"

#define MATERIAL_INFO_COUNT 5
#define MIX_VALUE_COUNT 6
#define MAX_SUB_MATERIAL_COUNT 9

enum MIX_TYPE
{
    MIX_ENHANCE                        = 0x65,
    MIX_ENHANCE_SKILL_CARD             = 0x66,
    MIX_ENHANCE_WITHOUT_FAIL           = 0x67,
    MIX_SET_LEVEL                      = 0xC9,
    MIX_SET_LEVEL_CREATE_ITEM          = 0xCA,
    MIX_SET_LEVEL_SET_FLAG             = 0xD3,
    MIX_SET_LEVEL_SET_FLAG_CREATE_ITEM = 0xD4,
    MIX_ADD_LEVEL                      = 0x12D,
    MIX_ADD_LEVEL_CREATE_ITEM          = 0x12E,
    MIX_ADD_LEVEL_SET_FLAG             = 0x137,
    MIX_ADD_LEVEL_SET_FLAG_CREATE_ITEM = 0x138,
    MIX_RECYCLE                        = 0x191,
    MIX_RECYCLE_ENHANCE                = 0x192,
    MIX_RESTORE_ENHANCE_SET_FLAG       = 0x1F5,
    MIX_CREATE_ITEM                    = 0x259,
};

enum CheckType : int
{
    CT_ItemGroup = 1,
    CT_ItemClass = 2,
    CT_ItemId = 3,
    CT_ItemRank = 4,
    CT_ItemLevel = 5,
    CT_FlagOn = 6,
    CT_FlagOff = 7,
    CT_EnhanceMatch = 8,
    CT_EnhanceMismatch = 9,
    CT_ItemCount = 10,
    CT_ElementalEffectMatch = 11,
    CT_ElementalEffectMismatch = 12,
    CT_ItemWearPositionMatch = 13,
    CT_ItemWearPositionMismatch = 14,
};

struct EnhanceInfo
{
    int nSID;
    uint Flag;
    int nRank;
    int nFailResult;
    int nMaxEnhance;
    uint nLocalFlag;
    int nNeedItemCode;
    float fPercentage[20];
};

struct MaterialInfo
{
    int type[MATERIAL_INFO_COUNT];
    int value[MATERIAL_INFO_COUNT];
};

struct MixBase
{
    int id;
    int type;
    int value[MIX_VALUE_COUNT];
    int sub_material_cnt;
    MaterialInfo main_material;
    MaterialInfo sub_material[MAX_SUB_MATERIAL_COUNT];
};


class Player;
class Item;
class MixManager
{
public:
    MixManager() = default;
    ~MixManager() = default;

    bool EnhanceItem(MixBase* pMixInfo, Player* pPlayer, Item* pMainMaterial, int nSubMaterialCountItem, std::vector<Item*> &pSubItem, std::vector<uint16> &pCountList);
    bool MixItem(MixBase* pMixInfo, Player* pPlayer, Item* pMainMaterial, int nSubMaterialCountItem, std::vector<Item*> &pSubItem, std::vector<uint16> &pCountList);
    bool EnhanceSkillCard(MixBase* pMixInfo, Player* pPlayer, Item* pMainMaterial, int nSubMaterialCount, std::vector<Item*> &pSubItem, std::vector<uint16> &pCountList);
    bool CreateItem(MixBase* pMixInfo, Player* pPlayer, Item* pMainMaterial, int nSubMaterialCount, std::vector<Item*> &pSubItem, std::vector<uint16> &pCountList);

    void RegisterEnhanceInfo(const EnhanceInfo& info);
    void RegisterMixInfo(const MixBase& info);
    MixBase* GetProperMixInfo(Item* pMainMaterial, int nSubMaterialCount, std::vector<Item*> &pSubItem, std::vector<uint16> &pCountList);
    bool getProperMixInfoSub(MixBase* mb, int SubMaterialCount, std::vector<Item*> &pSubItem, std::vector<uint16> &pCountList);
    Item* check_mixable_item(Player* pPlayer, uint hItem, int64 nItemCount);
    bool check_material_info(const MaterialInfo& info, Item* pItem, uint16 &pItemCount);
    EnhanceInfo* getenhanceInfo(int sid);
    void procEnhanceFail(Player* pPlayer, Item* pItem, int nFailResult);

private:
    std::vector<MixBase> m_vMixInfo{};
    std::vector<EnhanceInfo> m_vEnhanceInfo{};
};

#define sMixManager ACE_Singleton<MixManager, ACE_Null_Mutex>::instance()
#endif // PROJECT_MIXMANAGER_H
