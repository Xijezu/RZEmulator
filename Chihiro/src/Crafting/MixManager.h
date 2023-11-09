/*
 *  Copyright (C) 2017-2020 NGemity <https://ngemity.org/>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Common.h"

constexpr int32_t MATERIAL_INFO_COUNT = 5;
constexpr int32_t MIX_VALUE_COUNT = 6;
constexpr int32_t MAX_SUB_MATERIAL_COUNT = 9;

enum MIX_TYPE {
    MIX_ENHANCE = 101,
    MIX_ENHANCE_SKILL_CARD = 102,
    MIX_ENHANCE_WITHOUT_FAIL = 103,
    MIX_SET_LEVEL = 201,
    MIX_SET_LEVEL_CREATE_ITEM = 202,
    MIX_SET_LEVEL_SET_FLAG = 211,
    MIX_SET_LEVEL_SET_FLAG_CREATE_ITEM = 212,
    MIX_ADD_LEVEL = 301,
    MIX_ADD_LEVEL_CREATE_ITEM = 302,
    MIX_ADD_LEVEL_SET_FLAG = 311,
    MIX_ADD_LEVEL_SET_FLAG_CREATE_ITEM = 312,
    MIX_RECYCLE = 401,
    MIX_RECYCLE_ENHANCE = 402,
    MIX_RESTORE_ENHANCE_SET_FLAG = 501,
    MIX_CREATE_ITEM = 601,
    MIX_SET_ELEMENTAL_EFFECT = 701,
    MIX_SET_ELEMENTAL_EFFECT_PARAMETER = 702,
    MIX_SACRIFICE_ITEM_FOR_ETHEREAL_DURABILITY = 801,
    MIX_TRANSMIT_ETHEREAL_DURABILITY = 802,
    MIX_RECOVER_EXHAUSTED_ETHEREAL_DURABILITY = 803,
};

struct EnhanceInfo {
    int32_t nSID;
    uint32_t Flag;
    int32_t nRank;
    int32_t nFailResult;
    int32_t nMaxEnhance;
    uint32_t nLocalFlag;
    int32_t nNeedItemCode;
    float_t fPercentage[20];

    enum {
        RESULT_FAIL = 1,
        RESULT_SKILL_CARD_FAIL = 2,
        RESULT_ACCESSORY_FAIL = 3,
    };
};

struct MaterialInfo {
    int32_t type[MATERIAL_INFO_COUNT];
    int32_t value[MATERIAL_INFO_COUNT];
};

struct MixBase {
    int32_t id;
    int32_t type;
    int32_t value[MIX_VALUE_COUNT];
    int32_t sub_material_cnt;
    MaterialInfo main_material;
    MaterialInfo sub_material[MAX_SUB_MATERIAL_COUNT];

    enum {
        CHECK_ITEM_GROUP = 1,
        CHECK_ITEM_CLASS = 2,
        CHECK_ITEM_ID = 3,
        CHECK_ITEM_RANK = 4,
        CHECK_ITEM_LEVEL = 5,
        CHECK_FLAG_ON = 6,
        CHECK_FLAG_OFF = 7,

        CHECK_ENHANCE_MATCH = 8,
        CHECK_ENHANCE_DISMATCH = 9,
        CHECK_ITEM_COUNT = 10,
        CHECK_ELEMENTAL_EFFECT_MATCH = 11,
        CHECK_ELEMENTAL_EFFECT_MISMATCH = 12,
        CHECK_ITEM_WEAR_POSITION_MATCH = 13,
        CHECK_ITEM_WEAR_POSITION_MISMATCH = 14,
        CHECK_ITEM_COUNT_GE = 15,

        CHECK_ITEM_ETHEREAL_DURABILITY_E = 16,
        CHECK_ITEM_ETHEREAL_DURABILITY_NE = 17,
        CHECK_ITEM_GRADE = 18,

        CHECK_SAME_ITEM_ID = 19,
        CHECK_SAME_SUMMON_CODE = 20,
    };
};

class Player;
class Item;
class MixManager {
public:
    static MixManager &Instance()
    {
        static MixManager instance;
        return instance;
    }

    ~MixManager() = default;

    /// \brief Enhances an item
    /// \param pMixInfo The mix info to be used
    /// \param pPlayer The owner of the item
    /// \param pMainMaterial Main material to be used
    /// \param nSubMaterialCountItem count of submaterials
    /// \param pSubItem submaterials
    /// \param pCountList Usable amount of pSubItem[IDX]
    /// \return true on success, false on failure
    bool EnhanceItem(MixBase *pMixInfo, Player *pPlayer, Item *pMainMaterial, int32_t nSubMaterialCountItem, std::vector<Item *> &pSubItem, std::vector<uint16_t> &pCountList);
    /// Not implemented yet
    bool EnhanceSkillCard(MixBase *pMixInfo, Player *pPlayer, int32_t nSubMaterialCount, std::vector<Item *> &pSubItem);
    /// \brief General mix function
    /// \param pMixInfo The mix info to be used
    /// \param pPlayer The owner of the item
    /// \param pMainMaterial Main material to be used
    /// \param nSubMaterialCountItem count of submaterials
    /// \param pSubItem submaterials
    /// \param pCountList Usable amount of pSubItem[IDX]
    /// \return true on success, false on failure
    bool MixItem(MixBase *pMixInfo, Player *pPlayer, Item *pMainMaterial, int32_t nSubMaterialCountItem, std::vector<Item *> &pSubItem, std::vector<uint16_t> &pCountList);
    /// Not implemented yet
    bool CreateItem(MixBase *pMixInfo, Player *pPlayer, Item *pMainMaterial, int32_t nSubMaterialCount, std::vector<Item *> &pSubItem, std::vector<uint16_t> &pCountList);

    bool RestoreEnhance(MixBase *pMixInfo, Player *pPlayer, Item *pMainMaterial, int32_t nSubMaterialCount, std::vector<Item *> &pSubItem, std::vector<uint16_t> &pCountList);

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
    MixBase *GetProperMixInfo(Item *pMainMaterial, int32_t nSubMaterialCount, std::vector<Item *> &pSubItem, std::vector<uint16_t> &pCountList);
    /// \brief Gets the sub-mix-resource
    /// \param pMainMaterial Item to be mixed
    /// \param nSubMaterialCount number of submaterials
    /// \param pSubItem list of subitems
    /// \param pCountList amount of subitems[idx]
    /// \return
    bool getProperMixInfoSub(MixBase *mb, int32_t SubMaterialCount, std::vector<Item *> &pSubItem, std::vector<uint16_t> &pCountList);
    /// \brief Checks if mix is legit
    /// \param pPlayer owner
    /// \param hItem item to be mixed
    /// \param nItemCount amount if items
    /// \return hItem as Item
    Item *check_mixable_item(Player *pPlayer, uint32_t hItem, int64_t nItemCount);
    /// \brief Checks if the material is legit
    /// \param info The materialinfo to be checked on
    /// \param pItem item to be checked
    /// \param pItemCount amount of items
    /// \return true on success, false on failure
    bool check_material_info(const MaterialInfo &info, Item *pItem, uint16_t &pItemCount);
    /// \brief Gets the enhanceInfo for our item
    /// \param sid EnhanceInfo idx from ItemTemplate
    /// \return nullptr if not exist, EnhanceInfo on success
    EnhanceInfo *getenhanceInfo(int32_t sid);
    /// \brief Handles item mix failure
    /// \param pPlayer owner
    /// \param pItem failed item
    /// \param nFailResult fail type
    void procEnhanceFail(Player *pPlayer, Item *pItem, int32_t nFailResult);
    /// \brief Repair broken Items
    /// \param pPlayer The owner of the item
    /// \param pMainMaterial Main material to be used
    /// \param nSubMaterialCountItem count of submaterials
    /// \param pSubItem submaterials
    /// \param pCountList Usable amount of pSubItem[IDX]
    /// \return true on success, false on failure
    bool RepairItem(Player *pPlayer, Item *pMainMaterial, int32_t nSubMaterialCountItem, std::vector<Item *> &pSubItem, std::vector<uint16_t> &pCountList);

private:
    std::vector<MixBase> m_vMixInfo{};
    std::vector<EnhanceInfo> m_vEnhanceInfo{};
    /// \brief Check if a mix makes sense
    bool CompatibilityCheck(const int32_t *nSubMaterialCount, std::vector<Item *> &pSubItem, Item *pItem);
    bool post_arrange_check_material_info(
        MaterialInfo &info, Item *pMainMaterial, int32_t nSubMaterialCount, std::vector<Item *> pArrangedSubMaterial, std::vector<uint16_t> pArrangedCountList, Item *pItem, uint16_t pItemCount);

protected:
    MixManager() = default;
};

#define sMixManager MixManager::Instance()
