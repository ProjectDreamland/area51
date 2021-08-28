Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 12.00.8804 for 80x86
Copyright (C) Microsoft Corp 1984-1998. All rights reserved.

cl /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fo"Release/" /Fd"Release/" /FD /c LoopTest\LoopTest.cpp"

// LoopTest.cpp : Defines the entry point for the console application.
//

#include "stdio.h"
#include "malloc.h"
#include "windows.h"
#include "MMsystem.h"

typedef unsigned int uint;
typedef unsigned __int64 uint64;
DWORD start, stop;
void get_time_start()

{

start = timeGetTime();

}

DWORD get_time_stop()

{

DWORD time;
stop = timeGetTime();
time = DWORD(stop - start);
return time;

}

struct ThreadParams

{

HANDLE hThread;
HANDLE hStart;
HANDLE hEnd;
uint* data;
uint results;
uint id;
uint array_sz;
uint block_sz;
uint iterations;
uint padding[64]; // padding to avoid false sharing

} 

thread_parameters[2];

HANDLE hEvents[2];

void CacheBlocking (uint* results, uint* array, uint ARRAY_SZ, uint BLOCK_SZ, uint ITERATIONS)

{

uint sum = 0;
uint index = 0;
unsigned int i, j;
for (index = 0; index < ARRAY_SZ;) {
uint* data = &array[index];
index += BLOCK_SZ;
if (index > ARRAY_SZ) BLOCK_SZ = ARRAY_SZ - (index - BLOCK_SZ);
for (i = 0; i < ITERATIONS; i++)
for (j = 0; j < BLOCK_SZ; j++)
sum += data[j]+data[j] + ITERATIONS;

}

*results = sum;

}

void NonCacheBlocking (uint* results, uint* array, uint ARRAY_SZ, uint ITERATIONS)

{

uint sum = 0;
unsigned int i, j;
for (i = 0; i < ITERATIONS; i++)
for (j = 0; j < ARRAY_SZ; j++)
sum += array[j]+array[j] + ITERATIONS;
*results = sum;

}

DWORD WINAPI ThreadFunction(void *vThread)

{
struct ThreadParams *p = (struct ThreadParams *)vThread;

// software fix for 64K/1M aliasing

int *stackoffset = (int *)_alloca(512*p->id);
while (1) {
WaitForSingleObject (p->hStart, INFINITE);
CacheBlocking(&(p->results), p->data, p->array_sz,
p->block_sz, p->iterations);
ResetEvent (p->hStart);
SetEvent (p->hEnd);
}

return 0;

}

void InitializeThreads ()

{
thread_parameters[0].hThread = (HANDLE)CreateThread (NULL, // security
0, // default statck_size

(LPTHREAD_START_ROUTINE) ThreadFunction, // function
&(thread_parameters[0]), // arglist
CREATE_SUSPENDED, // initflag
NULL); // threadaddr
thread_parameters[0].hStart = (HANDLE)CreateEvent (NULL, TRUE,
FALSE, NULL);
thread_parameters[0].hEnd = (HANDLE)CreateEvent (NULL, TRUE,
FALSE, NULL);
thread_parameters[0].id = 0;
SetThreadAffinityMask (thread_parameters[0].hThread, 1 << 0);
thread_parameters[1].hThread = (HANDLE)CreateThread (NULL, // security
0, // default statck_size
(LPTHREAD_START_ROUTINE) ThreadFunction, // function
&(thread_parameters[1]), // arglist
CREATE_SUSPENDED, // initflag
NULL); // threadaddr
thread_parameters[1].hStart = (HANDLE)CreateEvent (NULL, TRUE,
FALSE, NULL);
thread_parameters[1].hEnd = (HANDLE)CreateEvent (NULL, TRUE,
FALSE, NULL);
thread_parameters[1].id = 1;
SetThreadAffinityMask (thread_parameters[0].hThread, 1 << 1);
hEvents[0] = thread_parameters[0].hEnd;
hEvents[1] = thread_parameters[1].hEnd;
ResumeThread(thread_parameters[0].hThread);
ResumeThread(thread_parameters[1].hThread);

}

void TimeThreadedCacheBlocking (uint* array, uint ARRAY_SZ, uint BLOCK_SZ, uint ITERATIONS)

{

uint array_sz = ARRAY_SZ/2;
uint sum = 0;
get_time_start();
thread_parameters[0].data = array;
thread_parameters[0].array_sz = array_sz;
thread_parameters[0].block_sz = BLOCK_SZ;
thread_parameters[0].iterations = ITERATIONS;
SetEvent(thread_parameters[0].hStart);
thread_parameters[1].data = &(array[array_sz]);
thread_parameters[1].array_sz = array_sz;
thread_parameters[1].block_sz = BLOCK_SZ;
thread_parameters[1].iterations = ITERATIONS;
SetEvent(thread_parameters[1].hStart);
WaitForMultipleObjects(2,
hEvents,
TRUE,
INFINITE);
ResetEvent(thread_parameters[0].hEnd);
ResetEvent(thread_parameters[1].hEnd);
sum = thread_parameters[0].results + thread_parameters[1].results;
printf ("%u msec\t", get_time_stop());
printf ("Block Size: %u K\t", BLOCK_SZ*sizeof(uint)/1024);
printf ("Results: %u\n", sum);

}

void TimeCacheBlocking (uint* array, uint ARRAY_SZ,
uint BLOCK_SZ, uint ITERATIONS)

{

uint sum = 0;
get_time_start();
CacheBlocking (&sum, array, ARRAY_SZ, BLOCK_SZ, ITERATIONS);
printf ("%u msec\t", get_time_stop());
printf ("Block Size: %u K\t", BLOCK_SZ*sizeof(uint)/1024);
printf ("Results: %u\n", sum);

}

void TimeNonCacheBlocking (uint* array, uint ARRAY_SZ, uint ITERATIONS)

{

uint sum = 0;
get_time_start();
NonCacheBlocking (&sum, array, ARRAY_SZ, ITERATIONS);
printf ("%u msec\t", get_time_stop());
printf ("Block Size: 0 K\t\t");
printf ("Results: %u\n", sum);

}

int main(int argc, char* argv[])

{

uint ITERATIONS=1000;
uint ARRAY_SZ=4096000;
uint* array = (uint*) malloc(sizeof(uint)*ARRAY_SZ);
SetThreadAffinityMask (GetCurrentThread(), 2);
for (unsigned int i = 0; i < ITERATIONS; i++)
for (unsigned int j = 0; j < ARRAY_SZ; j++)
array[j] = 3;
printf ("No Cache Blocking\n");
TimeNonCacheBlocking (array, ARRAY_SZ, ITERATIONS);
printf ("\nSingle Threaded Cache Blocking\n");
TimeCacheBlocking (array, ARRAY_SZ, 204800, ITERATIONS);
TimeCacheBlocking (array, ARRAY_SZ, 136534, ITERATIONS);
TimeCacheBlocking (array, ARRAY_SZ, 117029, ITERATIONS);
TimeCacheBlocking (array, ARRAY_SZ, 102400, ITERATIONS);
TimeCacheBlocking (array, ARRAY_SZ, 68267, ITERATIONS);
TimeCacheBlocking (array, ARRAY_SZ, 34134, ITERATIONS);
TimeCacheBlocking (array, ARRAY_SZ, 25600, ITERATIONS);
TimeCacheBlocking (array, ARRAY_SZ, 12800, ITERATIONS);
TimeCacheBlocking (array, ARRAY_SZ, 6400, ITERATIONS);
TimeCacheBlocking (array, ARRAY_SZ, 3200, ITERATIONS);
TimeCacheBlocking (array, ARRAY_SZ, 1600, ITERATIONS);
InitializeThreads();

printf ("\n2 Threads Cache Blocking\n");
TimeThreadedCacheBlocking (array, ARRAY_SZ, 204800, ITERATIONS);
TimeThreadedCacheBlocking (array, ARRAY_SZ, 136534, ITERATIONS);
TimeThreadedCacheBlocking (array, ARRAY_SZ, 117029, ITERATIONS);
TimeThreadedCacheBlocking (array, ARRAY_SZ, 102400, ITERATIONS);
TimeThreadedCacheBlocking (array, ARRAY_SZ, 68267, ITERATIONS);
TimeThreadedCacheBlocking (array, ARRAY_SZ, 34134, ITERATIONS);
TimeThreadedCacheBlocking (array, ARRAY_SZ, 25600, ITERATIONS);
TimeThreadedCacheBlocking (array, ARRAY_SZ, 12800, ITERATIONS);
TimeThreadedCacheBlocking (array, ARRAY_SZ, 6400, ITERATIONS);
TimeThreadedCacheBlocking (array, ARRAY_SZ, 3200, ITERATIONS);
TimeThreadedCacheBlocking (array, ARRAY_SZ, 1600, ITERATIONS);
free(array);
return 0;
}
