//==============================================================================
//  
//  xsc_vm_core.hpp
//  
//==============================================================================

#ifndef XSC_VM_CORE_HPP
#define XSC_VM_CORE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_files.hpp"
#include "xsc_vm_fileformat.hpp"

//==============================================================================
//  Defines / Switches
//==============================================================================

#define VM_SWITCH_TIMING    1

//==============================================================================

#if VM_SWITCH_TIMING
extern xtimer g_vmTimer;
#endif

//==============================================================================
//  Native function helpers
//==============================================================================

typedef void (*fnptr_vm)(u8*);

inline void* vmarg_ptr( u8*& pArgs )
{
    void* r = *(void**)pArgs;
    pArgs += 4;
    return r;
}

inline f32 vmarg_f32( u8*& pArgs )
{
    f32 r = *(f32*)pArgs;
    pArgs += 4;
    return r;
}

inline s32 vmarg_s32( u8*& pArgs )
{
    s32 r = *(s32*)pArgs;
    pArgs += 4;
    return r;
}

inline xbool vmarg_xbool( u8*& pArgs )
{
    xbool r = *(xbool*)pArgs;
    pArgs += 4;
    return r;
}

//==============================================================================
//  xsc_vm_core
//==============================================================================

class xsc_vm_module;

class xsc_vm_core
{
//==============================================================================
//  nativemethod
protected:
    struct nativemethod
    {
        u32                 Flags;
        const char*         pClassName;
        const char*         pMethodName;
        const char*         pSignature;
        fnptr_vm            pFn;
    };

//==============================================================================
//  stackframe
protected:
    struct stackframe
    {
        xsc_vm_module*      pModule;
        xsc_vm_methoddef*   pMethod;
        u8*                 pArguments;
        u8*                 pLocals;
        u8*                 pOperands;
        u8*                 pThis;
        u8*                 SP;
        u8*                 IP;
    };

//==============================================================================
//  Functions
public:
                        xsc_vm_core             ( void );
                       ~xsc_vm_core             ( void );

    void                Init                    ( s32                   StackByteSize,
                                                  s32                   StackDepth );           // Init size & depth of stack

    xbool               LoadModule              ( const char*           pFileName );            // Load a Module
    xbool               ReloadModule            ( xsc_vm_module*        pModule );              // Reload Module
    xbool               Link                    ( void );                                       // Link all modules

    xsc_vm_classdef*    FindClass               ( const char*           pClassName );           // Find a Class
    xsc_vm_methoddef*   FindMethod              ( const char*           pClassName,
                                                  const char*           pMethodName );          // Find a Method
    xsc_vm_fielddef*    FindField               ( const char*           pClassName,
                                                  const char*           pFieldName );           // Find a Field

    void                RegisterNativeMethod    ( const char*           pClassName,
                                                  const char*           pMethodName,
                                                  const char*           pSignature,
                                                  fnptr_vm              pFn     );              // Register Native Method

    nativemethod*       FindNativeMethod        ( const char*           pClassName,
                                                  const char*           pMethodName );          // Find a native method

    void                ExecuteMethod           ( xsc_vm_methoddef*     pMethod,
                                                  void*                 pThis,
                                                  ... );                                        // Execute method

    // System method setup
    void                RegisterSystemMethods   ( void );

    // Execution Unit
	void				ValidateExecTable		( void ) const;
    void                PushStackFrame          ( void );
    void                PopStackFrame           ( void );
    void                Exec                    ( void );
    void                ExecInstruction         ( void );

    s32                 Operand16               ( void );
    s32                 Pop_s32                 ( void );
    f32                 Pop_f32                 ( void );
    void                Push_s32                ( s32                   Value );
    void                Push_f32                ( f32                   Value );

    // Instruction Set
    void                Exec_nop                ( void );
    void                Exec_assert             ( void );
    void                Exec_break              ( void );

    void                Exec_ba                 ( void );
    void                Exec_bf                 ( void );
    void                Exec_bt                 ( void );

    void                Exec_ftoi               ( void );
    void                Exec_itof               ( void );

    void                Exec_icmp_eq            ( void );
    void                Exec_icmp_ge            ( void );
    void                Exec_icmp_gt            ( void );
    void                Exec_icmp_le            ( void );
    void                Exec_icmp_lt            ( void );
    void                Exec_icmp_ne            ( void );

    void                Exec_fcmp_eq            ( void );
    void                Exec_fcmp_ge            ( void );
    void                Exec_fcmp_gt            ( void );
    void                Exec_fcmp_le            ( void );
    void                Exec_fcmp_lt            ( void );
    void                Exec_fcmp_ne            ( void );

    void                Exec_idup               ( void );
    void                Exec_fdup               ( void );
    void                Exec_cdup               ( void );

    void                Exec_invoke             ( void );
    void                Exec_invokenative       ( void );
    void                Exec_invokestatic       ( void );
    void                Exec_invokevirtual      ( void );

    void                Exec_this               ( void );
    void                Exec_aaddr              ( void );
    void                Exec_laddr              ( void );
    void                Exec_faddr              ( void );

    void                Exec_iconst             ( void );
    void                Exec_fconst             ( void );
    void                Exec_iload              ( void );
    void                Exec_fload              ( void );
    void                Exec_cload              ( void );
    void                Exec_istore             ( void );
    void                Exec_fstore             ( void );
    void                Exec_cstore             ( void );

    void                Exec_iadd               ( void );
    void                Exec_idiv               ( void );
    void                Exec_imod               ( void );
    void                Exec_imul               ( void );
    void                Exec_ineg               ( void );
    void                Exec_isub               ( void );

    void                Exec_fadd               ( void );
    void                Exec_fdiv               ( void );
    void                Exec_fmod               ( void );
    void                Exec_fmul               ( void );
    void                Exec_fneg               ( void );
    void                Exec_fsub               ( void );

    void                Exec_bit_and            ( void );
    void                Exec_bit_or             ( void );
    void                Exec_log_and            ( void );
    void                Exec_log_or             ( void );
    void                Exec_not                ( void );
    void                Exec_shl                ( void );
    void                Exec_shr                ( void );
    void                Exec_xor                ( void );

    void                Exec_pop                ( void );

    void                Exec_vret               ( void );
    void                Exec_iret               ( void );
    void                Exec_fret               ( void );
    void                Exec_cret               ( void );
    void                Exec_rret               ( void );

    // Debugging
    xstring             Dump                    ( void ) const;                                 // Dump

    void                ValidateDisasmTable     ( void ) const;
    xstring             Disasm                  ( const xsc_vm_module*  pModule,
                                                  s32&                  IP );                   // Disassemble instruction

private:

//==============================================================================
//  Data
protected:
    s32                         m_StackByteSize;            // Size of stack in bytes
    u8*                         m_pStackBase;               // Pointer to base of stack
    xarray<stackframe>          m_StackFrames;              // Stack frames
    s32                         m_StackDepth;               // Depth of stack frames (0 = no stack frames)

    xarray<xsc_vm_module*>      m_Modules;                  // Loaded modules
    xarray<nativemethod*>       m_NativeMethods;            // Registered native methods

    // Execution context
    stackframe                  m_StackFrame;               // Current stack frame
};

//==============================================================================
#endif // XSC_VM_CORE_HPP
//==============================================================================
