#pragma once

#include "../task_scheduler.h"

class CTaskBackupLocal : public CTask
{
private:
    bool WaitForCompressor(int Status);
    static void DoBackup(void *pUser);

public:
    CTaskBackupLocal(CHive *pHive, CTaskScheduler *pTaskScheduler);

    virtual void Fire();
};