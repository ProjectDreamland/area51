#include "SkinPropSurface.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Render\Render.hpp"
#include "EventMgr\EventMgr.hpp"
#include "Dictionary\global_dictionary.hpp"

xstring g_SkinPropSurfaceStringList;
//=============================================================================
//=============================================================================

skin_prop_surface::anim_pair        skin_prop_surface::s_PairTable[] = 
{        
        anim_pair("NORMAL",                     skin_prop_surface::ANIM_NORMAL),
        anim_pair("DESTORY_ONCE_FINISH",        skin_prop_surface::ANIM_DESTORY_ONCE_FINISH),
     
        anim_pair( k_EnumEndStringConst,        skin_prop_surface::ANIM_INVLAID),  //**MUST BE LAST**//
};

skin_prop_surface::anim_table       skin_prop_surface::s_EnumTable(     s_PairTable    );    

//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

static struct skin_prop_surface_desc : public object_desc
{
    //-------------------------------------------------------------------------

    skin_prop_surface_desc( void ) : object_desc( 
        object::TYPE_SKIN_PROP_ANIM_SURFACE, 
        "SkinPropSurface", 
        "PROPS",

            // Object flags
            object::ATTR_RENDERABLE       | 
            object::ATTR_NEEDS_LOGIC_TIME |
            object::ATTR_COLLIDABLE | 
            object::ATTR_BLOCKS_ALL_PROJECTILES | 
            object::ATTR_BLOCKS_ALL_ACTORS | 
            object::ATTR_BLOCKS_RAGDOLL | 
            object::ATTR_BLOCKS_CHARACTER_LOS | 
            object::ATTR_BLOCKS_PLAYER_LOS | 
            object::ATTR_BLOCKS_PAIN_LOS | 
            object::ATTR_BLOCKS_SMALL_DEBRIS | 
            object::ATTR_SPACIAL_ENTRY,

            // Editor flags
            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_NO_ICON               |
            FLAGS_IS_DYNAMIC ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void )
    {
        return new skin_prop_surface ;
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourceName( void ) const
    {
        return "SkinGeom";
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourcePropertyName( void ) const 
    {
        return "RenderInst\\File";
    }

} s_SkinPropSurface_Desc;


//=============================================================================

const object_desc&  skin_prop_surface::GetTypeDesc( void ) const
{
    return s_SkinPropSurface_Desc;
}

//=============================================================================

const object_desc&  skin_prop_surface::GetObjectType( void )
{
    return s_SkinPropSurface_Desc;
}

//=============================================================================
// FUNCTIONS
//=============================================================================

skin_prop_surface::skin_prop_surface( void )  
{
    m_iMaterial = MAT_TYPE_NULL; 
    m_iBackupAnimString = -1;

    m_FloorProperties.Init( 50.0f, 0.5f );
}

//=============================================================================

skin_prop_surface::~skin_prop_surface( void )
{
}

//=============================================================================

bbox skin_prop_surface::GetLocalBBox( void ) const 
{ 
    // Start with geometry bbox
    bbox BBox( m_Inst.GetBBox() );
    
    // Include animation bbox if it's present
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    if( pAnimGroup )
    {
        BBox += pAnimGroup->GetBBox();
    }
    
    // Just to make sure it's valid
    const vector3 Size( BBox.GetSize() );
    if ( !Size.IsValid() ) // mreed: BBox.IsValid doesn't catch this one, but v3 does!
    {
        // m_Inst isn't ready for us yet
        BBox.Min( -100, -100, -100 );
        BBox.Max(  100,  100,  100 );
    }

    return BBox;
}

//=============================================================================

void skin_prop_surface::OnRender( void )
{
    CONTEXT( "skin_prop_surface::OnRender" );

    if( m_Inst.GetSkinGeom() && (m_hAnimGroup.IsLoaded() == TRUE) )
    {
        // Lookup skin geometry
        skin_geom* pSkinGeom = m_Inst.GetSkinGeom();
        if (!pSkinGeom)
            return;

        // Compute LOD mask
        u64 LODMask = m_Inst.GetLODMask(GetL2W()) ;
        if (LODMask == 0)
            return ;

        // Setup render flags
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;

        // compute bones
        s32 nActiveBones = m_AnimPlayer.GetNBones();
        const matrix4* pMatrices = m_AnimPlayer.GetBoneL2Ws() ;
        if (!pMatrices)
            return ;

        // Render
        m_Inst.Render(&GetL2W(), 
                          pMatrices, 
                          nActiveBones,
                          Flags,
                          LODMask,
                          GetFloorColor());
    }
    else
    {
#ifdef X_EDITOR
        draw_BBox( GetBBox() );
#endif // X_EDITOR
    }
//    draw_BBox( GetBBox() );
}

//=============================================================================

void skin_prop_surface::OnColCheck ( void )
{
    g_CollisionMgr.StartApply( GetGuid() );
    g_CollisionMgr.ApplyAABBox( GetBBox(), m_iMaterial );
    g_CollisionMgr.EndApply();    
}

//=============================================================================

#ifndef X_RETAIL
void skin_prop_surface::OnColRender     ( xbool bRenderHigh )
{
    (void)bRenderHigh;
    draw_BBox( GetBBox() );
}
#endif // X_RETAIL

//=============================================================================

void skin_prop_surface::OnMove( const vector3& NewPos )
{
    object::OnMove( NewPos );
    m_AnimPlayer.SetL2W( GetL2W() );

}

//=============================================================================

void skin_prop_surface::OnTransform( const matrix4& L2W )
{
    object::OnTransform( L2W );
    m_AnimPlayer.SetL2W( GetL2W() );
}

//=============================================================================

void skin_prop_surface::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "skin_prop_surface::OnAdvanceLogic" );

    if( m_hAnimGroup.IsLoaded() )
    {        
        m_AnimPlayer.Advance( DeltaTime );
    }

    // get the floor color for lighting
    m_FloorProperties.Update( GetPosition(), DeltaTime );

    g_EventMgr.HandleSuperEvents( m_AnimPlayer, this );
}

//=============================================================================

void skin_prop_surface::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    m_Inst.OnEnumProp ( List );

    List.PropEnumExternal( "RenderInst\\Anim",           "Resource\0anim",    "Resource File", PROP_TYPE_MUST_ENUM );    

    List.PropEnumHeader  ( "SkinPropSurf", "This is the properties that are unique for the skin prop", 0 );

    List.PropEnumExternal( "SkinPropSurf\\Audio Package",  "Resource\0audiopkg\0","The audio package associated with this skin prop surface.", 0 );

    List.PropEnumBool    ( "SkinPropSurf\\IsAnimDone", "Tells the system wether the animation is done playing or not", PROP_TYPE_EXPOSE | PROP_TYPE_READ_ONLY );

    List.PropEnumBool( "SkinPropSurf\\IsActive", "Tells whether an object is active or not. If you turn this off after loading the object will be not active. You can activate or deactivate an object any time.", PROP_TYPE_EXPOSE );

    if( m_hAnimGroup.GetPointer() )
    {
        //
        // TODO: HACK: We need a better way to do this in the future
        //
        anim_group* pAnimGroup = m_hAnimGroup.GetPointer();

        g_SkinPropSurfaceStringList.Clear();

        s32 i;
        for( i=0; i<pAnimGroup->GetNAnims(); i++ )
        {
            const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( i );

            g_SkinPropSurfaceStringList += AnimInfo.GetName();
            g_SkinPropSurfaceStringList += "~";
        }

        for( i=0; g_SkinPropSurfaceStringList[i]; i++ )
        {
            if( g_SkinPropSurfaceStringList[i] == '~' )
                g_SkinPropSurfaceStringList[i] = 0;
        }

        List.PropEnumEnum( "SkinPropSurf\\PlayAnim", g_SkinPropSurfaceStringList, "Only for triggers. Select an animation what you want the object to play", PROP_TYPE_EXPOSE );
    }
    else
    {
        List.PropEnumEnum( "SkinPropSurf\\PlayAnim", "\0\0", "Only for triggers. Select an animation what you want the object to play", PROP_TYPE_EXPOSE );
    }
}

//=============================================================================

xbool skin_prop_surface::OnProperty( prop_query& I )
{
    // HACK: fix this later!
    SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION );
    
    if( object::OnProperty( I ) )
        return( TRUE );
    
    if( m_Inst.OnProperty( I ) )
        return( TRUE );

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

    // External
    if( I.IsVar( "SkinPropSurf\\Audio Package" ) )
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
     
    if( I.IsVar( "SkinPropSurf\\PlayAnim" ) )
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
                    if( m_iBackupAnimString != -1 )
                        I.SetVarEnum( g_StringMgr.GetString( m_iBackupAnimString ) );
                    else
                        I.SetVarEnum( "" );
                }
            }
            else
            {
                if( m_iBackupAnimString != -1 )
                    I.SetVarEnum( g_StringMgr.GetString( m_iBackupAnimString ) );
                else
                    I.SetVarEnum( "" );
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

    if( I.IsVar( "SkinPropSurf\\IsAnimDone" ) )
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

    if( I.IsVar( "SkinPropSurf\\IsActive" ) )
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

//=============================================================================

void skin_prop_surface::EnumAttachPoints( xstring& String ) const
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

s32 skin_prop_surface::GetAttachPointIDByName( const char* pName ) const
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

xstring skin_prop_surface::GetAttachPointNameByID( s32 iAttachPt ) const
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

void skin_prop_surface::OnAttachedMove( s32             iAttachPt,
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

xbool skin_prop_surface::GetAttachPointData( s32      iAttachPt,
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
              
                if (Flags & ATTACH_USE_WORLDSPACE)
                    L2W.PreTranslate( m_AnimPlayer.GetBoneBindPosition( iAttachPt ) );


                return TRUE;
            }        
        }
    }

    return FALSE;
}

//=============================================================================