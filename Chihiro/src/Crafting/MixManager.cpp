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

#include "MixManager.h"
#include "GameContent.h"
#include "GameRule.h"
#include "Item.h"
#include "ItemFields.h"
#include "Log.h"
#include "MemPool.h"
#include "Messages.h"
#include "World.h"

void MixManager::RegisterEnhanceInfo(const EnhanceInfo &info)
{
    for (auto &ei : m_vEnhanceInfo)
    {
        if (ei.nSID == info.nSID)
            return;
    }
    m_vEnhanceInfo.emplace_back(info);
}

void MixManager::RegisterMixInfo(const MixBase &info)
{
    for (auto &mi : m_vMixInfo)
    {
        if (mi.id == info.id)
            return;
    }
    m_vMixInfo.emplace_back(info);
}

bool MixManager::EnhanceItem(MixBase *pMixInfo, Player *pPlayer, Item *pMainMaterial, int32_t nSubMaterialCountItem, std::vector<Item *> &pSubMaterial, std::vector<uint16_t> &pCountList)
{
    if (pMixInfo == nullptr || pPlayer == nullptr || pMainMaterial == nullptr)
        return false;

    int32_t nCurrentEnhance = pMainMaterial->GetItemEnhance();
    auto pInfo = getenhanceInfo(pMixInfo->value[0]);

    if (pInfo == nullptr)
    {
        Messages::SendResult(pPlayer, 256, TS_RESULT_INVALID_ARGUMENT, 0);
        return false;
    }

    Item *pCube = pSubMaterial.front();
    Item *pPowder{nullptr};

    if (pMixInfo->type == MIX_TYPE::MIX_ENHANCE_WITHOUT_FAIL)
    {
        if (pCube != nullptr && pCube->GetItemTemplate()->type == ItemType::TYPE_CUBE)
        {
            pPowder = pSubMaterial[1];
        }
        else
        {
            pPowder = pCube;
            pCube = pSubMaterial[1];
        }
    }

    if (pCube == nullptr || pCube->GetItemCode() != pInfo->nNeedItemCode)
    {
        Messages::SendResult(pPlayer, NGemity::Packets::TS_CS_MIX, TS_RESULT_INVALID_ARGUMENT, 0);
        return false;
    }

    if (!(pMixInfo->type != MIX_TYPE::MIX_ENHANCE || (pMixInfo->value[1] != 0 && pMixInfo->value[2] != 0)))
    {
        NG_LOG_ERROR("server.mixing", "Invalid mix!! %s", pPlayer->GetName());
    }

    int32_t itemEnhance = (pMixInfo->type == MIX_TYPE::MIX_ENHANCE) ? irand(pMixInfo->value[1], pMixInfo->value[2]) : 1;
    if (pInfo->nMaxEnhance < pMainMaterial->GetItemEnhance() + itemEnhance)
        itemEnhance = pInfo->nMaxEnhance - pMainMaterial->GetItemEnhance();

    if (itemEnhance <= 0)
    {
        Messages::SendResult(pPlayer, NGemity::Packets::TS_CS_MIX, TS_RESULT_INVALID_ARGUMENT, 0);
        return false;
    }

    pPlayer->EraseItem(pCube, 1);

    if (pMixInfo->type == MIX_ENHANCE_WITHOUT_FAIL)
        pPlayer->EraseItem(pPowder, 1);

    uint32_t nRandom = pInfo->fPercentage[nCurrentEnhance] * 100000;
    bool bResult{false};

    if (urand(0, 100000) <= nRandom)
    {
        pMainMaterial->GetItemInstance().SetEnhance(nCurrentEnhance + itemEnhance);
        Messages::SendItemMessage(pPlayer, pMainMaterial);
        std::vector<uint32_t> tmpVec{};
        tmpVec.emplace_back(pMainMaterial->GetHandle());
        Messages::SendMixResult(pPlayer, &tmpVec);
        bResult = true;
    }
    else
    {
        if (pMixInfo->type != MIX_ENHANCE_WITHOUT_FAIL)
        {
            procEnhanceFail(pPlayer, pMainMaterial, pInfo->nFailResult);
        }
        else
        {
            pMainMaterial->GetItemInstance().SetEnhance(pMainMaterial->GetItemInstance().GetEnhance() - 1);
            Messages::SendItemMessage(pPlayer, pMainMaterial);
        }
        Messages::SendMixResult(pPlayer, nullptr);
    }
    pMainMaterial->m_bIsNeedUpdateToDB = true;
    pMainMaterial->DBUpdate();

    return bResult;
}

bool MixManager::EnhanceSkillCard(MixBase *pMixInfo, Player *pPlayer, int32_t nSubMaterialCount, std::vector<Item *> &pSubMaterial)
{
    if (nSubMaterialCount != 3)
        return false;

    Item *skillCardMain = nullptr;
    Item *skillCardSec = nullptr;
    Item *skillCube = nullptr;

    for (int32_t i = 0; i < nSubMaterialCount; i++)
    {
        if (pSubMaterial[i]->GetItemTemplate()->group == GROUP_SKILLCARD)
        {
            if (skillCardMain == nullptr)
                skillCardMain = pSubMaterial[i];

            else
                skillCardSec = pSubMaterial[i];
        }
        else if (pSubMaterial[i]->GetItemTemplate()->group == GROUP_SKILL_CUBE)
        {
            skillCube = pSubMaterial[i];
        }
    }

    EnhanceInfo *pInfo = getenhanceInfo(pMixInfo->value[0]);
    if (pInfo == nullptr ||
        skillCardSec == nullptr ||
        skillCardMain->GetItemInstance().GetCode() != skillCardSec->GetItemInstance().GetCode() ||
        skillCardMain->GetItemInstance().GetEnhance() != skillCardSec->GetItemInstance().GetEnhance())
    {
        Messages::SendResult(pPlayer, 256, TS_RESULT_INVALID_ARGUMENT, 0);
        return false;
    }

    pPlayer->EraseItem(skillCube, 1);

    auto nRandom = (uint32_t)(pInfo->fPercentage[skillCardMain->GetItemInstance().GetEnhance()] * 100000.0f);
    if ((uint32_t)irand(0, 100000) > nRandom)
    {
        pPlayer->EraseItem(skillCardMain, 1);
        pPlayer->EraseItem(skillCardSec, 1);
        Messages::SendMixResult(pPlayer, nullptr);
    }
    else
    {
        pPlayer->EraseItem(skillCardSec, 1);
        skillCardMain->GetItemInstance().SetEnhance(skillCardMain->GetItemInstance().GetEnhance() + 1);
        skillCardMain->m_bIsNeedUpdateToDB = true;
        Messages::SendItemMessage(pPlayer, skillCardMain);
        std::vector<uint32_t> handles{};
        handles.emplace_back(skillCardSec->GetHandle());
        Messages::SendMixResult(pPlayer, &handles);
        skillCardMain->DBUpdate();
        return true;
    }
    return false;
}

bool MixManager::MixItem(MixBase *pMixInfo, Player *pPlayer, Item *pMainMaterial, int32_t nSubMaterialCountItem, std::vector<Item *> &pSubItem, std::vector<uint16_t> &pCountList)
{
    if (pMixInfo == nullptr || pPlayer == nullptr || pMainMaterial == nullptr)
        return false;

    for (int32_t i = 0; i < nSubMaterialCountItem; ++i)
    {
        pPlayer->EraseItem(pSubItem[i], pCountList[i]);
    }

    pMainMaterial->GetItemInstance().SetFlag(pMixInfo->value[2]);
    Messages::SendItemMessage(pPlayer, pMainMaterial);
    std::vector<uint32_t> handles{};
    handles.emplace_back(pMainMaterial->GetHandle());
    Messages::SendMixResult(pPlayer, &handles);
    pMainMaterial->DBUpdate();
    return true;
}

bool MixManager::CreateItem(MixBase *pMixInfo, Player *pPlayer, Item *pMainMaterial, int32_t nSubMaterialCount, std::vector<Item *> &pSubItem, std::vector<uint16_t> &pCountList)
{
    if (pMainMaterial != nullptr)
    {
        pPlayer->EraseItem(pMainMaterial, pMainMaterial->GetItemInstance().GetCount());
    }

    for (int32_t i = 0; i < nSubMaterialCount; ++i)
    {
        pPlayer->EraseItem(pSubItem[i], pCountList[i]);
    }
    bool bResult = false;
    if (pMixInfo->value[2] <= irand(0, 99))
    {
        Messages::SendMixResult(pPlayer, nullptr);
        return bResult;
    }

    int64_t nRandom = irand(pMixInfo->value[3], pMixInfo->value[4]);
    int64_t nItemCount = 0;
    if (pMixInfo->value[0] < 0)
        nItemCount = nRandom;
    else
        nItemCount = 1;

    if (nItemCount > 0)
    {
        bResult = true;
        Item *pItem{nullptr};
        while (nItemCount > 0)
        {
            int32_t nItemID = pMixInfo->value[0];
            nItemCount = nRandom;
            while (nItemID < 0)
            {
                GameContent::SelectItemIDFromDropGroup(nItemID, nItemID, nItemCount);
            }
            pItem = Item::AllocItem(0, nItemID, nItemCount, BY_MIX, pMixInfo->value[1], -1, -1, 0, 0, 0, 0, 0);
            if (!pItem->GetItemTemplate()->flaglist[FLAG_DUPLICATE])
            {
                //chatmsg
            }
            else
            {
                //chatmsg
            }

            pPlayer->PushItem(pItem, pItem->GetItemInstance().GetCount(), false);
            pItem->DBInsert();
            std::vector<uint32_t> handles{};
            handles.emplace_back(pItem->GetHandle());
            Messages::SendMixResult(pPlayer, &handles);
            nItemCount--;
        }
    }
    return bResult;
}

MixBase *MixManager::GetProperMixInfo(Item *pMainMaterial, int32_t nSubMaterialCount, std::vector<Item *> &pSubItem, std::vector<uint16_t> &pCountList)
{
    for (auto it = m_vMixInfo.begin(); it != m_vMixInfo.end(); ++it)
    {
        if ((*it).sub_material_cnt != nSubMaterialCount)
            continue;

        uint16_t nMainMaterialCount{1};
        if (!check_material_info((*it).main_material, pMainMaterial, nMainMaterialCount))
            continue;

        bool bSuccess{true};
        std::vector<Item *> pSubMaterialArrangeBuffer(MAX_SUB_MATERIAL_COUNT, nullptr);
        std::vector<uint16_t> pSubMaterialArrangeCountList(MAX_SUB_MATERIAL_COUNT, 0);
        std::vector<bool> bSubMaterialChecked(MAX_SUB_MATERIAL_COUNT, false);

        for (int32_t nMaterialInfoIdx = 0; nMaterialInfoIdx < nSubMaterialCount; ++nMaterialInfoIdx)
        {
            bool bFind{false};
            for (int32_t nSubMaterialIdx = 0; nSubMaterialIdx < nSubMaterialCount; ++nSubMaterialIdx)
            {
                if (bSubMaterialChecked[nSubMaterialIdx])
                    continue;

                if (check_material_info((*it).sub_material[nSubMaterialIdx], pSubItem[nSubMaterialIdx], pCountList[nSubMaterialIdx]))
                {
                    bSubMaterialChecked[nSubMaterialIdx] = true;
                    pSubMaterialArrangeBuffer[nMaterialInfoIdx] = pSubItem[nSubMaterialIdx];
                    pSubMaterialArrangeCountList[nMaterialInfoIdx] = pCountList[nSubMaterialIdx];
                    bFind = true;
                    break;
                }
            }

            if (!bFind)
            {
                bSuccess = false;
                break;
            }
        }

        if (!bSuccess)
            continue;

        if (!post_arrange_check_material_info((*it).main_material, pMainMaterial, nSubMaterialCount, pSubMaterialArrangeBuffer, pSubMaterialArrangeCountList, pMainMaterial, nMainMaterialCount))
            continue;

        for (int32_t nMaterialInfoIdx = 0; nMaterialInfoIdx < nSubMaterialCount; ++nMaterialInfoIdx)
        {
            if (!post_arrange_check_material_info((*it).sub_material[nMaterialInfoIdx], pMainMaterial, nSubMaterialCount, pSubMaterialArrangeBuffer, pSubMaterialArrangeCountList,
                                                  pSubMaterialArrangeBuffer[nMaterialInfoIdx], pSubMaterialArrangeCountList[nMaterialInfoIdx]))
            {
                bSuccess = false;
                break;
            }
        }

        if (!bSuccess)
            continue;

        return &(*it);
    }
    return nullptr;
}

bool MixManager::getProperMixInfoSub(MixBase *mb, int32_t SubMaterialCount, std::vector<Item *> &pSubItem, std::vector<uint16_t> &pCountList)
{
    bool vCheckInfo[9]{false};
    for (int32_t i = 0; i < SubMaterialCount; ++i)
    {
        bool ok{false};
        for (int32_t j = 0; j < SubMaterialCount; ++j)
        {
            if (!vCheckInfo[j])
            {
                if (check_material_info(mb->sub_material[j], pSubItem[i], pCountList[i]))
                {
                    vCheckInfo[j] = true;
                    ok = true;
                    break;
                }
            }
        }
        if (!ok)
            return false;
    }
    return true;
}

Item *MixManager::check_mixable_item(Player *pPlayer, uint32_t hItem, int64_t nItemCount)
{
    auto retItem = sMemoryPool.GetObjectInWorld<Item>(hItem);
    if (retItem == nullptr)
        return nullptr;
    if (!retItem->IsItem())
    {
        Messages::SendResult(pPlayer, 256, TS_RESULT_ACCESS_DENIED, 0);
        return nullptr;
    }
    if (retItem->GetItemInstance().GetCount() < nItemCount)
    {
        Messages::SendResult(pPlayer, 256, TS_RESULT_NOT_EXIST, 0);
        return nullptr;
    }
    if ((retItem->GetItemTemplate()->status_flag & 4) != 0)
    {
        Messages::SendResult(pPlayer, 256, TS_RESULT_ACCESS_DENIED, 0);
        return nullptr;
    }

    if (!pPlayer->IsMixable(retItem))
    {
        Messages::SendResult(pPlayer, 256, TS_RESULT_NOT_EXIST, 0);
        return nullptr;
    }
    return retItem;
}

bool MixManager::check_material_info(const MaterialInfo &info, Item *pItem, uint16_t &pItemCount)
{
    bool bIsCountChecked = false;
    if (info.type[0] == 0)
        return pItem == nullptr;

    if (pItem == nullptr)
        return false;

    for (int32_t i = 0; i < MATERIAL_INFO_COUNT; ++i)
    {
        switch (info.type[i])
        {
        case MixBase::CHECK_ITEM_GROUP:
            if (pItem->GetItemGroup() != info.value[i])
                return false;
            break;

        case MixBase::CHECK_ITEM_CLASS:
            if (static_cast<int32_t>(pItem->GetItemClass()) != info.value[i])
                return false;
            break;
        case MixBase::CHECK_ITEM_ID:
            if (static_cast<int32_t>(pItem->GetItemCode()) != info.value[i])
                return false;
            break;

        case MixBase::CHECK_ITEM_RANK:
            if (pItem->GetItemRank() != info.value[i])
                return false;
            break;

        case MixBase::CHECK_ITEM_LEVEL:
            if (static_cast<int32_t>(pItem->GetItemInstance().GetLevel()) != info.value[i])
                return false;
            break;

        case MixBase::CHECK_FLAG_ON:
            if (((1 << (info.value[i] & 0x1F)) & pItem->GetItemInstance().GetFlag()) == 0)
                return false;
            break;

        case MixBase::CHECK_FLAG_OFF:
            if (((1 << (info.value[i] & 0x1F)) & pItem->GetItemInstance().GetFlag()) != 0)
                return false;
            break;

        case MixBase::CHECK_ENHANCE_MATCH:
            if (pItem->GetItemEnhance() != info.value[i])
                return false;
            break;

        case MixBase::CHECK_ENHANCE_DISMATCH:
            if (pItem->GetItemEnhance() == info.value[i])
                return false;
            break;

        case MixBase::CHECK_ITEM_COUNT:
            if (pItemCount != info.value[i])
                return false;
            break;

        case MixBase::CHECK_ELEMENTAL_EFFECT_MATCH:
            break;
            /*tv = ((int32_t)pow(2.0,(double)pItem->GetElementalEffectType())) >> 1;
                if (tv != 0)
                {
                    if ((tv & info.value[i]) != tv)
                        return false;
                }
                else
                {
                    if (info.value[i] != 0)
                        return false;
                }
                break;*/

            /*case (int32_t)MixBase.CheckType.ElementalEffectMismatch:
                    tv = ((int32_t)Math.Pow(2.0, (double)pItem.GetElementalEffectType())) >> 1;
                    if (tv != 0)
                    {
                        if ((tv & info.value[i]) == tv)
                            return false;
                    }
                    else
                    {
                        if (info.value[i] == 0)
                            return false;
                    }
                    break;*/

        case MixBase::CHECK_ITEM_WEAR_POSITION_MATCH:
            if (static_cast<int32_t>(pItem->GetWearType()) != info.value[i])
                return false;
            break;

        case MixBase::CHECK_ITEM_WEAR_POSITION_MISMATCH:
            if (static_cast<int32_t>(pItem->GetWearType()) == info.value[i])
                return false;
            break;
        default:
            break;
        }
    }
    if (!bIsCountChecked)
        pItemCount = 1;
    return true;
}

EnhanceInfo *MixManager::getenhanceInfo(int32_t sid)
{
    auto info = std::find_if(m_vEnhanceInfo.begin(),
                             m_vEnhanceInfo.end(),
                             [sid](const EnhanceInfo &res) { return sid == res.nSID; });

    return info != m_vEnhanceInfo.end() ? &*info : nullptr;
}

void MixManager::procEnhanceFail(Player *pPlayer, Item *pItem, int32_t nFailResult)
{
    //nFailResult is not calculated correctly atm so this needs to be done
    // Note: nFailResult is part of the settings actually
    if (nFailResult == 0)
    {
        nFailResult = 1;
    }

    if (nFailResult == EnhanceInfo::RESULT_FAIL)
    {
        pItem->GetItemInstance().SetFlag(ITEM_FLAG_FAILED);
        for (int i = 0; MAX_SOCKET_NUMBER; ++i)
        {
            // Implement set socket code here
        }

        Messages::SendItemMessage(pPlayer, pItem);
        pItem->DBUpdate();
        return;
    }
    else if (nFailResult == EnhanceInfo::RESULT_SKILL_CARD_FAIL)
    {
        if (pItem->GetItemEnhance() <= 3)
        {
            pPlayer->EraseItem(pItem, 1);
            return;
        }
        else
        {
            pItem->GetItemInstance().SetEnhance(pItem->GetItemInstance().GetEnhance() - 3);
            Messages::SendItemMessage(pPlayer, pItem);
            pItem->DBUpdate();
            return;
        }
    }
    else if (nFailResult == EnhanceInfo::RESULT_ACCESSORY_FAIL)
    {
        if (pItem->GetItemInstance().GetEnhance() <= 3)
        {
            pItem->GetItemInstance().SetEnhance(0);
        }
        else
        {
            pItem->GetItemInstance().SetEnhance(pItem->GetItemInstance().GetEnhance() - 3);
        }
        Messages::SendItemMessage(pPlayer, pItem);
        pItem->DBUpdate();
        return;
    }
}

bool MixManager::CompatibilityCheck(const int32_t *nSubMaterialCount, std::vector<Item *> &pSubItem, Item *pItem)
{
    if (pItem == nullptr)
    {
        return false;
    }

    if (*nSubMaterialCount == 1)
    {
        if (pItem->GetItemInstance().GetFlag() % 2 == ITEM_FLAG_CARD)
        {
            switch (pSubItem[0]->GetItemInstance().GetCode())
            {
            case UNIT_CARD:
                return false;
            default:
                break;
            }
        }
        else if (pItem->GetItemInstance().GetFlag() % 2 == ITEM_FLAG_NORMAL)
        {
            switch (pSubItem[0]->GetItemInstance().GetCode())
            {
            case CHALK_OF_RESTORATION:
                return false;
            default:
                break;
            }
        }

        if (pItem->GetItemInstance().GetFlag() == ITEM_FLAG_FAILED)
        {
            return pSubItem[0]->GetItemInstance().GetCode() == E_REPAIR_POWDER;
        }
    }
    return true;
}

bool MixManager::RepairItem(Player *pPlayer, Item *pMainMaterial, int32_t nSubMaterialCountItem, std::vector<Item *> &pSubItem, std::vector<uint16_t> &pCountList)
{
    if (pPlayer == nullptr || pMainMaterial == nullptr)
        return false;

    if (pMainMaterial->GetItemInstance().GetFlag() == ITEM_FLAG_FAILED)
    {
        for (int32_t i = 0; i < nSubMaterialCountItem; ++i)
        {
            pPlayer->EraseItem(pSubItem[i], pCountList[i]);
        }

        pMainMaterial->GetItemInstance().SetFlag(ITEM_FLAG_NORMAL);
        Messages::SendItemMessage(pPlayer, pMainMaterial);
        std::vector<uint32_t> handles{};
        handles.emplace_back(pMainMaterial->GetHandle());
        Messages::SendMixResult(pPlayer, &handles);
        pMainMaterial->DBUpdate();
    }
    return true;
}

bool MixManager::post_arrange_check_material_info(MaterialInfo &info, Item *pMainMaterial, int32_t nSubMaterialCount, std::vector<Item *> pArrangedSubMaterial,
                                                  std::vector<uint16_t> pArrangedCountList, Item *pItem, uint16_t pItemCount)
{
    for (int32_t i = 0; i < MATERIAL_INFO_COUNT; ++i)
    {
        switch (info.type[i])
        {
        case MixBase::CHECK_SAME_ITEM_ID:
        {
            int32_t nSlotIndex = info.value[i];
            if (nSlotIndex < 0 && nSlotIndex > nSubMaterialCount)
            {
                NG_LOG_ERROR("server.mixing", "Invalid post_arrange_check!!");
                return false;
            }

            if (nSlotIndex == 0)
            {
                if (pItem->GetItemCode() != pMainMaterial->GetItemCode())
                    return false;
            }
            else if (pItem->GetItemCode() != pArrangedSubMaterial[nSlotIndex - 1]->GetItemCode())
            {
                return false;
            }
            break;
        }
        case MixBase::CHECK_SAME_SUMMON_CODE:
        {
            NG_LOG_DEBUG("server.mixing", "CHECK_SAME_SUMMON_CODE - Not implemented yet");
            return false;
        }
        default:
            break;
        }
    }
    return true;
}