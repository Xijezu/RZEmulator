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
#include "FieldPropBase.h"
#include "Unit.h"

class FieldProp;
struct FieldPropDeleteHandler {
    virtual void onFieldPropDelete(FieldProp *prop) = 0;
};

class Player;
struct TS_SC_ENTER;

class FieldProp : public WorldObject {
    friend class Skill;
    friend class FieldPropManager;

public:
    FieldProp() = delete;
    // Deleting the copy & assignment operators
    // Better safe than sorry
    FieldProp(const FieldProp &) = delete;
    FieldProp &operator=(const FieldProp &) = delete;
    /// \brief Used to generate the TS_SC_ENTER packet for a FieldProp
    /// \param pEnterPct Writable object
    /// \param pFieldProp The object we're generating for
    /// \param pPlayer the player who receives the packet
    static void EnterPacket(TS_SC_ENTER &pEnterPct, FieldProp *pFieldProp, Player *pPlayer);
    /// \brief Creates and spawns a fieldprop
    /// \param propDeleteHandler Always the instance of FieldPropManager, is used to delete it from its list
    /// \param pPropInfo MonsterRespawnInfo
    /// \param lifeTime how long the prop is on the map
    /// \return Newly created Fieldprop on success, nullptr on failure
    static FieldProp *Create(FieldPropDeleteHandler *propDeleteHandler, FieldPropRespawnInfo pPropInfo, uint32_t lifeTime);
    bool IsUsable(Player *) const;
    bool Cast();
    bool UseProp(Player *);
    int32_t GetCastingDelay() const;

    bool IsFieldProp() const override { return true; }

private:
    FieldProp(FieldPropDeleteHandler *propDeleteHandler, const FieldPropRespawnInfo& pPropInfo);

    uint32_t m_nRegenTime;
    FieldPropDeleteHandler *m_pDeleteHandler;
    FieldPropTemplate *m_pFieldPropBase;
    FieldPropRespawnInfo m_PropInfo;
    int32_t m_nUseCount;
    bool m_bIsCasting;
    uint32_t nLifeTime;
};
