//==============================================================================
//
//  xsc_vm_core.cpp
//
//==============================================================================

#include "xsc_vm_core.hpp"
#include "xsc_vm_module.hpp"

//==============================================================================
//  Defines
//==============================================================================

#if VM_SWITCH_TIMING
xtimer g_vmTimer;
#endif

//==============================================================================
//  xsc_vm_core
//==============================================================================

xsc_vm_core::xsc_vm_core( void )
{
    // Clear Data
    m_pStackBase  = NULL;
    m_StackDepth  = 0;

	// Validate internal data
#ifdef X_DEBUG
	ValidateExecTable();
    ValidateDisasmTable();
#endif

    // Setup default stack
    Init( 8192, 16 );
}

//==============================================================================
//  ~xsc_vm_core
//==============================================================================

xsc_vm_core::~xsc_vm_core( void )
{
    // Destroy all modules
    while( m_Modules.GetCount() > 0 )
    {
        delete m_Modules[0];
        m_Modules.Delete( 0 );
    }
}

//==============================================================================
//  Init
//==============================================================================

void xsc_vm_core::Init( s32 StackByteSize, s32 StackDepth )
{
    //
    // List of functions that the GCN compiler likes to throw away
    //
    if( 1 )
    {
        RegisterNativeMethod    ( "", "", "", 0 );
    }

    // Allocate a stack
    if( m_pStackBase )
    {
        x_free( m_pStackBase );
        m_pStackBase = NULL;
    }
    m_pStackBase = (u8*)x_malloc( StackByteSize );
    ASSERT( m_pStackBase );
    m_StackByteSize = StackByteSize;

    // Set anticipated number of stack frames
    m_StackFrames.SetCapacity( StackDepth );

    // Register system methods
    RegisterSystemMethods();
}

//==============================================================================
//  LoadModule
//==============================================================================

xbool xsc_vm_core::LoadModule( const char* pFileName )
{
    xsc_vm_module*  pModule = new xsc_vm_module;
    ASSERT( pModule );

    // Attempt to load module
    xbool Success = pModule->Load( this, pFileName );
    if( Success )
    {
        // Success, add module to list
        m_Modules.Append() = pModule;
    }
    else
    {
        // Failed, so delete module
        delete pModule;
    }

    // Return success code
    return Success;
}

//==============================================================================
//  ReloadModule
//==============================================================================

xbool xsc_vm_core::ReloadModule( xsc_vm_module* pModule )
{
    (void)pModule;
    ASSERT( 0 );
    return FALSE;
}

//==============================================================================
//  Link
//==============================================================================

xbool xsc_vm_core::Link( void )
{
    xbool   Success = TRUE;

    // Link each module in turn
    for( s32 i=0 ; i<m_Modules.GetCount() ; i++ )
    {
        // Link module
        Success &= m_Modules[i]->Link();
    }

    // Return success code
    return Success;
}

//==============================================================================
//  FindClass
//==============================================================================

xsc_vm_classdef* xsc_vm_core::FindClass( const char* pClassName )
{
    // Search each module in turn
    for( s32 i=0 ; i<m_Modules.GetCount() ; i++ )
    {
        xsc_vm_classdef* pClassDef = m_Modules[i]->GetClassDef( pClassName );
        if( pClassDef )
            return pClassDef;
    }

    // Not found
    return NULL;
}

//==============================================================================
//  FindMethod
//==============================================================================

xsc_vm_methoddef* xsc_vm_core::FindMethod( const char* pClassName, const char* pMethodName )
{
    // Search each module in turn
    for( s32 i=0 ; i<m_Modules.GetCount() ; i++ )
    {
        xsc_vm_methoddef* pMethodDef = m_Modules[i]->GetMethodDef( pClassName, pMethodName );
        if( pMethodDef )
            return pMethodDef;
    }

    // Not found
    return NULL;
}

//==============================================================================
//  FindField
//==============================================================================

xsc_vm_fielddef* xsc_vm_core::FindField( const char* pClassName, const char* pFieldName )
{
    // Search each module in turn
    for( s32 i=0 ; i<m_Modules.GetCount() ; i++ )
    {
        xsc_vm_fielddef* pFieldDef = m_Modules[i]->GetFieldDef( pClassName, pFieldName );
        if( pFieldDef )
            return pFieldDef;
    }

    // Not found
    return NULL;
}

//==============================================================================
//  RegisterNativeMethod
//==============================================================================

void xsc_vm_core::RegisterNativeMethod( const char* pClassName,
                                        const char* pMethodName,
                                        const char* pSignature,
                                        fnptr_vm    pFn )
{
    // Search for duplicate
    for( s32 i=0 ; i<m_NativeMethods.GetCount() ; i++ )
    {
        nativemethod*& nmp = m_NativeMethods[i];
        if( (x_strcmp( nmp->pClassName,  pClassName  ) == 0) &&
            (x_strcmp( nmp->pMethodName, pMethodName ) == 0) &&
            (x_strcmp( nmp->pSignature,  pSignature  ) == 0) )
        {
            // Error duplicate method registration
            ASSERT( 0 );
        }
    }

    // Add new native method record
    nativemethod*& nmp = m_NativeMethods.Append();
    nmp = new nativemethod;
    ASSERT( nmp );
    nmp->Flags        = XSC_VM_METHOD_NATIVE;
    nmp->pClassName   = pClassName;
    nmp->pMethodName  = pMethodName;
    nmp->pSignature   = pSignature;
    nmp->pFn          = pFn;
}

//==============================================================================
//  FindNativeMethod
//==============================================================================

xsc_vm_core::nativemethod* xsc_vm_core::FindNativeMethod( const char* pClassName, const char* pMethodName )
{
    // Search list
    for( s32 i=0 ; i<m_NativeMethods.GetCount() ; i++ )
    {
        if( (x_strcmp(m_NativeMethods[i]->pClassName,  pClassName ) == 0) &&
            (x_strcmp(m_NativeMethods[i]->pMethodName, pMethodName) == 0) ) //&&
//            (x_strcmp(m_NativeMethods[i]->pSignature,  pSignature ) == 0) )
            return m_NativeMethods[i];
    }

    // Not found
    return 0;
}

//==============================================================================
//  Execute a script method
//==============================================================================

void xsc_vm_core::ExecuteMethod( xsc_vm_methoddef* pMethod, void* pThis, ... )
{
    if( VM_SWITCH_TIMING )
        g_vmTimer.Start();

    if( pMethod )
    {
        // Create a stack frame for the method
        PushStackFrame();
        m_StackFrame.pModule    = pMethod->pClassDef->pHeader->pModule;
        m_StackFrame.pMethod    = pMethod;
        m_StackFrame.pArguments = (u8*)&pThis;
        m_StackFrame.pLocals    = m_pStackBase;
        m_StackFrame.pOperands  = m_pStackBase + pMethod->LocalSize + 0x80; // TODO: Fix LocalBytesRequired
        m_StackFrame.pThis      = (u8*)pThis;
        m_StackFrame.SP         = m_StackFrame.pOperands;
        m_StackFrame.IP         = m_StackFrame.pModule->m_pMethods + pMethod->MethodOffset;

        // Execute until completion
        Exec( );
    }

    if( VM_SWITCH_TIMING )
        g_vmTimer.Stop();
}

//==============================================================================
//  Dump
//==============================================================================

xstring xsc_vm_core::Dump( void ) const
{
    xstring Output;

    // Dump each module in turn
    for( s32 i=0 ; i<m_Modules.GetCount() ; i++ )
    {
        // Dump module
        Output += m_Modules[i]->Dump( );
    }

    // Return Dump
    return Output;
}

//==============================================================================
