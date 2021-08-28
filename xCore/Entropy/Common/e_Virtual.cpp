#include "x_files.hpp"
#include "e_virtual.hpp"

// BIG NOTE! This define needs to be changed as each individual platform specific
// virtual memory handler is implemented. This is a default handler that will work
// in a generic manner by just allocating and deallocating using x_malloc and x_free
#if defined(TARGET_GCN)
#error "This implementation is the default VM allocator. Please exclude from your build rules for this target."
#endif
// Default virtual memory allocation routines assuming no
// virtual memory is actually present.


//-----------------------------------------------------------------------------
void vm_Init(s32 VirtualSize, s32 PoolSize)
{
	(void)PoolSize;
    (void)VirtualSize;
}

//-----------------------------------------------------------------------------
void vm_Kill(void)
{
}

//-----------------------------------------------------------------------------
void* vm_Alloc(s32 nBytes)
{
	return x_malloc(nBytes);
}

//-----------------------------------------------------------------------------
void vm_Free(void* pAddress)
{
	x_free(pAddress);
}

//-----------------------------------------------------------------------------
void vm_DumpList(void)
{

}

//-----------------------------------------------------------------------------
void* vm_GetSwapSpace(void)
{
    return NULL;
}