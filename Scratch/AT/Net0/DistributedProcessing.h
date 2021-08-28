/*
	DistributedProcessing.h
	Distributed processing code.
*/

// Constants.

#define MAX_BUFFER_SIZE			1024
#define TIMEOUT					20
#define HELP_PORT				20007
#define MESSAGE_PACKET_SIZE		1024*1024
#define JOB_SIZE				128
#define MAX_PENDING_CONNECTIONS 5

//	mutex

struct mutex
{
private:

	CRITICAL_SECTION	CriticalSection;

public:

	mutex()
	{
		InitializeCriticalSection(&CriticalSection);
	}

	~mutex()
	{
		DeleteCriticalSection(&CriticalSection);
	}

	void lock()
	{
		EnterCriticalSection(&CriticalSection);
	}

	void unlock()
	{
		LeaveCriticalSection(&CriticalSection);
	}
};

//	rwMutex

struct rwMutex: protected mutex
{
private:

	volatile bool	WriteLock;
	volatile uint32	ReadLocks;

public:

	rwMutex(): WriteLock(false), ReadLocks(0) {}
	~rwMutex() {}

	void readLock()
	{
		while(1)
		{
			lock();
			if(!WriteLock)
			{
				ReadLocks++;
				unlock();
				return;
			}
			unlock();
			Sleep(TIMEOUT);
		};
	}

	void readUnlock()
	{
		lock();
		ReadLocks--;
		unlock();
	}

	void writeLock()
	{
		while(1)
		{
			lock();
			if(!WriteLock && !ReadLocks)
			{
				WriteLock = true;
				unlock();
				return;
			}
			unlock();
			Sleep(TIMEOUT);
		};
	}

	void writeUnlock()
	{
		lock();
		WriteLock = false;
		unlock();
	}
};

//	mutexLock

struct mutexLock
{
	mutex&	Mutex;

	mutexLock(mutex& InMutex):
		Mutex(InMutex)
	{
		Mutex.lock();
	}

	~mutexLock()
	{
		Mutex.unlock();
	}
};

//	thread

struct thread
{
private:

	mutex			Mutex;
	HANDLE			Handle;
	DWORD			ThreadId;
	volatile bool	Running;

	static DWORD StaticThreadProc(LPVOID Parameter);

public:

	thread()
	{
		Mutex.lock();
		Handle = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)StaticThreadProc,this,CREATE_SUSPENDED,&ThreadId);
		if(Handle == INVALID_HANDLE_VALUE)
			throw Str+"CreateThread failed: "+toString((uint32)GetLastError());
		Running = true;
		Mutex.unlock();
	}

	~thread()
	{
		if(isRunning())
			TerminateThread(Handle,0);

		CloseHandle(Handle);
	}

	void start()
	{
		ResumeThread(Handle);
	}

	bool isRunning()
	{
		mutexLock	MutexLock(Mutex);
		return Running;
	}

	virtual uint32 threadProc() = 0;
};

DWORD thread::StaticThreadProc(LPVOID Parameter)
{
	try
	{
		thread*	Thread = (thread*)Parameter;
		uint32	Result = Thread->threadProc();
		Thread->Mutex.lock();
		Thread->Running = false;
		Thread->Mutex.unlock();
		return (DWORD)Result;
	}
	catch(string Error)
	{
		cout << "Error in thread:\n\n" << stringToC<char>(Error).Data << "\n";
		return 0;
	}
}

//	processData

struct processData
{
public:

	mesh*			RenderMesh;
	mesh*			DetailMesh;
	real32			TraceDistance;
	uint32			SuperSampling,
					SizeX,
					SizeY;
	rawTexture2D	DetailTexture;

	// Constructors.

	processData(): RenderMesh(NULL), DetailMesh(NULL) {}

	processData(mesh* InRenderMesh,mesh* InDetailMesh,real32 InTraceDistance,uint32 InSuperSampling,uint32 InSizeX,uint32 InSizeY,resourceTexture2D* InDetailTexture):
		RenderMesh(InRenderMesh),
		DetailMesh(InDetailMesh),
		TraceDistance(InTraceDistance),
		SuperSampling(InSuperSampling),
		SizeX(InSizeX),
		SizeY(InSizeY),
		DetailTexture(0,0)
	{
		if(InDetailTexture)
		{
			DetailTexture = rawTexture2D(InDetailTexture->Width,InDetailTexture->Height);
			InDetailTexture->getData(0,*DetailTexture.Data,DetailTexture.Width * sizeof(rgba32));
			DetailTexture.Revision++;
		}
	}

	~processData()
	{
		delete RenderMesh;
		delete DetailMesh;
	}

	virtual void serialize(archive& Ar)
	{
		if(Ar.isLoading())
		{
			RenderMesh = new mesh;
			DetailMesh = new mesh;
		}
		RenderMesh->serialize(Ar);
		DetailMesh->serialize(Ar);
		Ar << TraceDistance;
		Ar << SuperSampling;
		Ar << SizeX;
		Ar << SizeY;
		Ar << DetailTexture;
	}
};

//	job

struct job
{
	volatile uint32		MinX,
						MinY,
						MaxX,
						MaxY;
	array< vector<4> >	NormalMap;
	array<rgba32>		Texture;
	volatile uint32		ActiveCount;
	volatile bool		Completed;

	job():
		ActiveCount(0),
		Completed(false)
	{}

	job(uint32 InMinX,uint32 InMinY,uint32 InMaxX,uint32 InMaxY):
		MinX(InMinX),
		MinY(InMinY),
		MaxX(InMaxX),
		MaxY(InMaxY),
		ActiveCount(0),
		Completed(false)
	{}

	uint32 sizeX() const { return (MaxX - MinX) + 1; }
	uint32 sizeY() const { return (MaxY - MinY) + 1; }

	void execute(processData* Data)
	{
		uint32	SizeX = (MaxX - MinX) + 1,
				SizeY = (MaxY - MinY) + 1;
		NormalMap.addMany(vec4(0,0,0,0),SizeX * SizeY);
		Texture.addMany(rgba32(127,127,127,0),SizeX * SizeY);

		calculateNormalMap(
			Data->RenderMesh,
			Data->DetailMesh,
			Data->TraceDistance,
			Data->SuperSampling,
			NormalMap,
			Texture,
			Data->SizeX,
			Data->SizeY,
			MinX,
			MinY,
			MaxX,
			MaxY,
			Data->DetailTexture.Revision ? &Data->DetailTexture : NULL,
			&progress::DummyProgress
			);
	}

	friend void operator<<(archive& Ar,job& J)
	{
		Ar << J.MinX;
		Ar << J.MinY;
		Ar << J.MaxX;
		Ar << J.MaxY;
		Ar << J.NormalMap;
		Ar << J.Texture;
	}
};

//	message

enum
{
	msgProcessData,	// Data server -> process server
	msgRequestJob,	// Process server -> data server
	msgJob,			// Data server -> process server
	msgJobResult,	// Process server -> data server
};

struct message
{
	uint32			DataSize;
	array<uint8>	Data;
};

//	messageWriter

struct messageWriter: archive
{
	message	Message;

	messageWriter(uint8 MessageType)
	{
		*this << MessageType;
	}

	virtual void growthHint(uintmax NumBytes)
	{
		Message.Data.grow(NumBytes);
	}

	virtual void serialize(volatile void* Data,uintmax Size)
	{
		for(uintmax Index = 0;Index < Size;Index++)
			Message.Data *= (uint8)((volatile uint8*)Data)[Index];
	}

	virtual bool isLoading() { return false; }
};

//	messageReader

struct messageReader: archive
{
	message*	Message;
	uintmax		FirstIndex;
	uint8		MessageType;

	messageReader(message* InMessage):
		Message(InMessage),
		FirstIndex(0)
	{
		*this << MessageType;
	}

	virtual void serialize(volatile void* Data,uintmax Size)
	{
		if(FirstIndex >= Message->DataSize)
			throw Str+"Message read past end of data at "+Message->DataSize+" bytes";

		for(uintmax Index = 0;Index < Size;Index++)
			((volatile uint8*)Data)[Index] = Message->Data[FirstIndex + Index];
		FirstIndex += Size;
	}

	virtual bool isLoading() { return true; }
};

//	connection

struct connection
{
protected:

	dataSocket*	Socket;
	message*	CurrentMessage;

	mutex		SendMutex;	// Only one thread can be sending a message at a time.

public:

	connection(dataSocket* InSocket):
		Socket(InSocket),
		CurrentMessage(NULL)
	{}

	virtual ~connection()
	{
		delete Socket;
	}

	bool closed() { return Socket->closed(); }

	void sendMessage(message& Message)
	{
		if(!closed())
		{
			try
			{
				mutexLock	MutexLock(SendMutex);

				Message.DataSize = (uint32)Message.Data.num();
				Socket->send(&Message.DataSize,sizeof(uint32));

				uintmax	BytesSent = 0;
				while(BytesSent < Message.Data.num())
				{
					uintmax	Size = Min<uintmax>(Message.Data.num() - BytesSent,MESSAGE_PACKET_SIZE);
					Socket->send(&Message.Data[BytesSent],Size);
					BytesSent += Size;
				};
			}
			catch(...)
			{
				// Catch a closed socket.
			}
		}
	}

	virtual void processMessage(messageReader& Reader)
	{
		throw Str+"Unhandled message type: "+toString((uint32)Reader.MessageType);
	}

	bool tick()
	{
		while(!Socket->closed() && Socket->recvDataQueued())
		{
			if(!CurrentMessage)
			{
				CurrentMessage = new message;
				if(!Socket->recv(&CurrentMessage->DataSize,sizeof(uint32)))
				{
					delete CurrentMessage;
					CurrentMessage = NULL;
				}
				CurrentMessage->Data.grow(CurrentMessage->DataSize);
			}
			else
			{
				if(CurrentMessage->Data.num() < CurrentMessage->DataSize)
				{
					uintmax	BlockSize = Min<uintmax>(MESSAGE_PACKET_SIZE,CurrentMessage->DataSize - CurrentMessage->Data.num());
					uint8*	Block = new uint8[BlockSize];
					uintmax	NumBytes = Socket->partialReceive(Block,BlockSize);

					CurrentMessage->Data.grow(NumBytes);
					for(uintmax Index = 0;Index < NumBytes;Index++)
						CurrentMessage->Data *= Block[Index];

					delete Block;
				}

				if(CurrentMessage->Data.num() == CurrentMessage->DataSize)
				{
					processMessage(messageReader(CurrentMessage));
					delete CurrentMessage;
					CurrentMessage = NULL;
				}
			}
		};

		return !Socket->closed();
	}
};
