#ifndef PROJECT_DECLARATIONS_H
#define PROJECT_DECLARATIONS_H

#include "WorldSocketMgr.h"
#include "GameAuthSession.h"
#include "WorldSession.h"


#define sWorldSocketMgr ACE_Singleton<WorldSocketMgr<WorldSession>, ACE_Thread_Mutex>::instance()

#endif // PROJECT_DECLARATIONS_H
