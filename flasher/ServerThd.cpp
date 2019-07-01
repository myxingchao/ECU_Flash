///////////////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_PORT	"27015"
#define DEFAULT_BLEN	512

#include "stdafx.h"
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "ServerThd.h"

#define DEFAULT_PORT	"27015"
#define DEFAULT_BUFLEN	512
///////////////////////////////////////////////////////////////////////////////////////////
CServerThd::CServerThd()
{
	m_pListenThd = NULL;
	m_pListenThd = AfxBeginThread(ServerListen, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	m_pListenThd->m_bAutoDelete = TRUE;
	m_pListenThd->ResumeThread();
	m_nThd = 0;
}
CServerThd::~CServerThd()
{
	/*if (m_pServerThd != NULL)
	{
		m_pServerThd->SuspendThread();
		//ULONG code;
		//::GetExitCodeThread(m_pServerThd->m_hThread, &code);
		//::AfxEndThread(code, FALSE);
		delete m_pServerThd;
		m_pServerThd = NULL;
	}*/
}
CHAR* CServerThd::GetHostIP()
{
	CHAR hname[MAX_PATH];
	gethostname(hname, sizeof(hname));
    hostent* hp = gethostbyname(hname);
	CHAR* ip = NULL;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 0), &wsa) == 0)
	{
		CHAR hname[MAX_PATH];
		if (gethostname(hname, sizeof(hname)) == 0)
		{
			hostent* hp = gethostbyname(hname);
			ip = inet_ntoa(*(in_addr*)hp->h_addr_list[0]);
			//sprintf(hname, "%u.%u.%u.%u",
			//	(UINT)(((BYTE*) hp->h_addr_list[0])[0]),
			//	(UINT)(((BYTE*) hp->h_addr_list[0])[1]),
			//	(UINT)(((BYTE*) hp->h_addr_list[0])[2]),
			//	(UINT)(((BYTE*) hp->h_addr_list[0])[3]));
		}
		WSACleanup();
	}
	return ip;
}
UINT CServerThd::ServerListen(LPVOID param)
{
	CHAR log[1024];
	((CServerThd*)param)->ServerListen(log);
	return 0;
}
void CServerThd::ServerListen(CHAR* log)
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
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	//Resolve the local address and port to be used by the server.
	addrinfo* result;
	ret = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (ret != 0)
	{
		sprintf(log, "Getaddrinfo failed: %d\n", ret);
		WSACleanup();
		return;
	}

	//Call the socket function and return its value to the ListenSocket variable.
	//For this server application, use the first IP address returned by the call to getaddrinfo that matched the address family,
	//socket type, and protocol specified in the hints parameter. In this example,
	//a TCP stream socket for IPv4 was requested with an address family of IPv4,
	//a socket type of SOCK_STREAM and a protocol of IPPROTO_TCP. So an IPv4 address is requested for the ListenSocket. 
	//If the server application wants to listen on IPv6, then the address family needs to be set to AF_INET6 in the hints parameter.
	//If a server wants to listen on both IPv6 and IPv4, then two listen sockets need to be created, one for IPv6 and one for IPv4.

	//Create a SOCKET object called lsocket for the server to listen for client connections.
	//SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKET lsocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (lsocket == INVALID_SOCKET)
	{
		sprintf(log, "Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//For a server to accept client connections, it must be bound to a network address within the system.
	//The following code demonstrates how to bind a socket that has already been created to an IP address and port.
	//Client applications use the IP address and port to connect to the host network.
	//The sockaddr structure holds information regarding the address family, IP address, and port number. 
	
	//Setup the TCP listening socket: passing the created socket and sockaddr structure returned from the getaddrinfo function as parameters.
	if (bind(lsocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
	{
		sprintf(log, "Binding failed: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(lsocket);
		WSACleanup();
		return;
	}
    freeaddrinfo(result);

	//Call the listen function, passing as parameters the created socket and a value for the backlog,
	//maximum length of the queue of pending connections to accept. In this example, the backlog parameter was set to SOMAXCONN.
	//This value is a special constant that instructs the Winsock provider for this socket to allow a maximum reasonable number of pending connections in the queue.
	if (listen(lsocket, SOMAXCONN) == SOCKET_ERROR) 
	{
		sprintf(log, "Error at bind(): %ld\n", WSAGetLastError());
		closesocket(lsocket);
		WSACleanup();
		return;
	}

	//Normally a server application would create a continuous loop that checks for connections requests.
	//If a connection request occurs, call the accept function to handle the request. 
	//Note that in this basic example, the code only accepts a single connection. 
	while (0 == 0)
	{
		//Check the exit flag before next waitting.
		if (::WaitForSingleObject(m_hExit, 0) != WAIT_TIMEOUT)//set
			break;
		//Create a temporary SOCKET object called csocket for accepting connections from clients.
		m_pCurSocket = accept(lsocket, NULL, NULL);
		if (m_pServerSocket[m_nThd] == INVALID_SOCKET)
		{
			sprintf(log, "Accept %d failed: %d\n", m_nThd, WSAGetLastError());
			break;
		}
		//Start receive thread.
		m_pServerThd[m_nThd] = AfxBeginThread(ServerReceive, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
		m_pServerThd[m_nThd]->m_bAutoDelete = TRUE;
		m_pServerThd[m_nThd]->ResumeThread();
		m_nThd ++;
	}
	//No longer need the server listen socket.
	closesocket(lsocket);
	WSACleanup();
}
UINT CServerThd::ServerReceive(LPVOID param)
{
	CHAR log[1024];
	((CServerThd*)param)->ServerReceive(log);
	return 0;
}
void CServerThd::ServerReceive(CHAR* log)
{
	SOCKET ssocket = m_pCurSocket;
	//The send and recv functions both return an integer value of the number of bytes sent or received, respectively, or an error.
	//Each function also takes the same parameters: the active socket, a char buffer, the number of bytes to send or receive, and any flags to use.
	CHAR rbuf[DEFAULT_BUFLEN];
	LONG rlen;
	//Receive until the peer shuts down the connection.
	while (0 == 0)
	{
		rlen = recv(ssocket, rbuf, DEFAULT_BUFLEN, 0);
		if (rlen > 0)
		{
			sprintf(log, "Bytes received: %d\n", rlen);
			//Echo the buffer back to the sender.
			if (send(ssocket, rbuf, rlen, 0) == SOCKET_ERROR)
			{
				sprintf(log, "Sending failed: %d\n", WSAGetLastError());
				break;
			}
		}
		else if (rlen == 0)
		{
			sprintf(log, "Connection closing...\n");
			break;
		}
		else//if (rlen < 0)
		{
			sprintf(log, "Recv failed: %d\n", WSAGetLastError());
			break;
		}
	}

	//When the server is done sending data to the client, the shutdown function can be called specifying SD_SEND to shutdown the sending side of the socket.
	//This allows the client to release some of the resources for this socket. The server application can still receive data on the socket. 
	//shutdown the send half of the connection since no more data will be sent.
	if (shutdown(ssocket, SD_SEND) == SOCKET_ERROR)
		sprintf(log, "Shutdown failed: %d\n", WSAGetLastError());
	//When the client application is done receiving data, the closesocket function is called to close the socket. 
	closesocket(ssocket);
	//When the client application is completed using the Windows Sockets DLL, the WSACleanup function is called to release resources.
	WSACleanup();
}



///////////////////////////////////////////////////////////////////////////////////////////
/*
void BaseServer()
{
	//All processes (applications or DLLs) that call Winsock functions must initialize the use of the Windows Sockets DLL
	//before making other Winsock functions calls. This also makes certain that Winsock is supported on the system.
	//The WSAStartup function is called to initiate use of WS2_32.dll.
	//The WSADATA structure contains information about the Windows Sockets implementation.
	//The MAKEWORD(2,2) parameter of WSAStartup makes a request for version 2.2 of Winsock on the system,
	//and sets the passed version as the highest version of Windows Sockets support that the caller can use.
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		printf("WSAStartup failed: %d\n", ret);
		return FALSE;
	}

	//The getaddrinfo function is used to determine the values in the sockaddr structure:
	//AF_INET is used to specify the IPv4 address family. 
	//SOCK_STREAM is used to specify a stream socket. 
	//IPPROTO_TCP is used to specify the TCP protocol . 
	//AI_PASSIVE flag indicates the caller intends to use the returned socket address structure in a call to the bind function. When the AI_PASSIVE flag is set and nodename parameter to the getaddrinfo function is a NULL pointer, the IP address portion of the socket address structure is set to INADDR_ANY for IPv4 addresses or IN6ADDR_ANY_INIT for IPv6 addresses. 
	addrinfo *result = NULL,
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	//Resolve the local address and port to be used by the server.
	LONG ret = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result)
	if (ret != 0)
	{
		printf("getaddrinfo failed: %d\n", ret);
		WSACleanup();
		return FALSE;
	}

	//Call the socket function and return its value to the ListenSocket variable.
	//For this server application, use the first IP address returned by the call to getaddrinfo that matched the address family,
	//socket type, and protocol specified in the hints parameter. In this example,
	//a TCP stream socket for IPv4 was requested with an address family of IPv4,
	//a socket type of SOCK_STREAM and a protocol of IPPROTO_TCP. So an IPv4 address is requested for the ListenSocket. 
	//If the server application wants to listen on IPv6, then the address family needs to be set to AF_INET6 in the hints parameter.
	//If a server wants to listen on both IPv6 and IPv4, then two listen sockets need to be created, one for IPv6 and one for IPv4.

	//Create a SOCKET object called ListenSocket for the server to listen for client connections.
	SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return FALSE;
	}

	//For a server to accept client connections, it must be bound to a network address within the system.
	//The following code demonstrates how to bind a socket that has already been created to an IP address and port.
	//Client applications use the IP address and port to connect to the host network.
	//The sockaddr structure holds information regarding the address family, IP address, and port number. 
	
	//Setup the TCP listening socket: passing the created socket and sockaddr structure returned from the getaddrinfo function as parameters.
	if (bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR)
	{
		printf("bind failed: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return FALSE;
	}
    freeaddrinfo(result);

	//Call the listen function, passing as parameters the created socket and a value for the backlog,
	//maximum length of the queue of pending connections to accept. In this example, the backlog parameter was set to SOMAXCONN.
	//This value is a special constant that instructs the Winsock provider for this socket to allow a maximum reasonable number of pending connections in the queue.
	if (listen(ListenSocket, SOMAXCONN ) == SOCKET_ERROR) 
	{
		printf( "Error at bind(): %ld\n", WSAGetLastError() );
		closesocket(ListenSocket);
		WSACleanup();
		return FALSE;
	}

	//Normally a server application would create a continuous loop that checks for connections requests.
	//If a connection request occurs, call the accept function to handle the request. 
	//Note that in this basic example, the code only accepts a single connection. 

	//Create a temporary SOCKET object called ClientSocket for accepting connections from clients.
	SOCKET ClientSocket = INVALID_SOCKET;
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET)
	{
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return FALSE;
	}
	//No longer need the server socket.
	closesocket(ListenSocket);

	//The send and recv functions both return an integer value of the number of bytes sent or received, respectively, or an error.
	//Each function also takes the same parameters: the active socket, a char buffer, the number of bytes to send or receive, and any flags to use.
	char rbuf[DEFAULT_BUFLEN];
	// Receive until the peer shuts down the connection.
	do {
		LONG rlen = recv(ClientSocket, rbuf, DEFAULT_BUFLEN, 0);
		if (rlen > 0)
		{
			printf("Bytes received: %d\n", rlen);

			// Echo the buffer back to the sender.
			LONG sret = send(ClientSocket, rbuf, rlen, 0);
			if (sret == SOCKET_ERROR)
			{
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return FALSE;
			}
			printf("Bytes sent: %d\n", sret);
		}
		else if (rlen == 0)
		{
			printf("Connection closing...\n");
		}
		else
		{
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return FALSE;
		}
	} while (rlen > 0);

	//When the server is done sending data to the client, the shutdown function can be called specifying SD_SEND to shutdown the sending side of the socket.
	//This allows the client to release some of the resources for this socket. The server application can still receive data on the socket. 
	//shutdown the send half of the connection since no more data will be sent.
	if (shutdown(ClientSocket, SD_SEND) == SOCKET_ERROR)
	{
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return FALSE;
	}
	//When the client application is done receiving data, the closesocket function is called to close the socket. 
	closesocket(ConnectSocket);
	//When the client application is completed using the Windows Sockets DLL, the WSACleanup function is called to release resources.
	WSACleanup();
	return TRUE;
}
#define DEFAULT_PORT "27015"
void BaseClient()
{
	//Declare an addrinfo object that contains a sockaddr structure and initialize these values. For this application, the Internet address family is unspecified so that either an IPv6 or IPv4 address can be returned. The application requests the socket type to be a stream socket for the TCP protocol. 
	addrinfo *result = NULL,
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//Call the getaddrinfo function requesting the IP address for the server name passed on the command line.
	//The getaddrinfo function returns its value as an integer that is checked for errors. 
	//Resolve the server address and port.
	LONG ret = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (ret != 0)
	{
		printf("getaddrinfo failed: %d\n", ret);
		WSACleanup();
		return FALSE;
	}

	//Call the socket function and return its value to the ConnectSocket variable.
	//For this application, use the first IP address returned by the call to getaddrinfo that matched the address family,
	//socket type, and protocol specified in the hints parameter. In this example,
	//a TCP stream socket was specified with a socket type of SOCK_STREAM and a protocol of IPPROTO_TCP.
	//The address family was left unspecified (AF_UNSPEC), so the returned IP address could be either an IPv6 or IPv4 address for the server. 
	//If the client application wants to connect using only IPv6 or IPv4,
	//then the address family needs to be set to AF_INET6 for IPv6 or AF_INET for IPv4 in the hints parameter. 

	//Attempt to connect to the first address returned by the call to getaddrinfo.
	ptr = result;
	//Create a SOCKET object called ConnectSocket for connecting to server.
	SOCKET ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return FALSE;
	}

	//Call the connect function, passing the created socket and the sockaddr structure as parameters.
	if (connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
	{
		printf("Unable to connect to server!\n");
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		WSACleanup();
		return FALSE;
	}
	freeaddrinfo(result);

	char* sbuf = "this is a test";
	char rbuf[DEFAULT_BUFLEN];
	LONG rlen;

	//Send an initial buffer
	rlen = send(ConnectSocket, sbuf, (int)strlen(sbuf), 0);
	if (rlen == SOCKET_ERROR)
	{
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return FALSE;
	}
	printf("Bytes Sent: %ld\n", rlen);

	//Receive data until the server closes the connection.
	do {
		rlen = recv(ConnectSocket, rbuf, DEFAULT_BUFLEN, 0);
		if ( rlen > 0 )
			printf("Bytes received: %d\n", rlen);
		else if ( rlen == 0 )
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());

	} while(rlen > 0);

	//When the client is done sending data to the server, the shutdown function can be called specifying SD_SEND to shutdown the sending side of the socket.
	//This allows the server to release some of the resources for this socket. The client application can still receive data on the socket. 
	//Shutdown the send half of the connection since no more data will be sent.
	if (shutdown(ConnectSocket, SD_SEND) == SOCKET_ERROR)
	{
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return FALSE;
	}
	//When the client application is done receiving data, the closesocket function is called to close the socket. 
	closesocket(ConnectSocket);
	//When the client application is completed using the Windows Sockets DLL, the WSACleanup function is called to release resources.
	WSACleanup();
	return TRUE;
}
*/

/*
///////////////////////////////////////////////////////////////////////////////////////////////
#include <winsock2.h> 
#pragma comment(lib,"ws2_32.lib") 
#include "ServerThd.h"

#define PORT			10001
#define	MAXTEMPBUFFER	4100

#define	COMM_OK			0
#define	COMM_ERR		1
#define	SEND_END		0
#define	SEND_REMAIN		1
#define	RECV_NONE		0
#define	RECV_END		1
#define	RECV_CONT		2
#define	MIN(a,b) (((a) < (b)) ? (a) : (b))

CServerThd::CServerThd()
{
}
CServerThd::~CServerThd()
{
	if (m_pServerThd != NULL)
	{
		m_pServerThd->SuspendThread();
		//ULONG code;
		//::GetExitCodeThread(m_pServerThd->m_hThread, &code);
		//::AfxEndThread(code, FALSE);
		delete m_pServerThd;
		m_pServerThd = NULL;
	}
}
void CServerThd::NewThread()
{
	m_pServerThd = NULL;
	m_pServerThd = AfxBeginThread(ServerThd, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	m_pServerThd->m_bAutoDelete = FALSE;
	m_pServerThd->ResumeThread();
}
void CServerThd::SetTitle(LPCTSTR lpszTitle)
{
	WORD		wVersionRequested;
	WSADATA		wsa;
	char		name[255];
	CString		strIP;
	PHOSTENT	hostinfo;
	wVersionRequested = MAKEWORD(2, 0);

	if (WSAStartup(wVersionRequested, &wsa) == 0)
	{
		if (gethostname(name, sizeof(name)) == 0)
		{
			if ((hostinfo = gethostbyname(name)) != NULL)
			{
				strIP = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);
			}
		}
		WSACleanup();
	}
}
UINT CServerThd::ServerThd(LPVOID pParam)
{
	char log[500];

	WSADATA  wsa;
	int  iRet = WSAStartup(MAKEWORD(2,2), &wsa);
	if (iRet != NO_ERROR)
		((CServerThd*)pParam)->AddRecvLog("Error at WSAStartup().");

	//create  a  socket
	SOCKET  initSocket;
	initSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (initSocket == INVALID_SOCKET)
	{
		sprintf(log, "Error at socket():%ld", WSAGetLastError());
		((CServerThd*)pParam)->AddRecvLog(log);
		WSACleanup();
		return  0;
	}

	//bind  a  socket
	sockaddr_in  service;
	service.sin_family		= AF_INET;
	service.sin_addr.s_addr = htonl(INADDR_ANY);
	service.sin_port		= htons(((CServerThd*)pParam)->m_nThdNum + PORT);
	((CServerThd*)pParam)->m_nThdNum ++;

	if (bind(initSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
	{
		((CServerThd*)pParam)->AddRecvLog("Bind failed.");
		shutdown(initSocket, 2);
		closesocket(initSocket);
		return  0;
	}
	else
	{
		sprintf(log, "Server%d bind successful(port %d).", ((CServerThd*)pParam)->m_nThdNum, ntohs(service.sin_port));
		((CServerThd*)pParam)->AddRecvLog(log);
	}

	//listen  on  a  socket  
	if (listen(initSocket, 20) == SOCKET_ERROR)
		((CServerThd*)pParam)->AddRecvLog("Error listening on socket.");
	else
	{
		((CServerThd*)pParam)->AddRecvLog("Listening Ok.");
	}

	//accept  a  connection
	int len = SOCKETBUFFSIZE;
	SOCKET acceptSocket = 0;

	char msg[100], data[100];
	while(TRUE)
	{
		//creat socket
		if (acceptSocket != 0)
		{
			shutdown(acceptSocket, 2);
			closesocket(acceptSocket);
			//((CServerThd*)pParam)->ClearLog();

			sprintf(log, "The connection was terminated, and the error code is %d.", ((CServerThd*)pParam)->m_nEtherErrorNo);
			((CServerThd*)pParam)->AddRecvLog(log);
			switch (((CServerThd*)pParam)->m_nEtherErrorNo)
			{
				case WSANOTINITIALISED: sprintf(log, "A successful WSAStartup must occur before using this function."); break;
				case WSAENETDOWN:		sprintf(log, "The network subsystem has failed."); break;
				case WSAEFAULT:			sprintf(log, "The buf parameter is not completely contained in a valid part of the user address space."); break;
				case WSAENOTCONN:		sprintf(log, "The socket is not connected."); break;
				case WSAEINTR:			sprintf(log, "The (blocking) call was canceled through WSACancelBlockingCall."); break;
				case WSAEINPROGRESS:	sprintf(log, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function."); break;
				case WSAENETRESET:		sprintf(log, "The connection has been broken due to the keep-alive activity detecting a failure while the operation was in progress."); break;
				case WSAENOTSOCK:		sprintf(log, "The descriptor is not a socket."); break;
				case WSAEOPNOTSUPP:		sprintf(log, "MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, out-of-band data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations."); break;
				case WSAESHUTDOWN:		sprintf(log, "The socket has been shut down; it is not possible to recv on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH."); break;
				case WSAEWOULDBLOCK:	sprintf(log, "The socket is marked as nonblocking and the receive operation would block."); break;
				case WSAEMSGSIZE:		sprintf(log, "The message was too large to fit into the specified buffer and was truncated."); break;
				case WSAEINVAL:			sprintf(log, "The socket has not been bound with bind, or an unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled or (for byte stream sockets only) len was zero or negative."); break;
				case WSAECONNABORTED:	sprintf(log, "The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable."); break;
				case WSAETIMEDOUT:		sprintf(log, "The connection has been dropped because of a network failure or because the peer system failed to respond."); break;
				case WSAECONNRESET:		sprintf(log, "The virtual circuit was reset by the remote side executing a \"hard\" or \"abortive\" close. The application should close the socket as it is no longer usable. On a UDP datagram socket this error would indicate that a previous send operation resulted in an ICMP \"Port Unreachable\" message."); break;
				default:				sprintf(log, "");
			}
			if (log[0] != 0)
				((CServerThd*)pParam)->AddRecvLog(log);
			((CServerThd*)pParam)->AddRecvLog("");
		}
		((CServerThd*)pParam)->AddRecvLog("Waiting for a client to connect...");
		acceptSocket = accept(initSocket, NULL, NULL);//wating for connecting.
		if (acceptSocket == 0xFFFFFFFF)
		{
			if (((CNFSimView*)((CServerThd*)pParam)->GetView())->MessageBox("Can not create a socket, please restart the program.\n", NULL, MB_OKCANCEL | MB_ICONASTERISK) == IDOK)
			{
				shutdown(initSocket, 2);
				closesocket(initSocket);
				((CServerThd*)pParam)->DisableLog();
				break;
			}
			else
				continue;
		}
		((CServerThd*)pParam)->ClearLog();
		((CServerThd*)pParam)->m_sRecvTbl.length = 0;
		((CServerThd*)pParam)->m_sRecvTbl.now=0;
		((CServerThd*)pParam)->m_sRecvTbl.remain = 0;
		((CServerThd*)pParam)->m_sRecvTbl.status = RECV_CONT;
		((CServerThd*)pParam)->m_sSendTbl.length = 0;
		((CServerThd*)pParam)->m_sSendTbl.now=0;
		((CServerThd*)pParam)->m_sSendTbl.remain = 0;
		((CServerThd*)pParam)->m_sSendTbl.status = SEND_END;
		((CServerThd*)pParam)->m_sRecvTbl.status = RECV_NONE;
		((CServerThd*)pParam)->AddRecvLog("A server thread is created.");

		//timeout
		//int timeout = 90000;
		//setsockopt(acceptSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(int));
		//setsockopt(acceptSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(int));

		//get username
		strcpy(msg, "username:");
		if (((CServerThd*)pParam)->Send(acceptSocket, msg, strlen(msg)) == COMM_ERR) continue;
		if (((CServerThd*)pParam)->Receive(acceptSocket, data, 50, &len) == COMM_ERR) continue;
		data[len] = 0;
		sprintf(log, "%s %s", msg, data);
		((CServerThd*)pParam)->AddRecvLog(log);
		strcpy(msg, "");
		if (((CServerThd*)pParam)->Send(acceptSocket, msg, strlen(msg)) == COMM_ERR) continue;

		//get password
		strcpy(msg, "password:");
		if (((CServerThd*)pParam)->Send(acceptSocket, msg, strlen(msg)) == COMM_ERR) continue;
		if (((CServerThd*)pParam)->Receive(acceptSocket, data, 50, &len) == COMM_ERR) continue;
		data[len] = 0;
		sprintf(log, "%s %s", msg, data);
		((CServerThd*)pParam)->AddRecvLog(log);
		strcpy(msg, "");
		if (((CServerThd*)pParam)->Send(acceptSocket, msg, strlen(msg)) == COMM_ERR) continue;

		sprintf(log, "User logged in server%d(port %d).", ((CServerThd*)pParam)->m_nThdNum, ntohs(service.sin_port));
		((CServerThd*)pParam)->AddRecvLog(log);

		//ready confirm
		strcpy(msg, "Ctl server is ready.");
		if (((CServerThd*)pParam)->Send(acceptSocket, msg, strlen(msg)) == COMM_ERR) continue;

		Sleep(2000);
		((CServerThd*)pParam)->AddRecvLog("");
		((CServerThd*)pParam)->SyncLog();

		//query loop
		while (TRUE)
		{
			//receive from client
			if (((CServerThd*)pParam)->Receive(acceptSocket, data, 50, &len) == COMM_ERR) break;
			data[len] = 0;
			strcpy(msg, "");
			if (((CServerThd*)pParam)->Send(acceptSocket, msg, strlen(msg)) == COMM_ERR) break;

			//send to client
			if (strstr(data, "?") >= data)//need response
			{
				//================================== IEC(NF) =========================================
				if (strcmp(data, "?VER") == 0)
					strcpy(msg, "1.00\r\n");
				else if (strcmp(data, "?IDX") == 0)
					sprintf(msg,"%s\r\n", _NFSimModelStringCmd[((CServerThd*)pParam)->m_sNFSettings.model]);
				else if (strcmp(data, "?VLT") == 0)
					sprintf(msg,"%.1f\r\n", ((CServerThd*)pParam)->m_sNFSettings.voltage);
				else if (strcmp(data, "?RNG") == 0)
					sprintf(msg,"%04d\r\n", ((CServerThd*)pParam)->m_sNFSettings.range);
				else if (strcmp(data, "?FRQ") == 0)
					sprintf(msg,"%.2f\r\n", ((CServerThd*)pParam)->m_sNFSettings.frequency);
				else if (strcmp(data, "?OUT") == 0)
					sprintf(msg,"%04d\r\n", ((CServerThd*)pParam)->m_sNFSettings.output == 1? 0 : 1);
				else if (strcmp(data, "?VUP") == 0)
					sprintf(msg,"%.1f\r\n", ((CServerThd*)pParam)->m_sNFSettings.limit);
				else if (strcmp(data, "?HDR") == 0)
					sprintf(msg,"%04d\r\n", ((CServerThd*)pParam)->m_sNFSettings.header == 1? 0 : 1);
				else if (strcmp(data, "?TPS") == 0)
					sprintf(msg,"%d\r\n", ((CServerThd*)pParam)->m_sNFSettings.wiring == 0? 0x0000 : 0x1C00);
				else if (strcmp(data, "?STS") == 0)
					sprintf(msg,"%04d\r\n", ((CServerThd*)pParam)->m_sNFSettings.sts);
				else if (strcmp(data, "?SRQ") == 0)
					sprintf(msg,"%04d\r\n", ((CServerThd*)pParam)->m_sNFSettings.srq);
				else if (strcmp(data, "?ERS") == 0)
					sprintf(msg,"%04d\r\n", ((CServerThd*)pParam)->m_sNFSettings.ers);
				//================================== IEC(NF) =========================================
				//================================== FLICKER =========================================
				else if (strcmp(data, ":FLICKER:COUNT?") == 0)
					strcpy(msg, "10\n");
				else if (strcmp(data, ":FLICKER:INTERVAL?") == 0)
					strcpy(msg, "60\n");
				else if (strcmp(data, ":FLICKER:MEASUREMENT?") == 0)
					strcpy(msg, "FLC\n");
				else if (strcmp(data, ":NUMERIC:FORMAT?") == 0)
					strcpy(msg, "FLO\n");
				else if (strcmp(data, ":COMMUNICATE:HEADER?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":COMMUNICATE:VERBOSE?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":INPUT:CFACTOR?") == 0)
					strcpy(msg, "3\n");
				else if (strcmp(data, ":INPUT:MODULE?") == 0)
					strcpy(msg, "30,30,30,30\n");
				else if (strstr(data, ":FLICKER:ELEMENT") >= data)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":FLICKER:STATE?") == 0)
					strcpy(msg, "RES\n");
				else if (strcmp(data, ":DISPLAY:MODE?") == 0)
					strcpy(msg, "FLIC\n");
				else if (strcmp(data, "*OPT?") == 0)
					strcpy(msg, "FL\n");
				else if (strcmp(data, "*IDN?") == 0)
					strcpy(msg, "YOKOGAWA,760304-4\n");

				else if (strcmp(data, ":FLICKER:UN:MODE?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":FLICKER:FREQUENCY?") == 0)
					strcpy(msg, "50\n");
				else if (strcmp(data, ":FLICKER:UN:VALUE?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":FLICKER:DMIN:LIMIT?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":FLICKER:DC:STATE?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":FLICKER:DMAX:STATE?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":FLICKER:DT:STATE?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":FLICKER:PST:STATE?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":FLICKER:PLT:STATE?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":FLICKER:DC:LIMIT?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":FLICKER:DMAX:LIMIT?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":FLICKER:DT:LIMIT?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":FLICKER:PST:LIMIT?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":FLICKER:PLT:LIMIT?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":FLICKER:PLT:NVALUE?") == 0)
					strcpy(msg, "1\n");
				else if (strcmp(data, ":NUMERIC:FLICKER:PERIOD?") == 0)
					strcpy(msg, "3\n");
				else if (strcmp(data, ":NUMERIC:FLICKER:COUNT?") == 0)
					strcpy(msg, "3\n");

				else if (strcmp(data, ":INP:MODU?") == 0)
					strcpy(msg, "30, 30, 30, 30\n");
				else if (strcmp(data, ":INP:MODU? 0") == 0)
					strcpy(msg, "30\n");
				else if (strcmp(data, ":INP:MODU? 1") == 0)
					strcpy(msg, "30\n");
				else if (strcmp(data, ":INP:MODU? 2") == 0)
					strcpy(msg, "30\n");
				else if (strcmp(data, ":INP:MODU? 3") == 0)
					strcpy(msg, "30\n");
				else if (strcmp(data, ":INP:MODU? 4") == 0)
					strcpy(msg, "30\n");
				else if (strcmp(data, ":INP:SYNC:ELEM1?") == 0)
					strcpy(msg, "1\n");
				//================================== FLICKER =========================================
				else
					strcpy(msg, "there's no answer~\r\n");

				if (((CServerThd*)pParam)->Send(acceptSocket, msg, strlen(msg)) == COMM_ERR) break;

				if (strstr(msg, "\r") >= msg)
					*strstr(msg, "\r") = 0;
				((CServerThd*)pParam)->AddSentLog(msg);
			}//if (strstr(data, "?") >= data)
			else //without response
			{
				char value[100];
				char* pos = strchr(data, ' ');
				if (pos >= data)
					strcpy(value, pos + 1);

				if (strstr(data, "VLT") >= data)
					((CServerThd*)pParam)->m_sNFSettings.voltage = atof(value);
				else if (strstr(data, "RNG") >= data)
					((CServerThd*)pParam)->m_sNFSettings.range = atoi(value);
				else if (strstr(data, "FRQ") >= data)
					((CServerThd*)pParam)->m_sNFSettings.frequency = atof(value);
				else if (strstr(data, "OUT") >= data)
					((CServerThd*)pParam)->m_sNFSettings.output = (atoi(value) == 1 ? 0 : 1);
				else if (strstr(data, "VUP") >= data)
					((CServerThd*)pParam)->m_sNFSettings.limit = atof(value);
				else if (strstr(data, "HDR") >= data)
					((CServerThd*)pParam)->m_sNFSettings.header = (atoi(value) == 1 ? 0 : 1);
				else if (strstr(data, "TPS") >= data)
					((CServerThd*)pParam)->m_sNFSettings.wiring = (atoi(value) == 0x1C00 ? 1 : 0);
				else if (strstr(data, "OST") >= data)
				{
					if (((CServerThd*)pParam)->m_sNFSettings.rin == 0 || ((CServerThd*)pParam)->m_sNFSettings.rin == 2)
					{
						switch (atoi(value))
						{
							case 254: ((CServerThd*)pParam)->m_sNFSettings.impedance = 0;break;//ZERO
							case 100: ((CServerThd*)pParam)->m_sNFSettings.impedance = 1;break;//1P2W100V
							case 200: ((CServerThd*)pParam)->m_sNFSettings.impedance = 2;break;//1P2W200V
							case 230: ((CServerThd*)pParam)->m_sNFSettings.impedance = 3;break;//1P2W230V
						}
					}
					else //rin == ES4153 || rin == 4151
					{
						switch (atoi(value))
						{
							case 254: ((CServerThd*)pParam)->m_sNFSettings.impedance = 0;break;//ZERO
							case 100: ((CServerThd*)pParam)->m_sNFSettings.impedance = 1;break;//1P2W100V,JP1P
							case 200: ((CServerThd*)pParam)->m_sNFSettings.impedance = 1;break;//1P2W200V,JP1P
							case 123: ((CServerThd*)pParam)->m_sNFSettings.impedance = 2;break;//3P3W200V,JP3P
							case 102: ((CServerThd*)pParam)->m_sNFSettings.impedance = 2;break;//1P3W200V,JP3P
							case 233: ((CServerThd*)pParam)->m_sNFSettings.impedance = 3;break;//1P2W230V,EU1P
							case 230: ((CServerThd*)pParam)->m_sNFSettings.impedance = 3;break;//3P4W400V,EU3P
						}
					}
				}
				((CServerThd*)pParam)->AddSentLog("");
			}//no response
			//#this operation will take 200ms. so set display after communication.#
			((CServerThd*)pParam)->AddRecvLog(data);
		}
	}
	return  0;
}
int CServerThd::ReadSocket(SOCKET socket, char* buf, int len)
{
    int total, n;
    for (total = 0; total < len; total += n)
	{
		n = recv(socket, buf + total, len - total, 0);
		if (n < 0)//error
            return n;
        if(n == 0)//complete
            break;
    }
    return total;
}
int CServerThd::WriteSocket(SOCKET socket, char* buf, int len)
{
    int total, n;
    for (total = 0; total < len; total += n)
	{
		n = send(socket, buf + total, len - total, 0);
        if(n < 0)
            return n;
        if(n == 0)
            break;
    }
    return total;
}
int CServerThd::WriteSocketAdd(SOCKET socket, char* buf, int len, int end)
{
    int*    tmpP;
    int     tmpLen = len;

    tmpP = (int*)buf;
    if (end > 0)
		tmpLen |= ENDFLAGMASK;			// 転送終了フラグ
    *tmpP = htonl(tmpLen);			// ホスト->ネットワークバイトオーダーに変換

    return WriteSocket(socket, buf, len + ADDINFSIZE);
}
int CServerThd::CheckEnd(void)
{
    if((0 == m_sRecvTbl.length) && (RECV_END == m_sRecvTbl.status))		// クエリ内容が無い場合
		return COMM_OK;
	else
		return COMM_ERR;
}
int CServerThd::FlashRoutine(SOCKET socket)
{
    if (RECV_CONT == m_sRecvTbl.status)
	{									// クエリ内容が通信機器間に残っている場合
		int	time = 10000;				// 受信タイムアウトを10sに設定
		setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&time, sizeof(int));
		while(TRUE)
		{					// データがあるかぎり空読み
			if(MAXTEMPBUFFER > ReadSocket(socket, m_sSendTbl.buffer, MAXTEMPBUFFER))
				break;
		}
		//setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&m_nRecvTmo, sizeof(int));
	}
	TRACE("@@@@m_sRecvTbl.status = %d\n", m_sRecvTbl.status);
	if (m_sRecvTbl.length > 0)				// コマンドの場合、このパターン
		delete m_sRecvTbl.buffOrgP;

	return COMM_OK;
}
int CServerThd::ReceiveSetup(SOCKET socket)
{
	FlashRoutine(socket);								// 受信データがあればフラッシュ
	m_sRecvTbl.length = 0;
	m_sRecvTbl.now = 0;
	m_sRecvTbl.remain = 0;
	m_sRecvTbl.status = RECV_END;
	return COMM_OK;
}
int CServerThd::ReceiveOnly(SOCKET socket, char* buf, int blen, int* rlen)
{
	if (blen < 0)
	{
		return COMM_ERR;
	}
	int	copyLen;
	*rlen = 0;
	while (TRUE)
	{
		if (0 == m_sRecvTbl.length)
		{
			if (COMM_OK != ReceiveBlock(socket))
			{		// 1ブロック読み込み
				m_nEtherErrorNo = WSAGetLastError();
				return COMM_ERR;				// deleteはreceiveBlock内部でやっている
			}
		}
		if (0 == m_sRecvTbl.length)
			//return COMM_ERR;
			return COMM_OK;
		copyLen = MIN(m_sRecvTbl.length, blen);
		memcpy(buf, m_sRecvTbl.buffP, copyLen);
		m_sRecvTbl.length -= copyLen;
		blen -= copyLen;
		m_sRecvTbl.buffP += copyLen;
		buf += copyLen;
		*rlen += copyLen;
		if (0 == m_sRecvTbl.length)					// 一時バッファを使い切ったら、一時バッファを削除
			delete m_sRecvTbl.buffOrgP;
		if ((0 == blen) || (COMM_OK == CheckEnd()))	// 要求サイズ分読み込み終了の場合、もしくはクエリ内容が全て無くなった場合、抜ける
			break;
	}
	return COMM_OK;
}
int CServerThd::ReceiveBlock(SOCKET socket)
{
	int	endFlag;

	// サーバ側がSocketを閉じた場合、ReadSocketは0を返すので、その対策
	if (ADDINFSIZE > ReadSocket(socket, (char*)&m_sRecvTbl.length, ADDINFSIZE))// 付加情報(1ブロックのサイズ)を読む
	{
		m_sRecvTbl.status = RECV_CONT;			// ココでエラーとなった場合、機種間に残っているのでRECV_CONTとする(^3)
		return COMM_ERR;										// タイムアウトエラーの場合、常にココでエラー、従ってm_sRecvTbl.length==0
	}
	m_sRecvTbl.length=ntohl(m_sRecvTbl.length);						// ネットワーク->ホストバイトオーダーに変換
	endFlag = m_sRecvTbl.length & ENDFLAGMASK;						// 終了フラグを抜き出す
	m_sRecvTbl.length &= ~ENDFLAGMASK;								// 終了フラグを落とす
	if (ENDFLAGMASK == endFlag)									// 強制的にRECV_ENDに変換する
		m_sRecvTbl.status = RECV_END;
	else
		m_sRecvTbl.status = RECV_CONT;

	if(0 == m_sRecvTbl.length)										// コマンド・レスポンスの場合、下に行ってはReadSocketで
		return COMM_OK;											// エラーとなってしまう為、ココでリターン

	// サーバ側がSocketを閉じた場合、ReadSocketは0を返すので、その対策
	m_sRecvTbl.buffOrgP = m_sRecvTbl.buffP = new char[m_sRecvTbl.length];
	if (m_sRecvTbl.length > ReadSocket(socket, m_sRecvTbl.buffP, m_sRecvTbl.length))
	{
		m_sRecvTbl.status = RECV_CONT;			// ココでエラーとなった場合、機種間に残っているのでRECV_CONTとする
		return COMM_ERR;
	}
	return COMM_OK;
}
int CServerThd::Receive(SOCKET socket, char* buf, int blen, int* rlen)
{
	if (blen < 0)
	{
		return COMM_ERR;
	}
	return ReceiveOnly(socket, buf, blen, rlen);
}
int CServerThd::Send(SOCKET socket, char* msg, int len)
{
	if (len < 0)
	{
		return COMM_ERR;
	}
	// RECV_NONEは、CheckEndにおいてEndでない事を意味する。Recvする以前にCheckEndを呼ばれる可能性がある為。
	// 又、Recvする際のreceiveSetupをコールするかどうかの判断にも使用される。
	ReceiveSetup(socket);					// フラグを設けるとバグの可能性が高くなる為、直接コールする
	memcpy(&m_sSendTbl.buffer[ADDINFSIZE], msg, len);
	if (0 > WriteSocketAdd(socket, m_sSendTbl.buffer, len, 1))
	{
		m_nEtherErrorNo = WSAGetLastError();
		return COMM_ERR;
	}
	return COMM_OK;
}
*/