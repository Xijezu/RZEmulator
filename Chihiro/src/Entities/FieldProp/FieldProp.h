#include "Common.h"
#include "FieldPropBase.h"
#include "Object.h"

class FieldProp;
struct FieldPropDeleteHandler
{
	virtual void onFieldPropDelete(FieldProp* prop) = 0;
};

class Player;
class FieldProp : public WorldObject
{
	friend class FieldPropManager;
public:
	FieldProp() = delete;
	static FieldProp* Create(FieldPropDeleteHandler* propDeleteHandler, FieldPropRespawnInfo pPropInfo, uint lifeTime);
	bool IsUsable(Player*) const;
	bool Cast() const;
	bool UseProp(Player*) const;
	uint GetCastingDelay() const;

private:
	FieldProp(FieldPropDeleteHandler* propDeleteHandler, FieldPropRespawnInfo pPropInfo);
	~FieldProp() = default;

	uint m_nRegenTime;
	FieldPropDeleteHandler* m_pDeleteHandler;
	FieldPropTemplate* m_pFieldPropBase;
	FieldPropRespawnInfo m_PropInfo;
	int m_nUseCount;
	bool m_bIsCasting;
	uint nLifeTime;
};

