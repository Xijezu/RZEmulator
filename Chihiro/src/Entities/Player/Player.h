#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "Common.h"
#include "Unit.h"
#include "TimeSync.h"
#include "QuestManager.h"
#include "Inventory.h"
#include "XPacket.h"

class WorldSession;
class Item;
class Summon;
class WorldLocation;

#define MAX_ITEM_COOLTIME_GROUP 20

enum BONUS_TYPE : int
{
    BONUS_PCBANG         = 0x0,
    BONUS_STAMINA        = 0x1,
    BONUS_PREMIUM_PCBANG = 0x2,
    MAX_BONUS_TYPE       = 0x3,
};

enum TRADE_MODE : int
{
    TM_REQUEST_TRADE = 0,
    TM_ACCEPT_TRADE,
    TM_BEGIN_TRADE,
    TM_CANCEL_TRADE,
    TM_REJECT_TRADE,
    TM_ADD_ITEM,
    TM_ADD_GOLD,
    TM_FREEZE_TRADE,
    TM_CONFIRM_TRADE,
    TM_PROCESS_TRADE,
    TM_REMOVE_ITEM,
    TM_MODIFY_COUNT
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

class Player : public Unit, public QuestEventHandler, public InventoryEventReceiver
{
    public:
        friend class Messages;
        friend class Summon;
        friend class WorldSession;

        explicit Player(uint32);
        ~Player() override;
        void CleanupsBeforeDelete();

        /* ****************** STATIC FUNCTIONS ****************** */
        static void EnterPacket(XPacket &, Player *, Player *);
        static void DoEachPlayer(const std::function<void(Player *)> &fn);
        static Player *FindPlayer(const std::string &szName);
        static void DB_ItemCoolTime(Player *);
        /* ****************** STATIC END ****************** */

        /* ****************** QUEST ****************** */
        void DoEachActiveQuest(const std::function<void(Quest *)> &fn) { m_QuestManager.DoEachActiveQuest(fn); }

        void UpdateQuestStatusByMonsterKill(int monster_id);
        void GetQuestByMonster(int monster_id, std::vector<Quest *> &vQuest, int type);
        void StartQuest(int code, int nStartQuestID, bool bForce);
        void EndQuest(int code, int nRewardID, bool bForce);
        void UpdateQuestStatusByItemUpgrade();
        void onStatusChanged(Quest *quest, int nOldStatus, int nNewStatus) override;
        void onProgressChanged(Quest *quest, QuestProgress oldProgress, QuestProgress newProgress) override;
        int GetQuestProgress(int nQuestID);

        int GetActiveQuestCount() const { return (int)m_QuestManager.m_vActiveQuest.size(); }

        bool IsTakeableQuestItem(int code) { return m_QuestManager.IsTakeableQuestItem(code); }

        bool CheckFinishableQuestAndGetQuestStruct(int code, Quest *&pQuest, bool bForce);
        bool IsInProgressQuest(int code);
        bool IsStartableQuest(int code, bool bForQuestMark);
        bool IsFinishableQuest(int code);
        bool CheckFinishableQuestAndGetQuestStruct(int code);
        Quest *FindQuest(int code);

        /* ****************** QUEST END ****************** */

        int GetPermission() const { return GetInt32Value(PLAYER_FIELD_PERMISSION); }
        int64 GetGold() const { return GetUInt64Value(PLAYER_FIELD_GOLD); }
        int64 GetStorageGold() const { return GetUInt64Value(PLAYER_FIELD_STORAGE_GOLD); }
        int GetPartyID() const { return GetInt32Value(PLAYER_FIELD_PARTY_ID); }
        int GetGuildID() const { return GetInt32Value(PLAYER_FIELD_GUILD_ID); }
        int AddStamina(int nStamina);
        int GetStaminaRegenRate();
        CONDITION_INFO GetCondition() const;
        bool IsSitdownable() const;
        bool IsSitdown() const override { return m_bSitdown; }

        uint GetJobDepth();

        std::string GetCharacterFlag(const std::string &flag) { return m_lFlagList[flag]; }

        /* ****************** DATABASE ****************** */
        bool ReadCharacter(const std::string&, int);
        bool ReadItemList(int);
        bool ReadItemCoolTimeList(int);
        bool ReadSummonList(int);
        bool ReadEquipItem();
        bool ReadSkillList(Unit *);
        bool ReadQuestList();
        bool ReadStateList(Unit *);
        bool ReadStorageSummonList(std::vector<Summon*>&);
        void DB_ReadStorage();
        void DB_UpdateStorageGold();
        /* ****************** DATABASE END ****************** */

        // Warping
        void PendWarp(int x, int y, uint8_t layer);

        void SetCharacterFlag(const std::string &key, const std::string& value);

        // Summon
        void LogoutNow(int callerIdx);
        void RemoveAllSummonFromWorld();

        void SendLoginProperties();
        void SendGoldChaosMessage();
        void SendJobInfo();
        void SendWearInfo();

        Summon *GetSummon(int);
        Summon *GetSummonByHandle(uint);
        void AddSummon(Summon *, bool);

        // Database
        void Update(uint diff) override;
        void OnUpdate() override;
        void Save(bool);

        // Item relevant
        Item *FindItemByCode(int);
        Item *FindItemBySID(int64);
        Item *FindItemByHandle(uint);
        Item *FindItem(uint code, uint flag, bool bFlag);
        Item *FindStorageItem(int code);
        /* InventoryEventReceiver Start */
        void onAdd(Inventory* pInventory, Item* pItem, bool bSkipUpdateItemToDB) override;
        void onRemove(Inventory* pInventory, Item* pItem, bool bSkipUpdateItemToDB) override;
        void onChangeCount(Inventory* pInventory, Item* pItem, bool bSkipUpdateItemToDB) override;
        /* InventoryEventReceiver End */
        Item *PushItem(Item *, int64, bool);
        Item *PopItem(Item *, int64, bool);
        Item *GetItem(uint idx);
        Item *GetStorageItem(uint idx);
        uint GetItemCount() const;
        uint GetStorageItemCount() const;
        void MoveStorageToInventory(Item *pItem, int64 count);
        void MoveInventoryToStorage(Item *pItem, int64 count);
        bool EraseItem(Item* pItem, int64 count);
        bool EraseBullet(int64 count);

        // Storage Relevant
        bool RemoveSummon(Summon* pSummon);
        void AddSummonToStorage(Summon* pSummon);
        void RemoveSummonFromStorage(Summon* pSummon);
        void OpenStorage();

        void StartTrade(Player *pTarget);
        void CancelTrade(bool bIsNeedBroadcast);
        void ClearTradeInfo();
        Player *GetTradeTarget();

        void AddEXP(int64 exp, uint jp, bool bApplyStanima) override;
        uint16_t putonItem(ItemWearType, Item *) override;
        uint16_t putoffItem(ItemWearType) override;
        void SendItemWearInfoMessage(Item *item, Unit *u);
        void ChangeLocation(float x, float y, bool bByRequest, bool bBroadcast);

        bool IsAlly(const Unit *pUnit) override;

        /* ****************** DIALOG ****************** */
        void SetLastContact(const std::string&, uint32_t);
        void SetLastContact(const std::string&, const std::string&);
        std::string GetLastContactStr(const std::string&);
        uint32_t GetLastContactLong(const std::string&);
        void SetDialogTitle(const std::string&, int);
        void SetDialogText(const std::string&);
        void AddDialogMenu(const std::string&, const std::string&);
        void ClearDialogMenu();

        bool IsFixedDialogTrigger(const std::string &szTrigger) { return false; }

        bool HasDialog() { return !m_szDialogTitle.empty(); }

        void ShowDialog();
        bool IsValidTrigger(const std::string &);
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

        void DoSummon(Summon *pSummon, Position pPosition);
        void DoUnSummon(Summon *pSummon);

        bool IsHunter();
        bool IsFighter();
        bool IsMagician();
        bool IsSummoner();
        bool IsUsingBow() const override;
        bool IsUsingCrossBow() const override;

        bool IsPlayer() const override { return true; }

        float GetMoveSpeed() const;

        bool TranslateWearPosition(ItemWearType &pos, Item *item, std::vector<int> &ItemList) override;

        CreatureStat *GetBaseStat() const override;

        uint GetCreatureGroup() const override { return 9; }

        ushort ChangeGold(int64);
        ushort ChangeStorageGold(int64);

        uint16 IsUseableItem(Item *pItem, Unit *pTarget);
        uint16 UseItem(Item *pItem, Unit *pTarget, const std::string &szParameter);

        void SendPacket(XPacket pPacket);
        WorldSession &GetSession() const { return *m_session; }
        void SetClientInfo(const std::string &value) { m_szClientInfo = value; }
        void SetSession(WorldSession *session) { m_session = session; }

        void applyJobLevelBonus() override;

        Item          *m_aBindSummonCard[6]{nullptr};
        WorldLocation *m_WorldLocation{nullptr};
        int           m_nWorldLocationId{0};
        TimeSynch     m_TS{200, 2, 10};

        Summon *m_pMainSummon{nullptr};
        uint m_hTamingTarget{ };
        int GetChaos() const;
        int GetMaxChaos() const;
        void AddChaos(int chaos);
        bool m_bSitdown{false};
    protected:

        bool isInLocationType(uint8 nLocationType);
        void onCompleteCalculateStat() override;
        void onBeforeCalculateStat() override;
        void onRegisterSkill(int64 skillUID, int skill_id, int prev_level, int skill_level) override;
        void onItemWearEffect(Item *pItem, bool bIsBaseVar, int type, float var1, float var2, float fRatio) override;
        void onExpChange() override;
        void onJobLevelUp() override;
        void onCantAttack(uint target, uint t) override;
        void onModifyStatAndAttribute() override;
        void applyPassiveSkillEffect(Skill *skill) override;
        void applyState(State &state) override;

        void onStartQuest(Quest *pQuest);
        void updateQuestStatus(Quest *pQuest);
        void onEndQuest(Quest *pQuest);
    private:
        void openStorage();
        Item *popItem(Item* pItem, int64 cnt, bool bSkipUpdateToDB);

        void applyCharm(Item *pItem);
        void setBonusMsg(BONUS_TYPE type, int nBonusPerc, int64 nBonusEXP, int nBonusJP);
        void clearPendingBonusMsg();
        void sendBonusEXPJPMsg();

        WorldSession *m_session{nullptr};
        std::string  m_szAccount;
        QuestManager m_QuestManager{ };
        bool         m_bStaminaActive{false};
        bool         m_bUsingTent{false};
        float        m_fDistEXPMod{1.0f};
        float        m_fActiveSummonExpAmp{0.0f};
        float        m_fDeactiveSummonExpAmp{0.0f};

        uint m_nItemCooltime[MAX_ITEM_COOLTIME_GROUP]{0};

        UNORDERED_MAP<std::string, std::string> m_lFlagList{ };
        UNORDERED_MAP<std::string, std::string> m_hsContact{ };

        std::vector<Skill *> m_vApplySummonPassive;
        std::vector<Skill *> m_vApmlifySummonPassive;

        std::vector<Summon *> m_vSummonList{};
        std::vector<Summon *> m_vStorageSummonList{};
        std::vector<Item*> m_vCharmList{};

        BonusInfo m_pBonusInfo[BONUS_TYPE::MAX_BONUS_TYPE]{ };

        Inventory m_Inventory;
        Inventory m_Storage;
        bool m_bIsStorageRequested{false};
        bool m_bIsStorageLoaded{false};
        bool m_bIsUsingStorage{false};

        // Dialog stuff
        int         m_nDialogType{ };
        bool        m_bNonNPCDialog{false};
        std::string m_szDialogTitle{ };
        std::string m_szDialogText{ };
        std::string m_szDialogMenu{ };
        std::string m_szSpecialDialogMenu{ };
        std::string m_szClientInfo{ };

        // Trade stuff
        bool   m_bTrading      = false;
        bool   m_bTradeFreezed = false;
        Player *m_pTradeTarget;
};

#endif // _PLAYER_H_
