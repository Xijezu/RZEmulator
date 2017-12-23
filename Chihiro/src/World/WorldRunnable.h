#ifndef PROJECT_WORLDRUNNABLE_H
#define PROJECT_WORLDRUNNABLE_H

#include "Common.h"

class WorldRunnable : public ACE_Based::Runnable {
public:
    void run() OVERRIDE;
};


#endif // PROJECT_WORLDRUNNABLE_H
