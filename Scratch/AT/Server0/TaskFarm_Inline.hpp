//=========================================================================
//
// TASKFARM_INLINE.HPP
//
//=========================================================================
#ifndef TASKFARM_HPP
//=========================================================================

#define LOG_TASK_FARM 1

#if LOG_TASK_FARM
#define TASK_FARM_LOG   x_printf
#else
inline void             EMPTY_LOG_FUNC( ... ){ };
#define TASK_FARM_LOG   EMPTY_LOG_FUNC
#endif


//=========================================================================
//=========================================================================
//=========================================================================
// TASK_FARM_LINK
//=========================================================================
//=========================================================================
//=========================================================================


inline task_farm_link::task_farm_link() { Clear(); pOwner=NULL; }
inline task_farm_link::~task_farm_link() { ASSERT( (pNext==this) && (pPrev==this) ); }
inline xbool task_farm_link::IsEmpty   ( void ) { return ((pNext==this) && (pPrev==this)); }
inline void  task_farm_link::Clear     ( void ) {pNext=this; pPrev=this;}
inline void  task_farm_link::SetOwner  ( void* apOwner ) {pOwner = apOwner;}
inline void* task_farm_link::GetOwner  ( void ) {return pOwner;}

inline task_farm_link* task_farm_link::GetNext( void ) { return pNext; }
inline task_farm_link* task_farm_link::GetPrev( void ) { return pPrev; }

inline void task_farm_link::InsertAtHead( task_farm_link& Link )
{
    ASSERT( Link.pPrev && Link.pNext );
    pNext = Link.pNext;
    pPrev = &Link;
    pNext->pPrev = this;
    Link.pNext = this;
}

inline void task_farm_link::InsertAtTail( task_farm_link& Link )
{
    ASSERT( Link.pPrev && Link.pNext );
    pPrev = Link.pPrev;
    pNext = &Link;
    pPrev->pNext = this;
    Link.pPrev = this;
}

inline void task_farm_link::Remove( void )
{
    pNext->pPrev = pPrev;
    pPrev->pNext = pNext;
    Clear();
}

inline task_farm_link* task_farm_link::PopHead( void )
{
    task_farm_link* pResult = pNext;
    pResult->Remove();
    return pResult;
}

inline task_farm_link* task_farm_link::PopTail( void )
{
    task_farm_link* pResult = pPrev;
    pResult->Remove();
    return pResult;
}

//=========================================================================
#endif //TASKFARM_HPP
//=========================================================================
