#ifndef VIRTUAL_MEMORY_HPP
#define VIRTUAL_MEMORY_HPP
#include "x_types.hpp"

#ifndef X_MEGABYTE
#define X_MEGABYTE(x)  ((x)*1048576)
#endif

#ifndef X_KILOBYTE
#define X_KILOBYTE(x)	((x)*1024)
#endif

#define VM_PAGE_SIZE	X_KILOBYTE(4)

#ifndef X_SECTION
#define X_SECTION(x)
#endif

#if defined(bwatson) && !defined(ENABLE_VM_CODE)
#define ENABLE_VM_CODE
#endif

	void		vm_Init(s32 VirtualSize, s32 PoolSize)  X_SECTION(permanent);
	void		vm_Kill(void)                           X_SECTION(permanent);
	void*		vm_Alloc(s32 nBytes)                    X_SECTION(permanent);
	void		vm_Free(void* pAddress)                 X_SECTION(permanent);
    void        vm_DumpList(void)                       X_SECTION(permanent);
    void*       vm_GetSwapSpace(void)                   X_SECTION(permanent);


#endif // VIRTUAL_MEMORY_HPP