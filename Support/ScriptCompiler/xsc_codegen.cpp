//==============================================================================
//
//  xsc_codegen
//
//==============================================================================

#include "xsc_codegen.hpp"
#include "xsc_tokenizer.hpp"
#include "xsc_errors.hpp"
#include "xsc_symbol_table.hpp"
#include "xsc_ast.hpp"
#include "../ScriptVM/xsc_vm_fileformat.hpp"
#include "../ScriptVM/xsc_vm_instructions.hpp"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  xsc_codegen
//==============================================================================

xsc_codegen::xsc_codegen( const xwstring&         Source,
                          const xsc_tokenizer&    Tokenizer,
                          xsc_errors&             Errors,
                          xsc_symbol_table&       SymbolTable,
                          xsc_ast&                AST,
                          xsc_parser&             Parser ) : m_Source     ( Source ),
                                                             m_Tokenizer  ( Tokenizer ),
                                                             m_Errors     ( Errors ),
                                                             m_SymbolTable( SymbolTable ),
                                                             m_AST        ( AST ),
                                                             m_Parser     ( Parser )
{
    // Initialize
    m_NoEmitThis = 0;
}

//==============================================================================
//  ~xsc_codegen
//==============================================================================

xsc_codegen::~xsc_codegen( void )
{
}

//==============================================================================
//  Save
//==============================================================================

xbool xsc_codegen::Save( const char* pFileName )
{
    s32         i;
    xbytestream Binary;

    // Output ClassDefs
    m_Header.ClassDefCount  = m_ClassDef.GetCount();
    for( i=0 ; i<m_ClassDef.GetCount() ; i++ )
        Binary.Append( (const byte*)&m_ClassDef[i], sizeof(xsc_vm_classdef) );

    // Output ClassRefs
    m_Header.ClassRefCount  = m_ClassRef.GetCount();
    for( i=0 ; i<m_ClassRef.GetCount() ; i++ )
        Binary.Append( (const byte*)&m_ClassRef[i], sizeof(xsc_vm_classref) );

    // Output MethodDefs
    m_Header.MethodDefCount  = m_MethodDef.GetCount();
    for( i=0 ; i<m_MethodDef.GetCount() ; i++ )
        Binary.Append( (const byte*)&m_MethodDef[i], sizeof(xsc_vm_methoddef) );

    // Output MethodRefs
    m_Header.MethodRefCount  = m_MethodRef.GetCount();
    for( i=0 ; i<m_MethodRef.GetCount() ; i++ )
        Binary.Append( (const byte*)&m_MethodRef[i], sizeof(xsc_vm_methodref) );

    // Output FieldDefs
    m_Header.FieldDefCount  = m_FieldDef.GetCount();
    for( i=0 ; i<m_FieldDef.GetCount() ; i++ )
        Binary.Append( (const byte*)&m_FieldDef[i], sizeof(xsc_vm_fielddef) );

    // Output FieldRefs
    m_Header.FieldRefCount  = m_FieldRef.GetCount();
    for( i=0 ; i<m_FieldRef.GetCount() ; i++ )
        Binary.Append( (const byte*)&m_FieldRef[i], sizeof(xsc_vm_fieldref) );

    // Output Int Constant Pool
    m_Header.ConstIntCount  = m_ConstantInt.GetCount();
    if( m_ConstantInt.GetCount() > 0 )
        Binary.Append( (const byte*)&m_ConstantInt[0], sizeof(s32)*m_ConstantInt.GetCount() );

    // Output Flt Constant Pool
    m_Header.ConstFltCount  = m_ConstantFlt.GetCount();
    if( m_ConstantFlt.GetCount() > 0 )
        Binary.Append( (const byte*)&m_ConstantFlt[0], sizeof(f32)*m_ConstantFlt.GetCount() );

    // Output Str Constant Pool
    m_Header.ConstStrCount  = m_ConstantStr.GetLength();
    Binary.Append( m_ConstantStr );

    // Output Methods
    m_Header.MethodsBytes  = m_Methods.GetLength();
    Binary.Append( m_Methods );

    // Insert Header at head of binary
    Binary.Insert( 0, (byte*)&m_Header, sizeof(xsc_vm_header) );

    // Save the file
    return Binary.SaveFile( pFileName );
}

//==============================================================================
//  EmitConstInt
//==============================================================================

s32 xsc_codegen::EmitConstInt( s32 ConstVal )
{
    // Search constant pool for a match
    for( s32 i=0 ; i<m_ConstantInt.GetCount() ; i++ )
    {
        if( ConstVal == m_ConstantInt[i] )
            return i;
    }

    // Add constant to pool
    m_ConstantInt.Append() = ConstVal;
    return m_ConstantInt.GetCount()-1;
}

//==============================================================================
//  EmitConstFlt
//==============================================================================

s32 xsc_codegen::EmitConstFlt( f32 ConstVal )
{
    // Search constant pool for a match
    for( s32 i=0 ; i<m_ConstantFlt.GetCount() ; i++ )
    {
        if( ConstVal == m_ConstantFlt[i] )
            return i;
    }

    // Add constant to pool
    m_ConstantFlt.Append() = ConstVal;
    return m_ConstantFlt.GetCount()-1;
}

//==============================================================================
//  EmitConstStr
//==============================================================================

s32 xsc_codegen::EmitConstStr( const xwstring& ConstVal )
{
    // TODO: Convert string to UTF8
    xstring String = ConstVal;

    // Seach constant pool for a match
    for( s32 i=0 ; i<m_ConstantStrIndex.GetCount() ; i++ )
    {
        if( m_ConstantStrIndex[i].String == String )
            return m_ConstantStrIndex[i].Offset;
    }

    // Add constant to pool
    str_record& Rec = m_ConstantStrIndex.Append();
    Rec.String = String;
    Rec.Offset = m_ConstantStr.GetLength();
    Rec.Length = String.GetLength();
    m_ConstantStr.Append( (const byte*)(const char*)String, Rec.Length+1 );
    return Rec.Offset;
}

//==============================================================================
//  EmitClassDef
//==============================================================================

s32 xsc_codegen::EmitClassDef( const xwstring&  ClassName,
                               s32              ByteSize )
{
    // Add to pool
    xsc_vm_classdef& ClassDef = m_ClassDef.Append();
    ClassDef.ClassNameOffset = EmitConstStr( ClassName );
    ClassDef.ByteSize        = ByteSize;
    ClassDef.pHeader         = 0;
    return m_ClassDef.GetCount()-1;
}

//==============================================================================
//  EmitClassRef
//==============================================================================

s32 xsc_codegen::EmitClassRef( const xwstring&  ClassName )
{
    // TODO: Convert to UTF8
    xstring String = ClassName;

    // Search for ClassRef
    for( s32 i=0 ; i<m_ClassRef.GetCount() ; i++ )
    {
        if( x_strcmp( (const char*)(m_ConstantStr.GetBuffer() + m_ClassRef[i].ClassNameOffset), String ) == 0 )
            return i;
    }

    // Add to pool
    xsc_vm_classref& ClassRef = m_ClassRef.Append();
    ClassRef.ClassNameOffset = EmitConstStr( ClassName );
    ClassRef.pClassDef       = 0;
    return m_ClassRef.GetCount()-1;
}

//==============================================================================
//  EmitMethodDef
//==============================================================================

s32 xsc_codegen::EmitMethodDef( const xwstring& ClassName,
                                const xwstring& MethodName,
                                const xwstring& Signature,
                                s32             ArgumentsSize,
                                s32             StackSize,
                                s32             LocalSize,
                                s32             ReturnSize,
                                u32             Flags )
{
    // Add to pool
    xsc_vm_methoddef& MethodDef = m_MethodDef.Append();

    MethodDef.ClassNameOffset           = EmitConstStr( ClassName );
    MethodDef.MethodNameOffset          = EmitConstStr( MethodName );
    MethodDef.SignatureOffset           = EmitConstStr( Signature );
    MethodDef.MethodOffset              = m_Methods.GetLength();
    MethodDef.ArgumentsSize             = ArgumentsSize;
    MethodDef.StackSize                 = StackSize;
    MethodDef.LocalSize                 = LocalSize;
    MethodDef.ReturnSize                = ReturnSize;
    MethodDef.Flags                     = Flags;
    MethodDef.pClassDef                 = 0;
    return m_MethodDef.GetCount()-1;
}

//==============================================================================
//  EmitMethodRef
//==============================================================================

s32 xsc_codegen::EmitMethodRef( const xwstring& ClassName,
                                const xwstring& MethodName,
                                const xwstring& Signature )
{
    // TODO: Convert to UTF8
    xstring String1 = ClassName;
    xstring String2 = MethodName;
    xstring String3 = Signature;

    // Search for MethodRef
    for( s32 i=0 ; i<m_MethodRef.GetCount() ; i++ )
    {
        if( ( x_strcmp( (const char*)(m_ConstantStr.GetBuffer() + m_MethodRef[i].ClassNameOffset ), String1 ) == 0 ) &&
            ( x_strcmp( (const char*)(m_ConstantStr.GetBuffer() + m_MethodRef[i].MethodNameOffset), String2 ) == 0 ) &&
            ( x_strcmp( (const char*)(m_ConstantStr.GetBuffer() + m_MethodRef[i].SignatureOffset ), String3 ) == 0 ) )
            return i;
    }

    // Add to pool
    xsc_vm_methodref&   MethodRef = m_MethodRef.Append();
    MethodRef.ClassNameOffset  = EmitConstStr( ClassName );
    MethodRef.MethodNameOffset = EmitConstStr( MethodName );
    MethodRef.SignatureOffset  = EmitConstStr( Signature );
    MethodRef.pMethodDef       = 0;
    return m_MethodRef.GetCount()-1;
}

//==============================================================================
//  EmitFieldDef
//==============================================================================

s32 xsc_codegen::EmitFieldDef( const xwstring&  ClassName,
                               const xwstring&  FieldName,
                               const xwstring&  Signature,
                               s32              FieldByteOffset )
{
    // Add to pool
    xsc_vm_fielddef&    FieldDef = m_FieldDef.Append();
    FieldDef.ClassNameOffset = EmitConstStr( ClassName );
    FieldDef.FieldNameOffset = EmitConstStr( FieldName );
    FieldDef.SignatureOffset = EmitConstStr( Signature );
    FieldDef.FieldByteOffset = FieldByteOffset;
    FieldDef.pClassDef       = 0;
    return m_FieldDef.GetCount()-1;
}

//==============================================================================
//  EmitFieldRef
//==============================================================================

s32 xsc_codegen::EmitFieldRef( const xwstring&  ClassName,
                               const xwstring&  FieldName )
{
    // TODO: Convert to UTF8
    xstring String1 = ClassName;
    xstring String2 = FieldName;

    // Search for FieldRef
    for( s32 i=0 ; i<m_FieldRef.GetCount() ; i++ )
    {
        if( ( x_strcmp( (const char*)(m_ConstantStr.GetBuffer() + m_FieldRef[i].ClassNameOffset), String1 ) == 0 ) &&
            ( x_strcmp( (const char*)(m_ConstantStr.GetBuffer() + m_FieldRef[i].FieldNameOffset), String2 ) == 0 ) )
            return i;
    }

    // Add to pool
    xsc_vm_fieldref&    FieldRef = m_FieldRef.Append();
    FieldRef.ClassNameOffset = EmitConstStr( ClassName );
    FieldRef.FieldNameOffset = EmitConstStr( FieldName );
    FieldRef.pFieldDef       = 0;
    return m_FieldRef.GetCount()-1;
}

//==============================================================================
//  EmitOpcode
//==============================================================================

void xsc_codegen::EmitOpcode( s32 Opcode )
{
    ASSERT( (Opcode>=vm_nop) && (Opcode<vm_last_opcode) );
    m_Methods.Append( (byte)Opcode );
}

//==============================================================================
//  EmitOperand
//==============================================================================

void xsc_codegen::EmitOperand( s32 Operand )
{
    ASSERT( (Operand>=-32768) && (Operand<32767) );
    m_Methods.Append( (byte)((Operand>>8)&0xff) );
    m_Methods.Append( (byte)((Operand>>0)&0xff) );
}

//==============================================================================
//  EmitOperandAt
//==============================================================================

void xsc_codegen::EmitOperandAt( s32 Operand, s32 Address )
{
    ASSERT( (Operand>=-32768) && (Operand<32767) );
    ASSERT( (Address >= 0) && ((Address+1) < m_Methods.GetLength()) );

    m_Methods.SetAt( Address,   (byte)((Operand>>8)&0xff) );
    m_Methods.SetAt( Address+1, (byte)((Operand>>0)&0xff) );
}

//==============================================================================
//  Dump
//==============================================================================

void xsc_codegen::Dump( void ) const
{
    s32     i;

    // Dump ClassDefs
    x_printf( "\n" );
    x_printf( "ClassDefs\n" );
    x_printf( "---------\n" );
    for( i=0 ; i<m_ClassDef.GetCount() ; i++ )
    {
        xsc_vm_classdef& cd = m_ClassDef[i];

        x_printf( "%s %4d bytes\n", m_ConstantStr.GetBuffer()+cd.ClassNameOffset, cd.ByteSize );
    }

    // Dump ClassRefs
    x_printf( "\n" );
    x_printf( "ClassRefs\n" );
    x_printf( "---------\n" );
    for( i=0 ; i<m_ClassRef.GetCount() ; i++ )
    {
        xsc_vm_classref& cr = m_ClassRef[i];

        x_printf( "%s\n", m_ConstantStr.GetBuffer()+cr.ClassNameOffset );
    }

    // Dump MethodDefs
    x_printf( "\n" );
    x_printf( "MethodDefs\n" );
    x_printf( "----------\n" );
    for( i=0 ; i<m_MethodDef.GetCount() ; i++ )
    {
        xsc_vm_methoddef& md = m_MethodDef[i];

        x_printf( "%s.%s %4d %4d %4d %4d %4d\n",
                  m_ConstantStr.GetBuffer()+md.ClassNameOffset,
                  m_ConstantStr.GetBuffer()+md.MethodNameOffset,
                  md.MethodOffset,
                  md.MethodLength,
                  md.ArgumentsSize,
                  md.StackSize,
                  md.LocalSize );
    }

    // Dump MethodRefs
    x_printf( "\n" );
    x_printf( "MethodRefs\n" );
    x_printf( "----------\n" );
    for( i=0 ; i<m_MethodRef.GetCount() ; i++ )
    {
        xsc_vm_methodref& mr = m_MethodRef[i];

        x_printf( "%s.%s\n",
                  m_ConstantStr.GetBuffer()+mr.ClassNameOffset,
                  m_ConstantStr.GetBuffer()+mr.MethodNameOffset );
    }

    // Dump FieldDefs
    x_printf( "\n" );
    x_printf( "FieldDefs\n" );
    x_printf( "---------\n" );
    for( i=0 ; i<m_FieldDef.GetCount() ; i++ )
    {
        xsc_vm_fielddef& fd = m_FieldDef[i];

        x_printf( "%s.%s %4d\n",
                  m_ConstantStr.GetBuffer()+fd.ClassNameOffset,
                  m_ConstantStr.GetBuffer()+fd.FieldNameOffset,
                  fd.FieldByteOffset );
    }

    // Dump FieldRefs
    x_printf( "\n" );
    x_printf( "FieldRefs\n" );
    x_printf( "---------\n" );
    for( i=0 ; i<m_FieldRef.GetCount() ; i++ )
    {
        xsc_vm_fieldref& fr = m_FieldRef[i];

        x_printf( "%s.%s\n",
                  m_ConstantStr.GetBuffer()+fr.ClassNameOffset,
                  m_ConstantStr.GetBuffer()+fr.FieldNameOffset );
    }

    // Dump ConstantInts
    x_printf( "\n" );
    x_printf( "ConstantInts\n" );
    x_printf( "------------\n" );
    for( i=0 ; i<m_ConstantInt.GetCount() ; i++ )
    {
        x_printf( "%d\n", m_ConstantInt[i] );
    }

    // Dump ConstantFlts
    x_printf( "\n" );
    x_printf( "ConstantFlts\n" );
    x_printf( "------------\n" );
    for( i=0 ; i<m_ConstantFlt.GetCount() ; i++ )
    {
        x_printf( "%f\n", m_ConstantFlt[i] );
    }

    // Dump ConstantStrs
    x_printf( "\n" );
    x_printf( "ConstantStrs\n" );
    x_printf( "------------\n" );
    for( i=0 ; i<m_ConstantStrIndex.GetCount() ; i++ )
    {
        x_printf( "%s\n", m_ConstantStr.GetBuffer() + m_ConstantStrIndex[i].Offset );
    }

    // TODO: Dump LineNumber & Symbol Records
}

//==============================================================================
