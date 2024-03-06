#pragma once

#include "../task_scheduler.h"

class CTaskClearOldSessions : public CTask
{
private:

public:
    CTaskClearOldSessions(CHive *pHive, CTaskScheduler *pTaskScheduler);

    virtual void Fire();
};