///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  brain_spec_ops.hpp
//
//      - brain specialization for a special ops AI
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef BRAIN_SPEC_OPS__HPP
#define BRAIN_SPEC_OPS__HPP

#include "brain.hpp"
#include "locomotion\locostates.hpp"

class brain_spec_ops : public brain
{
public: 
    
    enum GUN_TYPES
    {
        INVALID_GUN_TYPE = -1,
        MP3,
        GUN_TYPES_END
    };

                        brain_spec_ops( npc* newOwner );
    virtual             ~brain_spec_ops();

    virtual void        Init                ( void );
    virtual void        OnPain              ( const pain& Pain );
			void		OnNonAnimPain		( const pain& Pain , const vector3& Pos );

    virtual xbool       FireWeaponAt        ( guid thisTarget, s32 thisWeapon = 0 );
    virtual void        OnAdvanceLogic      ( f32 deltaTime );
    
protected:
    
    GUN_TYPES           GetGunType()            { return MP3; }
    void                PositionMuzzleFlash();
    
    base_loco::anim_type    m_LastAnimSet;
    guid                    m_MuzzleFlashGuid;
    s32                     m_MuzzleBone;
    xbool                   m_BoneIint;

};



#endif//BRAIN_SPEC_OPS__HPP 