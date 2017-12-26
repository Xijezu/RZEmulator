#include "Object.h"
#include "ClientPackets.h"
#include "GameHandler.h"
#include "World.h"
#include "ArRegion.h"
#include "Messages.h"

Object::Object()
{
    _subType = ST_Object;

    _uint32Values  = nullptr;
    _changedFields = nullptr;
    _valuesCount   = 0;

    m_inWorld       = false;
    m_objectUpdated = false;
}

Object::~Object()
{
    if (IsInWorld()) {
        //sLog->outCrash("Object::~Object");
        //ASSERT(false);
        RemoveFromWorld();
    }

    delete[] _uint32Values;
    delete[] _changedFields;
}

void Object::_InitValues()
{
    _uint32Values = new uint32[_valuesCount];
    memset(_uint32Values, 0, _valuesCount * sizeof(uint32));

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

void Object::SetInt32Value(uint16 index, int32 value)
{
            ASSERT(index < _valuesCount || PrintIndexError(index, true));

    if (m_int32Values[index] != value) {
        m_int32Values[index]  = value;
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated) {
            m_objectUpdated = true;
        }
    }
}

void Object::SetUInt32Value(uint16 index, uint32 value)
{
            ASSERT(index < _valuesCount || PrintIndexError(index, true));

    if (_uint32Values[index] != value) {
        _uint32Values[index]  = value;
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated) {
            m_objectUpdated = true;
        }
    }
}

void Object::UpdateUInt32Value(uint16 index, uint32 value)
{
            ASSERT(index < _valuesCount || PrintIndexError(index, true));

    _uint32Values[index]  = value;
    _changedFields[index] = true;
}

void Object::SetUInt64Value(uint16 index, uint64 value)
{
            ASSERT(index + 1 < _valuesCount || PrintIndexError(index, true));
    if (*((uint64 *) &(_uint32Values[index])) != value) {
        _uint32Values[index]      = PAIR64_LOPART(value);
        _uint32Values[index + 1]  = PAIR64_HIPART(value);
        _changedFields[index]     = true;
        _changedFields[index + 1] = true;

        if (m_inWorld && !m_objectUpdated) {
            m_objectUpdated = true;
        }
    }
}

bool Object::AddUInt64Value(uint16 index, uint64 value)
{
            ASSERT(index + 1 < _valuesCount || PrintIndexError(index, true));
    if (value && !*((uint64 *) &(_uint32Values[index]))) {
        _uint32Values[index]      = PAIR64_LOPART(value);
        _uint32Values[index + 1]  = PAIR64_HIPART(value);
        _changedFields[index]     = true;
        _changedFields[index + 1] = true;

        if (m_inWorld && !m_objectUpdated) {
            m_objectUpdated = true;
        }

        return true;
    }

    return false;
}

bool Object::RemoveUInt64Value(uint16 index, uint64 value)
{
            ASSERT(index + 1 < _valuesCount || PrintIndexError(index, true));
    if (value && *((uint64 *) &(_uint32Values[index])) == value) {
        _uint32Values[index]      = 0;
        _uint32Values[index + 1]  = 0;
        _changedFields[index]     = true;
        _changedFields[index + 1] = true;

        if (m_inWorld && !m_objectUpdated) {
            m_objectUpdated = true;
        }

        return true;
    }

    return false;
}

void Object::SetFloatValue(uint16 index, float value)
{
            ASSERT(index < _valuesCount || PrintIndexError(index, true));

    if (m_floatValues[index] != value) {
        m_floatValues[index]  = value;
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated) {
            m_objectUpdated = true;
        }
    }
}

void Object::SetByteValue(uint16 index, uint8 offset, uint8 value)
{
            ASSERT(index < _valuesCount || PrintIndexError(index, true));

    if (offset > 4) {
        MX_LOG_DEBUG("game", "Object::SetByteValue: wrong offset %u", offset);
        return;
    }

    if (uint8(_uint32Values[index] >> (offset * 8)) != value) {
        _uint32Values[index] &= ~uint32(uint32(0xFF) << (offset * 8));
        _uint32Values[index] |= uint32(uint32(value) << (offset * 8));
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated) {
            m_objectUpdated = true;
        }
    }
}

void Object::SetUInt16Value(uint16 index, uint8 offset, uint16 value)
{
            ASSERT(index < _valuesCount || PrintIndexError(index, true));

    if (offset > 2) {
        MX_LOG_ERROR("game", "Object::SetUInt16Value: wrong offset %u", offset);
        return;
    }

    if (uint16(_uint32Values[index] >> (offset * 16)) != value) {
        _uint32Values[index] &= ~uint32(uint32(0xFFFF) << (offset * 16));
        _uint32Values[index] |= uint32(uint32(value) << (offset * 16));
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated) {
            m_objectUpdated = true;
        }
    }
}

void Object::SetStatFloatValue(uint16 index, float value)
{
    if (value < 0)
        value = 0.0f;

    SetFloatValue(index, value);
}

void Object::SetStatInt32Value(uint16 index, int32 value)
{
    if (value < 0)
        value = 0;

    SetUInt32Value(index, uint32(value));
}

void Object::ApplyModUInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetUInt32Value(index);
    cur += (apply ? val : -val);
    if (cur < 0)
        cur   = 0;
    SetUInt32Value(index, cur);
}

void Object::ApplyModInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetInt32Value(index);
    cur += (apply ? val : -val);
    SetInt32Value(index, cur);
}

void Object::ApplyModSignedFloatValue(uint16 index, float val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    SetFloatValue(index, cur);
}

void Object::ApplyModPositiveFloatValue(uint16 index, float val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    if (cur < 0)
        cur   = 0;
    SetFloatValue(index, cur);
}

void Object::SetFlag(uint16 index, uint32 newFlag)
{
            ASSERT(index < _valuesCount || PrintIndexError(index, true));
    uint32 oldval = _uint32Values[index];
    uint32 newval = oldval | newFlag;

    if (oldval != newval) {
        _uint32Values[index]  = newval;
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated) {
            m_objectUpdated = true;
        }
    }
}

void Object::RemoveFlag(uint16 index, uint32 oldFlag)
{
            ASSERT(index < _valuesCount || PrintIndexError(index, true));
            ASSERT(_uint32Values);

    uint32 oldval = _uint32Values[index];
    uint32 newval = oldval & ~oldFlag;

    if (oldval != newval) {
        _uint32Values[index]  = newval;
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated) {
            m_objectUpdated = true;
        }
    }
}

void Object::SetByteFlag(uint16 index, uint8 offset, uint8 newFlag)
{
            ASSERT(index < _valuesCount || PrintIndexError(index, true));

    if (offset > 4) {
        MX_LOG_ERROR("entities", "Object::SetByteFlag: wrong offset %u", offset);
        return;
    }

    if (!(uint8(_uint32Values[index] >> (offset * 8)) & newFlag)) {
        _uint32Values[index] |= uint32(uint32(newFlag) << (offset * 8));
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated) {
            m_objectUpdated = true;
        }
    }
}

void Object::RemoveByteFlag(uint16 index, uint8 offset, uint8 oldFlag)
{
            ASSERT(index < _valuesCount || PrintIndexError(index, true));

    if (offset > 4) {
        MX_LOG_ERROR("entities", "Object::RemoveByteFlag: wrong offset %u", offset);
        return;
    }

    if (uint8(_uint32Values[index] >> (offset * 8)) & oldFlag) {
        _uint32Values[index] &= ~uint32(uint32(oldFlag) << (offset * 8));
        _changedFields[index] = true;

        if (m_inWorld && !m_objectUpdated) {
            m_objectUpdated = true;
        }
    }
}

bool Object::PrintIndexError(uint32 index, bool set) const
{
    //sLog->outError("Attempt %s non-existed value field: %u (count: %u) for object typeid: %u type mask: %u", (set ? "set value to" : "get value from"), index, _valuesCount, GetTypeId(), _subType);

    // ASSERT must fail after function call
    return false;
}

WorldObject::WorldObject(bool isWorldObject) : m_name(""), _isActive(false), m_isWorldObject(isWorldObject),
/*m_currMap(NULL),*/ m_InstanceId(0),
                                               m_notifyflags(0), m_executed_notifies(0)
{
}

WorldObject::~WorldObject()
{
}




void WorldObject::SendEnterMsg(Player *pPlayer)
{
    Position tmpPos = this->GetCurrentPosition(sWorld->GetArTime());
    XPacket packet(TS_SC_ENTER);
    packet << (uint8_t) GetMainType();
    packet << GetHandle();
    packet << tmpPos.GetPositionX();
    packet << tmpPos.GetPositionY();
    packet << (float)0; GetPositionZ();
    packet << (uint8) 0;//GetLayer();
    packet << (uint8_t)GetSubType();

    switch (GetSubType()) {
        case ST_NPC:
            NPC::EnterPacket(packet, dynamic_cast<NPC *>(this));
            break;
        case ST_Player:
            Player::EnterPacket(packet, dynamic_cast<Player *>(this));
            break;
        case ST_Summon:
            Summon::EnterPacket(packet, dynamic_cast<Summon *>(this));
            break;
        default:
            break;
    }
    pPlayer->SendPacket(packet);
    if(GetSubType() == ST_Player)
        Messages::SendWearInfo(pPlayer, dynamic_cast<Unit*>(this));

}

void WorldObject::SetWorldObject(bool on)
{
    if (!IsInWorld())
        return;

    //GetMap()->AddObjectToSwitchList(this, on);
}

bool WorldObject::IsWorldObject() const
{
    if (m_isWorldObject)
        return true;

// 	if (ToCreature() && ToCreature()->_isTempWorldObject)
// 		return true;

    return false;
}

void WorldObject::CleanupsBeforeDelete(bool /*finalCleanup*/)
{
    if (IsInWorld())
        RemoveFromWorld();
}

bool WorldObject::SetPendingMove(std::vector<Position> vMoveInfo, uint8_t speed)
{
    if (HasFlag(UNIT_FIELD_STATUS, MovePending)) {
        return false;
    } else {
        m_PendingMovePos    = std::move(vMoveInfo);
        m_nPendingMoveSpeed = speed;
        SetFlag(UNIT_FIELD_STATUS, MovePending);
    }
    return true;
}

bool WorldObject::Step(uint tm)
{
    bool res = ArMoveVector::Step(tm);
    if(res)
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
    //sArRegion->GetRegion(*this)->RemoveObject(this);
    if(!IsInWorld())
        return;
    Object::RemoveFromWorld();
}

void WorldObject::AddToWorld()
{
    //sArRegion->GetRegion(*this)->AddObject(this);
    Object::AddToWorld();
}

void WorldObject::AddNoise(int r1, int r2, int v)
{
    float prev_x = GetPositionX();
    float prev_y = GetPositionX();

    auto rs = (double)AR_REGION_SIZE;
    auto tx = (int) (GetPositionX() / rs);
    auto ty = (int) (GetPositionY() / rs);
    m_positionX = (float)(r1 % v - v / 2) + GetPositionX();
    m_positionY = (float)(r2 % v - v / 2) + GetPositionY();
    if ((int)(m_positionX / rs) != tx)
        m_positionX = prev_x;
    if ((int)(m_positionY / rs) != ty)
        m_positionY = prev_y;
}

ArMoveVector::ArMoveVector(const ArMoveVector *src)
{
    this->m_positionX = src->m_positionX;
    this->m_positionY = src->m_positionY;
    this->m_positionZ = src->m_positionZ;
    this->_orientation = src->_orientation;

    this->bIsMoving = src->bIsMoving;
    this->bWithZMoving = src->bWithZMoving;
    this->speed = src->speed;
    this->start_time = src->start_time;
    this->proc_time = src->proc_time;
    for(auto& mi : src->ends) {
        this->ends.emplace_back(MoveInfo(mi.end, mi.end_time));
    }
    this->bHasDirectionChanged = src->bHasDirectionChanged;
    this->direction.m_positionX = src->direction.m_positionX;
    this->direction.m_positionY = src->direction.m_positionY;
    this->direction.m_positionZ = src->direction.m_positionZ;
    this->direction._orientation = src->direction._orientation;
}

bool ArMoveVector::Step(uint current_time)
{
    bool res{false};
    std::vector<MoveInfo> removed{ };

    if(proc_time < current_time && bIsMoving) {
        for(auto& info : ends) {
            if(current_time < info.end_time) {
                uint et = current_time - proc_time;
                float fet = (float)et / (float)(info.end_time - proc_time);
                m_positionX = (info.end.m_positionX - m_positionX) * fet + m_positionX;
                m_positionY = (info.end.m_positionY - m_positionY) * fet + m_positionY;
                if(bWithZMoving) {
                    m_positionZ = (info.end.m_positionZ - m_positionZ) * fet + m_positionZ;
                }
                break;
            }
            m_positionX = info.end.m_positionX;
            m_positionY = info.end.m_positionY;
            m_positionZ = info.end.m_positionZ;
            proc_time = info.end_time;
            removed.emplace_back(info);
            if(!ends.empty())
                SetDirection(ends[0].end);
        }
        proc_time = current_time;
        for(auto info : removed) {
            this->ends.erase(std::remove(ends.begin(), ends.end(), info), ends.end());
        }
        if(ends.empty() || this->m_positionX == (ends.back()).end.m_positionX &&
                                   m_positionY == (ends.back()).end.m_positionY &&
                                   m_positionZ == (ends.back()).end.m_positionZ) {
            bIsMoving = false;
            ends.clear();
            res = true;
        } else {
            res = false;
        }
    } else {
        res = false;
    }
    return res;
}

void ArMoveVector::SetMultipleMove(std::vector<Position> _to, uint8_t _speed, uint _start_time, uint current_time)
{
    ends.clear();
    if (!_to.empty()) {
        speed = _speed;
        uint ct = _start_time;
        if (ct == 0)
            ct = sWorld->GetArTime();

        start_time = ct;
        proc_time  = ct;
        start_time = ct;
        SetDirection(_to[0]);
        float before_x = m_positionX;
        float before_y = m_positionY;

        for (auto pos : _to) {
            float cx = pos.m_positionX - before_x;
            float cy = pos.m_positionY - before_y;

            before_x = pos.m_positionX;
            before_y = pos.m_positionY;

            float length   = sqrt(cy * cy + cx * cx);
            auto  end_time = (uint)(length / ((float)this->speed / 30.0f) + (float)start_time);

            MoveInfo mi(pos, end_time);
            ends.emplace_back(mi);
            start_time = end_time;
        }

        bIsMoving = true;
    }
}

void ArMoveVector::SetMove(Position _to, uint8_t _speed, uint _start_time, uint current_time)
{
    double v9{ }, v10{ }, lengtha{ }, lengthb{ };
    ends.clear();
    speed = _speed;
    uint st = _start_time;
    if (_start_time == 0)
        st = sWorld->GetArTime();

    start_time = st;
    proc_time  = st;
    float X = _to.m_positionX;
    float Y = _to.m_positionY;
    SetDirection(_to);
    lengtha = sqrt(Y * Y + X * X);
    lengthb = speed;
    v9 = lengtha / (lengthb / 30.0);
    v10 = start_time;
    if((start_time & 0x80000000) != 0)
        v10 = v10 + 4294967300.0;

    MoveInfo mi(_to, start_time);
    mi.end_time = (uint)(v9+v10);
    ends.emplace_back(mi);
    bIsMoving = true;
}

void ArMoveVector::SetDirection(Position pos)
{
    float px{ }, py {};
    px = pos.m_positionX - m_positionX;
    py = pos.m_positionY - m_positionY;
    if(0.0 != px || 0.0 != py) {
        bHasDirectionChanged = true;
        direction.m_positionX = pos.m_positionX;
        direction.m_positionY = pos.m_positionY;
        direction.m_positionZ = pos.m_positionZ;
        _orientation = (float)atan2(py, px);
    }
}

Position ArMoveVector::GetTargetPos()
{
    Position result{ };
    if(!ends.empty())
        result = (ends.back()).end;
    else
        result = this->GetPosition();
    return result;
}

bool ArMoveVector::IsMoving(uint t)
{
    if(bIsMoving && !ends.empty())
        return t < (ends.back()).end_time;
    else
        return false;
}
