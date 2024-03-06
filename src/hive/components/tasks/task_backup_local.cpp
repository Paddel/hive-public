#include <hive/components/compressor.h>
#include <hive/components/event_logger.h>
#include <hive/hive.h>

#include "task_backup_local.h"

CTaskBackupLocal::CTaskBackupLocal(CHive *pHive, CTaskScheduler *pTaskScheduler) : CTask(pHive, pTaskScheduler)
{
    pTaskScheduler->AddTask(CTaskContainer::CONTAINER_DAY, this);
}

bool CTaskBackupLocal::WaitForCompressor(int Status)
{
    CCompressor *pCompressor = m_pHive->Compressor();

    for(int i = 0; i < 30 && pCompressor->GetStatus() != Status; i++)
        thread_sleep(100);

    if(pCompressor->GetStatus() != Status)
    {
        m_pHive->EventLogger()->AddEntryAlert("BACKUP", "waiting timed out status=%i wanted_status=%i", pCompressor->GetStatus(), Status);
        return false;
    }

    return true;
}

void CTaskBackupLocal::DoBackup(void *pUser)
{
    CTaskBackupLocal *pThis = (CTaskBackupLocal *)pUser;
    CCompressor *pCompressor = pThis->m_pHive->Compressor();
    bool Result = false;

    print("backup local start");

    /*Result = pCompressor->AddBundle(DEVICE_CLOUD, "/home/hive/secured");
    if(pThis->WaitForCompressor(CCompressor::STATUS_COLLECTING_DONE) == false)
        return pCompressor->Reset();*/
    
    Result = pCompressor->AddBundleFile("_pass", "/home/hive/secured", DEVICE_CLOUD);
    if(pThis->WaitForCompressor(CCompressor::STATUS_COLLECTING_DONE) == false)
        return pCompressor->Reset();

    Result = pCompressor->AddBundle(DEVICE_CLOUD, "/home/hive/statistics");
    if(pThis->WaitForCompressor(CCompressor::STATUS_COLLECTING_DONE) == false)
        return pCompressor->Reset();

    pCompressor->SetArchiveName("backup");
    pCompressor->Start();
    if(pThis->WaitForCompressor(CCompressor::STATUS_DONE) == false)
        return pCompressor->Reset();

    if(pCompressor->CountErrors() > 0)
        pThis->m_pHive->EventLogger()->AddEntryAlert("BACKUP", "backup failed. %i errors", pCompressor->CountErrors());

    pCompressor->Finish();
    print("backup local done");
}

void CTaskBackupLocal::Fire()
{
    //if(m_pHive->Settings()->GetValue(CSettings::SETTINGS_AUTOMISATION, "lights_off", true) == true)
    CCompressor *pCompressor = m_pHive->Compressor();
    if(pCompressor->GetStatus() == CCompressor::STATUS_DONE)
        pCompressor->Finish();
    else if(pCompressor->GetStatus() != CCompressor::STATUS_INACTIVE)
    {
        m_pHive->EventLogger()->AddEntryAlert("BACKUP", "local backup failed. compressor not inactive. status=%i", pCompressor->GetStatus());
        return;
    }

    thread_create(DoBackup, this);
}