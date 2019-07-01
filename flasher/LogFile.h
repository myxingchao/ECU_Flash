#pragma once

class CLogFile
{
public:
	CLogFile();
	~CLogFile();

public:
	LONG	SaveLog(CHAR* fname, CHAR* content);
	LONG	LoadLog(CHAR* fname, CHAR* content);

protected:
};