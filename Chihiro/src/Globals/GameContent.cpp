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

#include "GameContent.h"

#include "FieldPropManager.h"
#include "Maploader.h"
#include "MemPool.h"
#include "NPC.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "RegionTester.h"
#include "Skill.h"
#include "World.h"

bool GameContent::IsBlocked(float_t x, float_t y)
{
    if (static_cast<int32_t>(x) < 0)
        return true;
    if (static_cast<int32_t>(x) > sWorld.getIntConfig(CONFIG_MAP_WIDTH))
        return true;
    if (static_cast<int32_t>(y) < 0)
        return true;
    if (static_cast<int32_t>(y) > sWorld.getIntConfig(CONFIG_MAP_HEIGHT))
        return true;

    if (sWorld.getBoolConfig(CONFIG_NO_COLLISION_CHECK))
        return false;
    return sObjectMgr.g_qtBlockInfo.Collision({x, y});
}

bool GameContent::CollisionToLine(float_t x1, float_t y1, float_t x2, float_t y2)
{
    return sObjectMgr.g_qtBlockInfo.m_MasterNode.LooseCollision({{x1, y1}, {x2, y2}});
}

bool GameContent::LearnAllSkill(Unit *pUnit)
{
    auto depth = pUnit->GetJobDepth();
    for (int32_t i = 0; i <= depth; ++i) {
        int32_t nCurrJob = i == depth ? pUnit->GetCurrentJob() : pUnit->GetPrevJobId(i);
        auto tree = sObjectMgr.getSkillTree(nCurrJob);
        if (tree.empty())
            continue;

        if (i == depth)
            pUnit->SetCurrentJLv(depth == 0 ? 10 : 50);
        else
            pUnit->SetInt32Value(UNIT_FIELD_PREV_JLV + i, depth == 0 ? 10 : 50);
        for (auto &s : tree) {
            auto currSkill = pUnit->GetSkill(s.skill_id);
            int32_t currSkillLevel = 0;
            if (currSkill != nullptr)
                currSkillLevel = currSkill->m_nSkillLevel;

            if (currSkillLevel >= s.max_skill_lv)
                continue;

            for (currSkillLevel += 1; currSkillLevel <= s.max_skill_lv; ++currSkillLevel) {
                int32_t nNeedJP = sObjectMgr.GetNeedJpForSkillLevelUp(s.skill_id, currSkillLevel, nCurrJob);
                pUnit->SetJP(nNeedJP);
                pUnit->RegisterSkill(s.skill_id, currSkillLevel, 0, nCurrJob);
            }
        }
    }
    return true;
}

bool GameContent::IsInRandomPoolMonster(int32_t group_id, int32_t monster_id)
{
    bool result{false};
    //             ArMoveVector::MOVE_INFO *v3; // eax@11
    //             ArMoveVector::MOVE_INFO *v4; // eax@14
    //             std::_Vector_const_iterator<unsigned int,std::allocator<unsigned int32_t> > this; // [sp+8h] [bp-18h]@5
    //             std::_Vector_iterator<GameContent::RANDOM_POOL_INFO,std::allocator<GameContent::RANDOM_POOL_INFO> > itRandom; // [sp+10h] [bp-10h]@8
    //             std::_Vector_iterator<RANDOM_POOL,std::allocator<RANDOM_POOL> > it; // [sp+18h] [bp-8h]@5

    if (group_id == monster_id) {
        result = true;
    }
    else {
        if (group_id < 0) {
            /*
                    std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
                        &this,
                        dword_64F594,
                        &g_vRandomPool);
                    it.baseclass_0.baseclass_0.baseclass_0._Mycont = this.baseclass_0.baseclass_0._Mycont;
                    it.baseclass_0._Myptr = this._Myptr;
                    while ( 1 )
                    {
                        std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
                            &itRandom,
                            dword_64F598,
                            &g_vRandomPool);
                        if ( std::_Vector_const_iterator<X2D::Point<float>_std::allocator<X2D::Point<float>>>::operator__(
                                 &it,
                                 &itRandom) )
                            break;
                        if ( group_id == LODWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_(&it)->end.x) )
                        {
                            v3 = std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_(&it);
                            std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
                                &itRandom,
                                LODWORD(v3->end.z),
                                &v3->end.y);
                            while ( 1 )
                            {
                                v4 = std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_(&it);
                                std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
                                    &this,
                                    LODWORD(v4->end.face),
                                    &v4->end.y);
                                if ( std::_Vector_const_iterator<X2D::Point<float>_std::allocator<X2D::Point<float>>>::operator__(
                                         &itRandom,
                                         &this) )
                                    break;
                                if ( LODWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_(&itRandom)->end.x) == monster_id )
                                    return 1;
                                std::_Vector_const_iterator<std::pair<GameContent::HUNTAHOLIC_MONSTER_RESPAWN_INFO_const___unsigned_long>_std::allocator<std::pair<GameContent::HUNTAHOLIC_MONSTER_RESPAWN_INFO_const___unsigned_long>>>::operator__(&itRandom);
                            }
                            break;
                        }
                        std::_Vector_const_iterator<StructCreature::AdditionalDamageInfo_std::allocator<StructCreature::AdditionalDamageInfo>>::operator__(&it);
                    }
*/
            result = false;
        }
    }
    return result;
}

NPC *GameContent::GetNewNPC(NPCTemplate *npc_info, uint8_t layer)
{
    auto npc = new NPC{npc_info};
    npc->SetLayer(layer);
    for (auto &ql : sObjectMgr._questLinkStore) {
        if (ql.nNPCID == npc->m_pBase->id) {
            npc->LinkQuest(&ql);
        }
    }
    return npc;
}

void GameContent::AddNPCToWorld()
{
    for (auto &npc : sObjectMgr._npcTemplateStore) {
        if (/*npc.second.spawn_type == SPAWN_NORMAL &&*/ npc.second.local_flag == 0) {
            auto nn = GetNewNPC(&npc.second, 0);
            sWorld.AddObjectToWorld(nn);
        }
    }
}

bool GameContent::SelectItemIDFromDropGroup(int32_t nDropGroupID, int32_t &nItemID, int64_t &nItemCount)
{
    nItemID = 0;
    nItemCount = 1;

    auto dg = sObjectMgr.GetDropGroupInfo(nDropGroupID);
    if (dg != nullptr) {
        int32_t cp = 0;
        int32_t p = irand(1, 100000000);
        for (int32_t i = 0; i < MAX_DROP_GROUP; ++i) {
            cp += dg->drop_percentage[i];
            if (p <= cp) {
                // temporary workaround for dropgroup
                if (dg->drop_item_id[i] > 0 && sObjectMgr.GetItemBase((uint32_t)dg->drop_item_id[i]) == nullptr)
                    continue;
                nItemID = dg->drop_item_id[i];
                nItemCount = 1;
                return true;
            }
        }
    }
    nItemID = 0;
    nItemCount = 1;
    return false;
}

int64_t GameContent::GetItemSellPrice(int64_t price, int32_t rank, int32_t lv, bool same_price_for_buying)
{
    int64_t k = price;
    float_t f[8]{1.35f, 0.4f, 0.2f, 0.13f, 0.1f, 0.1f, 0.1f, 0.1f};

    ASSERT(rank > 8, "Rank cannot be bigger than 8: %d - GameContent::GetItemSellPrice", rank);

    for (int32_t i = 2; i <= lv; i++) {
        switch (rank) {
        case 0:
            k += (price * f[rank] * 0.1f) * 10;
            break;
        case 1:
            k += (price * f[rank - 1] * 0.1f) * 10;
            break;
        case 2:
            k += (price * f[rank - 1] * 0.01f) * 100;
            break;
        default:
            k += (price * f[rank - 1] * 0.001f) * 1000;
            break;
        }
    }

    return (k * (same_price_for_buying ? 1.0f : 0.25f));
}

Monster *GameContent::RespawnMonster(float_t x, float_t y, uint8_t layer, int32_t id, bool is_wandering, int32_t way_point_id, MonsterDeleteHandler *pDeleteHandler, bool /*bNeedLock*/)
{
    auto pMonster = sMemoryPool.AllocMonster((uint32_t)id);
    if (pMonster == nullptr)
        return nullptr;

    pMonster->SetCurrentXY(x, y);
    pMonster->SetLayer(layer);
    pMonster->m_bIsWandering = is_wandering;
    pMonster->m_pDeleteHandler = pDeleteHandler;
    if (way_point_id != 0)
        pMonster->m_pWayPointInfo = sObjectMgr.GetWayPoint(way_point_id);
    sWorld.AddMonsterToWorld(pMonster);

    return pMonster;
}

uint16_t GameContent::IsLearnableSkill(Unit *pUnit, int32_t skill_id, int32_t skill_level, int32_t &job_id)
{
    uint16_t ilsResult = TS_RESULT_ACCESS_DENIED;
    for (int32_t i = 0; i < 4; ++i) {
        if (pUnit->GetPrevJobLv(i) != 0) {
            ilsResult = isLearnableSkill(pUnit, skill_id, skill_level, pUnit->GetPrevJobId(i), pUnit->GetPrevJobLv(i));
            if (ilsResult == TS_RESULT_SUCCESS) {
                job_id = pUnit->GetPrevJobId(i);
                break;
            }
            if (ilsResult != TS_RESULT_ACCESS_DENIED && ilsResult != TS_RESULT_LIMIT_MAX && ilsResult != TS_RESULT_NOT_ENOUGH_JOB_LEVEL)
                break;
        }
    }
    if (ilsResult == TS_RESULT_ACCESS_DENIED || ilsResult == TS_RESULT_LIMIT_MAX || ilsResult == TS_RESULT_NOT_ENOUGH_JOB_LEVEL) {
        ilsResult = isLearnableSkill(pUnit, skill_id, skill_level, pUnit->GetCurrentJob(), pUnit->GetCurrentJLv());
        if (ilsResult == TS_RESULT_SUCCESS)
            job_id = pUnit->GetCurrentJob();
    }
    return ilsResult;
}

uint16_t GameContent::isLearnableSkill(Unit *pUnit, int32_t skill_id, int32_t skill_level, int32_t nJobID, int32_t unit_job_level)
{
    bool bMaxLimit = false;
    bool bFound = false;

    auto st = sObjectMgr.getSkillTree(nJobID);
    if (st.empty())
        return TS_RESULT_ACCESS_DENIED;

    for (auto stree : st) {
        if (stree.skill_id == skill_id) {
            if (stree.max_skill_lv >= skill_level) {
                if (stree.lv > pUnit->GetLevel()) {
                    return TS_RESULT_NOT_ENOUGH_LEVEL;
                }
                if (stree.job_lv <= unit_job_level) {
                    for (int32_t nsi = 0; nsi < 3; nsi++) {
                        if (stree.need_skill_id[nsi] == 0)
                            break;
                        if (pUnit->GetCurrentSkillLevel(stree.need_skill_id[nsi]) < stree.need_skill_lv[nsi]) {
                            return TS_RESULT_NOT_ENOUGH_SKILL;
                        }
                    }
                    auto sb = sObjectMgr.GetSkillBase(skill_id);
                    if (sb->id == 0)
                        return TS_RESULT_ACCESS_DENIED;

                    if (pUnit->GetJP() < (int32_t)((float)sb->GetNeedJobPoint(skill_level) * stree.jp_ratio)) {
                        return TS_RESULT_NOT_ENOUGH_JP;
                    }
                    return TS_RESULT_SUCCESS;
                }
                bFound = true;
            }
            else {
                bMaxLimit = true;
            }
        }
    }
    if (bFound)
        return TS_RESULT_NOT_ENOUGH_JOB_LEVEL;

    if (bMaxLimit)
        return TS_RESULT_LIMIT_MAX;

    return TS_RESULT_ACCESS_DENIED;
}

int32_t GameContent::GetLocationID(const float_t x, const float_t y)
{
    int32_t loc_id = 0;
    int32_t priority = 0x7fffffff;
    X2D::Pointf pt{};
    pt.x = x;
    pt.y = y;
    X2D::QuadTreeMapInfo::FunctorAdaptor fn{};
    sMapContent.g_qtLocationInfo->Enum(pt, fn);

    for (auto &info : fn.pResult) {
        if (info.priority < priority) {
            loc_id = info.location_id;
            priority = info.priority;
        }
    }
    return loc_id;
}