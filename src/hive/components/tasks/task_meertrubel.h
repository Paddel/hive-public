#pragma once

#include "../task_scheduler.h"

class CTaskMeertrubel : public CTask
{
private:
    bool m_Prepared;
    float m_InsertTime;
    float HourFloat();
    void CalcInsertTime();

public:
    CTaskMeertrubel(CHive *pHive, CTaskScheduler *pTaskScheduler);

    virtual void Fire();
};