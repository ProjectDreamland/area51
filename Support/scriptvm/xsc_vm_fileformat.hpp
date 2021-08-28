//==============================================================================
//  
//  xsc_fileformat.hpp
//  
//==============================================================================

#ifndef XSC_FILEFORMAT_HPP
#define XSC_FILEFORMAT_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"

//==============================================================================
//  xscm ( X Scipt Compiled Module ) File Format
//==============================================================================
//
//  An xscm file is an 'xsc_vm_module' header followed by binary data which
//  contains other 'xsc_vm_' structures and constant pools.
//
//  All Offsets are relative to the beginning of the file or the beginning
//  of their respective constant pool as documented in the comment.
//
//==============================================================================
//
//==============================================================================

#define XSC_VM_MAGIC    0x6d637378                      // Magic Number 'xscm'
#define XSC_VM_VERSION  1                               // Version Number

//==============================================================================
//  xsc_vm_header
//==============================================================================

struct xsc_vm_header
{
    // Format Identifier
    s32                 Magic;                          // 'xscm'
    s32                 Version;                        // File version number
    s32                 ModuleNameOffset;               // Offset to name of module in constant str pool
    class xsc_vm_module*pModule;                        // Pointer to Module class

    // Class, Method & Field Defs & Refs
    s16                 ClassDefCount;                  // Number of ClassDefs
    s16                 ClassRefCount;                  // Number of ClassRefs
    s16                 MethodDefCount;                 // Number of MethodDefs
    s16                 MethodRefCount;                 // Number of MethodRefs
    s16                 FieldDefCount;                  // Number of FieldDefs
    s16                 FieldRefCount;                  // Number fo FieldRefs

    // Constant Pools
    s16                 ConstIntCount;                  // Number of Constant Ints
    s16                 ConstFltCount;                  // Number of Constant Flts
    s16                 ConstStrCount;                  // Number of Constant Strs
    s16                 MethodsBytes;                   // Bytes of Method Bodies

    // Debugging
    s16                 LineNumberCount;                // Number of LineNumbers for debug
    s16                 SymbolCount;                    // Number of Symbols for debug
};

//==============================================================================
//  xsc_vm_classdef, classref
//==============================================================================

struct xsc_vm_classdef
{
    xsc_vm_header*      pHeader;                        // Resolved pointer to Module header
    s16                 ClassNameOffset;                // Offset to name of class in constant str pool
    s16                 ByteSize;                       // Byte size of a class instance
};

struct xsc_vm_classref
{
    xsc_vm_classdef*    pClassDef;                      // Resolved pointer to ClassDef
    s16                 ClassNameOffset;                // Offset to name of class in constant str pool
    s16                 Pad1;
};

//==============================================================================
//  xsc_vm_methoddef, methodref
//==============================================================================

#define XSC_VM_METHOD_CONST     (1<<0)
#define XSC_VM_METHOD_STATIC    (1<<1)
#define XSC_VM_METHOD_VIRTUAL   (1<<2)
#define XSC_VM_METHOD_NATIVE    (1<<3)

struct xsc_vm_methoddef
{
    xsc_vm_classdef*    pClassDef;                      // Resolved pointer to class definition
    u16                 Flags;                          // Flags
    s16                 ClassNameOffset;                // Offset to name of class in constant str pool
    s16                 MethodNameOffset;               // Offset to name of method in constant str pool
    s16                 SignatureOffset;                // Offset to signature in constant str pool
    s16                 ArgumentsSize;                  // bytes of arguments for a call to this method
    s16                 LocalSize;                      // Local bytes required for execution
    s16                 StackSize;                      // Stack bytes required for execution
    s16                 ReturnSize;                     // Stack bytes required for return
    s16                 MethodLength;                   // Length of method in bytes
    s32                 MethodOffset;                   // Offset to first instruction in method pool
};

struct xsc_vm_methodref
{
    xsc_vm_methoddef*   pMethodDef;                     // Resolved pointer to MethodDef
    s16                 ClassNameOffset;                // Offset to name of class in constant str pool
    s16                 MethodNameOffset;               // Offset to name of method in constant str pool
    s16                 SignatureOffset;                // Offset to signature in constant str pool
};

//==============================================================================
//  xsc_vm_fielddef, fieldref
//==============================================================================

struct xsc_vm_fielddef
{
    xsc_vm_classdef*    pClassDef;                      // Reolved pointer to class definition
    s16                 ClassNameOffset;                // Offset to name of class in constant str pool
    s16                 FieldNameOffset;                // Offset to name of field in constant str pool
    s16                 SignatureOffset;                // Offset to signature in constant str pool
    s16                 FieldByteOffset;                // Byte offset of field in class instance
};

struct xsc_vm_fieldref
{
    xsc_vm_fielddef*    pFieldDef;                      // Resolved pointer to FieldDef
    s16                 ClassNameOffset;                // Offset to name of class in constant str pool
    s16                 FieldNameOffset;                // Offset to name of field in constant str pool
};

//==============================================================================
//  xsc_vm_linenumber, symbol for debugging information
//==============================================================================

struct xsc_vm_linenumber
{
    s16                 iMethodDef;                     // Index of MethodDef this applies to
    s16                 MethodByte;                     // Byte within method this applies to
    s16                 LineNumber;                     // Line number in source file
    s16                 SourceFileNameOffset;           // Offset to source filename in constant str pool
};

struct xsc_vm_symbol
{
    s16                 SymbolNameOffset;               // Offset to name of symbol in constant str pool
    s16                 TypeFlags;                      // Symbol Type flags
    s16                 iClassRef;                      // Index of ClassRef if this is a class type
    s16                 iMethodDef;                     // Index of MethodDef this applies to
    s16                 ScopeStartByte;                 // Method Byte at which this symbol comes into scope
    s16                 ScopeEndByte;                   // Method Byte at which this symbol goes out of scope
};

//==============================================================================
#endif // XSC_FILEFORMAT_HPP
//==============================================================================
