#ifndef __BASE_H
#define __BASE_H

namespace fx_core
{

//============================================================================
// base
//============================================================================

class base
{

protected:
    char*               m_pType;


public:
    enum prop_type
    {
        PROP_BOOL,
        PROP_INT,
        PROP_FLOAT,
        PROP_STRING,
        PROP_V3,
        PROP_R3,
        PROP_COLOR,
        PROP_FILENAME,
        PROP_HEADER,
        PROP_LOOPMODE,
        PROP_CONTROLLERTYPE,
        PROP_COMBINEMODE
    };

    enum controller_type
    {
        CONTROLLERTYPE_LINEAR,
        CONTROLLERTYPE_SMOOTH,
        CONTROLLERTYPE_SIZEOF
    };

    enum combine_mode
    {
        COMBINEMODE_ALPHA       =  0,
        COMBINEMODE_ADDITIVE    =  1,
        COMBINEMODE_SUBTRACTIVE =  2,

        COMBINEMODE_GLOW_ALPHA  =  3,
        COMBINEMODE_GLOW_ADD    =  4,
        COMBINEMODE_GLOW_SUB    =  5,

        COMBINEMODE_DISTORT     =  6,
    };

public:
                        base                        ();
                       ~base                        ();

    virtual void        Render                      ( f32 T ) = 0;

    const char*         GetType                     ( void )        { return m_pType; }

    static s32          ControllerType_FromString   ( const char* pString );
    static const char*  ControllerType_ToString     ( s32 LoopType );

    static s32          CombineMode_FromString      ( const char* pString );
    static const char*  CombineMode_ToString        ( s32 LoopType );
};

//============================================================================

} // namespace fx_core

#endif
