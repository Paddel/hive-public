#include <ctime>
#include <cstdlib>

#include <core/wrapper.h>

#include "task_meertrubel.h"

CTaskMeertrubel::CTaskMeertrubel(CHive *pHive, CTaskScheduler *pTaskScheduler) : CTask(pHive, pTaskScheduler)
{
    pTaskScheduler->AddTask(CTaskContainer::CONTAINER_MINUTE, this);
    CalcInsertTime();
}

float CTaskMeertrubel::HourFloat()
{
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);
    return now->tm_hour + now->tm_min / 60.0f;
}

void CTaskMeertrubel::CalcInsertTime()
{
    m_InsertTime = 19.0f + (36 + randInt() % 22 - 11) / 100.0f;
}

void CTaskMeertrubel::Fire()
{
    float Now = HourFloat();

    if(m_Prepared == false && Now > m_InsertTime - 0.5f && Now < m_InsertTime)//prepare
            m_Prepared = true;
    else if(m_Prepared == true && Now > m_InsertTime && Now < m_InsertTime + 0.5f)
    {
#ifdef CONF_FAMILY_UNIX
	if (system("python3 extern/meertrubel/mt-run.py") != 0)
		print("Calling Meertrubel!");
#endif
        CalcInsertTime();
        m_Prepared = false;
    }
    else if(m_Prepared == true && (Now > m_InsertTime + 0.5f || Now < m_InsertTime - 0.5f))
        m_Prepared = false;//cancel
}