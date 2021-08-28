#ifndef RESOURCE_DESCRIPTORS_HPP
#define RESOURCE_DESCRIPTORS_HPP

#include <time.h>
#include "x_files.hpp"
#include "Property.hpp"
#include "rtti.hpp"

class rsc_desc_type;

//=========================================================================
// Base
//=========================================================================
class rsc_desc
{
public:    
    CREATE_RTTI_BASE( rsc_desc );

    virtual             ~rsc_desc           ( void ) {}
    const char*         GetRscName          ( void ) const;
    const char*         GetRscType          ( void ) const;
    void                SetRscName          ( const char* pRscName );

    virtual void        EnumProp            ( xarray<prop_enum>& List ) = 0;
    virtual xbool       Property            ( prop_query& I ) = 0;
    virtual void        GetDependencies     ( xarray<prop_enum>& List    ) = 0;
    virtual void        GetCompilerRules    ( xstring& CompilerRules     ) = 0;

protected:

                        rsc_desc( rsc_desc_type& Desc );

    rsc_desc_type&   m_Type;

private:

    char             m_Name[128];
};

//=========================================================================
// Manager
//=========================================================================
class rsc_desc_mgr
{
public:

    enum flags
    {
        FLAGS_MODIFY     = (1<<0),
        FLAGS_UPDATE_DEP = (1<<1),
        FLAGS_EXTERNAL   = (1<<2)

    };

    struct node
    {
        rsc_desc*   pDesc;
        time_t      TimeStamp;
        u32         Flags;
        s32         nDeps;
        xhandle     Dep[32];
        xhandle     DependentRsc;
        xstring     CompilerRules;
    };

    struct dependency
    {
        s32     RefCount;
        u32     Flags;
        char    FileName[256];
        time_t  TimeStamp;
    };

public:

                          rsc_desc_mgr      ( void );
    const rsc_desc_type*  GetFirstType      ( void ) const;
    const rsc_desc_type*  GetNextType       ( const rsc_desc_type* PrevType ) const;
    const rsc_desc_type&  GetType           ( const char* pType ) const;

    rsc_desc&             CreateRscDesc     ( const char* pType );
    void                  DeleteRscDesc     ( const char* pName );

    s32                   GetRscDescCount   ( void );
    node&                 GetRscDescIndex   ( s32 Index );

    s32                   GetDepCount       ( void );
    dependency&           GetDepIndex       ( s32 Index );

    node&                 GetRscDesc        ( const char* pName );


    s32                   BeginCompiling    ( void );
    void                  AddExternalRsc    ( const char* pExternalInfo, s32 Index );
    s32                   NextCompiling     ( void );

    void                  EndCompiling      ( void );

protected:
    
    xhandle               FindDep           ( const char* pFileName );
    xhandle               AddDep            ( const char* pFileName );
    void                  DelDep            ( const char* pFileName );
    void                  DelDep            ( xhandle Handle );
    s32                   FindRscDesc       ( const char* pName );
    bool                  NeedCompiling     ( node& RscDesc );
protected:

     xharray<node>          m_lRscDesc;
     xharray<dependency>    m_lDependency;
     xbool                  m_IsCompiling;
     s32                    m_CompilingIndex;
};

//=========================================================================
// Type declarator
//=========================================================================
// Use Macro to create the types don't use the class it self.
//=========================================================================
#define DEFINE_RSC_TYPE( VARNAME, TYPE, TYPE_NAME, FILE_EXT, FILE_DESC )                    \
static struct s_RscDesc##__LINE__##_TYPE : public rsc_desc_type                             \
{   s_RscDesc##__LINE__##_TYPE( void ) : rsc_desc_type( TYPE_NAME, FILE_EXT, FILE_DESC ) {} \
    rsc_desc* CreateRscDesc( void ) const { return (rsc_desc*)new TYPE; } }                 \
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
    const rsc_desc_type*    m_pNext;
    static rsc_desc_type*   s_pHead;

    friend rsc_desc_mgr;
};

//=========================================================================
// INLINE FUNCTIONS
//=========================================================================

//=========================================================================
inline
rsc_desc::rsc_desc( rsc_desc_type& Desc ) :
    m_Type( Desc )
{
}

//=========================================================================
inline
rsc_desc_type::rsc_desc_type( const char* pTypeName, const char* pFileExtension, const char* pTypeDescription ) :
    m_pTypeName         ( pTypeName ),
    m_pFileExtensions   ( pFileExtension ),
    m_pTypeDescription  ( pTypeDescription ),
    m_pNext             ( s_pHead )
{
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
const char* rsc_desc::GetRscName( void ) const
{
    return m_Name;
}

//=========================================================================
inline
const char* rsc_desc::GetRscType( void ) const
{
    return m_Type.GetName();
}

//=========================================================================
inline
void rsc_desc::SetRscName( const char* pRscName )
{
    for( s32 i=0; m_Name[i] = pRscName[i]; i++ )
    { ASSERT( i < 128 ); }
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
    return m_lDependency.GetCount();
}

//=========================================================================
inline
rsc_desc_mgr::dependency& rsc_desc_mgr::GetDepIndex( s32 Index )
{
    return m_lDependency[Index];
}


//=========================================================================
// END
//=========================================================================
#endif