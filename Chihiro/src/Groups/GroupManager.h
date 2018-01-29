#ifndef PROJECT_GROUPMANAGER_H
#define PROJECT_GROUPMANAGER_H

#include "Common.h"
#include "SharedMutex.h"

enum PARTY_TYPE : int
{
    TYPE_UNKNOWN                    = 0xFF,
    TYPE_NORMAL_PARTY               = 0,
    TYPE_RAID_ATTACKTEAM            = 1,
    TYPE_SIEGE_ATTACKTEAM           = 2,
    /*TYPE_HUNTAHOLIC_PARTY           = 3,
    TYPE_BATTLE_ARENA_TEAM          = 4,
    TYPE_BATTLE_ARENA_EXERCISE_TEAM = 5*/
};

enum ITEM_SHARE_MODE : int
{
    ITEM_SHARE_MONOPOLY = 0,
    ITEM_SHARE_RANDOM   = 1,
    ITEM_SHARE_LINEAR   = 2
};

class Player;
struct PartyMemberTag
{
    bool bIsOnline;
    int sid;
    std::string strName;
    Player *pPlayer;
    int nLevel;
    int nJobID;
};

struct PartyInfo
{
    uint                        nPartyPassword;
    int                         nPartyID;
    int                         nLeaderSID;
    ITEM_SHARE_MODE             eShareMode;
    PARTY_TYPE                  ePartyType;
    std::string                 strPartyName;
    std::string                 strLeaderName;
    int                         nLeaderJobID;
    std::vector<PartyMemberTag> vMemberNameList;
    std::vector<uint>           vOnlineList;
    int                         nLastItemAcquirerIdx;
    // ATtackTeamInfo
};

class Player;
class GroupManager
{
    public:
        GroupManager() = default;
        ~GroupManager() = default;

        void InitGroupSystem();

        ///- Getters
        int GetAttackTeamLeadPartyID(int nPartyID);
        int GetAttackTeamGuildID(int nPartyID);
        int GetMemberCount(int nPartyID);
        int GetMinLevel(int nPartyID);
        int GetMaxLevel(int nPartyID);
        int GetShareMode(int nPartyID);
        uint GetPassword(int nPartyID);
        std::string GetPartyName(int nPartyID);
        PARTY_TYPE GetPartyType(int nPartyID);
        std::string GetLeaderName(int nPartyID);
        void GetNearMember(Player *pPlayer, float distance, std::vector<Player *> &vList);

        ///- booleans
        bool IsLeader(int nPartyID, const std::string &szPlayerName);
        bool IsAttackTeamParty(int nPartyID);

        ///- Functionality
        bool JoinParty(int nPartyID, Player *pPlayer, uint nPass);
        bool DestroyParty(int nPartyID);
        bool LeaveParty(int nPartyID, const std::string &szName);
        int CreateParty(Player *pPlayer, const std::string &szName, PARTY_TYPE partyType);
        void DoEachMemberTag(int nPartyID, std::function<void(PartyMemberTag &)> fn);

        ///- Events
        void OnChangeCharacterLevel(int nPartyID, const std::string &szName, int nLevel);
        void OnChangeCharacterJob(int nPartyID, const std::string &szName, int nJobID);
        bool onLogin(int nPartyID, Player *pPlayer);
        bool onLogout(int nPartyID, Player *pPlayer);
    protected:
        void AddGroupToDatabase(const PartyInfo &);
        void LoadPartyInfo(PartyInfo &info);
        PartyInfo *getPartyInfo(int nPartyID);
        PartyInfo *getPartyInfoNC(int nPartyID);
    private:
        uint64                   m_nMaxPartyID{0};
        std::map<int, PartyInfo> m_hshPartyID{ };
        MX_SHARED_MUTEX i_lock;
};

#define sGroupManager ACE_Singleton<GroupManager, ACE_Thread_Mutex>::instance()
#endif // PROJECT_GROUPMANAGER_H
