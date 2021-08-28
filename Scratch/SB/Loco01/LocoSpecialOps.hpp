#ifndef LOCO_SPECOPS_HPP
#define LOCO_SPECOPS_HPP

//=========================================================================
// INCLUDE
//=========================================================================
#include "locomotion.hpp"

//=========================================================================
// CLASSES
//=========================================================================
class spec_ops_loco;

//=========================================================================
class spec_ops_state 
{
public:

                            spec_ops_state    ( spec_ops_loco& Loco, locomotion::states State );
    virtual xbool           OnAdvance         ( f32 nSeconds )=0;
    virtual void            OnEnter           ( void )=0;
    virtual xbool           OnExit            ( void )=0;
    virtual void            OnInit            ( void ){};

    spec_ops_loco&              m_Base;
    spec_ops_state*             m_pNext;
    locomotion::states          m_State;
    static spec_ops_state*      s_pHead;
};

//=========================================================================

struct spec_ops_default : public spec_ops_state
{
                        spec_ops_default    ( spec_ops_loco& Loco );
    virtual xbool       OnAdvance           ( f32 nSeconds );
    virtual void        OnEnter             ( void );
    virtual xbool       OnExit              ( void );

    void TurnRight( void );
    void LeftRight( void );


    s32               m_sTurning;
    f32               m_Timer;
};

//=========================================================================

struct spec_ops_run : public spec_ops_state
{
                        spec_ops_run        ( spec_ops_loco& Loco );
    virtual xbool       OnAdvance           ( f32 nSeconds );
    virtual void        OnEnter             ( void );
    virtual xbool       OnExit              ( void );
    virtual void        OnInit              ( void );

    f32             m_Timer;

};

//=========================================================================

class spec_ops_loco : public locomotion
{
public:

                        spec_ops_loco       ( void );
    xbool               Advance             ( f32 nSeconds );
    xbool               SetState            ( locomotion::states State );

    virtual void        OnInit              ( void );
    virtual void        OnAdvance           ( f32 nSeconds );

    radian              m_DestYaw ;

protected:

    spec_ops_state*     m_pActive;
    spec_ops_default    m_Default;
    spec_ops_run        m_Run;

    friend class spec_ops_state ;
};

//=========================================================================
// END
//=========================================================================
#endif
