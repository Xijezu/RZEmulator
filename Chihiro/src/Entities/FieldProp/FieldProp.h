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

class FieldProp : public WorldObject
{
		friend class Skill;
		friend class FieldPropManager;
	public:
		FieldProp() = delete;
		/// \brief Used to generate the TS_SC_ENTER packet for a FieldProp
		/// \param pEnterPct Writable object
		/// \param pFieldProp The object we're generating for
		/// \param pPlayer the player who receives the packet
		static void EnterPacket(XPacket &pEnterPct, FieldProp *pFieldProp, Player *pPlayer);
		/// \brief Creates and spawns a fieldprop
		/// \param propDeleteHandler Always the instance of FieldPropManager, is used to delete it from its list
		/// \param pPropInfo RespawnInfo
		/// \param lifeTime how long the prop is on the map
		/// \return Newly created Fieldprop on success, nullptr on failure
		static FieldProp *Create(FieldPropDeleteHandler *propDeleteHandler, FieldPropRespawnInfo pPropInfo, uint lifeTime);
		bool IsUsable(Player *) const;
		bool Cast();
		bool UseProp(Player *);
		uint GetCastingDelay() const;

		bool IsFieldProp() const override { return true; }

	private:
		FieldProp(FieldPropDeleteHandler *propDeleteHandler, FieldPropRespawnInfo pPropInfo);

		uint m_nRegenTime;
		FieldPropDeleteHandler *m_pDeleteHandler;
		FieldPropTemplate      *m_pFieldPropBase;
		FieldPropRespawnInfo m_PropInfo;
		int                  m_nUseCount;
		bool                 m_bIsCasting;
		uint                 nLifeTime;
};

