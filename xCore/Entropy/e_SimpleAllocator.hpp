#ifndef SIMPLE_ALLOCATOR
#define SIMPLE_ALLOCATOR

#include "x_types.hpp"

class simple_allocator
{
public:
                simple_allocator(void);
                ~simple_allocator(void);

    void*       Alloc   (s32 size);
    s32         Free    (void* base);
    void        Validate(void);
    xbool        IsValid    (void* base);
    void        Init    (void* base, s32 size);
    void        Kill    (void);
    void        DumpList(void);
    s32         GetFree (void) { return m_MemFree;}
    void        SanityCheck(void);

private:

    void        GrowCapacity( void );

    enum alloc_state
    {
        AF_FREE=0,          // This header block has never been used
        AF_AVAILABLE,       // This header *should* be in the memory chain
        AF_ALLOCATED,       // This is in the memory chain but also in use
    };

    struct alloc_node
    {
        alloc_node* m_pNext;
        alloc_state m_Status;
        void*       m_Address;
        s32         m_Length;
    };

    s32         m_TotalSize;
    void*       m_pBase;
    s32         m_MemFree;
    s32         m_MemUsed;
    s32         m_AllocCount;
    s32         m_FreeCount;
    alloc_node  m_RootBlock;
    alloc_node* m_pMemoryBlock;
    s32         m_nMemoryBlocks;
    s32         m_nMemoryBlocksAllocated;
    s32         m_LastBlockUsed;
    xbool        m_Initialized;
};


#endif
