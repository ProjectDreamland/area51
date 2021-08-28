/*
	Net.h
	Networking utilities.
*/

//	sockClient

struct sockClient
{
	static uint32	NumReferences;

	sockClient();
	~sockClient();
};

//	ipAddress

struct ipAddress
{
	static uint32	Any;
	static uint32	Loopback;
	static uint32	Broadcast;

	uint32	Address;
	uint16	Port;

	// Constructor.

	ipAddress() {}
	ipAddress(uint32 InAddress,uint16 InPort):
		Address(InAddress),
		Port(InPort)
	{}

	void getSockAddr(struct sockaddr_in* Result) const;

	string describe();
};

//	dataSocket

struct dataSocket
{
private:

	sockClient	SockClient;
	uint32		Socket;

public:

	dataSocket(uint32 InSocket);
	virtual ~dataSocket();

	static dataSocket* connect(ipAddress Address);

	ipAddress getRemoteAddress();

	bool recv(void* Data,uintmax DataLength);
	uintmax partialReceive(void* Data,uintmax DataLength);
	void send(void* Data,uintmax DataLength);

	bool recvDataQueued();
	bool closed();
};

//	udpDataSocket

struct udpDataSocket
{
private:
	
	sockClient	SockClient;
	uint32		Socket;

public:

	udpDataSocket(uint32 InSocket);
	virtual ~udpDataSocket();

	static udpDataSocket* bind(const ipAddress& Address);

	bool recv(ipAddress& SourceAddress,void* Data,uintmax DataLength);
	uintmax partialReceive(void* Data,uintmax DataLength);
	void sendTo(const ipAddress& Address,void* Data,uintmax DataLength);

	bool recvDataQueued();
};

//	listenSocket

struct listenSocket
{
private:

	sockClient	SockClient;
	uint32		Socket;

public:

	listenSocket(const ipAddress& Address);
	virtual ~listenSocket();

	ipAddress getLocalAddress();

	bool pendingConnection();
	dataSocket* acceptConnection();
};
