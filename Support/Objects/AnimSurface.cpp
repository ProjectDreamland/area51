
#include "AnimSurface.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "GameLib\RigidGeomCollision.hpp"
#include "Render\Render.hpp"
#include "EventMgr\EventMgr.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "Event.hpp"

xstring g_AnimSurfaceStringList;


//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

#define ANIMSURFACE_DATA_VERSION 100
//=============================================================================

static struct anim_surface_desc : public object_desc
{
    anim_surface_desc( void ) : object_desc( 
        object::TYPE_ANIM_SURFACE, 
        "Anim Surface", 
        "PROPS",
            object::ATTR_COLLIDABLE             | 
            object::ATTR_BLOCKS_ALL_PROJECTILES | 
            object::ATTR_BLOCKS_ALL_ACTORS      | 
            object::ATTR_BLOCKS_RAGDOLL         | 
            object::ATTR_BLOCKS_CHARACTER_LOS   | 
            object::ATTR_BLOCKS_PLAYER_LOS      | 
            object::ATTR_BLOCKS_PAIN_LOS        | 
            object::ATTR_BLOCKS_SMALL_DEBRIS    | 
            object::ATTR_RENDERABLE             |
            object::ATTR_NEEDS_LOGIC_TIME       |
            object::ATTR_SPACIAL_ENTRY          |
            object::ATTR_ACTOR_RIDEABLE,

            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_IS_DYNAMIC            |
            FLAGS_NO_ICON               |
            FLAGS_BURN_VERTEX_LIGHTING ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new anim_surface; }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourceName( void ) const
    {
        return "RigidGeom";
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourcePropertyName( void ) const 
    {
        return "RenderInst\\File";
    }

} s_AnimSurface_Desc;

//=============================================================================

const object_desc& anim_surface::GetTypeDesc( void ) const
{
    return s_AnimSurface_Desc;
}

//=============================================================================

const object_desc& anim_surface::GetObjectType( void )
{
    return s_AnimSurface_Desc;
}

//=============================================================================
// FUNCTIONS
//=============================================================================

anim_surface::anim_surface( void )
{
    m_iBackupAnimString = g_StringMgr.Add( "None" );
}

//=============================================================================

anim_surface::~anim_surface( void )
{
}

//=============================================================================

void anim_surface::UpdateZoneTrack ( void )
{ 
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, GetPosition() );
}

//=============================================================================

void anim_surface::OnMove( const vector3& NewPos )
{
    bbox BBox = GetBBox() ;
 
    object::OnMove( NewPos );
    m_AnimPlayer.SetL2W( GetL2W() );
 
    BBox += GetBBox() ;

    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
    if( pRigidGeom && pRigidGeom->m_Collision.nLowClusters > 0 )
        g_PolyCache.InvalidateCells( BBox, GetGuid() );

    // Make sure to track the object across zones
    UpdateZoneTrack();
}
 
//=============================================================================
 
void anim_surface::OnTransform( const matrix4& L2W )
{
    bbox BBox = GetBBox() ;
 
    object::OnTransform( L2W );
    m_AnimPlayer.SetL2W( GetL2W() );
 
    BBox += GetBBox() ;
    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
    if( pRigidGeom && pRigidGeom->m_Collision.nLowClusters > 0 )
        g_PolyCache.InvalidateCells( BBox, GetGuid() );

    // Make sure to track the object across zones
    UpdateZoneTrack();
}

//=============================================================================
xbool g_RunAnimSurfaceLogic = TRUE;

void anim_surface::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "anim_surface::OnAdvanceLogic" );
    
    if( !g_RunAnimSurfaceLogic )
        return;

    // Update animation?
    if( m_hAnimGroup.IsLoaded() )
    {

        m_AnimPlayer.Advance( DeltaTime );
        g_EventMgr.HandleSuperEvents( m_AnimPlayer, this );


        //
        // Consider updating polycache
        //
        {
            if( (m_AnimPlayer.GetFrame() != m_AnimPlayer.GetPrevFrame()) ||
                (m_AnimPlayer.GetCycle() != m_AnimPlayer.GetPrevCycle()) )
            {
                rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
                if( pRigidGeom && pRigidGeom->m_Collision.nLowClusters > 0 )
                    g_PolyCache.InvalidateCells( GetBBox(), GetGuid() );
            }
        }
    }

    m_Inst.OnAdvanceLogic( DeltaTime );
}

//=============================================================================

#ifndef X_RETAIL
xbool g_DoAnimSurfaceRender = TRUE;
#endif

void anim_surface::OnRender( void )
{
    CONTEXT( "anim_surface::OnRender" );
    
#ifndef X_RETAIL
    if( !g_DoAnimSurfaceRender )
        return;
#endif

    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
    
    if( pRigidGeom && (m_hAnimGroup.IsLoaded() == TRUE) )
    {
        // Compute bones
        const matrix4* pBone = GetBoneL2Ws() ;
        if (!pBone)
            return ;

        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;

        m_Inst.Render( pBone, Flags | GetRenderMode() );
    }
    else
    {
#ifdef X_EDITOR
        draw_BBox( GetBBox() );
#endif // X_EDITOR
    }
}

//=============================================================================

void anim_surface::OnColCheck ( void )
{
    CONTEXT("anim_surface::OnColCheck");
    
    // Compute the bone matrices, then let the play_surface code handle it.

    if( m_Inst.GetRigidGeom() && (m_hAnimGroup.IsLoaded() == TRUE) )
    {
        // Compute bones
        const matrix4* pBone = GetBoneL2Ws() ;
        if (!pBone)
            return ;
        play_surface::DoColCheck( pBone );
    }
    else
    {
        g_CollisionMgr.StartApply( GetGuid() );
        g_CollisionMgr.ApplyAABBox( GetBBox() );    
        g_CollisionMgr.EndApply();    
    }
}

//=============================================================================

void anim_surface::OnPolyCacheGather( void )
{
    RigidGeom_GatherToPolyCache( GetGuid(), 
                                 GetBBox(), 
                                 m_Inst.GetLODMask( U16_MAX ),
                                 GetBoneL2Ws(), 
                                 m_Inst.GetRigidGeom() );
}

//=============================================================================

#ifndef X_RETAIL
void anim_surface::OnColRender( xbool bRenderHigh )
{
    // Compute the bone matrices, then let the play_surface code handle it.

    if( m_Inst.GetRigidGeom() && (m_hAnimGroup.IsLoaded() == TRUE) )
    {
        // Compute bones
        const matrix4* pBone = GetBoneL2Ws() ;
        if (!pBone)
            return ;

        play_surface::DoColRender( pBone, bRenderHigh );
    }
}
#endif // X_RETAIL

//=============================================================================

const matrix4* anim_surface::GetBoneL2Ws( void )
{
    return m_AnimPlayer.GetBoneL2Ws() ;
}

//=============================================================================

void anim_surface::OnEnumProp( prop_enum& List )
{
    play_surface::OnEnumProp( List );
    
    List.PropEnumExternal( "RenderInst\\Anim",           "Resource\0anim\0",       "Resource File", PROP_TYPE_MUST_ENUM );
    
    List.PropEnumHeader  ( "AnimSurface", "This is the properties that are unique for the anim surface", 0 );

    List.PropEnumExternal( "AnimSurface\\Audio Package",  "Resource\0audiopkg\0",   "The audio package associated with this anim surface.", 0 );

    List.PropEnumBool    ( "AnimSurface\\IsAnimDone", "Tells the system wether the animation is done playing or not", PROP_TYPE_EXPOSE | PROP_TYPE_READ_ONLY );
    List.PropEnumFloat   ( "AnimSurface\\FrameCount", "Frame the current animation is on", PROP_TYPE_EXPOSE | PROP_TYPE_READ_ONLY | PROP_TYPE_DONT_SHOW );

    List.PropEnumBool( "AnimSurface\\IsActive", "Tells whether an object is active or not. If you turn this off after loading the object will be not active. You can activate or deactivate an object any time.", PROP_TYPE_EXPOSE );

    if( m_hAnimGroup.GetPointer() )
    {
        //
        // TODO: HACK: We need a better way to do this in the future
        //
        anim_group* pAnimGroup = m_hAnimGroup.GetPointer();

        g_AnimSurfaceStringList.Clear();

        s32 i;
        for( i=0; i<pAnimGroup->GetNAnims(); i++ )
        {
            const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( i );

            g_AnimSurfaceStringList += AnimInfo.GetName();
            g_AnimSurfaceStringList += "~";
        }

        for( i=0; g_AnimSurfaceStringList[i]; i++ )
        {
            if( g_AnimSurfaceStringList[i] == '~' )
                g_AnimSurfaceStringList[i] = 0;
        }

        List.PropEnumEnum( "AnimSurface\\PlayAnim", g_AnimSurfaceStringList, "Only for triggers. Select an animation what you want the object to play", PROP_TYPE_EXPOSE );
    }
    else
    {
        List.PropEnumEnum( "AnimSurface\\PlayAnim", "\0\0", "Only for triggers. Select an animation what you want the object to play", PROP_TYPE_EXPOSE );
    }
}

//=============================================================================

xbool anim_surface::OnProperty( prop_query& I )
{
    if( play_surface::OnProperty( I ) )
    {
        // Iniitialize the tracker
        if( I.IsVar( "Base\\Position" )) 
        {
            g_ZoneMgr.InitZoneTracking( *this, m_ZoneTracker );
        }

        return( TRUE );
    }

    if( I.IsVar( "RenderInst\\Anim" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hAnimGroup.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();
            if( pString[0] )
            {
                m_hAnimGroup.SetName( pString );
                anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
                
                if( pAnimGroup )
                {
                    m_AnimPlayer.SetAnimGroup( m_hAnimGroup );
                    m_AnimPlayer.SetAnim( 0, TRUE );
                }
            }
        }
        return( TRUE );
    }

    if( I.IsVar( "AnimSurface\\Audio Package" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                if( xstring(pString) == "<null>" )
                {
                    m_hAudioPackage.SetName( "" );
                }
                else
                {
                    m_hAudioPackage.SetName( pString );                

                    // Load the audio package.
                    if( m_hAudioPackage.IsLoaded() == FALSE )
                        m_hAudioPackage.GetPointer();
                }
            }
        }
        return( TRUE );
    }

    if( I.IsVar( "AnimSurface\\PlayAnim" ) )
    {
        if( I.IsRead() )
        {
            if( m_hAnimGroup.GetPointer() )
            {
                anim_group* pAnimGroup = m_hAnimGroup.GetPointer();

                s32 Index = m_AnimPlayer.GetAnimIndex();
                if( Index != -1 )
                {
                    I.SetVarEnum( pAnimGroup->GetAnimInfo(Index).GetName() );
                }
                else
                {
                    I.SetVarEnum( g_StringMgr.GetString( m_iBackupAnimString ) );
                }
            }
            else
            {
                I.SetVarEnum( g_StringMgr.GetString( m_iBackupAnimString ) );
            }
        }
        else
        {
            if( m_hAnimGroup.GetPointer() )
            {
                s32 Index = m_AnimPlayer.GetAnimIndex( I.GetVarEnum() );
                if( Index != -1 )
                {
                    m_iBackupAnimString = g_StringMgr.Add( I.GetVarEnum() );
                    m_AnimPlayer.SetSimpleAnim( Index );
                }
                else
                {
                    LOG_ERROR( "GAMEPLAY", "No animation found with name (%s) in anim group (%s)",
                        I.GetVarEnum(), m_hAnimGroup.GetName() );                        
                }
            }
            else
            {
                m_iBackupAnimString = g_StringMgr.Add( I.GetVarEnum() );
            }
        }

        return TRUE;
    }

    if( I.IsVar( "AnimSurface\\IsAnimDone" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarBool( m_AnimPlayer.IsAtEnd() );
        }
        else
        {
            ASSERT( 0 );
        }

        return TRUE;
    }

    if( I.IsVar ("AnimSurface\\FrameCount") )
    {
        if( I.IsRead () )
        {
            I.SetVarFloat ( m_AnimPlayer.GetFrame () );
        }
        else
        {
            ASSERT( 0 );
        }

        return TRUE;
    }

    if( I.IsVar( "AnimSurface\\IsActive" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarBool( IsActive() );
        }
        else
        {
            OnActivate( I.GetVarBool() );
        }

        return TRUE;
    }

    return( FALSE );
}

//=========================================================================

void anim_surface::GetBoneL2W( s32 iBone, matrix4& L2W )
{
    const matrix4* pBone = GetBoneL2Ws() ;

    if( pBone )
    {
        L2W = pBone[iBone];
    }
    else
    {
        L2W.Identity();
    }
}

//=========================================================================

bbox anim_surface::GetLocalBBox( void ) const
{
    if (m_hAnimGroup.GetPointer())
    {
        return m_hAnimGroup.GetPointer()->GetBBox();
    }
    else
    {
        return play_surface::GetLocalBBox();
    }
}

//=============================================================================

void anim_surface::EnumAttachPoints( xstring& String ) const
{
    String = "BaseObject~"; 
    
    s32 i;
    
    if ( m_hAnimGroup.IsLoaded() )
    {          
        const anim_group* pGroup = m_hAnimGroup.GetPointer();
        if (NULL != pGroup)
        {            
            s32 nBones = pGroup->GetNBones();    
        
            for (i=0;i<nBones;i++)
            {
                const anim_bone& Bone = pGroup->GetBone( i );

                String += Bone.Name;
                String += "~";
            }        
        }
    }


    for( i=0; String[i]; i++ )
    {
        if( String[i] == '~' )
            String[i] = 0;
    }
}

//=============================================================================

s32 anim_surface::GetAttachPointIDByName( const char* pName ) const
{
    if (x_stricmp(pName,"BaseObject")==0)
        return 0;
    
    if ( m_hAnimGroup.IsLoaded() )
    {          
        const anim_group* pGroup = m_hAnimGroup.GetPointer();
        if (NULL != pGroup)
        {            
            s32 nBones = pGroup->GetNBones();    
            s32 i;
            for (i=0;i<nBones;i++)
            {
                const anim_bone& Bone = pGroup->GetBone( i );

                if (x_stricmp( pName, Bone.Name ) == 0)
                {
                    return i+1;
                }            
            }      
        }
    }


    return -1;
}

//=============================================================================

xstring anim_surface::GetAttachPointNameByID( s32 iAttachPt ) const
{
    if (iAttachPt == 0)
        return xstring("BaseObject");

    if ( m_hAnimGroup.IsLoaded() )
    {          
        const anim_group* pGroup = m_hAnimGroup.GetPointer();
        if (NULL != pGroup)
        {               
            // Decrement by one to bring it into the range [0,nbones)
            iAttachPt -= 1;
            s32 nBones = pGroup->GetNBones();
            if ( (iAttachPt >= 0) &&
                 (iAttachPt < nBones ))
            {
                const anim_bone& Bone = pGroup->GetBone( iAttachPt );

                return Bone.Name;
            }       
        }
    }

    return xstring("INVALID\0");
}

//=============================================================================

void anim_surface::OnAttachedMove( s32             iAttachPt,
                                   const matrix4&  L2W )
{
    if (iAttachPt == 0)
    {                
        OnTransform( L2W );
    }
    else
    if ( m_hAnimGroup.IsLoaded() )
    {          
        const anim_group* pGroup = m_hAnimGroup.GetPointer();
        if (NULL != pGroup)
        {                 
            // Decrement by one to bring it into the range [0,nbones)
            iAttachPt -= 1;
            s32 nBones = pGroup->GetNBones();
            if ( (iAttachPt >= 0) &&
                 (iAttachPt < nBones ))
            {
                const matrix4* pTempL2W = m_AnimPlayer.GetBoneL2W( iAttachPt );
                if (pTempL2W)
                {
                    matrix4 BoneL2W = *pTempL2W;
                    BoneL2W.PreTranslate( m_AnimPlayer.GetBoneBindPosition( iAttachPt ) );

                    vector3 BonePos = BoneL2W.GetTranslation();
                    BonePos -= GetPosition();

                    matrix4 NewL2W = L2W;
                    NewL2W.Translate( -BonePos );

                    OnTransform( NewL2W );
                }
            }        
        }
    }
}



//=============================================================================

xbool anim_surface::GetAttachPointData( s32      iAttachPt,
                                        matrix4& L2W,
                                        u32      Flags )
{
    if (iAttachPt == 0)
    {
        L2W = GetL2W();
        return TRUE;
    }

    if ( m_hAnimGroup.IsLoaded() )
    {          
        const anim_group* pGroup = m_hAnimGroup.GetPointer();
        if (NULL != pGroup)
        {   
            // Decrement by one to bring it into the range [0,nbones)
            iAttachPt -= 1;
            s32 nBones = pGroup->GetNBones();
            if ( (iAttachPt >= 0) &&
                 (iAttachPt < nBones ))
            {
                const matrix4* pL2W = m_AnimPlayer.GetBoneL2W( iAttachPt, TRUE );            
                
                if ( NULL != pL2W )
                    L2W = *pL2W;
                else
                    L2W.Identity();
/*
                const matrix4*    pBindInvMtx = m_AnimPlayer.GetBoneBindInvMtx( iAttachPt );
                if (pBindInvMtx)
                {
                    matrix4 InvInv = *pBindInvMtx;
                    //InvInv.Invert();
                    L2W = InvInv * L2W;
                }
  */              
                if (Flags & ATTACH_USE_WORLDSPACE)
                    L2W.PreTranslate( m_AnimPlayer.GetBoneBindPosition( iAttachPt ) );
                
        
                return TRUE;
            }        
        }
    }

    return FALSE;
}

//=============================================================================

//temp hardcoded values
#define ENVIRO_DAMAGE       100.0f
#define ENVIRO_FORCE        15.f
#define MAX_ENVIRO_TARGETS  8

void anim_surface::OnEvent( const event& Event )
{
    (void)Event;
/*
    if( Event.Type == event::EVENT_PAIN )
    {
        //below is a HACK'd in, hardcoded pain until we get the new pain system online where
        //the pain event itself passes type, which gives damage, force, etc...
        const pain_event& PainEvent = pain_event::GetSafeType( Event );  
        pain Pain;
        Pain.Type           = pain::TYPE_GENERIC;
        Pain.Center         = PainEvent.Position;
        Pain.Origin         = GetGuid();
        Pain.PainEventID    = PainEvent.PainEventID;
        Pain.RadiusR0       = PainEvent.PainRadius;
        Pain.RadiusR1       = PainEvent.PainRadius;        
        Pain.DamageR0       = 1000.0f;
        Pain.DamageR1       = 1000.0f;
        Pain.ForceR0        = 25.0f;
        Pain.ForceR1        = 10.0f;                    

        //copied behavior from character, when we do new pain system we might
        //consider changing the pain delivery method also
        object *pObject;
        slot_id slotArray[MAX_ENVIRO_TARGETS];

        bbox PainBBox( Pain.Center, Pain.RadiusR0 );
        g_ObjMgr.SelectBBox( object::ATTR_DAMAGEABLE, PainBBox, object::TYPE_ALL_TYPES );    
        s32 slotCounter = 0;
        slot_id objectID = g_ObjMgr.StartLoop();    
        while( objectID != SLOT_NULL && slotCounter < MAX_ENVIRO_TARGETS )
        {
            slotArray[slotCounter] = objectID;
            slotCounter++;
            objectID = g_ObjMgr.GetNextResult( objectID );
        }
        g_ObjMgr.EndLoop();

        s32 c;
        for(c=0;c<slotCounter;c++)
        {    
            pObject = g_ObjMgr.GetObjectBySlot( slotArray[c] );
            // don't process pain to self.
            if( pObject && pObject != this )
            {
                // Get the closest point projection to the target.
                vector3 ClosestPoint;
                g_EventMgr.ClosestPointToAABox( Pain.Center, pObject->GetBBox(), ClosestPoint );

                //
                // Create the pain info specific to the target.
                //
                Pain.PtOfImpact = ClosestPoint;
                Pain.Direction  = pObject->GetBBox().GetCenter() - Pain.Center;
                Pain.Direction.Normalize();

                pObject->OnPain( Pain ); 
            }
        }
    }
    */
}

//=============================================================================
