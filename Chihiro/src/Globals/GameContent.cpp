/*
 *  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
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
#include "World.h"
#include "RegionTester.h"
#include "MemPool.h"

int GameContent::EnumSkillTargetsAndCalcDamage(Position _OriginalPos, uint8_t layer, Position _TargetPos, bool bTargetOrigin, float fEffectLength, int nRegionType, float fRegionProperty, int nOriginalDamage, bool bIncludeOriginalPos, Unit *pCaster, int nDistributeType, int nTargetMax, std::vector<Unit *> &vTargetList, bool bEnemyOnly)
{
    int nResult = nOriginalDamage;

    auto rOriginalPos = _TargetPos;
    if (!bTargetOrigin)
        rOriginalPos = _OriginalPos;

    auto rTargetPos = _OriginalPos;
    if (!bTargetOrigin)
        rTargetPos = _TargetPos;

    std::vector<uint> vList{ };
    sWorld->EnumMovableObject(rOriginalPos, layer, fEffectLength, vList, true, true);
    vTargetList.clear();

    std::unique_ptr<RegionTester> regionTest{ };
    switch (nRegionType)
    {
        case 0:
            regionTest = std::make_unique<DirectionRegionTester>();
            break;
        case 1:
            regionTest = std::make_unique<ArcCircleRegionTester>();
            break;
        case 2:
            regionTest = std::make_unique<CrossRegionTester>();
            break;
        default:
            regionTest = std::make_unique<CircleRegionTester>();
            break;
    }

    regionTest->Init(rOriginalPos, rTargetPos, fRegionProperty);

    int nTargetCount = 0;
    int nAllyCount   = 0;
    bool bIsAlly{false};

    for (const uint &uid : vList)
    {
        auto unit = sMemoryPool->GetObjectInWorld<Unit>(uid);
        if (unit != nullptr && unit->GetHealth() != 0)
        {
            if (!pCaster->IsEnemy(unit, true))
            {
                if (bEnemyOnly)
                    continue;
                bIsAlly = true;
            }

            auto current_pos = unit->GetCurrentPosition(sWorld->GetArTime());
            if (regionTest->IsInRegion(current_pos) && (bIncludeOriginalPos || !(rOriginalPos == current_pos)))
            {
                if (bIsAlly)
                {
                    ++nAllyCount;
                }
                else
                {
                    vTargetList.emplace_back(unit);
                    ++nTargetCount;
                }
            }
        }
    }

    if (nDistributeType == 3)
    {
//                 std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
//                     (v14 - 36),
//                     *(v22 + 8),
//                     v22);
//                 std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
//                     (v14 - 24),
//                     *(v22 + 4),
//                     v22);
//                 v32 = std::_Vector_iterator<KHash<_MARKET_INFO___khash_def::hashPr_str<khash_def::string_tr_khash_def::nocase_tr>>::node___std::allocator<KHash<_MARKET_INFO___khash_def::hashPr_str<khash_def::string_tr_khash_def::nocase_tr>>::node__>>::operator_(
//                           (v14 - 24),
//                           (v14 - 80),
//                           *(v14 + 28));
//                 v33.baseclass_0._Myptr = *(v14 - 32);
//                 v33.baseclass_0.baseclass_0.baseclass_0._Mycont = *(v14 - 36);
//                 std::sort<std::_Vector_iterator<StructCreature___std::allocator<StructCreature__>>_lessByDistantFromTarget>(
//                     *&v32->baseclass_0.baseclass_0.baseclass_0._Mycont,
//                     v33,
//                     *(v14 + 16));
//                if (nTargetCount > nTargetMax)
//                    Array.Resize(ref vTargetList, nTargetMax + nAllyCount);
    }

    if(nDistributeType == 4)
    {
//                 std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
//                     (v14 - 36),
//                     *(v22 + 8),
//                     v22);
//                 std::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>::_Vector_const_iterator<StateDamage_std::allocator<StateDamage>>(
//                     (v14 - 24),
//                     *(v22 + 4),
//                     v22);
//                 v34 = std::_Vector_iterator<KHash<_MARKET_INFO___khash_def::hashPr_str<khash_def::string_tr_khash_def::nocase_tr>>::node___std::allocator<KHash<_MARKET_INFO___khash_def::hashPr_str<khash_def::string_tr_khash_def::nocase_tr>>::node__>>::operator_(
//                           (v14 - 24),
//                           (v14 - 80),
//                           *(v14 + 28));
//                 v35.baseclass_0._Myptr = *(v14 - 32);
//                 v35.baseclass_0.baseclass_0.baseclass_0._Mycont = *(v14 - 36);
//                 std::sort<std::_Vector_iterator<StructCreature___std::allocator<StructCreature__>>_lessByDistantFromTarget>(
//                     *&v34->baseclass_0.baseclass_0.baseclass_0._Mycont,
//                     v35,
//                     *(v14 + 8));
    }

    if(nDistributeType == 2 || nDistributeType == 4)
    {
//                if (nTargetCount > nTargetMax)
//                    List.Resize(ref vTargetList, nTargetMax + nAllyCount);
    }

    if(nDistributeType == 1)
    {
        if(nTargetCount > nTargetMax)
            nResult = nTargetMax * nOriginalDamage / nTargetCount;
    }
    return nResult;
}
