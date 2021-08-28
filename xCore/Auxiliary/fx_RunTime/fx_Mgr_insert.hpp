//==============================================================================
//
//  fx_Mgr_insert.hpp
//
//==============================================================================

//==============================================================================
//  class fx_mgr
//  {
//==============================================================================

//------------------------------------------------------------------------------
protected:

        xbool       InitEffect      ( s32& Index, const char* pName );
        void        KillEffect      ( s32  Index );
        void        CreateEffect    ( s32  Index, fx_def* pEffectDef );
        
        void        BindHandle      (       fx_handle& Handle, s32 Index );
        void        UnbindHandle    (       fx_handle& Handle );
                                            
        void        RestartEffect   (       fx_handle& Handle );
        void        AdvanceLogic    (       fx_handle& Handle, f32 DeltaTime );
        void        Render          ( const fx_handle& Handle );

        xbool       IsFinished      ( const fx_handle& Handle );
        xbool       IsInstanced     ( const fx_handle& Handle );
        xbool       Validate        ( const fx_handle& Handle );

        void        SetScale        (       fx_handle& Handle, const vector3& Scale       );
        void        SetRotation     (       fx_handle& Handle, const radian3& Rotation    );
        void        SetTranslation  (       fx_handle& Handle, const vector3& Translation );
        void        SetColor        (       fx_handle& Handle, const xcolor&  Color       );
        xcolor      GetColor        (       fx_handle& Handle                             );
                                            
        void        SetSuspended    (       fx_handle& Handle, xbool Suspended );

const   bbox&       GetBounds       ( const fx_handle& Handle );

//------------------------------------------------------------------------------
protected:

        fx_effect_base*         m_pEffect   [ MAX_EFFECT_INSTANCES   ];
        fx_def*                 m_pEffectDef[ MAX_EFFECT_DEFINITIONS ];

static  fx_load_bitmap_fn*      m_pLoadBitmapFn;
static  fx_unload_bitmap_fn*    m_pUnloadBitmapFn;
static  fx_resolve_bitmap_fn*   m_pResolveBitmapFn;

static  s32                     m_NCtrlTypes;
static  fx_ctrl_type            m_CtrlType[ MAX_CTRL_TYPES ];
                                
static  s32                     m_NElementTypes;
static  fx_element_type         m_ElementType[ MAX_ELEMENT_TYPES ];

static  s32                     m_SpriteBudget;
static  s32                     m_SpritesThisFrame;

friend  fx_handle;
friend  fx_effect;
friend  fx_effect_base;
friend  fx_ctrl_reg;
friend  fx_element_reg;

//------------------------------------------------------------------------------
public:
        
static  void AddSpritesThisFrame( s32 Count )  { m_SpritesThisFrame += Count;  }
static  s32  GetSpritesThisFrame( void )       { return( m_SpritesThisFrame ); }
static  s32  GetSpriteBudget    ( void )       { return( m_SpriteBudget     ); }

//==============================================================================
//  };
//==============================================================================
