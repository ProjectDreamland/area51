//==============================================================================
//
//  xsc_vm_module.cpp
//
//==============================================================================

#include "xsc_vm_module.hpp"
#include "xsc_vm_core.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  xsc_vm_module
//==============================================================================

xsc_vm_module::xsc_vm_module( void )
{
    // Not yet loaded
    m_Loaded = FALSE;
}

//==============================================================================
//  ~xsc_vm_module
//==============================================================================

xsc_vm_module::~xsc_vm_module( void )
{
}

//==============================================================================
//  Endian conversions
//==============================================================================

static void EndianSwap( u32* v )
{
    *v = ((*v & 0x000000ff) << 24) |
         ((*v & 0x0000ff00) <<  8) |
         ((*v & 0x00ff0000) >>  8) |
         ((*v & 0xff000000) >> 24);
}

static void EndianSwap( s32* v )
{
    EndianSwap( (u32*)v );
}

static void EndianSwap( f32* v )
{
    EndianSwap( (u32*)v );
}

static void EndianSwap( u16* v )
{
    *v = ((*v & 0x00ff) << 8) |
         ((*v & 0xff00) >> 8);
}

static void EndianSwap( s16* v )
{
    EndianSwap( (u16*)v );
}

void xsc_vm_module::EndianConvertHeader    ( xsc_vm_header*     pHeader    )
{
    EndianSwap( &pHeader->Magic            );
    EndianSwap( &pHeader->Version          );
    EndianSwap( &pHeader->ModuleNameOffset );

    EndianSwap( &pHeader->ClassDefCount    );
    EndianSwap( &pHeader->ClassRefCount    );
    EndianSwap( &pHeader->MethodDefCount   );
    EndianSwap( &pHeader->MethodRefCount   );
    EndianSwap( &pHeader->FieldDefCount    );
    EndianSwap( &pHeader->FieldRefCount    );

    EndianSwap( &pHeader->ConstIntCount    );
    EndianSwap( &pHeader->ConstFltCount    );
    EndianSwap( &pHeader->ConstStrCount    );
    EndianSwap( &pHeader->MethodsBytes     );

    EndianSwap( &pHeader->LineNumberCount  );
    EndianSwap( &pHeader->SymbolCount      );
}

void xsc_vm_module::EndianConvertClassDefs ( xsc_vm_classdef*   pClassDef, s32 Count  )
{
    while( Count-- )
    {
        EndianSwap( &pClassDef->ClassNameOffset );
        EndianSwap( &pClassDef->ByteSize        );
        pClassDef++;
    }
}

void xsc_vm_module::EndianConvertClassRefs ( xsc_vm_classref*   pClassRef, s32 Count  )
{
    while( Count-- )
    {
        EndianSwap( &pClassRef->ClassNameOffset );
        pClassRef++;
    }
}

void xsc_vm_module::EndianConvertMethodDefs( xsc_vm_methoddef*  pMethodDef, s32 Count )
{
    while( Count-- )
    {
        EndianSwap( &pMethodDef->Flags            );
        EndianSwap( &pMethodDef->ClassNameOffset  );
        EndianSwap( &pMethodDef->MethodNameOffset );
        EndianSwap( &pMethodDef->SignatureOffset  );
        EndianSwap( &pMethodDef->ArgumentsSize    );
        EndianSwap( &pMethodDef->LocalSize        );
        EndianSwap( &pMethodDef->StackSize        );
        EndianSwap( &pMethodDef->ReturnSize       );
        EndianSwap( &pMethodDef->MethodLength     );
        EndianSwap( &pMethodDef->MethodOffset     );
        pMethodDef++;
    }
}

void xsc_vm_module::EndianConvertMethodRefs( xsc_vm_methodref*  pMethodRef, s32 Count )
{
    while( Count-- )
    {
        EndianSwap( &pMethodRef->ClassNameOffset  );
        EndianSwap( &pMethodRef->MethodNameOffset );
        EndianSwap( &pMethodRef->SignatureOffset  );
        pMethodRef++;
    }
}

void xsc_vm_module::EndianConvertFieldDefs ( xsc_vm_fielddef*   pFieldDef, s32 Count  )
{
    while( Count-- )
    {
        EndianSwap( &pFieldDef->ClassNameOffset  );
        EndianSwap( &pFieldDef->FieldNameOffset );
        EndianSwap( &pFieldDef->SignatureOffset  );
        EndianSwap( &pFieldDef->FieldByteOffset  );
        pFieldDef++;
    }
}

void xsc_vm_module::EndianConvertFieldRefs ( xsc_vm_fieldref*   pFieldRef, s32 Count  )
{
    while( Count-- )
    {
        EndianSwap( &pFieldRef->ClassNameOffset  );
        EndianSwap( &pFieldRef->FieldNameOffset );
        pFieldRef++;
    }
}

void xsc_vm_module::EndianConvertConstInts ( s32*               pConstInt, s32 Count  )
{
    while( Count-- )
    {
        EndianSwap( pConstInt );
        pConstInt++;
    }
}

void xsc_vm_module::EndianConvertConstFlts ( f32*               pConstFlt, s32 Count  )
{
    while( Count-- )
    {
        EndianSwap( pConstFlt );
        pConstFlt++;
    }
}

//==============================================================================
//  Load
//==============================================================================

xbool xsc_vm_module::Load( xsc_vm_core* pVM, const char* pFileName )
{
    xbool   Success = FALSE;

    // Set VM pointer
    m_pVM = pVM;

    // Load the module as a binary
    Success = m_Data.LoadFile( pFileName );
    if( Success )
    {
        // Save filename
        m_FileName = pFileName;

        // Pull class data from the loaded file
        byte* pData = m_Data.GetBuffer();
        m_pHeader   = (xsc_vm_header*)pData;
        ASSERT( m_pHeader );

#ifdef BIG_ENDIAN
        // Endian convert the header
        EndianConvertHeader( m_pHeader );
#endif

        // Check for valid header
        if( (m_pHeader->Magic == XSC_VM_MAGIC) && (m_pHeader->Version == XSC_VM_VERSION) )
        {
            m_pHeader->pModule = this;
            m_pClassDef   = (xsc_vm_classdef*  )(pData += sizeof(xsc_vm_header    ));
            m_pClassRef   = (xsc_vm_classref*  )(pData += sizeof(xsc_vm_classdef  )*m_pHeader->ClassDefCount  );
            m_pMethodDef  = (xsc_vm_methoddef* )(pData += sizeof(xsc_vm_classref  )*m_pHeader->ClassRefCount  );
            m_pMethodRef  = (xsc_vm_methodref* )(pData += sizeof(xsc_vm_methoddef )*m_pHeader->MethodDefCount );
            m_pFieldDef   = (xsc_vm_fielddef*  )(pData += sizeof(xsc_vm_methodref )*m_pHeader->MethodRefCount );
            m_pFieldRef   = (xsc_vm_fieldref*  )(pData += sizeof(xsc_vm_fielddef  )*m_pHeader->FieldDefCount  );
            m_pConstInt   = (s32*              )(pData += sizeof(xsc_vm_fieldref  )*m_pHeader->FieldRefCount  );
            m_pConstFlt   = (f32*              )(pData += sizeof(s32              )*m_pHeader->ConstIntCount  );
            m_pConstStr   = (char*             )(pData += sizeof(f32              )*m_pHeader->ConstFltCount  );
            m_pMethods    = (u8*               )(pData += sizeof(char             )*m_pHeader->ConstStrCount  );
            m_pLineNumber = (xsc_vm_linenumber*)(pData += sizeof(u8               )*m_pHeader->MethodsBytes   );
            m_pSymbol     = (xsc_vm_symbol    *)(pData += sizeof(xsc_vm_linenumber)*m_pHeader->LineNumberCount);
            m_Loaded      = TRUE;

#ifdef BIG_ENDIAN
            // Do endian conversion on all data
            EndianConvertClassDefs ( m_pClassDef,  m_pHeader->ClassDefCount  );
            EndianConvertClassRefs ( m_pClassRef,  m_pHeader->ClassRefCount  );
            EndianConvertMethodDefs( m_pMethodDef, m_pHeader->MethodDefCount );
            EndianConvertMethodRefs( m_pMethodRef, m_pHeader->MethodRefCount );
            EndianConvertFieldDefs ( m_pFieldDef,  m_pHeader->FieldDefCount  );
            EndianConvertFieldRefs ( m_pFieldRef,  m_pHeader->FieldRefCount  );
            EndianConvertConstInts ( m_pConstInt,  m_pHeader->ConstIntCount  );
            EndianConvertConstFlts ( m_pConstFlt,  m_pHeader->ConstFltCount  );
#endif
        }
        else
        {
            // Bad module file format or version
            ASSERT( 0 );
        }
    }

    // Return success code
    return Success;
}

//==============================================================================
//  Link
//==============================================================================

xbool xsc_vm_module::Link( void )
{
    s32     i;
    xbool   Success = TRUE;

    ASSERT( m_Loaded );
    ASSERT( m_pVM );

    // Link ClassDefs
    for( i=0 ; i<m_pHeader->ClassDefCount ; i++ )
    {
        m_pClassDef[i].pHeader = m_pHeader;
        Success &= (m_pClassDef[i].pHeader != 0);
        ASSERT( Success );
    }

    // Link ClassRefs
    for( i=0 ; i<m_pHeader->ClassRefCount ; i++ )
    {
        // Get Class name
        const char* pClassName = &m_pConstStr[m_pClassRef[i].ClassNameOffset];

        // Find Class
        m_pClassRef[i].pClassDef = m_pVM->FindClass( pClassName );
        Success &= (m_pClassRef[i].pClassDef != 0);
        ASSERT( Success );
        
        if( m_pClassRef[i].pClassDef == 0 )
        {
            x_printf( xfs( "Link error '%s'\n", &m_pConstStr[m_pClassRef[i].ClassNameOffset] ) );
        }
    }

    // Link MethodDefs
    for( i=0 ; i<m_pHeader->MethodDefCount ; i++ )
    {
        // Get Class and Method name
        const char* pClassName  = &m_pConstStr[m_pMethodDef[i].ClassNameOffset];
        const char* pMethodName = &m_pConstStr[m_pMethodDef[i].MethodNameOffset];

        // Link Class pointer
        m_pMethodDef[i].pClassDef = m_pVM->FindClass( pClassName );
        Success &= (m_pMethodDef[i].pClassDef != 0);
        ASSERT( Success );

        if( m_pMethodDef[i].pClassDef == 0 )
        {
            x_printf( xfs( "Link error '%s'\n", pClassName ) );
        }

        // Native?
        if( m_pMethodDef[i].Flags & XSC_VM_METHOD_NATIVE )
        {
            // Locate in registered native methods
            s32 Address = (s32)m_pVM->FindNativeMethod( pClassName, pMethodName );
            if( Address )
                m_pMethodDef[i].MethodOffset = Address;
            else
                Success = FALSE;
            ASSERT( Success );

            if( m_pMethodDef[i].MethodOffset == 0 )
            {
                x_printf( xfs( "Link error '%s':'%s'\n", pClassName, pMethodName ) );
            }
        }
    }

    // Link MethodRefs
    for( i=0 ; i<m_pHeader->MethodRefCount ; i++ )
    {
        // Get Class and Method name
        const char* pClassName  = &m_pConstStr[m_pMethodRef[i].ClassNameOffset ];
        const char* pMethodName = &m_pConstStr[m_pMethodRef[i].MethodNameOffset];

        // Find Method
        m_pMethodRef[i].pMethodDef = m_pVM->FindMethod( pClassName, pMethodName );
        Success &= (m_pMethodRef[i].pMethodDef != 0);
        ASSERT( Success );

        if( m_pMethodRef[i].pMethodDef == 0 )
        {
            x_printf( xfs( "Link error '%s':'%s'\n", pClassName, pMethodName ) );
        }
    }

    // Link FieldDefs
    for( i=0 ; i<m_pHeader->FieldDefCount ; i++ )
    {
        // Get Class name
        const char* pClassName  = &m_pConstStr[m_pFieldDef[i].ClassNameOffset];

        // Find Class
        m_pFieldDef[i].pClassDef = m_pVM->FindClass( pClassName );
        Success &= (m_pFieldDef[i].pClassDef != 0);
        ASSERT( Success );

        if( m_pFieldDef[i].pClassDef == 0 )
        {
            x_printf( xfs( "Link error '%s'\n", pClassName ) );
        }
    }

    // Link FieldRefs
    for( i=0 ; i<m_pHeader->FieldRefCount ; i++ )
    {
        // Get Class and Field name
        const char* pClassName = &m_pConstStr[m_pFieldRef[i].ClassNameOffset];
        const char* pFieldName = &m_pConstStr[m_pFieldRef[i].FieldNameOffset];

        // Find Field
        m_pFieldRef[i].pFieldDef = m_pVM->FindField( pClassName, pFieldName );
        Success &= (m_pFieldRef[i].pFieldDef != 0);
        ASSERT( Success );

        if( m_pFieldRef[i].pFieldDef == 0 )
        {
            x_printf( xfs( "Link error '%s':'%s'\n", pClassName, pFieldName ) );
        }
    }

    // Return success code
    return Success;
}

//==============================================================================
//  GetNumClassDefs
//==============================================================================

s32 xsc_vm_module::GetNumClassDefs( void )
{
    return m_pHeader->ClassDefCount;
}

//==============================================================================
//  GetNumMethodDefs
//==============================================================================

s32 xsc_vm_module::GetNumMethodDefs( void )
{
    return m_pHeader->MethodDefCount;
}

//==============================================================================
//  GetNumFieldDefs
//==============================================================================

s32 xsc_vm_module::GetNumFieldDefs( void )
{
    return m_pHeader->FieldDefCount;
}

//==============================================================================
//  GetConstantInt
//==============================================================================

s32 xsc_vm_module::GetConstantInt( s32 Index )
{
    return m_pConstInt[Index];
}

//==============================================================================
//  GetConstantFlt
//==============================================================================

f32 xsc_vm_module::GetConstantFlt( s32 Index )
{
    return m_pConstFlt[Index];
}

//==============================================================================
//  GetConstantStr
//==============================================================================

const char* xsc_vm_module::GetConstantStr( s32 Index )
{
    return &m_pConstStr[Index];
}

//==============================================================================
//  GetClassDef
//==============================================================================

xsc_vm_classdef* xsc_vm_module::GetClassDef( s32 Index ) const
{
    return &m_pClassDef[Index];
}

xsc_vm_classdef* xsc_vm_module::GetClassDef( const char* pClassName ) const
{
    ASSERT( m_Loaded );

    // Search definitions
    for( s32 i=0 ; i<m_pHeader->ClassDefCount ; i++ )
    {
        // Check name for a match
        if( x_strcmp( pClassName, &m_pConstStr[m_pClassDef[i].ClassNameOffset] ) == 0 )
            return &m_pClassDef[i];
    }

    // Not found
    return NULL;
}

//==============================================================================
//  GetMethodDef
//==============================================================================

xsc_vm_methoddef* xsc_vm_module::GetMethodDef( s32 Index ) const
{
    return &m_pMethodDef[Index];
}

xsc_vm_methoddef* xsc_vm_module::GetMethodDef( const char* pClassName, const char* pMethodName ) const
{
    ASSERT( m_Loaded );

    // Search definitions
    for( s32 i=0 ; i<m_pHeader->MethodDefCount ; i++ )
    {
        // Check names for a match
        if( (x_strcmp( pClassName,  &m_pConstStr[m_pMethodDef[i].ClassNameOffset ] ) == 0) &&
            (x_strcmp( pMethodName, &m_pConstStr[m_pMethodDef[i].MethodNameOffset] ) == 0) )
            return &m_pMethodDef[i];
    }

    // Not found
    return NULL;
}

//==============================================================================
//  GetFieldDef
//==============================================================================

xsc_vm_fielddef* xsc_vm_module::GetFieldDef( s32 Index ) const
{
    return &m_pFieldDef[Index];
}

xsc_vm_fielddef* xsc_vm_module::GetFieldDef( const char* pClassName, const char* pFieldName ) const
{
    ASSERT( m_Loaded );

    // Search definitions
    for( s32 i=0 ; i<m_pHeader->FieldDefCount ; i++ )
    {
        // Check names for a match
        if( (x_strcmp( pClassName, &m_pConstStr[m_pFieldDef[i].ClassNameOffset] ) == 0) &&
            (x_strcmp( pFieldName, &m_pConstStr[m_pFieldDef[i].FieldNameOffset] ) == 0) )
            return &m_pFieldDef[i];
    }

    // Not found
    return NULL;
}

//==============================================================================
//  Dump
//==============================================================================

xstring xsc_vm_module::Dump( void ) const
{
    s32     i;
    xstring Output;

    ASSERT( m_Loaded );

    // Dump ModuleName
    Output.AddFormat( "Module = '%s'\n", (const char*)m_FileName );

    // Dump ClassDefs
    Output.AddFormat( "\n" );
    Output.AddFormat( "ClassDefs\n" );
    Output.AddFormat( "---------\n" );
    for( i=0 ; i<m_pHeader->ClassDefCount ; i++ )
    {
        xsc_vm_classdef& cd = m_pClassDef[i];

        Output.AddFormat( "%s %4d bytes\n", m_pConstStr+cd.ClassNameOffset, cd.ByteSize );
    }

    // Dump ClassRefs
    Output.AddFormat( "\n" );
    Output.AddFormat( "ClassRefs\n" );
    Output.AddFormat( "---------\n" );
    for( i=0 ; i<m_pHeader->ClassRefCount ; i++ )
    {
        xsc_vm_classref& cr = m_pClassRef[i];

        Output.AddFormat( "%s\n", m_pConstStr+cr.ClassNameOffset );
    }

    // Dump MethodDefs
    Output.AddFormat( "\n" );
    Output.AddFormat( "MethodDefs\n" );
    Output.AddFormat( "----------\n" );
    for( i=0 ; i<m_pHeader->MethodDefCount ; i++ )
    {
        xsc_vm_methoddef& md = m_pMethodDef[i];

        Output.AddFormat( "%s.%s %4d %4d %4d %4d %4d\n",
                          m_pConstStr+md.ClassNameOffset,
                          m_pConstStr+md.MethodNameOffset,
                          md.MethodOffset,
                          md.MethodLength,
                          md.ArgumentsSize,
                          md.StackSize,
                          md.LocalSize );

        // Disassemble method
        s32 IP    = m_pMethodDef[i].MethodOffset;
        s32 IPend = IP + m_pMethodDef[i].MethodLength;
        while( IP < IPend )
        {
            xstring s = m_pVM->Disasm( this, IP );
            Output.AddFormat( "    %s\n", (const char*)s );
        }
    }

    // Dump MethodRefs
    Output.AddFormat( "\n" );
    Output.AddFormat( "MethodRefs\n" );
    Output.AddFormat( "----------\n" );
    for( i=0 ; i<m_pHeader->MethodRefCount ; i++ )
    {
        xsc_vm_methodref& mr = m_pMethodRef[i];

        Output.AddFormat( "%s.%s\n",
                          m_pConstStr+mr.ClassNameOffset,
                          m_pConstStr+mr.MethodNameOffset );
    }

    // Dump FieldDefs
    Output.AddFormat( "\n" );
    Output.AddFormat( "FieldDefs\n" );
    Output.AddFormat( "---------\n" );
    for( i=0 ; i<m_pHeader->FieldDefCount ; i++ )
    {
        xsc_vm_fielddef& fd = m_pFieldDef[i];

        Output.AddFormat( "%s.%s %4d\n",
                          m_pConstStr+fd.ClassNameOffset,
                          m_pConstStr+fd.FieldNameOffset,
                          fd.FieldByteOffset );
    }

    // Dump FieldRefs
    Output.AddFormat( "\n" );
    Output.AddFormat( "FieldRefs\n" );
    Output.AddFormat( "---------\n" );
    for( i=0 ; i<m_pHeader->FieldRefCount ; i++ )
    {
        xsc_vm_fieldref& fr = m_pFieldRef[i];

        Output.AddFormat( "%s.%s\n",
                          m_pConstStr+fr.ClassNameOffset,
                          m_pConstStr+fr.FieldNameOffset );
    }

    // Dump ConstantInts
    Output.AddFormat( "\n" );
    Output.AddFormat( "ConstantInts\n" );
    Output.AddFormat( "------------\n" );
    for( i=0 ; i<m_pHeader->ConstIntCount ; i++ )
    {
        Output.AddFormat( "%d\n", m_pConstInt[i] );
    }

    // Dump ConstantFlts
    Output.AddFormat( "\n" );
    Output.AddFormat( "ConstantFlts\n" );
    Output.AddFormat( "------------\n" );
    for( i=0 ; i<m_pHeader->ConstFltCount ; i++ )
    {
        Output.AddFormat( "%f\n", m_pConstFlt[i] );
    }

    // Dump ConstantStrs
    Output.AddFormat( "\n" );
    Output.AddFormat( "ConstantStrs\n" );
    Output.AddFormat( "------------\n" );
    const char* p = m_pConstStr;
    while( p<(m_pConstStr+m_pHeader->ConstStrCount) )
    {
        s32 Len = x_strlen( p );
        Output.AddFormat( "%s\n", p );
        p += Len+1;
    }

    // TODO: Dump LineNumber & Symbol Records

    // Return dump
    return Output;
}

//==============================================================================
