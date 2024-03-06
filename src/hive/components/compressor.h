#pragma once

#include <core/array.hpp>
#include <core/wrapper.h>

class CHive;

class CCompressor
{
public:
	struct CCompressFile
	{
		char m_aName[1024];
		int m_Status;
		char m_aErrorMessage[256];
	};

	struct CCompressBundle
	{
		CArray<CCompressFile> m_lFiles;
		char m_aRootPath[1024];
		char m_aName[1024];
		char m_Device;
	};

	enum
	{
		STATUS_INACTIVE = 0,
		STATUS_COLLECTING,
		STATUS_COLLECTING_DONE,
		STATUS_COMPRESSING,
		STATUS_DONE,

		FILESTATUS_UNCOMPRESSED=0,
		FILESTATUS_COMPRESSED,
		FILESTATUS_ERROR,
	};

private:
	CHive *m_pHive;
	char m_aArchiveName[128];
	int m_Status;
	CArray<CCompressBundle> m_lBundles;
	struct archive *m_Archive;
	LOCK m_FilesLock;
	LOCK m_AddBundleLock;

	static void Compress(void *pUser);
	void OnFileError(CCompressFile *pFile, const char *pErrorMessage);
	int NewBundle(const char *pName, const char *pRootPath, char Device);
	void AddFile(int BundleID, const char *pFile);

public:
	CCompressor(CHive *pHive);

	void Start();
	void TryCompress();
	void Finish();
	void RemoveItem(int Bundle, int File);
	int NumBundles();
	int NumFiles(int Bundle);
	bool AddBundleFile(const char *pName, const char *pPath, char Device);
	bool AddBundle(char Device, const char *pPath);
	void StartReadingFiles();
	CCompressBundle *GetBundle(int Bundle);
	void StopReadingFiles();
	void FileStream(class CTCPUnpacker *pUnpacker, CCompressFile *pFile);
	void FileTreeStream(class CUnpacker *pUnpacker, char Device);
	void Reset();
	bool BundleHasFileWithStatus(CCompressBundle *pBundle, int Status);
	int CountFiles();
	int CountCompressed();
	int CountErrors();
	void SetArchiveName(const char *pName);
	char *ArchiveName() { return m_aArchiveName; }
	int GetStatus() const { return m_Status; }
};