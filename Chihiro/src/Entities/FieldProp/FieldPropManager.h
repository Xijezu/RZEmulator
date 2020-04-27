#pragma once
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

#include "Common.h"
#include "FieldProp.h"
#include "SharedMutex.h"

struct FieldPropRegenInfo
{
    FieldPropRegenInfo(uint32_t t, uint32_t lt)
    {
        tNextRegen = t;
        nLifeTime = lt;
    }

    FieldPropRespawnInfo pRespawnInfo;
    uint32_t tNextRegen;
    uint32_t nLifeTime;
};

class FieldPropManager : public FieldPropDeleteHandler
{
public:
    static FieldPropManager &Instance()
    {
        static FieldPropManager instance;
        return instance;
    }

    // Deleting the copy & assignment operators
    // Better safe than sorry
    FieldPropManager(const FieldPropManager &) = delete;
    FieldPropManager &operator=(const FieldPropManager &) = delete;

    ~FieldPropManager() = default;
    void SpawnFieldPropFromScript(FieldPropRespawnInfo prop, int32_t lifeTime);
    void RegisterFieldProp(FieldPropRespawnInfo prop);
    void onFieldPropDelete(FieldProp *prop) override;
    void Update(uint32_t diff);

private:
    std::vector<FieldPropRespawnInfo> m_vRespawnInfo{};
    std::vector<FieldPropRegenInfo> m_vRespawnList{};
    std::vector<FieldProp *> m_vExpireObject{};
    NG_SHARED_MUTEX i_lock;

protected:
    FieldPropManager() = default;
};
#define sFieldPropManager FieldPropManager::Instance()