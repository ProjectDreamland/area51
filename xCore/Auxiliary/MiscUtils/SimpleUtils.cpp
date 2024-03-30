///////////////////////////////////////////////////////////////////////////////
//
//  SimpleUtils.cpp
//
//      Small utility functions
//
///////////////////////////////////////////////////////////////////////////////

#include "..\MiscUtils\SimpleUtils.hpp"
#include "Obj_mgr\obj_mgr.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Objects\Player.hpp"
#include "Characters\Character.hpp"
#include "objects\Turret.hpp"
#include <x_stdio.hpp>
#include "Entropy.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "NetworkMgr\Networkmgr.hpp"

//=============================================================================

void    SMP_UTIL_StrSafeCpy( char* Dest, const char* Src, s32 MaxLen )
{
    s32 Len = x_strlen( Src ) + 1; // plus 1 for x_strsavecpy
 
    ASSERT( Len < MaxLen );

    //concatenate so we don't write over memory..
    if (Len > MaxLen)
        Len = MaxLen-1;
    
    //ODD NOTE : x_strsavecpy expects strlen + 1 for some reason, otherwise it drops the last
    //character. Proably involves the way it interpets the end of line char..

    if (Len==0)
        Dest[0] = '\0';
    else
        x_strsavecpy( Dest, Src, Len ); 
}

//=============================================================================

xbool   SMP_UTIL_FileExist( const char* File )
{
    X_FILE* pFile = x_fopen( File, "rb" );
    
    if (pFile == NULL)
        return FALSE;
    
    x_fclose( pFile );
    
    return TRUE;
}

//=============================================================================

void SMP_UTIL_draw_Polygon( 
                                const vector3*    pPoint,
                                s32               NPoints,
                                xcolor            Color,
                                xbool             DoWire )
{
    ( void ) DoWire;
    
    s32 P = NPoints-1;
    draw_Begin(DRAW_LINES);
    draw_Color(Color);
    for( s32 i=0; i<NPoints; i++ )
    {
        draw_Vertex(pPoint[P]);
        draw_Vertex(pPoint[i]);
        P = i;
    }
    draw_End();
}

//=============================================================================

void SMP_UTIL_draw_Cube(   const vector3&   Halves,
                           const matrix4&   Matrix,
                           xcolor           Color )
{
    const s32   NumPoints   = 8;
    s16  Index[]     = {0,1,1,2,2,3,3,0,4,5,5,6,6,7,7,4,0,4,1,5,2,6,3,7};
    vector3     vPoints[NumPoints];
    
    vPoints[0] = vector3(     Halves.GetX(),      Halves.GetY(),     Halves.GetZ());
    vPoints[1] = vector3(     Halves.GetX(),     -Halves.GetY(),     Halves.GetZ());
    vPoints[2] = vector3(    -Halves.GetX(),     -Halves.GetY(),     Halves.GetZ());
    vPoints[3] = vector3(    -Halves.GetX(),      Halves.GetY(),     Halves.GetZ());
    
    vPoints[4] = vector3(     Halves.GetX(),      Halves.GetY(),     -Halves.GetZ());
    vPoints[5] = vector3(     Halves.GetX(),     -Halves.GetY(),     -Halves.GetZ());
    vPoints[6] = vector3(    -Halves.GetX(),     -Halves.GetY(),     -Halves.GetZ());
    vPoints[7] = vector3(    -Halves.GetX(),      Halves.GetY(),     -Halves.GetZ());
    
    Matrix.Transform( vPoints, vPoints, NumPoints );
    
    draw_Begin( DRAW_LINES, DRAW_USE_ALPHA | DRAW_NO_ZWRITE );
    draw_Color( Color   );
    draw_Verts( vPoints, NumPoints );
    draw_Execute( Index, sizeof(Index)/sizeof(s16) );
    draw_End();
}

//=============================================================================

void SMP_UTIL_PointObjectAt( 
                                 object*            pObject, 
                                 const vector3&     rDir, 
                                 const vector3&     rPos, 
                                 const f32          Roll )
{
    ASSERT( pObject != NULL );
    
    radian  Pitch,Yaw;
    matrix4 M;
    
    rDir.GetPitchYaw( Pitch,Yaw );
    
    M.Setup( vector3(1.0f,1.0f,1.0f), radian3(Pitch,Yaw,Roll), rPos );
    
    pObject->OnTransform( M );
}

//=============================================================================

#if !defined( CONFIG_RETAIL )

void SMP_UTIL_draw_MatrixAxis( const matrix4& rM )
{
    vector3 Pos = rM.GetTranslation();
    
    vector3 X = rM*vector3(100.0f,0.0f,0.0f);
    vector3 Y = rM*vector3(0.0f,100.0f,0.0f);
    vector3 Z = rM*vector3(0.0f,0.0f,100.0f);
    
    draw_Line( Pos, X, xcolor(255,0, 0) );
    draw_Line( Pos, Y, xcolor(0,255, 0) );
    draw_Line( Pos, Z, xcolor(0,0, 255) );
}

#endif // !defined( CONFIG_RETAIL )

//=============================================================================

vector3 SMP_UTIL_RandomVector( f32 Extent )
{
    f32 HalfExtent = Extent*.5f;
    
    return vector3(
        x_frand(-HalfExtent, HalfExtent),
        x_frand(-HalfExtent, HalfExtent),
        x_frand(-HalfExtent, HalfExtent)
        );
}

//=============================================================================

vector3 SMP_UTIL_RandomVector( f32 Extent, const vector3& Orientation )
{
    vector3 Random = SMP_UTIL_RandomVector(Extent);
    
    Random.Rotate( radian3(Orientation.GetPitch(), Orientation.GetYaw(), 0.0f ) );

    return Random;
}

//=============================================================================

xbool SMP_UTIL_IsGuidOfType ( object** ppObject, const guid& rGuid, const rtti& rRunTimeInfo )
{
    if ( ppObject == NULL || rGuid == NULL_GUID )
        return FALSE;
    
    *ppObject = g_ObjMgr.GetObjectByGuid( rGuid );
    
    if (*ppObject == NULL || (*ppObject)->IsKindOf( rRunTimeInfo ) == FALSE )
    {
        return FALSE;
    }
   
    return TRUE;
}

//=============================================================================

f32 SMP_UTIL_GetVolume ( const bbox& BBox )
{
    vector3 Size = BBox.GetSize();
    return  (Size.GetX()*Size.GetY()*Size.GetZ());
}

//=============================================================================

static const s32 ALERT_MAX = 32 ;

void SMP_UTIL_Broadcast_Alert( alert_package& Package )
{
    slot_id idList[ ALERT_MAX ];
    s32     nCount = 0 ;

    // Set up a BBox
    bbox AlertBox( Package.m_Position , Package.m_AlertRadius ) ;

    if ( Package.m_Target == alert_package::ALERT_TARGET_NPC )
    {
        g_ObjMgr.SelectBBox( object::ATTR_CHARACTER_OBJECT, AlertBox ) ;
    }

    slot_id aID = g_ObjMgr.StartLoop();
    while( aID != SLOT_NULL )
    {
        character* pCharacter = ( character* ) g_ObjMgr.GetObjectBySlot(aID);
        
        if ( pCharacter && pCharacter->GetGuid() != Package.m_Origin )
        {
            ASSERT( nCount < ALERT_MAX );
            if( nCount < ALERT_MAX )
            {
                idList[nCount] = aID ;
                nCount++;
            }
        }
        aID = g_ObjMgr.GetNextResult( aID );

    }
    g_ObjMgr.EndLoop();

    for ( s32 i = 0; i < nCount ; i++ )
    {
        character* pCharacter = ( character* ) g_ObjMgr.GetObjectBySlot( idList[i] );
        ASSERT( pCharacter ) ;
        pCharacter->OnAlert( Package ) ;
    }
}

//=============================================================================

xbool   SMP_UTIL_IsButtonVar( prop_query& rPropQuery, const char* pLabel, const char* pButtonTag, xbool& IsRead )
{
    IsRead = FALSE;
    
    if( rPropQuery.IsVar( pLabel ) )
    {
        IsRead =  rPropQuery.IsRead() ;
        
        if( IsRead )
        {
            rPropQuery.SetVarButton( pButtonTag );
        }
        
        return( TRUE );
    }
    
    return FALSE;
}

//=========================================================================

xbool SMP_UTIL_IsParticleFxVar( prop_query& I, const char* pName, rhandle<char>& hParticleFx )
{
    // Found?
    if( I.IsVar( pName ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( hParticleFx.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                hParticleFx.SetName( pString );                

                // Load the particle effect.
                if( hParticleFx.IsLoaded() == FALSE )
                    hParticleFx.GetPointer();
            }
        }
        return( TRUE );
    }
    else
    {
        return FALSE;
    }        
}

//=========================================================================

xbool SMP_UTIL_IsAudioVar( prop_query& I, const char* pName, rhandle<char>& hAudioPackage, s32& SoundID )
{
    // Found?
    if( I.IsVar( pName ) )
    {
        if( I.IsRead() )
        {
            if( SoundID != -1 )
                I.SetVarExternal( g_StringMgr.GetString( SoundID ), 64 );
            else
                I.SetVarExternal( "", 64 );
        }
        else
        {
            // Get the FileName
            xstring ExtString = I.GetVarExternal();
            if( !ExtString.IsEmpty() )
            {
                xstring String( ExtString );

                if( String == "<null>" )
                {
                    SoundID = -1;
                }
                else
                {
                    s32 PkgIndex = String.Find( '\\', 0 );

                    if( PkgIndex != -1 )
                    {
                        xstring Pkg = String.Left( PkgIndex );
                        String.Delete( 0, PkgIndex+1 );

                        hAudioPackage.SetName( Pkg );                

                        // Load the audio package.
                        if( hAudioPackage.IsLoaded() == FALSE )
                            hAudioPackage.GetPointer();
                    }

                    SoundID = g_StringMgr.Add( String );
                }
            }
        }
        return( TRUE );
    }
    else
    {
        return FALSE;
    }
}

//=========================================================================

xbool SMP_UTIL_IsBoneVar( prop_query& I, const char* pName, object& Object, s32& BoneName, s32& BoneIndex )
{
    // Attach bone name?
    if ( I.IsVar( pName ) )
    {
        // Update UI?
        if ( I.IsRead() )
        {
            // Lookup bone name
            const char* pBoneName = "<NONE>";
            if ( BoneName >= 0 )
                pBoneName = g_StringMgr.GetString( BoneName );

            // Update UI
            I.SetVarExternal( pBoneName, x_strlen( pBoneName )+1 );
        }
        else
        {
            // Get name string index and clear bone index
            BoneName  = g_StringMgr.Add( I.GetVarExternal() );
            BoneIndex = -1;

            // Lookup bone index if bone is found
            const char*       pBoneName  = g_StringMgr.GetString( BoneName );
            const anim_group* pAnimGroup = Object.GetAnimGroupPtr();
            if( pAnimGroup )
                BoneIndex = pAnimGroup->GetBoneIndex( pBoneName );
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//=========================================================================

xbool SMP_UTIL_IsAnimVar( prop_query&               I, 
                          const char*               pAnimGroupPropName, 
                          const char*               pAnimPropName, 
                                anim_group::handle& hAnimGroup, 
                                s32&                AnimGroupName, 
                                s32&                AnimName )
{
    // AnimGroup, AnimName
    if( I.IsVar( pAnimGroupPropName ) )
    {
        if( I.IsRead() )
        {
            if( AnimGroupName >= 0 )
                I.SetVarExternal( g_StringMgr.GetString( AnimGroupName ), 256 );
            else
                I.SetVarExternal("", 256);
        }
        else
        {
            // Get the FileName
            xstring String = I.GetVarExternal();
            if( !String.IsEmpty() )
            {
                // Clear?
                if( String == "<null>" )
                {
                    // Clear anim group and name
                    hAnimGroup.SetName( "" );
                    AnimGroupName = -1;
                    AnimName      = -1;
                }
                else
                {            
                    // Setup anim group and name
                    s32 PkgIndex = String.Find( '\\', 0 );
                    if( PkgIndex != -1 )
                    {
                        xstring Pkg = String.Left( PkgIndex );
                        Pkg += "\0\0";
                        AnimName = g_StringMgr.Add( String.Right( String.GetLength() - PkgIndex - 1) );
                        AnimGroupName = g_StringMgr.Add( Pkg );
                        hAnimGroup.SetName( Pkg );
                    }
                    else
                    {
                        AnimGroupName = g_StringMgr.Add( String );
                        hAnimGroup.SetName( String );
                    }
                }
            }
        }            
        return TRUE;
    }

    // AnimName
    if( I.IsVar( pAnimPropName ) )
    {
        if( I.IsRead() )
        {
            if( AnimName >= 0 )
                I.SetVarString( g_StringMgr.GetString( AnimName ), 256 );
            else
                I.SetVarString( "", 256 );
        }
        else
        {
            if( x_strlen( I.GetVarString() ) > 0 )
                AnimName = g_StringMgr.Add( I.GetVarString() );
        }
        return TRUE;
    }

    return FALSE;
}

//=========================================================================

xbool SMP_UTIL_IsTemplateVar( prop_query& I, const char* pName, s32& TemplateID )
{
    if( I.IsVar( pName ) )
    {
        if( I.IsRead() )
        {
            if( TemplateID < 0 )
            {
                I.SetVarFileName("",256);
            }
            else
            {
                I.SetVarFileName( g_TemplateStringMgr.GetString( TemplateID ), 256 );
            }            
        }
        else
        {
            if ( x_strlen( I.GetVarFileName() ) > 0 )
            {
                TemplateID = g_TemplateStringMgr.Add( I.GetVarFileName() );
            }
        }        
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//=========================================================================

#ifdef USE_OBJECT_NAMES

xbool SMP_UTIL_IsObjectNameVar( prop_query& I, const char* pPropertyName, guid ObjectGuid )
{
    if( I.IsVar( pPropertyName ) )
    {
        if( I.IsRead() )
        {
            // Lookup name from object guid (if it's present)
            object*     pObject = g_ObjMgr.GetObjectByGuid( ObjectGuid );
            const char* pObjectName  = "<NONE>";
            if( pObject )
                pObjectName = pObject->GetName();

            // Update UI                    
            I.SetVarString( pObjectName, x_strlen( pObjectName ) + 1 );
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#endif

//=========================================================================

factions SMP_UTIL_GetFactionForGuid( guid Guid )
{
    object* pObject = g_ObjMgr.GetObjectByGuid( Guid );
    if ( pObject && pObject->IsKindOf( actor::GetRTTI() ) == TRUE )
    {
        actor& pActor = actor::GetSafeType( *pObject );
        return pActor.GetFaction();
    }
    else if ( pObject && pObject->IsKindOf( turret::GetRTTI() ) == TRUE )
    {
        turret& pTurret = turret::GetSafeType( *pObject );
        return pTurret.GetFaction();
    }
    else        
    {
        return FACTION_NOT_SET;
    }
}

//=========================================================================

xbool SMP_UTIL_IsPlayerFaction ( guid Guid )
{
    object* pObject = g_ObjMgr.GetObjectByGuid( Guid );
    if ( pObject && pObject->IsKindOf( actor::GetRTTI() ) == TRUE )
    {
        actor* pActor = (actor*)pObject;
        factions Faction = pActor->GetFaction();
        if (Faction >= FACTION_PLAYER_NORMAL && Faction <= FACTION_PLAYER_STRAIN3)
            return TRUE;
    }

    return FALSE;
}

//=========================================================================

god*  SMP_UTIL_Get_God( void )
{
    slot_id GodSlot = g_ObjMgr.GetFirst( object::TYPE_GOD ) ;
    god* pGod = NULL ;

    if ( GodSlot != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( GodSlot ) ;
        if ( pObject )
        {
            pGod = (god*) pObject ;
        }
        else 
        {
            pGod = NULL ;
        }
    }

    return pGod ;
}

//=============================================================================

player* SMP_UTIL_GetActivePlayer( void )
{
    slot_id PlayerSlot = g_ObjMgr.GetFirst( object::TYPE_PLAYER ) ;

    while ( PlayerSlot != SLOT_NULL )
    {
        player* pPlayer = (player*)g_ObjMgr.GetObjectBySlot( PlayerSlot ) ;
        if ( pPlayer && pPlayer->IsActivePlayer() )
        {
            return pPlayer;
        }
        PlayerSlot = g_ObjMgr.GetNext( PlayerSlot ) ;
    }

    return NULL;
}

//=========================================================================

guid SMP_UTIL_GetActivePlayerGuid( void )
{
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if ( pPlayer != NULL )
    {
        return pPlayer->GetGuid();
    }
    else
    {
        return NULL_GUID;
    }
}

//=========================================================================

bbox SMP_UTIL_GetUntransformedBBox( const matrix4& M, const bbox& TransBBox )
{
    matrix4 InverseM(M);

    InverseM.InvertSRT();

    bbox UnTransformedBBox = TransBBox;

    UnTransformedBBox.Transform( InverseM );

    return UnTransformedBBox;
}

//===========================================================================

u16     SMP_UTIL_GetZoneForPosition  ( const vector3& vPosition, f32 DistToCheck )
{
    // Just in case the position we are passing in is at the same height as the object under it.
    vector3 vStartPos = vPosition + vector3( 0.f, 5.f, 0.f );
    vector3 vEndPos( vPosition.GetX(), vPosition.GetY() + DistToCheck, vPosition.GetZ() );
    g_CollisionMgr.LineOfSightSetup( NULL_GUID, vStartPos, vEndPos );
    g_CollisionMgr.SetMaxCollisions(1);
    g_CollisionMgr.CheckCollisions();

    // If there is no collisions, you are going to get bad data back.
    if ( g_CollisionMgr.m_nCollisions <= 0 )
    {
        return 255;
    }

    object_ptr<object> pObject( g_CollisionMgr.m_Collisions[0].ObjectHitGuid );
    if ( ! pObject.IsValid() )
    {
        return 255;
    }

    return pObject.m_pObject->GetZone1();
}

//=============================================================================

#ifdef TARGET_PC

// Draw arc of XZ circle
void draw_Arc( const vector3& C, f32 R, radian Dir, radian FOV, xcolor Color, f32 PercentDraw )
{
    f32 Sin, Cos ;

    s32    i          = 1 + (s32)(R * FOV * PercentDraw) ;
    radian Angle      = Dir - (FOV * 0.5f) ;
    radian DeltaAngle = FOV / i ;

    draw_Begin(DRAW_LINE_STRIPS) ;
    draw_Color(Color) ;
    draw_Vertex( C ) ;
    while(i--)
    {
        x_sincos(Angle, Sin, Cos) ;
        draw_Vertex( C.GetX() + (R * Sin),
                     C.GetY(),
                     C.GetZ() + (R * Cos) ) ;

        Angle += DeltaAngle ;
    }
    x_sincos(Angle, Sin, Cos) ;
    draw_Vertex( C.GetX() + (R * Sin),
                 C.GetY(),
                 C.GetZ() + (R * Cos) ) ;
    draw_Vertex( C ) ;
    draw_End() ;
}

// Draw arc of XZ circle
void draw_3DCircle( const vector3& C, f32 R, xcolor Color, const vector3& Up, f32 PercentDraw )
{
    s32     nSteps     = MAX(6,(s32)(R_360 / PercentDraw));
    radian  Angle      = 0;
    radian  DeltaAngle = R_360 / nSteps;

    quaternion Q(Up,0);

    vector3 Perp;
    if (x_abs(Up.Dot(vector3(0,1,0))) > 0.001f)
    {
        Perp = Up.Cross(vector3(1,0,0));        
    }
    else
    {
        Perp = Up.Cross(vector3(0,1,0));        
    }

    Perp.NormalizeAndScale( R );
    
    draw_Begin(DRAW_LINE_STRIPS) ;
    draw_Color(Color) ;

    while(nSteps--)
    {
        quaternion Q(Up,Angle);

        vector3 NewV = Q * Perp;

        NewV += C;
        draw_Vertex( NewV );
        Angle += DeltaAngle ;
    }    

    draw_End() ;
}
#endif

//=============================================================================

void draw_Cylinder( const vector3& Center, f32 Radius, f32 Height, s32 nSteps, xcolor Color, xbool bCapped, const vector3& Up )
{
    (void)bCapped;

    nSteps = MAX(5, nSteps);

    radian      Angle = 0;
    radian      DeltaAngle = R_360 / nSteps;

    vector3 Vertical = Up;
    Vertical.NormalizeAndScale( Height/2 );

    //quaternion Q(Up,0);

    vector3 Perp;
    if (x_abs(Up.Dot(vector3(0,1,0))) > 0.001f)
    {
        Perp = Up.Cross(vector3(1,0,0));        
    }
    else
    {
        Perp = Up.Cross(vector3(0,1,0));        
    }

    Perp.NormalizeAndScale( Radius );

    smem_StackPushMarker();
    vector3*    pVert = (vector3*)smem_StackAlloc( sizeof(vector3)*nSteps*2 );
    ASSERT(pVert);
    
    s32 i;
    for (i=0;i<nSteps;i++)
    {
        quaternion Q(Up,Angle);

        vector3 NewV = Q * Perp;

        NewV += Center;

        pVert[i*2+0] = NewV + Vertical;
        pVert[i*2+1] = NewV - Vertical;
           
        Angle += DeltaAngle ;
    }    

    // Draw it
    draw_Begin( DRAW_TRIANGLES, DRAW_USE_ALPHA | DRAW_CULL_NONE );
    draw_Color( Color );

    for (i=0;i<nSteps;i++)
    {
        draw_Vertex( pVert[((i+0)%nSteps)*2+0] );
        draw_Vertex( pVert[((i+0)%nSteps)*2+1] );
        draw_Vertex( pVert[((i+1)%nSteps)*2+1] );

        draw_Vertex( pVert[((i+0)%nSteps)*2+0] );
        draw_Vertex( pVert[((i+1)%nSteps)*2+1] );
        draw_Vertex( pVert[((i+1)%nSteps)*2+0] );
    }

    draw_End();

    smem_StackPopToMarker();
}

//=============================================================================

select_slot_iterator::select_slot_iterator( void )
{
    m_SlotID = SLOT_NULL;
}

//=============================================================================

select_slot_iterator::~select_slot_iterator ( void )
{
    //m_SlotID must be SLOT_NULL becuase by convention 
    //when we get a list from the obj_mngr we must always call the
    //EndLoop, this check guarnentee that is never not called..
    
    ASSERT(m_SlotID == SLOT_NULL);
}

//=============================================================================

object* select_slot_iterator::Get( void )
{
    return g_ObjMgr.GetObjectBySlot( m_SlotID );
}

//=============================================================================

void select_slot_iterator::Begin( void )
{
    m_SlotID = g_ObjMgr.StartLoop();
}

//=============================================================================

void select_slot_iterator::End( void )
{
   g_ObjMgr.EndLoop();
   m_SlotID = SLOT_NULL;
}

//=============================================================================

void select_slot_iterator::Next( void )
{
    m_SlotID = g_ObjMgr.GetNextResult( m_SlotID );
}

//=============================================================================

xbool select_slot_iterator::AtEnd( void )
{
    if (m_SlotID == SLOT_NULL)
        return TRUE;

    return FALSE;
}

//=============================================================================

xbool select_slot_iterator::IsEmpty( void )
{
    if (m_SlotID == SLOT_NULL)
        return TRUE;

    return FALSE;
}

//=============================================================================
/*
object_ptr<object>::object_ptr( const guid& rGuid ) 
{ 
    m_pObject = NULL;
    
    if (rGuid != NULL)
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( rGuid );
    }
}
*/

//=============================================================================

s32 SMP_UTIL_GetInfoAllPlayers( s32 MaxPlayersToGet, matrix4* pL2Ws, player** ppPlayer )
{
    s32 nPlayers = 0;

#ifdef X_EDITOR
    nPlayers = 1;
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if (NULL == pPlayer)
        return 0;

    if (MaxPlayersToGet > 0)
    {
        if ( NULL != ppPlayer )
            ppPlayer[0] = pPlayer;
        if ( NULL != pL2Ws )
            pL2Ws[0] = pPlayer->GetL2W();    
    }
#else
    nPlayers = g_NetworkMgr.GetLocalPlayerCount();
    
    s32 i;

    for (i=0;i<MIN(nPlayers, MaxPlayersToGet);i++)
    {
        s32 NetSlot = g_NetworkMgr.GetLocalPlayerSlot( i );
        netobj* pNetObj = NetObjMgr.GetObjFromSlot( NetSlot );
        player& Player = player::GetSafeType( *pNetObj );
        
        if ( NULL != ppPlayer )
            ppPlayer[i] = &Player;
        if ( NULL != pL2Ws )
            pL2Ws[i] = Player.GetL2W();  
    }
#endif

    return nPlayers;
}

//=============================================================================

xbool SMP_UTIL_InitFXFromString( const char* pFXOName, fx_handle& FXHandle )
{
    if (NULL == pFXOName)
        return FALSE;

    rhandle<char> FxResource;    
    FxResource.SetName( pFXOName );
    const char* pScrewedUpName = FxResource.GetPointer();
    if ( pScrewedUpName )
    {
        if (FXHandle.InitInstance( pScrewedUpName ))
            return TRUE;
    }
    return FALSE;
}

//=============================================================================

void SMP_UTIL_EnumHiddenManualResource( prop_enum&           PropEnum,
                                        const char*          pPropertyName,
                                        smp_resource_type    Type )
{
    const char* pTypeString = NULL;
    // Filling out the entire string here because there are some types
    // that use a leading tag other than "resource".  I don't know if we
    // need them yet, or at all, but I'll leave it open to easy modification
    // later.
    switch(Type)
    {
    case SMP_FXO:
        pTypeString = "Resource\0fxo";
        break;
    case SMP_AUDIOPKG:
        pTypeString = "Resource\0audiopkg";
        break;
    case SMP_XBMP:
        pTypeString = "Resource\0xbmp";
        break;
    case SMP_ANIM:
        pTypeString = "Resource\0anim";
        break;
    case SMP_SKINGEOM:
        pTypeString = "Resource\0skingeom";
        break;
    case SMP_RIGIDGEOM:
        pTypeString = "Resource\0rigidgeom";
        break;
    }

    // Nick and I looked through this briefly, but with all the changes, we are unsure which properties actually work as intended.
    // Sooooo... we've removed everything except the DONT_SHOW.
    PropEnum.PropEnumExternal( pPropertyName, pTypeString, "", /*PROP_TYPE_READ_ONLY | PROP_TYPE_DONT_SAVE |*/ PROP_TYPE_DONT_SHOW /*| PROP_TYPE_DONT_SAVE_MEMCARD*/ );    
}

//=============================================================================

xbool SMP_UTIL_IsHiddenManualResource(  prop_query&  I,
                                        const char*  pPropertyName,
                                        const char*  pResourceName )
{
    // Found?
    if( I.IsVar( pPropertyName ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( pResourceName, RESOURCE_NAME_SIZE );
        }
        else
        {
            // Ignore write attempts
        }
        return( TRUE );
    }
    else
    {
        return FALSE;
    }        
}

//=============================================================================
