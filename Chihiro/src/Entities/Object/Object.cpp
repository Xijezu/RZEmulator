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

#include "Object.h"

#include "ClientPackets.h"
#include "FieldPropManager.h"
#include "Item.h"
#include "Messages.h"
#include "NPC.h"
#include "RegionContainer.h"
#include "SkillProp/SkillProp.h"
#include "Summon.h"
#include "World.h"
#include "WorldSession.h"

// used for creating values for respawn for example
#define PAIR64_HIPART(x) (uint32_t)((uint64_t(x) >> 32) & UI64LIT(0x00000000FFFFFFFF))
#define PAIR64_LOPART(x) (uint32_t)(uint64_t(x) & UI64LIT(0x00000000FFFFFFFF))

Object::Object()
{
    _subType = ST_Object;

    _uint32Values = nullptr;
    _changedFields = nullptr;
    _valuesCount = 0;

    m_inWorld = false;
    m_bDeleteRequest = false;
    m_objectUpdated = false;
}

Object::~Object()
{
    if (IsInWorld())
    {
        // sLog->outCrash("Object::~Object");
        // ASSERT(false);
        RemoveFromWorld();
    }

    delete[] _uint32Values;
    delete[] _changedFields;
}

void Object::_InitValues()
{
    _uint32Values = new uint32_t[_valuesCount];
    memset(_uint32Values, 0, _valuesCount * sizeof(uint32_t));

    _changedFields = new bool[_valuesCount];
    memset(_changedFields, 0, _valuesCount * sizeof(bool));

    m_objectUpdated = false;
}

void Object::AddToWorld()
{
    if (m_inWorld)
        return;

    ASSERT(_uint32Values);

    m_inWorld = true;
}

void Object::RemoveFromWorld()
{
    if (!m_inWorld)
        return;

    m_inWorld = false;
}

void Object::SetInt32Value(uint16_t index, int32_t value)
{
    ASSERT(index < _valuesCount || PrintIndexError(index, true));

    if (m_int32Values[index] != value)
    {
        m_int32Values[index] = value;
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated)
        {
            m_objectUpdated = true;
        }
    }
}

void Object::SetUInt32Value(uint16_t index, uint32_t value)
{
    ASSERT(index < _valuesCount || PrintIndexError(index, true));

    if (_uint32Values[index] != value)
    {
        _uint32Values[index] = value;
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated)
        {
            m_objectUpdated = true;
        }
    }
}

void Object::UpdateUInt32Value(uint16_t index, uint32_t value)
{
    ASSERT(index < _valuesCount || PrintIndexError(index, true));

    _uint32Values[index] = value;
    _changedFields[index] = true;
}

void Object::SetUInt64Value(uint16_t index, uint64_t value)
{
    ASSERT(index + 1 < _valuesCount || PrintIndexError(index, true));
    if (*((uint64_t *)&(_uint32Values[index])) != value)
    {
        _uint32Values[index] = PAIR64_LOPART(value);
        _uint32Values[index + 1] = PAIR64_HIPART(value);
        _changedFields[index] = true;
        _changedFields[index + 1] = true;

        if (m_inWorld && !m_objectUpdated)
        {
            m_objectUpdated = true;
        }
    }
}

bool Object::AddUInt64Value(uint16_t index, uint64_t value)
{
    ASSERT(index + 1 < _valuesCount || PrintIndexError(index, true));
    if (value && !*((uint64_t *)&(_uint32Values[index])))
    {
        _uint32Values[index] = PAIR64_LOPART(value);
        _uint32Values[index + 1] = PAIR64_HIPART(value);
        _changedFields[index] = true;
        _changedFields[index + 1] = true;

        if (m_inWorld && !m_objectUpdated)
        {
            m_objectUpdated = true;
        }

        return true;
    }

    return false;
}

bool Object::RemoveUInt64Value(uint16_t index, uint64_t value)
{
    ASSERT(index + 1 < _valuesCount || PrintIndexError(index, true));
    if (value && *((uint64_t *)&(_uint32Values[index])) == value)
    {
        _uint32Values[index] = 0;
        _uint32Values[index + 1] = 0;
        _changedFields[index] = true;
        _changedFields[index + 1] = true;

        if (m_inWorld && !m_objectUpdated)
        {
            m_objectUpdated = true;
        }

        return true;
    }

    return false;
}

void Object::SetFloatValue(uint16_t index, float value)
{
    ASSERT(index < _valuesCount || PrintIndexError(index, true));

    if (m_floatValues[index] != value)
    {
        m_floatValues[index] = value;
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated)
        {
            m_objectUpdated = true;
        }
    }
}

void Object::SetByteValue(uint16_t index, uint8_t offset, uint8_t value)
{
    ASSERT(index < _valuesCount || PrintIndexError(index, true));

    if (offset > 4)
    {
        NG_LOG_DEBUG("game", "Object::SetByteValue: wrong offset %u", offset);
        return;
    }

    if (uint8_t(_uint32Values[index] >> (offset * 8)) != value)
    {
        _uint32Values[index] &= ~uint32_t(uint32_t(0xFF) << (offset * 8));
        _uint32Values[index] |= uint32_t(uint32_t(value) << (offset * 8));
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated)
        {
            m_objectUpdated = true;
        }
    }
}

void Object::SetUInt16Value(uint16_t index, uint8_t offset, uint16_t value)
{
    ASSERT(index < _valuesCount || PrintIndexError(index, true));

    if (offset > 2)
    {
        NG_LOG_ERROR("game", "Object::SetUInt16Value: wrong offset %u", offset);
        return;
    }

    if (uint16_t(_uint32Values[index] >> (offset * 16)) != value)
    {
        _uint32Values[index] &= ~uint32_t(uint32_t(0xFFFF) << (offset * 16));
        _uint32Values[index] |= uint32_t(uint32_t(value) << (offset * 16));
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated)
        {
            m_objectUpdated = true;
        }
    }
}

void Object::SetStatFloatValue(uint16_t index, float value)
{
    if (value < 0)
        value = 0.0f;

    SetFloatValue(index, value);
}

void Object::SetStatInt32Value(uint16_t index, int32_t value)
{
    if (value < 0)
        value = 0;

    SetUInt32Value(index, uint32_t(value));
}

void Object::ApplyModUInt32Value(uint16_t index, int32_t val, bool apply)
{
    int32_t cur = GetUInt32Value(index);
    cur += (apply ? val : -val);
    if (cur < 0)
        cur = 0;
    SetUInt32Value(index, cur);
}

void Object::ApplyModInt32Value(uint16_t index, int32_t val, bool apply)
{
    int32_t cur = GetInt32Value(index);
    cur += (apply ? val : -val);
    SetInt32Value(index, cur);
}

void Object::ApplyModSignedFloatValue(uint16_t index, float val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    SetFloatValue(index, cur);
}

void Object::ApplyModPositiveFloatValue(uint16_t index, float val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    if (cur < 0)
        cur = 0;
    SetFloatValue(index, cur);
}

void Object::SetFlag(uint16_t index, uint32_t newFlag)
{
    ASSERT(index < _valuesCount || PrintIndexError(index, true));
    uint32_t oldval = _uint32Values[index];
    uint32_t newval = oldval | newFlag;

    if (oldval != newval)
    {
        _uint32Values[index] = newval;
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated)
        {
            m_objectUpdated = true;
        }
    }
}

void Object::RemoveFlag(uint16_t index, uint32_t oldFlag)
{
    ASSERT(index < _valuesCount || PrintIndexError(index, true));
    ASSERT(_uint32Values);

    uint32_t oldval = _uint32Values[index];
    uint32_t newval = oldval & ~oldFlag;

    if (oldval != newval)
    {
        _uint32Values[index] = newval;
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated)
        {
            m_objectUpdated = true;
        }
    }
}

void Object::SetByteFlag(uint16_t index, uint8_t offset, uint8_t newFlag)
{
    ASSERT(index < _valuesCount || PrintIndexError(index, true));

    if (offset > 4)
    {
        NG_LOG_ERROR("entities", "Object::SetByteFlag: wrong offset %u", offset);
        return;
    }

    if (!(uint8_t(_uint32Values[index] >> (offset * 8)) & newFlag))
    {
        _uint32Values[index] |= uint32_t(uint32_t(newFlag) << (offset * 8));
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated)
        {
            m_objectUpdated = true;
        }
    }
}

void Object::RemoveByteFlag(uint16_t index, uint8_t offset, uint8_t oldFlag)
{
    ASSERT(index < _valuesCount || PrintIndexError(index, true));

    if (offset > 4)
    {
        NG_LOG_ERROR("entities", "Object::RemoveByteFlag: wrong offset %u", offset);
        return;
    }

    if (uint8_t(_uint32Values[index] >> (offset * 8)) & oldFlag)
    {
        _uint32Values[index] &= ~uint32_t(uint32_t(oldFlag) << (offset * 8));
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated)
        {
            m_objectUpdated = true;
        }
    }
}

bool Object::PrintIndexError(uint32_t index, bool set) const
{
    // sLog->outError("Attempt %s non-existed value field: %u (count: %u) for object typeid: %u type mask: %u", (set ? "set value to" : "get value from"), index, _valuesCount, GetTypeId(), _subType);

    // ASSERT must fail after function call
    return false;
}

WorldObject::WorldObject(bool isWorldObject)
    : pRegion(nullptr)
    , region_index(-1)
    , m_name("")
    , _isActive(false)
    , m_isWorldObject(isWorldObject)
{
}

WorldObject::~WorldObject() = default;

void WorldObject::SendEnterMsg(Player *pPlayer)
{
    Position tmpPos = this->GetCurrentPosition(sWorld.GetArTime());
    XPacket packet(NGemity::Packets::TS_SC_ENTER);
    packet << (uint8_t)GetMainType();
    packet << GetHandle();
    packet << tmpPos.GetPositionX();
    packet << tmpPos.GetPositionY();
    packet << tmpPos.GetPositionZ();
    packet << (uint8_t)GetLayer();
    packet << (uint8_t)GetSubType();

    switch (GetSubType())
    {
    case ST_NPC:
        NPC::EnterPacket(packet, dynamic_cast<NPC *>(this), pPlayer);
        break;
    case ST_Player:
        Player::EnterPacket(packet, dynamic_cast<Player *>(this), pPlayer);
        break;
    case ST_Summon:
        Summon::EnterPacket(packet, dynamic_cast<Summon *>(this), pPlayer);
        break;
    case ST_Mob:
        Monster::EnterPacket(packet, dynamic_cast<Monster *>(this), pPlayer);
        break;
    case ST_Object:
        Item::EnterPacket(packet, dynamic_cast<Item *>(this));
        break;
    case ST_FieldProp:
        FieldProp::EnterPacket(packet, dynamic_cast<FieldProp *>(this), pPlayer);
        break;
    case ST_SkillProp:
        SkillProp::EnterPacket(packet, dynamic_cast<SkillProp *>(this), pPlayer);
        break;
    default:
        break;
    }

    pPlayer->SendPacket(packet);
    if (IsPlayer())
        Messages::SendWearInfo(pPlayer, dynamic_cast<Unit *>(this));
}

bool WorldObject::SetPendingMove(std::vector<Position> vMoveInfo, uint8_t speed)
{
    if (HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_PENDED))
    {
        return false;
    }
    else
    {
        m_PendingMovePos = std::move(vMoveInfo);
        m_nPendingMoveSpeed = speed;
        SetFlag(UNIT_FIELD_STATUS, STATUS_MOVE_PENDED);
    }
    return true;
}

bool WorldObject::Step(uint32_t tm)
{
    bool res = ArMoveVector::Step(tm);
    if (res)
    {
        auto pos = GetTargetPos();
        this->m_positionX = pos.GetPositionX();
        this->m_positionY = pos.GetPositionY();
        this->m_positionZ = pos.GetPositionZ();
        this->_orientation = pos.GetOrientation();
    }
    this->lastStepTime = tm;
    return res;
}

void WorldObject::RemoveFromWorld()
{
    if (!IsInWorld())
        return;
    Object::RemoveFromWorld();
}

void WorldObject::AddToWorld()
{
    Object::AddToWorld();
}

void WorldObject::AddNoise(int32_t r1, int32_t r2, int32_t v)
{
    float prev_x = GetPositionX();
    float prev_y = GetPositionY();

    auto rs = (double)sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE);
    auto tx = (int32_t)(GetPositionX() / rs);
    auto ty = (int32_t)(GetPositionY() / rs);
    m_positionX = (float)(r1 % v - v / 2) + GetPositionX();
    m_positionY = (float)(r2 % v - v / 2) + GetPositionY();
    if ((int32_t)(m_positionX / rs) != tx)
        m_positionX = prev_x;
    if ((int32_t)(m_positionY / rs) != ty)
        m_positionY = prev_y;
}

Position WorldObject::GetCurrentPosition(uint32_t t)
{
    Position result{};
    ArMoveVector _mv{};
    if (bIsMoving && IsInWorld())
    {
        _mv = ArMoveVector{*dynamic_cast<ArMoveVector *>(this)};
        _mv.Step(t);
        result.m_positionX = _mv.GetPositionX();
        result.m_positionY = _mv.GetPositionY();
        result.m_positionZ = _mv.GetPositionZ();
        result._orientation = _mv.GetOrientation();
    }
    else
    {
        result.m_positionX = GetPositionX();
        result.m_positionY = GetPositionY();
        result.m_positionZ = GetPositionZ();
        result._orientation = GetOrientation();
    }
    return result;
}

void ArMoveVector::Copy(const ArMoveVector &src)
{
    this->m_positionX = src.m_positionX;
    this->m_positionY = src.m_positionY;
    this->m_positionZ = src.m_positionZ;
    this->_orientation = src._orientation;

    this->bIsMoving = src.bIsMoving;
    this->bWithZMoving = src.bWithZMoving;
    this->speed = src.speed;
    this->start_time = src.start_time;
    this->proc_time = src.proc_time;
    for (auto &mi : src.ends)
    {
        this->ends.emplace_back(MoveInfo(mi.end, mi.end_time));
    }
    this->bHasDirectionChanged = src.bHasDirectionChanged;
    this->direction.m_positionX = src.direction.m_positionX;
    this->direction.m_positionY = src.direction.m_positionY;
    this->direction.m_positionZ = src.direction.m_positionZ;
    this->direction._orientation = src.direction._orientation;
}

bool ArMoveVector::Step(uint32_t current_time)
{
    bool res{false};
    std::vector<MoveInfo> removed{};

    if (proc_time < current_time && bIsMoving)
    {
        for (auto &info : ends)
        {
            if (current_time < info.end_time)
            {
                uint32_t et = current_time - proc_time;
                float fet = (float)et / (float)(info.end_time - proc_time);
                m_positionX = (info.end.m_positionX - m_positionX) * fet + m_positionX;
                m_positionY = (info.end.m_positionY - m_positionY) * fet + m_positionY;
                if (bWithZMoving)
                {
                    m_positionZ = (info.end.m_positionZ - m_positionZ) * fet + m_positionZ;
                }
                break;
            }
            m_positionX = info.end.m_positionX;
            m_positionY = info.end.m_positionY;
            m_positionZ = info.end.m_positionZ;
            proc_time = info.end_time;
            removed.emplace_back(info);
            if (!ends.empty())
                SetDirection(ends[0].end);
        }
        proc_time = current_time;
        for (auto &info : removed)
        {
            this->ends.erase(std::remove(ends.begin(), ends.end(), info), ends.end());
        }
        if (ends.empty() || (this->m_positionX == (ends.back()).end.m_positionX && m_positionY == (ends.back()).end.m_positionY && m_positionZ == (ends.back()).end.m_positionZ))
        {
            bIsMoving = false;
            ends.clear();
            res = true;
        }
        else
        {
            res = false;
        }
    }
    else
    {
        res = false;
    }
    return res;
}

void ArMoveVector::SetMultipleMove(std::vector<Position> &_to, uint8_t _speed, uint32_t _start_time, uint32_t current_time)
{
    ends.clear();
    if (!_to.empty())
    {
        speed = _speed;
        uint32_t ct = _start_time;
        if (ct == 0)
            ct = sWorld.GetArTime();

        start_time = ct;
        proc_time = ct;
        uint32_t start_time2 = ct;
        SetDirection(_to.front());
        float before_x = m_positionX;
        float before_y = m_positionY;

        for (const auto &pos : _to)
        {
            float cx = pos.m_positionX - before_x;
            float cy = pos.m_positionY - before_y;

            before_x = pos.m_positionX;
            before_y = pos.m_positionY;

            float length = sqrt(cy * cy + cx * cx);
            auto end_time = (uint32_t)(length / ((float)this->speed / 30.0f) + (float)start_time2);

            MoveInfo mi(pos, end_time);
            ends.emplace_back(mi);
            start_time2 = end_time;
        }

        bIsMoving = true;
    }
}

void ArMoveVector::SetMove(Position _to, uint8_t _speed, uint32_t _start_time, uint32_t current_time)
{
    double v9{}, v10{}, lengtha{}, lengthb{};

    ends.clear();
    speed = _speed;
    uint32_t st = _start_time;
    if (_start_time == 0)
        st = sWorld.GetArTime();

    start_time = st;
    proc_time = st;
    float X = _to.GetPositionX() - GetPositionX();
    float Y = _to.GetPositionY() - GetPositionY();
    SetDirection(_to);
    lengtha = sqrt(Y * Y + X * X);
    lengthb = speed;
    v9 = lengtha / (lengthb / 30.0);
    v10 = start_time;
    if ((start_time & 0x80000000) != 0)
        v10 = v10 + 4294967300.0f;

    MoveInfo mi{_to, (uint32_t)(v9 + v10)};
    ends.emplace_back(mi);

    bIsMoving = true;
}

void ArMoveVector::SetDirection(Position pos)
{
    float px{}, py{};
    px = pos.m_positionX - m_positionX;
    py = pos.m_positionY - m_positionY;
    if (0.0 != px || 0.0 != py)
    {
        bHasDirectionChanged = true;
        direction.m_positionX = pos.m_positionX;
        direction.m_positionY = pos.m_positionY;
        direction.m_positionZ = pos.m_positionZ;
        _orientation = atan2(py, px);
    }
}

Position ArMoveVector::GetTargetPos()
{
    Position result{};
    if (!ends.empty())
        result = (ends.back()).end;
    else
        result = this->GetPosition();
    return result;
}

bool ArMoveVector::IsMoving(uint32_t t)
{
    if (bIsMoving && !ends.empty())
        return t < (ends.back()).end_time;
    else
        return false;
}

float Position::GetRX() const
{
    return static_cast<uint32_t>(GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE));
}
float Position::GetRY() const
{
    return static_cast<uint32_t>(GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE));
}