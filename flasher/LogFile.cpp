#include "StdAfx.h"
#include "LogFile.h"

CLogFile::CLogFile(void)
{
}
CLogFile::~CLogFile(void)
{
}
LONG CLogFile::SaveLog(CHAR* fname, CHAR* content)
{
	HANDLE hf = ::CreateFile(fname, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return 0;
	DWORD len;
	::WriteFile(hf, content, strlen(content), &len, NULL);
	::CloseHandle(hf);
	return len;
}
LONG CLogFile::LoadLog(CHAR* fname, CHAR* content)
{
	HANDLE hf = ::CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hf == INVALID_HANDLE_VALUE)
		return 0;
	DWORD len;
	::ReadFile(hf, content, ::GetFileSize(hf, NULL), &len, NULL);
	::CloseHandle(hf);
	return len;
}