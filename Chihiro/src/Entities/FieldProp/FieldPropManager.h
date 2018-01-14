#ifndef PROJECT_FIELDPROPMANAGER_H
#define PROJECT_FIELDPROPMANAGER_H

#include "FieldProp.h"
#include "Common.h"
#include "SharedMutex.h"

struct FieldPropRegenInfo {
    FieldPropRegenInfo(uint t, uint lt)
    {
        tNextRegen = t;
        nLifeTime = lt;
        pRespawnInfo = {};
    }
    FieldPropRespawnInfo pRespawnInfo;
    uint tNextRegen;
    uint nLifeTime;
};

class FieldPropManager : public FieldPropDeleteHandler
{
public:
    void SpawnFieldPropFromScript(FieldPropRespawnInfo prop, int lifeTime);
    void RegisterFieldProp(FieldPropRespawnInfo prop);
    void onFieldPropDelete(FieldProp* prop) override;
    void Update(uint diff);
private:
    std::vector<FieldPropRespawnInfo> m_vRespawnInfo{};
    std::vector<FieldPropRegenInfo> m_vRespawnList{};
    std::vector<FieldProp*> m_vExpireObject{};
    MX_SHARED_MUTEX i_lock;
};
#define sFieldPropManager ACE_Singleton<FieldPropManager, ACE_Null_Mutex>::instance()
#endif // PROJECT_FIELDPROPMANAGER_H
