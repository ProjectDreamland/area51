//=========================================================================
// INCLUDES
//=========================================================================
#include "Coupler.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "e_ScratchMem.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "e_Draw.hpp"
#include "Objects/AnimSurface.hpp"
#include "Objects/Actor/Actor.hpp"
#include "TriggerEX/TriggerEx_Object.hpp"

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct coupler_desc : public object_desc
{
    coupler_desc( void ) : object_desc( 
            object::TYPE_COUPLER, 
            "Coupler", 
            "SCRIPT",
            object::ATTR_NEEDS_LOGIC_TIME,
            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_TARGETS_OBJS |
            FLAGS_IS_DYNAMIC ) 
    {
        s32 i;
        i=1;
    }

    //---------------------------------------------------------------------
    virtual object* Create          ( void ) { return new coupler; }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32     OnEditorRender  ( object& Object ) const 
    { 
        // Call default render
        object_desc::OnEditorRender( Object );        
        return EDITOR_ICON_COUPLER; 
    }

#endif // X_EDITOR

    virtual void    OnEnumProp  ( prop_enum&  List  );
    virtual xbool   OnProperty  ( prop_query& I     );


} s_coupler_Desc;

//=========================================================================

void coupler_desc::OnEnumProp( prop_enum& List )
{
    object_desc::OnEnumProp( List );

    List.PropEnumHeader( "Coupler", "Coupler Properties", 0 );
    List.PropEnumButton( "Coupler\\Update All Couplers",  "Updates all relative data for all children of all couplers", 0 );    
}

//========================================================================

xbool coupler_desc::OnProperty( prop_query& I )
{
    if (object_desc::OnProperty( I ))
    {
        return TRUE;
    }  
    else if ( I.IsVar("Coupler\\Update All Couplers" ))
    {
        if (I.IsRead())
        {
            I.SetVarButton( "Update All Couplers" );
        }   
        else
        {
            slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_COUPLER );
            while (SlotID != SLOT_NULL)
            {
                object* pObj = g_ObjMgr.GetObjectBySlot( SlotID );
                if (!pObj)
                    continue;
                                
                coupler& Coupler = coupler::GetSafeType( *pObj );
                Coupler.UpdateAllRelative();

                SlotID = g_ObjMgr.GetNext( SlotID );
            }
        }

        return TRUE;
    }
    
    return FALSE;
}
//=========================================================================

const object_desc& coupler::GetTypeDesc( void ) const
{
    return s_coupler_Desc;
}

//=========================================================================

const object_desc& coupler::GetObjectType( void )
{
    return s_coupler_Desc;
}


//=========================================================================
//=========================================================================

void coupler::child::Reset( void )
{
    m_Guid                 = 0;
    m_AttachPtID           = -1;    
    m_AttachPointString    = -1;
    m_bAttachPointDirty    = FALSE;
    m_bApplyPosition       = TRUE;
    m_bApplyRotation       = TRUE;

    m_RelativePos.Zero();
    m_RelativeRot.Zero();
}

//=========================================================================

void coupler::child::SetAttachPoint( const char* pAttachPoint )
{
    m_AttachPointString = g_StringMgr.Add( pAttachPoint );
    m_bAttachPointDirty = TRUE;
}

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

coupler::coupler( void )
{
    m_nChild                  = 0;
    
    s32 i;
    for (i=0;i<COUPLER_MAX_CHILDREN;i++)
    {
        m_Child[i].Reset();
    }

    m_ParentGuid                  = 0;
    m_ParentID                    = -1;
    m_ParentAttachPointString     = -1;
    m_bParentAttachPointDirty     = TRUE;
    m_pChildPhys                  = NULL;
    m_bChildPhysics               = FALSE;
    m_bFirstTimeThrough           = FALSE;

    m_LastParentL2W.Identity();    
}

//=============================================================================

coupler::~coupler( void )
{   
    if (m_pChildPhys)
        delete[] m_pChildPhys;
    m_pChildPhys = NULL;
}

//=============================================================================

void coupler::OnEnumProp( prop_enum& List )
{
    // Call base class
    object::OnEnumProp( List );

    smem_StackPushMarker();
    
    char* pDestEnum = (char*)smem_StackAlloc( 256 );
    x_memset(pDestEnum,0,256);
    x_sprintf(pDestEnum, "attachpnt_picker~attachpnt_picker\\%s", (const char*)guid_ToString( m_ParentGuid ) );
    s32 i;
    for (i=0;i<256;i++)
    {
        if (pDestEnum[i] == '~')
            pDestEnum[i] = 0;
    }

    // Add properties
    List.PropEnumHeader("Coupler", "Properties for the coupler object", 0 );
    List.PropEnumBool  ("Coupler\\Active", "Is the coupler active?", PROP_TYPE_EXPOSE  );

    List.PropEnumBool  ("Coupler\\Physics", "Should simple physics be applied to the children?", PROP_TYPE_MUST_ENUM );

    List.PropEnumGuid  ("Coupler\\Parent Object", "Guid of object to move the target to", PROP_TYPE_EXPOSE | PROP_TYPE_MUST_ENUM);
    List.PropEnumExternal( "Coupler\\Parent Attach Point", pDestEnum, "Available attach points on the destination object.", PROP_TYPE_MUST_ENUM );

    List.PropEnumInt   ("Coupler\\Child Count", "Current number of targets.", PROP_TYPE_DONT_SHOW );

    List.PropEnumButton   ( "Coupler\\Add Child", 
                            "Press the button to add a new target.",
                            PROP_TYPE_MUST_ENUM );

    List.PropEnumButton  ("Coupler\\Update All Relative", 
                            "Press the button to lock in the relative placement of all child objects.",
                            PROP_TYPE_MUST_ENUM | PROP_TYPE_EXPOSE);

    List.PropEnumGuid    ("Coupler\\Set Child 0","",PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_SHOW | PROP_TYPE_EXPOSE);
    List.PropEnumGuid    ("Coupler\\Set Child 1","",PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_SHOW | PROP_TYPE_EXPOSE);
    List.PropEnumGuid    ("Coupler\\Set Child 2","",PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_SHOW | PROP_TYPE_EXPOSE);
    List.PropEnumGuid    ("Coupler\\Set Child 3","",PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_SHOW | PROP_TYPE_EXPOSE);

    for (i=0;i<m_nChild;i++)
    {
        char* pTargetEnum = (char*)smem_StackAlloc( 256 );
        x_memset(pTargetEnum,0,256);
        x_sprintf(pTargetEnum, "attachpnt_picker~attachpnt_picker\\%s", (const char*)guid_ToString( m_Child[i].m_Guid ) );

        s32 j;
        for (j=0;j<256;j++)
        {
            if (pTargetEnum[j] == '~')
                pTargetEnum[j] = 0;
        }
        
//        xstring TypeName = "unknown";
        big_string description = "unknown";

        if (m_Child[i].m_Guid != 0)
        {
/*            object* pObj = g_ObjMgr.GetObjectByGuid( m_Child[i].m_Guid );
            if (pObj)
            {
                TypeName = g_ObjMgr.GetNameFromType( pObj->GetType() );
            }*/
            object* pObject = g_ObjMgr.GetObjectByGuid( m_Child[i].m_Guid );
            if (pObject)
            {
#ifdef X_EDITOR
                if (pObject->IsKindOf( trigger_ex_object::GetRTTI()))
                {
                    //special naming for triggers
                    trigger_ex_object* pTrigger = (trigger_ex_object*)pObject;
                    description.Set(pTrigger->GetTriggerName());
                    if (description.IsEmpty())
                    {
                        description.Set(pObject->GetTypeDesc().GetTypeName());
                    }
                }
                else
#endif // X_EDITOR
                {
#ifdef X_EDITOR
                    description.Set(pObject->GetName());
                    if (description.IsEmpty())
                    {
                        description.Set(pObject->GetTypeDesc().GetTypeName());
                    }
#else
                    description.Set(pObject->GetTypeDesc().GetTypeName());
#endif
                }
            }
        }
        List.PropEnumString( xfs("Coupler\\Child [%d]",i), description.Get(), PROP_TYPE_HEADER );

        s32 iHeader = List.PushPath(xfs("Coupler\\Child [%d]\\",i));

        List.PropEnumGuid    ("Object",  "Guid of object to move", PROP_TYPE_EXPOSE | PROP_TYPE_MUST_ENUM  );  
        List.PropEnumExternal("Attach Point", pTargetEnum, "Available attach points on the target object.", PROP_TYPE_MUST_ENUM );
        List.PropEnumBool    ("Apply Position", "Update position as the parent object moves", 0 );
        List.PropEnumBool    ("Apply Rotation", "Update rotation as the parent object rotates", 0 );
        
        //List.PropEnumBool    ("Relative Placement", "TRUE to keep the targets position and orientation relative to the destination attach point.  This must be set before setting the target GUID.  ** See the editor help for detailed information **" );    
        
        List.PropEnumButton  ("Update Relative", 
                                "Press the button to lock in the relative placement of the objects.",
                                PROP_TYPE_MUST_ENUM | PROP_TYPE_EXPOSE);

        List.PropEnumButton  ("Snap Attach Points", 
                                "Press the button to lock in the relative placement of the objects.",
                                PROP_TYPE_MUST_ENUM );

        List.PropEnumVector3 ("Rel Position",    "Relative Position Data.", 0 );
        List.PropEnumRotation("Rel Rotation",    "Relative Rotation Data.", 0 );
        List.PropEnumButton  ("Remove", 
                                "Press the button to remove this target.",
                                PROP_TYPE_MUST_ENUM );

        if (m_bChildPhysics)
        {
            List.PropEnumFloat( "Mass", "Mass of child object", 0 );
            List.PropEnumFloat( "Dampening", "How fast velocity is lost", 0 );

            List.PropEnumFloat( "StrutLength", "", PROP_TYPE_DONT_SHOW );
            List.PropEnumVector3( "StrutXFormCol1","",PROP_TYPE_DONT_SHOW );
            List.PropEnumVector3( "StrutXFormCol2","",PROP_TYPE_DONT_SHOW );
            List.PropEnumVector3( "StrutXFormCol3","",PROP_TYPE_DONT_SHOW );
            List.PropEnumVector3( "StrutXFormCol4","",PROP_TYPE_DONT_SHOW );
            List.PropEnumVector3( "StrutFreeEnd","",PROP_TYPE_DONT_SHOW );
        }
        List.PopPath(iHeader);
    }
    
    smem_StackPopToMarker();
}

//=============================================================================

xbool coupler::OnProperty( prop_query& I )
{
    // Call base class
    if( object::OnProperty( I ) )
        return TRUE;

    // Active?
    if (I.IsVar("Coupler\\Active"))
    {
        if (I.IsRead())
            I.SetVarBool((GetAttrBits() & object::ATTR_NEEDS_LOGIC_TIME) != 0);
        else
            OnActivate(I.GetVarBool());
        return TRUE;
    }
    else if( I.IsVar( "Coupler\\Update All Relative" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Update All Relative" );
        }
        else
        {
            UpdateAllRelative();
        }
        return TRUE;
    }
    else if (I.IsVar("Coupler\\Set Child 0"))
    {
        if (I.IsRead())
            I.SetVarGUID(0);
        else
        {
            if (m_nChild > 0)
            {
                m_Child[0].m_Guid = I.GetVarGUID();
            }
        }
        return TRUE;
    }
    else if (I.IsVar("Coupler\\Set Child 1"))
    {
        if (I.IsRead())
            I.SetVarGUID(0);
        else
        {
            if (m_nChild > 1)
            {
                m_Child[1].m_Guid = I.GetVarGUID();
            }
        }
        return TRUE;
    }
    else if (I.IsVar("Coupler\\Set Child 2"))
    {
        if (I.IsRead())
            I.SetVarGUID(0);
        else
        {
            if (m_nChild > 2)
            {
                m_Child[2].m_Guid = I.GetVarGUID();
            }
        }
        return TRUE;
    }
    else if (I.IsVar("Coupler\\Set Child 3"))
    {
        if (I.IsRead())
            I.SetVarGUID(0);
        else
        {
            if (m_nChild > 3)
            {
                m_Child[3].m_Guid = I.GetVarGUID();
            }
        }
        return TRUE;
    }
    else if( I.IsVar( "Coupler\\Add Child" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Add Child" );
        }
        else
        {
            if (m_nChild < COUPLER_MAX_CHILDREN)
                m_nChild++;
        }
        return TRUE;
    }
    else
    if (I.VarInt("Coupler\\Child Count", m_nChild))
        return TRUE;
    else
    if (I.IsVar( "Coupler\\Physics"))
    {
        if (I.IsRead())
        {
            xbool B = m_bChildPhysics;
            I.SetVarBool( B );
        }
        else
        {
            xbool Old = m_bChildPhysics;
            m_bChildPhysics = I.GetVarBool();
            if (Old != m_bChildPhysics)
            {
                if (m_bChildPhysics)
                {
                    ASSERT( NULL == m_pChildPhys );
                    m_pChildPhys = new child_phys[ COUPLER_MAX_CHILDREN ];
                    ASSERT(m_pChildPhys);                    
                }
                else
                {
                    delete[] m_pChildPhys;
                    m_pChildPhys = NULL;
                }
                UpdateAllRelative();
            }
        }
        return TRUE;
    }
    else
    if (I.VarGUID("Coupler\\Parent Object", m_ParentGuid))
    {
        m_bParentAttachPointDirty = TRUE;
        UpdateAttachPoints();
        return TRUE;
    }
    else
    if (I.IsVar("Coupler\\Parent Attach Point"))
    {
        if( I.IsRead() )
        {
            UpdateAttachPoints();

            xstring AP = "";
            if ( m_ParentGuid != 0 )
            {
                object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
                if (pObj)
                {
                    AP = pObj->GetAttachPointNameByID(m_ParentID);                    
                }                
            }                
            I.SetVarExternal( AP, 256 );
        }
        else
        {
            m_ParentAttachPointString = g_StringMgr.Add( I.GetVarExternal() );
            m_bParentAttachPointDirty = TRUE;  
        }
        return TRUE;
    }
    else 
    if (I.IsSimilarPath( "Coupler\\Child" ) )
    {
        s32 i = I.GetIndex(0);
        if (i>=0 && i<m_nChild)
        {
            if ( I.IsVar( "Coupler\\Child []" ) )
            {
                if( I.IsRead() )
                {
                    big_string description = "unknown";

                    if (m_Child[i].m_Guid != 0)
                    {
                        object* pObject = g_ObjMgr.GetObjectByGuid( m_Child[i].m_Guid );
                        if (pObject)
                        {
        #ifdef X_EDITOR
                            if (pObject->IsKindOf( trigger_ex_object::GetRTTI()))
                            {
                                //special naming for triggers
                                trigger_ex_object* pTrigger = (trigger_ex_object*)pObject;
                                description.Set(pTrigger->GetTriggerName());
                                if (description.IsEmpty())
                                {
                                    description.Set(pObject->GetTypeDesc().GetTypeName());
                                }
                            }
                            else
        #endif // X_EDITOR
                            {
        #ifdef X_EDITOR
                                description.Set(pObject->GetName());
                                if (description.IsEmpty())
                                {
                                    description.Set(pObject->GetTypeDesc().GetTypeName());
                                }
        #else
                                description.Set(pObject->GetTypeDesc().GetTypeName());
        #endif
                            }
                        }
                    }
                    I.SetVarString(description.Get(), 255 );
                }
                return TRUE;
            }
            else
            if (I.IsVar("Coupler\\Child []\\Remove"))
            {
                if( I.IsRead() )
                {
                    I.SetVarButton( "Remove Child" );
                }
                else
                {
                    s32 j;
                    for (j=i;j<m_nChild-1;j++)
                    {
                        m_Child[j] = m_Child[j+1];
                    }
                    m_nChild--;
                    for (j=m_nChild;j<COUPLER_MAX_CHILDREN;j++)
                    {
                        m_Child[j].Reset();
                    }
                }
                return TRUE;
            }
            else
            if( I.IsVar( "Coupler\\Child []\\Update Relative" ) )
            {
                if( I.IsRead() )
                {
                    I.SetVarButton( "Update Relative" );
                }
                else
                {
                    UpdateRelativeData(i);
                }
                return TRUE;
            }
            if( I.IsVar( "Coupler\\Child []\\Snap Attach Points" ) )
            {
                if( I.IsRead() )
                {
                    I.SetVarButton( "Snap Attach Points" );
                }
                else
                {
                    if (m_nChild < COUPLER_MAX_CHILDREN)
                    {
                        SnapAttachPoints(i);
                    }
                }
                return TRUE;
            }
            if (I.VarGUID("Coupler\\Child []\\Object", m_Child[i].m_Guid))
            {
                if (I.IsRead())
                {
                }
                else
                {
                    // Zeroing out the relative data here is a bad thing.
                    // When blueprints are instantiated, the object gets reset as
                    // part of the guid fixup procedure.  This occurs after the 
                    // coupler has been created and the relative data loaded.
                    
                    //m_Child[i].m_RelativePos.Zero();
                    //m_Child[i].m_RelativeRot.Zero();                    
                }
                return TRUE;
            }
            else
            if (I.VarVector3("Coupler\\Child []\\Rel Position", m_Child[i].m_RelativePos))
            {                
                return TRUE;
            }
            else
            if (I.VarRotation("Coupler\\Child []\\Rel Rotation", m_Child[i].m_RelativeRot))
            {
                return TRUE;
            }
            else
            if (I.IsVar("Coupler\\Child []\\Attach Point"))
            {
                if( I.IsRead() )
                {
                    UpdateAttachPoints();

                    xstring AP = "";
                    if ( m_Child[i].m_Guid != 0 )
                    {
                        object* pObj = g_ObjMgr.GetObjectByGuid( m_Child[i].m_Guid );
                        if (pObj)
                        {
                            AP = pObj->GetAttachPointNameByID( m_Child[i].m_AttachPtID );                    
                        }                
                    }                
                    I.SetVarExternal( AP, 256 );
                }
                else
                {
                    m_Child[i].m_AttachPointString = g_StringMgr.Add( I.GetVarExternal() );
                    m_Child[i].m_bAttachPointDirty = TRUE;            
                }
                return TRUE;
            }
            else
            if (I.IsVar("Coupler\\Child []\\Apply Position"))
            {
                if (I.IsRead())
                {
                    xbool B = m_Child[i].m_bApplyPosition;
                    I.SetVarBool( B );
                }
                else
                {
                    m_Child[i].m_bApplyPosition = I.GetVarBool();                    
                }
                return TRUE;
            }
            else
            if (I.IsVar("Coupler\\Child []\\Apply Rotation"))
            {
                if (I.IsRead())
                {
                    xbool B = m_Child[i].m_bApplyRotation;
                    I.SetVarBool( B );
                }
                else
                {
                    m_Child[i].m_bApplyRotation = I.GetVarBool();                    
                }
                return TRUE;
            }
            else
            if (I.IsVar("Coupler\\Child []\\Mass"))
            {
                if (I.IsRead())
                {
                    if (m_bChildPhysics && m_pChildPhys)
                    {
                        I.SetVarFloat( m_pChildPhys[ i ].m_Mass );
                    }
                    else
                    {
                        I.SetVarFloat(1);
                    }
                }
                else
                {
                    if (m_bChildPhysics && m_pChildPhys)
                    {
                        m_pChildPhys[ i ].m_Mass = I.GetVarFloat();
                    }
                }
                return TRUE;
            }
            else
            if (I.IsVar("Coupler\\Child []\\Dampening"))
            {
                if (I.IsRead())
                {
                    if (m_bChildPhysics && m_pChildPhys)
                    {
                        I.SetVarFloat( m_pChildPhys[ i ].m_Dampening );
                    }
                    else
                    {
                        I.SetVarFloat(1);
                    }
                }
                else
                {
                    if (m_bChildPhysics && m_pChildPhys)
                    {
                        m_pChildPhys[ i ].m_Dampening = I.GetVarFloat();
                    }
                }
                return TRUE;
            }    
            else
            if (I.IsVar("Coupler\\Child []\\StrutXFormCol1"))
            {
                if (I.IsRead())
                {
                    vector3 Col,Dummy;
                    if (m_bChildPhysics && m_pChildPhys)
                        m_pChildPhys[i].m_RelativeTransform.GetColumns(Col,Dummy,Dummy);
                    else
                        Col.Set(0,0,0);
                    I.SetVarVector3(Col);
                }
                else
                {
                    vector3 Col = I.GetVarVector3();
                    if (m_bChildPhysics && m_pChildPhys)
                    {
                        vector3 V1,V2,V3;
                        m_pChildPhys[i].m_RelativeTransform.GetColumns( V1,  V2, V3 );
                        m_pChildPhys[i].m_RelativeTransform.SetColumns( Col, V2, V3 );
                    }
                }
            }
            else
            if (I.IsVar("Coupler\\Child []\\StrutXFormCol2"))
            {
                if (I.IsRead())
                {
                    vector3 Col,Dummy;
                    if (m_bChildPhysics && m_pChildPhys)
                        m_pChildPhys[i].m_RelativeTransform.GetColumns(Dummy,Col,Dummy);
                    else
                        Col.Set(0,0,0);
                    I.SetVarVector3(Col);
                }
                else
                {
                    vector3 Col = I.GetVarVector3();
                    if (m_bChildPhysics && m_pChildPhys)
                    {
                        vector3 V1,V2,V3;
                        m_pChildPhys[i].m_RelativeTransform.GetColumns( V1,  V2, V3 );
                        m_pChildPhys[i].m_RelativeTransform.SetColumns( V1, Col, V3 );
                    }
                }
            }
            else
            if (I.IsVar("Coupler\\Child []\\StrutXFormCol3"))
            {
                if (I.IsRead())
                {
                    vector3 Col,Dummy;
                    if (m_bChildPhysics && m_pChildPhys)
                        m_pChildPhys[i].m_RelativeTransform.GetColumns(Dummy,Dummy,Col);
                    else
                        Col.Set(0,0,0);
                    I.SetVarVector3(Col);
                }
                else
                {
                    vector3 Col = I.GetVarVector3();
                    if (m_bChildPhysics && m_pChildPhys)
                    {
                        vector3 V1,V2,V3;
                        m_pChildPhys[i].m_RelativeTransform.GetColumns( V1, V2, V3  );
                        m_pChildPhys[i].m_RelativeTransform.SetColumns( V1, V2, Col );
                    }
                }
            }
            else
            if (I.IsVar("Coupler\\Child []\\StrutXFormCol4"))
            {
                if (I.IsRead())
                {
                    vector3 Col;
                    if (m_bChildPhysics && m_pChildPhys)
                        Col = m_pChildPhys[i].m_RelativeTransform.GetTranslation();
                    else
                        Col.Set(0,0,0);
                    I.SetVarVector3(Col);
                }
                else
                {
                    vector3 Col = I.GetVarVector3();
                    if (m_bChildPhysics && m_pChildPhys)
                    {
                        m_pChildPhys[i].m_RelativeTransform.SetTranslation( Col );
                    }
                }
            }
            else
            if (I.VarVector3("Coupler\\Child []\\StrutFreeEnd",m_pChildPhys[i].m_FreeEnd))
            {
                return TRUE;
            }
            else
            if (I.VarFloat("Coupler\\Child []\\StrutLength", m_pChildPhys[ i ].m_StrutLength))
                return TRUE;                        
        }
    }
   
    
    return FALSE;
}

//=========================================================================

void coupler::OnMove( const vector3& NewPos )
{
    // Call base class
    object::OnMove(NewPos);
}

//=========================================================================

void coupler::OnTransform( const matrix4& L2W )
{    
    // Call base class
    object::OnTransform(L2W);
}
    
//=========================================================================

void coupler::OnInit( void )
{
}

//=========================================================================

#ifndef X_RETAIL
void coupler::OnDebugRender  ( void )
{
#ifdef X_EDITOR
    CONTEXT("coupler::OnDebugRender" );

    matrix4 ParentL2W;
    vector3 ParentAP(0,0,0);

    if (m_ParentGuid != 0)
    {            
        object* pDestObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
        if (NULL != pDestObj)
        {
            UpdateAttachPoints();

            if (!m_bParentAttachPointDirty)
            {
                if (pDestObj->GetAttachPointData( m_ParentID, ParentL2W, object::ATTACH_USE_WORLDSPACE ))
                {
                    ParentAP = ParentL2W.GetTranslation();

                    draw_Begin( DRAW_TRIANGLES, DRAW_CULL_NONE );
                    
                    vector3 Temp[4];
                    
                    Temp[0].Set(0,0,-400);
                    Temp[1].Set(0,0,0);
                    Temp[2].Set(50,0,0);
                    Temp[3].Set(-50,0,0);

                    vector3 Out[4];
    
                    ParentL2W.Transform(Out,Temp,4);

                    draw_Color(XCOLOR_RED);
                    draw_Vertex(Out[0]);
                    draw_Vertex(Out[1]);
                    draw_Color(XCOLOR_BLUE);
                    draw_Vertex(Out[2]);

                    draw_Color(XCOLOR_RED);
                    draw_Vertex(Out[0]);
                    draw_Color(XCOLOR_BLUE);
                    draw_Vertex(Out[3]);
                    draw_Color(XCOLOR_RED);
                    draw_Vertex(Out[1]);

                    draw_End();
                }
            }
        }
    }

    s32  i;
    for (i=0;i<m_nChild;i++)
    {
        matrix4 ChildL2W;

        object* pChildObj = g_ObjMgr.GetObjectByGuid( m_Child[i].m_Guid );
        if (NULL != pChildObj)
        {
            UpdateAttachPoints();

            if (!m_Child[i].m_bAttachPointDirty)
            {
                if (pChildObj->GetAttachPointData( m_Child[i].m_AttachPtID, ChildL2W, object::ATTACH_USE_WORLDSPACE ))
                {
                    draw_Begin( DRAW_TRIANGLES, DRAW_CULL_NONE );
                    
                    vector3 Temp[4];
                    
                    Temp[0].Set(0,0,-400);
                    Temp[1].Set(0,0,0);
                    Temp[2].Set(50,0,0);
                    Temp[3].Set(-50,0,0);

                    vector3 Out[4];
    
                    ChildL2W.Transform(Out,Temp,4);

                    draw_Color(XCOLOR_GREEN);
                    draw_Vertex(Out[0]);
                    draw_Vertex(Out[1]);
                    draw_Color(XCOLOR_BLUE);
                    draw_Vertex(Out[2]);

                    draw_Color(XCOLOR_GREEN);
                    draw_Vertex(Out[0]);
                    draw_Color(XCOLOR_BLUE);
                    draw_Vertex(Out[3]);
                    draw_Color(XCOLOR_GREEN);
                    draw_Vertex(Out[1]);

                    draw_End();
                }
            }

            if (m_bChildPhysics && m_pChildPhys)
            {
                child_phys  P = m_pChildPhys[ i ];
                child       C = m_Child[ i ];

                draw_Sphere( P.m_FreeEnd, 15, XCOLOR_RED );
                draw_Line( ParentAP, P.m_FreeEnd, XCOLOR_RED );

            }
        }
    }

    // Call base class
    object::OnDebugRender();
#endif // X_EDITOR
}
#endif // X_RETAIL


//=========================================================================

#ifdef X_EDITOR

s32 coupler::OnValidateProperties( xstring& ErrorMsg )
{
    s32 nErrors = 0;
    
    // Validate all child objects
    for( s32 i = 0; i < m_nChild; i++ )
    {
        // Lookup child guid
        guid ChildGuid = m_Child[i].m_Guid;
        
        // Skip if child not specified
        if( ChildGuid == NULL_GUID )
            continue;
            
        // Make sure child exists
        nErrors += OnValidateObject( xstring( xfs( "Child object[%d]", i ) ), &ChildGuid, ErrorMsg );
        
        // Lookup child object
        object* pChildObject = g_ObjMgr.GetObjectByGuid( ChildGuid );
        if( !pChildObject )
            continue;
            
        // Make sure child is a dynamic object
        const object_desc& TypeDesc = pChildObject->GetTypeDesc();
        if( TypeDesc.IsDynamic() == FALSE )
        {
            nErrors++;
            ErrorMsg += xfs( "\n\nChild object[%d] Guid[%s] of Type[%s] is not allowed.\n", i, guid_ToString( ChildGuid ), TypeDesc.GetTypeName() );
            ErrorMsg += "Only dynamic objects such are prop surfaces etc can be coupled.\n";
            ErrorMsg += "This child will not behave properly correctly and should be converted to a valid type!!!\n\n";
        }
    }
    
    return nErrors;
}

#endif // X_EDITOR

//=========================================================================

void coupler::OnAdvanceLogic( f32 DeltaTime )
{       
    (void) DeltaTime;

    if (m_ParentGuid == 0)
        return;

    matrix4 ParentL2W;

    object* pDestObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
    if (NULL == pDestObj)
        return;

    if (m_bParentAttachPointDirty || (m_Child[0].m_bAttachPointDirty) ||
                                     (m_Child[1].m_bAttachPointDirty) ||
                                     (m_Child[2].m_bAttachPointDirty) ||
                                     (m_Child[3].m_bAttachPointDirty))
    {
        UpdateAttachPoints();
    }

    if (m_bParentAttachPointDirty)
        return;

    if (!pDestObj->GetAttachPointData( m_ParentID, ParentL2W ))
        return;

    vector3 ParentAP = ParentL2W.GetTranslation();
    radian3 ParentRot = ParentL2W.GetRotation();
    vector3 LastParentPos = m_LastParentL2W.GetTranslation();
    radian3 LastParentRot = m_LastParentL2W.GetRotation();

    f32 ParentDelta = (LastParentPos - ParentAP).Length();
    f32 ParentRotDelta = LastParentRot.Difference( ParentRot);

    xbool bSleep = FALSE;
    if ((ParentDelta < 0.001f) && (ParentRotDelta < 0.001f))
        bSleep = TRUE;

    if ( pDestObj->IsKindOf( anim_surface::GetRTTI() ) ||
         pDestObj->IsKindOf( actor::GetRTTI() ) )        
    {
        bSleep = FALSE;
    }

    xbool firstTimeFired = FALSE;
    if (bSleep)
    {
        // The parent has essentially remained stationary.
        // If physics isn't on return.
        // If physics is on, and is at rest, return
        if (m_bChildPhysics && m_pChildPhys)
        {
            xbool bRet = TRUE;
            s32 i;
            for (i=0;i<m_nChild;i++)
            {
                if (m_pChildPhys[i].m_LastVelocity.LengthSquared() > 0.1f)
                {
                    bRet = FALSE;
                    break;
                }
            }
            if (bRet)
                return;
        }
        else
        {
            return;
        }
    }

    s32 i;
    matrix4 ChildL2W;

    u16 NewZone1 = pDestObj->GetZone1();
    u16 NewZone2 = pDestObj->GetZone2();

    for (i=0;i<m_nChild;i++)
    {
        if (m_bChildPhysics && m_pChildPhys)
        {
            AdvancePhysics( DeltaTime );
        
            object* pTargetObj = g_ObjMgr.GetObjectByGuid( m_Child[i].m_Guid );
            if (NULL == pTargetObj)
                continue;

            child_phys& P = m_pChildPhys[ i ];

            matrix4 ParentL2W_World;
            
            pDestObj->GetAttachPointData( m_ParentID, ParentL2W_World, object::ATTACH_USE_WORLDSPACE );

            //Move the child
            vector3 StrutDelta = ParentL2W_World.GetTranslation() - P.m_FreeEnd;
        
            quaternion  Q;

            StrutDelta.Normalize();

            Q.Setup( vector3(0,1,0), StrutDelta );

            matrix4 NewChildL2W = ParentL2W_World;

            NewChildL2W.Rotate( Q );

            NewChildL2W.SetTranslation( P.m_FreeEnd );  

            NewChildL2W = NewChildL2W * P.m_RelativeTransform;
            
            pTargetObj->OnTransform( NewChildL2W );


            if( pTargetObj->IsKindOf(actor::GetRTTI()) )
            {            
                if( m_bFirstTimeThrough )
                {                
                    firstTimeFired = TRUE;
                    actor& pTargetActor = actor::GetSafeType( *pTargetObj );
                    pTargetActor.SetZone1( GetZone1() );
                    pTargetActor.SetZone2( GetZone2() );
                    pTargetActor.InitZoneTracking();
                }
            }
            else
            {
                pTargetObj->SetZone1( NewZone1 );
                pTargetObj->SetZone2( NewZone2 );           
            }
        }
        else
        {
            if (m_Child[i].m_bAttachPointDirty)
                continue;

            object* pTargetObj = g_ObjMgr.GetObjectByGuid( m_Child[i].m_Guid );
            if (NULL == pTargetObj)
                continue;

            if (!pTargetObj->GetAttachPointData( m_Child[i].m_AttachPtID, ChildL2W ))
                continue;

            ChildL2W.Identity();

            if (m_Child[i].m_bApplyPosition && m_Child[i].m_bApplyRotation)
            {
                ChildL2W.SetTranslation( m_Child[i].m_RelativePos );
                ChildL2W.SetRotation( m_Child[i].m_RelativeRot );
                ChildL2W = ParentL2W * ChildL2W;
            }
            else if (m_Child[i].m_bApplyPosition && !m_Child[i].m_bApplyRotation)
            {
                ChildL2W.SetTranslation( m_Child[i].m_RelativePos );
                ChildL2W = ParentL2W * ChildL2W;

                radian3 ChildBaseRot = pTargetObj->GetL2W().GetRotation();

                ChildL2W.SetRotation(ChildBaseRot);
            }
            else if (!m_Child[i].m_bApplyPosition && m_Child[i].m_bApplyRotation)
            {
                ChildL2W.SetRotation( m_Child[i].m_RelativeRot );
                ChildL2W = ParentL2W * ChildL2W;

                vector3 ChildBasePos = pTargetObj->GetL2W().GetTranslation();

                ChildL2W.SetTranslation(ChildBasePos);
            }
            else
            {
                continue;
            }

            //SH: If things start getting screwed up and matrices begin
            //    coming unglued (ie: non orthonormal), put this line in.
            //    First noticed with the turrets in the alamo level scaling
            //    outside the spatial database boundaries.
            //ChildL2W.Orthogonalize();

            pTargetObj->OnAttachedMove( m_Child[i].m_AttachPtID, ChildL2W );

            if( pTargetObj->IsKindOf(actor::GetRTTI()) )
            {            
                if( m_bFirstTimeThrough )
                {                
                    firstTimeFired = TRUE;
                    actor& pTargetActor = actor::GetSafeType( *pTargetObj );
                    pTargetActor.SetZone1( GetZone1() );
                    pTargetActor.SetZone2( GetZone2() );
                    pTargetActor.InitZoneTracking();
                }
            }
            else
            {
                pTargetObj->SetZone1( NewZone1 );
                pTargetObj->SetZone2( NewZone2 );
            }
        }                
    }  

    if( firstTimeFired )
    {    
        m_bFirstTimeThrough = FALSE;
    }

    m_LastParentL2W = ParentL2W;
}

//=========================================================================

void coupler::OnActivate( xbool bFlag )
{
    m_bFirstTimeThrough = TRUE;

    object::OnActivate(bFlag);
}

//=========================================================================

void coupler::UpdateAttachPoints( void )
{
    if (m_bParentAttachPointDirty)
    {
        if ( m_ParentGuid != 0 )
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
            if (pObj && (m_ParentAttachPointString!=-1))
            {
                m_ParentID = pObj->GetAttachPointIDByName( g_StringMgr.GetString( m_ParentAttachPointString ));
                m_bParentAttachPointDirty = FALSE;

                matrix4 ParentL2W;
                if (pObj->GetAttachPointData( m_ParentID, ParentL2W, object::ATTACH_USE_WORLDSPACE ))
                {
                    m_LastParentL2W = ParentL2W;
                }
            }
        }
    }

    s32 i;
    for (i=0;i<m_nChild;i++)
    {
        if (m_Child[i].m_bAttachPointDirty)
        {
            if ( m_Child[i].m_Guid != 0 )
            {
                object* pObj = g_ObjMgr.GetObjectByGuid( m_Child[i].m_Guid );
                if (pObj)
                {
                    m_Child[i].m_AttachPtID = pObj->GetAttachPointIDByName( g_StringMgr.GetString( m_Child[i].m_AttachPointString ));
                    m_Child[i].m_bAttachPointDirty = FALSE;
                }
            }
        }
    }
}

//=========================================================================

void coupler::UpdateRelativeData( s32 iChild )
{

    if ((iChild < 0) || (iChild >= COUPLER_MAX_CHILDREN ))
        return;

    UpdateAttachPoints();


    if (( m_ParentGuid != 0 ) && (m_Child[iChild].m_Guid != 0 ))
    {
        object* pDestObj   = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
        object* pTargetObj = g_ObjMgr.GetObjectByGuid( m_Child[iChild].m_Guid );

        if (pDestObj && pTargetObj)
        {
            matrix4 ParentL2W, ChildL2W;

            xbool bDestOk = pDestObj->GetAttachPointData( m_ParentID, ParentL2W );
            xbool bTargetOk = pTargetObj->GetAttachPointData( m_Child[iChild].m_AttachPtID, ChildL2W );            

            if (bDestOk && bTargetOk)
            {                      
                ParentL2W.InvertRT();

                matrix4 Res = ParentL2W * ChildL2W;

                m_Child[iChild].m_RelativePos = Res.GetTranslation();
                m_Child[iChild].m_RelativeRot = Res.GetRotation();

                if (m_bChildPhysics && m_pChildPhys)
                {
                    // Get the strut length 
                    pDestObj->GetAttachPointData  ( m_ParentID, ParentL2W, object::ATTACH_USE_WORLDSPACE );                    

                    vector3 ParentAP = ParentL2W.GetTranslation();                    
                    bbox    Box      = pTargetObj->GetBBox();
                    vector3 FreeEnd  = Box.GetCenter();

                    m_pChildPhys[ iChild ].m_FreeEnd     = FreeEnd;
                    m_pChildPhys[ iChild ].m_StrutLength = (FreeEnd - ParentAP).Length();                  
                    m_pChildPhys[ iChild ].m_LastVelocity.Set(0,0,0);

                    vector3 Strut = ParentAP - FreeEnd;


                    matrix4 StrutL2W;

                    StrutL2W.Identity();
                    
                    vector3 StrutX,StrutY,StrutZ;
                    StrutY = Strut;
                    StrutY.Normalize();

                    StrutZ = vector3(1,0,0).Cross(StrutY);
                    StrutX = StrutY.Cross(StrutZ);

                    StrutL2W.SetColumns( StrutX, StrutY, StrutZ );
                    StrutL2W.SetTranslation( FreeEnd );

                    StrutL2W.InvertRT();
                    
                    pTargetObj->GetAttachPointData( m_Child[iChild].m_AttachPtID, ChildL2W, object::ATTACH_USE_WORLDSPACE );            
                    
                    m_pChildPhys[ iChild ].m_RelativeTransform = StrutL2W * ChildL2W;
                }
            }
        }                
    }   
}

//=========================================================================

void coupler::SnapAttachPoints( s32 iChild )
{
    if ((iChild < 0) || (iChild >= COUPLER_MAX_CHILDREN ))
        return;

    if (( m_ParentGuid != 0 ) && (m_Child[iChild].m_Guid != 0 ))
    {
        object* pParentObj   = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
        object* pChildObj = g_ObjMgr.GetObjectByGuid( m_Child[iChild].m_Guid );

        if (pParentObj && pChildObj)
        {
            matrix4 ParentL2W, ChildL2W;

            xbool bParentOk   = pParentObj->GetAttachPointData  ( m_ParentID, ParentL2W, object::ATTACH_USE_WORLDSPACE );
            xbool bChildOk = pChildObj->GetAttachPointData( m_Child[iChild].m_AttachPtID, ChildL2W, object::ATTACH_USE_WORLDSPACE );            

            if (bParentOk && bChildOk)
            {                      
                vector3 ParentAPPos = ParentL2W.GetTranslation();
                vector3 ParentObjPos = pParentObj->GetL2W().GetTranslation();

                // Get the actual L2W 
                pParentObj->GetAttachPointData( m_ParentID, ParentL2W );            
                pChildObj->GetAttachPointData( m_Child[iChild].m_AttachPtID, ChildL2W );            
           
                ParentL2W.InvertRT();

                matrix4 Res = ParentL2W * ChildL2W;

                vector3 ParentDelta = ParentAPPos - ParentObjPos;
                
                m_Child[iChild].m_RelativePos = ParentDelta;
                m_Child[iChild].m_RelativeRot = Res.GetRotation();
            }
        }                
    }   
}

//=========================================================================

xbool coupler::AddChild( guid ChildGuid, 
                       const char* AttachPoint )
{
    s32 iChild = GetChildIDByGuid( ChildGuid );
    if (iChild != -1)
    {
        m_Child[ iChild ].SetAttachPoint( AttachPoint );        
    }
    else
    {
        // No existing child by that guid
        if (m_nChild == COUPLER_MAX_CHILDREN)
        {
            iChild = -1;
        }
        else
        {
            iChild = m_nChild;
            m_Child[ iChild ].Reset();
            m_Child[ iChild ].m_Guid = ChildGuid;
            m_Child[ iChild ].SetAttachPoint( AttachPoint );
            m_nChild++;
        }
    }

    if (iChild != -1)
    {    
        UpdateAttachPoints();
        return TRUE;
    }
    
    return FALSE;
}

//=========================================================================

xbool coupler::RemoveChild( guid ChildGuid )
{
    s32 iChild = GetChildIDByGuid( ChildGuid );
    if (iChild == -1)
    {   
        return FALSE;
    }

    s32 j;
    for (j=iChild;j<m_nChild-1;j++)
    {
        m_Child[j] = m_Child[j+1];
    }
    m_nChild--;
    for (j=m_nChild;j<COUPLER_MAX_CHILDREN;j++)
    {
        m_Child[j].Reset();
    }

    return TRUE;
}

//=========================================================================

void coupler::SnapAttachPoints( guid ChildGuid )
{
    s32 iChild = GetChildIDByGuid( ChildGuid );
    if (iChild != -1)
    {
        SnapAttachPoints( iChild );
    }
}

//=========================================================================

void coupler::UpdateRelative( guid ChildGuid )
{
    s32 iChild = GetChildIDByGuid( ChildGuid );
    if (iChild != -1)
    {
        UpdateRelativeData( iChild );
    }
}

//=========================================================================

void coupler::UpdateAllRelative( void )
{
    s32 i;
    for (i=0;i<m_nChild;i++)
    {
        UpdateRelativeData(i);
    }
}

//=========================================================================

s32 coupler::GetChildIDByGuid( guid ChildGuid )
{
    s32 i;
    for (i=0;i<m_nChild;i++)
    {
        if (m_Child[i].m_Guid == ChildGuid)
            return i;
    }
    return -1;
}

//=========================================================================

void coupler::AdvancePhysics( f32 DeltaTime )
{
    if (DeltaTime == 0)
        return;
    if (m_ParentGuid == 0)
        return;
    if (NULL == m_pChildPhys)
        return;

    matrix4 ParentL2W;

    object* pDestObj = g_ObjMgr.GetObjectByGuid( m_ParentGuid );
    if (NULL == pDestObj)
        return;

    if (m_bParentAttachPointDirty || (m_Child[0].m_bAttachPointDirty) ||
        (m_Child[1].m_bAttachPointDirty) ||
        (m_Child[2].m_bAttachPointDirty) ||
        (m_Child[3].m_bAttachPointDirty))
    {
        UpdateAttachPoints();
    }

    if (m_bParentAttachPointDirty)
        return;

    if (!pDestObj->GetAttachPointData( m_ParentID, ParentL2W ))
        return;

    vector3 ParentAP = ParentL2W.GetTranslation();

    vector3 ParentDelta = ParentAP - m_LastParentL2W.GetTranslation();
    f32     ParentDeltaSpeed;
    
    if (DeltaTime == 0)
        ParentDeltaSpeed = 0;
    else
        ParentDeltaSpeed = ParentDelta.Length() / DeltaTime;    

    if (ParentDeltaSpeed > 5000.0f)
    {
        // Consider this a disjoint movement.
        // Don't apply normal physics.
        // Treat this as a teleport.
        s32 i;
        for (i=0;i<m_nChild;i++)
        {
            child_phys& P = m_pChildPhys[i];
            child&      C = m_Child[i];

            object* pChild = g_ObjMgr.GetObjectByGuid( C.m_Guid );
            if (NULL == pChild)
                continue;
            
            vector3 Delta = P.m_FreeEnd - m_LastParentL2W.GetTranslation();;

            P.m_FreeEnd = ParentAP + Delta;
        }
        
        m_LastParentL2W = ParentL2W;

        return;
    }   

    m_LastParentL2W = ParentL2W;

    s32 i;
    for (i=0;i<m_nChild;i++)
    {
        child_phys& P = m_pChildPhys[i];
        child&      C = m_Child[i];

        object* pChild = g_ObjMgr.GetObjectByGuid( C.m_Guid );
        if (NULL == pChild)
            continue;

        P.m_LastVelocity *= (P.m_Dampening * DeltaTime);

        vector3  OrigFreeEndPos = P.m_FreeEnd;

        // Apply simple forces
        vector3 G(0,-981,0);

        G *= (DeltaTime*DeltaTime) * P.m_Mass;

        P.m_FreeEnd += G;
        P.m_FreeEnd += P.m_LastVelocity;

        // Apply constraints
        vector3 Delta = P.m_FreeEnd - ParentAP;

        vector3 Correct = Delta;
        Correct.NormalizeAndScale( P.m_StrutLength );
        
        P.m_FreeEnd = ParentAP + Correct; 

        P.m_LastVelocity = P.m_FreeEnd - OrigFreeEndPos;
        P.m_LastVelocity /= DeltaTime;
    }
}

//=========================================================================

//=========================================================================

//=========================================================================
