#include <hive/hive.h>
#include <hive/webserver/ssl.h>
#include <hive/webserver/webserver.h>

#include "task_clear_old_sessions.h"

CTaskClearOldSessions::CTaskClearOldSessions(CHive *pHive, CTaskScheduler *pTaskScheduler) : CTask(pHive, pTaskScheduler)
{
    pTaskScheduler->AddTask(CTaskContainer::CONTAINER_SECOND, this);
}

void CTaskClearOldSessions::Fire()
{
    m_pHive->Webserver()->ClearOldSessions();
}