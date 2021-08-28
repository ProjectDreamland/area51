#include "LocoSpecialOps.hpp"
#pragma warning( disable : 4355 ) // warning 'this' used in base member initializer list

//==============================================================================
//==============================================================================
//==============================================================================
// SPEC OPS ANIM
//==============================================================================
//==============================================================================
//==============================================================================

spec_ops_state*  spec_ops_state::s_pHead = NULL;

//==============================================================================

spec_ops_state::spec_ops_state( spec_ops_loco& Loco, locomotion::states State ) :
    m_Base  ( Loco ),
    m_State ( State ),
    m_pNext ( s_pHead )
{
    s_pHead = this;
}

//==============================================================================
//==============================================================================
//==============================================================================
// LOCO SPEC OPS
//==============================================================================
//==============================================================================
//==============================================================================

//==============================================================================

xbool spec_ops_loco::SetState( locomotion::states State )
{
    if( m_pActive && m_pActive->OnExit() == FALSE )
        return FALSE;

    spec_ops_state* pNext = spec_ops_state::s_pHead;
    while( pNext )
    {
        if( State == pNext->m_State )
        {
            m_pActive = pNext;
            m_pActive->OnEnter();
            return TRUE;
        }

        pNext = pNext->m_pNext;
    }

    return FALSE;
}

//==============================================================================

xbool spec_ops_loco::Advance( f32 nSeconds )
{
    if( m_pActive )
    {
        return m_pActive->OnAdvance( nSeconds );
    }
    return FALSE;
}

//==============================================================================

void spec_ops_loco::OnAdvance( f32 nSeconds )
{
    locomotion::OnAdvance( nSeconds );
}

//==============================================================================

void spec_ops_loco::OnInit( void )
{
    // Call Init to the register states
    spec_ops_state* pNext = spec_ops_state::s_pHead;
    while( pNext )
    {
        pNext->OnInit();
        pNext = pNext->m_pNext;
    }
}

//==============================================================================
spec_ops_loco::spec_ops_loco( void ) :
    m_Default ( *this ),
    m_Run     ( *this )
    //Idle    ( *this ),
{
    m_pActive = NULL;
}

//==============================================================================
// DEFAULT
//==============================================================================

//==============================================================================
spec_ops_default::spec_ops_default( spec_ops_loco& Loco ) : 
    spec_ops_state( Loco, locomotion::STATE_IDLE ) 
{
}

//==============================================================================

xbool spec_ops_default::OnAdvance( f32 nSeconds )
{
    vector3 DeltaPos;

    //
    // Handle the ending of a turn
    //
    m_Timer    += nSeconds;
    if( m_sTurning && m_Base.m_Player.IsAtEnd() )
    {
        // What happen with the remaining time?
        if( m_sTurning == 1 )
        {
            //m_Base.m_Player.SetYaw( m_Base.m_Player.GetYaw() -R_45 );
        }
        else
        {
            //m_Base.m_Player.SetYaw( m_Base.m_Player.GetYaw() +R_45 );
        }
        m_Base.m_Player.SetAnimHoriz( "HSO_DEFAULT_IDLE",0 );
        m_sTurning = 0;
    }

    //
    // Set the vector to the angle that we are moving at
    //
    f32 H,V;
    m_Base.ComputeAim( H, V );
    if( H < -1 )
    {
        H = -1;

        // Go into a turn
        m_sTurning = 1;
        m_Base.m_Player.SetAnimHoriz( "HSO_DEFAULT_TURN_RIGHT" );
    }
    else if( H > 1 )
    {
        H = 1;

        // Go into a turn
        m_sTurning = -1;
        m_Base.m_Player.SetAnimHoriz( "HSO_DEFAULT_TURN_LEFT" );//
    }

    m_Base.m_Controler.SetBlendFactor( H, V, 0 );

    //
    // Advance the animation
    //
    //m_Base.m_Player.Advance     ( nSeconds, DeltaPos );
    //m_Base.m_Player.SetPosition ( m_Base.m_Player.GetPosition() + DeltaPos );

    //NEW
    radian  DeltaYaw ;
    m_Base.m_Player.Advance     ( nSeconds, DeltaPos, DeltaYaw );
    m_Base.m_Player.SetYaw      ( x_ModAngle2(m_Base.m_Player.GetYaw()      + DeltaYaw) );
    m_Base.m_Player.SetPosition ( m_Base.m_Player.GetPosition() + DeltaPos );


    //
    // Do we have to transition to another state?
    //    
    if( (m_Timer > 0.0f ) && (m_Base.m_Player.GetPosition() - m_Base.m_MoveAt).LengthSquared() > (80*80) )
    {
        //m_Base.m_Player.SetOverrideRootBlend( TRUE );
        m_Base.SetState( locomotion::STATE_RUN );
    }

    return TRUE;
}

//==============================================================================

void spec_ops_default::OnEnter( void )
{
    m_Timer    = 0;
    m_sTurning = 0;
    m_Base.m_Motion = locomotion::MOTION_IDLE;

    m_Base.m_Player.SetAnimHoriz( "HSO_DEFAULT_IDLE", 0.25f );

    m_Base.m_Controler.Init(  m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS1"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS2"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS3"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS4"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS5"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS6"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS7"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS8"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS9") );                              


//    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 Spine"   );

    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 Spine1"  );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 Spine2"  );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 Spine3"  );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 Neck"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 Head"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 HeadNub" );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Clavicle"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L UpperArm"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Forearm"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Hand"       );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Clavicle"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R UpperArm"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Forearm"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Hand"       );    
    
}

//==============================================================================

xbool spec_ops_default::OnExit( void )
{
    return TRUE;
}

//==============================================================================
// RUN
//==============================================================================

//==============================================================================

spec_ops_run::spec_ops_run( spec_ops_loco& Loco ) 
: spec_ops_state( Loco, locomotion::STATE_RUN )
{
}

//==============================================================================

void spec_ops_run::OnInit( void )
{
}

//==============================================================================

xbool spec_ops_run::OnAdvance( f32 nSeconds )
{
    vector3 DesireDir;
    vector3 CurDir;

    m_Timer += nSeconds;

    
    locomotion::motion Motion = m_Base.ComputeMotion();
    //if (        (m_Base.m_Motion != locomotion::MOTION_TRANSITION)
            //&&  (m_Base.m_Motion != Motion) )
    //{
        //m_Base.m_Player.SetYaw(CurrYaw) ;
    //}

    //
    // choose which animation we should be playing
    //    

#if 0

    {

        /*
        switch( Motion )
        {
        case locomotion::MOTION_FORWARD:
            x_DebugMsg ( "MOTION_FORWARD\n" ); 
            break;
        case locomotion::MOTION_LEFT:
            x_DebugMsg( "MOTION_LEFT\n" );
            break;
        case locomotion::MOTION_RIGHT:
            x_DebugMsg( "MOTION_RIGHT\n" );
            break;
        case locomotion::MOTION_BACK:
            x_DebugMsg( "MOTION_BACK\n" );
            break;
        case locomotion::MOTION_TRANSITION:
            x_DebugMsg( "MOTION_TRANSITION\n" );
            break;
        };
        */

        if(
            //(!((m_Base.m_Motion == locomotion::MOTION_TRANSITION) && (!m_Base.m_Player.IsAtEnd()))) &&

            (  ( ( ( ( m_Base.m_Motion != locomotion::MOTION_TRANSITION ) && Motion != m_Base.m_Motion ) || 
                 ( m_Base.m_ToTransition != Motion )
               ) && 1
             ) ||
             (( m_Base.m_Motion == locomotion::MOTION_TRANSITION ) && m_Base.m_Player.IsAtEnd())
          ))
        {

            //m_Base.m_Player.SetOverrideRootBlend( TRUE );
            m_Timer = 0;

            
            if( m_Base.m_Motion == locomotion::MOTION_FORWARD && Motion == locomotion::MOTION_LEFT )
            {
                m_Base.m_Player.SetAnimHoriz( "HSO_PROWL_BLEND_F2L");
                m_Base.m_Motion       = locomotion::MOTION_TRANSITION;
                m_Base.m_ToTransition = locomotion::MOTION_LEFT;
                x_printf( "Transion: F 2 L\n");
            }
            else if( m_Base.m_Motion == locomotion::MOTION_FORWARD && Motion == locomotion::MOTION_RIGHT )
            {
                m_Base.m_Player.SetAnimHoriz( "HSO_PROWL_BLEND_F2R");
                m_Base.m_Motion       = locomotion::MOTION_TRANSITION;
                m_Base.m_ToTransition = locomotion::MOTION_RIGHT;
                x_printf( "Transion: F 2 L\n");
            }
            else if( m_Base.m_Motion == locomotion::MOTION_BACK && Motion == locomotion::MOTION_LEFT )
            {
                m_Base.m_Player.SetAnimHoriz( "HSO_PROWL_BLEND_B2L");
                m_Base.m_Motion       = locomotion::MOTION_TRANSITION;
                m_Base.m_ToTransition = locomotion::MOTION_LEFT;
                x_printf( "Transion: B 2 L\n");
            }
            else if( m_Base.m_Motion == locomotion::MOTION_BACK && Motion == locomotion::MOTION_RIGHT )
            {
                m_Base.m_Player.SetAnimHoriz( "HSO_PROWL_BLEND_B2R");
                m_Base.m_Motion       = locomotion::MOTION_TRANSITION;
                m_Base.m_ToTransition = locomotion::MOTION_RIGHT;
                x_printf( "Transion: B 2 L\n");
            }
            else if( m_Base.m_Motion == locomotion::MOTION_LEFT && Motion == locomotion::MOTION_BACK )
            {
                m_Base.m_Player.SetAnimHoriz( "HSO_PROWL_BLEND_L2B");
                m_Base.m_Motion       = locomotion::MOTION_TRANSITION;
                m_Base.m_ToTransition = locomotion::MOTION_BACK;
                x_printf( "Transion: L 2 B\n");
            }
            else if( m_Base.m_Motion == locomotion::MOTION_LEFT && Motion == locomotion::MOTION_FORWARD )
            {
                m_Base.m_Player.SetAnimHoriz( "HSO_PROWL_BLEND_L2F");
                m_Base.m_Motion       = locomotion::MOTION_TRANSITION;
                m_Base.m_ToTransition = locomotion::MOTION_FORWARD;
                x_printf( "Transion: L 2 F\n");
            }
            else if( m_Base.m_Motion == locomotion::MOTION_RIGHT && Motion == locomotion::MOTION_BACK )
            {
                m_Base.m_Player.SetAnimHoriz( "HSO_PROWL_BLEND_R2B");
                m_Base.m_Motion       = locomotion::MOTION_TRANSITION;
                m_Base.m_ToTransition = locomotion::MOTION_BACK;
                x_printf( "Transion: L 2 B\n");
            }
            else if( m_Base.m_Motion == locomotion::MOTION_RIGHT && Motion == locomotion::MOTION_FORWARD )
            {
                m_Base.m_Player.SetAnimHoriz( "HSO_PROWL_BLEND_R2F");
                m_Base.m_Motion       = locomotion::MOTION_TRANSITION;
                m_Base.m_ToTransition = locomotion::MOTION_FORWARD;
                x_printf( "Transion: L 2 F\n");
            }
            else
            
            {
                switch( Motion )
                {
                case locomotion::MOTION_FORWARD:
                    m_Base.m_Player.SetAnimHoriz( "HSO_PROWL_FORWARD") ;
                    m_Base.m_Motion = locomotion::MOTION_FORWARD;
                    x_printf( "Move: F\n");
                    break;
                case locomotion::MOTION_LEFT:
                    m_Base.m_Player.SetAnimHoriz( "HSO_PROWL_STRAFE_L") ;
                    m_Base.m_Motion = locomotion::MOTION_LEFT;
                    x_printf( "Move: L\n");
                    break;
                case locomotion::MOTION_RIGHT:
                    m_Base.m_Player.SetAnimHoriz( "HSO_PROWL_STRAFE_R") ;
                    m_Base.m_Motion = locomotion::MOTION_RIGHT;
                    x_printf( "Move: R\n");
                    break;
                case locomotion::MOTION_BACK:
                    m_Base.m_Player.SetAnimHoriz( "HSO_PROWL_BACKWARD") ;
                    m_Base.m_Motion = locomotion::MOTION_BACK;
                    x_printf( "Move: B\n");
                    break;
                }

                if (Motion != locomotion::MOTION_TRANSITION)
                    m_Base.m_ToTransition = Motion;
            }
        }
    }
#endif

    //
    // Set the vector to the angle that we are looking at
    //
    f32 H,V;
    m_Base.ComputeAim( H, V );
    H = fMin( 1, fMax( -1, H) );
    m_Base.m_Controler.SetBlendFactor( H, V );

    //
    // Advance anim
    //
    vector3 DeltaPos;
    radian  DeltaYaw ;

    m_Base.m_Player.Advance     ( nSeconds, DeltaPos, DeltaYaw );
    m_Base.m_Player.SetYaw      ( x_ModAngle2(m_Base.m_Player.GetYaw() + DeltaYaw) );

    // If playing a transition, rotate the delta pos into the yaw before the transition
    //if (m_Base.m_bTransition)
    {
        DeltaYaw = m_Base.m_Player.GetYaw() - m_Base.m_MotionYaw ;
        DeltaPos.RotateY(-DeltaYaw) ;
        
        //DeltaPos.Zero() ;
    }

    m_Base.m_Player.SetPosition ( m_Base.m_Player.GetPosition() + DeltaPos );

    //
    // Switch state if we can
    //
    if( (m_Base.m_MoveAt - m_Base.m_Player.GetPosition()).LengthSquared() < (50*50) )
    {
        vector3 DesireDir = m_Base.m_LookAt - m_Base.m_Player.GetPosition();
        f32     H         = v3_AngleBetween( DesireDir, vector3(0,0,1) );

        if( DesireDir.Dot( vector3(1,0,0) ) < 0 )
            H = -H;
        //m_Base.m_Player.SetYaw( H );

        //m_Base.m_Player.SetOverrideRootBlend( TRUE );
        m_Base.SetState( locomotion::STATE_IDLE );
    }

    return TRUE;
}

//==============================================================================

void spec_ops_run::OnEnter( void )
{
    m_Timer = 10;

    m_Base.m_Controler.Init(  m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS1"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS2"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS3"),
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS4"),
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS5"),
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS6"),
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS7"),
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS8"),
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS9")
                              );


    m_Base.m_Controler.Init(  m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS1"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS2"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS3"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS4"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS5"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS6"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS7"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS8"), 
                              m_Base.m_Player.GetAnimIndex("HSO_LOOK_POS9") );                              


    m_Base.m_Controler.SetBoneMask( 0.3f, "Bip01 Spine"   );
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 Spine1"  );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 Spine2"  );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 Spine3"  );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 Neck"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 Head"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 HeadNub" );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Clavicle"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L UpperArm"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Forearm"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Hand"       );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Clavicle"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R UpperArm"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Forearm"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Hand"       );    

/*
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger0"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger01"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger02"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger0Nub" );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger1"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger11"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger12"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger1Nub" );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger2"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger21"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger22"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger2Nub" );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger3"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger31"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger32"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger3Nub" );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger4"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger41"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger42"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 L Finger4Nub" );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger0"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger01"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger02"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger0Nub" );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger1"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger11"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger12"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger1Nub" );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger2"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger21"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger22"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger2Nub" );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger3"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger31"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger32"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger3Nub" );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger4"    );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger41"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger42"   );    
    m_Base.m_Controler.SetBoneMask( 1.0f, "Bip01 R Finger4Nub" );    
  */  
    /*
    "Bip01 L Thigh"     
    "Bip01 L Calf"      
    "Bip01 L Foot"      
    "Bip01 L Toe0"      `
    "Bip01 L Toe0Nub"   
    "Bip01 R Thigh"     
    "Bip01 R Calf"      
    "Bip01 R Foot"      
    "Bip01 R Toe0"      
    "Bip01 R Toe0Nub"   
    "Bip01 Prop1"       
    */
}

//==============================================================================

xbool spec_ops_run::OnExit( void )
{
   // m_Base.m_Controler.Clear();
    return TRUE;
}
