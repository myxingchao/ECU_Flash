#pragma once
#include <winsock2.h>

#define DEFAULT_PORT	"27015"
#define DEFAULT_BLEN	512

class CClientThd
{
protected:
	CClientThd();
	virtual	~CClientThd();
public:
	void ClientComm(CHAR* log);
};