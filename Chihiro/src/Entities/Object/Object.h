#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "Common.h"
//#include "ArRegion.h"
#include "Dynamic/UnorderedMap.h"
#include "Encryption/ByteBuffer.h"
#include "Util.h"

// used for creating values for respawn for example
#define MAKE_PAIR64(l, h)  uint64(uint32(l) | (uint64(h) << 32))
#define PAIR64_HIPART(x)   (uint32)((uint64(x) >> 32) & UI64LIT(0x00000000FFFFFFFF))
#define PAIR64_LOPART(x)   (uint32)(uint64(x)         & UI64LIT(0x00000000FFFFFFFF))

#define PAIR32_HIPART(x)   (uint16)((uint32(x) >> 16) & 0x0000FFFF)
#define PAIR32_LOPART(x)   (uint16)(uint32(x)         & 0x0000FFFF)

typedef unsigned long  DWORD;
typedef unsigned short WORD;
#define LOWORD(a) ((WORD)(a))
#define HIWORD(a) ((WORD)(((DWORD)(a) >> 16) & 0xFFFF))

enum ObjType : int {
    OBJ_STATIC  = 0, // Player (Pyrok)
    OBJ_MOVABLE = 1, // NPC (Pyrok)
    OBJ_CLIENT  = 2 // Static (Pyrok)
};

enum MainType : int {
    MT_Player       = 0,
    MT_NPC          = 1,
    MT_StaticObject = 2
};

enum SubType : int {
    ST_Player    = 0,
    ST_NPC       = 1,
    ST_Object    = 2, // Also Item
    ST_Mob       = 3,
    ST_Summon    = 4,
    ST_SkillProp = 5,
    ST_FieldProp = 6,
    ST_Pet       = 7,
};

enum EUnitFields {
    UNIT_FIELD_HANDLE                = 0x0000, // Size: 1
    UNIT_FIELD_ACCOUNT_ID            = 0x0001, // Size: 1
    UNIT_FIELD_LEVEL                 = 0x0002, // Size: 1
    UNIT_FIELD_STATUS                = 0x0003, // Size: 1>
    UNIT_FIELD_PERMISSION            = 0x0004, // Size: 1
    UNIT_FIELD_JOBPOINT              = 0x0005, // Size: 1
    UNIT_FIELD_HEALTH                = 0x0006, // Size: 1
    UNIT_FIELD_MANA                  = 0x0007, // Size: 1
    UNIT_FIELD_MAX_HEALTH            = 0x0008, // Size: 1
    UNIT_FIELD_MAX_MANA              = 0x0009, // Size: 1
    UNIT_FIELD_STAMINA               = 0x000A, // Size: 1
    UNIT_FIELD_PARTY_ID              = 0x000B, // Size: 1
    UNIT_FIELD_GUILD_ID              = 0x000C, // Size: 1
    UNIT_FIELD_RACE                  = 0x000D, // Size: 1
    UNIT_FIELD_SEX                   = 0x000E, // Size: 1
    UNIT_FIELD_PREV_JOB              = 0x000F, // Size: 3
    UNIT_FIELD_PREV_JLV              = 0x0012, // Size: 3
    UNIT_FIELD_HAVOC                 = 0x0015, // Size: 1
    UNIT_FIELD_JOB_DEPTH             = 0x0016, // Size: 1
    UNIT_FIELD_IP                    = 0x0017, // Size: 1
    UNIT_FIELD_CHA                   = 0x0018, // Size: 1
    UNIT_FIELD_PKC                   = 0x0019, // Size: 1
    UNIT_FIELD_DKC                   = 0x001A, // Size: 1
    UNIT_FIELD_SKIN_COLOR            = 0x001B, // Size: 1
    UNIT_FIELD_SUMMON                = 0x001C, // Size: 6
    UNIT_FIELD_BELT                  = 0x0023, // Size: 6
    UNIT_FIELD_GOLD                  = 0x0029, // Size: 1
    UNIT_FIELD_CHAOS                 = 0x002A, // Size: 1
    UNIT_FIELD_ACTIVE_SUMMON         = 0x002B, // Size: 1
    UNIT_FIELD_REMAIN_SUMMON_TIME    = 0x002D, // Size: 1
    UNIT_FIELD_PET                   = 0x002E, // Size: 1
    UNIT_FIELD_CHAT_BLOCK_TIME       = 0x002F, // Size: 2
    UNIT_FIELD_GUILD_BLOCK_TIME      = 0x0031, // Size: 2
    UNIT_FIELD_PK_MODE               = 0x0033, // Size: 1
    UNIT_FIELD_MODEL                 = 0x0034, // Size: 5
    UNIT_FIELD_JOB                   = 0x0039, // Size: 1
    UNIT_FIELD_JLV                   = 0x003A, // Size: 1
    UNIT_FIELD_MAX_HEALTH_MODIFIER   = 0x003B, // Size: 1
    UNIT_FIELD_MAX_MANA_MODIFIER     = 0x003C, // Size: 1
    UNIT_FIELD_HP_REGEN_MOD          = 0x003D, // Size: 1
    UNIT_FIELD_MP_REGEN_MOD          = 0x003E, // Size: 1
    UNIT_FIELD_HEAL_RATIO            = 0x003F, // Size: 1
    UNIT_FIELD_MP_HEAL_RATIO         = 0x0040, // Size: 1
    UNIT_FIELD_HEAL_RATIO_BY_ITEM    = 0x0041, // Size: 1
    UNIT_FIELD_MP_HEAL_RATIO_BY_ITEM = 0x0042, // Size: 1
    UNIT_FIELD_HEAL_RATIO_BY_REST    = 0x0043, // Size: 1
    UNIT_FIELD_MP_HEAL_RATIO_BY_REST = 0x0044, // Size: 1
    UNIT_FIELD_HATE_RATIO            = 0x0045, // Size: 1
    UNIT_FIELD_ADDITIONAL_HEAL       = 0x0046, // Size: 1
    UNIT_FIELD_ADDITIONAL_MP_HEAL    = 0x0047, // Size: 1
    UNIT_FIELD_UID                   = 0x0048, // Size: 1
    UNIT_FIELD_EXP                   = 0x0049, // Size: 2
    UNIT_FIELD_DEAD_TIME             = 0x004B, // Size: 1
    UNIT_FIELD_MAX_CHAOS             = 0x004C, // Size: 1
    UNIT_END                         = 0x004D
};

enum EBattleFields {
    BATTLE_FIELD_TARGET_HANDLE         = UNIT_END + 0x0001, // Size: 1
    BATTLE_FIELD_NEXT_ATTACKABLE_TIME  = UNIT_END + 0x0002, // Size: 2
    BATTLE_FIELD_END                   = UNIT_END + 0x0003
};

class ArRegion;

class Object {
public:
    virtual ~Object();

    bool IsInWorld() const
    { return m_inWorld; }

    bool IsDeleteRequested() const
    { return m_bDeleteRequest; }

    void DeleteThis() { m_bDeleteRequest = true; }

    virtual void AddToWorld();
    virtual void RemoveFromWorld();

    uint32 GetHandle() const
    { return GetUInt32Value(0); }

    SubType GetSubType() const
    { return _subType; }

    MainType GetMainType() const
    { return _mainType; }

    ObjType GetObjType() const
    { return _objType; }

    int32 GetInt32Value(uint16 index) const
    {
                ASSERT(index < _valuesCount || PrintIndexError(index, false));
        return m_int32Values[index];
    }

    uint32 GetUInt32Value(uint16 index) const
    {
                ASSERT(index < _valuesCount || PrintIndexError(index, false));
        return _uint32Values[index];
    }

    uint64 GetUInt64Value(uint16 index) const
    {
                ASSERT(index + 1 < _valuesCount || PrintIndexError(index, false));
        return *((uint64 *) &(_uint32Values[index]));
    }

    float GetFloatValue(uint16 index) const
    {
                ASSERT(index < _valuesCount || PrintIndexError(index, false));
        return m_floatValues[index];
    }

    uint8 GetByteValue(uint16 index, uint8 offset) const
    {
                ASSERT(index < _valuesCount || PrintIndexError(index, false));
                ASSERT(offset < 4);
        return *(((uint8 *) &_uint32Values[index]) + offset);
    }

    uint16 GetUInt16Value(uint16 index, uint8 offset) const
    {
                ASSERT(index < _valuesCount || PrintIndexError(index, false));
                ASSERT(offset < 2);
        return *(((uint16 *) &_uint32Values[index]) + offset);
    }

    void SetInt32Value(uint16 index, int32 value);

    void SetUInt32Value(uint16 index, uint32 value);

    void UpdateUInt32Value(uint16 index, uint32 value);

    void SetUInt64Value(uint16 index, uint64 value);

    void SetFloatValue(uint16 index, float value);

    void SetByteValue(uint16 index, uint8 offset, uint8 value);

    void SetUInt16Value(uint16 index, uint8 offset, uint16 value);

    void SetInt16Value(uint16 index, uint8 offset, int16 value)
    { SetUInt16Value(index, offset, (uint16) value); }

    void SetStatFloatValue(uint16 index, float value);

    void SetStatInt32Value(uint16 index, int32 value);

    bool AddUInt64Value(uint16 index, uint64 value);

    bool RemoveUInt64Value(uint16 index, uint64 value);

    void ApplyModUInt32Value(uint16 index, int32 val, bool apply);

    void ApplyModInt32Value(uint16 index, int32 val, bool apply);

    void ApplyModUInt64Value(uint16 index, int32 val, bool apply);

    void ApplyModPositiveFloatValue(uint16 index, float val, bool apply);

    void ApplyModSignedFloatValue(uint16 index, float val, bool apply);

    void SetFlag(uint16 index, uint32 newFlag);

    void RemoveFlag(uint16 index, uint32 oldFlag);

    void ToggleFlag(uint16 index, uint32 flag)
    {
        if (HasFlag(index, flag))
            RemoveFlag(index, flag);
        else
            SetFlag(index, flag);
    }

    bool HasFlag(uint16 index, uint32 flag) const
    {
        if (index >= _valuesCount && !PrintIndexError(index, false)) return false;
        return (_uint32Values[index] & flag) != 0;
    }

    void SetByteFlag(uint16 index, uint8 offset, uint8 newFlag);

    void RemoveByteFlag(uint16 index, uint8 offset, uint8 newFlag);

    void ToggleFlag(uint16 index, uint8 offset, uint8 flag)
    {
        if (HasByteFlag(index, offset, flag))
            RemoveByteFlag(index, offset, flag);
        else
            SetByteFlag(index, offset, flag);
    }

    bool HasByteFlag(uint16 index, uint8 offset, uint8 flag) const
    {
                ASSERT(index < _valuesCount || PrintIndexError(index, false));
                ASSERT(offset < 4);
        return (((uint8 *) &_uint32Values[index])[offset] & flag) != 0;
    }

    void ApplyModFlag(uint16 index, uint32 flag, bool apply)
    {
        if (apply) SetFlag(index, flag); else RemoveFlag(index, flag);
    }

    void SetFlag64(uint16 index, uint64 newFlag)
    {
        uint64 oldval = GetUInt64Value(index);
        uint64 newval = oldval | newFlag;
        SetUInt64Value(index, newval);
    }

    void RemoveFlag64(uint16 index, uint64 oldFlag)
    {
        uint64 oldval = GetUInt64Value(index);
        uint64 newval = oldval & ~oldFlag;
        SetUInt64Value(index, newval);
    }

    void ToggleFlag64(uint16 index, uint64 flag)
    {
        if (HasFlag64(index, flag))
            RemoveFlag64(index, flag);
        else
            SetFlag64(index, flag);
    }

    bool HasFlag64(uint16 index, uint64 flag) const
    {
                ASSERT(index < _valuesCount || PrintIndexError(index, false));
        return (GetUInt64Value(index) & flag) != 0;
    }

    void ApplyModFlag64(uint16 index, uint64 flag, bool apply)
    {
        if (apply) SetFlag64(index, flag); else RemoveFlag64(index, flag);
    }

    uint16 GetValuesCount() const
    { return _valuesCount; }

    virtual bool hasQuest(uint32 /* quest_id */) const
    { return false; }

protected:
    Object();

    void _InitValues();

    uint16 m_updateFlag;

    union {
        int32  *m_int32Values;
        uint32 *_uint32Values;
        float  *m_floatValues;
    };
    bool   *_changedFields;

    uint16 _valuesCount;
    bool   m_objectUpdated;

    MainType _mainType;
    ObjType  _objType;
    SubType  _subType;
private:
    bool m_inWorld;
    bool m_bDeleteRequest;

    // for output helpful error messages from asserts
    bool PrintIndexError(uint32 index, bool set) const;

    Object(const Object &);                              // prevent generation copy constructor
    Object &operator=(Object const &);                   // prevent generation assignment operator
};

struct Position {
    float  m_positionX{ };
    float  m_positionY{ };
    float  m_positionZ{ };
    float  _orientation{ };
    uint8 m_nLayer{ };

    float prevX{}, prevY{};

    bool operator==(Position pos) {
        return pos.m_positionX == m_positionX &&
                pos.m_positionY == m_positionY &&
                pos.m_positionZ == m_positionZ &&
                pos.m_nLayer == m_nLayer;

    }

    void SetCurrentXY(float x, float y) {
        if(prevX > -1) {
            prevX = (int)(GetPositionX() / 180);
            prevY = (int)(GetPositionY() / 180);
        }
        this->m_positionX = x;
        this->m_positionY = y;
    }

    uint8 GetLayer() const
    { return m_nLayer; }

    void SetLayer(uint8 value)
    { m_nLayer = value; }

    void Relocate(float x, float y)
    {
        m_positionX = x;
        m_positionY = y;
    }

    void Relocate(float x, float y, float z)
    {
        m_positionX = x;
        m_positionY = y;
        m_positionZ = z;
    }

    void Relocate(float x, float y, float z, float orientation)
    {
        m_positionX  = x;
        m_positionY  = y;
        m_positionZ  = z;
        _orientation = orientation;
    }

    void Relocate(const Position &pos)
    {
        m_positionX  = pos.m_positionX;
        m_positionY  = pos.m_positionY;
        m_positionZ  = pos.m_positionZ;
        _orientation = pos._orientation;
    }

    void Relocate(const Position *pos)
    {
        m_positionX  = pos->m_positionX;
        m_positionY  = pos->m_positionY;
        m_positionZ  = pos->m_positionZ;
        _orientation = pos->_orientation;
    }

    void SetOrientation(float orientation)
    {
        _orientation = orientation;
    }

    float GetPositionX() const
    { return m_positionX; }

    float GetPositionY() const
    { return m_positionY; }

    float GetPositionZ() const
    { return m_positionZ; }

    float GetOrientation() const
    { return _orientation; }

    void GetPosition(float &x, float &y) const
    {
        x = m_positionX;
        y = m_positionY;
    }

    void GetPosition(float &x, float &y, float &z) const
    {
        x = m_positionX;
        y = m_positionY;
        z = m_positionZ;
    }

    void GetPosition(float &x, float &y, float &z, float &o) const
    {
        x = m_positionX;
        y = m_positionY;
        z = m_positionZ;
        o = _orientation;
    }

    void GetPosition(Position *pos) const
    {
        if (pos)
            pos->Relocate(m_positionX, m_positionY, m_positionZ, _orientation);
    }

    Position GetPosition() const
    {
        Position pos{ };
        pos.m_positionX = GetPositionX();
        pos.m_positionY = GetPositionY();
        pos.m_positionZ = GetPositionZ();
        pos._orientation = GetOrientation();
        return pos;
    }

    float GetExactDist2dSq(float x, float y) const
    {
        float dx = m_positionX - x;
        float dy = m_positionY - y;
        return dx * dx + dy * dy;
    }

    float GetExactDist2d(const float x, const float y) const
    {
        return sqrt(GetExactDist2dSq(x, y));
    }

    float GetExactDist2dSq(const Position *pos) const
    {
        float dx = m_positionX - pos->m_positionX;
        float dy = m_positionY - pos->m_positionY;
        return dx * dx + dy * dy;
    }

    float GetExactDist2d(const Position *pos) const
    {
        float tmp = sqrt(GetExactDist2dSq(pos));
        return tmp;
    }

    float GetExactDistSq(float x, float y, float z) const
    {
        float dz = m_positionZ - z;
        return GetExactDist2dSq(x, y) + dz * dz;
    }

    float GetExactDist(float x, float y, float z) const
    {
        return sqrt(GetExactDistSq(x, y, z));
    }

    float GetExactDistSq(const Position *pos) const
    {
        float dx = m_positionX - pos->m_positionX;
        float dy = m_positionY - pos->m_positionY;
        float dz = m_positionZ - pos->m_positionZ;
        return dx * dx + dy * dy + dz * dz;
    }

    float GetExactDist(const Position *pos) const
    {
        return sqrt(GetExactDistSq(pos));
    }

    std::string ToString() const
    {
        return string_format("Position: X: %u, Y: %u, Layer: %u", GetPositionX(), GetPositionY(), GetLayer());
    }
};


class ArMoveVector : public Position {
public:
    ArMoveVector() = default;
    explicit ArMoveVector(const ArMoveVector*);
    ~ArMoveVector() = default;

    virtual bool Step(uint current_time);
    virtual void SetMultipleMove(std::vector<Position> _to, uint8_t _speed, uint _start_time, uint current_time);
    virtual void SetMove(Position _to, uint8_t _speed, uint _start_time, uint current_time);
    void SetDirection(Position pos);
    Position GetTargetPos();
    bool IsMoving(uint t);
    void StopMove() { ends.clear(); bIsMoving = false; }

    struct MoveInfo {
        MoveInfo(Position pos, uint t) {
            end = Position(pos);
            end_time = t;
        }

        bool operator==(MoveInfo mv) {
            return mv.end == end && mv.end_time == end_time;
        }
        Position end{ };
        uint end_time{ };
    };

public:
    bool bIsMoving{false};
    bool bWithZMoving{false};
    uint8_t speed{ };
    uint start_time{ };
    uint proc_time{ };
    std::vector<MoveInfo> ends{ };
    bool bHasDirectionChanged{false};
    Position direction{ };
    uint lastStepTime{};
};

class Player;

class WorldObject : public Object, public ArMoveVector {
public:
    ~WorldObject() override;

    virtual void Update(uint32 /*time_diff*/)
    {}

    bool SetPendingMove(std::vector<Position> vMoveInfo, uint8_t speed);
    bool Step(uint tm) override;


    void SendEnterMsg(Player *);
    void AddNoise(int, int, int);

    void AddToWorld() override;
    void RemoveFromWorld() override;

    Position GetCurrentPosition(uint t)
    {
        Position result{ };
        ArMoveVector _mv { };
        if(bIsMoving && IsInWorld()) {
            _mv = ArMoveVector(this);
            _mv.Step(t);
            result.m_positionX = _mv.GetPositionX();
            result.m_positionY = _mv.GetPositionY();
            result.m_positionZ = _mv.GetPositionZ();
            result._orientation = _mv.GetOrientation();
        } else {
            result.m_positionX = GetPositionX();
            result.m_positionY = GetPositionY();
            result.m_positionZ = GetPositionZ();
            result._orientation = GetOrientation();
        }
        return result;
    }

    const char *GetName() const
    { return m_name.c_str(); }

    void SetName(const std::string &newname)
    { m_name = newname; }

    virtual const char *GetNameForLocaleIdx(LocaleConstant /*locale_idx*/) const
    { return GetName(); }

    ArRegion *_region;
    bool _bIsInWorld{false};

protected:
    explicit WorldObject(bool isWorldObject); //note: here it means if it is in grid object list or world object list
    std::vector<Position> m_PendingMovePos{};
    uint8_t m_nPendingMoveSpeed{};
    uint lastProcessTime{0};
    std::string m_name;
    bool        _isActive;
    const bool  m_isWorldObject;

};

#endif // _OBJECT_H_