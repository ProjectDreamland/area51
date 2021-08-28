//==============================================================================
//
//  CollisionMgr_Private.hpp
//
//==============================================================================

#if !defined(X_RETAIL) 
    #define CheckCollisions     fn_LOG( __FILE__, __LINE__ ), g_CollisionMgr.fn_CheckCollisions
#else 
    #define CheckCollisions     fn_CheckCollisions
#endif 

//==============================================================================

#ifdef ENABLE_COLLISION_STATS
inline 
void collision_mgr::fn_LOG ( const char* pFileName, s32 LineNumber )
{
    m_pLogFileName = pFileName;
    m_LogLineNumber = LineNumber;
}
#endif // ENABLE_COLLISION_STATS

//==============================================================================

inline 
xbool collision_mgr::IsInIgnoreList ( guid Guid )
{
	for( s32 i = 0 ; i < m_nIgnoredObjects ; i++ )
	{
		if( Guid == m_IgnoreList[i] )
		{
			return TRUE;
		}
	}
	return FALSE;
}

//==============================================================================

inline 
guid collision_mgr::GetMovingObjGuid( void ) const
{
    return m_MovingObjGuid;
}

//==============================================================================

inline
void collision_mgr::SetMaxCollisions( s32 nMaxCollisions )
{
    ASSERT( nMaxCollisions > 0 );

    if( m_nMaxCollisions > MAX_COLLISION_MGR_COLLISIONS )
        nMaxCollisions = MAX_COLLISION_MGR_COLLISIONS;
    
    m_nMaxCollisions = nMaxCollisions;
}

//==============================================================================

inline
const bbox& collision_mgr::GetDynamicBBox( void ) const
{
    return m_DynamicBBoxes[m_ContextInfo.Context];
}

//==============================================================================

inline
const collision_mgr::dynamic_cylinder& collision_mgr::GetDynamicCylinder( void ) const
{
    return m_CylinderInfo[m_ContextInfo.Context];
}

//==============================================================================

inline
void	collision_mgr::AddToIgnoreList( guid Guid )
{
	ASSERT( m_nIgnoredObjects < MAX_IGNORED_OBJECTS );
	m_IgnoreList[ m_nIgnoredObjects ] = Guid;
	m_nIgnoredObjects++;
}

//==============================================================================

inline
void	collision_mgr::ClearIgnoreList( void )
{
	m_nIgnoredObjects = 0;
}
 
//==============================================================================

inline
const collision_mgr::dynamic_ray& collision_mgr::GetDynamicRay( void ) const
{
    return m_RayInfo[0];
}

//==============================================================================

inline
void collision_mgr::CollectPermeables( void )
{
    m_bCollectPermeable = TRUE;
}

//==============================================================================

inline
s32 collision_mgr::GetNPermeables( void )
{
    return m_nPermeables;
}

//==============================================================================

inline
guid collision_mgr::GetPermeableGuid( s32 Index )
{
    ASSERT( (Index>=0) && (Index<m_nPermeables) );
    return m_Permeable[Index];
}

//==============================================================================

inline
s32 CompareCollisions( const void* C1, const void* C2 )
{
    ASSERT( C1 != NULL );
    ASSERT( C2 != NULL );

    f32 T1 = ((collision_mgr::collision*)(C1))->T;
    f32 T2 = ((collision_mgr::collision*)(C2))->T;

    return (T1 > T2) ? 1 : ((T1 < T2) ? -1 : 0);
}

//==============================================================================

inline
void collision_mgr::SortCollisions( void )
{
    if( m_nCollisions > 1 )
    {
        x_qsort( m_Collisions, m_nCollisions, sizeof( collision_mgr::collision ), CompareCollisions );
    }
}

//==============================================================================

inline
void collision_mgr::StartApply( guid      Guid )
{
    //
    // Object is colliding against self
    //
    ASSERT( (Guid==0) || (Guid != m_MovingObjGuid) );

    ASSERT( !m_bApplyStarted ); // EndApply() wasn't called
    m_bApplyStarted = TRUE;
    m_ContextInfo.Context = 0;
    m_ContextInfo.Guid = Guid;

    // transforms do nothing
    m_ContextInfo.L2W.Identity();
    m_ContextInfo.W2L.Identity();
}

//==============================================================================

inline
void collision_mgr::EndApply( void )
{
    ASSERT( m_bApplyStarted ); // StartApply() wasn't called
    m_bApplyStarted = FALSE;
    m_ContextInfo.Context = 0;
    m_ContextInfo.Guid = 0;
}

//==============================================================================

inline
void collision_mgr::UseLowPoly( void )
{
    m_bUseLowPoly = TRUE;
}

//==============================================================================

inline
void collision_mgr::StopOnFirstCollisionFound( void )
{
    m_bStopOnFirstCollisionFound = TRUE;
}

//==============================================================================

inline
void collision_mgr::IgnoreGlass( void )
{
    m_bIgnoreGlass = TRUE;
}

//==============================================================================

inline
void collision_mgr::DoNotRemoveDuplicateGuids( void )
{
    m_bRemoveDuplicateGuids = FALSE;
}

//==============================================================================

inline
void collision_mgr::UsePolyCache( void )
{
    m_bUsePolyCache = TRUE;
}

//==============================================================================



