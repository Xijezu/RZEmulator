#include "Player/Player.h"
#include "MemPool.h"
#include "Database/DatabaseEnv.h"
#include "Network/GameNetwork/WorldSession.h"
#include "ObjectMgr.h"
#include "GameNetwork/ClientPackets.h"
#include "Messages.h"
#include "Scripting/XLua.h"
#include "World.h"
#include "Skill.h"
#include "ArRegion.h"
#include "NPC.h"
#include "MemPool.h"
#include "SharedMutex.h"
// we can disable this warning for this since it only
// causes undefined behavior when passed to the base class constructor
#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

Player::Player(uint32 handle) : Unit(true), m_session(nullptr), m_TS(TimeSynch(200, 2, 10))
{
#ifdef _MSC_VER
#   pragma warning(default:4355)
#endif
    _mainType = MT_Player;
    _subType  = ST_Player;
    _objType  = OBJ_CLIENT;
    _valuesCount = BATTLE_FIELD_END;

    _InitValues();
    SetUInt32Value(UNIT_FIELD_HANDLE, handle);
}

Player::~Player()
{
    /*if(m_pSubSummon != nullptr) {
        sWorld->RemoveObjectFromWorld(m_pSubSummon);
        m_pSubSummon = nullptr;
    }*/

    if(!IsInWorld())
        return;

    for (auto &t : m_lInventory) {
        Item::PendFreeItem(t.second);
        t.second = nullptr;
    }
    m_lInventory.clear();

    for(auto &t : m_vSummonList) {
        if(t->IsInWorld())
            sWorld->RemoveObjectFromWorld(t);
        sMemoryPool->RemoveObject(t, true);
        t = nullptr;
    }
    m_vSummonList.clear();
}

void Player::EnterPacket(XPacket &pEnterPct, Player *pPlayer, Player* pReceiver)
{
    Unit::EnterPacket(pEnterPct, pPlayer, pReceiver);
    pEnterPct << (uint8_t)pPlayer->GetInt32Value(UNIT_FIELD_SEX);
    pEnterPct << (uint32_t)pPlayer->GetInt32Value(UNIT_FIELD_MODEL + 1);
    pEnterPct << (uint32_t)pPlayer->GetInt32Value(UNIT_FIELD_MODEL);
    pEnterPct.fill(pPlayer->GetName(), 19);
    pEnterPct << (uint16_t)pPlayer->GetCurrentJob();
    pEnterPct << (uint32_t)0; // TODO: Ride Handle
    pEnterPct << (uint32_t)pPlayer->GetInt32Value(UNIT_FIELD_GUILD_ID);
}

bool Player::ReadCharacter(std::string _name, int _race)
{
    int mainSummon = 0;
    int subSummon = 0;

    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_GET_CHARACTER);
    stmt->setString(0, _name);
    stmt->setInt32(1, m_session->GetAccountId());
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt)) {
        SetName(_name);
        SetUInt32Value(UNIT_FIELD_UID, (*result)[0].GetUInt32());
        m_szAccount = (*result)[1].GetString();
        SetInt32Value(UNIT_FIELD_PERMISSION, (*result)[2].GetInt32());
        SetInt32Value(UNIT_FIELD_PARTY_ID, (*result)[3].GetInt32());
        SetInt32Value(UNIT_FIELD_GUILD_ID, (*result)[4].GetInt32());
        Relocate((float) (*result)[5].GetInt32(), (float) (*result)[6].GetInt32(), (float) (*result)[7].GetInt32(), 0);
        SetLayer((*result)[8].GetInt32());
        SetInt32Value(UNIT_FIELD_RACE, (*result)[9].GetInt32());
        SetInt32Value(UNIT_FIELD_SEX, (*result)[10].GetInt32());
        SetInt32Value(UNIT_FIELD_LEVEL, (*result)[11].GetInt32());
        SetUInt64Value(UNIT_FIELD_EXP, (int) (*result)[12].GetUInt64());
        SetInt32Value(UNIT_FIELD_HEALTH, (*result)[13].GetInt32());
        SetInt32Value(UNIT_FIELD_MANA, (*result)[14].GetInt32());
        SetInt32Value(UNIT_FIELD_STAMINA, (*result)[15].GetInt32());
        SetInt32Value(UNIT_FIELD_HAVOC, (*result)[16].GetInt32());
        SetInt32Value(UNIT_FIELD_JOB_DEPTH, (*result)[17].GetInt8());
        SetInt32Value(UNIT_FIELD_JOBPOINT, (*result)[18].GetInt32());
        for (int i = 0; i < 3; i++) {
            SetInt32Value(UNIT_FIELD_PREV_JOB + i, (*result)[19 + i].GetInt32());
            SetInt32Value(UNIT_FIELD_PREV_JLV + i, (*result)[22 + i].GetInt32());
        }
        SetInt32Value(UNIT_FIELD_IP, (int) (*result)[25].GetFloat());
        SetInt32Value(UNIT_FIELD_CHA, (*result)[26].GetInt32());
        SetInt32Value(UNIT_FIELD_PKC, (*result)[27].GetInt32());
        SetInt32Value(UNIT_FIELD_DKC, (*result)[28].GetInt32());
        for (int i = 0; i < 6; i++) {
            SetInt32Value(UNIT_FIELD_SUMMON + i, (*result)[29 + i].GetInt32());
            SetInt32Value(UNIT_FIELD_BELT + i, (*result)[41 + i].GetInt32());
        }
        SetInt32Value(UNIT_FIELD_SKIN_COLOR, (*result)[35].GetInt32());
        for (int i = 0; i < 5; i++) {
            SetInt32Value(UNIT_FIELD_MODEL + i, (*result)[36 + i].GetInt32());
        }
        SetUInt64Value(UNIT_FIELD_GOLD, (*result)[47].GetUInt64());
        SetInt32Value(UNIT_FIELD_CHAOS, (*result)[48].GetInt32());
        std::string flag_list = (*result)[50].GetString();
        Tokenizer      token(flag_list, '\n');
        for (auto iter : token) {
            Tokenizer flag(iter, ':');
            if (flag.size() == 2) {
                SetFlag(flag[1], flag[0]);
            }
        }
        mainSummon = (*result)[51].GetInt32();
        subSummon = (*result)[52].GetInt32();
        SetInt32Value(UNIT_FIELD_REMAIN_SUMMON_TIME, (*result)[53].GetInt32());
        SetInt32Value(UNIT_FIELD_PET, (*result)[54].GetInt32());
        SetUInt64Value(UNIT_FIELD_CHAT_BLOCK_TIME, (*result)[55].GetUInt64());
        SetUInt64Value(UNIT_FIELD_GUILD_BLOCK_TIME, (*result)[56].GetUInt64());
        SetInt32Value(UNIT_FIELD_PK_MODE, (*result)[57].GetInt8());
        SetInt32Value(UNIT_FIELD_JOB, (*result)[58].GetInt32());
        SetInt32Value(UNIT_FIELD_JLV, (*result)[59].GetInt32());
        m_szClientInfo = (*result)[60].GetString();

        if(GetLevel() == 0) {
            SetLevel(1);
            SetCurrentJLv(1);
        }
        /*if (!ReadWearInfo(GetInt32Value(UNIT_FIELD_HANDLE)))
        {
            return false;
        }*/
        if (!ReadItemList(GetInt32Value(UNIT_FIELD_UID)) || !ReadSummonList(GetInt32Value(UNIT_FIELD_UID))) {
            return false;
        }

        int nSummonIdx = 0;
        for(int i = 0; i < 6; ++i) {
            if (GetInt32Value(UNIT_FIELD_SUMMON + i) != 0) {
                auto pSummon = GetSummon(GetInt32Value(UNIT_FIELD_SUMMON + i));
                if (pSummon != nullptr) {
                    //nSummonHP[nSummonIdx] = pSummon.m_nHP;
                    pSummon->m_cSlotIdx = (uint8_t) nSummonIdx;
                    //nSummonMP[nSummonIdx] = pSummon.m_fMP;
                    pSummon->CalculateStat();
                    m_aBindSummonCard[nSummonIdx] = pSummon->m_pItem;
                    nSummonIdx++;
                } else {
                    MX_LOG_ERROR("entities", "Invalid Summon Bind!");
                }
            }
        }

        if(mainSummon != 0) {
            m_pMainSummon = GetSummon(mainSummon);
        }

        if(!ReadEquipItem() || !ReadSkillList(GetInt32Value(UNIT_FIELD_UID)))
            return false;

        CalculateStat();
        Messages::SendHPMPMessage(this, this, GetHealth(), GetMana(), true);
        //Messages::sendEnterMessage(this, this, false);

    } else {
        return false;
    }
    return true;
}

bool Player::ReadItemList(int sid)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_GET_ITEMLIST);
    stmt->setInt32(0, sid);
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt)) {
        int j = 0, inv = 0;
        do {
            Field *fields     = result->Fetch();
            int   i           = 0;
            int   uid         = fields[i++].GetInt32();
            int   idx         = fields[i++].GetInt32();
            int   code        = fields[i++].GetInt32();
            int64 cnt         = fields[i++].GetInt64();
            int   gcode       = fields[i++].GetInt32();
            int   level       = fields[i++].GetInt32();
            int   enhance     = fields[i++].GetInt32();
            int   flag        = fields[i++].GetInt32();
            int   summon_id   = fields[i++].GetInt32();
            int   socket_0    = fields[i++].GetInt32();
            int   socket_1    = fields[i++].GetInt32();
            int   socket_2    = fields[i++].GetInt32();
            int   socket_3    = fields[i++].GetInt32();
            int   remain_time = fields[i++].GetInt32();

            auto item = Item::AllocItem(uid, code, cnt, (GenerateCode) gcode, level, enhance, flag,
                                        socket_0, socket_1, socket_2, socket_3, remain_time);

            if(code != 0) {

                item->m_Instance.nWearInfo     = (ItemWearType) fields[i++].GetInt32();
                item->m_Instance.nOwnSummonUID = summon_id;
                item->m_Instance.OwnerHandle   = GetHandle();
                item->m_Instance.nIdx          = inv;
                item->m_pSummon = nullptr;
                item->m_bIsNeedUpdateToDB      = inv != idx;
                item->m_Instance.nOwnerUID     = sid;
                m_lInventory[j++] = item;
            }
        } while (result->NextRow());
    }
    return true;
}

bool Player::ReadSkillList(int)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_GET_SKILL);
    stmt->setInt32(0, GetInt32Value(UNIT_FIELD_UID));
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt)) {
        do {
            Field *fields     = result->Fetch();
            auto  sid         = fields[0].GetInt32();
            auto  owner_id    = fields[1].GetInt32();
            auto  summon_id   = fields[2].GetInt32();
            auto  skill_id    = fields[3].GetInt32();
            auto  skill_level = fields[4].GetInt32();
            auto  cool_time   = fields[5].GetInt32();
            if (summon_id == 0)
                SetSkill(sid, skill_id, skill_level, cool_time);


        } while (result->NextRow());
    }

    SetSkill(-1, 6001, 20, 0);
    SetSkill(-1, 6002, 20, 0);
    SetSkill(-1, 6003, 20, 0);
    SetSkill(-1, 6004, 20, 0);
    SetSkill(-1, 6005, 20, 0);
    SetSkill(-1, 6006, 20, 0);
    SetSkill(-1, 6007, 20, 0);
    SetSkill(-1, 6008, 20, 0);
    SetSkill(-1, 6009, 20, 0);
    SetSkill(-1, 6010, 20, 0);
    SetSkill(-1, 6013, 20, 0);
    SetSkill(-1, 6014, 20, 0);
    SetSkill(-1, 6015, 20, 0);
    SetSkill(-1, 6016, 20, 0);
    SetSkill(-1, 6017, 20, 0);
    SetSkill(-1, 6018, 20, 0);
    SetSkill(-1, 6019, 20, 0);
    SetSkill(-1, 6020, 20, 0);
    SetSkill(-1, 6021, 20, 0);
    /*SetSkill(-2, 6901, 20, 0);
    SetSkill(-2, 6902, 20, 0);
    SetSkill(-2, 6903, 20, 0);
    SetSkill(-2, 6904, 20, 0);
    SetSkill(-2, 6905, 20, 0);
    SetSkill(-2, 6906, 20, 0);*/
    SetSkill(-1, 6022, 20, 0);
    SetSkill(-1, 6023, 20, 0);
    SetSkill(-1, 6024, 20, 0);
    SetSkill(-1, 6025, 20, 0);
    SetSkill(-1, 6026, 20, 0);
    SetSkill(-1, 6027, 20, 0);
    SetSkill(-1, 6028, 20, 0);
    SetSkill(-1, 6029, 20, 0);
    SetSkill(-1, 6030, 20, 0);
    SetSkill(-1, 6031, 20, 0);
    SetSkill(-1, 6032, 20, 0);
    SetSkill(-1, 6033, 20, 0);
    SetSkill(-1, 6034, 20, 0);
    SetSkill(-1, 6035, 20, 0);
    SetSkill(-1, 6036, 20, 0);
    SetSkill(-1, 6037, 20, 0);
    SetSkill(-1, 6038, 20, 0);
    SetSkill(-1, 6039, 20, 0);
    SetSkill(-1, 6040, 20, 0);
    SetSkill(-1, 6041, 20, 0);
    SetSkill(-1, 6042, 20, 0);
    SetSkill(-1, 6043, 20, 0);
    SetSkill(-1, 6044, 20, 0);
    SetSkill(-1, 6045, 20, 0);
    SetSkill(-1, 6046, 20, 0);
    SetSkill(-1, 6047, 20, 0);
    SetSkill(-1, 6048, 20, 0);
    SetSkill(-1, 6049, 20, 0);
    SetSkill(-1, 6061, 20, 0);
    SetSkill(-1, 6062, 20, 0);
    SetSkill(-1, 6063, 20, 0);
    SetSkill(-1, 6064, 20, 0);
    SetSkill(-1, 6065, 20, 0);
    SetSkill(-1, 6066, 20, 0);
    SetSkill(-1, 10009, 20, 0);
    SetSkill(-1, 10010, 20, 0);

    return true;
}

bool Player::ReadEquipItem()
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_GET_EQUIP_ITEM);
    stmt->setInt32(0, GetInt32Value(UNIT_FIELD_UID));
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt)) {
        do {
            Field *fields = result->Fetch();
            uint32_t sid = fields[0].GetUInt32();
            int summon_id = fields[1].GetInt32();
            int idx = fields[2].GetInt32();
            int wear_info = idx;

            Unit* unit = nullptr;
            if(summon_id == 0) {
                unit = this;
            } else {
                unit = GetSummon(summon_id);
            }
            Item* item = FindItemBySID(sid);
            std::vector<int> indices{ };
            if(item != nullptr && unit != nullptr) {
                if(unit->m_anWear[wear_info] == nullptr) {
                    auto iwt = (ItemWearType)wear_info;
                    if(TranslateWearPosition(iwt, item, &indices)) {
                        unit->m_anWear[wear_info] = item;
                        item->m_Instance.nWearInfo = (ItemWearType)wear_info;
                        item->m_bIsNeedUpdateToDB = true;
                        if(unit->GetSubType() == ST_Summon) {
                            item->m_Instance.OwnSummonHandle = unit->GetHandle();
                            item->m_Instance.nOwnSummonUID = (int)unit->GetInt32Value(UNIT_FIELD_UID);
                        }
                    }
                }
            }

        } while (result->NextRow());
    }
    return true;
}

bool Player::ReadSummonList(int UID)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_GET_SUMMONLIST);
    stmt->setInt32(0, UID);
    if (PreparedQueryResult result = CharacterDatabase.Query(stmt)) {
        do {
            Field *fields = result->Fetch();
            int i = 0;
            //PrepareStatement(CHARACTER_GET_SUMMONLIST, "SELECT sid, account_id, code, card_uid, exp, jp,
            // last_decreased_exp, name, transform, lv, jlv, max_level, fp, prev_level_01, prev_level_02,
            // prev_id_01, prev_id_02, sp, hp, mp FROM Summon WHERE owner_id = ?", CONNECTION_SYNCH);
            uint  sid               = fields[i++].GetUInt32();
            int  account_id         = fields[i++].GetInt32();
            int  code               = fields[i++].GetInt32();
            uint card_uid           = fields[i++].GetUInt32();
            uint exp                = fields[i++].GetUInt32();
            int  jp                 = fields[i++].GetInt32();
            uint last_decreased_exp = fields[i++].GetUInt32();
            std::string name        = fields[i++].GetString();
            int transform           = fields[i++].GetInt32();
            int lv                  = fields[i++].GetInt32();
            int jlv                 = fields[i++].GetInt32();
            int max_level           = fields[i++].GetInt32();
            int fp                  = fields[i++].GetInt32();
            int prev_level_01       = fields[i++].GetInt32();
            int prev_level_02       = fields[i++].GetInt32();
            int prev_id_01          = fields[i++].GetInt32();
            int prev_id_02          = fields[i++].GetInt32();
            int sp                  = fields[i++].GetInt32();
            int hp                  = fields[i++].GetInt32();
            int mp                  = fields[i++].GetInt32();

            auto summon = Summon::AllocSummon(this, code);
            summon->SetUInt32Value(UNIT_FIELD_UID, sid);
            summon->m_nSummonInfo = code;
            summon->m_nCardUID = card_uid;
            summon->SetUInt64Value(UNIT_FIELD_EXP, exp);
            summon->SetJP(jp);
            summon->SetName(name);
            summon->SetLevel(lv);
            summon->SetCurrentJLv(jlv);
            summon->SetInt32Value(UNIT_FIELD_PREV_JOB, prev_id_01);
            summon->SetInt32Value(UNIT_FIELD_PREV_JOB + 1, prev_id_02);
            summon->SetInt32Value(UNIT_FIELD_PREV_JLV, prev_level_01);
            summon->SetInt32Value(UNIT_FIELD_PREV_JLV + 1, prev_level_02);
            summon->SetInt32Value(UNIT_FIELD_HEALTH, hp);
            summon->SetInt32Value(UNIT_FIELD_MANA, mp);
            summon->m_nTransform = transform;
            summon->CalculateStat();
            Item* card = FindItemBySID(card_uid);
            if(card != nullptr) {
                card->m_pSummon = summon;
                card->m_Instance.Socket[0] = sid;
                card->m_bIsNeedUpdateToDB = true;
                card->m_Instance.OwnSummonHandle = summon->GetHandle();
                summon->m_pItem = card;
                /*m_player.AddSummon(summon, false);
                readCreatureSkillList(summon);
                readStateInfo(summon);
                summon.SetLoginComplete();
                summon.m_nSP = sp;
                summon.m_nHP = hp;
                summon.m_fMP = mp;*/
            }
            m_vSummonList.push_back(summon);
        } while (result->NextRow());
    }
    return true;
}

void Player::SendPropertyMessage(std::string key, std::string value)
{
    XPacket packet(TS_SC_PROPERTY);
    packet << GetUInt32Value(UNIT_FIELD_HANDLE);
    packet << (uint8) 0;
    packet.fill(key, 16);
#if EPIC > 4
    packet << (uint64) 0;
#else
    packet << (uint32) 0;
#endif
    packet << value;
    packet << (uint8) 0;
    SendPacket(packet);
}

void Player::SendPropertyMessage(std::string key, int64 value)
{
    XPacket packet(TS_SC_PROPERTY);
    packet << GetUInt32Value(UNIT_FIELD_HANDLE);
    packet << (uint8) 1;
    packet.fill(key, 16);
    packet << value;
    SendPacket(packet);
}

void Player::SendLoginProperties()
{
    sWorld->AddSession(m_session);
    CalculateStat();
    // Login();
    Messages::SendQuestList(this);
    for(auto summon: m_vSummonList) {
        Messages::SendAddSummonMessage(this, summon);
    }

    Messages::SendItemList(this, false);
    Messages::SendCreatureEquipMessage(this, false);

    Messages::SendSkillList(this, this, -1);
    // TODO Summon Skill Msg

    SendWearInfo();
    SendGoldChaosMessage();
    SendPropertyMessage("chaos", (int64) GetUInt32Value(UNIT_FIELD_CHAOS));
    Messages::SendLevelMessage(this, this);
    Messages::SendEXPMessage(this, this);
    SendJobInfo();

    Messages::SendStatInfo(this, this);


    SendPropertyMessage("pk_count", (int64) GetUInt32Value(UNIT_FIELD_PKC));
    SendPropertyMessage("dk_count", (int64) GetUInt32Value(UNIT_FIELD_DKC));
    SendPropertyMessage("immoral", (int64) GetUInt32Value(UNIT_FIELD_IP));
    SendPropertyMessage("stamina", (int64) GetUInt32Value(UNIT_FIELD_STAMINA));
    SendPropertyMessage("max_stamina", (int64) GetUInt32Value(UNIT_FIELD_STAMINA));
    SendPropertyMessage("channel", (int64) 0);
    SendPropertyMessage("client_info", m_szClientInfo);
    Messages::SendGameTime(this);
    ChangeLocation(GetPositionX(), GetPositionY(), false, false);

    if(!_bIsInWorld) {
        sWorld->AddObjectToWorld(this);
    }

    if (m_pMainSummon != nullptr) {
        m_pMainSummon->SetFlag(UNIT_FIELD_STATUS, StatusFlags::Invincible);
        m_pMainSummon->SetCurrentXY(GetPositionX(),GetPositionY());
        m_pMainSummon->AddNoise(rand32(), rand32(), 50);
        m_pMainSummon->SetLayer(GetLayer());
        sWorld->AddSummonToWorld(m_pMainSummon);
    }
}

void Player::SendGoldChaosMessage()
{
    XPacket packet(TS_SC_GOLD_UPDATE);
    packet << GetUInt64Value(UNIT_FIELD_GOLD);
    packet << GetInt32Value(UNIT_FIELD_CHAOS);
    SendPacket(packet);
}

void Player::SendJobInfo()
{
    SendPropertyMessage("job", GetCurrentJob());
    SendPropertyMessage("jlv", GetCurrentJLv());
    for (int i = 0; i < 3; ++i) {
        SendPropertyMessage(formatString("job_%d", i), GetPrevJobId(i));
        SendPropertyMessage(formatString("jlv_%d", i), GetPrevJobLv(i));
    }
}
void Player::SendWearInfo()
{
    XPacket packet(TS_SC_WEAR_INFO);
    packet << GetHandle();
    for (int i = 0; i < Item::MAX_ITEM_WEAR; i++) {
        int wear_info = (m_anWear[i] != nullptr ? m_anWear[i]->m_Instance.Code : 0);
        if (i == 2 && wear_info == 0)
            wear_info = GetInt32Value(UNIT_FIELD_MODEL + 2);
        if (i == 4 && wear_info == 0)
            wear_info = GetInt32Value(UNIT_FIELD_MODEL + 3);
        if (i == 5 && wear_info == 0)
            wear_info = GetInt32Value(UNIT_FIELD_MODEL + 4);
        packet << wear_info;
    }
    for (auto &i : m_anWear) {
        packet << (i != nullptr ? i->m_Instance.nEnhance : 0);
    }
    for (auto &i : m_anWear) {
        packet << (i != nullptr ? i->m_Instance.nLevel : 0);
    }
    SendPacket(packet);
}

void Player::Save(bool bOnlyPlayer)
{
    // "UPDATE `Character` SET x = ?, y = ?, z = ?, layer = ?, exp = ?, lv = ?, hp = ?, mp = ?, stamina = ?, jlv = ?, jp = ?, total_jp = ?, job_0 = ?, job_1 = ?, job_2 = ?,
    // jlv_0 = ?, jlv_1 = ?, jlv_2 = ?, permission = ?, job = ?, gold = ?, party_id = ?, guild_id = ? WHERE sid = ?"
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_UPDATE_CHARACTER);
    uint8_t i = 0;
    stmt->setFloat(i++, GetPositionX());
    stmt->setFloat(i++, GetPositionY());
    stmt->setFloat(i++, GetPositionZ());
    stmt->setInt32(i++, GetLayer());
    stmt->setInt64(i++, GetEXP());
    stmt->setInt32(i++, GetLevel());
    stmt->setInt32(i++, GetHealth());
    stmt->setInt32(i++, GetMana());
    stmt->setInt32(i++, GetStamina());
    stmt->setInt32(i++, GetCurrentJLv());
    stmt->setInt32(i++, GetJP());
    stmt->setInt32(i++, GetTotalJP());
    stmt->setInt32(i++, GetPrevJobId(0));
    stmt->setInt32(i++, GetPrevJobId(1));
    stmt->setInt32(i++, GetPrevJobId(2));
    stmt->setInt32(i++, GetPrevJobLv(0));
    stmt->setInt32(i++, GetPrevJobLv(1));
    stmt->setInt32(i++, GetPrevJobLv(2));
    stmt->setInt32(i++, GetPermission());
    stmt->setInt32(i++, GetCurrentJob());
    stmt->setInt64(i++, GetGold());
    stmt->setInt32(i++, GetPartyID());
    stmt->setInt32(i++, GetGuildID());
    for(auto summon : m_aBindSummonCard)
        stmt->setInt32(i++, summon != nullptr && summon->m_pSummon != nullptr ? summon->m_pSummon->GetInt32Value(UNIT_FIELD_UID) : 0);
    stmt->setInt32(i++, m_pMainSummon != nullptr ? m_pMainSummon->GetInt32Value(UNIT_FIELD_UID) : 0);
    stmt->setInt32(i++, 0);// Sub Summon
    stmt->setInt32(i++, 0); // Pet
    stmt->setInt32(i++, GetInt32Value(UNIT_FIELD_CHAOS));
    stmt->setString(i++, m_szClientInfo);
    stmt->setInt32(i++, GetInt32Value(UNIT_FIELD_UID));
    CharacterDatabase.Execute(stmt);

    if(!bOnlyPlayer) {
        for (auto& item : m_lInventory) {
            if(item.second->m_bIsNeedUpdateToDB)
                item.second->DBUpdate();
        }

        for(auto& summon : m_vSummonList) {
            if(summon == nullptr)
                continue;
            Summon::DB_UpdateSummon(this, summon);
        }
    }
}

uint Player::GetJobDepth()
{
    auto res = sObjectMgr->GetJobInfo(GetCurrentJob());
    if(res != nullptr)
        return res->job_depth;
    return 0;
}

void Player::applyJobLevelBonus()
{
    int          levels[4]{ };
    int          jobs[4]{ };
    uint          i = 0;
    CreatureStat stat{ };

    if (GetCurrentJob() != 0) {
        uint jobDepth = GetJobDepth();
        for (i = 0; i < jobDepth; i++) {
            jobs[i]   = GetPrevJobId(i);
            levels[i] = GetPrevJobLv(i);
        }
        //i++;

        jobs[i]   = GetCurrentJob();
        levels[i] = GetCurrentJLv();
        stat = sObjectMgr->GetJobLevelBonus(jobDepth, jobs, levels);
        m_cStat.Add(stat);
    }
}

Item *Player::FindItemByCode(int id) {
    for (auto &t : m_lInventory) {
        if (t.second->m_Instance.Code == id)
            return t.second;
    }
    return nullptr;
}

Item *Player::FindItemBySID(uint64_t uid)
{
    for (auto &t : m_lInventory) {
        if (t.second->m_Instance.UID == uid)
            return t.second;
    }
    return nullptr;
}

Item *Player::FindItemByHandle(uint32_t handle)
{
    for(auto &t : m_lInventory) {
        if(t.second->m_nHandle == handle)
            return t.second;
    }
    return nullptr;
}

// TODO
uint16_t Player::putonItem(ItemWearType pos, Item *item)
{
    uint16_t result;

    if(pos == ItemWearType::WearShield) {
        if(item->m_pItemBase->group == 1)
            Unit::SetFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon);
    }
    result = Unit::putonItem(pos, item);
    if(result == 0) {
        if(m_anWear[pos] != nullptr && m_anWear[pos]->GetHandle() == item->GetHandle()) {
            // Weight modifier
        }
        // UpdateWeight
        // UpdateQuest
        result = 0;
    }
    return result;
}

uint16_t Player::putoffItem(ItemWearType pos)
{
    if (pos > Item::MAX_ITEM_WEAR || pos < 0)
        return TS_RESULT_ACCESS_DENIED;

    Item *item = m_anWear[pos];
    if (item != nullptr) {
        item->m_Instance.nWearInfo = ItemWearType::WearNone;
        item->m_bIsNeedUpdateToDB  = true;
        m_anWear[pos] = nullptr;
        SendItemWearInfoMessage(item, this);
    }
    return TS_RESULT_SUCCESS;
}

void Player::SendItemWearInfoMessage(Item* item, Unit *u)
{
    XPacket packet(TS_SC_ITEM_WEAR_INFO);
    packet << (uint32_t) item->m_nHandle;
    packet << (int16_t) item->m_Instance.nWearInfo;
    packet << (uint32_t) (u != nullptr ? u->GetHandle() : 0);
    packet << (int32_t) item->m_Instance.nEnhance;
    SendPacket(packet);
}

Summon* Player::GetSummon(int summon_sid)
{
    for(auto summon : m_vSummonList) {
        if(summon != nullptr)
            if(summon->GetInt32Value(UNIT_FIELD_UID) == summon_sid)
                return summon;
    }
    return nullptr;
}

void Player::SetLastContact(std::string szKey, std::string szValue)
{
    m_hsContact[szKey] = std::move(szValue);
}

void Player::SetLastContact(std::string szKey, uint32_t nValue)
{
    SetLastContact(std::move(szKey), std::to_string(nValue));
}

std::string Player::GetLastContactStr(std::string szKey)
{
    std::string res = "";
    if(m_hsContact.count(szKey) == 1)
        return m_hsContact[szKey];
    return res;
}

uint32_t Player::GetLastContactLong(std::string szKey)
{
    auto szValue = GetLastContactStr(szKey);
    return (uint32_t )std::stoul(szValue);
}

void Player::SetDialogTitle(std::string szTitle, int type)
{
    if(!szTitle.empty()) {
        m_nDialogType = type;

        if(type != 0)
            m_bNonNPCDialog = true;

        m_szDialogTitle = szTitle;
        m_szDialogMenu = "";
        m_szSpecialDialogMenu = "";
    }
}

void Player::SetDialogText(std::string szText)
{
    if(!szText.empty()) {
        m_szDialogText = szText;
        m_szDialogMenu = "";
        m_szSpecialDialogMenu = "";
    }
}

void Player::AddDialogMenu(std::string szKey, std::string szValue)
{
    if (!szKey.empty())
    {
        if (szKey.find('\t') == std::string::npos && szValue.find('\t') == std::string::npos)
        {
            m_szDialogMenu += "\t";
            m_szDialogMenu += szKey;
            m_szDialogMenu += "\t";
            m_szDialogMenu += szValue.empty() ? "" : szValue;
            m_szDialogMenu += "\t";
        }
    }
}

void Player::ShowDialog()
{
    if (m_szDialogTitle.length() > 0 || m_szDialogText.length() > 0) {
        uint npc = GetLastContactLong("npc");
        Messages::SendDialogMessage(this, npc, m_nDialogType, m_szDialogTitle, m_szDialogText, m_szDialogMenu);
        m_nDialogType   = 0;
        m_szDialogTitle = "";
        m_szDialogText  = "";
    }
}

bool Player::IsValidTrigger(std::string szTrigger)
{
    Tokenizer tokenizer(m_szDialogMenu, '\t');
    for(auto s : tokenizer)
    {
        if (s == szTrigger)
            return true;
    }
    return false;
}

ushort_t Player::ChangeGold(long nGold)
{
    if(nGold != GetGold()) {
        if(nGold > MAX_GOLD_FOR_INVENTORY)
            return TS_RESULT_TOO_MUCH_MONEY;
        if(nGold < 0)
            return TS_RESULT_TOO_CHEAP;
        SetUInt64Value(UNIT_FIELD_GOLD, nGold);
        SendGoldChaosMessage();
    }
    return TS_RESULT_SUCCESS;
}

void Player::PushItem(Item *pItem, int count, bool bSkipUpdateToDB)
{
    if(pItem->m_Instance.nOwnerUID == GetUInt32Value(UNIT_FIELD_UID)) {
        MX_LOG_ERROR("entities", "Player::PushItem(): tried to push already owned Item: %d, %s", pItem->m_Instance.nOwnerUID, GetName());
        return;
    }

    // In this case gold
    if(pItem->m_Instance.Code == 0) {
        long nPrevGoldAmount = GetGold();
        long gold = GetGold() + pItem->m_Instance.nCount;
        if(ChangeGold(gold) != 0) {
            // Log
        }
        Item::PendFreeItem(pItem);
        return;
    }

    if(pItem->m_Instance.nIdx == 0) {
        pItem->m_Instance.nIdx = (int)m_lInventory.size();
        pItem->m_bIsNeedUpdateToDB = true;
    }

    if(pItem->m_pItemBase->flaglist[FLAG_DUPLICATE] == 1) {
        auto i = FindItemByCode(pItem->m_Instance.Code);
        if(i != nullptr) {
            i->m_Instance.nCount += count;
            i->m_bIsNeedUpdateToDB = !bSkipUpdateToDB;
            Messages::SendItemMessage(this, i);
            return;
        }
    }

    pItem->SetOwnerInfo(GetHandle(), GetUInt32Value(UNIT_FIELD_UID), 0);
    if (pItem->GetWearType() == WearRideItem && m_anWear[WearRideItem] == nullptr)
        putonItem(WearRideItem, pItem);

    if(pItem->m_Instance.UID != 0) {
        // Update Item Owner
    }

    sMemoryPool->AllocItemHandle(pItem);

    if(!bSkipUpdateToDB) {
        pItem->DBInsert();
    }
    m_lInventory[m_lInventory.size()] = pItem;
    Messages::SendItemMessage(this, pItem);
}

void Player::ChangeLocation(float x, float y, bool bByRequest, bool bBroadcast)
{
    Position client_pos{};
    client_pos.Relocate(x,y,0,0);
    uint ct = sWorld->GetArTime();
    Position pos = this->GetCurrentPosition(ct);
    if(client_pos.GetExactDist2d(&pos) < 120.0f) {
        pos.m_positionX = client_pos.m_positionX;
        pos.m_positionY = client_pos.m_positionY;
        pos.m_positionZ = client_pos.m_positionZ;
        pos._orientation = client_pos._orientation;
    }
    int nl = sObjectMgr->GetLocationID(x, y);
    XPacket locPct(TS_SC_CHANGE_LOCATION);
    locPct << (int)m_nWorldLocationId;
    locPct << (int)nl;
    SendPacket(locPct);
    if(m_nWorldLocationId != nl) {
        if(m_nWorldLocationId != 0) {
            sWorldLocationMgr->RemoveFromLocation(this);
            this->m_WorldLocation = nullptr;
        }
        if(nl != 0) {
            this->m_WorldLocation = sWorldLocationMgr->AddToLocation(nl, this);
        }
        m_nWorldLocationId = nl;
    }
}

void Player::Update(uint diff)
{
    if(!IsInWorld())
        return;

    uint ct = sWorld->GetArTime();

    bool bIsMoving = IsMoving(ct);
    if(HasFlag(UNIT_FIELD_STATUS, MovePending)) {
        processPendingMove();
    }
    if(!bIsMoving) {
        onAttackAndSkillProcess();
    }

    Unit::Update(diff);
}

void Player::OnUpdate()
{
    uint ct = sWorld->GetArTime();
    if(m_nLastSaveTime + 30000 < ct) {
        this->Save(false);
        Position pos = GetCurrentPosition(ct);
        ChangeLocation(pos.GetPositionX(), pos.GetPositionY(), false, true);
    }

    for(auto summon : m_aBindSummonCard) {
        if(summon != nullptr && summon->m_pSummon != nullptr)
            summon->m_pSummon->OnUpdate();
    }

    Unit::OnUpdate();
}

void Player::onRegisterSkill(int skillUID, int skill_id, int prev_level, int skill_level)
{
    auto sb = sObjectMgr->GetSkillBase(skill_id);
    if(sb->id != 0 && sb->is_valid == 2)
        return;
    if(prev_level != 0) {
        Skill::DB_UpdateSkill(this,skillUID,skill_level);
    } else {
        Skill::DB_InsertSkill(this, skillUID, this->GetUInt32Value(UNIT_FIELD_UID), 0, skill_id, skill_level);
    }
    Messages::SendPropertyMessage(this, this, "jp", GetJP());
    Messages::SendSkillList(this,this,skill_id);
}

void Player::onExpChange()
{
    int level = 1;
    auto exp = GetEXP();
    long calcExp = 0;
    if(sObjectMgr->GetNeedExp(1) <= exp) {
        do
        {
            if (level >= 300)
                break;
            ++level;
        }
        while (sObjectMgr->GetNeedExp(level) <= exp);
    }
    level -= 1;
    Messages::SendEXPMessage(this, this);
    int oldLevel = GetLevel();
    if(level != 0 && level != oldLevel) {
        SetLevel(level);
        if(level < oldLevel) {
            this->CalculateStat();
        } else  {
//            sScriptingMgr->RunString(this, "on_player_level_up()");

            /*if(GetLevel() > GetUInt32Value(UNIT_FIELD_MAX_REACHED_LEVEL))
                SetUInt64Value(UNIT_FIELD_MAX_REACHED_LEVEL)*/

            this->CalculateStat();
            if(GetHealth() != 0) {
                SetHealth(GetMaxHealth());
                SetMana(GetMaxMana());
            }
            Messages::BroadcastHPMPMessage(this, 0, 0, false);
            // TODO: Guild & Party level update
        }
        this->Save(false);
        Messages::BroadcastLevelMsg(this);
    } else {
        if(m_nLastSaveTime + 3000 < sWorld->GetArTime())
            Save(true);
    }
}

void Player::onChangeProperty(std::string key, int value)
{
    if(key == "hp") {
        Messages::BroadcastHPMPMessage(this, value, 0, false);
        return;
    } else if(key == "lvl" || key == "lv" || key == "level") {
        SetEXP(sObjectMgr->GetNeedExp(value));
        return;
    } else if(key == "exp") {
        onExpChange();
        return;
    } else if(key == "gold") {
        SendGoldChaosMessage();
        return;
    } else if(key == "job") {
        this->CalculateStat();
        //return;
    } else if(key == "jlvl" || key == "jlv" || key == "job_level") {
        Messages::SendPropertyMessage(this, this, "job_level", GetCurrentJLv());
        return;
    }
    Messages::SendPropertyMessage(this, this, key, value);
}

void Player::AddSummon(Summon *pSummon, bool bSendMsg)
{
    m_vSummonList.emplace_back(pSummon);
    if(bSendMsg)
        Messages::SendAddSummonMessage(this, pSummon);
    if(pSummon->HasFlag(UNIT_FIELD_STATUS, StatusFlags::LoginComplete)) {
        Summon::DB_UpdateSummon(this, pSummon);
    }
}

Summon *Player::GetSummonByHandle(uint handle)
{
    for(auto s : m_vSummonList) {
        if(s == nullptr)
            continue;
        if(s->GetHandle() == handle)
            return s;
    }
    return nullptr;
}

void Player::PendWarp(int x, int y, uint8_t layer)
{
    Unit::SetFlag(UNIT_FIELD_STATUS, StatusFlags::Invincible);

    if(m_pMainSummon != nullptr)
        m_pMainSummon->SetFlag(UNIT_FIELD_STATUS, StatusFlags::Invincible);
    // PendWarp end, ProcessWarp start
    if(x >= 0.0f && y >= 0.0f /*MapWidth check*/) {
        int min_rx = -1, min_ry = -1, max_rx = 0, max_ry = 0;
        for(auto& s : m_vSummonList) {
            if(s == nullptr)
                continue;
            if(s->IsInWorld()) {
                if((int)((s->GetPositionX() + 108) / g_nRegionSize) < min_rx)
                    min_rx = (int)((s->GetPositionX() + 108) / g_nRegionSize);
                if ((int)((s->GetPositionX() + 108) /g_nRegionSize) > max_rx)
                    max_rx = (int)((s->GetPositionX() + 108) / g_nRegionSize);
                if ((int)((s->GetPositionY() + 112) /g_nRegionSize) < min_ry)
                    min_ry = (int)((GetPositionY() + 112) / g_nRegionSize);
                if ((int)((GetPositionY() + 112) / g_nRegionSize) > min_ry)
                    max_ry = (int)((GetPositionY() + 112) / g_nRegionSize);
            }
        }

        int rx = (int)(GetPositionX() / g_nRegionSize);
        int min_rx_value = (int)(x / g_nRegionSize);
        if (min_rx_value >= rx)
            min_rx_value = rx;

        int max_rx_value = (int)(x/ g_nRegionSize);
        if (max_rx_value < rx)
            max_rx_value = rx;

        int ry = (int)(GetPositionY() / g_nRegionSize);
        int min_ry_value = (int)(y / g_nRegionSize);
        if (min_ry_value >= ry)
            min_ry_value = ry;

        int max_ry_value = (int)(y / g_nRegionSize);
        if (max_ry_value < ry)
            max_ry_value = ry;

        if(IsInWorld()) {
            Position pos{};
            pos.Relocate(x, y, 0);

            sWorld->WarpBegin(this);
            sWorld->WarpEnd(this, pos, layer);
            ClearDialogMenu();
        }
    }
}

void Player::ClearDialogMenu()
{
    m_szDialogMenu = "";
}

void Player::LogoutNow(int callerIdx)
{
    if(IsInWorld()) {
        RemoveAllSummonFromWorld();
        sWorld->RemoveObjectFromWorld(this);
    }
    Save(false);
}

void Player::RemoveAllSummonFromWorld()
{
    for(auto& s : m_vSummonList) {
        if(s == nullptr)
            continue;
        if(s->IsInWorld()) {
            sWorld->RemoveObjectFromWorld(s);
        }
    }
}

void Player::SendPacket(XPacket &pPacket)
{
    if(m_session != nullptr) {
        if(m_session->GetSocket() != nullptr) {
            m_session->GetSocket()->SendPacket(pPacket);
        }
    }
}

bool Player::Erase(Item *pItem, uint64 count, bool bSkipUpdateToDB)
{
    if(FindItemByHandle(pItem->m_nHandle) == nullptr)
        return false;

    if(pItem->m_Instance.nCount <= count) {
        // TODO update weight
        PopItem(pItem, false);
        // POP ITEM
        return true;
    }

    // TODO update weight
    uint64 nc = pItem->m_Instance.nCount - count;
    SetItemCount(pItem, nc, bSkipUpdateToDB);
    return true;
}

void Player::SetItemCount(Item *pItem, uint64 nc, bool bSkipUpdateToDB)
{
    pItem->m_Instance.nCount = nc;
    Messages::SendItemCountMessage(this, pItem);
    if(!bSkipUpdateToDB)
        pItem->DBUpdate();
}

void Player::PopItem(Item *pItem, bool bSkipUpdateToDB)
{
    pItem->SetOwnerInfo(0, 0, 0);

    Messages::SendItemDestroyMessage(this, pItem);
    if(!bSkipUpdateToDB)
        pItem->DBUpdate();

    m_lInventory.erase(pItem->m_Instance.nIdx);

    delete pItem;
}

void Player::DoSummon(Summon* pSummon, Position pPosition)
{
    /*            if (!this.m_bIsSummonable)
                return false;*/
    if(pSummon->IsInWorld() /*|| m_pMainSummon != nullptr*/)
        return;

    DoUnSummon(m_pMainSummon);

    // TODO Do Subsummon here
    m_pMainSummon = pSummon;
    pSummon->SetCurrentXY(pPosition.GetPositionX(), pPosition.GetPositionY());
    pSummon->m_nLayer = this->GetLayer();
    pSummon->StopMove();
    sWorld->AddSummonToWorld(pSummon);
    pSummon->SetFlag(UNIT_FIELD_STATUS, StatusFlags::NeedToCalculateStat);
}

void Player::DoUnSummon(Summon *pSummon)
{
    if(pSummon == nullptr)
        return;

    if(!pSummon->IsInWorld())
        return;

    m_pMainSummon = nullptr;

    XPacket usPct(TS_SC_UNSUMMON);
    usPct << pSummon->GetHandle();
    sWorld->Broadcast((uint)(pSummon->GetPositionX() / g_nRegionSize), (uint)(pSummon->GetPositionY() / g_nRegionSize), pSummon->GetLayer(), usPct);
    if(sArRegion->IsVisibleRegion((uint)(pSummon->GetPositionX() / g_nRegionSize), (uint)(pSummon->GetPositionY() / g_nRegionSize),
                                  (uint)(GetPositionX() / g_nRegionSize), (uint)(GetPositionY() / g_nRegionSize)) == 0) {
        SendPacket(usPct);
    }
    sWorld->RemoveObjectFromWorld(pSummon);
}

void Player::onCantAttack(uint target, uint t)
{
    if(!bIsMoving || !IsInWorld()) {
        if(m_nLastCantAttackTime + 100 < t) {
            m_nLastCantAttackTime = t;
            Messages::SendCantAttackMessage(this, this->GetHandle(), target, TS_RESULT_TOO_FAR);
        }
    }
}

bool Player::TranslateWearPosition(ItemWearType &pos, Item *pItem, std::vector<int> *vpOverlappItemList)
{
    if (!Unit::TranslateWearPosition(pos, pItem, vpOverlappItemList))
        return false;
    if ((pItem->m_Instance.Flag & 1) != 0)
        return false;
    bool ok{false};
    if (pItem->m_pItemBase->limit_hunter != 0 && IsHunter())
        ok  = true;
    if (pItem->m_pItemBase->limit_fighter != 0 && IsFighter())
        ok  = true;
    if (pItem->m_pItemBase->limit_magician != 0 && IsMagician())
        ok  = true;
    if (pItem->m_pItemBase->limit_summoner != 0 && IsSummoner())
        ok  = true;

    if (!ok
        || (GetRace() == 3 && pItem->m_pItemBase->limit_gaia == 0)
        || (GetRace() == 4 && pItem->m_pItemBase->limit_deva == 0)
        || (GetRace() == 5 && pItem->m_pItemBase->limit_asura == 0))
        return false;

    Item *item1{nullptr}, *item2{nullptr};
    if (pos == ItemWearType::WearShield) {
        if (pItem->m_pItemBase->group == ItemGroup::Bullet) {
            item1 = m_anWear[0];
            if (item1 == nullptr
                || item1->m_pItemBase->iclass != ItemClass::ClassCrossBow
                || item1->m_pItemBase->iclass != ItemClass::ClassLightBow
                || item1->m_pItemBase->iclass != ItemClass::ClassHeavyBow)
                return false;
        }
        if (pItem->m_pItemBase->group == ItemGroup::Weapon) {
            item1 = m_anWear[0];
            if (item1 == nullptr)
                return false;

            if (item1->m_pItemBase->iclass == ItemClass::ClassOneHandSword && pItem->m_pItemBase->iclass == ItemClass::ClassOneHandSword) {
                // TODO: Check for PassiveSkill
                pos = ItemWearType::WearWeapon;
            } else if (item1->m_pItemBase->iclass == ItemClass::ClassDagger && pItem->m_pItemBase->iclass == ItemClass::ClassDagger) {
                // TODO: Check for PassiveSkill
                pos = ItemWearType::WearWeapon;
            } else if (item1->m_pItemBase->iclass == ItemClass::ClassOneHandAxe && pItem->m_pItemBase->iclass == ItemClass::ClassOneHandAxe) {
                // TODO: CheckForPassiveSkill
                pos = ItemWearType::WearWeapon;
            }
        }
    }
    if (pos == ItemWearType::WearDecoShield) {
        if (pItem->m_pItemBase->iclass == ItemClass::ClassDecoOneHandSword
            || pItem->m_pItemBase->iclass == ItemClass::ClassDecoTwoHandSword
            || pItem->m_pItemBase->iclass == ItemClass::ClassDecoDagger
            || pItem->m_pItemBase->iclass == ItemClass::ClassDecoTwoHandSpear
            || pItem->m_pItemBase->iclass == ItemClass::ClassDecoTwoHandAxe
            || pItem->m_pItemBase->iclass == ItemClass::ClassDecoOneHandMace
            || pItem->m_pItemBase->iclass == ItemClass::ClassDecoTwoHandMace
            || pItem->m_pItemBase->iclass == ItemClass::ClassDecoHeavyBow
            || pItem->m_pItemBase->iclass == ItemClass::ClassDecoLightBow
            || pItem->m_pItemBase->iclass == ItemClass::ClassDecoCrossBow
            || pItem->m_pItemBase->iclass == ItemClass::ClassDecoOneHandStaff
            || pItem->m_pItemBase->iclass == ItemClass::ClassDecoTwoHandStaff
            || pItem->m_pItemBase->iclass == ItemClass::ClassOneHandAxe) {
            item1 = m_anWear[1];
            item2 = m_anWear[0];
            if (item1 != nullptr
                && item1->m_pItemBase->iclass != ItemClass::ClassOneHandSword
                && item1->m_pItemBase->iclass != ItemClass::ClassDagger
                && item1->m_pItemBase->iclass != ItemClass::ClassOneHandAxe
                || item2 == nullptr
                || item2->m_pItemBase->iclass != ItemClass::ClassOneHandSword
                || item2->m_pItemBase->iclass != ItemClass::ClassDagger
                || item2->m_pItemBase->iclass != ItemClass::ClassOneHandAxe
                || m_anWear[14] == nullptr) { // TODO: PassiveSkill
                pos = ItemWearType::WearDecoWeapon;
            }
        } else {
            item1 = m_anWear[1];
            item2 = m_anWear[0];
            if (item1 != nullptr && item1->m_pItemBase->iclass != ItemClass::ClassShield
                || item2 != nullptr && item2->m_pItemBase->wear_type == ItemWearType::WearTwoHand) {
                return false;
            }
        }
    }
    if (pItem->GetWearType() == ItemWearType::WearTwoHand)
        pos = ItemWearType::WearWeapon;
    if (pos == ItemWearType::WearTwoFingerRing)
        pos = ItemWearType::WearRing;
    item1   = m_anWear[9];
    if (item1 != nullptr) {
        if (item1->GetWearType() != ItemWearType::WearTwoFingerRing && pItem->GetWearType() == ItemWearType::WearRing)
            pos = ItemWearType::WearSecondRing;
    }
    if (pos == ItemWearType::WearBagSlot) {
        item1 = m_anWear[23];
        if (item1 != nullptr) {
            if (item1->m_Instance.Code != pItem->m_Instance.Code) {
                int nCurrentVarIdx = -1;

                for (int i = 0; i < 4; ++i) {
                    if (item1->m_pItemBase->opt_type[i] == 20) {
                        nCurrentVarIdx = i;
                        break;
                    }
                }
                int nCurrentVarIdx2 = -1;

                for (int i = 0; i < 4; ++i) {
                    if (pItem->m_pItemBase->opt_type[i] == 20) {
                        nCurrentVarIdx2 = i;
                        break;
                    }
                }

                if (nCurrentVarIdx == -1)
                    return false;
                if (nCurrentVarIdx2 == -1)
                    return false;

                // TODO Check Capacity
            }
        }
    }
    if (pos != ItemWearType::WearDecoWeapon && pos != ItemWearType::WearDecoShield) {

    } else {
        item1 = m_anWear[pos != ItemWearType::WearDecoWeapon ? 1 : 0];
        if(item1 == nullptr || (int)item1->m_pItemBase->iclass > (int)ItemClass::ClassShield)
            return false;
        switch(item1->m_pItemBase->iclass) {
            case ItemClass::ClassShield:
                if (pItem->m_pItemBase->iclass != ItemClass::ClassDecoShield)
                    return false;
                break;
            case ItemClass::ClassOneHandSword:
                if (pItem->m_pItemBase->iclass != ItemClass::ClassDecoOneHandSword)
                    return false;
                break;
            case ItemClass::ClassTwoHandSword:
                if (pItem->m_pItemBase->iclass != ItemClass::ClassDecoTwoHandSword)
                    return false;
                break;
            case ItemClass::ClassDagger:
                if (pItem->m_pItemBase->iclass != ItemClass::ClassDecoDagger)
                    return false;
                break;
            case ItemClass::ClassSpear:
                if (pItem->m_pItemBase->iclass != ItemClass::ClassDecoTwoHandSpear)
                    return false;
                break;
            case ItemClass::ClassTwoHandAxe:
                if (pItem->m_pItemBase->iclass != ItemClass::ClassDecoTwoHandAxe)
                    return false;
                break;
            case ItemClass::ClassOneHandMace:
                if (pItem->m_pItemBase->iclass != ItemClass::ClassDecoOneHandMace)
                    return false;
                break;
            case ItemClass::ClassTwoHandMace:
                if (pItem->m_pItemBase->iclass != ItemClass::ClassDecoTwoHandMace)
                    return false;
                break;
            case ItemClass::ClassHeavyBow:
                if (pItem->m_pItemBase->iclass != ItemClass::ClassDecoHeavyBow)
                    return false;
                break;
            case ItemClass::ClassLightBow:
                if (pItem->m_pItemBase->iclass != ItemClass::ClassDecoLightBow)
                    return false;
                break;
            case ItemClass::ClassCrossBow:
                if(pItem->m_pItemBase->iclass != ItemClass::ClassDecoCrossBow)
                    return false;
                break;
            case ItemClass::ClassOneHandStaff:
                if(pItem->m_pItemBase->iclass != ItemClass::ClassDecoOneHandStaff)
                    return false;
                break;
            case ItemClass::ClassTwoHandStaff:
                if(pItem->m_pItemBase->iclass != ItemClass::ClassDecoTwoHandStaff)
                    return false;
                break;
            case ItemClass::ClassOneHandAxe:
                if(pItem->m_pItemBase->iclass != ItemClass::ClassDecoOneHandAxe)
                    return false;
                break;
            default:
                return false;
        }
    }
    if(pos >= 24 || pos < 0)
        return false;

    if(vpOverlappItemList != nullptr) {
        if(m_anWear[pos] != nullptr)
            vpOverlappItemList->emplace_back(pos);
        if(pItem->m_pItemBase->wear_type == ItemWearType::WearTwoHand) {
            if(m_anWear[1] != nullptr) {
                if (!pItem->IsBow() && !pItem->IsCrossBow()
                    || m_anWear[1]->m_pItemBase->group != ItemGroup::Bullet) {
                    vpOverlappItemList->emplace_back(1);
                }
                if(m_anWear[15] != nullptr) {
                    vpOverlappItemList->emplace_back(15);
                }
            }
        }
        if(pos == ItemWearType::WearShield) {
            item1 = m_anWear[15];
            if(item1 != nullptr) {
                if(pItem->m_pItemBase->iclass == ItemClass::ClassShield && item1->m_pItemBase->iclass != ItemClass::ClassDecoShield)
                    vpOverlappItemList->emplace_back(15);
                if(item1->m_pItemBase->iclass == ItemClass::ClassDecoShield)
                    vpOverlappItemList->emplace_back(15);
            }
        }
        item1 = m_anWear[0];
        if(item1 != nullptr) {
            if(item1->m_pItemBase->wear_type == ItemWearType::WearTwoHand) {
                if(pos == ItemWearType::WearShield) {
                    if(!item1->IsBow() && !item1->IsCrossBow() || pItem->m_pItemBase->group != ItemGroup::Bullet)
                        vpOverlappItemList->emplace_back(0);
                }
            }
        }

        if(m_anWear[10] != nullptr && pItem->GetWearType() == ItemWearType::WearTwoFingerRing)
            vpOverlappItemList->emplace_back(10);

        item1 = m_anWear[9];
        if(item1 != nullptr) {
            if(item1->GetWearType() == ItemWearType::WearTwoFingerRing) {
                if(pItem->GetWearType() == ItemWearType::WearRing)
                    vpOverlappItemList->emplace_back(9);
            }
        }
    }
    return true;
}

bool Player::IsHunter()
{
    int job_id = GetCurrentJob();
    if (job_id == 0)
    {
        switch(GetRace())
        {
            case 3:
                job_id = 100;
                break;
            case 4:
                job_id = 200;
                break;
            case 5:
                job_id = 300;
                break;
            default:
                break;
        }
    }
    auto info = sObjectMgr->GetJobInfo((uint)job_id);

    if (info != nullptr)
        return info->job_class == 2;
    return false;
}

bool Player::IsFighter()
{
    int job_id = GetCurrentJob();
    if (job_id == 0)
    {
        switch(GetRace())
        {
            case 3:
                job_id = 100;
                break;
            case 4:
                job_id = 200;
                break;
            case 5:
                job_id = 300;
                break;
            default:
                break;
        }
    }
    auto info = sObjectMgr->GetJobInfo((uint)job_id);

    if (info != nullptr)
        return info->job_class == 1;
    return false;
}

bool Player::IsMagician()
{
    int job_id = GetCurrentJob();
    if (job_id == 0)
    {
        switch(GetRace())
        {
            case 3:
                job_id = 100;
                break;
            case 4:
                job_id = 200;
                break;
            case 5:
                job_id = 300;
                break;
            default:
                break;
        }
    }
    auto info = sObjectMgr->GetJobInfo((uint)job_id);

    if (info != nullptr)
        return info->job_class == 3;
    return false;
}

bool Player::IsSummoner()
{
    int job_id = GetCurrentJob();
    if (job_id == 0)
    {
        switch(GetRace())
        {
            case 3:
                job_id = 100;
                break;
            case 4:
                job_id = 200;
                break;
            case 5:
                job_id = 300;
                break;
            default:
                break;
        }
    }
    auto info = sObjectMgr->GetJobInfo((uint)job_id);

    if (info != nullptr)
        return info->job_class == 4;
    return false;
}

bool Player::IsInProgressQuest(int code)
{
    bool result{false};

    auto q =m_QuestManager.FindQuest(code);
    if(q != nullptr)
        result = !q->IsFinishable();
    else
        result = false;
    return result;
}

bool Player::IsStartableQuest(int code, bool bForQuestMark)
{
    auto qbs = sObjectMgr->GetQuestBase(code);
    if (qbs->nLimitLevel - GetLevel() > 4
        || qbs->nLimitJobLevel > GetCurrentJLv()
        || bForQuestMark
           && qbs->nLimitIndication != 0
           && GetLevel() - qbs->nLimitLevel > 12)
        return false;

    if (qbs->nLimitJob != 0) {
        if (qbs->nLimitJob != GetCurrentJob())
            return false;
    } else {
        if ((!IsHunter() || (qbs->LimitFlag & 0x20) == 0)
            && (!IsFighter() || (qbs->LimitFlag & 0x10) == 0)
            && (!IsMagician() || (qbs->LimitFlag & 0x40) == 0)
            && (!IsSummoner() || (qbs->LimitFlag & 0x80) == 0))
            return false;
    }
    if ((GetRace() != 3 || (qbs->LimitFlag & 8) == 0)
        && (GetRace() != 4 || (qbs->LimitFlag & 2) == 0)
        && (GetRace() != 5 || (qbs->LimitFlag & 4) == 0))
        return false;
    int fgid = qbs->nLimitFavorGroupID;
    if(fgid == 999) {
        auto npc = sMemoryPool->GetObjectInWorld<NPC>(GetLastContactLong("npc"));

        if(npc != nullptr)
            fgid = npc->m_pBase->id;
        else
            fgid = 0;
    }
    // TODO: Favor
    return m_QuestManager.IsStartableQuest(code);
}

bool Player::IsFinishableQuest(int code)
{
    return CheckFinishableQuestAndGetQuestStruct(code);
}

bool Player::CheckFinishableQuestAndGetQuestStruct(int code)
{
    bool result{false};
    auto q1 = m_QuestManager.FindQuest(code);
    if(q1 != nullptr && q1->IsFinishable())  {
        result = true;
    } else {
        result = false;
    }
    return result;
}

void Player::onStatusChanged(Quest *quest, int nOldStatus, int nNewStatus)
{

}

void Player::onProgressChanged(Quest *quest, QuestProgress oldProgress, QuestProgress newProgress)
{

}

Quest *Player::FindQuest(int code)
{
    return m_QuestManager.FindQuest(code);
}

float Player::GetMoveSpeed() const
{
    return m_Attribute.nMoveSpeed;
}

void Player::onModifyStatAndAttribute()
{
    Messages::SendStatInfo(this, this);
}

uint16 Player::IsUseableItem(Item *pItem, Unit *pTarget)
{
    uint ct = sWorld->GetArTime();
    if(pItem->m_pItemBase->cool_time_group < 0 || pItem->m_pItemBase->cool_time_group > 40 || pItem->m_pItemBase->cool_time_group != 0
        && false/*m_nItemCoolTime[pItem->m_pItemBase->cool_time_group] > ct*/)
        return TS_RESULT_COOL_TIME;
    // Weight
    // Ride IDX
    if(pItem->m_pItemBase->use_max_level != 0 && pItem->m_pItemBase->use_max_level < GetLevel())
        return TS_RESULT_LIMIT_MAX;
    if(pItem->m_pItemBase->use_min_level <= GetLevel()) {
        if(pTarget == nullptr)
            return TS_RESULT_SUCCESS;

        if(pItem->m_pItemBase->target_max_level != 0 && pItem->m_pItemBase->target_max_level < pTarget->GetLevel())
            return TS_RESULT_LIMIT_MAX;
        if(pItem->m_pItemBase->target_min_level <= pTarget->GetLevel())
            return TS_RESULT_SUCCESS;
    }
    return TS_RESULT_LIMIT_MIN;
}

uint16 Player::UseItem(Item *pItem, Unit *pTarget, const std::string &szParameter)
{
    if(pTarget == nullptr)
        pTarget = this;
    if(pItem->m_Instance.nCount < 1)
        return TS_RESULT_NOT_ENOUGH_ITEM; // NOT_ACTABLE?


    uint16  result{0};
    for(int i = 0; i < Item::MAX_OPTION_NUMBER; ++i) {
        if(pItem->m_pItemBase->base_type[i] != 0) {
            result = pTarget->onItemUseEffect(this, pItem, pItem->m_pItemBase->base_type[i], pItem->m_pItemBase->base_var[i][0], pItem->m_pItemBase->base_var[i][1], szParameter);
            if(result != TS_RESULT_SUCCESS)
                return result;
        }

        if(pItem->m_pItemBase->opt_type[i] != 0) {
            result = pTarget->onItemUseEffect(this, pItem, pItem->m_pItemBase->opt_type[i], pItem->m_pItemBase->opt_var[i][0], pItem->m_pItemBase->opt_var[i][1], szParameter);
            if(result != TS_RESULT_SUCCESS)
                return result;
        }

    }
}

CreatureStat *Player::GetBaseStat() const
{
    uint stat_id = 0;
    auto job = sObjectMgr->GetJobInfo(GetCurrentJob());
    if(job != nullptr)
        stat_id = job->stat_id;
    return sObjectMgr->GetStatInfo(stat_id);
}

Item *Player::FindItem(uint code, uint flag, bool bFlag)
{
    for(auto& i : m_lInventory) {
        bool isFlagged = (flag & i.second->m_Instance.Flag) != 0;
        if(i.second->m_Instance.Code == code) {
            if(bFlag == isFlagged)
                return i.second;
        }
    }
    return nullptr;
}

void Player::DoEachPlayer(const std::function<void(Player *)> &fn)
{
    MX_SHARED_GUARD readGuard(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& m = sMemoryPool->GetPlayers();
    for(HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if(itr->second != nullptr)
            fn(itr->second);
    }
}

Player* Player::FindPlayer(const std::string &szName)
{
    MX_SHARED_GUARD readGuard(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& m = sMemoryPool->GetPlayers();
    for(HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if(itr->second->GetName() == szName)
            return itr->second;
    }
    return nullptr;
}
