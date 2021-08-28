//==============================================================================
//
//  fx_Mgr_private.hpp
//
//==============================================================================

#ifndef FX_MGR_PRIVATE_HPP
#define FX_MGR_PRIVATE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"

//==============================================================================
//  CONTROLLER REGISTRATION
//==============================================================================

typedef void fx_ctrl_ctor_fn( void* pAddress );

struct fx_ctrl_reg
{
public:
    fx_ctrl_reg     ( const char*       pName, 
                      fx_ctrl_ctor_fn*  pFactorFn );
};

//==============================================================================

#define REGISTER_FX_CONTROLLER_CLASS( T, pName )                               \
                                                                               \
inline void fx_Construct_ ## T( void* pAddress )                               \
{                                                                              \
    new( pAddress ) T;                                                         \
}                                                                              \
                                                                               \
fx_ctrl_reg CtrlReg_ ## T( pName, fx_Construct_ ## T );                           

//==============================================================================
//  ELEMENT REGISTRATION
//==============================================================================

typedef void fx_element_ctor_fn( void* pAddress );

struct fx_element_reg
{
public:
    fx_element_reg          ( const char*           pName, 
                              fx_element_ctor_fn*   pFactoryFn,
                              fx_element_memory_fn* pMemoryFn );
};

//==============================================================================

#undef REGISTER_FX_ELEMENT_CLASS

#define REGISTER_FX_ELEMENT_CLASS( T, pName, pMemoryFn )                       \
                                                                               \
inline void fx_Construct_ ## T( void* pAddress )                               \
{                                                                              \
    new( pAddress ) T;                                                         \
}                                                                              \
                                                                               \
fx_element_reg ElementReg_ ## T( pName, fx_Construct_ ## T, pMemoryFn );                           

//==============================================================================
//  TYPES
//==============================================================================

class fx_mgr;
class fx_handle;
class fx_effect_clone;

//==============================================================================

enum fx_loop
{
    FX_CLAMP,
    FX_TILE,
    FX_MIRROR,
//  FX_CUMULATIVE,
};

//==============================================================================

enum fx_flags
{   
    FX_SINGLETON        = (1 <<  0),

    FX_DEFERRED_DELETE  = (1 <<  1),

    FX_MASTER_LOGIC     = (1 << 30),
    FX_MASTER_COPY      = (1 << 31),
};

//==============================================================================

struct fx_ctrl_type
{
//  xstring                 Name;
    char                    Name[32];
    fx_ctrl_ctor_fn*        pFactoryFn;
};

//==============================================================================

struct fx_element_type
{
//  xstring                 Name;
    char                    Name[32];
    fx_element_ctor_fn*     pFactoryFn;
    fx_element_memory_fn*   pMemoryFn;   // How much memory does element need?
};

//==============================================================================

struct fx_ctrl_def
{   
    s32         TotalSize;      // In 32-bit values.
    s32         TypeIndex;      //
    s32         NOutputValues;  // How many output channels of data?
    fx_loop     LeadIn;         // How to evaluate before DataBegin?
    fx_loop     LeadOut;        // How to evaluate after  DataEnd?
    f32         DataBegin;      // Within effect.
    f32         DataEnd;        // Within effect.
    s32         OutputIndex;    // Where in StagingArea to write output.
};

//==============================================================================

struct fx_element_def
{
    s32         TotalSize;          // In 32-bit values.
    s32         TypeIndex;          //
    xbool       ReadZ;              //
    s32         CombineMode;        // -1:Subtract  0:Multiple  +1:Add
    s32         CtrlOffsets[13];    // For SRT/C values from Staging Area.
    vector3p    Scale;              // Constant values for S.
    radian3     Rotate;             // Constant values for R.
    vector3p    Translate;          // Constant values for T.
    f32         Color[4];           // Constant values for C (as floats).
    f32         TimeStart;          // Within effect.
    f32         TimeStop;           // Within effect.
    // Replications?  And ranges for constant values?
};

//==============================================================================

struct fx_def
{
        s32                 TotalSize;      // In 32-bit values.  (Excludes names.)
mutable u32                 Flags;          // See fx_flags.
        s32                 NSAValues;      // Staging area values.
        s32                 NControllers;
        s32                 NElements;
        s32                 NBitmaps;
        s32                 MasterCopy;
mutable s32                 NInstances;     // Number of effects of this def.
        char*               pEffectName;
        fx_ctrl_def**       pCtrlDef;
        fx_element_def**    pElementDef;
        xhandle*            pDiffuseMap;
        xhandle*            pAlphaMap;
};

//==============================================================================
//  class fx_effect_base
//==============================================================================

class fx_effect_base
{
//------------------------------------------------------------------------------

public:

        void        Initialize      ( const fx_def* pEffectDef );

const   matrix4&    GetL2W          ( void ) const;
const   vector3&    GetScale        ( void ) const;
const   radian3&    GetRotation     ( void ) const;
const   vector3&    GetTranslation  ( void ) const;
        xcolor      GetColor        ( void ) const;
        f32         GetUniformScale ( void ) const;

        void        SetScale        ( const vector3& Scale       );
        void        SetRotation     ( const radian3& Rotation    );
        void        SetTranslation  ( const vector3& Translation );
        void        SetColor        ( const xcolor   Color       );        

        void        AddReference    ( void );
        void        RemoveReference ( void );
        s32         GetReferences   ( void );
        fx_element**GetElementList  ( void );

        void        Render          ( void );

        void        GetBitmaps      (       s32       Index,
                                      const xbitmap*& pDiffuseMap,
                                      const xbitmap*& pAlphaMap ) const;

//------------------------------------------------------------------------------

virtual f32         GetAge          ( void ) const = 0;
virtual const bbox& GetBounds       ( void ) const = 0;
virtual xbool       IsSuspended     ( void ) const = 0;
virtual xbool       IsFinished      ( void ) const = 0;
virtual xbool       IsInstanced     ( void ) const = 0;
virtual void        Restart         ( void ) = 0; 
virtual void        SetSuspended    (       xbool     Suspended ) = 0;
virtual void        AdvanceLogic    (       f32       DeltaTime ) = 0;

//------------------------------------------------------------------------------

protected:

        vector3             m_Scale;
        radian3             m_Rotation;
        vector3             m_Translation;

        xcolor              m_Color;
        xbool               m_ColorDirty;
                            
mutable matrix4             m_L2W;
mutable xbool               m_L2WDirty;
mutable xbool               m_EL2WDirty;
                            
mutable bbox                m_BBox;
mutable xbool               m_BBoxDirty;

        u32                 m_Flags;
        s32                 m_NReferences;
        fx_element**        m_pElement;
        const fx_def*       m_pEffectDef;

friend fx_mgr;
friend fx_effect_clone;
};

//==============================================================================
//  class fx_effect
//==============================================================================

class fx_effect : public fx_effect_base
{
//------------------------------------------------------------------------------

public:

        void        Initialize      ( const fx_def* pEffectDef );
static  void        ForceConstruct  ( void* pAddress );

//------------------------------------------------------------------------------

virtual f32         GetAge          ( void ) const;
virtual const bbox& GetBounds       ( void ) const;
virtual xbool       IsSuspended     ( void ) const;
virtual xbool       IsFinished      ( void ) const;
virtual xbool       IsInstanced     ( void ) const;
virtual void        Restart         ( void ); 
virtual void        SetSuspended    (       xbool     Suspended );
virtual void        AdvanceLogic    (       f32       DeltaTime );

//------------------------------------------------------------------------------

protected:

        f32                 m_Age;
        xbool               m_Done;
        xbool               m_Suspended;
                            
        f32*                m_pStagingArea;
        fx_ctrl**           m_pCtrl;
};

//==============================================================================
//  class fx_effect_clone
//==============================================================================

class fx_effect_clone : public fx_effect_base
{
public:
        void        Initialize      (       fx_effect_base* pMasterEffect, 
                                      const fx_def*         pEffectDef );
static  void        ForceConstruct  ( void* pAddress );

//------------------------------------------------------------------------------

virtual f32         GetAge          ( void ) const;
virtual const bbox& GetBounds       ( void ) const;
virtual xbool       IsSuspended     ( void ) const;
virtual xbool       IsFinished      ( void ) const;
virtual xbool       IsInstanced     ( void ) const;
virtual void        Restart         ( void ); 
virtual void        SetSuspended    (       xbool     Suspended );
virtual void        AdvanceLogic    (       f32       DeltaTime );

//------------------------------------------------------------------------------

protected:
        fx_effect_base*  m_pEffect;
};

//==============================================================================
//  Inlines for class fx_effect_base
//==============================================================================

inline
const matrix4& fx_effect_base::GetL2W( void ) const
{
    if( m_L2WDirty )
    {
        m_L2W.Setup( m_Scale, m_Rotation, m_Translation );
        m_L2WDirty = FALSE;
    }
    return( m_L2W );
}

//==============================================================================

inline
f32 fx_effect_base::GetUniformScale( void ) const
{
    return( (m_Scale.GetX() + m_Scale.GetY() + m_Scale.GetZ()) / 3.0f );
}

//==============================================================================

inline
const vector3& fx_effect_base::GetScale( void ) const
{
    return( m_Scale );
}

//==============================================================================

inline
const radian3& fx_effect_base::GetRotation( void ) const
{
    return( m_Rotation );
}

//==============================================================================

inline
const vector3& fx_effect_base::GetTranslation( void ) const
{
    return( m_Translation );
}

//==============================================================================

inline
xcolor fx_effect_base::GetColor( void ) const
{
    return( m_Color );
}

//==============================================================================

inline
void fx_effect_base::SetScale( const vector3& Scale )
{
    ASSERT( Scale.IsValid() );
    m_Scale     = Scale;
    m_L2WDirty  = TRUE;
    m_EL2WDirty = TRUE;
}

//==============================================================================

inline
void fx_effect_base::SetRotation( const radian3& Rotation )
{
    ASSERT( Rotation.IsValid() );
    m_Rotation  = Rotation;
    m_L2WDirty  = TRUE;
    m_EL2WDirty = TRUE;
}

//==============================================================================

inline
void fx_effect_base::SetTranslation( const vector3& Translation )
{
    ASSERT( Translation.IsValid() );
    m_Translation = Translation;
    m_L2WDirty    = TRUE;
    m_EL2WDirty   = TRUE;
}

//==============================================================================

inline
void fx_effect_base::SetColor( const xcolor Color )
{
    m_Color      = Color;
    m_ColorDirty = TRUE;
}

//==============================================================================

inline
void fx_effect_base::AddReference( void )
{
    m_NReferences++;
}

//==============================================================================

inline
void fx_effect_base::RemoveReference( void )
{
    m_NReferences--;
}

//==============================================================================

inline
s32 fx_effect_base::GetReferences( void )
{
    return( m_NReferences );
}

//==============================================================================

inline
fx_element** fx_effect_base::GetElementList( void )
{
    return( m_pElement );
}

//==============================================================================
#endif // FX_MGR_PRIVATE_HPP
//==============================================================================
