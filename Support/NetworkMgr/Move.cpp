//==============================================================================
//
//  Move.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#if 0

//==============================================================================
//	DEBUG DEFINES AND MACROS
//==============================================================================

// Comment or uncomment to log update details.
//#define LOG_MOVE_DETAILS  

// If X_DEBUG is defined, then force LOG_MOVE_DETAILS to be defined.
#if defined(mtraub) && defined(X_DEBUG) && !defined(LOG_MOVE_DETAILS)
//#define LOG_MOVE_DETAILS
#endif

// If logging move details, then define a helper macro.
#ifdef LOG_MOVE_DETAILS
#define LOG(c) Msg[C++] = c;
#else
#define LOG(c) (void)0
#endif    

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Move.hpp"
#include "x_bitstream.hpp"
#include "x_log.hpp"

//==============================================================================
//  FUNCTIONS
//==============================================================================

move::move( void )
{
    Slot      = -1;
    Seq       = -1;
    SendsLeft =  0;
    ACKs      =  0;
    NACKs     =  0;
    SendLimit =  0;
}

//==============================================================================

void move::AddPain( const move& Move )
{
    for( s32 i = 0; i < 32; i++ )
    {
        if( Move.Pain[i].Amount > Pain[i].Amount )
        {
            Pain[i].Type = Move.Pain[i].Type;
            Pain[i].Yaw  = Move.Pain[i].Yaw;
        }

        Pain[i].Amount += Move.Pain[i].Amount;
    }
}

//==============================================================================

void move::Read( bitstream& BS, const move& Compress )
{
    s32     i;
    xbool   Log = FALSE;   // Set if move is "interesting" for logging purposes.

    #ifdef LOG_MOVE_DETAILS
    // Variables for move logging.
    char   Msg[64];
    s32    C = 0;
    x_memset( Msg, 0, 64 );
    #endif

    LOG('[');
    LOG('0' + Slot / 10);
    LOG('0' + Slot % 10);
    LOG(']');

    Position        = Compress.Position;
    Pitch           = Compress.Pitch;
    Yaw             = Compress.Yaw;
    JumpSeq         = Compress.JumpSeq;
    LifeSeq         = Compress.LifeSeq;
    Weapon          = Compress.Weapon;
    ReloadSeq       = Compress.ReloadSeq;
    FireSeq         = Compress.FireSeq;
    TossSeq         = Compress.TossSeq;
    MeleeSeq        = Compress.MeleeSeq;

//  FirePos         = Compress.FirePos; 
//  FireVel         = Compress.FireVel;
//  FireStrength    = Compress.FireStrength;
//  FireWeaponIndex = Compress.FireWeaponIndex;
//  TossPos         = Compress.TossPos;
//  TossVel         = Compress.TossVel;
//  TossStrength    = Compress.TossStrength;
  
    BS.ReadMarker();

//  BS.ReadRangedS32( Slot,  0, 31 );
//  BS.ReadRangedS32( Team, -1,  1 );

    BS.ReadFlag     ( Respawn      );
    BS.ReadFlag     ( Crouch       );

    xbool P = FALSE;
    xbool O = FALSE;
                            
    if( BS.ReadFlag() )   { BS.ReadF32( Position.GetX() ); P=TRUE; } // TO DO - Limit bits.
    if( BS.ReadFlag() )   { BS.ReadF32( Position.GetY() ); P=TRUE; } // TO DO - Limit bits.
    if( BS.ReadFlag() )   { BS.ReadF32( Position.GetZ() ); P=TRUE; } // TO DO - Limit bits.
    if( BS.ReadFlag() )   { BS.ReadF32( Pitch );           O=TRUE; } // TO DO - Limit bits.
    if( BS.ReadFlag() )   { BS.ReadF32( Yaw   );           O=TRUE; } // TO DO - Limit bits.

    if(P) LOG('P');
    if(O) LOG('O');

    if( BS.ReadFlag() )   { BS.ReadRangedS32( JumpSeq  , 0, 7 ); LOG('J'); }
    if( BS.ReadFlag() )   { BS.ReadRangedS32( LifeSeq  , 0, 7 ); LOG('L'); Log = TRUE; }
    if( BS.ReadFlag() )   { BS.ReadS32      ( Weapon );          LOG('W'); } // TO DO - Limit bits.
    if( BS.ReadFlag() )   { BS.ReadRangedS32( ReloadSeq, 0, 7 ); LOG('R'); }
    if( BS.ReadFlag() )   { BS.ReadRangedS32( MeleeSeq , 0, 7 ); LOG('M'); }

    if( BS.ReadFlag() )
    {
        BS.ReadRangedS32( FireSeq, 0, 7 );
//      BS.ReadVector   ( FireVel      );
//      BS.ReadVector   ( FirePos      );
//      BS.ReadF32      ( FireStrength );
//      BS.ReadRangedS32( FireWeaponIndex, 0, 3 );
        Log = TRUE;
        LOG('F');
    }

    if( BS.ReadFlag() )
    {
        BS.ReadRangedS32( TossSeq, 0, 7 );
        BS.ReadVector   ( TossPos       );
        BS.ReadVector   ( TossVel       );
        BS.ReadFlag     ( TossFrag      );
    //  BS.ReadF32      ( TossStrength  );
        Log = TRUE;
        LOG('T');
    }

/*
    for( i = 0; i < 4; i++ )
    {
        if( BS.ReadFlag() )
        {
            BS.ReadS32( AmmoClip   [i] );
            BS.ReadS32( AmmoReserve[i] );
            Log = TRUE;
        }
    }
*/
    if( BS.ReadFlag() )
    {
        for( i = 0; i < 32; i++ )
        {
            if( BS.ReadFlag() )
            {
                BS.ReadS32      ( Pain[i].Amount );
                BS.ReadRangedS32( Pain[i].Type, -1, 50 );
                BS.ReadF32      ( Pain[i].Yaw );
            }
            else
            {
                Pain[i].Amount =   0;
                Pain[i].Type   =  -1;
                Pain[i].Yaw    = R_0;
            }
        }
        Log = TRUE;
    }
    else
    {
        for( i = 0; i < 32; i++ )
        {
            Pain[i].Amount =   0;
            Pain[i].Type   =  -1;
            Pain[i].Yaw    = R_0;
        }
    }

    BS.ReadMarker();

    #ifdef LOG_MOVE_DETAILS
    // Only display moves which are "interesting".
    if( Log )
        LOG_MESSAGE( "move::Read", "Data:%s", Msg );
    #endif        
}

//==============================================================================

void move::Write( bitstream& BS, const move& Compress )
{
    s32     i;
    xbool   Log     = FALSE;   // Set if "interesting" for logging purposes.
    xbool   GotPain = FALSE;
    vector3 DPos    = Position - Compress.Position;
    radian  DPitch  = Pitch    - Compress.Pitch;
    radian  DYaw    = Yaw      - Compress.Yaw;

    #ifdef LOG_MOVE_DETAILS
    // Variables for move logging.
    char   Msg[64];
    s32    C = 0;
    x_memset( Msg, 0, 64 );
    #endif

    LOG('[');
    LOG('0' + Slot / 10);
    LOG('0' + Slot % 10);
    LOG(']');

    JumpSeq   &= 0x07;
    LifeSeq   &= 0x07;
    ReloadSeq &= 0x07;
    FireSeq   &= 0x07;
    TossSeq   &= 0x07;
    MeleeSeq  &= 0x07;

    BS.WriteMarker();

//  BS.WriteRangedS32( Slot,  0, 31 );
//  BS.WriteRangedS32( Team, -1,  1 );

    BS.WriteFlag     ( Respawn      );
    BS.WriteFlag     ( Crouch       );

    xbool P = FALSE;
    xbool O = FALSE;

    if( BS.WriteFlag( !IN_RANGE( -0.005f, DPos.GetX(), 0.005f ) ) )
        { BS.WriteF32( Position.GetX() ); P = TRUE; }
    else
        Position.GetX() = Compress.Position.GetX();

    if( BS.WriteFlag( !IN_RANGE( -0.005f, DPos.GetY(), 0.005f ) ) )
        { BS.WriteF32( Position.GetY() ); P = TRUE; }
    else
        Position.GetY() = Compress.Position.GetY();

    if( BS.WriteFlag( !IN_RANGE( -0.005f, DPos.GetZ(), 0.005f ) ) )
        { BS.WriteF32( Position.GetZ() ); P = TRUE; }
    else
        Position.GetZ() = Compress.Position.GetZ();

    if( BS.WriteFlag( !IN_RANGE( -0.001f, DPitch, 0.001f ) ) )
        { BS.WriteF32( Pitch ); O=TRUE; }
    else
        Pitch = Compress.Pitch;

    if( BS.WriteFlag( !IN_RANGE( -0.001f, DYaw  , 0.001f ) ) )
        { BS.WriteF32( Yaw ); O=TRUE; }
    else
        Yaw = Compress.Yaw;

    if(P) LOG('P');
    if(O) LOG('O');

    if( BS.WriteFlag( JumpSeq   != Compress.JumpSeq   ) )  { BS.WriteRangedS32( JumpSeq  , 0, 7 ); LOG('J'); }
    if( BS.WriteFlag( LifeSeq   != Compress.LifeSeq   ) )  { BS.WriteRangedS32( LifeSeq  , 0, 7 ); LOG('L'); Log = TRUE; }
    if( BS.WriteFlag( Weapon    != Compress.Weapon    ) )  { BS.WriteS32      ( Weapon );          LOG('W'); }
    if( BS.WriteFlag( ReloadSeq != Compress.ReloadSeq ) )  { BS.WriteRangedS32( ReloadSeq, 0, 7 ); LOG('R'); }
    if( BS.WriteFlag( MeleeSeq  != Compress.MeleeSeq  ) )  { BS.WriteRangedS32( MeleeSeq , 0, 7 ); LOG('M'); }

//  if( (Fire            != Compress.Fire           ) ||
//      (FireVel         != Compress.FireVel        ) ||
//      (FireStrength    != Compress.FireStrength   ) ||
//      (FireWeaponIndex != Compress.FireWeaponIndex) )
    if( FireSeq != Compress.FireSeq )
    {
        BS.WriteFlag     ( TRUE );
        BS.WriteRangedS32( FireSeq, 0, 7 );
//      BS.WriteVector   ( FireVel      );
//      BS.WriteVector   ( FirePos      );
//      BS.WriteF32      ( FireStrength );
//      BS.WriteRangedS32( FireWeaponIndex, 0,    3 );
        Log = TRUE;
        LOG('F');
    }
    else
    {
        BS.WriteFlag     ( FALSE );
    }

    if( TossSeq != Compress.TossSeq )
    {
        BS.WriteFlag     ( TRUE );
        BS.WriteRangedS32( TossSeq, 0, 7 );
        BS.WriteVector   ( TossPos       );
        BS.WriteVector   ( TossVel       );
        BS.WriteFlag     ( TossFrag      );
    //  BS.WriteF32      ( TossStrength  );
        Log = TRUE;
        LOG('T');
    }
    else
    {
        BS.WriteFlag     ( FALSE );
    }
/*
    for( i=0 ; i<4 ; i++ )
    {
        if( (AmmoClip   [i] != Compress.AmmoClip   [i]) ||
            (AmmoReserve[i] != Compress.AmmoReserve[i]) )
        {
            BS.WriteFlag( TRUE );
            BS.WriteS32 ( AmmoClip   [i] );
            BS.WriteS32 ( AmmoReserve[i] );
            Log = TRUE;
        }
        else
        {
            BS.WriteFlag( FALSE );
        }
    }
*/
    // Scan the pain list for ANY pain...
    for( i = 0; i < 32; i++ )
    {
        if( Pain[i].Amount > 0 )
        {
            GotPain = TRUE;     // Got some pain!
            break;
        }
    }

    if( BS.WriteFlag( GotPain ) )
    {
        for( i = 0; i < 32; i++ )
        {
            if( BS.WriteFlag( Pain[i].Amount > 0 ) )
            {
                BS.WriteS32      ( Pain[i].Amount );
                BS.WriteRangedS32( Pain[i].Type, -1, 50 );
                BS.WriteF32      ( Pain[i].Yaw );
            }
        }
        Log = TRUE;
    }

    BS.WriteMarker();

    #ifdef LOG_MOVE_DETAILS
    // Only display moves which are "interesting".
    if( Log )
        LOG_MESSAGE( "move::Write", "Data:%s", Msg );
    #endif        
}

//==============================================================================
#endif
