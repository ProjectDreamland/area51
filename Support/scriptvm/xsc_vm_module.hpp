//==============================================================================
//  
//  xsc_vm_module.hpp
//  
//==============================================================================

#ifndef XSC_VM_MODULE_HPP
#define XSC_VM_MODULE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"
#include "x_bytestream.hpp"
#include "xsc_vm_fileformat.hpp"

//==============================================================================
//  Defines
//==============================================================================

class xsc_vm_core;

//==============================================================================
//  xsc_vm_module
//==============================================================================

class xsc_vm_module
{
    friend class xsc_vm_core;

//==============================================================================
//  Functions
public:
                        xsc_vm_module           ( void );
                       ~xsc_vm_module           ( void );

    void                EndianConvertHeader     ( xsc_vm_header*    pHeader );
    void                EndianConvertClassDefs  ( xsc_vm_classdef*  pClassDef,  s32 Count );
    void                EndianConvertClassRefs  ( xsc_vm_classref*  pClassRef,  s32 Count );
    void                EndianConvertMethodDefs ( xsc_vm_methoddef* pMethodDef, s32 Count );
    void                EndianConvertMethodRefs ( xsc_vm_methodref* pMethodRef, s32 Count );
    void                EndianConvertFieldDefs  ( xsc_vm_fielddef*  pFieldDef,  s32 Count );
    void                EndianConvertFieldRefs  ( xsc_vm_fieldref*  pFieldRef,  s32 Count );
    void                EndianConvertConstInts  ( s32*              pConstInt,  s32 Count );
    void                EndianConvertConstFlts  ( f32*              pConstFlt,  s32 Count );

    xbool               Load                    ( xsc_vm_core*      pVM,
                                                  const char*       pFileName );            // Load module
    xbool               Link                    ( void );                                   // Link module

    s32                 GetNumClassDefs         ( void );
    s32                 GetNumMethodDefs        ( void );
    s32                 GetNumFieldDefs         ( void );

    s32                 GetConstantInt          ( s32 Index );
    f32                 GetConstantFlt          ( s32 Index );
    const char*         GetConstantStr          ( s32 Index );

    xsc_vm_classdef*    GetClassDef             ( s32 Index ) const;
    xsc_vm_classdef*    GetClassDef             ( const char*       pClassName ) const;     // Find Class
    xsc_vm_methoddef*   GetMethodDef            ( s32 Index ) const;
    xsc_vm_methoddef*   GetMethodDef            ( const char*       pClassName,
                                                  const char*       pMethodName ) const;    // Get MethodDef
    xsc_vm_fielddef*    GetFieldDef             ( s32 Index ) const;
    xsc_vm_fielddef*    GetFieldDef             ( const char*       pClassName,
                                                  const char*       pFieldName ) const;     // Get FieldDef

    xstring             Dump                    ( void ) const;                             // Dump Module

//==============================================================================
//  Data
protected:
    xbool                   m_Loaded;                   // TRUE when loaded
    xsc_vm_core*            m_pVM;                      // Pointer to the virtual machine

    xstring                 m_FileName;                 // FileName of module
    xbytestream             m_Data;                     // Loaded module file data

    xsc_vm_header*          m_pHeader;                  // Module header
    xsc_vm_classdef*        m_pClassDef;                // Class Defs
    xsc_vm_classref*        m_pClassRef;                // Class Refs
    xsc_vm_methoddef*       m_pMethodDef;               // Method Defs
    xsc_vm_methodref*       m_pMethodRef;               // Method Refs
    xsc_vm_fielddef*        m_pFieldDef;                // Field Defs
    xsc_vm_fieldref*        m_pFieldRef;                // Field Refs

    s32*                    m_pConstInt;                // Int constant pool
    f32*                    m_pConstFlt;                // Flt constant pool
    char*                   m_pConstStr;                // Str constant pool
    u8*                     m_pMethods;                 // Methods pool

    xsc_vm_linenumber*      m_pLineNumber;              // Linenumbers for debugging
    xsc_vm_symbol*          m_pSymbol;                  // Symbols for debugging
};

//==============================================================================
#endif // XSC_VM_MODULE_HPP
//==============================================================================
