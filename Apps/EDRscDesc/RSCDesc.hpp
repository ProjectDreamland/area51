#ifndef RESOURCE_DESCRIPTORS_HPP
#define RESOURCE_DESCRIPTORS_HPP

#include <time.h>

#include "x_files.hpp"
#include "Auxiliary\MiscUtils\Property.hpp"
#include "Auxiliary\MiscUtils\rtti.hpp"
#include "Parsing\TextOut.hpp"
#include "Parsing\TextIn.hpp"

class rsc_desc_type;
struct _finddata_t;

//=========================================================================
// To declare a type use this macro.
//=========================================================================
#define DEFINE_RSC_TYPE( VARNAME, TYPE, TYPE_NAME, FILE_EXT, FILE_DESC )

//=========================================================================
// rsc_desc_editor
//=========================================================================

class rsc_desc_editor : public prop_interface
{
        
};

//=========================================================================
// rsc_desc
//=========================================================================
//
// OnGetCompilerDependencies- This are the dependencies that this resource needs in order to be compile.
//                            Note that additional dependecies can be given by the compiler it self.
//
// OnGetFinalDependencies   - Ones the resource is compile this function will return which other
//                            compiled resources this particular resource needs. (Such a geom may need a texture).
//                            note that before this resource can answer this question it first need to be compile.
//
//=========================================================================
class rsc_desc : public prop_interface
{
public:    
        CREATE_RTTI_BASE( rsc_desc );

        const char*         GetName                  ( void ) const;
        const char*         GetType                  ( void ) const;
        const char*         GetPath                  ( void ) const;
        void                SetFullName              ( const char* pRscName );
        void                GetFullName              ( char* pRscName ) const;
        xbool               IsBeingEdited            ( void ) const;
        void                SetBeingEdited           ( xbool bBeingEdited );
        xbool               IsChanged                ( void ) const;
        void                SetChanged               ( xbool bChanged );
        u32                 GetFullNameHash          ( void ) { return m_FullNameHash; }
        u32                 GetNameHash              ( void ) { return m_NameHash; }
static  xbool               IsVerbose                ( void ) { return s_bVerbose; }
static  void                SetVerbose               ( xbool bVerbose ) { s_bVerbose = bVerbose; }
    
        virtual void        OnStartEdit              ( void );
#if defined( X_EDITOR )
        virtual void        OnSave                   ( text_out&                     TextOut    );
#endif
        virtual void        OnLoad                   ( text_in&                      TextIn     );
        virtual void        OnEnumProp               ( prop_enum&        List             );
        virtual xbool       OnProperty               ( prop_query&       I                );
        virtual void        OnGetCompilerDependencies( xarray<xstring>&  List             ) = 0;
        virtual void        OnGetCompilerRules       ( xstring&          CompilerRules    ) = 0;
        virtual void        OnCheckIntegrity         ( void )                               = 0;
        virtual void        OnGetFinalDependencies   ( xarray<xstring>& List, platform Platform, const char* pDirectory ) = 0;

protected:

                        rsc_desc( rsc_desc_type& Desc );

    rsc_desc_type&   m_Type;

private:

        char             m_Name[256];               
        char             m_Path[256];
        u32              m_NameHash;
        u32              m_FullNameHash;
        xbool            m_bBeingEdited;
        xbool            m_bChange;
static  xbool            s_bVerbose;
    
};

//=========================================================================
// Manager
//=========================================================================
class rsc_desc_mgr
{
public:

    enum flags
    {
        FLAGS_EXTERNAL   = (1<<0),  // Tells the system that the resource is external
        FLAGS_SKIP       = (1<<1),  // Tells the system not to compile a certain resource
        FLAGS_RELOAD     = (1<<2)   // Tells the system to relad the resource from the resource manager.
    };

    struct node
    {
        rsc_desc*       pDesc;
        time_t          TimeStamp;
        u32             Flags;
        xarray<xhandle> Dependency;
        s32             nExternals;
        xhandle         External[32];
        s32             RefCount;
        xstring         Theme;
    };

    struct dependency
    {
        s32     RefCount;
        u32     Flags;
        char    Name[256];
        u32     NameHash;
        time_t  TimeStamp;

        void SetName( const char* pName );
        const char* GetName( void ) { return Name; }
    };

public:

                          rsc_desc_mgr              ( void );
    const rsc_desc_type*  GetFirstType              ( void ) const;
    const rsc_desc_type*  GetNextType               ( const rsc_desc_type* PrevType ) const;
    const rsc_desc_type&  GetType                   ( const char* pType ) const;

    rsc_desc&             GetRscDescByString        ( const char* pName );

    rsc_desc&             CreateRscDesc             ( const char* pName );
    void                  DeleteRscDesc             ( const char* pName );
    void                  DeleteRscDesc             ( xhandle hRsc );
    void                  CleanResource             ( const rsc_desc& RscDesc );

    rsc_desc&             Load                      ( const char* pFileName );
#if defined( X_EDITOR )
    void                  Save                      ( rsc_desc& Desc );
#endif

    s32                   GetRscDescCount           ( void );
    node&                 GetRscDescIndex           ( s32 Index );

    s32                   GetDepCount               ( void );
    dependency&           GetDepIndex               ( s32 Index );

    void                  RefreshDesc               ( void );
    node*                 GetRscDesc                ( const char* pName );
    void                  DeleteRscDescFromDrive    ( const char* pName );

    xbool                 BeginCompiling            ( u32 Platform );
    void                  AddExternalRsc            ( const char* pExternalInfo, s32 Index );
    s32                   NextCompiling             ( void );
    xbool                 IsCompiling               ( void );
    xbool                 IsStopBuild               ( void );
    void                  StopBuild                 ( void );
    void                  GetMakeRules              ( s32 Index, xstring& MakeRules, xstring& Output );
    void                  EndCompiling              ( void );
    void                  ScanResources             ( void );

protected:

    enum
    {
        PLATFORM_COUNT   = 4,
    };

    struct platform_info
    {
        char        OutputDir[256];
        char        Cmd[32];
        xbool       bCompile;
        platform    Platform;
    };


protected:
    
    void                  RefreshAllCompiledRsc     ( void );
    xhandle               FindDep                   ( const char* pFileName );
    xhandle               AddDep                    ( const char* pFileName );
    void                  DelDep                    ( const char* pFileName );
    void                  DelDep                    ( xhandle Handle );
    s32                   FindRscDesc               ( const char* pName );
    xbool                 NeedCompiling             ( node& RscDesc );
    void                  ParseExternalRsc          ( const char* pExternalInfo, char* pRscName, xstring& CompilerRules );
    void                  ClearRscDesc              ( void );

    void                  ScanForRscDesc            ( void );
    void                  ScanForRscDesc            ( const char* DirName, long& hFile, _finddata_t& Data, const char* ThemeName );
    
protected:

     xharray<node>          m_lRscDesc;
     xharray<dependency>    m_lSrcDep;
     xbool                  m_IsCompiling;
     s32                    m_CompilingIndex;
     xbool                  m_bStopBuild;

     platform_info          m_Platform     [PLATFORM_COUNT];
     char                   m_CompilerDir  [256];
};

//=========================================================================
// Type declarator
//=========================================================================
// Use Macro to create the types don't use the class it self.
//=========================================================================
#undef  DEFINE_RSC_TYPE
#define DEFINE_RSC_TYPE( VARNAME, TYPE, TYPE_NAME, FILE_EXT, FILE_DESC )                  \
static struct TYPE##_RscDesc##__LINE__ : public rsc_desc_type                             \
{   TYPE##_RscDesc##__LINE__( void ) : rsc_desc_type( TYPE_NAME, FILE_EXT, FILE_DESC ) {} \
    rsc_desc* CreateRscDesc( void ) const { return (rsc_desc*)new TYPE; } }               \
    VARNAME;

class rsc_desc_type
{
public:
                        rsc_desc_type     ( const char* pTypeName, const char* pFileExtension, const char* pTypeDescription );
    const char*         GetName           ( void ) const;
    const char*         GetFileExtensions ( void ) const;

protected:

    // user must probide a declaration for this
    virtual rsc_desc*   CreateRscDesc     ( void ) const = 0;

protected:

    const char*             m_pTypeName;
    const char*             m_pFileExtensions;
    const char*             m_pTypeDescription;
    rsc_desc_type*          m_pNext;
    static rsc_desc_type*   s_pHead;

    friend rsc_desc_mgr;
};

//=========================================================================
// EXTERN THE GLOBAL CLASS
//=========================================================================
extern rsc_desc_mgr g_RescDescMGR;

//=========================================================================
// INLINE FUNCTIONS
//=========================================================================

//=========================================================================
inline
rsc_desc::rsc_desc( rsc_desc_type& Desc ) :
    m_Type( Desc )
{
    m_Name[0]=0;
    m_Path[0]=0;
    m_bBeingEdited = FALSE;
    m_bChange      = FALSE;
}

//=========================================================================
inline
rsc_desc_type::rsc_desc_type( const char* pTypeName, const char* pFileExtension, const char* pTypeDescription ) :
    m_pTypeName         ( pTypeName ),
    m_pFileExtensions   ( pFileExtension ),
    m_pTypeDescription  ( pTypeDescription )
{
    m_pNext = s_pHead;
    s_pHead = this;
}

//=========================================================================
inline 
const char* rsc_desc_type::GetName( void ) const
{
    return m_pTypeName;
}

//=========================================================================
inline 
const char* rsc_desc_type::GetFileExtensions( void ) const
{
    return m_pFileExtensions;
}

//=========================================================================
inline
const char* rsc_desc::GetName( void ) const
{
    return m_Name;
}

//=========================================================================
inline
const char* rsc_desc::GetType( void ) const
{
    return m_Type.GetName();
}

//=========================================================================
inline
xbool rsc_desc::IsBeingEdited( void ) const
{
    return m_bBeingEdited;
}

//=========================================================================
inline
xbool rsc_desc::IsChanged( void ) const
{
    return m_bChange;
}

//=========================================================================
inline
void rsc_desc::SetChanged( xbool bChanged )
{
    m_bChange = bChanged;
}

//=========================================================================
inline
void rsc_desc::SetBeingEdited( xbool bBeenEdited )
{
    m_bBeingEdited = bBeenEdited;
}

//=========================================================================
inline
const char* rsc_desc::GetPath( void ) const
{
    return m_Path;
}

//=========================================================================
inline
s32 rsc_desc_mgr::GetRscDescCount( void )
{
    return m_lRscDesc.GetCount();
}

//=========================================================================
inline
rsc_desc_mgr::node& rsc_desc_mgr::GetRscDescIndex( s32 Index )
{
    ASSERT( m_lRscDesc[Index].pDesc );
    return m_lRscDesc[Index];
}

//=========================================================================
inline
s32 rsc_desc_mgr::GetDepCount( void )
{
    return m_lSrcDep.GetCount();
}

//=========================================================================
inline
rsc_desc_mgr::dependency& rsc_desc_mgr::GetDepIndex( s32 Index )
{
    return m_lSrcDep[Index];
}

//=========================================================================
// END
//=========================================================================
#endif
