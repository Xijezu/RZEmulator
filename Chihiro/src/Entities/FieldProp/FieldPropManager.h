#pragma once
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

#include "FieldProp.h"
#include "Common.h"
#include "SharedMutex.h"

struct FieldPropRegenInfo
{
    FieldPropRegenInfo(uint t, uint lt)
    {
        tNextRegen   = t;
        nLifeTime    = lt;
        pRespawnInfo = { };
    }

    FieldPropRespawnInfo pRespawnInfo;
    uint                 tNextRegen;
    uint                 nLifeTime;
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
        void SpawnFieldPropFromScript(FieldPropRespawnInfo prop, int lifeTime);
        void RegisterFieldProp(FieldPropRespawnInfo prop);
        void onFieldPropDelete(FieldProp *prop) override;
        void Update(uint diff);
    private:
        std::vector<FieldPropRespawnInfo> m_vRespawnInfo{ };
        std::vector<FieldPropRegenInfo>   m_vRespawnList{ };
        std::vector<FieldProp *>          m_vExpireObject{ };
        NG_SHARED_MUTEX i_lock;
    protected:
        FieldPropManager() = default;
};
#define sFieldPropManager FieldPropManager::Instance()