#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "XPacket.h"
#include "Common.h"
#include "Object.h"
#include "Item/Item.h"
#include "Summon/Summon.h"
#include "WorldLocation.h"
#include "TimeSync.h"
#include "QuestManager.h"
class WorldSession;

#define MAX_ITEM_COOLTIME_GROUP 20

enum BONUS_TYPE : int
{
    BONUS_PCBANG         = 0x0,
    BONUS_STAMINA        = 0x1,
    BONUS_PREMIUM_PCBANG = 0x2,
    MAX_BONUS_TYPE       = 0x3,
};

struct BonusInfo
{
    int type;
    int rate;
    int64 exp;
    int jp;
};

enum CONDITION_INFO : int
{
    CONDITION_GOOD    = 0,
    CONDITION_AVERAGE = 1,
    CONDITION_BAD     = 2
};

class Player : public Unit, public QuestEventHandler {
public:
    friend class Messages;

    explicit Player(uint32);
    ~Player() override;
    void CleanupsBeforeDelete();

    /* ****************** STATIC FUNCTIONS ****************** */
    static void EnterPacket(XPacket &, Player *, Player*);
    static void DoEachPlayer(const std::function<void (Player*)>& fn);
    static Player* FindPlayer(const std::string& szName);
    static void DB_ItemCoolTime(Player*);
    /* ****************** STATIC END ****************** */

    /* ****************** QUEST ****************** */
    void DoEachActiveQuest(const std::function<void(Quest *)> &fn) { m_QuestManager.DoEachActiveQuest(fn); }
    void UpdateQuestStatusByMonsterKill(int monster_id);
    void GetQuestByMonster(int monster_id, std::vector<Quest*> &vQuest, int type);
    void StartQuest(int code, int nStartQuestID, bool bForce);
    void EndQuest(int code, int nRewardID, bool bForce);
    void UpdateQuestStatusByItemUpgrade();
    void onStatusChanged(Quest* quest, int nOldStatus, int nNewStatus) override ;
    void onProgressChanged(Quest* quest, QuestProgress oldProgress, QuestProgress newProgress) override;
    int GetQuestProgress(int nQuestID);
    int GetActiveQuestCount() const { return (int)m_QuestManager.m_vActiveQuest.size(); }
    bool IsTakeableQuestItem(int code) { return m_QuestManager.IsTakeableQuestItem(code); }
    bool CheckFinishableQuestAndGetQuestStruct(int code, Quest *&pQuest, bool bForce);
    bool IsInProgressQuest(int code);
    bool IsStartableQuest(int code, bool bForQuestMark);
    bool IsFinishableQuest(int code);
    bool CheckFinishableQuestAndGetQuestStruct(int code);
    Quest* FindQuest(int code);
    /* ****************** QUEST END ****************** */

    int GetPermission() { return GetInt32Value(UNIT_FIELD_PERMISSION); }
    int64 GetGold() { return GetUInt64Value(UNIT_FIELD_GOLD); }
    int GetPartyID() { return GetInt32Value(UNIT_FIELD_PARTY_ID); }
    int GetGuildID() { return GetInt32Value(UNIT_FIELD_GUILD_ID); }
    int AddStamina(int nStamina);
    int GetStaminaRegenRate();
    CONDITION_INFO GetCondition() const;

    uint GetJobDepth();
    std::string GetFlag(const std::string& flag) { return m_lFlagList[flag]; }

    /* ****************** DATABASE ****************** */
    bool ReadCharacter(std::string, int);
    bool ReadItemList(int);
    bool ReadItemCoolTimeList(int);
    bool ReadSummonList(int);
    bool ReadEquipItem();
    bool ReadSkillList(Unit*);
    bool ReadQuestList();
    bool ReadStateList(Unit*);
    /* ****************** DATABASE END ****************** */

    // Warping
    void PendWarp(int x, int y, uint8_t layer);

    void SetFlag(const std::string& key, std::string value);

    // Network
    void SendPropertyMessage(std::string key, std::string value);
    void SendPropertyMessage(std::string key, int64 value);

    // Summon
    void LogoutNow(int callerIdx);
    void RemoveAllSummonFromWorld();

    void SendLoginProperties();
    void SendGoldChaosMessage();
    void SendJobInfo();
    void SendWearInfo();

    Summon *GetSummon(int);
    Summon *GetSummonByHandle(uint);
    void AddSummon(Summon*,bool);

    // Database
    void Update(uint diff) override;
    void OnUpdate() override;
    void Save(bool);

    // Item relevant
    Item *FindItemByCode(int);
    Item *FindItemBySID(uint64_t);
    Item *FindItemByHandle(uint32_t);
    Item* FindItem(uint code, uint flag, bool bFlag);
    void PushItem(Item *, uint64, bool);
    void PopItem(Item*,bool);
    bool EraseBullet(int64 count);

    bool Erase(Item*,int64,bool);
    void SetItemCount(Item* pItem, int64 nc, bool bSkipUpdateToDB);
    void AddEXP(int64 exp, uint jp, bool bApplyStanima) override;
    uint16_t putonItem(ItemWearType, Item *) override;
    uint16_t putoffItem(ItemWearType) override;
    void SendItemWearInfoMessage(Item* item, Unit *u);
    void ChangeLocation(float x, float y, bool bByRequest, bool bBroadcast);

    /* ****************** DIALOG ****************** */
    void SetLastContact(std::string, uint32_t);
    void SetLastContact(std::string, std::string);
    std::string GetLastContactStr(std::string);
    uint32_t GetLastContactLong(std::string);
    void SetDialogTitle(std::string, int);
    void SetDialogText(std::string);
    void AddDialogMenu(std::string, std::string);
    void ClearDialogMenu();
    bool IsFixedDialogTrigger(const std::string& szTrigger) { return false; }
    bool HasDialog() { return !m_szDialogTitle.empty(); }
    void ShowDialog();
    bool IsValidTrigger(const std::string&);
    /* ****************** DIALOG END ****************** */

    /* ****************** WORLDLOCATION BEGIN ****************** */
    bool IsInTown();
// Function       :   public bool StructPlayer::IsInField()
    bool IsInBattleField();
    bool IsInEventmap();
// Function       :   public bool IsUsingTent()
    bool IsInSiegeOrRaidDungeon();
    bool IsInSiegeDungeon();
    bool IsInDungeon();
    /* ****************** WORLDLOCATION END ****************** */


    void onChangeProperty(std::string, int);
    Position GetLastTownPosition();

    void DoSummon(Summon* pSummon, Position pPosition);
    void DoUnSummon(Summon* pSummon);

    bool IsHunter();
    bool IsFighter();
    bool IsMagician();
    bool IsSummoner();
    bool IsUsingBow() const override;
    bool IsUsingCrossBow() const override;
    bool IsPlayer() const override { return true; }

    float GetMoveSpeed() const;

    bool TranslateWearPosition(ItemWearType& pos, Item* item, std::vector<int>& ItemList) override;

    CreatureStat* GetBaseStat() const override;
    uint GetCreatureGroup() const override { return 9; }

    ushort ChangeGold(int64);

    uint16 IsUseableItem(Item* pItem, Unit* pTarget);
    uint16 UseItem(Item* pItem, Unit* pTarget, const std::string& szParameter);

    void SendPacket(XPacket pPacket);

    WorldSession& GetSession() const
    { return *m_session; }

    void SetClientInfo(const std::string& value) { m_szClientInfo = value; }

    void SetSession(WorldSession *session) { m_session = session; }

    void applyJobLevelBonus() override;

    std::map<uint64, Item *>             m_lInventory;
    Item                                    *m_aBindSummonCard[6]{nullptr};
    WorldLocation *m_WorldLocation{nullptr};
    int m_nWorldLocationId{0};
    TimeSynch m_TS{200,2,10};

    Summon* m_pMainSummon{nullptr};
    uint m_hTamingTarget{};
    int GetChaos() const;
    int GetMaxChaos() const;
    void AddChaos(int chaos);
protected:

    bool isInLocationType(uint8 nLocationType);
    void onCompleteCalculateStat() override;
    void onBeforeCalculateStat() override;
    void onRegisterSkill(int64 skillUID, int skill_id, int prev_level, int skill_level) override;
    void onItemWearEffect(Item* pItem, bool bIsBaseVar, int type, float var1, float var2, float fRatio) override;
    void onExpChange() override;
    void onJobLevelUp() override;
    void onCantAttack(uint target, uint t) override;
    void onModifyStatAndAttribute() override;
    void applyPassiveSkillEffect(Skill* skill) override;
    void applyState(State& state) override;

    void onStartQuest(Quest* pQuest);
    void updateQuestStatus(Quest* pQuest);
    void onEndQuest(Quest* pQuest);

private:
    void applyCharm(Item* pItem);
    void setBonusMsg(BONUS_TYPE type, int nBonusPerc, int64 nBonusEXP, int nBonusJP);
    void clearPendingBonusMsg();
    void sendBonusEXPJPMsg();

    WorldSession *m_session{nullptr};
    std::string  m_szAccount;
    QuestManager m_QuestManager{};
    uint m_nLastStaminaUpdateTime{0};
    bool m_bStaminaActive{false};
    bool m_bUsingTent{false};
    float m_fDistEXPMod{1.0f};
    float m_fActiveSummonExpAmp{0.0f};
    float m_fDeactiveSummonExpAmp{0.0f};

    uint m_nItemCooltime[MAX_ITEM_COOLTIME_GROUP]{0};

    UNORDERED_MAP<std::string, std::string> m_lFlagList{ };
    UNORDERED_MAP<std::string, std::string> m_hsContact{ };

    std::vector<Summon *> m_vSummonList{nullptr};

    uint m_nLastSaveTime{ }, m_nLastCantAttackTime{ };

    BonusInfo m_pBonusInfo[BONUS_TYPE::MAX_BONUS_TYPE]{};

    // Dialog stuff
    int         m_nDialogType{ };
    bool        m_bNonNPCDialog{false};
    std::string m_szDialogTitle{ };
    std::string m_szDialogText{ };
    std::string m_szDialogMenu{ };
    std::string m_szSpecialDialogMenu{ };
    std::string m_szClientInfo{ };
};

#endif // _PLAYER_H_
