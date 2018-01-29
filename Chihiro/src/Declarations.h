#ifndef PROJECT_DECLARATIONS_H
#define PROJECT_DECLARATIONS_H

#include "WorldSocketMgr.h"
#include "GameAuthSession.h"
#include "WorldSession.h"

template class WorldSocket<GameAuthSession>;
template class WorldSocketMgr<GameAuthSession>;
template class WorldSocket<WorldSession>;
template class WorldSocketMgr<WorldSession>;
#define sWorldSocketMgr ACE_Singleton<WorldSocketMgr<WorldSession>, ACE_Thread_Mutex>::instance()

#endif // PROJECT_DECLARATIONS_H
