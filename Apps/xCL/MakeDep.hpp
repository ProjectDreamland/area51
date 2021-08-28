//=============================================================================

#include "x_files.hpp"

//=============================================================================

struct dep_node
{
    xstring         m_Key;
    xstring         m_PathName;
    xarray<s32>     m_Includes;
    u8              m_MD5Digest[16];
};

//=============================================================================

extern xarray<dep_node> g_DepNodes;

//=============================================================================

s32     dep_ProcessFullyQualifiedFile   ( const xstring& FileName, const xstring& Key );
void    dep_AddIncludePath              ( const xstring& Path );

//=============================================================================
