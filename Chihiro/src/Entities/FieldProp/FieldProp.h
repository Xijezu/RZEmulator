#include "Common.h"
#include "FieldPropBase.h"
#include "Unit.h"

class FieldProp;
struct FieldPropDeleteHandler
{
	virtual void onFieldPropDelete(FieldProp* prop) = 0;
};

class Player;
class XPacket;

class FieldProp : public Unit
{
    friend class Skill;
	friend class FieldPropManager;
public:
	FieldProp() = delete;
	static void EnterPacket(XPacket &pEnterPct, FieldProp *pFieldProp, Player *pPlayer);
	static FieldProp* Create(FieldPropDeleteHandler* propDeleteHandler, FieldPropRespawnInfo pPropInfo, uint lifeTime);
	bool IsUsable(Player*) const;
	bool Cast();
	bool UseProp(Player*);
	uint GetCastingDelay() const;

private:
	FieldProp(FieldPropDeleteHandler* propDeleteHandler, FieldPropRespawnInfo pPropInfo);

	uint m_nRegenTime;
	FieldPropDeleteHandler* m_pDeleteHandler;
	FieldPropTemplate* m_pFieldPropBase;
	FieldPropRespawnInfo m_PropInfo;
	int m_nUseCount;
	bool m_bIsCasting;
	uint nLifeTime;
};

