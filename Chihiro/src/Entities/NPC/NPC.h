#ifndef _NPC_H_
#define _NPC_H_

#include "Entities/Unit/Unit.h"
#include "Network/GameNetwork/WorldSession.h"

class NPC : public Unit {
public:
    NPC();
    static void EnterPacket(XPacket& pEnterPct, NPC* pNPC);

    NPCTemplate m_pBase;
private:
};

#endif // _NPC_H_
