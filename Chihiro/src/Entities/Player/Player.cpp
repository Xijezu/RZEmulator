#include "Player/Player.h"
#include "MemPool.h"
#include "Database/DatabaseEnv.h"
#include "Network/GameNetwork/GameHandler.h"
#include "ObjectMgr.h"
#include "GameNetwork/ClientPackets.h"
#include "Messages.h"
#include "Scripting/XLua.h"
#include "World.h"
#include "Skill.h"
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
    _valuesCount = UNIT_END;

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
        m_lInventory.erase(t.first);
        delete t.second;
    }
    for(auto &t : m_vSummonList) {
        delete t;
    }
    m_vSummonList.clear();
}

void Player::EnterPacket(XPacket &pEnterPct, Player *pPlayer)
{
    Unit::EnterPacket(pEnterPct, pPlayer);
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

        if(getLevel() == 0) {
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
            Field *fields = result->Fetch();
            auto skill = new Skill();
            skill->sid = fields[0].GetInt32();
            skill->owner_id = fields[1].GetInt32();
            skill->summon_id = fields[2].GetInt32();
            skill->skill_id = fields[3].GetInt32();
            skill->skill_level = fields[4].GetInt32();
            skill->cool_time = fields[5].GetInt32();
            if(skill->summon_id == 0)
                m_vSkillList.push_back(skill);
        } while (result->NextRow());
    }
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
            if(item != nullptr && unit != nullptr) {
                if(unit->m_anWear[wear_info] == nullptr) {
                    if(true) {// TODO: TranslateWearPosition
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
            summon->SetInt32Value(UNIT_FIELD_PREV_JOB, prev_level_01);
            summon->SetInt32Value(UNIT_FIELD_PREV_JOB + 1, prev_level_02);
            summon->SetInt32Value(UNIT_FIELD_PREV_JLV, prev_id_01);
            summon->SetInt32Value(UNIT_FIELD_PREV_JLV + 1, prev_id_02);
            summon->SetInt32Value(UNIT_FIELD_HEALTH, hp);
            summon->SetInt32Value(UNIT_FIELD_MANA, mp);
            summon->m_nTransform = transform;
            summon->CalculateStat();
            Item* card = FindItemBySID(card_uid);
            if(card != nullptr) {
                card->m_pSummon = summon;
                card->m_Instance.Socket[0] = sid;
                card->m_bIsNeedUpdateToDB = true;
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
    packet << (uint64) 0;
    packet << value;
    packet << (uint8) 0;
    packet.FinalizePacket();
    m_session->GetSocket().SendPacket(packet);
}

void Player::SendPropertyMessage(std::string key, int64 value)
{
    XPacket packet(TS_SC_PROPERTY);
    packet << GetUInt32Value(UNIT_FIELD_HANDLE);
    packet << (uint8) 1;
    packet.fill(key, 16);
    packet << value;
    packet.FinalizePacket();
    m_session->GetSocket().SendPacket(packet);
}

void Player::SendLoginProperties()
{
    sWorld->AddSession(&this->GetSession());
    CalculateStat();
    // Login();
    for(auto summon: m_vSummonList) {
        Messages::SendAddSummonMessage(this, summon);
    }

    Messages::SendItemList(this, false);
    Messages::SendCreatureEquipMessage(this, false);

    Messages::SendSkillList(this, this, 0);
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

    Messages::SendGameTime(this);
    ChangeLocation(GetPositionX(), GetPositionY(), false, false);

    if(!_bIsInWorld) {
        AddToWorld();
        sWorld->AddObjectToWorld(this);
    }

    if (m_pMainSummon != nullptr) {
        m_pMainSummon->SetFlag(UNIT_FIELD_STATUS, StatusFlags::Invincible);
        m_pMainSummon->SetCurrentXY(GetPositionX(),GetPositionY());
        m_pMainSummon->AddNoise(rand32(), rand32(), 30);
        m_pMainSummon->SetLayer(GetLayer());
        sWorld->AddSummonToWorld(m_pMainSummon);
    }
}

void Player::SendGoldChaosMessage()
{
    XPacket packet(TS_SC_GOLD_UPDATE);
    packet << GetUInt64Value(UNIT_FIELD_GOLD);
    packet << GetInt32Value(UNIT_FIELD_CHAOS);
    packet.FinalizePacket();
    m_session->GetSocket().SendPacket(packet);
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
    packet.FinalizePacket();
    m_session->GetSocket().SendPacket(packet);
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
    stmt->setInt32(i++, getLevel());
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
    stmt->setInt32(i++, GetInt32Value(UNIT_FIELD_UID));
    CharacterDatabase.Execute(stmt);

    if(!bOnlyPlayer) {
        for (auto item : m_lInventory) {
            item.second->DBUpdate();
        }
    }
}

int Player::GetJobDepth()
{
    int  job = GetCurrentJob();
    auto res = sObjectMgr->GetJobInfo(GetCurrentJob());
    return res.job_depth;
}

void Player::applyJobLevelBonus()
{
    int          levels[4]{ };
    int          jobs[4]{ };
    int          i = 0;
    CreatureStat stat{ };

    if (GetCurrentJob() != 0) {
        int jobDepth = GetJobDepth();
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

uint16_t Player::putonItem(ItemWearType pos, Item *item)
{
    if (pos > Item::MAX_ITEM_WEAR || pos < 0)
        return TS_RESULT_ACCESS_DENIED;

    if(m_anWear[pos] != nullptr) {
        m_anWear[pos]->m_Instance.nWearInfo = WearNone;
        m_anWear[pos]->m_bIsNeedUpdateToDB = true;
    }

    item->m_bIsNeedUpdateToDB  = true;
    item->m_Instance.nWearInfo = pos;
    m_anWear[pos] = item;
    SendItemWearInfoMessage(*item, this);
    return TS_RESULT_SUCCESS;
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
        SendItemWearInfoMessage(*item, this);
    }
    return TS_RESULT_SUCCESS;
}

void Player::SendItemWearInfoMessage(Item item, Unit *u)
{
    XPacket packet(TS_SC_ITEM_WEAR_INFO);
    packet << (uint32_t) item.m_nHandle;
    packet << (int16_t) item.m_Instance.nWearInfo;
    packet << (uint32_t) (u != nullptr ? u->GetHandle() : 0);
    packet << (int32_t) item.m_Instance.nEnhance;
    GetSession().GetSocket().SendPacket(packet);
}

Summon* Player::GetSummon(int summon_sid)
{
    for(auto summon : m_vSummonList) {
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
            m_szDialogMenu += szValue.empty() ? "0" : szValue;
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
    if(pItem->m_Instance.nIdx == 0) {
        pItem->m_Instance.nIdx = (int)m_lInventory.size();
        pItem->m_bIsNeedUpdateToDB = true;
    }

    if(pItem->m_pItemBase.flaglist[FLAG_DUPLICATE] == 1) {
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
    m_session->GetSocket().SendPacket(locPct);
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
    SkillBase sb = sObjectMgr->GetSkillBase(skill_id);
    if(sb.id != 0 && sb.is_valid == 2)
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
    int oldLevel = getLevel();
    if(level != 0 && level != oldLevel) {
        SetLevel(level);
        if(level < oldLevel) {
            this->CalculateStat();
        } else  {
//            sScriptingMgr->RunString(this, "on_player_level_up()");

            /*if(getLevel() > GetUInt32Value(UNIT_FIELD_MAX_REACHED_LEVEL))
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
        // Update Summon
    }
}

Summon *Player::GetSummonByHandle(uint handle)
{
    for(auto s : m_vSummonList) {
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
        RemoveFromWorld();
        Save(false);
        sWorldLocationMgr->RemoveFromLocation(this);
    }
}

void Player::RemoveAllSummonFromWorld()
{
    for(auto& s : m_vSummonList) {
        if(s->IsInWorld()) {
            s->RemoveFromWorld();
            sWorld->RemoveObjectFromWorld(s);
        }
    }
}
