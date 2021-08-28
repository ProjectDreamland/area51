/*
	Net.cpp
	Networking utilities.
*/

#include "SHCore.h"
#include <winsock2.h>

uint32	ipAddress::Any = INADDR_ANY;
uint32	ipAddress::Loopback = INADDR_LOOPBACK;
uint32	ipAddress::Broadcast = INADDR_BROADCAST;
uint32	sockClient::NumReferences = 0;

//	sockClient::sockClient

sockClient::sockClient()
{
	if(!NumReferences++)
	{
		WSADATA	WSAData;
		int32	Result = WSAStartup(MAKEWORD(1,0),&WSAData);
		if(Result)
			throw Str+"WSAStartup failed: "+toString(Result);
	}
}

//	sockClient::~sockClient

sockClient::~sockClient()
{
	if(!--NumReferences)
	{
		int32	Result = WSACleanup();
		if(Result)
			throw Str+"WSACleanup failed: "+toString(Result);
	}
}

//	ipAddress::getSockAddr

void ipAddress::getSockAddr(struct sockaddr_in* Result) const
{
	Result->sin_family = AF_INET;
	Result->sin_addr.S_un.S_addr = htonl(Address);
	Result->sin_port = htons(Port);
}

//	ipAddress::describe

string ipAddress::describe()
{
	return Str+toString((Address&0xff000000)>>24)+"."+toString((Address&0x00ff0000)>>16)+"."+toString((Address&0x0000ff00)>>8)+"."+toString(Address&0x000000ff)+":"+toString((uint32)Port);
}

//	dataSocket::dataSocket

dataSocket::dataSocket(uint32 InSocket):
	Socket(InSocket)
{
	if(Socket != INVALID_SOCKET)
	{
		uint32	NoBlocking = 1;
		if(::ioctlsocket((SOCKET)Socket,FIONBIO,&NoBlocking) == SOCKET_ERROR)
			throw Str+"Failed to make socket non-blocking: "+toString((int32)WSAGetLastError());
	}
}

//	dataSocket::~dataSocket

dataSocket::~dataSocket()
{
	if(Socket != INVALID_SOCKET)
	{
		shutdown((SOCKET)Socket,SD_SEND);
		closesocket((SOCKET)Socket);
		Socket = INVALID_SOCKET;
	}
}

//	dataSocket::connect

dataSocket* dataSocket::connect(ipAddress Address)
{
	sockClient	SockClient;
	SOCKET		Socket;

	// Create the socket.

	Socket = ::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(Socket == INVALID_SOCKET)
		throw Str+"Couldn't create socket for listen: "+toString((int32)WSAGetLastError());

	// Bind the socket.

	sockaddr_in	LocalSockAddr;
	ipAddress(ipAddress::Any,0).getSockAddr(&LocalSockAddr);
	if(::bind(Socket,(sockaddr*)&LocalSockAddr,sizeof(sockaddr_in)) == SOCKET_ERROR)
		throw Str+"Couldn't bind socket for listen: "+toString((int32)WSAGetLastError());

    // Connect to the remote socket.

	sockaddr_in	RemoteSockAddr;
	Address.getSockAddr(&RemoteSockAddr);
	if(::connect(Socket,(sockaddr*)&RemoteSockAddr,sizeof(sockaddr_in)) == SOCKET_ERROR)
		throw Str+"Couldn't connect socket: "+toString((int32)WSAGetLastError());

	return new dataSocket((uint32)Socket);
}

//	dataSocket::getRemoteAddress

ipAddress dataSocket::getRemoteAddress()
{
	sockaddr_in	RemoteSockAddr;
    int32		RemoteSockAddrLength = sizeof(sockaddr_in);
	if(getpeername((SOCKET)Socket,(sockaddr*)&RemoteSockAddr,(int*)&RemoteSockAddrLength) == SOCKET_ERROR)
		throw Str+"getpeername failed: "+toString((int32)WSAGetLastError());

	return ipAddress(ntohl(RemoteSockAddr.sin_addr.S_un.S_addr),ntohs(RemoteSockAddr.sin_port));
}

//	dataSocket::recv

bool dataSocket::recv(void* Data,uintmax DataLength)
{
	int32	Result = ::recv((SOCKET)Socket,(char*)Data,(int)DataLength,0);

	if(Result == SOCKET_ERROR)
		throw Str+"recv failed: "+toString((int32)WSAGetLastError());
	else
		return Result == DataLength;
}

//	dataSocket::partialReceive

uintmax dataSocket::partialReceive(void* Data,uintmax DataLength)
{
	int32	Result = ::recv((SOCKET)Socket,(char*)Data,(int)DataLength,0);

	if(Result == SOCKET_ERROR)
		throw Str+"recv failed: "+toString((int32)WSAGetLastError());
	else
		return Result;
}

//	dataSocket::send

void dataSocket::send(void* Data,uintmax DataLength)
{
	uintmax	BytesSent = 0;

	while(BytesSent < DataLength)
	{
		int32	Result = ::send((SOCKET)Socket,(char*)Data,(int)DataLength,0);

		if(Result == SOCKET_ERROR)
		{
			int32	LastError = WSAGetLastError();
			if(LastError == WSAEWOULDBLOCK)
				Sleep(10);
			else
				throw Str+"send failed: "+toString(LastError);
		}
		else
			BytesSent += Result;
	};
}

//	dataSocket::recvDataQueued

bool dataSocket::recvDataQueued()
{
	char	TempBuf[256];
	int32	NumBytes;
	NumBytes = ::recv((SOCKET)Socket,TempBuf,1,MSG_PEEK);
	if(!NumBytes || (NumBytes == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK))
		return false;
	else
		return true;
}

//	dataSocket::closed

bool disconnectError(int32 Error)
{
	if(Error == WSAECONNRESET || Error == WSAECONNABORTED || Error == WSAETIMEDOUT)
		return true;
	else
		return false;
}

bool dataSocket::closed()
{
	fd_set	SocketSet;
	TIMEVAL	SelectTime = {0, 0};

	FD_ZERO(&SocketSet);
	FD_SET((SOCKET)Socket,&SocketSet);

	int32	Result = ::select(0,&SocketSet,NULL,NULL,&SelectTime);

	if(Result == SOCKET_ERROR)
	{
		if(disconnectError(WSAGetLastError()))
			return true;
		else
			throw Str+"select failed: "+toString((int32)WSAGetLastError());
	}

	if(Result)
	{
		char	TempBuf[256];
		int32	NumBytes;
		NumBytes = ::recv((SOCKET)Socket,TempBuf,1,MSG_PEEK);
		if(!NumBytes || (NumBytes == SOCKET_ERROR && disconnectError(WSAGetLastError())))
			return true;
	}

	return false;
}

//	udpDataSocket::udpDataSocket

udpDataSocket::udpDataSocket(uint32 InSocket):
	Socket(InSocket)
{
	if(Socket != INVALID_SOCKET)
	{
		uint32	NoBlocking = 1;
		if(ioctlsocket((SOCKET)Socket,FIONBIO,&NoBlocking) == SOCKET_ERROR)
			throw Str+"Failed to make socket non-blocking: "+toString((int32)WSAGetLastError());

		uint32	Broadcast = 1;
		if(setsockopt((SOCKET)Socket,SOL_SOCKET,SO_BROADCAST,(char*)&Broadcast,sizeof(Broadcast)) == SOCKET_ERROR)
			throw Str+"Failed to make socket broadcasting: "+toString((int32)WSAGetLastError());
	}
}

//	udpDataSocket::~udpDataSocket

udpDataSocket::~udpDataSocket()
{
	if(Socket != INVALID_SOCKET)
	{
		closesocket((SOCKET)Socket);
		Socket = INVALID_SOCKET;
	}
}

//	udpDataSocket::bind

udpDataSocket* udpDataSocket::bind(const ipAddress& Address)
{
	sockClient	SockClient;
	SOCKET		Socket;

	// Create the socket.

	Socket = ::socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(Socket == INVALID_SOCKET)
		throw Str+"Couldn't create socket for bind: "+toString((int32)WSAGetLastError());

	// Bind the socket.

	sockaddr_in	LocalSockAddr;
	Address.getSockAddr(&LocalSockAddr);
	if(::bind(Socket,(sockaddr*)&LocalSockAddr,sizeof(sockaddr_in)) == SOCKET_ERROR)
		throw Str+"Couldn't bind UDP socket: "+toString((int32)WSAGetLastError());

	return new udpDataSocket((uint32)Socket);
}

//	udpDataSocket::recv

bool udpDataSocket::recv(ipAddress& SourceAddress,void* Data,uintmax DataLength)
{
	sockaddr_in	SourceSockAddr;
    int32		SourceSockAddrLength = sizeof(sockaddr_in);
	int32		Result = ::recvfrom((SOCKET)Socket,(char*)Data,(int)DataLength,0,(sockaddr*)&SourceSockAddr,(int*)&SourceSockAddrLength);

	SourceAddress.Address = ntohl(SourceSockAddr.sin_addr.S_un.S_addr);
	SourceAddress.Port = ntohs(SourceSockAddr.sin_port);

	if(Result == SOCKET_ERROR)
		throw Str+"recv failed: "+toString((int32)WSAGetLastError());
	else
		return Result == DataLength;
}

//	udpDataSocket::partialReceive

uintmax udpDataSocket::partialReceive(void* Data,uintmax DataLength)
{
	int32	Result = ::recv((SOCKET)Socket,(char*)Data,(int)DataLength,0);

	if(Result == SOCKET_ERROR)
		throw Str+"recv failed: "+toString((int32)WSAGetLastError());
	else
		return Result;
}

//	udpDataSocket::sendTo
//

void udpDataSocket::sendTo(const ipAddress& Address,void* Data,uintmax DataLength)
{
	sockaddr_in	RemoteSockAddr;
	Address.getSockAddr(&RemoteSockAddr);
	int32	Result = ::sendto((SOCKET)Socket,(char*)Data,(int)DataLength,0,(sockaddr*)&RemoteSockAddr,sizeof(sockaddr_in));

	if(Result != DataLength)
		throw Str+"sendto failed: "+toString((int32)WSAGetLastError());
}

//	udpDataSocket::recvDataQueued

bool udpDataSocket::recvDataQueued()
{
	fd_set	SocketSet;
	TIMEVAL	SelectTime = {0, 0};

	FD_ZERO(&SocketSet);
	FD_SET((SOCKET)Socket,&SocketSet);

	int32	Result = ::select(0,&SocketSet,NULL,NULL,&SelectTime);

	if(Result == SOCKET_ERROR)
		throw Str+"select failed: "+toString((int32)WSAGetLastError());

	if(Result)
		return true;
	else
		return false;
}

//	listenSocket::listenSocket

listenSocket::listenSocket(const ipAddress& Address)
{
	// Create the socket.

	Socket = ::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(!Socket)
		throw Str+"Couldn't create socket for listen.  GetLastError returned: "+toString((int32)WSAGetLastError());

	// Bind the socket.

	sockaddr_in	SockAddr;
	Address.getSockAddr(&SockAddr);
	if(::bind((SOCKET)Socket,(sockaddr*)&SockAddr,sizeof(sockaddr_in)) == SOCKET_ERROR)
		throw Str+"Couldn't bind socket for listen.  GetLastError returned: "+toString((int32)WSAGetLastError());

	// Start listening on the socket.

	if(::listen((SOCKET)Socket,SOMAXCONN) == SOCKET_ERROR)
		throw Str+"Couldn't listen.  GetLastError returned: "+toString((int32)WSAGetLastError());
}

//	listenSocket::~listenSocket

listenSocket::~listenSocket()
{
	if(Socket != INVALID_SOCKET)
	{
		closesocket((SOCKET)Socket);
		Socket = INVALID_SOCKET;
	}
}

//	listenSocket::getLocalAddress

ipAddress listenSocket::getLocalAddress()
{
	sockaddr_in	LocalSockAddr;
    int32		LocalSockAddrLength = sizeof(sockaddr_in);
	if(getsockname((SOCKET)Socket,(sockaddr*)&LocalSockAddr,(int*)&LocalSockAddrLength) == SOCKET_ERROR)
		throw Str+"getsockname failed: "+toString((int32)WSAGetLastError());

	return ipAddress(ntohl(LocalSockAddr.sin_addr.S_un.S_addr),ntohs(LocalSockAddr.sin_port));
}

//	listenSocket::pendingConnection

bool listenSocket::pendingConnection()
{
	fd_set	SocketSet;
	TIMEVAL	SelectTime = {0, 0};

	FD_ZERO(&SocketSet);
	FD_SET((SOCKET)Socket,&SocketSet);

	int32	Result = ::select(0,&SocketSet,NULL,NULL,&SelectTime);

	if(Result == SOCKET_ERROR)
		throw Str+"select failed: "+toString((int32)WSAGetLastError());

	if(Result)
		return true;
	else
		return false;
}

//	listenSocket::acceptConnection

dataSocket* listenSocket::acceptConnection()
{
	sockaddr_in	AcceptedAddress;
	int32		AcceptedAddressSize = sizeof(sockaddr_in);
	SOCKET		AcceptedSocket = ::accept((SOCKET)Socket,(sockaddr*)&AcceptedAddress,(int*)&AcceptedAddressSize);

	if(AcceptedSocket != INVALID_SOCKET)
		return new dataSocket((uint32)AcceptedSocket);
	else
		return NULL;
}