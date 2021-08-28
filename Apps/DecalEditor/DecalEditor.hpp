#ifndef DECAL_EDITOR_HPPP
#define DECAL_EDITOR_HPPP

#include "..\EDRscDesc\RSCDesc.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"

//=========================================================================
// DECAL DESC.
//=========================================================================
struct decal_pkg_desc : public rsc_desc
{
    CREATE_RTTI( decal_pkg_desc, rsc_desc, rsc_desc )

                        decal_pkg_desc           ( void );
    virtual void        OnEnumProp               ( prop_enum&  List         );
    virtual xbool       OnProperty               ( prop_query& I            );
    virtual void        OnCheckIntegrity         ( void );
    virtual void        OnGetCompilerDependencies( xarray<xstring>& List    );
    virtual void        OnGetCompilerRules       ( xstring& CompilerRules   );
    virtual void        OnGetFinalDependencies   ( xarray<xstring>& List, platform Platform, const char* pDirectory );

    struct decal_desc
    {
        decal_desc( void );
        void Reset( void );

        char    Name[32];
        vector2 MinSize;
        vector2 MaxSize;
        radian  MinRoll;
        radian  MaxRoll;
        xcolor  Color;
        char    BitmapName[256];
        char    PreferredFormat[256];
        s32     MaxVisible;
        f32     FadeTime;
        u32     Flags;
        char    BlendMode[256];
    };

    struct group_desc
    {
        group_desc( void );
        
        char                Name[32];
        xcolor              Color;
        s32                 iDecal;
        s32                 nDecals;
    };

protected:
    void                EnumGroups          ( prop_enum&  List    );
    void                EnumDecals          ( prop_enum&  List,
                                              s32         iGroup  );
    xbool               OnPropertyGroups    ( prop_query& I       );
    xbool               HandleFlag          ( prop_query& I,
                                              const char* PropName,
                                              u32&        PropFlags,
                                              u32         FlagBit );
    xbool               OnPropertyDecals    ( prop_query& I       );
    void                AddGroup            ( void                );
    void                RemoveGroup         ( s32         Index   );
    void                AddDecal            ( s32         iGroup  );
    void                GrowDecalList       ( s32         NewSize );
    void                RemoveDecal         ( s32         iGroup,
                                              s32         iDecal  );

    xarray<group_desc>  m_Groups;
    xarray<decal_desc>  m_Decals;
};

//=========================================================================
// END
//=========================================================================
#endif