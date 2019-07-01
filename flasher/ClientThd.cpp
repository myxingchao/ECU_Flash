
///////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>

//#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "ClientThd.h"

#define DEFAULT_PORT	"27015"
#define DEFAULT_BUFLEN	512
//----------------------------------------------
CClientThd::CClientThd()
{
}
CClientThd::~CClientThd()
{
}
void CClientThd::ClientComm(CHAR* log)
{
	//All processes (applications or DLLs) that call Winsock functions must initialize the use of the Windows Sockets DLL
	//before making other Winsock functions calls. This also makes certain that Winsock is supported on the system.
	//The WSAStartup function is called to initiate use of WS2_32.dll.
	//The WSADATA structure contains information about the Windows Sockets implementation.
	//The MAKEWORD(2,2) parameter of WSAStartup makes a request for version 2.2 of Winsock on the system,
	//and sets the passed version as the highest version of Windows Sockets support that the caller can use.
	WSADATA wsa;
	LONG ret = WSAStartup(MAKEWORD(2,2), &wsa);
	if (ret != 0)
	{
		sprintf(log, "WSAStartup failed: %d\n", ret);
		return;
	}

	//The getaddrinfo function is used to determine the values in the sockaddr structure:
	//AF_INET is used to specify the IPv4 address family. 
	//SOCK_STREAM is used to specify a stream socket. 
	//IPPROTO_TCP is used to specify the TCP protocol . 
	//AI_PASSIVE flag indicates the caller intends to use the returned socket address structure in a call to the bind function. When the AI_PASSIVE flag is set and nodename parameter to the getaddrinfo function is a NULL pointer, the IP address portion of the socket address structure is set to INADDR_ANY for IPv4 addresses or IN6ADDR_ANY_INIT for IPv6 addresses. 
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	//hints.ai_flags = AI_PASSIVE;
	//Resolve the local address and port to be used by the server.
	addrinfo* result;
	ret = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (ret != 0)
	{
		sprintf(log, "Getaddrinfo failed: %d\n", ret);
		WSACleanup();
		return;
	}

	//Call the socket function and return its value to the ConnectSocket variable.
	//For this application, use the first IP address returned by the call to getaddrinfo that matched the address family,
	//socket type, and protocol specified in the hints parameter. In this example,
	//a TCP stream socket was specified with a socket type of SOCK_STREAM and a protocol of IPPROTO_TCP.
	//The address family was left unspecified (AF_UNSPEC), so the returned IP address could be either an IPv6 or IPv4 address for the server. 
	//If the client application wants to connect using only IPv6 or IPv4,
	//then the address family needs to be set to AF_INET6 for IPv6 or AF_INET for IPv4 in the hints parameter. 
	SOCKET csocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (csocket == INVALID_SOCKET)
	{
		sprintf(log, "Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//Call the connect function, passing the created socket and the sockaddr structure as parameters.
	if (connect(csocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
	{
		sprintf(log, "Unable to connect to server!\n");
		freeaddrinfo(result);
		closesocket(csocket);
		WSACleanup();
		return;
	}
	freeaddrinfo(result);

	CHAR rbuf[DEFAULT_BUFLEN];
	LONG rlen;
	//Send an initial buffer.
	CHAR* sbuf = "this is a test";
	rlen = send(csocket, sbuf, (int)strlen(sbuf), 0);
	if (rlen < 0)
	{
		sprintf(log, "Sending failed: %d\n", WSAGetLastError());
		closesocket(csocket);
		WSACleanup();
		return;
	}
	sprintf(log, "Bytes Sent: %ld\n", rlen);

	//Receive data until the server closes the connection.
	while (0 == 0)
	{
		rlen = recv(csocket, rbuf, DEFAULT_BUFLEN, 0);
		if ( rlen > 0 )
			sprintf(log, "Bytes received: %d\n", rlen);
		else if ( rlen == 0 )
			sprintf(log, "Connection closed\n");
		else//if (rlen < 0)
			sprintf(log, "Receiving failed: %d\n", WSAGetLastError());
	}

	//When the client is done sending data to the server, the shutdown function can be called specifying SD_SEND to shutdown the sending side of the socket.
	//This allows the server to release some of the resources for this socket. The client application can still receive data on the socket. 
	//Shutdown the send half of the connection since no more data will be sent.
	if (shutdown(csocket, SD_SEND) == SOCKET_ERROR)
	{
		printf("Shutdown failed: %d\n", WSAGetLastError());
		closesocket(csocket);
		WSACleanup();
		return;
	}
	//When the client application is done receiving data, the closesocket function is called to close the socket. 
	closesocket(csocket);
	//When the client application is completed using the Windows Sockets DLL, the WSACleanup function is called to release resources.
	WSACleanup();
}