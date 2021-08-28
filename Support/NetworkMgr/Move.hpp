//==============================================================================
//
//  Move.hpp
//
//==============================================================================

#ifndef MOVE_HPP
#define MOVE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_math.hpp"
#include "MoveMgr.hpp"

//==============================================================================
//  DEFINES
//==============================================================================


//==============================================================================
//  TYPES
//==============================================================================

class bitstream;

//==============================================================================
                
struct move     
{               
    s32         Slot;
//  s32         Team;
                
    vector3     Position;
    radian      Pitch;
    radian      Yaw;

    xbool       Crouch;
    s32         JumpSeq;            // & 0x07

    s32         LifeSeq;            // & 0x07
    xbool       Respawn;

    s32         Weapon;             
    s32         ReloadSeq;          // & 0x07
    s32         FireSeq;            // & 0x07
    s32         TossSeq;            // & 0x07
    s32         MeleeSeq;           // & 0x07

    net_pain    Pain[32];

    vector3     TossPos;            // See comments for FirePos below.
    vector3     TossVel;
    xbool       TossFrag;

/*
    vector3     FirePos;            // If we send over bullet data, can't fire
    vector3     FireVel;            // more than one bullet if seq number is
    f32         FireStrength;       // off by more than one.
    s32         FireWeaponIndex;

    vector3     TossPos;            // See above.
    vector3     TossVel;
    f32         TossStrength;

    s32         AmmoClip[4];        // How many weapons are there?
    s32         AmmoReserve[4];     // Do we even need to send clip and ammo?
*/

    s32         Seq;
    s32         ACKs;       // Number of ACKs.
    s32         NACKs;      // Number of NACKs.
    s32         SendsLeft;  // Counts down to 0.
    s32         SendLimit;  // Total number of send attempts to try.
                
                move    ( void );
    void        Read    ( bitstream& BitStream, const move& Compress );
    void        Write   ( bitstream& BitStream, const move& Compress );
    void        AddPain ( const move& Move );
};

//==============================================================================
#endif // MOVE_HPP
//==============================================================================
