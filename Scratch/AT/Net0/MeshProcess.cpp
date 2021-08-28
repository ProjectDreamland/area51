/*
	MeshProcess.cpp
	Mesh processor.
*/

#include <iostream>
using std::cout;
#include <time.h>
#include <conio.h>

#include "..\Core\SHCore.h"
#include "..\DX9Interface\DX9Interface.h"

#include <windows.h>

#include "..\Core\DistributedProcessing.h"

//	verifyFileExists

string verifyFileExists(string Filename)
{
	FILE*	FileHandle = fopen(stringToC<char>(Filename).Data,"rb");

	if(FileHandle)
		fclose(FileHandle);
	else
		throw Str+"Couldn't open \'"+Filename+"\' for reading.";

	return Str+Filename;
}

//	parseOption

bool parseOption(char* Args[],uint32 NumArgs,string Key,string& OutValue)
{
	for(uint32 ArgIndex = 1;ArgIndex < NumArgs;ArgIndex++)
	{
		string	ArgString = Str+Args[ArgIndex];
		if(caps(left(ArgString,Key.num() - 1)) == caps(Key))
		{
			OutValue = right(ArgString,Key.num());
			return true;
		}
	}

	return false;
}

//	textProgress

struct textProgress: progress
{
	string	Description;
	uint32	NumDots;

	time_t	StartTime;

	textProgress()
	{
		Description = Str;
		NumDots = 0;
	}

	~textProgress()
	{
		update(Str,0,0);
	}

	virtual void update(string NewDescription,uint32 Numerator,uint32 Denominator)
	{
		if(Description != NewDescription)
		{
			if(Description != Str)
			{
				if(!Denominator)
					cout << "....";

				cout << "done(" << (time(NULL) - StartTime) << " seconds)\n";
			}
			else
				StartTime = time(NULL);

			cout << stringToC<char>(NewDescription).Data;

			if(!Denominator && NewDescription != Str)
				cout << "....";

			Description = NewDescription;
			NumDots = 0;
		}

		if(Denominator != 0)
		{
			uint32	NewNumDots = Numerator * 8 / Denominator;

			while(NumDots < NewNumDots)
			{
				cout << ".";
				NumDots++;
			};
		}
	}
};

//	applyDetailBumps

normalTexture2D* applyDetailBumps(normalTexture2D& NormalMap,processParameters& Parameters)
{
	normalTexture2D*	FinalNormalMap = new normalTexture2D(NormalMap.Width,NormalMap.Height);

	// Perturb the normals by the bumps.

	for(uint32 Y = 0;Y < NormalMap.Height;Y++)
	{
		for(uint32 X = 0;X < NormalMap.Width;X++)
		{
			real32		A = Parameters.DetailBumpMap[Y * NormalMap.Width + X] * Parameters.DetailBumpScale,
						B = Parameters.DetailBumpMap[Y * NormalMap.Width + ((X + 1) % NormalMap.Width)] * Parameters.DetailBumpScale,
						C = Parameters.DetailBumpMap[((Y + 1) % NormalMap.Height) * NormalMap.Width + X] * Parameters.DetailBumpScale;
			vector<3>	Normal = vec3(
				real32(NormalMap.Data[Y * NormalMap.Width + X].R) / 128.0f - 1.0f,
				real32(NormalMap.Data[Y * NormalMap.Width + X].G) / 128.0f - 1.0f,
				real32(NormalMap.Data[Y * NormalMap.Width + X].B) / 128.0f - 1.0f
				);
			vector<3>	TangentX = normal(vec3(255,0,B - A)),
						TangentY = normal(vec3(0,255,C - A)),
						TangentZ = normal(cross(TangentX,TangentY));

			FinalNormalMap->Data[Y * NormalMap.Width + X].R = (uint8)Clamp<uint32>(
				dot(
					Normal,
					vec3(
						TangentX[0],
						TangentY[0],
						TangentZ[0]
						)
					) * 128.0f + 128.0f,
				0,
				255
				);
			FinalNormalMap->Data[Y * NormalMap.Width + X].G = (uint8)Clamp<uint32>(
				dot(
					Normal,
					vec3(
						TangentX[1],
						TangentY[1],
						TangentZ[1]
						)
					) * 128.0f + 128.0f,
				0,
				255
				);
			FinalNormalMap->Data[Y * NormalMap.Width + X].B = (uint8)Clamp<uint32>(
				dot(
					Normal,
					vec3(
						TangentX[2],
						TangentY[2],
						TangentZ[2]
					)
				) * 128.0f + 128.0f,
				0,
				255
				);
			FinalNormalMap->Data[Y * NormalMap.Width + X].A = NormalMap.Data[Y * NormalMap.Width + X].A;
		}
	}

	return FinalNormalMap;
}

//	meshProcessViewport

struct meshProcessViewport: viewport
{
	static dxDeviceManager	DeviceManager;

	//	Constructor.

	meshProcessViewport(uint32 Width,uint32 Height)
	{
		// Create the D3D device for the viewport.

		DeviceManager.createDevice(this,Width,Height);
	}
};

dxDeviceManager meshProcessViewport::DeviceManager(NULL);

//	createSHM

shTexture2D* createSHM(normalTexture2D* FinalNormalMap,const processParameters& Parameters,const array<mesh*>& Meshes,progress* Progress)
{
	// Build a list of random sample directions.

	array< vector<3> >	SampleDirections;
	SampleDirections.addMany(vec3(0,0,0),Parameters.TransferSampleDirections);

	for(uint32 SampleIndex = 0;SampleIndex < Parameters.TransferSampleDirections;SampleIndex++)
	{
		vector<3>	SampleVector;

		while(1)
		{
			SampleVector = vec3(
				real32(rand()) * (2.0f / (real32)RAND_MAX) - 1.0f,
				real32(rand()) * (2.0f / (real32)RAND_MAX) - 1.0f,
				real32(rand()) * (2.0f / (real32)RAND_MAX) - 1.0f
				);

			real32	Magnitude = magnitude(SampleVector);

			if(Magnitude > 0.1f && Magnitude < 1.0f)
				break;
		};

		SampleDirections[SampleIndex] = normal(SampleVector);
	}

	// Create the spherical harmonic map in chunks of up to MAX_BUFFER_SIZE x MAX_BUFFER_SIZE.

	shTexture2D*			SHM = new shTexture2D(FinalNormalMap->Width,FinalNormalMap->Height,Parameters.TransferCoefficients,calculateNumMips(FinalNormalMap->Width,FinalNormalMap->Height));
	uint32					ChunkWidth = Min<uint32>(FinalNormalMap->Width,MAX_BUFFER_SIZE),
							ChunkHeight = Min<uint32>(FinalNormalMap->Height,MAX_BUFFER_SIZE),
							NumChunksX = (FinalNormalMap->Width + ChunkWidth - 1) / ChunkWidth,
							NumChunksY = (FinalNormalMap->Height + ChunkHeight - 1) / ChunkHeight;
	meshProcessViewport*	Viewport = new meshProcessViewport(ChunkWidth,ChunkHeight);

	for(uint32 ChunkY = 0;ChunkY < NumChunksY;ChunkY++)
	{
		for(uint32 ChunkX  = 0;ChunkX < NumChunksX;ChunkX++)
		{
			// Calculate the SHM for this chunk.

			shTexture2D	ChunkSHM(ChunkWidth,ChunkHeight,Parameters.TransferCoefficients,1);
			calculateDiffuseTransfer(
				Viewport->Device,
				Meshes[Parameters.TransferRenderMesh],
				Meshes[Parameters.TransferDetailMesh],
				vec2(
					ChunkX * ChunkWidth / real32(FinalNormalMap->Width),
					ChunkY * ChunkHeight / real32(FinalNormalMap->Height)
					),
				vec2(
					(ChunkX + 1) * ChunkWidth / real32(FinalNormalMap->Width),
					(ChunkY + 1) * ChunkHeight / real32(FinalNormalMap->Height)
					),
				FinalNormalMap,
				Parameters.TraceDistance,
				SampleDirections,
				&ChunkSHM,
				Progress
				);
			Viewport->Device->flushResources();

			// Copy the chunk's normal map into the output normal map.

			uint32	ChunkTop = ChunkY * ChunkHeight;
			for(uint32 Y = 0;Y < ChunkHeight && ChunkTop + Y < FinalNormalMap->Height;Y++)
			{
				memcpy(
					&SHM->Data[((ChunkTop + Y) * FinalNormalMap->Width + ChunkX * ChunkWidth) * Parameters.TransferCoefficients],
					&ChunkSHM.Data[Y * ChunkWidth * Parameters.TransferCoefficients],
					Min(ChunkWidth,FinalNormalMap->Width - ChunkX * ChunkWidth) * sizeof(real32) * Parameters.TransferCoefficients
					);
			}
		}
	}

	// Close the viewport.

	delete Viewport;
	Viewport = NULL;

	// Kill the seams.

    for(uint32 CoefficientIndex = 0;CoefficientIndex < SHM->NumCoefficients;CoefficientIndex++)
	{
		array<real32>	CoefficientMap;
		CoefficientMap.grow(SHM->Width * SHM->Height);
		for(uint32 Y = 0;Y < SHM->Height;Y++)
			for(uint32 X = 0;X < SHM->Width;X++)
				CoefficientMap *= SHM->Data[(Y * SHM->Width + X) * SHM->NumCoefficients + CoefficientIndex];

		CoefficientMap = expandBorders(CoefficientMap,SHM->Width,SHM->Height,CoefficientMap,0.0f);

		for(uint32 Y = 0;Y < SHM->Height;Y++)
			for(uint32 X = 0;X < SHM->Width;X++)
				SHM->Data[(Y * SHM->Width + X) * SHM->NumCoefficients + CoefficientIndex] = CoefficientMap[Y * SHM->Width + X];
	}

	// Build the SHM mip maps.

	real32*	DestPtr = *SHM->Data + SHM->Width * SHM->Height * SHM->NumCoefficients;
	real32*	SrcPtr = *SHM->Data;
	uint32	LastMipWidth = SHM->Width,
			LastMipHeight = SHM->Height;
	for(uint32 MipIndex = 1;MipIndex < SHM->NumMips;MipIndex++)
	{
		uint32	MipWidth = Max<uint32>(SHM->Width >> MipIndex,1),
				MipHeight = Max<uint32>(SHM->Height >> MipIndex,1);
		for(uint32 Y = 0;Y < MipHeight;Y++)
		{
			real32*	SrcRows[2] = { SrcPtr + Y * 2 * LastMipWidth * SHM->NumCoefficients, SrcPtr + (Y * 2 + 1) * LastMipWidth * SHM->NumCoefficients };
			for(uint32 X = 0;X < MipWidth;X++)
			{
				for(uint32 CoefficientIndex = 0;CoefficientIndex < SHM->NumCoefficients;CoefficientIndex++)
				{
					*DestPtr++ = (SrcRows[0][0] + SrcRows[0][SHM->NumCoefficients] + SrcRows[1][0] + SrcRows[1][SHM->NumCoefficients]) * 0.25f;
					SrcRows[0]++;
					SrcRows[1]++;
				}

				SrcRows[0] += SHM->NumCoefficients;
				SrcRows[1] += SHM->NumCoefficients;
			}
		}
		SrcPtr += LastMipWidth * LastMipHeight * SHM->NumCoefficients;
		LastMipWidth = MipWidth;
		LastMipHeight = MipHeight;
	}

	return SHM;
}

//	shadeTexture2D

struct shadeTexture2D : resourceTexture2D
{
	shTexture2D*	SHM;

	//	Constructor.

	shadeTexture2D(shTexture2D* InSHM)
	{
		SHM = InSHM;
		CacheId = NextUniqueCacheId++;
		Width = SHM->Width;
		Height = SHM->Height;
		NumMips = 1;
		Format = tfA8R8G8B8;
		RenderTarget = false;
		DepthStencil = false;
	}

	//	getData

	virtual void getData(uint32 MipIndex,void* Buffer,uint32 Stride)
	{
		real32*	SrcSH = *SHM->Data;

		for(uint32 Y = 0;Y < Height;Y++)
		{
			rgba32*	DestColor = (rgba32*)((uint8*)Buffer + Stride * Y);
			for(uint32 X = 0;X < Width;X++)
			{
				real32	Value = SHM->MeanVector[0];
				for(uint32 CoefficientIndex = 0;CoefficientIndex < SHM->NumCoefficients;CoefficientIndex++)
					Value += SrcSH[CoefficientIndex] * SHM->OptimizationMatrix[CoefficientIndex * SHM->NumUnoptimizedCoefficients];

				*DestColor++ = rgba32(
					uint8(Clamp<int32>(Value * 255.0f,0,255)),
					uint8(Clamp<int32>(Value * 255.0f,0,255)),
					uint8(Clamp<int32>(Value * 255.0f,0,255)),
					0
					);
				SrcSH += SHM->NumCoefficients;
			}
		}
	}
};

//	dataServer

struct dataServer
{
protected:

	mutex		JobMutex;
	array<job*>	IdleJobs;
	array<job*>	ActiveJobs;
	array<job*>	CompletedJobs;

	rwMutex			DataMessageMutex;
	messageWriter	DataMessageWriter;

	// getIdleJob - Assumes JobMutex is held by the caller!

	job* getIdleJob(const array<job*>& ExcludeJobs)
	{
		if(IdleJobs.num())
		{
			job*	Job = IdleJobs[0];
			Job->ActiveCount++;
			IdleJobs = IdleJobs.remove(0);
			ActiveJobs *= Job;
			return Job;
		}
		else
		{
			job*	BestJob = NULL;
			for(uint32 JobIndex = 0;JobIndex < ActiveJobs.num();JobIndex++)
				if(!BestJob || ActiveJobs[JobIndex]->ActiveCount < BestJob->ActiveCount)
					BestJob = ActiveJobs[JobIndex];

			if(BestJob)
				BestJob->ActiveCount++;

			return BestJob;
		}
	}

	//	remoteProcessConnection

	friend struct remoteProcessConnection;

	struct remoteProcessConnection: connection, thread
	{
	private:

		array<job*>	ActiveJobs;
		dataServer*	Server;
		uint32		NumCompletedJobs;

		mutex		FinishMutex;
		bool		FinishedSendingData;

	public:
		
		remoteProcessConnection(dataServer* InServer,dataSocket* InSocket):
			Server(InServer),
			NumCompletedJobs(0),
			connection(InSocket),
			FinishedSendingData(false)
		{
			cout << "Opened connection to process server " << stringToC<char>(Socket->getRemoteAddress().describe()).Data << "\n";
			start();
		}

		~remoteProcessConnection()
		{
			cout << "Closed connection to process server " << stringToC<char>(Socket->getRemoteAddress().describe()).Data << "(" << NumCompletedJobs << " jobs executed)\n";

			mutexLock	MutexLock(Server->JobMutex);
			for(uint32 JobIndex = 0;JobIndex < ActiveJobs.num();JobIndex++)
			{
				job*	ActiveJob = ActiveJobs[JobIndex];
				
				Server->ActiveJobs = Server->ActiveJobs.removeItem(ActiveJob);
				ActiveJob->ActiveCount--;

				if(!ActiveJob->Completed)
					Server->IdleJobs *= ActiveJob;
			}
		}

		virtual void processMessage(messageReader& Reader)
		{
			switch(Reader.MessageType)
			{
				case msgRequestJob:
				{
					FinishMutex.lock();
					FinishedSendingData = true;
					FinishMutex.unlock();

					mutexLock	MutexLock(Server->JobMutex);
					job*		Job = Server->getIdleJob(ActiveJobs);
					if(Job)
					{
						cout << stringToC<char>(Socket->getRemoteAddress().describe()).Data << ": Job sent\n";
						ActiveJobs *= Job;

						messageWriter	Writer(msgJob);
						Writer << *Job;
						sendMessage(Writer.Message);
					}
					break;
				}
				case msgJobResult:
				{
					mutexLock	MutexLock(Server->JobMutex);
					job*		JobResult = new job;
					Reader << *JobResult;

					for(uint32 JobIndex = 0;JobIndex < ActiveJobs.num();JobIndex++)
					{
						job*	ActiveJob = ActiveJobs[JobIndex];
						if(ActiveJob->MinX == JobResult->MinX && ActiveJob->MinY == JobResult->MinY)
						{
							if(!ActiveJob->Completed)
								Server->CompletedJobs *= JobResult;

							ActiveJobs = ActiveJobs.remove(JobIndex--);
							Server->ActiveJobs = Server->ActiveJobs.removeItem(ActiveJob);
							if(--ActiveJob->ActiveCount == 0)
								delete ActiveJob;
							else
								ActiveJob->Completed = true;
							break;
						}
					}

					cout << stringToC<char>(Socket->getRemoteAddress().describe()).Data << ": Job done(" << (uint32)Server->getCompletedJobs() << "/" << (uint32)Server->getTotalJobs() << " done)\n";
					NumCompletedJobs++;

					break;
				}
				default:
					connection::processMessage(Reader);
					break;
			};
		}

		virtual uint32 threadProc()
		{
			// Send the process data.

			Server->DataMessageMutex.readLock();
			sendMessage(Server->DataMessageWriter.Message);
			Server->DataMessageMutex.readUnlock();

			while(tick())
				Sleep(TIMEOUT);

			return 0;
		}

		bool finishedSendingData()
		{
			mutexLock	MutexLock(FinishMutex);
			return FinishedSendingData;
		}
	};

	listenSocket*					ListenSocket;
	array<remoteProcessConnection*>	ProcessConnections;
	time_t							LastHelpRequestTime;

public:

	// Constructor.

	dataServer(processData* Data):
		DataMessageWriter(msgProcessData),
		LastHelpRequestTime(0)
	{
		ListenSocket = new listenSocket(ipAddress(ipAddress::Any,0));

		// Build the process data message.

		DataMessageMutex.writeLock();
		Data->serialize(DataMessageWriter);
		DataMessageMutex.writeUnlock();

		// Build a job list.

		JobMutex.lock();
		for(uint32 Y = 0;Y < Data->SizeY;Y += JOB_SIZE / Data->SuperSampling)
			for(uint32 X = 0;X < Data->SizeX;X += JOB_SIZE / Data->SuperSampling)
				IdleJobs *= new job(X,Y,Min(X + JOB_SIZE / Data->SuperSampling - 1,Data->SizeX - 1),Min(Y + JOB_SIZE / Data->SuperSampling- 1,Data->SizeY - 1));
		JobMutex.unlock();
	}

	~dataServer()
	{
		for(uint32 ConnectionIndex = 0;ConnectionIndex < ProcessConnections.num();ConnectionIndex++)
			delete ProcessConnections[ConnectionIndex];

		delete ListenSocket;
		ListenSocket = NULL;
	}

	uintmax getTotalJobs() { mutexLock	MutexLock(JobMutex); return IdleJobs.num() + ActiveJobs.num() + CompletedJobs.num(); }
	uintmax getCompletedJobs() { mutexLock	MutexLock(JobMutex); return CompletedJobs.num(); }

	// getNormalMap - Put the pieces of the normal map back together.

	void getNormalMap(array<vector<4> >& NormalMapData,uint32 NormalMapWidth)
	{
		mutexLock	MutexLock(JobMutex);

		for(uint32 JobIndex = 0;JobIndex < CompletedJobs.num();JobIndex++)
		{
			job*	Job = CompletedJobs[JobIndex];
			uint32	JobSizeX = (Job->MaxX - Job->MinX) + 1,
					JobSizeY = (Job->MaxY - Job->MinY) + 1;

			for(uint32 Y = Job->MinY;Y <= Job->MaxY;Y++)
				for(uint32 X = Job->MinX;X <= Job->MaxX;X++)
					NormalMapData[NormalMapWidth * Y + X] = Job->NormalMap[JobSizeX * (Y - Job->MinY) + (X - Job->MinX)];
		}
	}

	// getTexture - Put the pieces of the generated texture back together.

	void getTexture(rawTexture2D& Texture)
	{
		mutexLock	MutexLock(JobMutex);

		for(uint32 JobIndex = 0;JobIndex < CompletedJobs.num();JobIndex++)
		{
			job*	Job = CompletedJobs[JobIndex];
			uint32	JobSizeX = (Job->MaxX - Job->MinX) + 1,
					JobSizeY = (Job->MaxY - Job->MinY) + 1;

			for(uint32 Y = Job->MinY;Y <= Job->MaxY;Y++)
				for(uint32 X = Job->MinX;X <= Job->MaxX;X++)
					Texture.Data[Texture.Width * Y + X] = Job->Texture[JobSizeX * (Y - Job->MinY) + (X - Job->MinX)];
		}
	}

	void tick()
	{
		if(getCompletedJobs() < getTotalJobs() && time(NULL) - LastHelpRequestTime >= 10)
		{
			// Broadcast a help request.

			udpDataSocket*	HelpSocket = udpDataSocket::bind(ipAddress(ipAddress::Any,0));
			uint16	LocalDataPort = ListenSocket->getLocalAddress().Port;
			HelpSocket->sendTo(ipAddress(ipAddress::Broadcast,HELP_PORT),&LocalDataPort,sizeof(uint16));
			delete HelpSocket;

			LastHelpRequestTime = time(NULL);
		}

		// For each connection...

		uint32	NumPendingConnections = 0;

		for(uint32 ConnectionIndex = 0;ConnectionIndex < ProcessConnections.num();ConnectionIndex++)
		{
			// Check if the connection has been closed.

			if(!ProcessConnections[ConnectionIndex]->isRunning())
			{
				delete ProcessConnections[ConnectionIndex];
				ProcessConnections = ProcessConnections.remove(ConnectionIndex--);
				continue;
			}

			// Check if the connection has been sent the process data yet.

			if(!ProcessConnections[ConnectionIndex]->finishedSendingData())
				NumPendingConnections++;
		}

		// Check for new connections.

		while(ListenSocket->pendingConnection())
		{
			if(NumPendingConnections < MAX_PENDING_CONNECTIONS)
			{
				ProcessConnections *= new remoteProcessConnection(this,ListenSocket->acceptConnection());
				NumPendingConnections++;
			}
			else
				break;
		};
	}
};

//	main

void main(int NumArgs,char* Args[])
{
	try
	{
		cout << "MeshProcess\n";

		if(NumArgs < 2)
		{
			cout << "Usage: MeshProcess <command>\n";
			cout << "Commands: Client CreateShadeMap\n";
			return;
		}

		if(stricmp(Args[1],"Client") == 0 || stricmp(Args[1],"CreateFloatingPointNormalMap") == 0)
		{
			if(NumArgs < 3)
			{
				cout << "Usage: MeshProcess Client <data file>\n";
				return;
			}

			textProgress	Progress;
			time_t			StartTime = time(NULL);
			string			InputFilename = verifyFileExists(Str+Args[2]);

			// Load the input data.

			fileArchive*		File = new fileArchive(InputFilename,true);
			processParameters	Parameters;
			Parameters.serialize(*File);
			delete File;

			// Build the meshes.

			array<mesh*>	Meshes;

			for(uint32 MeshIndex = 0;MeshIndex < Parameters.InputMeshes.num();MeshIndex++)
				Meshes *= buildMesh(&Parameters.InputMeshes[MeshIndex],&Progress);

			processData	ProcessData(
				Meshes[Parameters.NormalMapRenderMesh],
				Meshes[Parameters.NormalMapDetailMesh],
				Parameters.TraceDistance,
				Parameters.NormalMapSuperSampling,
				Parameters.NormalMapWidth,
				Parameters.NormalMapHeight,
				Parameters.UseDetailTexture ? &Parameters.DetailTexture : NULL
				);

			// Create the normal map and texture.

			array< vector<4> >	NormalMapData;
			rawTexture2D	Texture(Parameters.NormalMapWidth,Parameters.NormalMapHeight);

			{
				// Start the data server.

				dataServer	DataServer(&ProcessData);

				// Wait for the process servers to finish processing the normal map.

				while(1)
				{
					if(DataServer.getCompletedJobs() == DataServer.getTotalJobs())
						break;

					DataServer.tick();
					Sleep(TIMEOUT);
				};

				NormalMapData.addMany(vec4(0,0,0,0),Parameters.NormalMapWidth * Parameters.NormalMapHeight);
				DataServer.getNormalMap(NormalMapData,Parameters.NormalMapWidth);
				DataServer.getTexture(Texture);
			}

			// Expand the borders of the texture.

			Texture.Data = expandBorders<rgba32,vector<4> >(Texture.Data,Texture.Width,Texture.Height,NormalMapData,vec4(0,0,0,0));

			// Expand the borders of the normal map to keep the unmapped grey out of mipmaps.

			NormalMapData = expandBorders<vector<4>,vector<4> >(NormalMapData,Parameters.NormalMapWidth,Parameters.NormalMapHeight,NormalMapData,vec4(0,0,0,0));

			// Quantize and pack the floating point normalm map to a RGBA32 texture.

			normalTexture2D		NormalMap(Parameters.NormalMapWidth,Parameters.NormalMapHeight);

			for(uint32 Index = 0;Index < NormalMapData.num();Index++)
				NormalMap.Data[Index] = rgba32(
					(uint8)Clamp<uint32>(NormalMapData[Index][0] * 128.0f + 128.0f,0,255),
					(uint8)Clamp<uint32>(NormalMapData[Index][1] * 128.0f + 128.0f,0,255),
					(uint8)Clamp<uint32>(NormalMapData[Index][2] * 128.0f + 128.0f,0,255),
					(uint8)Clamp<uint32>(NormalMapData[Index][3] * 128.0f + 128.0f,0,255)
					);

			// Apply the detail bump map to the normal map.

			normalTexture2D*	FinalNormalMap = &NormalMap;

			if(Parameters.DetailBumpMap.num())
				FinalNormalMap = applyDetailBumps(NormalMap,Parameters);

			if(stricmp(Args[1],"CreateFloatingPointNormalMap") == 0)
			{
				fileArchive	NormalMapFile(Args[3],false);
				for(uint32 Index = 0;Index < NormalMapData.num();Index++)
					NormalMapFile << NormalMapData[Index];
			}
			else
			{
				// Save the normal map.

				Progress.update(Str+"Saving normal map",0,0);

				saveTGA(Parameters.NormalMapOutputFilename,FinalNormalMap);
				saveTGA(Parameters.TextureMapOutputFilename,&Texture);

				Progress.update(Str,0,0);

				if(Parameters.ProcessSHM)
				{
					// Create the SHM.

					shTexture2D*	SHM = createSHM(FinalNormalMap,Parameters,Meshes,&Progress);
					saveSHM(Parameters.UnoptimizedOutputFilename,SHM);

					// Optimize the SHM.

					if(Parameters.NumOptimizedCoefficients != Parameters.TransferCoefficients && Parameters.OptimizedOutputFilename != Str)
					{
						shTexture2D*	OptimizedSHM = new shTexture2D(SHM->Width,SHM->Height,Parameters.NumOptimizedCoefficients,SHM->NumMips,SHM->NumCoefficients);
						optimizeSHM(SHM,OptimizedSHM,&Progress);
						saveSHM(Parameters.OptimizedOutputFilename,OptimizedSHM);
					}
				}
			}

			// Display the processing time.

			time_t	EndTime = time(NULL);
			cout << "Completed in " << ((EndTime - StartTime) / 60) << "m " << ((EndTime - StartTime) % 60) << "s.  Press a key to continue.\n";
			_getch();
		}
		else if(stricmp(Args[1],"CreateShadeMap") == 0)
		{
			if(NumArgs < 4)
			{
				cout << "Usage: MeshProcess CreateShadeMap <input .SHM> <output .TGA>\n";
				return;
			}

			string	ShadeMapFilename = Str+Args[3],
					SHMFilename = verifyFileExists(Str+Args[2]);

			// Load the SHM.

			shTexture2D*	SHM = loadSHM(SHMFilename);

			// Output the shade map.

			shadeTexture2D	ShadeMap(SHM);
			saveTGA(ShadeMapFilename,&ShadeMap);
		}
		else if(stricmp(Args[1],"DumpRawSHM") == 0)
		{
			if(NumArgs < 4)
			{
				cout << "Usage: MeshProcess CreateShadeMap <input .SHM> <output .RAW>\n";
				return;
			}

			string	RawFilename = Str+Args[3],
					SHMFilename = verifyFileExists(Str+Args[2]);

			// Load the SHM.

			shTexture2D*	SHM = loadSHM(SHMFilename);

			// Dump the raw SHM coefficients to file.

			fileArchive	RawFile(RawFilename,false);

			for(uint32 Y = 0;Y < SHM->Height;Y++)
				for(uint32 X = 0;X < SHM->Width;X++)
					for(uint32 CoefficientIndex = 0;CoefficientIndex < SHM->NumCoefficients;CoefficientIndex++)
						RawFile << SHM->Data[(Y * SHM->Width + X) * SHM->NumCoefficients + CoefficientIndex];
		}

		cout << "\n";
	}
	catch(string Error)
	{
		cout << "Error:\n\n" << stringToC<char>(Error).Data << "\n";
	}
}