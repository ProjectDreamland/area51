
#include "animation_obj.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "EventMgr\EventMgr.hpp"

//=========================================================================
// VARIABLES
//=========================================================================

static const char* s_pActions  = "0.NoAction\0""1.PlayFoward\0""2.PlayBackwords\0""3.Pause\0""4.Continue\0""5.Restart\0";
static const char* s_pPlayType = "0.OnesThenStop\0""1.OnesThenDie\0""2.Loop\0""3.PinPong\0";

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

//=========================================================================

static struct animation_obj_desc : public object_desc
{
    animation_obj_desc( void ) : object_desc( 
            object::TYPE_ANIMATION_OBJECT, 
            "Animation Object", 
            "SCRIPT",

            object::ATTR_NEEDS_LOGIC_TIME,

            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_TARGETS_OBJS          |
            FLAGS_IS_DYNAMIC ) {}

    //---------------------------------------------------------------------
    virtual object* Create          ( void ) { return new animation_obj; }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32     OnEditorRender  ( object& Object ) const 
    { 
        object_desc::OnEditorRender( Object );
        return EDITOR_ICON_GEAR; 
    }

#endif // X_EDITOR

} s_animation_obj_Desc;

//=========================================================================

const object_desc& animation_obj::GetTypeDesc( void ) const
{
    return s_animation_obj_Desc;
}

//=========================================================================

const object_desc& animation_obj::GetObjectType( void )
{
    return s_animation_obj_Desc;
}

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

animation_obj::animation_obj( void )
{
    m_Flags  = FLAGS_DESTROY_AFTER_PLAYING;
    m_nBones = 0;
    m_iCurrentKeyFrame = 0;
    m_Time=0;
    m_iFrame=0;
    m_Cycle = 0;
    m_Flags |= FLAGS_LOOP;
}

//=============================================================================

void animation_obj::OnInit( void )
{
    // No logic from the start
    //SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );
}

//=============================================================================

void animation_obj::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader( "animation_obj", "This are the properties for the animation_obj", 0 );

    List.PropEnumInt( "animation_obj\\KeyCount", 
        "Tells the user how may keyframes there are in the system.", PROP_TYPE_READ_ONLY );

    List.PropEnumEnum( "animation_obj\\Action", s_pActions, 
        "This are the different actions triggers have access to."
        "As a user you can test the object at run time as well.", 
        PROP_TYPE_MUST_ENUM | PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SAVE );

    List.PropEnumEnum( "animation_obj\\PlayType", s_pPlayType, 
        "This tells how you want the animation to do when it reaches the end."
        "You can make it repeat, stop, or die.", 
        PROP_TYPE_MUST_ENUM | PROP_TYPE_EXPOSE );

    List.PropEnumInt( "animation_obj\\SaveKeyCount", "no comment", PROP_TYPE_DONT_SHOW );

    List.PropEnumInt( "animation_obj\\CurrentKey", 
        "This is the keyframe which you are tying to add or insert."
        "To change the mode from add to insert force this key to be the last one", 
        PROP_TYPE_MUST_ENUM );

    List.PropEnumButton( "animation_obj\\KeyAction", 
        "This button is use to create keyframes for the animated object ", 0 );

    for( s32 i=0; i<m_KeyFrame.GetCount(); i++ )
    {
        List.PropEnumHeader   ( xfs("animation_obj\\Key[%d]",i), "Key frame use to animated the object.", 0 );
        List.PropEnumButton   ( xfs("animation_obj\\Key[%d]\\SetKey",i),    "Sets the key values base from the object position.", PROP_TYPE_MUST_ENUM );
        List.PropEnumButton   ( xfs("animation_obj\\Key[%d]\\JumpToKey",i),  "Make the animated object take this keys values.", PROP_TYPE_MUST_ENUM );
        List.PropEnumFloat    ( xfs("animation_obj\\Key[%d]\\Time",i),      "What is the delta time from the previous keyframe before getting to this Keyframe.", 0 );
        List.PropEnumRotation ( xfs("animation_obj\\Key[%d]\\Rotate",i),    "Rotation of the object.", 0 );
        List.PropEnumVector3  ( xfs("animation_obj\\Key[%d]\\Translate",i), "Translation of the object.", 0 );
        List.PropEnumButton   ( xfs("animation_obj\\Key[%d]\\DeleteKey",i), "Deletes the key and from the set", PROP_TYPE_MUST_ENUM );
    }


/*    
    List.AddButton  ( "animation_obj\\UpdateObjects", "This button is use to update the transformation of the object that the objects dependent.", PROP_TYPE_DONT_SAVE );

    List.AddInt     ( "animation_obj\\BoneNumberSave", "This indicates how many bones does the animation has there for how many guids must be set", PROP_TYPE_DONT_SHOW );

    List.AddExternal( "animation_obj\\Anim",           "Resource\0anim\0",       "Resource File", PROP_TYPE_MUST_ENUM );
    
    List.AddHeader  ( "animation_obj", "This is the properties that are unique for the anim surface" );

    List.AddExternal( "animation_obj\\Audio Package",  "Resource\0audiopkg\0",   "The audio package associated with this anim surface." );

    List.AddBool    ( "animation_obj\\Active",       "This will indicate whether the object is active from the begining or not." );
    List.AddBool    ( "animation_obj\\Loop",         "Tells that the animation is going to loop for ever." );

    List.AddInt     ( "animation_obj\\BoneCount",    "This indicates how many bones does the animation has there for how many guids must be set", PROP_TYPE_READ_ONLY );

    for( s32 i=0; i<m_nGuids; i++ )
    {
        List.AddGuid( xfs("animation_obj\\Guid[%d]",i), "Object which bone x is asociated with." );
    }
    */
}

//=============================================================================

void animation_obj::SetEditKeyFrame( s32 Index )
{
    // The index must be in range
    ASSERT( Index >= 0 );
    ASSERT( Index < m_KeyFrame.GetCount() );

    keyframe& Key = m_KeyFrame[Index];

    // Set the key zero data
    matrix4 L2W = GetL2W();
    Key.Translate = L2W.GetTranslation();
    Key.Rot.Setup( L2W );    
}

//=============================================================================

void animation_obj::JumpToKey( s32 Index )
{
    // Make sure that we have a happy index
    Index = iMin( iMax( Index, 0 ), m_KeyFrame.GetCount()-1 );
    if( Index == -1 ) return;

    keyframe& Key = m_KeyFrame[Index];

    // Set the key zero data
    matrix4 L2W( vector3(1,1,1), Key.Rot, Key.Translate );
    OnTransform( L2W );
}

//=============================================================================

xbool animation_obj::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
        return( TRUE );

    //
    // This are the states for the animation object that the user can play with
    // this is actually usefull for game-side and for the editor so you can test the object
    //
    if( I.IsVar( "animation_obj\\Action" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarEnum( "NoAction" );
        }
        else
        {
            const char* pCmd = I.GetVarEnum();

            s32 iCmd = (s32)(pCmd[0]-'0');

            switch( iCmd )
            {
            case 1:
                if( (FLAGS_PLAY_BACKWARDS & m_Flags) != 0 )
                {
                    m_iFrame = GetNextFrameIndex( m_iFrame );
                    m_Flags &= (~FLAGS_PLAY_BACKWARDS);
                }
                break;
            case 2:
                if( (FLAGS_PLAY_BACKWARDS & m_Flags) == 0 )
                {
                    m_iFrame = GetNextFrameIndex( m_iFrame );
                    m_Flags |= FLAGS_PLAY_BACKWARDS;
                }                
                break;
            case 3:
                OnActivate( FALSE );
                break;
            case 4:
                OnActivate( TRUE );
                break;                    
            case 5:
                //OnActivate( TRUE );
                // restart
                break;                    
            default:
                return FALSE;
            }
        }

        return TRUE;
    }

    //
    // Set the type which is going to play
    //
    if( I.IsVar( "animation_obj\\PlayType" ) )
    {
        if( I.IsRead() )
        {
            if( m_Flags & FLAGS_LOOP )
            {
                I.SetVarEnum( "2.Loop" );
            }
            else if( m_Flags & FLAGS_PING_PONG )
            {
                I.SetVarEnum( "3.PinPong" );
            }
            else if( m_Flags & FLAGS_DESTROY_AFTER_PLAYING )
            {
                I.SetVarEnum( "1.OnesThenDie" );
            }
            else
            {
                I.SetVarEnum( "0.Ones" );
            }
        }
        else
        {
            const char* pCmd = I.GetVarEnum();

            s32 iCmd = (s32)(pCmd[0]-'0');

            switch( iCmd )
            {
            case 0:
                m_Flags &= (~(FLAGS_PING_PONG | FLAGS_LOOP | FLAGS_DESTROY_AFTER_PLAYING ));
                break;
            case 1:
                m_Flags &= (~(FLAGS_PING_PONG | FLAGS_LOOP | FLAGS_DESTROY_AFTER_PLAYING));
                m_Flags |= FLAGS_DESTROY_AFTER_PLAYING;
                break;
            case 2:
                m_Flags &= (~(FLAGS_PING_PONG | FLAGS_LOOP | FLAGS_DESTROY_AFTER_PLAYING ));
                m_Flags |= FLAGS_LOOP;
                break;
            case 3:
                m_Flags &= (~(FLAGS_PING_PONG | FLAGS_LOOP | FLAGS_DESTROY_AFTER_PLAYING ));
                m_Flags |= FLAGS_PING_PONG;
                break;                    
            default:
                return FALSE;
            }
        }

        return TRUE;
    }

    // This property only gets use in the editor. So we take advantage of
    // this case and if there is zero keys we will assume that its current position
    // is key frame zero. 
    if( I.IsVar( "animation_obj\\KeyCount"))
    {
        if( I.IsRead() )
        {
            if( m_KeyFrame.GetCount() == 0 )
            {
                keyframe& Key = m_KeyFrame.Append();
            
                // Set the key zero data
                SetEditKeyFrame(0);
                Key.Time = 0;

                // make sure we start at the end
                m_iCurrentKeyFrame = 1;
            }

            I.SetVarInt( m_KeyFrame.GetCount() );
        }
        else
        {
            ASSERT( I.IsRead() == TRUE );
        }
        return TRUE;
    }
    
    // This key count only is used for save and load stuff
    if( I.IsVar( "animation_obj\\SaveKeyCount"))
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_KeyFrame.GetCount() );
        }
        else
        {
            m_KeyFrame.SetCount( I.GetVarInt() );
        }
    }

    //
    // Deal with all the key frame editing
    //
    if( I.VarInt( "animation_obj\\CurrentKey", m_iCurrentKeyFrame, 0, m_KeyFrame.GetCount() ) )
        return TRUE;


    if( I.IsVar( "animation_obj\\KeyAction" ) )
    {
        if( I.IsRead() )
        {
            if( m_iCurrentKeyFrame >= m_KeyFrame.GetCount() )
            {
                I.SetVarButton( "New" );
            }
            else
            {
                I.SetVarButton( "Insert" );
            }
        }
        else
        {
            if( m_iCurrentKeyFrame >= m_KeyFrame.GetCount() )
            {
                // new Key 
                keyframe& Key = m_KeyFrame.Append();
            
                // Set the new key frame
                m_iCurrentKeyFrame = m_KeyFrame.GetCount();

                // Set the key value
                SetEditKeyFrame(m_iCurrentKeyFrame-1);
                Key.Time = 1.0f;

                // Set some default value
                m_CurrentKey = Key;
            }
            else
            {
            }
        }

        return TRUE;
    }

    //
    // If anyone wants to playaround with the actual key frames here it is.
    // This is also use for save and load.
    //
    if( I.IsVar( "animation_obj\\Key[]\\Rotate" ) )
    {
        if( I.IsRead() )
        {
            radian3 Rot(  m_KeyFrame[ I.GetIndex(0) ].Rot );
            I.SetVarRotation( Rot );
        }
        else
        {
            m_KeyFrame[ I.GetIndex(0) ].Rot.Setup( I.GetVarRotation() );
        }
        return TRUE;
    }

    if( I.VarVector3( "animation_obj\\Key[]\\Translate", m_KeyFrame[ I.GetIndex(0) ].Translate ))
    {
        return TRUE;
    }

    if( I.VarFloat( "animation_obj\\Key[]\\Time", m_KeyFrame[ I.GetIndex(0) ].Time, 0, 60.0f*60 ))
    {
        return TRUE;
    }

    if( I.IsVar( "animation_obj\\Key[]\\SetKey" ))
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "SetKey" );
        }
        else
        {
            s32 Index = I.GetIndex(0);
            SetEditKeyFrame( Index );
        }
    }

    if( I.IsVar( "animation_obj\\Key[]\\JumpToKey" ))
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "JumpToKey" );
        }
        else
        {
            s32 Index = I.GetIndex(0);
            JumpToKey( Index );
        }
        return TRUE;
    }


    if( I.IsVar( "animation_obj\\Key[]\\DeleteKey" ))
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "DeleteKey" );
        }
        else
        {
            s32 Index = I.GetIndex(0);
            m_KeyFrame.Delete( Index );
        }
    }
    


    /*
    if( I.VarInt( "animation_obj\\Bone Number", m_nGuids ) )
    {
        ASSERT( I.IsRead() == TRUE);
    }
    else if( I.IsVar( "animation_obj\\UpdateObjects" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Update" );
        }
        else
        {
            UpdateAnimL2W();
            UpdateObjects();
        }
    }
    else if( I.IsVar( "animation_obj\\Active" ) )
    {
        if( I.IsRead() ) I.SetVarBool( (m_Flags&FLAGS_ACTIVE) == FLAGS_ACTIVE );
        else             m_Flags = I.GetVarBool()?m_Flags|FLAGS_ACTIVE:m_Flags&~FLAGS_ACTIVE;

        // Make sure to set it active if we need to activated
        if(m_Flags&FLAGS_ACTIVE) 
            SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );
        else
            SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );            
    }
    else if( I.IsVar( "animation_obj\\Loop" ) )
    {
        if( I.IsRead() ) I.SetVarBool( (m_Flags&FLAGS_LOOP) == FLAGS_LOOP );
        else             m_Flags = I.GetVarBool()?m_Flags|FLAGS_LOOP:m_Flags&~FLAGS_LOOP;
    }
    else if( I.VarInt( "animation_obj\\Bone Number", m_nGuids ) )
    {
    }
    else if( I.VarGUID( "animation_obj\\Guid[]", m_Guid[I.GetIndex(0)] ))
    {
    }
    else if( I.IsVar( "animation_obj\\Anim" ) )
    {
        if( I.IsRead() )
        {
            if( m_hAnimGroup.IsLoaded() )
            {
                I.SetVarExternal( m_hAnimGroup.GetName(), RESOURCE_NAME_SIZE );
            }
            else
            {
                I.SetVarExternal( "\0", RESOURCE_NAME_SIZE );
            }
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
                    m_Anim.SetAnimGroup( m_hAnimGroup );
                    m_Anim.SetAnim( 0, TRUE );

                    // Set how many bones are avariable.
                    m_nGuids = m_Anim.GetNBones();
                    ASSERT( m_nGuids <= MAX_ANIMATED_OBJECTS );
                }
            }
        }
    }
    else if( I.IsVar( "animation_obj\\Audio Package" ) )
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
                m_hAudioPackage.SetName( pString );                

                // Load the audio package.
                if( m_hAudioPackage.IsLoaded() == FALSE )
                    m_hAudioPackage.GetPointer();
            }
        }
    }
    else
    {
        return FALSE;
    }
*/        
    return( TRUE );
}

//=========================================================================

#ifndef X_RETAIL
void animation_obj::OnDebugRender  ( void )
{
    CONTEXT( "animation_obj::OnRender" );

#ifdef X_EDITOR

    if( GetAttrBits() & ATTR_EDITOR_SELECTED )
    {
        // Draw a line to all the objects that it refers to
        s32 i;

        for( i=0; i<m_nBones; i++ )
        {
            if( m_Bone[i].ObjectGuid == 0 )
                continue;

            object* pObject = g_ObjMgr.GetObjectByGuid( m_Bone[i].ObjectGuid );
       
            if( pObject ) 
            {
                draw_Line( GetPosition(), pObject->GetBBox().GetCenter(), xcolor( 255,0,0,255) );
            }
        }

        for( i=0; i<m_KeyFrame.GetCount(); i++ )
        {
            matrix4 L2W( vector3(1,1,1), m_KeyFrame[i].Rot, m_KeyFrame[i].Translate );
            draw_SetL2W( L2W );
            draw_Axis  ( 100 );
            draw_Label ( m_KeyFrame[i].Translate + vector3(0,140,0), xcolor(255,255,255,255), "%d", i );

            // Render the conection of the key frame
            L2W.Identity();
            draw_SetL2W( L2W );
            if ( (i+1) < m_KeyFrame.GetCount() )
            {
                draw_Line( m_KeyFrame[i].Translate, m_KeyFrame[i+1].Translate, xcolor( 0,255,0,255) );
            }
            else
            {
                draw_Line( m_KeyFrame[i].Translate, m_KeyFrame[0].Translate, xcolor( 0,255,0,255) );
            }
        }

        // Show Progress
        {
            matrix4 L2W( vector3(1,1,1), m_CurrentKey.Rot, m_CurrentKey.Translate );
            draw_SetL2W( L2W );
            draw_Axis  ( 100 );
            draw_Label ( m_CurrentKey.Translate + vector3(0,160,0), xcolor(255,255,255,255), "Cursor" );
        }
        
    }
#endif // X_EDITOR
}
#endif // X_RETAIL

//=========================================================================

void animation_obj::UpdateAnimL2W( void )
{
    /*
    anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    if( pAnimGroup == NULL ) 
        return;

    matrix4 MBind;

    MBind.Identity();
    MBind.SetTranslation( pAnimGroup->GetBoneBindInvMatrix(0).GetTranslation() );
    MBind.RotateY( R_180 ); // Fix max bug
    MBind = GetL2W() * MBind;

    m_Anim.SetL2W( MBind );
    */
}

//=========================================================================

void animation_obj::OnMove( const vector3& NewPos )
{
    object::OnMove( NewPos );

    //UpdateAnimL2W();
    //UpdateObjects();

    //m_BindPose.SetTranslation( NewPos );
}

//=========================================================================

void animation_obj::OnTransform( const matrix4& L2W )
{
    object::OnTransform( L2W );

    //UpdateAnimL2W();
    //UpdateObjects();

    //m_BindPose = L2W;
}

//=========================================================================

void animation_obj::UpdateObjects( void )
{
/*
    // If nothing is loaded there is nothing to do
    if( m_hAnimGroup.IsLoaded() == FALSE )
        return;

    //
    // Compute the Local to world matrix
    //
    const matrix4* pMatrix = m_Anim.GetBoneL2Ws(FALSE) ;
    if (!pMatrix)
        return ;

    //
    // Move the objects
    //
    matrix4 MaxBug;
    MaxBug.Identity();
    MaxBug.RotateY( R_180 );
    MaxBug.RotateX( R_90 );

    for( s32 i=0; i<m_nGuids; i++ )
    {
        if( m_Guid[i] == 0 )
            continue;

        object* pObject = g_ObjMgr.GetObjectByGuid( m_Guid[i] );
       
        if( pObject ) 
        {
            pObject->OnTransform( pMatrix[i] * MaxBug ); 
        }
    }
    */
}

//=========================================================================

s32 animation_obj::GetNextFrameIndex( s32 iFrame )
{
    ASSERT( m_KeyFrame.GetCount() > 1 );

    if( m_Flags & FLAGS_PLAY_BACKWARDS )
    {
        // Test end condition
        if( iFrame == 0 )
        {
            if( m_Flags & FLAGS_LOOP )
            {
                iFrame = m_KeyFrame.GetCount()-1;
            }
            else if( m_Flags & FLAGS_PING_PONG )
            {
                iFrame = m_KeyFrame.GetCount()-1;
            }
            else 
                return -1;
        }
        else
        {
            // Go to previous key
            iFrame--;
        }
    }
    else
    {
        // Test end condition
        if( iFrame == (m_KeyFrame.GetCount()-1) )
        {
            if( m_Flags & FLAGS_LOOP )
            {
                iFrame = 0;
            }
            else if( m_Flags & FLAGS_PING_PONG )
            {
                iFrame = 0;
            }
            else 
                return -1;
        }
        else
        {
            // Go to next key
            iFrame++;
        }
    }

    return iFrame;
}

//=========================================================================

xbool animation_obj::HandleStates( f32 DeltaTime, f32& ParamTime, s32& iNext )
{
    // If only one frame there is nothing to do
    if( m_KeyFrame.GetCount() <= 1 )
        return FALSE;

    // Get the next node
    iNext = GetNextFrameIndex( m_iFrame );
    ASSERT( iNext != -1 );

    // This is the total time we must travel
    xbool bBackWards = !!(m_Flags & FLAGS_PLAY_BACKWARDS);
    f32 TotalTime;
    if( bBackWards )
    {
        TotalTime = m_KeyFrame[ m_iFrame ].Time;
    }
    else
    {
        TotalTime = m_KeyFrame[ iNext ].Time;
    }
    
    // Advance the time
    m_Time += DeltaTime;

    // OKay the time is going to roll over
    if( m_Time >= TotalTime )
    {
        // handle the ping-pong
        if( m_Flags & FLAGS_PING_PONG )
        {
            if( m_iFrame == 1 && iNext == 0 )
            {
                ASSERT( (m_Flags & FLAGS_PLAY_BACKWARDS) );
                m_Flags &= (~FLAGS_PLAY_BACKWARDS);
            }
            else if( m_iFrame == 2 && iNext == 0 )
            {
                ASSERT( !(m_Flags & FLAGS_PLAY_BACKWARDS) );
                m_Flags |= FLAGS_PLAY_BACKWARDS;                
            }
        }

        // Move on to the next
        m_iFrame = iNext;

        // Check what the next node is going to be
        iNext = GetNextFrameIndex( m_iFrame );

        // if there is no other node to go to then we are done
        if( iNext == -1 )
        {
            if( m_Flags & FLAGS_DESTROY_AFTER_PLAYING )
            {
                g_ObjMgr.DestroyObject( GetGuid() );
                return FALSE;
            }
            else
            {
                OnActivate( FALSE );
                return FALSE;
            }
        }

        // Okay lets spend the remaining of the time with this node
        m_Time = m_Time - TotalTime;

        // We should recurse here to see if we are out of time again
        // But we will cheat and say there is not need for that.
        // Get the total time again
        bBackWards = !!(m_Flags & FLAGS_PLAY_BACKWARDS);
        if( bBackWards )
        {
            TotalTime = m_KeyFrame[ m_iFrame ].Time;
        }
        else
        {
            TotalTime = m_KeyFrame[ iNext ].Time;
        }
    }

    ParamTime = m_Time / TotalTime;
    return TRUE;
}

//=========================================================================

void animation_obj::ComputeFrame( keyframe& Key, s32 iPrev, s32 iNext, f32 Time )
{
    //matrix4 L2W;
    keyframe& Prev = m_KeyFrame[ iPrev ];
    keyframe& Next = m_KeyFrame[ iNext ];

    Key.Time = Time;
    Key.Translate = Prev.Translate + Key.Time * ( Next.Translate - Prev.Translate );
    Key.Rot = BlendSlow( Prev.Rot, Next.Rot, Key.Time );
}

//=========================================================================

void animation_obj::OnAdvanceLogic( f32 DelaTime )
{
    CONTEXT( "animation_obj::OnAdvanceLogic" );

    // OKay advance everything that we need to
    f32 ParamTime;
    s32 iNext;
    if( HandleStates( DelaTime, ParamTime, iNext ) == FALSE )
         return;

    // Handle the math to compute where the object should be move
    ComputeFrame( m_CurrentKey, m_iFrame, iNext, ParamTime );

    // Move
    //UpdateObjects();
}

//=========================================================================

void animation_obj::OnActivate( xbool Flag )
{
    if( Flag )
    {
        SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );
        m_Flags |= FLAGS_ACTIVE;
    }
    else
    {
        SetAttrBits( GetAttrBits() & (~object::ATTR_NEEDS_LOGIC_TIME) );
        m_Flags &= ~(FLAGS_ACTIVE);
    }
}
