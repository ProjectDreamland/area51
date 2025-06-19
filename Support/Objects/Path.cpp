//=========================================================================
// INCLUDES
//=========================================================================

#include "Entropy.hpp"
#include "Path.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"


#ifdef X_EDITOR
#include <Windows.h>
#endif // X_EDITOR

//=========================================================================
// DEFINES
//=========================================================================

#define KEY_RADIUS          20
#define PATH_RADIUS         50
#define JUST_AHEAD_TIME     (0.05f)

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct path_desc : public object_desc
{
    path_desc( void ) : object_desc( 
            object::TYPE_PATH, 
            "Path", 
            "SCRIPT",

#ifdef X_EDITOR
            object::ATTR_COLLIDABLE          | 
            object::ATTR_COLLISION_PERMEABLE |
            object::ATTR_SPACIAL_ENTRY       |
            object::ATTR_RENDERABLE,
#else // X_EDITOR
            0,  // Doesn't have to do anything during game
#endif // X_EDITOR

            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_NO_ICON               |
            FLAGS_TARGETS_OBJS          |
            FLAGS_IS_DYNAMIC ) 
    {
        m_SegmentDensityMultiplier = 10;
    }

    //---------------------------------------------------------------------
    
    virtual object* Create          ( void ) { return new path; }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32     OnEditorRender  ( object& Object ) const 
    { 
        s32 Icon = -1;

        // Is this a path with no keys?
        object_ptr<path> pPath(Object.GetGuid()) ;
        if ((pPath) && (pPath->GetKeyCount() == 0))
            Icon = EDITOR_ICON_PATH; 

        // Selected or placing?
        if ( Object.GetAttrBits() & (object::ATTR_EDITOR_SELECTED | object::ATTR_EDITOR_PLACEMENT_OBJECT))
            Icon = EDITOR_ICON_PATH; 

        // No icon for populated unselected paths
        if (pPath)
            pPath->RenderPath();

        return Icon;
    }

    //---------------------------------------------------------------------

    virtual void    OnEditorBandSelected( object& Object, plane DragSelect[5] ) const
    {
        // This should be a path
        object_ptr<path> pPath(Object.GetGuid()) ;
        if (!pPath)
            return ;

        // Select/deselect all the keys
        for (s32 i = 0 ; i < pPath->GetKeyCount() ; i++)
        {
            // Lookup key
            path::key& Key = pPath->GetKey(i) ;

            // Select the key
            Key.m_Flags |= path::key::FLAG_SELECTED ;

            // Check all planes of band volume
            for (s32 j = 0 ; j < 5 ; j++)
            {
                // Outside of band volume?
                if (DragSelect[j].InBack(Key.m_Position))
                {
                    // Deselect the key and get out of here!
                    Key.m_Flags &= ~path::key::FLAG_SELECTED ;
                    continue ;
                }
            }
            
            // Put path icon at key
            if( Key.m_Flags & path::key::FLAG_SELECTED )
                pPath->SetPathIconPosition( i );                
        }
    }

    //---------------------------------------------------------------------

    virtual void    OnEditorSelectAll( object& Object ) const
    {
        // This should be a path
        object_ptr<path> pPath(Object.GetGuid()) ;
        if (!pPath)
            return ;

        // Select all the keys
        for (s32 i = 0 ; i < pPath->GetKeyCount() ; i++)
            pPath->GetKey(i).m_Flags |= path::key::FLAG_SELECTED ;
            
        // Put path icon at first key
        pPath->SetPathIconPosition( 0 );                
    }

#endif X_EDITOR

    //---------------------------------------------------------------------
    virtual void    OnEnumProp  ( prop_enum&  List  );
    virtual xbool   OnProperty  ( prop_query& I     );
    //---------------------------------------------------------------------

    s32         GetSegmentDensityMultiplier( void ) const
    {
        // This whole segment density thing is just a stopgap.
        // The editor runs incredibly slowly with long/large paths.
        // The path rendering should probably be doing a fixed distance step
        // along the spline, or a delta-error base subdivision...
        return m_SegmentDensityMultiplier;
    }

    //---------------------------------------------------------------------
    //  DATA   DATA   DATA   DATA   DATA   DATA   DATA   DATA   DATA   DATA   
    //---------------------------------------------------------------------
public:

    s32       m_SegmentDensityMultiplier;

} s_path_Desc;

//=========================================================================

const object_desc& path::GetTypeDesc( void ) const
{
    return s_path_Desc;
}

//=========================================================================

const object_desc& path::GetObjectType( void )
{
    return s_path_Desc;
}

//========================================================================

void path_desc::OnEnumProp( prop_enum& List )
{
    object_desc::OnEnumProp( List );

    List.PropEnumHeader( "Path", "Editor Path Properties", 0 );
    List.PropEnumInt   ( "Path\\Segment Density",  "Larger number shows more detail on the path in editor", 0 );    
}

//========================================================================

xbool path_desc::OnProperty( prop_query& I )
{
    if (object_desc::OnProperty( I ))
    {
    }  
    else if ( I.VarInt("Path\\Segment Density", m_SegmentDensityMultiplier ))
    {
        return TRUE;
    }
    else
    {   
        return FALSE;
    }

    return TRUE;
}


//=========================================================================
// UTILITY FUNCTIONS
//=========================================================================

// A simple function to set/get a flag bit property
static void OnPropertyVarBool( prop_query& I, u32& Flags, u32 FlagMask )
{
    // Setting up UI?
    if (I.IsRead())
        I.SetVarBool((Flags & FlagMask) != 0) ;
    else
    {
        // Clear flag
        Flags &= ~FlagMask ;

        // Set flag bits?
        if (I.GetVarBool())
            Flags |= FlagMask ;
    }
}

//=========================================================================

// Returns spline position
static vector3 GetSplinePos( const vector3& P0,
                             const vector3& V0,
                             const vector3& P1,
                             const vector3& V1,
                             f32 T )
{
    // Compute time powers
    f32 T2   = T*T ;
    f32 T3   = T2*T ;
    f32 T3x2 = T3*2 ;
    f32 T2x3 = T2*3 ;

    // Compute coefficients
    f32 w0 =  T3x2 - T2x3 + 1.0f ;
    f32 w1 = -T3x2 + T2x3 ;    
    f32 w2 =  T3  - (2.0f*T2) + T ; 
    f32 w3 =  T3  -  T2 ;   
    
    // Compute final spline position
    return ( (w0*P0) + (w1*P1) + (w2*V0) + (w3*V1) ) ;
}

//=========================================================================
// KEY FUNCTIONS
//=========================================================================

// Constructor
path::key::key()
{
    SetDefaults() ;
}

//=========================================================================

// Enumerates key properties
void path::key::OnEnumProp( prop_enum& List, path& Path, s32 iKey, u32 PresentFlags, u32 NotPresentFlags )
{
    // Lookup path info
    u32 PathFlags      = Path.GetFlags() ;
    s32 nSelectedCount = Path.GetSelectedKeyCount();

    // Allow creation only if 1 key is selected
    u32 CreateBtnFlags = PROP_TYPE_MUST_ENUM ;
    if ( ( iKey == -1 ) && ( nSelectedCount > 1 ) )
        CreateBtnFlags |= PROP_TYPE_READ_ONLY ;

#ifdef X_EDITOR

    // Add buttons
    List.PropEnumButton ("CreateKeyBefore",  "Inserts a new key before this one.", CreateBtnFlags);
    List.PropEnumButton ("CreateKeyAfter",   "Inserts a new key after  this one.", CreateBtnFlags);
    List.PropEnumButton ("DeleteKey",        "Deletes the selected key(s)",        PROP_TYPE_MUST_ENUM);

#endif

    // Selected flag
    List.PropEnumBool("Selected",          "Editor selection state.",           PresentFlags) ;
    
    // Corner flag?
    if (Path.GetCurveType() == path::CURVE_TYPE_SPLINE)
        List.PropEnumBool("Corner", "Key is a sharp corner instead of auto spline curved", PresentFlags) ;
    else
        List.PropEnumBool("Corner", "Key is a sharp corner instead of auto spline curved", NotPresentFlags) ;
    
    // Deactivate tracker flag
    List.PropEnumBool("DeactivateTracker", "Tracker will deactive when it reaches this key",             PresentFlags) ;
    
    // Activate/deactivate object flags?
    if (PathFlags & path::FLAG_KEY_OBJECT_GUID)
    {
        List.PropEnumBool("ActivateObject",    "Object will activate when the tracker reaches this key",     PresentFlags) ;
        List.PropEnumBool("DeactivateObject",  "Object will de-activate when the tracker reaches this  key", PresentFlags) ;
    }
    else
    {
        List.PropEnumBool("ActivateObject",    "Object will activate when the tracker reaches this key",     NotPresentFlags) ;
        List.PropEnumBool("DeactivateObject",  "Object will de-activate when the tracker reaches this  key", NotPresentFlags) ;
    }

#ifdef X_EDITOR

    // Show absolute time if only one key is selected, or this is not a multi-key
    if ( Path.GetAttrBits() & (object::ATTR_EDITOR_SELECTED | object::ATTR_EDITOR_PLACEMENT_OBJECT))
    {
        if( ( nSelectedCount == 1 ) && ( iKey >= 1 ) )
            List.PropEnumFloat("Time", "Absolute time of key along path.", PresentFlags | PROP_TYPE_MUST_ENUM | PROP_TYPE_DONT_EXPORT | PROP_TYPE_DONT_SAVE );
    }

#endif

    // DeltaTime? (assumed zero for first key)
    if( iKey != 0 )
    {
        if (PathFlags & path::FLAG_KEY_DELTA_TIME)
            List.PropEnumFloat("DeltaTime", "Delta time since last key.", PresentFlags) ;
        else
            List.PropEnumFloat("DeltaTime", "Delta time since last key.", NotPresentFlags) ;
    }
            
    // Position?
    if (PathFlags & path::FLAG_KEY_POSITION)
        List.PropEnumVector3("Position", "World position.", PresentFlags) ;
    else
        List.PropEnumVector3("Position", "World position.", NotPresentFlags) ;

    // Rotation?
    if (PathFlags & path::FLAG_KEY_ROTATION)
        List.PropEnumRotation("Rotation", "World rotation.", PresentFlags) ;
    else
        List.PropEnumRotation("Rotation", "World rotation.", NotPresentFlags) ;

    // FieldOfView?
    if (PathFlags & path::FLAG_KEY_FIELD_OF_VIEW)
        List.PropEnumAngle("FieldOfView", "Camera field of view.", PresentFlags) ;
    else
        List.PropEnumAngle("FieldOfView", "Camera field of view.", NotPresentFlags) ;

    // Roll?
    if (PathFlags & path::FLAG_KEY_ROLL)
        List.PropEnumAngle("Roll", "Camera roll.", PresentFlags) ;
    else
        List.PropEnumAngle("Roll", "Camera roll.", NotPresentFlags) ;
    
    // Color?
    if (PathFlags & path::FLAG_KEY_COLOR)
        List.PropEnumColor("Color", "Color.", PresentFlags) ;
    else
        List.PropEnumColor("Color", "Color.", NotPresentFlags) ;

    // Object guid?
    if (PathFlags & path::FLAG_KEY_OBJECT_GUID)
        List.PropEnumGuid("ObjectGuid", "Guid of object that can be activated/de-activeated.", PresentFlags) ;
    else
        List.PropEnumGuid("ObjectGuid", "Guid of object that can be activated/de-activeated.", NotPresentFlags) ;
}

//=========================================================================

// Evaluates key properties
xbool path::key::OnProperty( prop_query& I, path& Path, s32 iKey )
{
#ifndef X_EDITOR
    (void)iKey;
#endif
    
#ifdef X_EDITOR

    // Create key after?
    if( I.IsVar("CreateKeyAfter"))
    {
        if (I.IsRead())
            I.SetVarButton( "CreateKeyAfter" );
        else
            Path.CreateKey(iKey, TRUE) ;

        return TRUE ;
    }

    // Create key before?
    if( I.IsVar("CreateKeyBefore"))
    {
        if (I.IsRead())
            I.SetVarButton( "CreateKeyBefore" );
        else
            Path.CreateKey(iKey, FALSE) ;

        return TRUE ;
    }

    // Delete key?
    if( I.IsVar("DeleteKey"))
    {
        if (I.IsRead())
        {
            I.SetVarButton( "DeleteKey(s)" );
        }
        else
        {
            ASSERT(iKey != -1) ;
            Path.DeleteKey(iKey) ;
        }

        return TRUE ;
    }
#endif  //#ifdef X_EDITOR

    // Selected?
    if (I.IsVar("Selected"))
    {
        OnPropertyVarBool(I, m_Flags, key::FLAG_SELECTED) ;
        return TRUE ;            
    }

    // Corner?
    if (I.IsVar("Corner"))
    {
        OnPropertyVarBool(I, m_Flags, key::FLAG_CORNER) ;
        return TRUE ;            
    }

    // DeactivateTracker?
    if (I.IsVar("DeactivateTracker"))
    {
        OnPropertyVarBool(I, m_Flags, key::FLAG_DEACTIVATE_TRACKER) ;
        return TRUE ;            
    }

    // ActivateObject?
    if (I.IsVar("ActivateObject"))
    {
        OnPropertyVarBool(I, m_Flags, key::FLAG_ACTIVATE_OBJECT) ;
        return TRUE ;            
    }

    // DeactivateObject?
    if (I.IsVar("DeactivateObject"))
    {
        OnPropertyVarBool(I, m_Flags, key::FLAG_DEACTIVATE_OBJECT) ;
        return TRUE ;            
    }

#ifdef X_EDITOR

    // Time?
    if ( I.IsVar( "Time" ) )
    {
        // Get current key absolute time
        ASSERT( iKey >= 0 );
        ASSERT( iKey < Path.m_Keys.GetCount() );
        f32 KeyTime = Path.GetKeyTime( iKey );
        
        // Setup UI?
        if( I.IsRead() )
            I.SetVarFloat( KeyTime );
        else
        {
            // Update keys delta time so absolute time is set
            f32 NewKeyTime = I.GetVarFloat( 0.0f, F32_MAX );
            Path.GetKey( iKey ).m_DeltaTime += NewKeyTime - KeyTime;
        }            
        
        return TRUE ;
    }

#endif  //#ifdef X_EDITOR

    // DeltaTime?
    if (I.VarFloat("DeltaTime", m_DeltaTime))
        return TRUE ;
    
    // Position?
    if (I.VarVector3("Position", m_Position))
    {
        // Force update of bounds
        if (I.IsRead() == FALSE)
            Path.UpdateBounds() ;

        return TRUE ;
    }

    // Rotation?
    if (I.IsVar("Rotation"))
    {
        if (I.IsRead())
            I.SetVarRotation(radian3(m_Rotation)) ;
        else
            m_Rotation.Setup(I.GetVarRotation()) ;
        return TRUE ;
    }

    // FieldOfView?
    if (I.VarAngle("FieldOfView", m_FieldOfView))
        return TRUE ;

    // Roll?
    if (I.VarAngle("Roll", m_Roll))
        return TRUE ;
    
    // Color?
    if (I.VarColor("Color", m_Color))
        return TRUE ;

    // ObjectGuid?
    if (I.VarGUID("ObjectGuid", m_ObjectGuid))
        return TRUE ;

    // Not found
    return FALSE ;
}

//=========================================================================

// Sets up defaults
void path::key::SetDefaults( void )
{
    m_Flags       = 0 ;
    m_DeltaTime   = 1.0f ;
    m_Position.Zero() ;
    m_Rotation.Identity() ;
    m_FieldOfView = R_60 ;
    m_Roll        = R_0 ;
    m_Color       = XCOLOR_WHITE ;
    m_ObjectGuid  = 0 ;
}

//=========================================================================

// Accumulated multi-selection key values
void path::key::MultiSelect( const key& Key )
{
    if (m_Flags != Key.m_Flags)
        m_Flags = 0xFFFFFFFF ;

    if (m_DeltaTime != Key.m_DeltaTime)
        m_DeltaTime = F32_MAX ;

    if (m_Position != Key.m_Position)
        m_Position.Zero() ;

    if (m_Rotation.Difference(Key.m_Rotation) > 0.0001f)
        m_Rotation.Identity() ;
    
    if (m_FieldOfView != Key.m_FieldOfView)
        m_FieldOfView = F32_MAX ;

    if (m_Roll != Key.m_Roll)
        m_Roll = F32_MAX ;

    if (m_Color != Key.m_Color)
        m_Color = XCOLOR_WHITE ;

    if (m_ObjectGuid != Key.m_ObjectGuid)
        m_ObjectGuid = 0 ;
}

//=========================================================================

// Interpolates between keys
void path::key::Interpolate( const path& Path,
                                   s32   iKeyA, f32 KeyATime,
                                   s32   iKeyB, f32 KeyBTime,
                                   f32   T,
                                   f32   PrevTime, 
                                   f32   CurrTime )
{
    // Lookup keys
    const key& KeyA = Path.GetKey(iKeyA) ;
    const key& KeyB = Path.GetKey(iKeyB) ;

    // Begin with defaults
    SetDefaults() ;

    // Interpolate so:  
    //      When T=0, the Key=KeyA
    //      When T=1, the Key=KeyB

    //m_Rotation    = Blend(KeyA.m_Rotation, KeyB.m_Rotation, T) ;
    m_FieldOfView = KeyA.m_FieldOfView + (T * (KeyB.m_FieldOfView - KeyA.m_FieldOfView)) ;
    m_Roll        = KeyA.m_Roll + (T * (KeyB.m_Roll - KeyA.m_Roll)) ;
    m_Color.R     = (u8)((f32)KeyA.m_Color.R + (T * (f32)(KeyB.m_Color.R - KeyA.m_Color.R))) ;
    m_Color.G     = (u8)((f32)KeyA.m_Color.G + (T * (f32)(KeyB.m_Color.G - KeyA.m_Color.G))) ;
    m_Color.B     = (u8)((f32)KeyA.m_Color.B + (T * (f32)(KeyB.m_Color.B - KeyA.m_Color.B))) ;
    m_Color.A     = (u8)((f32)KeyA.m_Color.A + (T * (f32)(KeyB.m_Color.A - KeyA.m_Color.A))) ;

    vector3 AlternatePosition;  

    // Use spline position?
    if (Path.GetCurveType() == path::CURVE_TYPE_SPLINE)
    {
        // Compute key tangents
        vector3 KeyATangent = Path.GetKeyTangent(iKeyA) ;
        vector3 KeyBTangent = Path.GetKeyTangent(iKeyB) ;

        // Compute spline position
        m_Position = GetSplinePos( KeyA.m_Position, KeyATangent,
                                   KeyB.m_Position, KeyBTangent, T ) ;

        if (T < 1)
        {
            AlternatePosition = GetSplinePos( KeyA.m_Position, KeyATangent,
                                              KeyB.m_Position, KeyBTangent, T+JUST_AHEAD_TIME ) ;
        }
        else
        {   
            AlternatePosition = GetSplinePos( KeyA.m_Position, KeyATangent,
                                              KeyB.m_Position, KeyBTangent, T-JUST_AHEAD_TIME ) ;
        }
    }
    else
    {
        // Use linear position
        m_Position = KeyA.m_Position + (T * (KeyB.m_Position - KeyA.m_Position)) ;
        if (T < 1)
            AlternatePosition = KeyA.m_Position + ((T+JUST_AHEAD_TIME) * (KeyB.m_Position - KeyA.m_Position)) ;
        else
            AlternatePosition = KeyA.m_Position + ((T+JUST_AHEAD_TIME) * (KeyB.m_Position - KeyA.m_Position)) ;

    }

    // Compute rotation so that we are facing the position just ahead of where we currently are
    if (T < 1)
    {
        // Alternate position is ahead of current
        vector3 Delta = AlternatePosition - m_Position;
        radian3 Rot;
        Rot.Roll = 0;
        Delta.GetPitchYaw( Rot.Pitch, Rot.Yaw );

        m_Rotation.Setup( Rot );
    }
    else
    {
        // Alternate position is behind current
        vector3 Delta = m_Position - AlternatePosition;
        radian3 Rot;
        Rot.Roll = 0;
        Delta.GetPitchYaw( Rot.Pitch, Rot.Yaw );

        m_Rotation.Setup( Rot );
    }

    // Clear flags
    m_Flags = 0 ;

    // Collect the flags only when the keys have been crossed.
    // NOTE: The current way these are setup are so that a tracker
    //       can start active on a key that has a deactivate tracker
    //       event. If you change the logic below you will break this!!!

    // Playing forwards?
    if (CurrTime >= PrevTime)
    {
        // Did tracker cross KeyA?
        if ((KeyATime > PrevTime) && (KeyATime <= CurrTime))
        {
            m_Flags      = KeyA.m_Flags ;
            m_ObjectGuid = KeyA.m_ObjectGuid ;
            KeyA.m_Flags |= key::FLAG_RENDER_OBJECT_EVENT ;
        }

        // Did tracker cross KeyB?
        if ((KeyBTime > PrevTime) && (KeyBTime <= CurrTime))
        {
            m_Flags      = KeyB.m_Flags ;
            m_ObjectGuid = KeyB.m_ObjectGuid ;
            KeyB.m_Flags |= key::FLAG_RENDER_OBJECT_EVENT ;
        }
    }
    else
    {
        // Did tracker cross KeyA?
        if ((KeyATime >= CurrTime) && (KeyATime < PrevTime))
        {
            m_Flags      = KeyA.m_Flags ;
            m_ObjectGuid = KeyA.m_ObjectGuid ;
            KeyA.m_Flags |= key::FLAG_RENDER_OBJECT_EVENT ;
        }

        // Did tracker cross KeyB?
        if ((KeyBTime >= CurrTime) && (KeyBTime < PrevTime))
        {
            m_Flags      = KeyB.m_Flags ;
            m_ObjectGuid = KeyB.m_ObjectGuid ;
            KeyB.m_Flags |= key::FLAG_RENDER_OBJECT_EVENT ;
        }
    }
}

//=========================================================================

// Render key
void path::key::OnRender( path& Path, s32 iKey )
{
#ifdef X_EDITOR

    // Lookup event info
    xstring EventLabel = "" ;
    vector3 EventStart = m_Position ;
    vector3 EventEnd   = m_Position ;
    if (Path.GetFlags() & path::FLAG_KEY_OBJECT_GUID)
    {
        // Attached to an object?
        object_ptr<object> pObject(m_ObjectGuid) ;
        if (pObject)
        {
            // Activate event?
            if (m_Flags & key::FLAG_ACTIVATE_OBJECT)
                EventLabel = "Activate\n" ;

            // Deactivate event?
            if (m_Flags & key::FLAG_DEACTIVATE_OBJECT)
                EventLabel += "Deactivate\n" ;

            // Update end
            EventEnd = pObject->GetPosition() ;
        }
    }
    vector3 EventPos = (EventStart + EventEnd) * 0.5f ;

    // Is path selected?
    if (Path.GetAttrBits() & ATTR_EDITOR_SELECTED)
    {
        // Setup color
        xcolor Color = (m_Flags & key::FLAG_SELECTED) ? XCOLOR_YELLOW : XCOLOR_RED ;

        // Draw sphere
        draw_Sphere(m_Position, KEY_RADIUS, Color) ;

        // Draw label
        draw_Label(m_Position - vector3(0, PATH_RADIUS * 1.25f, 0), Color, "Key:%d", iKey) ;

        // Render rotation axis?
        if (Path.GetFlags() & path::FLAG_KEY_ROTATION)
        {
            // Create L2W matrix for key
            matrix4 L2W ;
            L2W.Identity() ;
            L2W.SetRotation(m_Rotation) ;
            L2W.SetTranslation(m_Position) ;

            // Draw axis
            draw_SetL2W(L2W) ;
            draw_Axis(KEY_RADIUS*5.0f) ;
            draw_ClearL2W() ;
        }

        // Draw event(s)?
        if ( (m_Flags & key::FLAG_SELECTED) && (EventLabel.GetLength()) )
        {
            draw_Line (EventStart, EventEnd, XCOLOR_AQUA) ;
            draw_Label(EventPos, XCOLOR_AQUA, EventLabel) ;
        }
    }
    else
    {
        // Draw sphere
        draw_Sphere(m_Position, KEY_RADIUS, XCOLOR_GREEN) ;

        // Draw event?
        if ( (m_Flags & key::FLAG_RENDER_OBJECT_EVENT) && (EventLabel.GetLength()) )
        {
            draw_Line (EventStart, EventEnd, XCOLOR_AQUA) ;
            draw_Label(EventPos, XCOLOR_AQUA, EventLabel) ;

            // Clear
            m_Flags &= ~key::FLAG_RENDER_OBJECT_EVENT ;
        }
    }
#else // X_EDITOR
    (void)Path ;
    (void)iKey ;
#endif  // X_EDITOR
}

//=========================================================================
// PATH FUNCTIONS
//=========================================================================

path::path( void )
{
    m_Flags     = path::FLAG_KEY_DELTA_TIME | path::FLAG_KEY_POSITION ; // Flags
    m_CurveType = CURVE_TYPE_SPLINE ;                       // Type of curve
    m_bPathOn   = TRUE;
}

//=============================================================================

bbox path::GetLocalBBox( void ) const
{
    bbox BBox ;
    BBox.Clear() ;
    
    // Add all keys (in local space!) to bounds
    BBox += vector3(0,0,0) ;
    for (s32 i = 0 ; i < m_Keys.GetCount() ; i++)
        BBox += m_Keys[i].m_Position - GetPosition() ;

    // Inflate for selection purposes
    BBox.Inflate(KEY_RADIUS, KEY_RADIUS, KEY_RADIUS) ;
    BBox.Inflate(PATH_RADIUS, PATH_RADIUS, PATH_RADIUS) ;

    return BBox ;
}

//=============================================================================

void path::OnInit( void )
{
}

//=============================================================================

void path::OnColCheck( void )
{
#ifdef X_EDITOR

    s32 i ; 

    g_CollisionMgr.StartApply( GetGuid() );

    // Loop through all keys first
    for (i = 0 ; i < m_Keys.GetCount() ; i++)
        g_CollisionMgr.ApplySphere(m_Keys[i].m_Position, KEY_RADIUS, 0, i) ;

    // Apply main object if we haven't hit ourself yet!
    s32 iHitKey = -1 ;
    for (i = 0 ; i < g_CollisionMgr.m_nCollisions ; i++)
    {
        // Hit key?
        if (g_CollisionMgr.m_Collisions[i].ObjectHitGuid == GetGuid())
        {
            // Flag key is hit!
            iHitKey = g_CollisionMgr.m_Collisions[i].PrimitiveKey ;

            AutoGenerateRotation();

            // Apply key again, but with bigger path icon radius so that
            // for multi overlapping icons, the select logic will work okay
            g_CollisionMgr.ApplySphere(m_Keys[iHitKey].m_Position, PATH_RADIUS, 0, iHitKey) ;
            break ;
        }
    }

    // Apply main object if no keys
    if( m_Keys.GetCount() == 0 )
        g_CollisionMgr.ApplySphere(GetPosition(), PATH_RADIUS, 0, -1) ;

    g_CollisionMgr.EndApply();
#endif // X_EDITOR
}

//=============================================================================

void path::OnColNotify( object& Object )
{
    (void)Object ;

#ifdef X_EDITOR
    s32 i ; 

    // Lookup collision
    const collision_mgr::collision& Collision = g_CollisionMgr.m_Collisions[0] ;

    // Select all keys?
    if (Collision.PrimitiveKey == -1)
    {
        // Select all
        for (i = 0 ; i < m_Keys.GetCount() ; i++)
            m_Keys[i].m_Flags |= key::FLAG_SELECTED ;
    }
    else
    {
        // Put path object at the selected key (but leave the keys where they are)
        SetPathIconPosition( Collision.PrimitiveKey );

        // Holding shift?
        if ( (::GetKeyState( VK_LSHIFT ) & ~1 ) != 0)
        {
            // Find min/max selected keys
            s32 iMin = Collision.PrimitiveKey ;
            s32 iMax = Collision.PrimitiveKey ;
            for (i = 0 ; i < m_Keys.GetCount() ; i++)
            {
                // Selected?
                if (m_Keys[i].m_Flags & key::FLAG_SELECTED)
                {
                    iMin = x_min(iMin, i) ;
                    iMax = x_max(iMax, i) ;
                }
            }

            // Select all between min and max
            for (i = iMin ; i <= iMax ; i++)
                m_Keys[i].m_Flags |= key::FLAG_SELECTED ;
        }
        else
        // Holding tab?
        if ( (::GetKeyState( VK_TAB ) & ~1 ) != 0)
        {
            // Toggle selected key
            m_Keys[Collision.PrimitiveKey].m_Flags ^= key::FLAG_SELECTED ;
        }
        else
        {
            // Deselect all
            for (i = 0 ; i < m_Keys.GetCount() ; i++)
                m_Keys[i].m_Flags &= ~key::FLAG_SELECTED ;

            // Select chosen
            m_Keys[Collision.PrimitiveKey].m_Flags |= key::FLAG_SELECTED ;
        }
    }
#endif // X_EDITOR
}

//=============================================================================

void path::OnEnumProp( prop_enum& List )
{
    s32 i ;

    // Call base class
    object::OnEnumProp( List );

    List.PropEnumHeader("Path", "Properties for the path object", 0 );

    List.PropEnumEnum("Path\\CurveType", "LINEAR\0SPLINE\0",
        "Specifies the type of curve between points.",
        PROP_TYPE_MUST_ENUM);

    // Key flags
    List.PropEnumBool("Path\\KeySelected", 
                  "Keys contain \"Selected\" property.",
                  PROP_TYPE_MUST_ENUM | PROP_TYPE_DONT_EXPORT | PROP_TYPE_DONT_SHOW);
    
    List.PropEnumBool("Path\\KeyDeltaTime", 
                  "Keys contain \"DeltaTime\" property.",
                  PROP_TYPE_MUST_ENUM | PROP_TYPE_DONT_EXPORT | PROP_TYPE_DONT_SHOW);
    
    List.PropEnumBool("Path\\KeyPosition", 
                  "Keys contain \"Position\" property.",
                  PROP_TYPE_MUST_ENUM | PROP_TYPE_DONT_EXPORT | PROP_TYPE_DONT_SHOW);
    
    List.PropEnumBool("Path\\KeyRotation", 
                  "Keys contain \"Rotation\" property.",
                  PROP_TYPE_MUST_ENUM);

    List.PropEnumBool("Path\\Auto Yaw", 
                  "If keys contain \"Rotation\" property, auto generate rotation yaw based on the path",
                  PROP_TYPE_MUST_ENUM);

    List.PropEnumBool("Path\\Auto Pitch", 
                  "If keys contain \"Rotation\" property, auto generate rotation pitch based on the path",
                  PROP_TYPE_MUST_ENUM);
    
    List.PropEnumBool("Path\\KeyFieldOfView", 
                  "Keys contain \"FieldOfView\" property.",
                  PROP_TYPE_MUST_ENUM);
    
    List.PropEnumBool("Path\\KeyRoll", 
                  "Keys contain \"Roll\" property.",
                  PROP_TYPE_MUST_ENUM);
    
    List.PropEnumBool("Path\\KeyColor", 
                  "Keys contain \"Color\" property.",
                  PROP_TYPE_MUST_ENUM);

    List.PropEnumBool("Path\\KeyObjectGuid", 
                  "Keys contain \"ObjectGuid\" property.",
                  PROP_TYPE_MUST_ENUM);

    List.PropEnumInt  ("Path\\KeyCount",  "Number of keys in this path.", PROP_TYPE_MUST_ENUM );

#ifdef X_EDITOR

    // Don't process these if the path isn't physically selected
    if ( GetAttrBits() & (object::ATTR_EDITOR_SELECTED | object::ATTR_EDITOR_PLACEMENT_OBJECT) )
    {
        // Commands
        u32 CommandFlags = PROP_TYPE_MUST_ENUM | PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT;
        List.PropEnumFloat ( "Path\\TotalTime",    "Total time of whole path. Use to set total time for the whole path.", CommandFlags );
        if( m_Keys.GetCount() > 2 )
        {
            List.PropEnumButton( "Path\\SmoothSpeed",  "Smooths speed with regard to time of whole path.  This will get rid of fast and slow segments of the path and make everything smooth", CommandFlags );
            List.PropEnumButton( "Path\\EqualSpacing", "Position keys equal distances apart along whole path.", CommandFlags );
        }
        List.PropEnumButton( "Path\\SelectAllKeys", "Selects all the keys in path.", CommandFlags );
        
        // Add selected key(s) properties only if there are selected keys and the path is selected
        if( GetSelectedKeyCount())             
        {
            // Add header
            List.PropEnumString("Path\\SelectedKey(s)", "Properties of all selected keys. When selecting with left mouse button, hold down tab to toggle a key, hold down shift to select a range of keys.", PROP_TYPE_HEADER) ;
            s32 iHeader = List.PushPath("Path\\SelectedKey(s)\\") ;

            // Commands
            s32 iStartKey = -1;
            s32 iEndKey   = -1;
            if( GetKeysInfo( TRUE, TRUE, iStartKey, iEndKey ) >= 2 )
            {
                // Add commands if valid range is found
                List.PropEnumFloat ( "TotalTime",    "Total time of selected keys.", CommandFlags );
                List.PropEnumButton( "SmoothSpeed",  "Smooths speed with regard to time of selected keys.  This will get rid of fast and slow segments of the keys and make everything smooth", CommandFlags );
                List.PropEnumButton( "EqualSpacing", "Position keys equal distances apart along the segment of selected keys.", CommandFlags );
            }
            
            // Setup selection key
            GetKeysInfo( TRUE, FALSE, iStartKey, iEndKey );
            s32 iKey = -1;              // Multi-select
            if( iStartKey == iEndKey )
                iKey = iStartKey;       // Single
                
            // Enum multi-select key(s)
            key MultiSelectKey ;
            MultiSelectKey.OnEnumProp( List, 
                                    *this,
                                    iKey,
                                    PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT | PROP_TYPE_MUST_ENUM, 
                                    PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT | PROP_TYPE_DONT_SHOW ) ;
            List.PopPath(iHeader) ;
        }
    }
#endif


    // Add the individual key properties
    for(i = 0 ; i < m_Keys.GetCount() ; i++ )
    {
        // Add common properties
        List.PropEnumHeader(xfs("Path\\Key[%d]", i), "Key", 0 ) ;
        s32 iHeader = List.PushPath(xfs("Path\\Key[%d]\\", i)) ;

        // Enum key
        m_Keys[i].OnEnumProp( List, 
                              *this, 
                              i, 
                              PROP_TYPE_EXPOSE | PROP_TYPE_MUST_ENUM, 
                              PROP_TYPE_EXPOSE | PROP_TYPE_MUST_ENUM );

        List.PopPath(iHeader) ;
    }
}

//=============================================================================

xbool path::OnProperty( prop_query& I )
{
#ifdef X_EDITOR
    s32 i ;
#endif

    // Call base class
    if( object::OnProperty( I ) )
    {
        // Reading from UI/File?
        if( I.IsRead() == FALSE )
        {
            // Updating transform?
            if(     ( I.IsVar( "Base\\Position" ) )
                ||  ( I.IsVar( "Base\\Rotation" ) ) )
            {
                // Recompute bounds and rotations
                UpdateBounds();
                AutoGenerateRotation();
            }
        }
        
        return( TRUE );
    }
    
    // If writing to any path property, force update of bounds
    if ( I.IsRead() == FALSE )
        SetFlagBits( GetFlagBits() | object::FLAG_DIRTY_TRANSFORM );

    // Curve type?
    if( I.IsVar("Path\\CurveType" ) )
    {
        if( I.IsRead() )
        {
            switch(m_CurveType)
            {
                case CURVE_TYPE_LINEAR:  I.SetVarEnum("LINEAR") ;    break ;
                case CURVE_TYPE_SPLINE:  I.SetVarEnum("SPLINE") ;    break ;
            }
        }
        else
        {
            if (!x_stricmp(I.GetVarEnum(), "LINEAR"))
                m_CurveType = CURVE_TYPE_LINEAR ;
            else if (!x_stricmp(I.GetVarEnum(), "SPLINE"))
                m_CurveType = CURVE_TYPE_SPLINE ;
        }
        return TRUE ;
    }

    // KeyTime?
    if( I.IsVar("Path\\KeyDeltaTime" ) )
    {
        OnPropertyVarBool(I, m_Flags, path::FLAG_KEY_DELTA_TIME) ;
        return TRUE ;
    }

    // KeyPosition?
    if( I.IsVar("Path\\KeyPosition" ) )
    {
        OnPropertyVarBool(I, m_Flags, path::FLAG_KEY_POSITION) ;
        return TRUE ;
    }

    // KeyRotation?
    if( I.IsVar("Path\\KeyRotation" ) )
    {
        OnPropertyVarBool(I, m_Flags, path::FLAG_KEY_ROTATION) ;
        return TRUE ;
    }

    // Auto Yaw?
    if( I.IsVar("Path\\Auto Yaw" ) )
    {
        OnPropertyVarBool(I, m_Flags, path::FLAG_KEY_AUTO_YAW) ;
        AutoGenerateRotation();
        return TRUE ;
    }

    // Auto Pitch?
    if( I.IsVar("Path\\Auto Pitch" ) )
    {
        OnPropertyVarBool(I, m_Flags, path::FLAG_KEY_AUTO_PITCH) ;
        AutoGenerateRotation();
        return TRUE ;
    }

    // KeyFieldOfView?
    if( I.IsVar("Path\\KeyFieldOfView" ) )
    {
        OnPropertyVarBool(I, m_Flags, path::FLAG_KEY_FIELD_OF_VIEW) ;
        return TRUE ;
    }

    // KeyRoll?
    if( I.IsVar("Path\\KeyRoll" ) )
    {
        OnPropertyVarBool(I, m_Flags, path::FLAG_KEY_ROLL) ;
        return TRUE ;
    }

    // KeyColor?
    if( I.IsVar("Path\\KeyColor" ) )
    {
        OnPropertyVarBool(I, m_Flags, path::FLAG_KEY_COLOR) ;
        return TRUE ;
    }

    // KeyObjectGuid?
    if( I.IsVar("Path\\KeyObjectGuid" ) )
    {
        OnPropertyVarBool(I, m_Flags, path::FLAG_KEY_OBJECT_GUID) ;
        return TRUE ;
    }

#ifdef X_EDITOR

    // Delete keys?
    if( I.IsVar("Path\\DeleteKey(s)"))
    {
        if (I.IsRead())
            I.SetVarButton( "Delete keys(s)" );
        else
        {
            // Loop through all keys
            for (s32 i = m_Keys.GetCount()-1 ; i >= 0 ; i--)
            {
                // If key is selected, delete it!
                if (m_Keys[i].m_Flags & key::FLAG_SELECTED)
                    DeleteKey(i) ;
            }
        }

        return TRUE ;
    }
    
#endif  //#ifdef X_EDITOR


    // Key count?
    if( I.IsVar("Path\\KeyCount"))
    {
        if( I.IsRead() )
            I.SetVarInt( m_Keys.GetCount() ) ;
        else
        {
            // Get new count
            s32 Count = I.GetVarInt() ;
            
            // Add new keys on the end
            while(Count > m_Keys.GetCount())
                CreateKey(m_Keys.GetCount()-1, TRUE) ;

            // Set count incase of deleting keys
            m_Keys.SetCount(Count) ;
        }

        return TRUE ;
    }

#ifdef X_EDITOR

    // Total time?
    if( I.IsVar("Path\\TotalTime"))
    {
        if( I.IsRead() )
            I.SetVarFloat( GetTotalTime( FALSE ) );
        else
            ScaleTime( FALSE, I.GetVarFloat( 0.0f, F32_MAX ) );

        return TRUE ;
    }

    // Make path keys constant speed
    if( I.IsVar( "Path\\SmoothSpeed" ) )
    {
        if (I.IsRead())
            I.SetVarButton( "SmoothSpeed" );
        else
            SmoothSpeed( FALSE );

        return TRUE ;
    }

    // Make path keys equal spacing
    if( I.IsVar( "Path\\EqualSpacing" ) )
    {
        if (I.IsRead())
            I.SetVarButton( "EqualSpacing" );
        else
            EqualSpacing( FALSE );

        return TRUE ;
    }

    // Select all keys
    if( I.IsVar("Path\\SelectAllKeys"))
    {
        // Set button text
        if (I.IsRead())
            I.SetVarButton( "SelectAllKeys" );
        else
        {
            // Select all the keys
            for( s32 i = 0; i < m_Keys.GetCount(); i++ )
                m_Keys[i].m_Flags |= key::FLAG_SELECTED;
        }

        return TRUE ;
    }

    // Selected keys string?
    if( I.IsVar("Path\\SelectedKey(s)"))
    {
        ASSERT(I.IsRead()) ;
        xstring List = "Keys[" ;
        xbool bFirst=TRUE ;
        for (i = 0 ; i < m_Keys.GetCount() ; i++)
        {
            // Is this key selected?
            if (m_Keys[i].m_Flags & key::FLAG_SELECTED)
            {
                // Get first
                if (bFirst)
                {
                    List += xfs("%d", i) ;
                    bFirst = FALSE ;
                }
                else
                    List += xfs(",%d",i) ;
            }
        }
        List += "]" ;
        
        I.SetVarString(List, List.GetLength()+1) ;
        return TRUE ;
    }

    // Selected keys?
    if (I.IsSimilarPath("Path\\SelectedKey(s)"))
    {
        s32   iHeader = I.PushPath("Path\\SelectedKey(s)\\") ; 
        xbool bFound = FALSE ;

        // Total time?
        if( I.IsVar("TotalTime"))
        {
            if( I.IsRead() )
                I.SetVarFloat( GetTotalTime( TRUE ) );
            else
                ScaleTime( TRUE, I.GetVarFloat( 0.0f, F32_MAX ) );
                
            return TRUE ;
        }

        // Make path keys constant speed
        if( I.IsVar( "SmoothSpeed" ) )
        {
            if (I.IsRead())
                I.SetVarButton( "SmoothSpeed" );
            else
                SmoothSpeed( TRUE );

            return TRUE ;
        }

        // Make path keys equal spacing
        if( I.IsVar( "EqualSpacing" ) )
        {
            if (I.IsRead())
                I.SetVarButton( "EqualSpacing" );
            else
                EqualSpacing( TRUE );

            return TRUE ;
        }

        // Setting up UI?
        if (I.IsRead())
        {
            // Setup default key values
            key   MultiSelectKey ;
            s32   iKey   = -1;
            xbool bFirst = TRUE ;

            // Loop over all keys
            for (i = 0 ; i < m_Keys.GetCount() ; i++)
            {
                // Selected?
                if (m_Keys[i].m_Flags & key::FLAG_SELECTED)
                {
                    // Grab from first selected key or multi-select?
                    if (bFirst)
                    {
                        MultiSelectKey = m_Keys[i] ;
                        iKey           = i;
                        bFirst         = FALSE ;
                    }
                    else
                    {
                        // Accumualte values
                        MultiSelectKey.MultiSelect(m_Keys[i]) ;
                        iKey = -1;
                    }
                }
            }

            // Setup UI?
            bFound |= MultiSelectKey.OnProperty( I, *this, iKey ) ;
        }
        else
        {
            // Write to all the selected keys (loop backwards incase of deletion)
            for (i = m_Keys.GetCount()-1 ; i >= 0 ; i--)
            {
                // Write to this key if it's selected
                if (m_Keys[i].m_Flags & key::FLAG_SELECTED)
                    bFound |= m_Keys[i].OnProperty(I, *this, i) ;
            }
        }

        I.PopPath(iHeader) ;

        if (bFound)
            return TRUE ;
    }
#endif  //#ifdef X_EDITOR

    // Key[] ?
    if (I.IsSimilarPath("Path\\Key["))
    {
        // Lookup key
        s32  iKey = I.GetIndex(0) ;
        key& Key  = m_Keys[iKey] ;

        s32 iHeader = I.PushPath("Path\\Key[]\\") ; 

        // Read/Write key?
        xbool bFound = Key.OnProperty(I, *this, iKey) ;
        
        I.PopPath(iHeader) ;

        if (bFound)
            return TRUE ;
    }

    return FALSE ;
}

//=========================================================================

#if !defined( CONFIG_RETAIL )

void path::RenderSegments( f32    StartTime, 
                           f32    EndTime, 
                           s32    nSegments, 
                           xbool  bLines,
                           xcolor Color )
{
    // Must have at least one segments!
    if (nSegments < 1)
        nSegments = 1 ;

    // Init time and compute delta time
    f32 Time      = StartTime ;
    f32 DeltaTime = (EndTime - StartTime) / nSegments ;
    
    // Init first key
    key KeyA, KeyB ;
    GetInterpKey(Time, Time, KeyA) ;

    // Init second key
    Time += DeltaTime ;
    GetInterpKey(Time, Time, KeyB) ;
    
    // Draw all segments
    for (s32 i = 0 ; i < nSegments ; i++)
    {
        // Render line between segments?
        if (bLines)
            draw_Line(KeyA.m_Position, KeyB.m_Position, Color) ;
        else
            draw_Point(KeyA.m_Position, Color, 1) ;

        // Next segment
        KeyA = KeyB ;
        Time += DeltaTime ;
        GetInterpKey(Time, Time, KeyB) ;
    }
}

//=========================================================================

void path::OnRender  ( void )
{
    s32 i;
    i=1;
}

//=========================================================================

void path::RenderPath( void )
{
#ifdef X_EDITOR
    CONTEXT("path::OnRender" );

    s32     i ;
    f32     TimeA, TimeB ;
    xcolor  Color ;
    f32     TotalTime = GetTotalTime( FALSE ) ;
    f32     Spacing ;

    // Draw lines and dots between keys?
    if ((m_Keys.GetCount() >= 2) && (TotalTime > 0.0f))
    {
        // Compute time spacing
        Spacing = (f32)s_path_Desc.GetSegmentDensityMultiplier();

        // Render lines between keys
        TimeA = 0 ;
        TimeB = 0 ;
        for (i = 1 ; i < m_Keys.GetCount() ; i++)
        {
            // Compute key times
            TimeA = TimeB ;
            TimeB += m_Keys[i].m_DeltaTime ;

            // Setup color
            if (GetAttrBits() & ATTR_EDITOR_SELECTED)
                Color = (m_Keys[i].m_Flags & key::FLAG_SELECTED) ? xcolor( 200, 200,   0 ) : XCOLOR_RED ;
            else
                Color = XCOLOR_GREEN ;
            
            // Render segments
            RenderSegments(TimeA, TimeB, (s32)((TimeB - TimeA) * Spacing), TRUE, Color) ;
        }

        // Render dots between keys
        if (GetAttrBits() & ATTR_EDITOR_SELECTED)
        {
            TimeA = 0 ;
            TimeB = 0 ;
            for (i = 1 ; i < m_Keys.GetCount() ; i++)
            {
                // Compute key times
                TimeA = TimeB ;
                TimeB += m_Keys[i].m_DeltaTime ;

                // Setup color
                Color = (m_Keys[i].m_Flags & key::FLAG_SELECTED) ? XCOLOR_WHITE : XCOLOR_PURPLE ;
                
                // Render segments
                RenderSegments(TimeA, TimeB, (s32)((TimeB - TimeA) * Spacing), FALSE, Color) ;
            }
        }
    }

    // Draw keys
    for (i = 0 ; i < m_Keys.GetCount() ; i++)
        m_Keys[i].OnRender(*this, i) ;
#endif // X_EDITOR
}

#endif // !defined( CONFIG_RETAIL )

//=========================================================================

void path::CreateKey( s32 iRefKey /*= -1*/, xbool bAfterRefKey /*= TRUE*/ )
{
    // Default new key index and position
    s32     iNewKey = 0 ;
    key     MidKey ;

    // Is this the first key ever?
    if ((iRefKey == -1) || (m_Keys.GetCount() == 0))
    {
        iNewKey = 0 ;
        iRefKey = -1 ;
    }
    else
    {
        // Compute position inbetween keys
        s32 iPrevKey = iRefKey ;
        s32 iNextKey = iRefKey ;
        if (bAfterRefKey)
            iNextKey = x_min(iRefKey+1, m_Keys.GetCount()-1) ;
        else
            iPrevKey = x_max(iRefKey-1, 0) ;
        
        // Compute mid key
        f32 MidTime = (GetKeyTime(iPrevKey) + GetKeyTime(iNextKey)) * 0.5f ;
        GetInterpKey(MidTime, MidTime, MidKey) ;
        MidKey.m_DeltaTime = m_Keys[iNextKey].m_DeltaTime * 0.5f ;

        // Insert after or before reference key?
        if (bAfterRefKey)
            iNewKey = iRefKey+1 ;
        else
        {
            iNewKey = iRefKey ;
            iRefKey += 1 ;  // Don't forget to update reference key!
        }
    }

    // Create new key
    key& Key = m_Keys.Insert(iNewKey) ;
    Key.SetDefaults() ;

    // Is this the first key added?
    if (m_Keys.GetCount() == 1)
    {
        Key.m_Position  = GetPosition() ;
    }
    // Is this the second key added?
    else if (m_Keys.GetCount() == 2)
    {
        Key = m_Keys[iRefKey] ;
        if (bAfterRefKey)
            Key.m_Position += vector3(0,0,100*1) ;
        else
            Key.m_Position -= vector3(0,0,100*1) ;
    }
    // Adding to front of path?
    else if (iNewKey == 0)
    {
        Key = m_Keys[iRefKey] ;
        Key.m_Position += m_Keys[iRefKey].m_Position - m_Keys[iRefKey+1].m_Position ;
        m_Keys[iRefKey].m_DeltaTime = m_Keys[iRefKey+1].m_DeltaTime ;
    }
    // Adding to end of path?
    else if (iNewKey == (m_Keys.GetCount()-1))
    {
        Key = m_Keys[iRefKey] ;
        Key.m_Position += m_Keys[iRefKey].m_Position - m_Keys[iRefKey-1].m_Position ;
        Key.m_DeltaTime = m_Keys[iRefKey].m_DeltaTime ;
    }
    else
    {
        // Adding in middle of path?
        if (bAfterRefKey)
        {
            Key = MidKey ;
            m_Keys[iRefKey+2].m_DeltaTime *= 0.5f ;
        }
        else
        {
            Key = MidKey ;
            m_Keys[iRefKey].m_DeltaTime *= 0.5f ;
        }
    }

    // Update object and select new key
    SetFlagBits( GetFlagBits() | object::FLAG_DIRTY_TRANSFORM );
    SelectKey(iNewKey) ;
}

//=========================================================================

void path::DeleteKey( s32 Index )
{
    // Is the path object at this key?
    if ( (GetPosition() == m_Keys[Index].m_Position) && (m_Keys.GetCount() > 1) )
    {
        // Get next valid key to place path anchor at
        s32 iPathKey = Index+1 ;
        if (iPathKey >= m_Keys.GetCount())
            iPathKey = Index-1 ;
        ASSERT(iPathKey >= 0) ;
        ASSERT(iPathKey < m_Keys.GetCount()) ;
        ASSERT(iPathKey != Index) ;

        // Move path icon
        SetPathIconPosition( iPathKey );
    }

    // Delete the key
    m_Keys.Delete(Index) ;

    // Update transform
    SetFlagBits( GetFlagBits() | object::FLAG_DIRTY_TRANSFORM );
}

//=========================================================================

void path::SelectKey( s32 Index )
{
    // De-select all keys
    for (s32 i = 0 ; i < m_Keys.GetCount() ; i++)
        m_Keys[i].m_Flags &= ~key::FLAG_SELECTED ;

    // Select the new key
    m_Keys[Index].m_Flags |= key::FLAG_SELECTED ;

    // Put path object at the new key
    SetPathIconPosition( Index );    
}

//=========================================================================

void path::ValidatePosition( void )
{
    s32 i ;

    // Check position against all keys
    for ( i = 0 ; i < m_Keys.GetCount() ; i++ )
    {
        // Path position is at a key - all is good!
        if (GetPosition() = m_Keys[i].m_Position)
            return ;
    }

    // Put path object at 1st key
    if (m_Keys.GetCount())
        SetPathIconPosition( 0 );

    AutoGenerateRotation();
}

//=========================================================================

#if defined( X_EDITOR )

void path::OnSave( text_out& TextOut )
{
    // Call base class
    object::OnSave(TextOut) ;

    // Make sure path position is valid
    ValidatePosition() ;
}

#endif // defined( X_EDITOR )

//=========================================================================

void path::OnLoad( text_in&  TextIn )
{
    // Call base class
    object::OnLoad(TextIn) ;

    // Make sure path position is valid
    ValidatePosition() ;
}

//=========================================================================

void path::OnMove( const vector3& NewPos )
{
    // Force update of bounds
    SetFlagBits( GetFlagBits() | object::FLAG_DIRTY_TRANSFORM );

    // Compute delta
    vector3 DeltaPos = NewPos - GetPosition() ;

    // Call base class
    object::OnMove( NewPos );

    // Leave keys?
    if (m_Flags & path::FLAG_KEYS_LOCKED)
        return ;

    // Move all selected keys
    for (s32 i = 0 ; i < m_Keys.GetCount() ; i++)
    {
        if (m_Keys[i].m_Flags & key::FLAG_SELECTED)
            m_Keys[i].m_Position += DeltaPos ;
    }

    AutoGenerateRotation();
}

//=========================================================================

void path::OnTransform( const matrix4& L2W )
{
    s32 i ;

    // Compute deltas
    vector3 Origin   = L2W.GetTranslation() ;
    vector3 DeltaPos = Origin - GetPosition() ;
    matrix4 PrevL2W  = GetL2W() ;
    PrevL2W.InvertRT() ;
    matrix4 DeltaRot   = PrevL2W * L2W ;
    
    // Call base class
    object::OnTransform( L2W );

    // Leave keys?
    if (m_Flags & path::FLAG_KEYS_LOCKED)
        return ;

    // Translate all selected keys
    for (i = 0 ; i < m_Keys.GetCount() ; i++)
    {
        // Lookup key
        path::key& Key = m_Keys[i] ;

        // Only move selected keys
        if (Key.m_Flags & key::FLAG_SELECTED)
            Key.m_Position += DeltaPos ;
    }

    // Compute delta rotation
    quaternion  QDeltaRot(DeltaRot.GetRotation()) ;

    // Compute delta rotation matrix to rotate about the origin
    matrix4 OriginDeltaRot ;
    OriginDeltaRot.Identity() ;
    OriginDeltaRot.Translate(-Origin) ;
    OriginDeltaRot = DeltaRot * OriginDeltaRot ;
    OriginDeltaRot.Translate(Origin) ;

    // Rotate all keys
    for (i = 0 ; i < m_Keys.GetCount() ; i++)
    {
        // Lookup key
        path::key& Key = m_Keys[i] ;

        // Update position
        Key.m_Position = OriginDeltaRot * Key.m_Position ;

        // Update rotation
        Key.m_Rotation = QDeltaRot * Key.m_Rotation ;
        Key.m_Rotation.Normalize() ;
    }

    AutoGenerateRotation();
}
    
//=========================================================================

// Returns key time
f32 path::GetKeyTime( s32 iKey ) const
{
    f32 Time = 0 ;
    
    // Always skip first key (for key zero, time = 0)
    for (s32 i = 1 ; i <= iKey ; i++)
        Time += m_Keys[i].m_DeltaTime ;

    return Time ;
}

//=========================================================================

// Returns path icon selected key
s32 path::GetPathIconSelectedKey( void ) const
{
    // No keys?
    if( m_Keys.GetCount() == 0 )
        return -1;
        
    // Loop through all keys
    vector3 Pos = GetPosition();
    for( s32 i = 0; i < m_Keys.GetCount() ; i++ )
    {
        // Is icon at key?
        if( m_Keys[i].m_Position == Pos )
            return i;
    }
    
    // Snap to first key
    return 0;
}

//=========================================================================

// Sets icon position to key
void path::SetPathIconPosition( s32 iKey )
{
    // Nothing to do?
    if( (iKey == -1) || (iKey >= m_Keys.GetCount()) )
        return;
    
    // Stop keys from being moved and set path position
    m_Flags |= path::FLAG_KEYS_LOCKED ;
    OnMove( m_Keys[ iKey ].m_Position ) ;
    m_Flags &= ~path::FLAG_KEYS_LOCKED ;
}

//=========================================================================

// Returns selected key count
s32 path::GetSelectedKeyCount( void ) const
{
    // Reset count
    s32 Count = 0 ;

    // Check all keys
    for (s32 i = 0 ; i < m_Keys.GetCount() ; i++)
    {
        // Selected?
        if (m_Keys[i].m_Flags & key::FLAG_SELECTED)
            Count++ ;
    }
    
    return Count ;
}

//=========================================================================

// Returns key tangent for spline
vector3 path::GetKeyTangent( s32 iKey ) const
{
    // Is this key a corner?
    if (m_Keys[iKey].m_Flags & key::FLAG_CORNER)
        return vector3(0,0,0) ;

    // Compute clamped indices of surrounding keys
    s32 iPrevKey = x_max(iKey-1,0);
    s32 iNextKey = x_min(iKey+1, m_Keys.GetCount()-1);

    // Check for a looping spline (need at least two keys)
    s32 iLastKey = m_Keys.GetCount()-1 ;
    if (iLastKey >= 0)
    {
        // Does this path loop?
        if (m_Keys[0].m_Position == m_Keys[iLastKey].m_Position)
        {
            // Fix up the indices for correct spline looping
            // NOTE: The -1 and +1 are so the identical position is skipped
            //       and the tangents match up!
            if (iKey == 0)
                iPrevKey = iLastKey-1 ;
            else
            if (iKey == iLastKey)
                iNextKey = +1 ;
        }
    }
   
    // Compute direction between surrounding keys
    vector3 Delta = (m_Keys[iNextKey].m_Position - m_Keys[iPrevKey].m_Position) ;

    // Scale by 0.5f so that if keys are equally spaced, the internal 
    // points will also be equally spaced
    // ie. the tracker will move at a constant speed.
    Delta *= 0.5f ;

    return Delta ;
}

//=========================================================================

// Returns key, given previous and current time
void path::GetInterpKey( f32 PrevTime, f32 CurrTime, key& Key ) const
{
    // Make time valid
    f32 TotalTime = GetTotalTime( FALSE ) ;
    
    // Make sure times are valid
    if (PrevTime < 0)
        PrevTime = 0 ;
    else
    if (PrevTime > TotalTime)
        PrevTime = TotalTime ;

    if (CurrTime < 0)
        CurrTime = 0 ;
    else
    if (CurrTime > TotalTime)
        CurrTime = TotalTime ;

    // If no keys, return default
    if (m_Keys.GetCount() == 0)
    {
        Key.SetDefaults() ;
        return ;
    }

    // Just the one key?
    if (m_Keys.GetCount() == 1)
    {
        Key = m_Keys[0] ;
        return ;
    }

    // Search for keys that surround the requested time
    s32 iKeyA = 0 ;
    s32 iKeyB = 1 ;
    f32 KeyATime = 0 ;
    f32 KeyBTime = m_Keys[1].m_DeltaTime ;
    while( (iKeyB < (m_Keys.GetCount()-1)) && (KeyBTime < CurrTime) )
    {
        // Next keys
        iKeyA++ ;
        iKeyB++ ;

        // Next times
        KeyATime = KeyBTime ;
        KeyBTime += m_Keys[iKeyB].m_DeltaTime ;
    }

    // Lookup keys
    const key& KeyA = m_Keys[iKeyA] ;
    const key& KeyB = m_Keys[iKeyB] ;

    // Past front?
    if (CurrTime < KeyATime)
    {
        Key = KeyA ;
        return ;
    }

    // Past end?
    if (CurrTime > KeyBTime)
    {
        Key = KeyB ;
        return ;
    }

    // Keys have same time?
    if (KeyATime == KeyBTime)
    {
        Key = KeyB ;
        return ;
    }

    // Compute ratio between keys
    ASSERT(CurrTime >= KeyATime) ;
    ASSERT(CurrTime <= KeyBTime) ;
    f32 T = (CurrTime - KeyATime) / (KeyBTime - KeyATime) ;

    // Interpolate keys
    Key.Interpolate(*this, 
                    iKeyA, KeyATime, 
                    iKeyB, KeyBTime, 
                    T, 
                    PrevTime, CurrTime) ;
}

//=========================================================================

// Returns total time of all keys
f32 path::GetTotalTime( xbool bSelectedOnly ) const
{
    f32 TotalTime = 0.0f;
    
    // Total just selected?
    if( bSelectedOnly )
    {
        // Loop over all keys
        for (s32 i = 1; i < m_Keys.GetCount(); i++ )
        {
            // Lookup key
            key& Key = m_Keys[i];

            // Add to total if selected
            if( Key.m_Flags & key::FLAG_SELECTED )
                TotalTime += m_Keys[i].m_DeltaTime ;
        }
    }
    else
    {
        // Loop over all keys
        for ( s32 i = 1; i < m_Keys.GetCount(); i++ )
            TotalTime += m_Keys[i].m_DeltaTime ;
    }
                    
    return TotalTime ;
}

//=========================================================================

// Forces update of bounds
void path::UpdateBounds( void )
{
    // Just flag transform as dirty and bounds will get re-computed
    SetFlagBits(GetFlagBits() | object::FLAG_DIRTY_TRANSFORM);

    AutoGenerateRotation();
}

//=========================================================================

void path::AutoGenerateRotation( void )
{
    s32     nKeys     = GetKeyCount();
    u32     PathFlags = GetFlags();
    s32     i;

    for (i=0;i<nKeys;i++)
    {
        vector3 Tangent = GetKeyTangent( i );
        radian  Pitch,Yaw;
        radian3 Rot = m_Keys[i].m_Rotation.GetRotation();

        Tangent.GetPitchYaw( Pitch, Yaw );

        // Yaw seems to be off by 90 degrees.  There is probably something
        // that is grabbing the wrong coordinate axis somewhere.
        // I don't have time to track it down, and this makes everything
        // "better" in the interim. - SH
        Yaw -= R_90;

        if ( PathFlags & FLAG_KEY_AUTO_YAW )
        {
            Rot.Yaw = Yaw;
        }
        if ( PathFlags & FLAG_KEY_AUTO_PITCH )
        {
            Rot.Pitch = Pitch;
        }

        m_Keys[i].m_Rotation.Setup( Rot );
    }
}

//=========================================================================

void path::OnActivate( xbool Flag )
{
    // We don't need to call back to the base object OnActivate because
    // we don't actually use logic time.

    m_bPathOn = Flag;
}

//=========================================================================

xbool path::IsPathOn( void )
{
    return m_bPathOn;
}

//=========================================================================

#ifdef X_EDITOR

// Returns length (in cm) of key segment
f32 path::GetKeyLength( s32 iKey ) const
{
    // Key zero is always time=0, length=0
    if( iKey == 0 )
        return 0.0f;
    
    // Lookup key times
    f32 CurrKeyTime = GetKeyTime( iKey );
    f32 PrevKeyTime = GetKeyTime( iKey - 1 );
    if( CurrKeyTime == PrevKeyTime )
        return 0.0f;
    
    // Compute delta time based small time increments
    s32 Steps     = x_max( 1, (s32)( ( CurrKeyTime - PrevKeyTime ) / 0.001f ) );
    f32 DeltaTime = ( CurrKeyTime - PrevKeyTime ) / (f32)Steps;
    
    // Get start key
    key PrevKey, CurrKey;
    GetInterpKey( PrevKeyTime, PrevKeyTime, PrevKey );
    
    // Loop over inbetween keys
    f32 TotalLength = 0.0f;
    for( f32 Time = PrevKeyTime ; Time < CurrKeyTime ; Time += DeltaTime )
    {
        // Lookup current key
        GetInterpKey( Time, Time + DeltaTime, CurrKey );
        
        // Accumulate length
        vector3 Delta     = CurrKey.m_Position - PrevKey.m_Position;
        f32     LengthSqr = Delta.LengthSquared();
        if( LengthSqr > 0.00001f )
            TotalLength += x_sqrt( LengthSqr );
        
        // Setup previous key ready for next delta
        PrevKey = CurrKey;
    }
    
    return TotalLength;        
}

//=============================================================================

// Returns selected key range
xbool path::GetKeysInfo( xbool bSelectedOnly, xbool bSkipKey0, s32& iStartKey, s32& iEndKey ) const
{
    s32 i;

    // Clear indices
    iStartKey = -1;
    iEndKey   = -1;

    // Setup key0
    s32 Key0 = 0;
    if( bSkipKey0 )
        Key0 = 1;
    
    // Just selected keys?
    if( bSelectedOnly )
    {    
        // Find start key
        for( i = Key0; i < m_Keys.GetCount(); i++ )
        {
            if( m_Keys[i].m_Flags & key::FLAG_SELECTED )
            {
                iStartKey = i;
                break;
            }
        }

        // Find end key
        for( i = m_Keys.GetCount()-1; i >= Key0 ; i-- )
        {
            if( m_Keys[i].m_Flags & key::FLAG_SELECTED )
            {
                iEndKey = i;
                break;
            }
        }

        // If any inbetween keys are not selected, then range is invalid
        for( i = iStartKey+1; i < iEndKey; i++ )
        {
            if( ( m_Keys[i].m_Flags & key::FLAG_SELECTED ) == 0 )
                return 0;
        }
    }
    else
    {
        // Use all keys
        if( m_Keys.GetCount() == 0 )
            return 0;
            
        iStartKey = Key0;
        iEndKey   = m_Keys.GetCount()-1;
    }
    
    // Return count
    return iEndKey - iStartKey + 1;
}

//=========================================================================

// Returns selected key range and total time + length of segment (skips key0)
s32 path::GetKeysInfo( xbool bSelectedOnly, 
                       s32&  iStartKey, 
                       s32&  iEndKey,
                       f32&  TotalTime,
                       f32&  TotalLength )
{
    s32 i;

    // Clear info
    iStartKey   = -1;
    iEndKey     = -1;
    TotalTime   = 0.0f;
    TotalLength = 0.0f;

    // Lookup valid range
    s32 nKeys = GetKeysInfo( bSelectedOnly, TRUE, iStartKey, iEndKey );

    // No keys or just one key?
    if( ( iStartKey == -1 ) || ( iEndKey == -1 ) || ( iStartKey == iEndKey ) )
        return 0;
       
    // Above logic should always skip key 0       
    ASSERT( iStartKey > 0 );
    ASSERT( iEndKey   > 0 );
           
    // Compute the total time and length of the selected keys
    // (skip the first since it has no previous key)
    for( i = iStartKey ; i <= iEndKey; i++ )
    {
        // Lookup key
        key& Key = m_Keys[i];

        // Using all keys or selected?
        if( ( bSelectedOnly == FALSE ) || ( Key.m_Flags & key::FLAG_SELECTED ) )
        {
            // Accumulate info
            TotalTime   += Key.m_DeltaTime;
            TotalLength += GetKeyLength( i );
        }
    }
    
    // Return key count
    return nKeys;
}

//=========================================================================

void path::ScaleTime( xbool bSelectedOnly, f32 TotalTime )
{
    // Get current total time
    f32 CurrentTotalTime = GetTotalTime( bSelectedOnly );
    if( CurrentTotalTime < 0.0f )
        return;

    // Compute time scale to apply to selected keys
    f32 Scale = TotalTime / CurrentTotalTime;

    // Scale time of selected keys
    for( s32 i = 1; i < m_Keys.GetCount(); i++ )
    {
        // Lookup key
        key& Key = m_Keys[i];

        // Scale time if selected
        if( ( bSelectedOnly == FALSE ) || ( Key.m_Flags & key::FLAG_SELECTED ) )
            Key.m_DeltaTime *= Scale ;
    }

    // Update rotation    
    AutoGenerateRotation();
}

//=========================================================================

void path::SmoothSpeed( xbool bSelectedOnly )
{
    s32 i;
    s32 nKeys;
    s32 iStartKey;
    s32 iEndKey;
    f32 TotalTime;
    f32 TotalLength;
    
    // Lookup info
    nKeys = GetKeysInfo( bSelectedOnly, iStartKey, iEndKey, TotalTime, TotalLength );
    if( nKeys < 2 )
        return;
        
    // Key0 should never be selected
    ASSERT( iStartKey > 0 );
    ASSERT( iEndKey   > 0 );
        
    // Now scale the time base on the distances
    for( i = iStartKey; i <= iEndKey; i++ )
    {
        // Lookup key
        key& Key = m_Keys[i];

        // If selected, scale the time based on the distance ratio
        if( ( bSelectedOnly == FALSE ) || ( Key.m_Flags & key::FLAG_SELECTED ) )
            Key.m_DeltaTime = TotalTime * GetKeyLength( i ) / TotalLength;
    }
    
    // Counter act length floating errors
    ScaleTime( bSelectedOnly, TotalTime );

    // Update rotation
    AutoGenerateRotation();
}

//=========================================================================

void path::EqualSpacing( xbool bSelectedOnly )
{
    s32 i;
    s32 nKeys;
    s32 iStartKey;
    s32 iEndKey;
    f32 TotalTime;
    f32 TotalLength;

    // Lookup info
    nKeys = GetKeysInfo( bSelectedOnly, iStartKey, iEndKey, TotalTime, TotalLength );
    if( nKeys < 2 )
        return;

    // Key0 should never be selected
    ASSERT( iStartKey > 0 );
    ASSERT( iEndKey   > 0 );

    // Copy keys so we can read/write at the same time
    xarray<key> DstKeys = m_Keys;
    
    // Compute spacing distance
    f32 Spacing = TotalLength / (f32)nKeys;        
    
    // Lookup key times
    f32 StartTime = GetKeyTime( iStartKey -1 );
    f32 EndTime   = GetKeyTime( iEndKey );
    f32 Length    = 0.0f;
    
    // Lookup previous key    
    key PrevKey, CurrKey;
    GetInterpKey( StartTime, StartTime, PrevKey );
    
    // Loop over all of path segment in small time increments
    s32 iKey = iStartKey;
    for( f32 Time = StartTime; ( Time < EndTime ) && ( iKey < iEndKey ) ; Time += 0.001f )
    {
        // Lookup current position
        GetInterpKey( Time, Time + 0.001f, CurrKey );

        // Accumulate distance
        vector3 Delta     = CurrKey.m_Position - PrevKey.m_Position;
        f32     LengthSqr = Delta.LengthSquared();
        if( LengthSqr > 0.00001f )
        {
            Length += x_sqrt( LengthSqr );
            
            // Crossed spacing boundary?
            if( Length >= Spacing )
            {
                // Update key position
                Length -= Spacing;
                DstKeys[ iKey++ ].m_Position = CurrKey.m_Position;
            }
        }
        
        // Get ready for next delta
        PrevKey = CurrKey;
    }

    // Lookup icon key
    s32 iIconKey = GetPathIconSelectedKey();

    // Keep new key positions (leave ends alone)
    for( i = iStartKey; i < iEndKey ; i++ )
        m_Keys[i].m_Position = DstKeys[i].m_Position;
        
    // Update icon
    SetPathIconPosition( iIconKey );

    // Update rotation
    AutoGenerateRotation();
}

#endif  //#ifdef X_EDITOR

//=========================================================================
