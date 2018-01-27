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

        /// \brief Enhances an item
        /// \param pMixInfo The mix info to be used
        /// \param pPlayer The owner of the item
        /// \param pMainMaterial Main material to be used
        /// \param nSubMaterialCountItem count of submaterials
        /// \param pSubItem submaterials
        /// \param pCountList Usable amount of pSubItem[IDX]
        /// \return true on success, false on failure
        bool EnhanceItem(MixBase *pMixInfo, Player *pPlayer, Item *pMainMaterial, int nSubMaterialCountItem, std::vector<Item *> &pSubItem, std::vector<uint16> &pCountList);
        /// \brief General mix function
        /// \param pMixInfo The mix info to be used
        /// \param pPlayer The owner of the item
        /// \param pMainMaterial Main material to be used
        /// \param nSubMaterialCountItem count of submaterials
        /// \param pSubItem submaterials
        /// \param pCountList Usable amount of pSubItem[IDX]
        /// \return true on success, false on failure
        bool MixItem(MixBase *pMixInfo, Player *pPlayer, Item *pMainMaterial, int nSubMaterialCountItem, std::vector<Item *> &pSubItem, std::vector<uint16> &pCountList);
        /// Not implemented yet
        bool EnhanceSkillCard(MixBase *pMixInfo, Player *pPlayer, Item *pMainMaterial, int nSubMaterialCount, std::vector<Item *> &pSubItem, std::vector<uint16> &pCountList);
        /// Not implemented yet
        bool CreateItem(MixBase *pMixInfo, Player *pPlayer, Item *pMainMaterial, int nSubMaterialCount, std::vector<Item *> &pSubItem, std::vector<uint16> &pCountList);

        /// \brief Adds the EnhanceInfo to our list
        /// \param info thing to be added to our list
        void RegisterEnhanceInfo(const EnhanceInfo &info);
        /// \brief Adds the MixBase to our list
        /// \param info thing to be added to our list²
        void RegisterMixInfo(const MixBase &info);
        /// \brief Gets the MixBase for the current enhance attempt
        /// \param pMainMaterial Item to be mixed
        /// \param nSubMaterialCount number of submaterials
        /// \param pSubItem list of subitems
        /// \param pCountList amount of subitems[idx]
        /// \return proper MixBase for the item
        MixBase *GetProperMixInfo(Item *pMainMaterial, int nSubMaterialCount, std::vector<Item *> &pSubItem, std::vector<uint16> &pCountList);
        /// \brief Gets the sub-mix-resource
        /// \param pMainMaterial Item to be mixed
        /// \param nSubMaterialCount number of submaterials
        /// \param pSubItem list of subitems
        /// \param pCountList amount of subitems[idx]
        /// \return
        bool getProperMixInfoSub(MixBase *mb, int SubMaterialCount, std::vector<Item *> &pSubItem, std::vector<uint16> &pCountList);
        /// \brief Checks if mix is legit
        /// \param pPlayer owner
        /// \param hItem item to be mixed
        /// \param nItemCount amount if items
        /// \return hItem as Item
        Item *check_mixable_item(Player *pPlayer, uint hItem, int64 nItemCount);
        /// \brief Checks if the material is legit
        /// \param info The materialinfo to be checked on
        /// \param pItem item to be checked
        /// \param pItemCount amount of items
        /// \return true on success, false on failure
        bool check_material_info(const MaterialInfo &info, Item *pItem, uint16 &pItemCount);
        /// \brief Gets the enhanceInfo for our item
        /// \param sid EnhanceInfo idx from ItemTemplate
        /// \return nullptr if not exist, EnhanceInfo on success
        EnhanceInfo *getenhanceInfo(int sid);
        /// \brief Handles item mix failure
        /// \param pPlayer owner
        /// \param pItem failed item
        /// \param nFailResult fail type
        void procEnhanceFail(Player *pPlayer, Item *pItem, int nFailResult);

    private:
        std::vector<MixBase>     m_vMixInfo{ };
        std::vector<EnhanceInfo> m_vEnhanceInfo{ };
};

#define sMixManager ACE_Singleton<MixManager, ACE_Null_Mutex>::instance()
#endif // PROJECT_MIXMANAGER_H
