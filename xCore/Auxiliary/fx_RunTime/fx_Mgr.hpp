//==============================================================================
//
//  fx_Mgr.hpp
//
//==============================================================================

#ifndef FX_MGR_HPP
#define FX_MGR_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_math.hpp"
#include "x_color.hpp"
#include "x_bitmap.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

#define MAX_CTRL_TYPES           32
#define MAX_ELEMENT_TYPES        32

#define MAX_EFFECT_DEFINITIONS  256
#define MAX_EFFECT_INSTANCES    512

#if !defined(X_RETAIL) || defined(X_QA)
    #define DEBUG_FX
#else
    #undef DEBUG_FX
#endif

//==============================================================================
//  TYPE ANNOUCEMENTS
//==============================================================================

class  fx_mgr;
struct fx_def;
class  fx_effect_base;
struct fx_ctrl_def;
struct fx_element_def;

//==============================================================================
//  EFFECTS SYSTEM DEBUG OPTIONS
//==============================================================================

#ifdef DEBUG_FX
struct fx_debug
{
    xbool   EffectAxis;
    xbool   EffectCenter;
    xbool   EffectVolume;
    xbool   EffectBounds;
    xbool   EffectReserved;   // RESERVED

    xbool   ElementAxis;
    xbool   ElementCenter;
    xbool   ElementVolume;
    xbool   ElementBounds;
    xbool   ElementWire;
    xbool   ElementSpriteCenter;
    xbool   ElementSpriteCount;
    xbool   ElementCustom;
    xbool   ElementReserved;  // RESERVED
};

extern fx_debug FXDebug;
#endif // DEBUG_FX

//==============================================================================
//  CLASS FX_CTRL
//==============================================================================
//  
//  In general, you only need to look at the fx_controller class if you are
//  creating a custom controller type.
//  
//  Controllers have only one "relevant" function: Evaluate().
//
//==============================================================================

class fx_ctrl
{
//------------------------------------------------------------------------------
protected:

const   fx_ctrl_def*    m_pCtrlDef;
        f32             m_AgeRate;      // Rate DeltaTime advances a cycle.
        s32             m_Cycle;        // Which cycle are we on?
        f32             m_CycleAge;     // Where in current cycle (0.0 - 1.0)?
        f32*            m_pOutput;

//------------------------------------------------------------------------------
public:
        void            Initialize          ( fx_ctrl_def* pCtrlDef, 
                                              f32*         pOutput );
        void            AdvanceLogic        ( f32          DeltaTime );
        f32             ComputeLogicalTime  ( void ) const;

//------------------------------------------------------------------------------
public: // THIS FUNCTION IS AVAILABLE TO BE REDEFINED BY DESCENDANT CLASSES

virtual void            Evaluate            ( f32 LogicalTime ) const = 0;
};

//==============================================================================
//  CLASS FX_ELEMENT
//==============================================================================
//
//  In general, you only need to look at the fx_element class if you are
//  creating a custom element type.
//
//  Relevant functions, all virtual:
//
//      void    Initialize()    - Sets up all internal values.
//      void    AdvanceLogic()  - Advances logic by given delta seconds.
//      void    Render()        - Renders the element.
//      xbool   IsFinished()    - Reports if element has completed its logic.
//      void    Reset()         - Resets element for reuse after its Finished.
//
//==============================================================================

class fx_element
{
friend  fx_effect_base;
//------------------------------------------------------------------------------
protected:  
mutable matrix4         m_Local;            // Local matrix from SRT of element.
mutable matrix4         m_L2W;              // L2W of element.
mutable xbool           m_LocalDirty;       // Need to recompute L2W because local part is dirty?
mutable xbool           m_BaseDirty;        // Need to recompute L2W because base part if dirty?

const   fx_element_def* m_pElementDef;
const   f32*            m_pInput;

        vector3p        m_Scale;            // From here...
        radian3         m_Rotate;           //       ...  
        vector3p        m_Translate;        //    ...
        f32             m_FloatColor[4];    // ... to here MUST BE ADJECENT.

        xcolor          m_BaseColor;        // Color of parent effect.
        xcolor          m_Color;            // Color of element.
        xbool           m_ColorDirty;       // Need to reapply colors?

        bbox            m_BBox;

//------------------------------------------------------------------------------
public:

        void            BaseL2W         ( void );
        void            BaseLogic       ( void );
        void            BaseColor       ( const xcolor Color );
        void            BaseColor       ( void );
const   matrix4&        GetL2W          ( const fx_effect_base* pEffect ) const;
        f32             GetUniformScale ( void ) const;
const   bbox&           GetBBox         ( void ) const;

//------------------------------------------------------------------------------
public: // THESE FUNCTIONS ARE AVAILABLE TO BE REDEFINED BY DESCENDANT CLASSES

virtual void            Initialize      ( const fx_element_def* pElementDef, 
                                                f32*            pInput );

virtual void            AdvanceLogic    ( const fx_effect_base* pEffect, 
                                                f32             DeltaTime );

virtual void            Render          ( const fx_effect_base* pEffect ) const;

virtual xbool           IsFinished      ( const fx_effect_base* pEffect ) const;

virtual void            Reset           ( void );

virtual void            ApplyColor      ( void );
};

//==============================================================================
//  ELEMENT REGISTRATION MACROS
//==============================================================================
//
//  The following macros is used to register elements.
//  
//  You only need to bother with this macro if you are going to do a new or 
//  customized element.
//
//  If you create a new fx_element descendant class named "my_element", the 
//  you can place the following:
//
//      REGISTER_FX_ELEMENT_CLASS( my_element, "MyElement", MyElementMemoryFn );
//
//  withing the "my_element.cpp" file (outside of any function).  Note that
//  the string "MyElement" would be the value given in the "Custom Type" field
//  in the fx_Editor.  Also note that MyElementMemoryFn must be a function which
//  satisfies the "fx_element_memory_fn" function signature.
//
//==============================================================================

typedef s32   fx_element_memory_fn  ( const fx_element_def& ElementDef );

//------------------------------------------------------------------------------

fx_element_memory_fn    DefaultElementMemoryFn; 

//------------------------------------------------------------------------------

#define REGISTER_FX_ELEMENT_CLASS( T, pName, pMemoryFn )   //** Details hidden.

//==============================================================================
//  HOOKS FOR LOADING AND UNLOADING BITMAPS
//==============================================================================

typedef xbool          fx_load_bitmap_fn     ( const char*           pBitmapName,
                                                     xhandle&        Handle );
typedef void           fx_unload_bitmap_fn   (       xhandle         Handle );
typedef const xbitmap* fx_resolve_bitmap_fn  (       xhandle         Handle );

//==============================================================================
//  CLASS FX_HANDLE 
//==============================================================================

class fx_handle
{

//------------------------------------------------------------------------------
public:

                    fx_handle       ( void );
                    fx_handle       ( const fx_handle& Handle );
                   ~fx_handle       ( void );

        xbool       InitInstance    ( const char* pName );
        void        KillInstance    ( void );

        void        AdvanceLogic    ( f32 DeltaTime );
        void        Restart         ( void );

        void        Render          ( void ) const;

        void        SetScale        ( const vector3& Scale       );
        void        SetRotation     ( const radian3& Rotation    );
        void        SetTranslation  ( const vector3& Translation );
        vector3     GetTranslation  ( void ) const;
        void        SetTransform    ( const matrix4& L2W );
        void        SetColor        ( const xcolor&  Color       );
        xcolor      GetColor        ( void );

        void        SetSuspended    ( xbool Suspended );

const   bbox&       GetBounds       ( void ) const;

        xbool       Validate        ( void ) const;
        xbool       IsFinished      ( void ) const;
        xbool       IsInstanced     ( void ) const;

const   fx_handle&  operator =      ( const fx_handle& Handle );   

//------------------------------------------------------------------------------
protected:

mutable s32         Index;
friend  fx_mgr;

};

//==============================================================================
//  PRIVATE STUFF
//==============================================================================

#include "fx_Mgr_private.hpp"

//==============================================================================
//  CLASS FX_MGR
//==============================================================================

class fx_mgr
{

//------------------------------------------------------------------------------
public:
                    fx_mgr          ( void );
                   ~fx_mgr          ( void );       
                             
        void        SetBitmapFns    ( fx_load_bitmap_fn*    pLoadFn,
                                      fx_unload_bitmap_fn*  pUnloadFn,
                                      fx_resolve_bitmap_fn* pResolveFn );

        xbool       LoadEffect      ( const char* pEffectName,     X_FILE* pFile     );
        xbool       LoadEffect      ( const char* pEffectName, const char* pFileName );
        xbool       UnloadEffect    ( const char* pEffectName );

        void        SetSpriteBudget ( s32 MaxSprites );
        s32         GetSpriteCount  ( void );
        vector3     GetTranslation  ( const fx_handle& Handle );       
        void        EndOfFrame      ( void );

//------------------------------------------------------------------------------

#include "fx_Mgr_insert.hpp"

};

//==============================================================================
//  STORAGE ANNOUNCEMENTS
//==============================================================================

extern fx_mgr FXMgr;

//==============================================================================
// Inlines
//==============================================================================

inline
f32 fx_element::GetUniformScale( void ) const
{
    return( (m_Scale.X + m_Scale.Y + m_Scale.Z) / 3.0f );
}

//==============================================================================

inline
const bbox& fx_element::GetBBox( void ) const
{
    return( m_BBox );
}

//==============================================================================

inline
void fx_element::BaseL2W( void )
{
    m_BaseDirty = TRUE;
}

//==============================================================================

inline
void fx_element::BaseColor( const xcolor Color )
{
    m_BaseColor  = Color;
    m_ColorDirty = TRUE;
}

//==============================================================================

inline
void fx_element::BaseColor( void )
{
    if( m_ColorDirty )
    {
        ApplyColor();
        m_ColorDirty = FALSE;
    }
}

//==============================================================================

inline
const matrix4& fx_element::GetL2W( const fx_effect_base* pEffect ) const
{
    if( m_LocalDirty )
        m_Local.Setup( m_Scale, m_Rotate, m_Translate );

    if( m_BaseDirty || m_LocalDirty )
        m_L2W = pEffect->GetL2W() * m_Local;

    m_LocalDirty = FALSE;
    m_BaseDirty  = FALSE;

    return( m_L2W );
}

//==============================================================================

//==============================================================================
#endif // FX_MGR_HPP
//==============================================================================
