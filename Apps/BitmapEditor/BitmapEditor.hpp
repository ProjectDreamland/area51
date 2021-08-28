#ifndef BitmapMOTION_EDITOR_HPPP
#define BitmapMOTION_EDITOR_HPPP

#include "../EDRscDesc\RSCDesc.hpp"
#include "MeshUtil\RawMesh.hpp"
#include "MeshUtil\RawAnim.hpp"

//=========================================================================
// BITMAP DESC.
//=========================================================================
struct bitmap_desc : public rsc_desc
{
    CREATE_RTTI( bitmap_desc, rsc_desc, rsc_desc )

                        bitmap_desc              ( void );
    virtual void        OnEnumProp               ( prop_enum&  List         );
    virtual xbool       OnProperty               ( prop_query& I            );
    virtual void        OnCheckIntegrity         ( void );
    virtual void        OnGetCompilerDependencies( xarray<xstring>& List    );
    virtual void        OnGetCompilerRules       ( xstring& CompilerRules   );
    virtual void        OnGetFinalDependencies   ( xarray<xstring>& List, platform Platform, const char* pDirectory ){}

    char                m_FileName[256];
    char                m_PreferFormat[256];
    char                m_OptLevel    [256];
};

//=========================================================================
// ENVIROMENT MAP DESC.
//=========================================================================
struct envmap_desc : public rsc_desc
{
    CREATE_RTTI( envmap_desc, rsc_desc, rsc_desc )

                        envmap_desc              ( void );
    virtual void        OnEnumProp               ( prop_enum&  List         );
    virtual xbool       OnProperty               ( prop_query& I            );
    virtual void        OnCheckIntegrity         ( void );
    virtual void        OnGetCompilerDependencies( xarray<xstring>& List    );
    virtual void        OnGetCompilerRules       ( xstring& CompilerRules   );
    virtual void        OnGetFinalDependencies   ( xarray<xstring>& List, platform Platform, const char* pDirectory ){}

    char                m_FileName[6][256];
    char                m_PreferFormat[256];
    char                m_OptLevel    [256];
};


//=========================================================================
// BitmapMOTION EDITOR
//=========================================================================
class Bitmapmotion_ed : public prop_interface
{
public:

                   ~Bitmapmotion_ed   ( void );
                    Bitmapmotion_ed   ( void );
    virtual void    OnEnumProp        ( prop_enum&  List  );
    virtual xbool   OnProperty        ( prop_query& I            );
            void    BeginEdit         ( bitmap_desc&    BitmapDesc );      
            void    BeginEdit         ( envmap_desc&    EnvMapDesc );      
            void    EndEdit           ( void );      
            void    Save              ( void );
            void    Load              ( const char* pFileName );
            void    NewBitmap         ( void );
            void    NewEnvmap         ( void );

            xbool   NeedSave          ( void );

protected:

    bitmap_desc*        m_pBitmapDesc;
    envmap_desc*        m_pEnvmapDesc;
    xbitmap             m_Bitmap[8];
};

//=========================================================================
// END
//=========================================================================
#endif