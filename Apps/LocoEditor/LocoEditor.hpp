#ifndef LOCOMOTION_EDITOR_HPPP
#define LOCOMOTION_EDITOR_HPPP

#include "../EDRscDesc\RSCDesc.hpp"
#include "MeshUtil\RawMesh.hpp"
#include "MeshUtil\RawAnim.hpp"

//=========================================================================
// ANIMATION DESC.
//=========================================================================
struct animation_desc : public rsc_desc
{
    CREATE_RTTI( animation_desc, rsc_desc, rsc_desc )

                        animation_desc              ( void );
    virtual void        OnEnumProp                  ( prop_enum&  List         );
    virtual xbool       OnProperty                  ( prop_query& I            );
    virtual void        OnCheckIntegrity            ( void );
    virtual void        OnGetCompilerDependencies   ( xarray<xstring>& List    );
    virtual void        OnGetCompilerRules          ( xstring& CompilerRules   );
    virtual void        OnGetFinalDependencies      ( xarray<xstring>& List, platform Platform, const char* pDirectory  );
    virtual void        OnLoad                      ( text_in& TextIn );

    struct anim_info
    {
        anim_info( void )
        {
            SetupDefaults();
        }

        void SetupDefaults( void )
        {
            // Setup defaults
            Name[0]             =
            FileName[0]         = 0 ; 
            Angle               = R_0; 
            FPS                 = 30; 
            Downsample          = 1.0f;
            bLoop               = FALSE ;
            iLoopFrame          = 0;
            EndFrameOffset      = 0;
            bAccumHorizMotion   = TRUE ;
            bAccumVertMotion    = FALSE ;
            bAccumYawMotion     = FALSE ;
            bGravity            = TRUE ;
            bWorldCollision     = TRUE ;
            Weight              = 1.0f ;
            BlendTime           = -1.0f;
            ChainAnim[0]        = 0;
            iChainFrame         = -1;
            ChainCyclesMin      = 1.0f;
            ChainCyclesMax      = 1.0f;
            bChainCyclesInteger = TRUE;
            bBlendFrames        = TRUE;
            bBlendLoop          = TRUE;
        }

        char    FileName[256];
        char    Name[256];
        radian  Angle;
        s32     FPS;
        f32     Downsample;
        xbool   bLoop;
        s32     iLoopFrame;
        s32     EndFrameOffset;
        xbool   bAccumYawMotion;
        xbool   bAccumHorizMotion;
        xbool   bAccumVertMotion;
        xbool   bGravity;
        xbool   bWorldCollision;
        f32     Weight;
        f32     BlendTime;
        char    ChainAnim[256];
        s32     iChainFrame;
        f32     ChainCyclesMin;
        f32     ChainCyclesMax;
        xbool   bChainCyclesInteger;
        xbool   bBlendFrames;
        xbool   bBlendLoop;
    };

    char                m_BindPose[256];
    xarray<anim_info>   m_lAnimInfo;
    xbool               m_bGenericAnim;
    xbool               m_bDisableWarnings;
};

//=========================================================================
// LOCOMOTION EDITOR
//=========================================================================
class locomotion_ed : public prop_interface
{
public:

                   ~locomotion_ed   ( void );
                    locomotion_ed   ( void );
    virtual void    OnEnumProp      ( prop_enum&  List  );
    virtual xbool   OnProperty      ( prop_query& I            );
            void    BeginEdit       ( animation_desc& AnimDesc );      
            void    EndEdit         ( void );      
            void    Save            ( void );
            void    Load            ( const char* pFileName );
            void    New             ( void );
            xbool   NeedSave        ( void );


protected:

    animation_desc*     m_pDesc;
    rawmesh             m_RawMesh;
    xarray<rawanim>     m_lRawAnim;
};

//=========================================================================
// END
//=========================================================================
#endif