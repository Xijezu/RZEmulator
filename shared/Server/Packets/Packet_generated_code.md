### Example of complex generated structure and packet (TS_SC_SKILL)
#### Packet declaration
```cpp
// Some enum with specified underlying type (after ":")
enum TS_SKILL__TYPE : uint8_t
{
	ST_Fire = 0,
	ST_Casting = 1,
	ST_CastingUpdate = 2,
	ST_Cancel = 3,
	ST_RegionFire = 4,
	ST_Complete = 5
};

enum TS_SKILL__HIT_TYPE : uint8_t
{
	SHT_DAMAGE = 0,
	SHT_MAGIC_DAMAGE = 1,
	SHT_DAMAGE_WITH_KNOCK_BACK = 2,
	SHT_RESULT = 10,
	SHT_ADD_HP = 20,
	SHT_ADD_MP = 21,
	SHT_ADD_HP_MP_SP = 22,
	SHT_REBIRTH = 23,
	SHT_RUSH = 30,
	SHT_CHAIN_DAMAGE = 40,
	SHT_CHAIN_MAGIC_DAMAGE = 41,
	SHT_CHAIN_HEAL = 42,
	SHT_NOT_USE = 100
};

enum TS_SKILL__DAMAGE_TYPE : uint8_t
{
	SDT_TYPE_NONE = 0,
	SDT_TYPE_FIRE = 1,
	SDT_TYPE_WATER = 2,
	SDT_TYPE_WIND = 3,
	SDT_TYPE_EARTH = 4,
	SDT_TYPE_LIGHT = 5,
	SDT_TYPE_DARK = 6,
	SDT_TYPE_COUNT = 7
};

enum TS_SKILL_RESULT_SUCESS_TYPE : int32_t {
	SRST_AddState = 10,
	SRST_RemoveState = 11,
	SRST_AddHate = 12,
	SRST_TurnOn = 21,
	SRST_TurnOff = 22,
	SRST_SummonDead = 30,
	SRST_TargetDead = 31,
	SRST_CreateItem = 40,
	SRST_RespawnMonster = 41
};

// Structure definitions
#define TS_SC_SKILL__HIT_DAMAGE_INFO_DEF(_) \
	_(simple) (int32_t, target_hp) \
	_(simple) (TS_SKILL__DAMAGE_TYPE, damage_type) \
	_(simple) (int32_t, damage) \
	_(simple) (int32_t, flag) \
	_(array)  (uint16_t, elemental_damage, 7)
CREATE_STRUCT(TS_SC_SKILL__HIT_DAMAGE_INFO);

#define TS_SC_SKILL__HIT_DAMAGE_DEF(_) \
	_(simple) (TS_SC_SKILL__HIT_DAMAGE_INFO, damage)
CREATE_STRUCT(TS_SC_SKILL__HIT_DAMAGE);

#define TS_SC_SKILL__HIT_DAMAGE_WITH_KNOCKBACK_DEF(_) \
	_(simple) (TS_SC_SKILL__HIT_DAMAGE_INFO, damage) \
	_(simple) (float, x) \
	_(simple) (float, y) \
	_(simple)(int8_t, speed) \
	_(simple) (uint32_t, knock_back_time)
CREATE_STRUCT(TS_SC_SKILL__HIT_DAMAGE_WITH_KNOCKBACK);

#define TS_SC_SKILL__HIT_RESULT_DEF(_) \
	_(simple) (bool, bResult) \
	_(simple) (TS_SKILL_RESULT_SUCESS_TYPE, success_type)
CREATE_STRUCT(TS_SC_SKILL__HIT_RESULT);

#define TS_SC_SKILL__HIT_ADD_STAT_DEF(_) \
	_(simple) (int32_t, target_stat) \
	_(simple) (int32_t, nIncStat)
CREATE_STRUCT(TS_SC_SKILL__HIT_ADD_STAT);

#define TS_SC_SKILL__HIT_ADDHPMPSP_DEF(_) \
	_(simple) (int32_t, target_hp) \
	_(simple) (int32_t, nIncHP) \
	_(simple) (int32_t, nIncMP) \
	_(simple) (int32_t, nIncSP) \
	_(def)(simple)(int32_t, target_mp) \
	  _(impl)(simple)(int32_t, target_mp, version >= EPIC_7_1) \
	  _(impl)(simple)(int16_t, target_mp, version < EPIC_7_1)
CREATE_STRUCT(TS_SC_SKILL__HIT_ADDHPMPSP);

#define TS_SC_SKILL__HIT_REBIRTH_DEF(_) \
	_(simple) (int32_t, target_hp) \
	_(simple) (int32_t, nIncHP) \
	_(simple) (int32_t, nIncMP) \
	_(simple) (int32_t, nRecoveryEXP) \
	_(def)(simple)(int32_t, target_mp) \
	  _(impl)(simple)(int32_t, target_mp, version >= EPIC_7_1) \
	  _(impl)(simple)(int16_t, target_mp, version < EPIC_7_1)
CREATE_STRUCT(TS_SC_SKILL__HIT_REBIRTH);

#define TS_SC_SKILL__HIT_RUSH_DEF(_) \
	_(simple) (bool, bResult) \
	_(simple) (float, x) \
	_(simple) (float, y) \
	_(simple) (float, face) \
	_(simple) (int8_t, speed)
CREATE_STRUCT(TS_SC_SKILL__HIT_RUSH);

#define TS_SC_SKILL__HIT_CHAIN_DAMAGE_DEF(_) \
	_(simple) (TS_SC_SKILL__HIT_DAMAGE_INFO, damage) \
	_(simple) (uint32_t, hFrom)
CREATE_STRUCT(TS_SC_SKILL__HIT_CHAIN_DAMAGE);

#define TS_SC_SKILL__HIT_CHAIN_HEAL_DEF(_) \
	_(simple) (int32_t, target_hp) \
	_(simple) (int32_t, nIncHP) \
	_(simple) (uint32_t, hFrom)
CREATE_STRUCT(TS_SC_SKILL__HIT_CHAIN_HEAL);

#define TS_SC_SKILL__HIT_DETAILS_DEF(_) \
	_(simple) (TS_SKILL__HIT_TYPE, type) \
	_(simple) (uint32_t, hTarget) \
	_(simple) (TS_SC_SKILL__HIT_DAMAGE               , hitDamage             , type == SHT_DAMAGE || type == SHT_MAGIC_DAMAGE) \
	_(simple) (TS_SC_SKILL__HIT_DAMAGE_WITH_KNOCKBACK, hitDamageWithKnockBack, type == SHT_DAMAGE_WITH_KNOCK_BACK) \
	_(simple) (TS_SC_SKILL__HIT_RESULT               , hitResult             , type == SHT_RESULT) \
	_(simple) (TS_SC_SKILL__HIT_ADD_STAT             , hitAddStat            , type == SHT_ADD_HP || type == SHT_ADD_MP) \
	_(simple) (TS_SC_SKILL__HIT_ADDHPMPSP            , hitAddHPMPSP          , type == SHT_ADD_HP_MP_SP) \
	_(simple) (TS_SC_SKILL__HIT_REBIRTH              , hitRebirth            , type == SHT_REBIRTH) \
	_(simple) (TS_SC_SKILL__HIT_RUSH                 , hitRush               , type == SHT_RUSH) \
	_(simple) (TS_SC_SKILL__HIT_CHAIN_DAMAGE         , hitChainDamage        , type == SHT_CHAIN_DAMAGE || type == SHT_CHAIN_MAGIC_DAMAGE) \
	_(simple) (TS_SC_SKILL__HIT_CHAIN_HEAL           , hitChainHeal          , type == SHT_CHAIN_HEAL)
CREATE_STRUCT(TS_SC_SKILL__HIT_DETAILS);

#define TS_SC_SKILL__FIRE_DEF(_) \
	_(simple) (bool, bMultiple) \
	_(simple) (float, range) \
	_(simple) (int8_t, target_count) \
	_(simple) (int8_t, fire_count) \
	_(count)  (uint16_t, hits) \
	_(padmarker)(before_hit_marker) \
	_(dynarray)(TS_SC_SKILL__HIT_DETAILS, hits) \
	_(pad)    (45*(uint32_t)hits.size(), before_hit_marker) /* weird fixed padding with fire struct in assembleMessage */
CREATE_STRUCT(TS_SC_SKILL__FIRE);

#define TS_SC_SKILL__CAST_DEF(_) \
	_(simple) (uint32_t, tm) \
	_(simple) (uint16_t, nErrorCode)
CREATE_STRUCT(TS_SC_SKILL__CAST);

// Packet definition
#define TS_SC_SKILL_DEF(_) \
	_(simple) (uint16_t, skill_id) \
	_(simple) (uint8_t, skill_level) \
	_(simple) (uint32_t, caster) \
	_(simple) (uint32_t, target) \
	_(simple) (float, x) \
	_(simple) (float, y) \
	_(simple) (float, z) \
	_(simple) (uint8_t, layer) \
	_(simple) (TS_SKILL__TYPE, type) \
	_(def)(simple)(int32_t, hp_cost) \
	  _(impl)(simple)(int32_t, hp_cost, version >= EPIC_7_3) \
	  _(impl)(simple)(int16_t, hp_cost, version < EPIC_7_3) \
	_(def)(simple)(int32_t, mp_cost) \
	  _(impl)(simple)(int32_t, mp_cost, version >= EPIC_7_3) \
	  _(impl)(simple)(int16_t, mp_cost, version < EPIC_7_3) \
	_(simple) (int32_t, caster_hp) \
	_(def)(simple)(int32_t, caster_mp) \
	  _(impl)(simple)(int32_t, caster_mp, version >= EPIC_7_3) \
	  _(impl)(simple)(int16_t, caster_mp, version < EPIC_7_3) \
	_(padmarker)(skill_type_marker) \
	_(simple) (TS_SC_SKILL__FIRE , fire   , type == ST_Fire || type == ST_RegionFire) \
	_(simple) (TS_SC_SKILL__CAST , casting, type == ST_Casting || type == ST_CastingUpdate) \
	_(pad)    (9, skill_type_marker) /* padding to match fire size */

CREATE_PACKET(TS_SC_SKILL, 401);
```
#### Generated code
```cpp
enum TS_SKILL__TYPE : uint8_t {
  ST_Fire = 0,
  ST_Casting = 1,
  ST_CastingUpdate = 2,
  ST_Cancel = 3,
  ST_RegionFire = 4,
  ST_Complete = 5
};

enum TS_SKILL__HIT_TYPE : uint8_t {
  SHT_DAMAGE = 0,
  SHT_MAGIC_DAMAGE = 1,
  SHT_DAMAGE_WITH_KNOCK_BACK = 2,
  SHT_RESULT = 10,
  SHT_ADD_HP = 20,
  SHT_ADD_MP = 21,
  SHT_ADD_HP_MP_SP = 22,
  SHT_REBIRTH = 23,
  SHT_RUSH = 30,
  SHT_CHAIN_DAMAGE = 40,
  SHT_CHAIN_MAGIC_DAMAGE = 41,
  SHT_CHAIN_HEAL = 42,
  SHT_NOT_USE = 100
};

enum TS_SKILL__DAMAGE_TYPE : uint8_t {
  SDT_TYPE_NONE = 0,
  SDT_TYPE_FIRE = 1,
  SDT_TYPE_WATER = 2,
  SDT_TYPE_WIND = 3,
  SDT_TYPE_EARTH = 4,
  SDT_TYPE_LIGHT = 5,
  SDT_TYPE_DARK = 6,
  SDT_TYPE_COUNT = 7
};

enum TS_SKILL_RESULT_SUCESS_TYPE : int32_t {
  SRST_AddState = 10,
  SRST_RemoveState = 11,
  SRST_AddHate = 12,
  SRST_TurnOn = 21,
  SRST_TurnOff = 22,
  SRST_SummonDead = 30,
  SRST_TargetDead = 31,
  SRST_CreateItem = 40,
  SRST_RespawnMonster = 41
};

struct TS_SC_SKILL__HIT_DAMAGE_INFO {
  static inline const char *getName() {
    return "TS_SC_SKILL__HIT_DAMAGE_INFO";
  };
  int32_t target_hp;
  TS_SKILL__DAMAGE_TYPE damage_type;
  int32_t damage;
  int32_t flag;
  uint16_t elemental_damage[7];

  uint32_t getSize(int version) const {
    uint32_t size = 0;
    (void)(version); // This is to avoid a compiler warning if "version" is not used

    size += PacketDeclaration::getSizeOf((int32_t)target_hp, version);
    size += PacketDeclaration::getSizeOf((TS_SKILL__DAMAGE_TYPE)damage_type,
                                         version);
    size += PacketDeclaration::getSizeOf((int32_t)damage, version);
    size += PacketDeclaration::getSizeOf((int32_t)flag, version);
    for (int i = 0; i < 7; ++i)
      size +=
          PacketDeclaration::getSizeOf((uint16_t)elemental_damage[i], version);

    return size;
  }
  template <class T> void serialize(T *buffer) const {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->write("target_hp", (int32_t)target_hp);
    buffer->write("damage_type", (TS_SKILL__DAMAGE_TYPE)damage_type);
    buffer->write("damage", (int32_t)damage);
    buffer->write("flag", (int32_t)flag);
    buffer->template writeArray<uint16_t>("elemental_damage", elemental_damage,
                                          7);
  }
  template <class T> void deserialize(T *buffer) {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->template read<int32_t>("target_hp", target_hp);
    buffer->template read<TS_SKILL__DAMAGE_TYPE>("damage_type", damage_type);
    buffer->template read<int32_t>("damage", damage);
    buffer->template read<int32_t>("flag", flag);
    buffer->template readArray<uint16_t>("elemental_damage", elemental_damage,
                                         7);
  }
};

struct TS_SC_SKILL__HIT_DAMAGE {
  static inline const char *getName() { return "TS_SC_SKILL__HIT_DAMAGE"; };
  TS_SC_SKILL__HIT_DAMAGE_INFO damage;

  uint32_t getSize(int version) const {
    uint32_t size = 0;
    (void)(version);

    size += PacketDeclaration::getSizeOf((TS_SC_SKILL__HIT_DAMAGE_INFO)damage,
                                         version);
    return size;
  }
  template <class T> void serialize(T *buffer) const {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->write("damage", (TS_SC_SKILL__HIT_DAMAGE_INFO)damage);
  }
  template <class T> void deserialize(T *buffer) {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->template read<TS_SC_SKILL__HIT_DAMAGE_INFO>("damage", damage);
  }
};

struct TS_SC_SKILL__HIT_DAMAGE_WITH_KNOCKBACK {
  static inline const char *getName() {
    return "TS_SC_SKILL__HIT_DAMAGE_WITH_KNOCKBACK";
  };
  TS_SC_SKILL__HIT_DAMAGE_INFO damage;
  float x;
  float y;
  int8_t speed;
  uint32_t knock_back_time;

  uint32_t getSize(int version) const {
    uint32_t size = 0;
    (void)(version);

    size += PacketDeclaration::getSizeOf((TS_SC_SKILL__HIT_DAMAGE_INFO)damage,
                                         version);
    size += PacketDeclaration::getSizeOf((float)x, version);
    size += PacketDeclaration::getSizeOf((float)y, version);
    size += PacketDeclaration::getSizeOf((int8_t)speed, version);
    size += PacketDeclaration::getSizeOf((uint32_t)knock_back_time, version);

    return size;
  }
  template <class T> void serialize(T *buffer) const {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->write("damage", (TS_SC_SKILL__HIT_DAMAGE_INFO)damage);
    buffer->write("x", (float)x);
    buffer->write("y", (float)y);
    buffer->write("speed", (int8_t)speed);
    buffer->write("knock_back_time", (uint32_t)knock_back_time);
  }
  template <class T> void deserialize(T *buffer) {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->template read<TS_SC_SKILL__HIT_DAMAGE_INFO>("damage", damage);
    buffer->template read<float>("x", x);
    buffer->template read<float>("y", y);
    buffer->template read<int8_t>("speed", speed);
    buffer->template read<uint32_t>("knock_back_time", knock_back_time);
  }
};

struct TS_SC_SKILL__HIT_RESULT {
  static inline const char *getName() { return "TS_SC_SKILL__HIT_RESULT"; };
  bool bResult;
  TS_SKILL_RESULT_SUCESS_TYPE success_type;

  uint32_t getSize(int version) const {
    uint32_t size = 0;
    (void)(version);

    size += PacketDeclaration::getSizeOf((bool)bResult, version);
    size += PacketDeclaration::getSizeOf(
        (TS_SKILL_RESULT_SUCESS_TYPE)success_type, version);

    return size;
  }
  template <class T> void serialize(T *buffer) const {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->write("bResult", (bool)bResult);
    buffer->write("success_type", (TS_SKILL_RESULT_SUCESS_TYPE)success_type);
  }
  template <class T> void deserialize(T *buffer) {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->template read<bool>("bResult", bResult);
    buffer->template read<TS_SKILL_RESULT_SUCESS_TYPE>("success_type",
                                                       success_type);
  }
};

struct TS_SC_SKILL__HIT_ADD_STAT {
  static inline const char *getName() { return "TS_SC_SKILL__HIT_ADD_STAT"; };
  int32_t target_stat;
  int32_t nIncStat;

  uint32_t getSize(int version) const {
    uint32_t size = 0;
    (void)(version);

    size += PacketDeclaration::getSizeOf((int32_t)target_stat, version);
    size += PacketDeclaration::getSizeOf((int32_t)nIncStat, version);

    return size;
  }
  template <class T> void serialize(T *buffer) const {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->write("target_stat", (int32_t)target_stat);
    buffer->write("nIncStat", (int32_t)nIncStat);
  }
  template <class T> void deserialize(T *buffer) {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->template read<int32_t>("target_stat", target_stat);
    buffer->template read<int32_t>("nIncStat", nIncStat);
  }
};
# 95 "TS_SC_SKILL.h"
struct TS_SC_SKILL__HIT_ADDHPMPSP {
  static inline const char *getName() { return "TS_SC_SKILL__HIT_ADDHPMPSP"; };
  int32_t target_hp;
  int32_t nIncHP;
  int32_t nIncMP;
  int32_t nIncSP;
  int32_t target_mp;

  uint32_t getSize(int version) const {
    uint32_t size = 0;
    (void)(version);

    size += PacketDeclaration::getSizeOf((int32_t)target_hp, version);
    size += PacketDeclaration::getSizeOf((int32_t)nIncHP, version);
    size += PacketDeclaration::getSizeOf((int32_t)nIncMP, version);
    size += PacketDeclaration::getSizeOf((int32_t)nIncSP, version);
    if (version >= EPIC_7_1)
      size += PacketDeclaration::getSizeOf((int32_t)target_mp, version);
    if (version < EPIC_7_1)
      size += PacketDeclaration::getSizeOf((int16_t)target_mp, version);

    return size;
  }
  template <class T> void serialize(T *buffer) const {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->write("target_hp", (int32_t)target_hp);
    buffer->write("nIncHP", (int32_t)nIncHP);
    buffer->write("nIncMP", (int32_t)nIncMP);
    buffer->write("nIncSP", (int32_t)nIncSP);
    if (version >= EPIC_7_1)
      buffer->write("target_mp", (int32_t)target_mp);
    if (version < EPIC_7_1)
      buffer->write("target_mp", (int16_t)target_mp);
  }
  template <class T> void deserialize(T *buffer) {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->template read<int32_t>("target_hp", target_hp);
    buffer->template read<int32_t>("nIncHP", nIncHP);
    buffer->template read<int32_t>("nIncMP", nIncMP);
    buffer->template read<int32_t>("nIncSP", nIncSP);
    if (version >= EPIC_7_1)
      buffer->template read<int32_t>("target_mp", target_mp);
    if (version < EPIC_7_1)
      buffer->template read<int16_t>("target_mp", target_mp);
  }
};
# 105 "TS_SC_SKILL.h"
struct TS_SC_SKILL__HIT_REBIRTH {
  static inline const char *getName() { return "TS_SC_SKILL__HIT_REBIRTH"; };
  int32_t target_hp;
  int32_t nIncHP;
  int32_t nIncMP;
  int32_t nRecoveryEXP;
  int32_t target_mp;

  uint32_t getSize(int version) const {
    uint32_t size = 0;
    (void)(version);

    size += PacketDeclaration::getSizeOf((int32_t)target_hp, version);
    size += PacketDeclaration::getSizeOf((int32_t)nIncHP, version);
    size += PacketDeclaration::getSizeOf((int32_t)nIncMP, version);
    size += PacketDeclaration::getSizeOf((int32_t)nRecoveryEXP, version);
    if (version >= EPIC_7_1)
      size += PacketDeclaration::getSizeOf((int32_t)target_mp, version);
    if (version < EPIC_7_1)
      size += PacketDeclaration::getSizeOf((int16_t)target_mp, version);

    return size;
  }
  template <class T> void serialize(T *buffer) const {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->write("target_hp", (int32_t)target_hp);
    buffer->write("nIncHP", (int32_t)nIncHP);
    buffer->write("nIncMP", (int32_t)nIncMP);
    buffer->write("nRecoveryEXP", (int32_t)nRecoveryEXP);
    if (version >= EPIC_7_1)
      buffer->write("target_mp", (int32_t)target_mp);
    if (version < EPIC_7_1)
      buffer->write("target_mp", (int16_t)target_mp);
  }
  template <class T> void deserialize(T *buffer) {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->template read<int32_t>("target_hp", target_hp);
    buffer->template read<int32_t>("nIncHP", nIncHP);
    buffer->template read<int32_t>("nIncMP", nIncMP);
    buffer->template read<int32_t>("nRecoveryEXP", nRecoveryEXP);
    if (version >= EPIC_7_1)
      buffer->template read<int32_t>("target_mp", target_mp);
    if (version < EPIC_7_1)
      buffer->template read<int16_t>("target_mp", target_mp);
  }
};

struct TS_SC_SKILL__HIT_RUSH {
  static inline const char *getName() { return "TS_SC_SKILL__HIT_RUSH"; };
  bool bResult;
  float x;
  float y;
  float face;
  int8_t speed;

  uint32_t getSize(int version) const {
    uint32_t size = 0;
    (void)(version);

    size += PacketDeclaration::getSizeOf((bool)bResult, version);
    size += PacketDeclaration::getSizeOf((float)x, version);
    size += PacketDeclaration::getSizeOf((float)y, version);
    size += PacketDeclaration::getSizeOf((float)face, version);
    size += PacketDeclaration::getSizeOf((int8_t)speed, version);

    return size;
  }
  template <class T> void serialize(T *buffer) const {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->write("bResult", (bool)bResult);
    buffer->write("x", (float)x);
    buffer->write("y", (float)y);
    buffer->write("face", (float)face);
    buffer->write("speed", (int8_t)speed);
  }
  template <class T> void deserialize(T *buffer) {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->template read<bool>("bResult", bResult);
    buffer->template read<float>("x", x);
    buffer->template read<float>("y", y);
    buffer->template read<float>("face", face);
    buffer->template read<int8_t>("speed", speed);
  }
};

struct TS_SC_SKILL__HIT_CHAIN_DAMAGE {
  static inline const char *getName() {
    return "TS_SC_SKILL__HIT_CHAIN_DAMAGE";
  };
  TS_SC_SKILL__HIT_DAMAGE_INFO damage;
  uint32_t hFrom;

  uint32_t getSize(int version) const {
    uint32_t size = 0;
    (void)(version);

    size += PacketDeclaration::getSizeOf((TS_SC_SKILL__HIT_DAMAGE_INFO)damage,
                                         version);
    size += PacketDeclaration::getSizeOf((uint32_t)hFrom, version);
    ;
    return size;
  }
  template <class T> void serialize(T *buffer) const {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->write("damage", (TS_SC_SKILL__HIT_DAMAGE_INFO)damage);
    buffer->write("hFrom", (uint32_t)hFrom);
  }
  template <class T> void deserialize(T *buffer) {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->template read<TS_SC_SKILL__HIT_DAMAGE_INFO>("damage", damage);
    buffer->template read<uint32_t>("hFrom", hFrom);
  }
};

struct TS_SC_SKILL__HIT_CHAIN_HEAL {
  static inline const char *getName() { return "TS_SC_SKILL__HIT_CHAIN_HEAL"; };
  int32_t target_hp;
  int32_t nIncHP;
  uint32_t hFrom;

  uint32_t getSize(int version) const {
    uint32_t size = 0;
    (void)(version);

    size += PacketDeclaration::getSizeOf((int32_t)target_hp, version);
    size += PacketDeclaration::getSizeOf((int32_t)nIncHP, version);
    size += PacketDeclaration::getSizeOf((uint32_t)hFrom, version);

    return size;
  }
  template <class T> void serialize(T *buffer) const {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->write("target_hp", (int32_t)target_hp);
    buffer->write("nIncHP", (int32_t)nIncHP);
    buffer->write("hFrom", (uint32_t)hFrom);
  }
  template <class T> void deserialize(T *buffer) {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->template read<int32_t>("target_hp", target_hp);
    buffer->template read<int32_t>("nIncHP", nIncHP);
    buffer->template read<uint32_t>("hFrom", hFrom);
  }
};
# 138 "TS_SC_SKILL.h"
struct TS_SC_SKILL__HIT_DETAILS {
  static inline const char *getName() { return "TS_SC_SKILL__HIT_DETAILS"; };
  TS_SKILL__HIT_TYPE type;
  uint32_t hTarget;
  TS_SC_SKILL__HIT_DAMAGE hitDamage;
  TS_SC_SKILL__HIT_DAMAGE_WITH_KNOCKBACK hitDamageWithKnockBack;
  TS_SC_SKILL__HIT_RESULT hitResult;
  TS_SC_SKILL__HIT_ADD_STAT hitAddStat;
  TS_SC_SKILL__HIT_ADDHPMPSP hitAddHPMPSP;
  TS_SC_SKILL__HIT_REBIRTH hitRebirth;
  TS_SC_SKILL__HIT_RUSH hitRush;
  TS_SC_SKILL__HIT_CHAIN_DAMAGE hitChainDamage;
  TS_SC_SKILL__HIT_CHAIN_HEAL hitChainHeal;

  uint32_t getSize(int version) const {
    uint32_t size = 0;
    (void)(version);

    size += PacketDeclaration::getSizeOf((TS_SKILL__HIT_TYPE)type, version);
    size += PacketDeclaration::getSizeOf((uint32_t)hTarget, version);
    if (type == SHT_DAMAGE || type == SHT_MAGIC_DAMAGE)
      size += PacketDeclaration::getSizeOf((TS_SC_SKILL__HIT_DAMAGE)hitDamage,
                                           version);
    if (type == SHT_DAMAGE_WITH_KNOCK_BACK)
      size += PacketDeclaration::getSizeOf(
          (TS_SC_SKILL__HIT_DAMAGE_WITH_KNOCKBACK)hitDamageWithKnockBack,
          version);
    if (type == SHT_RESULT)
      size += PacketDeclaration::getSizeOf((TS_SC_SKILL__HIT_RESULT)hitResult,
                                           version);
    if (type == SHT_ADD_HP || type == SHT_ADD_MP)
      size += PacketDeclaration::getSizeOf(
          (TS_SC_SKILL__HIT_ADD_STAT)hitAddStat, version);
    if (type == SHT_ADD_HP_MP_SP)
      size += PacketDeclaration::getSizeOf(
          (TS_SC_SKILL__HIT_ADDHPMPSP)hitAddHPMPSP, version);
    if (type == SHT_REBIRTH)
      size += PacketDeclaration::getSizeOf((TS_SC_SKILL__HIT_REBIRTH)hitRebirth,
                                           version);
    if (type == SHT_RUSH)
      size +=
          PacketDeclaration::getSizeOf((TS_SC_SKILL__HIT_RUSH)hitRush, version);
    if (type == SHT_CHAIN_DAMAGE || type == SHT_CHAIN_MAGIC_DAMAGE)
      size += PacketDeclaration::getSizeOf(
          (TS_SC_SKILL__HIT_CHAIN_DAMAGE)hitChainDamage, version);
    if (type == SHT_CHAIN_HEAL)
      size += PacketDeclaration::getSizeOf(
          (TS_SC_SKILL__HIT_CHAIN_HEAL)hitChainHeal, version);

    return size;
  }
  template <class T> void serialize(T *buffer) const {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->write("type", (TS_SKILL__HIT_TYPE)type);
    buffer->write("hTarget", (uint32_t)hTarget);
    if (type == SHT_DAMAGE || type == SHT_MAGIC_DAMAGE)
      buffer->write("hitDamage", (TS_SC_SKILL__HIT_DAMAGE)hitDamage);
    if (type == SHT_DAMAGE_WITH_KNOCK_BACK)
      buffer->write(
          "hitDamageWithKnockBack",
          (TS_SC_SKILL__HIT_DAMAGE_WITH_KNOCKBACK)hitDamageWithKnockBack);
    if (type == SHT_RESULT)
      buffer->write("hitResult", (TS_SC_SKILL__HIT_RESULT)hitResult);
    if (type == SHT_ADD_HP || type == SHT_ADD_MP)
      buffer->write("hitAddStat", (TS_SC_SKILL__HIT_ADD_STAT)hitAddStat);
    if (type == SHT_ADD_HP_MP_SP)
      buffer->write("hitAddHPMPSP", (TS_SC_SKILL__HIT_ADDHPMPSP)hitAddHPMPSP);
    if (type == SHT_REBIRTH)
      buffer->write("hitRebirth", (TS_SC_SKILL__HIT_REBIRTH)hitRebirth);
    if (type == SHT_RUSH)
      buffer->write("hitRush", (TS_SC_SKILL__HIT_RUSH)hitRush);
    if (type == SHT_CHAIN_DAMAGE || type == SHT_CHAIN_MAGIC_DAMAGE)
      buffer->write("hitChainDamage",
                    (TS_SC_SKILL__HIT_CHAIN_DAMAGE)hitChainDamage);
    if (type == SHT_CHAIN_HEAL)
      buffer->write("hitChainHeal", (TS_SC_SKILL__HIT_CHAIN_HEAL)hitChainHeal);
  }
  template <class T> void deserialize(T *buffer) {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->template read<TS_SKILL__HIT_TYPE>("type", type);
    buffer->template read<uint32_t>("hTarget", hTarget);
    if (type == SHT_DAMAGE || type == SHT_MAGIC_DAMAGE)
      buffer->template read<TS_SC_SKILL__HIT_DAMAGE>("hitDamage", hitDamage);
    if (type == SHT_DAMAGE_WITH_KNOCK_BACK)
      buffer->template read<TS_SC_SKILL__HIT_DAMAGE_WITH_KNOCKBACK>(
          "hitDamageWithKnockBack", hitDamageWithKnockBack);
    if (type == SHT_RESULT)
      buffer->template read<TS_SC_SKILL__HIT_RESULT>("hitResult", hitResult);
    if (type == SHT_ADD_HP || type == SHT_ADD_MP)
      buffer->template read<TS_SC_SKILL__HIT_ADD_STAT>("hitAddStat",
                                                       hitAddStat);
    if (type == SHT_ADD_HP_MP_SP)
      buffer->template read<TS_SC_SKILL__HIT_ADDHPMPSP>("hitAddHPMPSP",
                                                        hitAddHPMPSP);
    if (type == SHT_REBIRTH)
      buffer->template read<TS_SC_SKILL__HIT_REBIRTH>("hitRebirth", hitRebirth);
    if (type == SHT_RUSH)
      buffer->template read<TS_SC_SKILL__HIT_RUSH>("hitRush", hitRush);
    if (type == SHT_CHAIN_DAMAGE || type == SHT_CHAIN_MAGIC_DAMAGE)
      buffer->template read<TS_SC_SKILL__HIT_CHAIN_DAMAGE>("hitChainDamage",
                                                           hitChainDamage);
    if (type == SHT_CHAIN_HEAL)
      buffer->template read<TS_SC_SKILL__HIT_CHAIN_HEAL>("hitChainHeal",
                                                         hitChainHeal);
  }
};
# 149 "TS_SC_SKILL.h"
struct TS_SC_SKILL__FIRE {
  static inline const char *getName() { return "TS_SC_SKILL__FIRE"; };
  bool bMultiple;
  float range;
  int8_t target_count;
  int8_t fire_count;
  std::vector<TS_SC_SKILL__HIT_DETAILS> hits;
  struct _metadata_hits {
    enum { addNullTerminator = 0 };
  };

  uint32_t getSize(int version) const {
    uint32_t size = 0;
    (void)(version);
    uint32_t hits_size = 0;
    (void)(hits_size);

    size += PacketDeclaration::getSizeOf((bool)bMultiple, version);
    size += PacketDeclaration::getSizeOf((float)range, version);
    size += PacketDeclaration::getSizeOf((int8_t)target_count, version);
    size += PacketDeclaration::getSizeOf((int8_t)fire_count, version);
    hits_size = PacketDeclaration::getClampedCount<uint16_t>(
        hits.size() + _metadata_hits::addNullTerminator);
    size += sizeof(uint16_t);
    const uint32_t before_hit_marker = size;
    for (size_t i = 0; i < hits_size; i++)
      size += PacketDeclaration::getSizeOf((TS_SC_SKILL__HIT_DETAILS)hits[i],
                                           version);
    if (size < before_hit_marker + (45 * (uint32_t)hits.size()))
      size = before_hit_marker + (45 * (uint32_t)hits.size());

    return size;
  }
  template <class T> void serialize(T *buffer) const {
    const int version = buffer->getVersion();
    (void)(version);

    uint32_t hits_size = 0;
    (void)(hits_size);

    buffer->write("bMultiple", (bool)bMultiple);
    buffer->write("range", (float)range);
    buffer->write("target_count", (int8_t)target_count);
    buffer->write("fire_count", (int8_t)fire_count);
    hits_size = PacketDeclaration::getClampedCount<uint16_t>(
        hits.size() + _metadata_hits::addNullTerminator);
    buffer->writeSize("hits", (uint16_t)hits_size);
    const uint32_t before_hit_marker = buffer->getParsedSize();
    buffer->template writeDynArray<TS_SC_SKILL__HIT_DETAILS>("hits", hits,
                                                             hits_size);
    if (buffer->getParsedSize() <
        before_hit_marker + (45 * (uint32_t)hits.size()))
      buffer->pad("pad_"
                  "before_hit_marker",
                  before_hit_marker + (45 * (uint32_t)hits.size()) -
                      buffer->getParsedSize());
  }
  template <class T> void deserialize(T *buffer) {
    const int version = buffer->getVersion();
    (void)(version);

    uint32_t hits_size = 0;
    (void)(hits_size);

    buffer->template read<bool>("bMultiple", bMultiple);
    buffer->template read<float>("range", range);
    buffer->template read<int8_t>("target_count", target_count);
    buffer->template read<int8_t>("fire_count", fire_count);
    buffer->template readSize<uint16_t>("hits", hits_size);
    const uint32_t before_hit_marker = buffer->getParsedSize();
    buffer->template readDynArray<TS_SC_SKILL__HIT_DETAILS>("hits", hits,
                                                            hits_size);
    if (buffer->getParsedSize() <
        before_hit_marker + (45 * (uint32_t)hits.size()))
      buffer->discard("pad_"
                      "before_hit_marker",
                      before_hit_marker + (45 * (uint32_t)hits.size()) -
                          buffer->getParsedSize());
  }
};

struct TS_SC_SKILL__CAST {
  static inline const char *getName() { return "TS_SC_SKILL__CAST"; };
  uint32_t tm;
  uint16_t nErrorCode;

  uint32_t getSize(int version) const {
    uint32_t size = 0;
    (void)(version);

    size += PacketDeclaration::getSizeOf((uint32_t)tm, version);
    size += PacketDeclaration::getSizeOf((uint16_t)nErrorCode, version);

    return size;
  }
  template <class T> void serialize(T *buffer) const {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->write("tm", (uint32_t)tm);
    buffer->write("nErrorCode", (uint16_t)nErrorCode);
  }
  template <class T> void deserialize(T *buffer) {
    const int version = buffer->getVersion();
    (void)(version);

    buffer->template read<uint32_t>("tm", tm);
    buffer->template read<uint16_t>("nErrorCode", nErrorCode);
  }
};
# 181 "TS_SC_SKILL.h"
struct TS_SC_SKILL {
  static inline const char *getName() { return "TS_SC_SKILL"; }
  static const uint16_t packetID = 401;
  static inline uint16_t getId(int version) {
    (void)version;
    return 401;
  }
  uint16_t id;

  uint16_t skill_id;
  uint8_t skill_level;
  uint32_t caster;
  uint32_t target;
  float x;
  float y;
  float z;
  uint8_t layer;
  TS_SKILL__TYPE type;
  int32_t hp_cost;
  int32_t mp_cost;
  int32_t caster_hp;
  int32_t caster_mp;
  TS_SC_SKILL__FIRE fire;
  TS_SC_SKILL__CAST casting;

  uint32_t getSize(int version) const {
    uint32_t size = 7;
    (void)(version);

    size += PacketDeclaration::getSizeOf((uint16_t)skill_id, version);
    size += PacketDeclaration::getSizeOf((uint8_t)skill_level, version);
    size += PacketDeclaration::getSizeOf((uint32_t)caster, version);
    size += PacketDeclaration::getSizeOf((uint32_t)target, version);
    size += PacketDeclaration::getSizeOf((float)x, version);
    size += PacketDeclaration::getSizeOf((float)y, version);
    size += PacketDeclaration::getSizeOf((float)z, version);
    size += PacketDeclaration::getSizeOf((uint8_t)layer, version);
    size += PacketDeclaration::getSizeOf((TS_SKILL__TYPE)type, version);
    if (version >= EPIC_7_3)
      size += PacketDeclaration::getSizeOf((int32_t)hp_cost, version);
    if (version < EPIC_7_3)
      size += PacketDeclaration::getSizeOf((int16_t)hp_cost, version);
    if (version >= EPIC_7_3)
      size += PacketDeclaration::getSizeOf((int32_t)mp_cost, version);
    if (version < EPIC_7_3)
      size += PacketDeclaration::getSizeOf((int16_t)mp_cost, version);
    size += PacketDeclaration::getSizeOf((int32_t)caster_hp, version);
    if (version >= EPIC_7_3)
      size += PacketDeclaration::getSizeOf((int32_t)caster_mp, version);
    if (version < EPIC_7_3)
      size += PacketDeclaration::getSizeOf((int16_t)caster_mp, version);
    const uint32_t skill_type_marker = size;
    if (type == ST_Fire || type == ST_RegionFire)
      size += PacketDeclaration::getSizeOf((TS_SC_SKILL__FIRE)fire, version);
    if (type == ST_Casting || type == ST_CastingUpdate)
      size += PacketDeclaration::getSizeOf((TS_SC_SKILL__CAST)casting, version);
    if (size < skill_type_marker + (9))
      size = skill_type_marker + (9);

    return size;
  }
  template <class T> void serialize(T *buffer) const {
    const int version = buffer->getVersion();
    (void)(version);
    uint32_t size = getSize(buffer->getVersion());
    buffer->writeHeader(size, packetID);

    buffer->write("skill_id", (uint16_t)skill_id);
    buffer->write("skill_level", (uint8_t)skill_level);
    buffer->write("caster", (uint32_t)caster);
    buffer->write("target", (uint32_t)target);
    buffer->write("x", (float)x);
    buffer->write("y", (float)y);
    buffer->write("z", (float)z);
    buffer->write("layer", (uint8_t)layer);
    buffer->write("type", (TS_SKILL__TYPE)type);
    if (version >= EPIC_7_3)
      buffer->write("hp_cost", (int32_t)hp_cost);
    if (version < EPIC_7_3)
      buffer->write("hp_cost", (int16_t)hp_cost);
    if (version >= EPIC_7_3)
      buffer->write("mp_cost", (int32_t)mp_cost);
    if (version < EPIC_7_3)
      buffer->write("mp_cost", (int16_t)mp_cost);
    buffer->write("caster_hp", (int32_t)caster_hp);
    if (version >= EPIC_7_3)
      buffer->write("caster_mp", (int32_t)caster_mp);
    if (version < EPIC_7_3)
      buffer->write("caster_mp", (int16_t)caster_mp);
    const uint32_t skill_type_marker = buffer->getParsedSize();
    if (type == ST_Fire || type == ST_RegionFire)
      buffer->write("fire", (TS_SC_SKILL__FIRE)fire);
    if (type == ST_Casting || type == ST_CastingUpdate)
      buffer->write("casting", (TS_SC_SKILL__CAST)casting);
    if (buffer->getParsedSize() < skill_type_marker + (9))
      buffer->pad("pad_"
                  "skill_type_marker",
                  skill_type_marker + (9) - buffer->getParsedSize());
  }
  template <class T> void deserialize(T *buffer) {
    const int version = buffer->getVersion();
    (void)(version);
    buffer->readHeader(id);

    buffer->template read<uint16_t>("skill_id", skill_id);
    buffer->template read<uint8_t>("skill_level", skill_level);
    buffer->template read<uint32_t>("caster", caster);
    buffer->template read<uint32_t>("target", target);
    buffer->template read<float>("x", x);
    buffer->template read<float>("y", y);
    buffer->template read<float>("z", z);
    buffer->template read<uint8_t>("layer", layer);
    buffer->template read<TS_SKILL__TYPE>("type", type);
    if (version >= EPIC_7_3)
      buffer->template read<int32_t>("hp_cost", hp_cost);
    if (version < EPIC_7_3)
      buffer->template read<int16_t>("hp_cost", hp_cost);
    if (version >= EPIC_7_3)
      buffer->template read<int32_t>("mp_cost", mp_cost);
    if (version < EPIC_7_3)
      buffer->template read<int16_t>("mp_cost", mp_cost);
    buffer->template read<int32_t>("caster_hp", caster_hp);
    if (version >= EPIC_7_3)
      buffer->template read<int32_t>("caster_mp", caster_mp);
    if (version < EPIC_7_3)
      buffer->template read<int16_t>("caster_mp", caster_mp);
    const uint32_t skill_type_marker = buffer->getParsedSize();
    if (type == ST_Fire || type == ST_RegionFire)
      buffer->template read<TS_SC_SKILL__FIRE>("fire", fire);
    if (type == ST_Casting || type == ST_CastingUpdate)
      buffer->template read<TS_SC_SKILL__CAST>("casting", casting);
    if (buffer->getParsedSize() < skill_type_marker + (9))
      buffer->discard("pad_"
                      "skill_type_marker",
                      skill_type_marker + (9) - buffer->getParsedSize());
  }
};
```
