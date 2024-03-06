
#ifndef CONF_DUMMY
#include <archive.h>
#include <archive_entry.h>
#endif

#include <core/protocol.h>
#include <core/tcp_unpacker.h>
#include <core/wrapper.h>
#include <hive/components/request_manager.h>
#include <hive/hive.h>
#include <hive/webserver/webserver.h>

#include "compressor.h"

CCompressor::CCompressor(CHive *pHive)
{
	m_pHive = pHive;

	m_Status = STATUS_INACTIVE;
	m_aArchiveName[0] = '\0';

	m_FilesLock = lock_create();
	m_AddBundleLock = lock_create();
}

void CCompressor::Compress(void *pUser)
{
	CCompressor *pThis = (CCompressor *)pUser;
	CRequestManager *pRequestManager = pThis->m_pHive->RequestManager();

	for (int Bundle = 0; Bundle < pThis->m_lBundles.Size(); Bundle++)
	{
		CCompressBundle *pBundle = &pThis->m_lBundles[Bundle];
		for (int i = 0; i < pBundle->m_lFiles.Size() && pThis->m_Status == STATUS_COMPRESSING; i++)
		{
			CCompressFile *pFile = &pBundle->m_lFiles[i];

			if (pFile->m_Status != FILESTATUS_UNCOMPRESSED)
				continue;

			char aName[256];
			char aPath[1024];
			str_format(aPath, sizeof(aPath), "%s/%s", pBundle->m_aRootPath, pFile->m_aName);
			str_copy(aName, aPath, sizeof(aName));
			if (fs_parent_dir(aPath) == 0)
			{
				str_copy(aName, aName + str_length(aPath) + 1, sizeof(aName));


				CRequestTCP *pRequestTCP = pRequestManager->NewRequestTCP(pBundle->m_Device);
				CMsg Msg(MSG_FILE, pBundle->m_Device);
				Msg.AddInteger(pRequestTCP->GetRequestID());
				Msg.AddString(aPath);
				Msg.AddString(aName);
				pThis->m_pHive->SendMsg(&Msg);

				pRequestTCP->WaitForResponse();

				if (pRequestTCP->GetError() == false)
					pThis->FileStream(pRequestTCP->m_pUnpacker, pFile);
				else
					pThis->OnFileError(pFile, pRequestTCP->ErrorMessage());

				net_tcp_close_socket(pRequestTCP->m_TCPSocket);
				pRequestManager->RemoveRequest(pRequestTCP);
			}
		}
	}

	if (pThis->m_Status == STATUS_COMPRESSING)
		pThis->m_Status = STATUS_DONE;
}

void CCompressor::OnFileError(CCompressFile *pFile, const char *pErrorMessage)
{
	if (pFile->m_Status != FILESTATUS_UNCOMPRESSED)
		return;

	pFile->m_Status = FILESTATUS_ERROR;
	str_copy(pFile->m_aErrorMessage, pErrorMessage, sizeof(pFile->m_aErrorMessage));
}

int CCompressor::NewBundle(const char *pName, const char *pRootPath, char Device)
{
	if (m_Status != STATUS_INACTIVE && m_Status != STATUS_COLLECTING_DONE)
		return -1;

	if (m_Status == STATUS_INACTIVE)
		str_copy(m_aArchiveName, pName, sizeof(m_aArchiveName));

	lock_wait(m_FilesLock);

	CCompressBundle Bundle;
	str_copy(Bundle.m_aRootPath, pRootPath, sizeof(Bundle.m_aRootPath));
	str_copy(Bundle.m_aName, pName, sizeof(Bundle.m_aName));
	Bundle.m_Device = Device;
	m_lBundles += Bundle;

	m_Status = STATUS_COLLECTING;

	lock_unlock(m_FilesLock);

	return m_lBundles.Size() - 1;
}


void CCompressor::AddFile(int BundleID, const char *pFile)
{
	lock_wait(m_FilesLock);

	if (m_Status == STATUS_COLLECTING && BundleID >= 0 && BundleID < m_lBundles.Size())
	{
		CCompressFile File;
		str_copy(File.m_aName, pFile, sizeof(File.m_aName));
		File.m_Status = FILESTATUS_UNCOMPRESSED;
		m_lBundles[BundleID].m_lFiles += File;
	}

	lock_unlock(m_FilesLock);
}


void CCompressor::Start()
{
	if (m_Status != STATUS_COLLECTING_DONE)
		return;

	char aPath[1024];
	str_format(aPath, sizeof(aPath), "/home/hive/compressed/%s.zip", m_aArchiveName);
#ifndef CONF_DUMMY
	m_Archive = archive_write_new();
	archive_write_set_format_zip(m_Archive);
	archive_write_open_filename(m_Archive, aPath);
#endif

	TryCompress();
}

void CCompressor::TryCompress()
{
	if (m_Status != STATUS_COLLECTING_DONE && m_Status != STATUS_DONE)
		return;

	lock_wait(m_FilesLock);

	//reset error files
	for (int Bundle = 0; Bundle < m_lBundles.Size(); Bundle++)
		for (int i = 0; i < m_lBundles[Bundle].m_lFiles.Size(); i++)
			if (m_lBundles[Bundle].m_lFiles[i].m_Status == FILESTATUS_ERROR)
				m_lBundles[Bundle].m_lFiles[i].m_Status = FILESTATUS_UNCOMPRESSED;

	lock_unlock(m_FilesLock);

	m_Status = STATUS_COMPRESSING;
	thread_create(Compress, this);
}

void CCompressor::Finish()
{
	if (m_Status != STATUS_DONE)
		return;

#ifndef CONF_DUMMY
	archive_write_close(m_Archive);
	archive_write_free(m_Archive);
#endif

	Reset();
}

void CCompressor::RemoveItem(int Bundle, int File)
{
	if (m_Status != STATUS_COLLECTING && m_Status != STATUS_COLLECTING_DONE)
		return;

	if (Bundle < 0 || Bundle >= m_lBundles.Size())
		return;

	lock_wait(m_FilesLock);

	if (File == -1 || (File == 0 && m_lBundles[Bundle].m_lFiles.Size() == 1))
		m_lBundles.Remove(Bundle);
	else if (File >= 0 && File < m_lBundles[Bundle].m_lFiles.Size())
		m_lBundles[Bundle].m_lFiles.Remove(File);

	lock_unlock(m_FilesLock);

	if (m_lBundles.Size() == 0)
		Reset();
}

int CCompressor::NumBundles()
{
	return m_lBundles.Size();
}

int CCompressor::NumFiles(int Bundle)
{
	if (Bundle < 0 || Bundle >= m_lBundles.Size())
		return 0;
	return m_lBundles[Bundle].m_lFiles.Size();
}

bool CCompressor::AddBundleFile(const char *pName, const char *pPath, char Device)
{
	if (m_Status != STATUS_INACTIVE && m_Status != STATUS_COLLECTING_DONE)
		return false;

	char aName[1024];
	str_format(aName, sizeof(aName), "/%s", pName);

	int BundleID = m_pHive->Compressor()->NewBundle(pName, pPath, Device);
	if (BundleID != -1)
		m_pHive->Compressor()->AddFile(BundleID, aName);

	if (m_Status == STATUS_COLLECTING)
		m_Status = STATUS_COLLECTING_DONE;

	return true;
}

bool CCompressor::AddBundle(char Device, const char *pPath)
{
	if (m_Status != STATUS_INACTIVE && m_Status != STATUS_COLLECTING_DONE)
		return false;

	if(lock_trylock(m_AddBundleLock) != 0)
		return false;

	CRequestTCP *pRequestTCP = m_pHive->RequestManager()->NewRequestTCP(Device);
	CMsg Msg(MSG_COMPRESS_DIRECTORY, Device);
	Msg.AddInteger(pRequestTCP->GetRequestID());
	Msg.AddString(pPath);
	m_pHive->SendMsg(&Msg);

	pRequestTCP->WaitForResponse();

	bool Result = false;
	if (pRequestTCP->GetError() == false)
	{
		FileTreeStream(pRequestTCP->m_pUnpacker, Device);
		Result = true;
	}

	net_tcp_close_socket(pRequestTCP->m_TCPSocket);
	m_pHive->RequestManager()->RemoveRequest(pRequestTCP);

	m_Status = STATUS_COLLECTING_DONE;

	lock_unlock(m_AddBundleLock);

	return Result;
}

void CCompressor::StartReadingFiles()
{
	lock_wait(m_FilesLock);
}

CCompressor::CCompressBundle *CCompressor::GetBundle(int Bundle)
{
	if (Bundle < 0 || Bundle >= m_lBundles.Size())
		return 0x0;
	return &m_lBundles[Bundle];
}

void CCompressor::StopReadingFiles()
{
	lock_unlock(m_FilesLock);
}

void CCompressor::FileStream(CTCPUnpacker *pUnpacker, CCompressFile *pFile)
{
	long long FileSize = pUnpacker->GetLongLong();
	const char *pName = pUnpacker->GetString();
#ifndef CONF_DUMMY
	archive_entry *pEntry = archive_entry_new();
	archive_entry_set_pathname(pEntry, pFile->m_aName + 1);//remove /
	archive_entry_set_size(pEntry, FileSize);
	archive_entry_set_filetype(pEntry, AE_IFREG);
	archive_entry_set_perm(pEntry, 0644);
	archive_write_header(m_Archive, pEntry);
#endif
		
	unsigned int ReceivedSize = 0;

	while (true)
	{
		pUnpacker->GetByteStream();
		if (pUnpacker->IsConnected() == false || m_Status != STATUS_COMPRESSING)
			break;
		int Bytes = pUnpacker->GetRestSize();

#ifndef CONF_DUMMY
		archive_write_data(m_Archive, pUnpacker->GetRest(), Bytes);
#else
		pUnpacker->GetRest();
#endif
		ReceivedSize += Bytes;
	}

	if (ReceivedSize < FileSize)
	{
		OnFileError(pFile, "Stream interrupted");
#ifndef CONF_DUMMY
		int NameSize = str_length(pFile->m_aName) + str_length(".fail");
		char *pFailFileName = new char[NameSize];
		str_format(pFailFileName, NameSize, "%s.fail", pFile->m_aName + 1);
		archive_entry_set_pathname(pEntry, pFailFileName);//remove /(pEntry);
		delete[] pFailFileName;
#endif
	}
	else
		pFile->m_Status = FILESTATUS_COMPRESSED;

#ifndef CONF_DUMMY
	archive_entry_free(pEntry);
#endif
}

void CCompressor::FileTreeStream(CUnpacker *pUnpacker, char Device)
{
	char aStreamData[MAX_PACKET_LEN];
	int StreamBytes = 0;

	const char *pPath = pUnpacker->GetString();
	const char *pName = pUnpacker->GetString();
	int Bundle = NewBundle(pName, pPath, Device);

	if (Bundle != -1)
	{
		while (m_Status == STATUS_COLLECTING)
		{
			const char *pFileName = pUnpacker->GetString();
			if (pUnpacker->GetError() == true)
				break;

			AddFile(Bundle, pFileName);
		}
	}

	if(m_Status == STATUS_COLLECTING)
		m_Status = STATUS_COLLECTING_DONE;
}

void CCompressor::Reset()
{
	lock_wait(m_FilesLock);
	m_Status = STATUS_INACTIVE;
	m_lBundles.RemoveAll();
	lock_unlock(m_FilesLock);
}

bool CCompressor::BundleHasFileWithStatus(CCompressor::CCompressBundle *pBundle, int Status)
{
	for (int i = 0; i < pBundle->m_lFiles.Size(); i++)
		if (pBundle->m_lFiles[i].m_Status == Status)
			return true;
	return false;
}

int CCompressor::CountFiles()
{
	int NumFiles = 0;
	for (int Bundle = 0; Bundle < m_lBundles.Size(); Bundle++)
		NumFiles += m_lBundles[Bundle].m_lFiles.Size();
	return NumFiles;
}

int CCompressor::CountCompressed()
{
	int NumCompressed = 0;
	for (int Bundle = 0; Bundle < m_lBundles.Size(); Bundle++)
		for (int i = 0; i < m_lBundles[Bundle].m_lFiles.Size(); i++)
			if (m_lBundles[Bundle].m_lFiles[i].m_Status == FILESTATUS_COMPRESSED)
				NumCompressed++;
	return NumCompressed;
}

int CCompressor::CountErrors()
{
	int NumErrors = 0;
	for (int Bundle = 0; Bundle < m_lBundles.Size(); Bundle++)
		for (int i = 0; i < m_lBundles[Bundle].m_lFiles.Size(); i++)
			if (m_lBundles[Bundle].m_lFiles[i].m_Status == FILESTATUS_ERROR)
				NumErrors++;
	return NumErrors;
}

void CCompressor::SetArchiveName(const char *pName)
{
	str_copy(m_aArchiveName, pName, sizeof(m_aArchiveName));
}