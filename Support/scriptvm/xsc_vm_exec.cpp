//==============================================================================
//
//  xsc_vm_exec.cpp
//
//==============================================================================

#include "xsc_vm_core.hpp"
#include "xsc_vm_module.hpp"
#include "xsc_vm_instructions.hpp"

//==============================================================================
//  Defines
//==============================================================================

struct exec_data
{
    s32     Opcode;
    void    (xsc_vm_core::*pfn)();
};

exec_data ExecData[] =
{
    { vm_nop,           &xsc_vm_core::Exec_nop               },          // no operation                  
    { vm_assert,        &xsc_vm_core::Exec_assert            },          // assert exception              
    { vm_break,         &xsc_vm_core::Exec_break             },          // break exception               

    { vm_ba,            &xsc_vm_core::Exec_ba                },          // Branch Always                 
    { vm_bf,            &xsc_vm_core::Exec_bf                },          // Branch False                  
    { vm_bt,            &xsc_vm_core::Exec_bt                },          // Branch True                   

    { vm_itof,          &xsc_vm_core::Exec_itof              },          // cast int to float
    { vm_ftoi,          &xsc_vm_core::Exec_ftoi              },          // cast float to int

    { vm_icmp_eq,       &xsc_vm_core::Exec_icmp_eq           },          // compare equal int             
    { vm_icmp_ge,       &xsc_vm_core::Exec_icmp_ge           },          // compare greater or equal int  
    { vm_icmp_gt,       &xsc_vm_core::Exec_icmp_gt           },          // compare greater int           
    { vm_icmp_le,       &xsc_vm_core::Exec_icmp_le           },          // compare less or equal int     
    { vm_icmp_lt,       &xsc_vm_core::Exec_icmp_lt           },          // compare less than int         
    { vm_icmp_ne,       &xsc_vm_core::Exec_icmp_ne           },          // compare not equal int         
                        
    { vm_fcmp_eq,       &xsc_vm_core::Exec_fcmp_eq           },          // compare equal float           
    { vm_fcmp_ge,       &xsc_vm_core::Exec_fcmp_ge           },          // compare greater or equal float
    { vm_fcmp_gt,       &xsc_vm_core::Exec_fcmp_gt           },          // compare greater float         
    { vm_fcmp_le,       &xsc_vm_core::Exec_fcmp_le           },          // compare less or equal float   
    { vm_fcmp_lt,       &xsc_vm_core::Exec_fcmp_lt           },          // compare less than float       
    { vm_fcmp_ne,       &xsc_vm_core::Exec_fcmp_ne           },          // compare not equal float       
                        
    { vm_idup,          &xsc_vm_core::Exec_idup              },          // Duplicate top stack item int  
    { vm_fdup,          &xsc_vm_core::Exec_fdup              },          // Duplicate top stack item float
    { vm_cdup,          &xsc_vm_core::Exec_cdup              },          // Duplicate top stack item class
                        
    { vm_invoke,        &xsc_vm_core::Exec_invoke            },          // Invoke instance method        
    { vm_invokenative,  &xsc_vm_core::Exec_invokenative      },          // Invoke native method          
    { vm_invokestatic,  &xsc_vm_core::Exec_invokestatic      },          // Invoke static method          
    { vm_invokevirtual, &xsc_vm_core::Exec_invokevirtual     },          // Invoke virtual method         
                        
    { vm_this,          &xsc_vm_core::Exec_this              },          // Load this pointer
    { vm_aaddr,         &xsc_vm_core::Exec_aaddr             },          // Load Argument address
    { vm_laddr,         &xsc_vm_core::Exec_laddr             },          // Load Local address
    { vm_faddr,         &xsc_vm_core::Exec_faddr             },          // Add field address to top of stack
                        
    { vm_iconst,        &xsc_vm_core::Exec_iconst            },          // Load Const Int                
    { vm_fconst,        &xsc_vm_core::Exec_fconst            },          // Load Const Float              
    { vm_iload,         &xsc_vm_core::Exec_iload             },          // Load Int from address
    { vm_fload,         &xsc_vm_core::Exec_fload             },          // Load Flt from address
    { vm_cload,         &xsc_vm_core::Exec_cload             },          // Load Class from address
    { vm_istore,        &xsc_vm_core::Exec_istore            },          // Store Int to address
    { vm_fstore,        &xsc_vm_core::Exec_fstore            },          // Store Flt to address
    { vm_cstore,        &xsc_vm_core::Exec_cstore            },          // Store Class to address
                        
    { vm_iadd,          &xsc_vm_core::Exec_iadd              },          // Add top 2 stack Int           
    { vm_idiv,          &xsc_vm_core::Exec_idiv              },          // Div top 2 stack Int           
    { vm_imod,          &xsc_vm_core::Exec_imod              },          // Mod top 2 stack Int           
    { vm_imul,          &xsc_vm_core::Exec_imul              },          // Mul top 2 stack Int           
    { vm_ineg,          &xsc_vm_core::Exec_ineg              },          // Neg top stack Int             
    { vm_isub,          &xsc_vm_core::Exec_isub              },          // Sub top 2 stack Int           
                        
    { vm_fadd,          &xsc_vm_core::Exec_fadd              },          // Add top 2 stack Float         
    { vm_fdiv,          &xsc_vm_core::Exec_fdiv              },          // Div top 2 stack Float         
    { vm_fmod,          &xsc_vm_core::Exec_fmod              },          // Mod top 2 stack Float         
    { vm_fmul,          &xsc_vm_core::Exec_fmul              },          // Mul top 2 stack Float         
    { vm_fneg,          &xsc_vm_core::Exec_fneg              },          // Neg top stack Float           
    { vm_fsub,          &xsc_vm_core::Exec_fsub              },          // Sub top 2 stack Float         
                        
    { vm_bit_and,       &xsc_vm_core::Exec_bit_and           },          // Bitwise And top 2 stack Int   
    { vm_bit_or,        &xsc_vm_core::Exec_bit_or            },          // Bitwise Or top 2 stack Int    
    { vm_log_and,       &xsc_vm_core::Exec_log_and           },          // Logical And top 2 stack Int   
    { vm_log_or,        &xsc_vm_core::Exec_log_or            },          // Logical Or top 2 stack Int    
    { vm_not,           &xsc_vm_core::Exec_not               },          // Not top stack Int             
    { vm_shl,           &xsc_vm_core::Exec_shl               },          // Shift Left                    
    { vm_shr,           &xsc_vm_core::Exec_shr               },          // Shift Right                   
    { vm_xor,           &xsc_vm_core::Exec_xor               },          // Xor top 2 stack Int           
                        
    { vm_pop,           &xsc_vm_core::Exec_pop               },          // Pop n bytes from stack        
                        
    { vm_vret,          &xsc_vm_core::Exec_vret              },          // Return Void                   
    { vm_iret,          &xsc_vm_core::Exec_iret              },          // Return Int                    
    { vm_fret,          &xsc_vm_core::Exec_fret              },          // Return Flt                    
    { vm_cret,          &xsc_vm_core::Exec_cret              },          // Return Class                  
    { vm_rret,          &xsc_vm_core::Exec_rret              },          // Return Reference              
};

//==============================================================================
//  ValidateExecTable
//==============================================================================

void xsc_vm_core::ValidateExecTable( void ) const
{
    // Loop through table validating entries
    for( s32 i=0 ; i<(s32)((sizeof(ExecData)/sizeof(exec_data))) ; i++ )
    {
        ASSERT( ExecData[i].Opcode == i );
    }
}

//==============================================================================
//  PushStackFrame
//==============================================================================

void xsc_vm_core::PushStackFrame( void )
{
    // Save Stack Frame
    if( m_StackDepth > 0 )
        m_StackFrames[m_StackDepth-1] = m_StackFrame;

    // Add a new stack frame
    m_StackFrames.Append();
    m_StackDepth++;
}

//==============================================================================
//  PopStackFrame
//==============================================================================

void xsc_vm_core::PopStackFrame( void )
{
    ASSERT( m_StackFrames.GetCount() > 0 );

    // Delete last frame
    m_StackFrames.Delete( m_StackFrames.GetCount()-1 );
    m_StackDepth--;
    if( m_StackDepth > 0 )
        m_StackFrame = m_StackFrames[m_StackDepth-1];
}

//==============================================================================
//  Exec
//==============================================================================

void xsc_vm_core::Exec( void )
{
    while( m_StackDepth != 0 )
    {
//        s32 IP = m_StackFrame.IP - m_StackFrame.pModule->m_pMethods;
//        x_printf( "SP = %08X : %s\n", m_StackFrame.SP, (const char*)Disasm( m_StackFrame.pModule, IP ) );
        ExecInstruction();
    }
}

//==============================================================================
//  ExecInstruction
//==============================================================================

void xsc_vm_core::ExecInstruction( void )
{
/*
    s32 IP = m_StackFrame.IP - m_StackFrame.pModule->m_pMethods;
    xstring s = Disasm( m_StackFrame.pModule, IP );
    x_printf( "%s\n", (const char*)s );
*/

    // Read opcode and call execution function
    s32 Opcode = *m_StackFrame.IP++;
    ASSERT( (Opcode >= 0) && (Opcode < vm_last_opcode) );
    void (xsc_vm_core::*pfn)() = ExecData[Opcode].pfn;
    (this->*pfn)( );
}

//==============================================================================
//  Execution Helpers
//==============================================================================

inline
s32 xsc_vm_core::Operand16( void )
{
    s16 v1  = *m_StackFrame.IP++ << 8;
        v1 += *m_StackFrame.IP++;

    return (s32)v1;
/*
    s32 Operand = (s16)(((s16)m_StackFrame.IP[0])<<8) + (s16)(((s16)m_StackFrame.IP[1]));
    m_StackFrame.IP += 2;
    return Operand;
*/
}

inline
s32 xsc_vm_core::Pop_s32( void )
{
    m_StackFrame.SP -= 4;
    s32 Value = *((s32*)(m_StackFrame.SP));
    return Value;
}

inline
f32 xsc_vm_core::Pop_f32( void )
{
    m_StackFrame.SP -= 4;
    f32 Value = *((f32*)(m_StackFrame.SP));
    return Value;
}

inline
void xsc_vm_core::Push_s32( s32 Value )
{
    *((s32*)(m_StackFrame.SP)) = Value;
    m_StackFrame.SP += 4;
}

inline
void xsc_vm_core::Push_f32( f32 Value )
{
    *((f32*)(m_StackFrame.SP)) = Value;
    m_StackFrame.SP += 4;
}

//==============================================================================
//  Instruction Set
//==============================================================================

void xsc_vm_core::Exec_nop          ( void )
{
    ASSERT( 0 );
}

void xsc_vm_core::Exec_assert       ( void )
{
    ASSERT( 0 );
}

void xsc_vm_core::Exec_break        ( void )
{
    ASSERT( 0 );
}

void xsc_vm_core::Exec_ba           ( void )
{
    s32 Offset = Operand16();
    m_StackFrame.IP += Offset;
}

void xsc_vm_core::Exec_bf           ( void )
{
    s32 Offset = Operand16();
    if( !Pop_s32() )
        m_StackFrame.IP += Offset;
}

void xsc_vm_core::Exec_bt           ( void )
{
    s32 Offset = Operand16();
    if( Pop_s32() )
        m_StackFrame.IP += Offset;
}

void xsc_vm_core::Exec_itof         ( void )
{
    Push_f32( (f32)Pop_s32() );
}

void xsc_vm_core::Exec_ftoi         ( void )
{
    Push_s32( (s32)Pop_f32() );
}

void xsc_vm_core::Exec_icmp_eq      ( void )
{
    Push_s32( Pop_s32() == Pop_s32() );
}

void xsc_vm_core::Exec_icmp_ge      ( void )
{
    Push_s32( Pop_s32() <= Pop_s32() );
}

void xsc_vm_core::Exec_icmp_gt      ( void )
{
    Push_s32( Pop_s32() < Pop_s32() );
}

void xsc_vm_core::Exec_icmp_le      ( void )
{
    Push_s32( Pop_s32() >= Pop_s32() );
}

void xsc_vm_core::Exec_icmp_lt      ( void )
{
    Push_s32( Pop_s32() > Pop_s32() );
}

void xsc_vm_core::Exec_icmp_ne      ( void )
{
    Push_s32( Pop_s32() != Pop_s32() );
}

void xsc_vm_core::Exec_fcmp_eq      ( void )
{
    Push_s32( Pop_f32() == Pop_f32() );
}

void xsc_vm_core::Exec_fcmp_ge      ( void )
{
    Push_s32( Pop_f32() <= Pop_f32() );
}

void xsc_vm_core::Exec_fcmp_gt      ( void )
{
    Push_s32( Pop_f32() < Pop_f32() );
}

void xsc_vm_core::Exec_fcmp_le      ( void )
{
    Push_s32( Pop_f32() >= Pop_f32() );
}

void xsc_vm_core::Exec_fcmp_lt      ( void )
{
    Push_s32( Pop_f32() > Pop_f32() );
}

void xsc_vm_core::Exec_fcmp_ne      ( void )
{
    Push_s32( Pop_f32() != Pop_f32() );
}

void xsc_vm_core::Exec_idup         ( void )
{
    Push_s32( *((s32*)(m_StackFrame.SP-4)) );
}

void xsc_vm_core::Exec_fdup         ( void )
{
    Push_f32( *((f32*)(m_StackFrame.SP-4)) );
}

void xsc_vm_core::Exec_cdup         ( void )
{
    ASSERT( 0 );
}

void xsc_vm_core::Exec_invoke       ( void )
{
    // Read MethodRef index
    s32 Index = Operand16();

    // Get MethodDef
    xsc_vm_methoddef* pMethodDef = m_StackFrame.pModule->m_pMethodRef[Index].pMethodDef;
    ASSERT( pMethodDef );
    ASSERT( !(pMethodDef->Flags & XSC_VM_METHOD_NATIVE) );
    ASSERT( !(pMethodDef->Flags & XSC_VM_METHOD_STATIC) );

    // Get data from old stack frame
    s32     ArgByteSize = pMethodDef->ArgumentsSize;
    u8*     pArgs       = m_StackFrame.SP - ArgByteSize;
    s32     pThis       = *(s32*)pArgs;
    u8*     pLocals     = m_StackFrame.SP;
    u8*     pOperands   = pLocals + pMethodDef->LocalSize + 0x80; // TODO: Fix LocalBytesRequired

    // Remove args from old stack frame
    m_StackFrame.SP -= ArgByteSize;

    // Create a stack frame for the method
    PushStackFrame();
    m_StackFrame.pModule    = pMethodDef->pClassDef->pHeader->pModule;
    m_StackFrame.pMethod    = pMethodDef;
    m_StackFrame.pArguments = pArgs;
    m_StackFrame.pLocals    = pLocals;
    m_StackFrame.pOperands  = pOperands;
    m_StackFrame.pThis      = (u8*)pThis;
    m_StackFrame.SP         = pOperands;
    m_StackFrame.IP         = m_StackFrame.pModule->m_pMethods + pMethodDef->MethodOffset;
}

void xsc_vm_core::Exec_invokenative ( void )
{
    // Read MethodRef index
    s32 Index = Operand16();

    // Get MethodDef
    xsc_vm_methoddef* pMethodDef = m_StackFrame.pModule->m_pMethodRef[Index].pMethodDef;
    ASSERT( pMethodDef );
    ASSERT( pMethodDef->Flags & XSC_VM_METHOD_NATIVE );

    nativemethod*   pNativeMethod       = (nativemethod*)pMethodDef->MethodOffset;
    s32             ArgByteSize         = pMethodDef->ArgumentsSize;

    // Call Method
    pNativeMethod->pFn( m_StackFrame.SP-ArgByteSize );

    // Remove args from stack
    m_StackFrame.SP -= ArgByteSize;

    // Move stack to acount for return value
    switch( pNativeMethod->pSignature[0] )
    {
    case 'V':
        break;
    case 'I':
        m_StackFrame.SP += 4;
        break;
    case 'F':
        m_StackFrame.SP += 4;
        break;
    case '&':
        m_StackFrame.SP += 4;
        break;
    default:
        // TODO: Finish return types
        ASSERT( 0 );
    }
}

void xsc_vm_core::Exec_invokestatic ( void )
{
    ASSERT( 0 );
}

void xsc_vm_core::Exec_invokevirtual( void )
{
    ASSERT( 0 );
}

void xsc_vm_core::Exec_this         ( void )
{
    Push_s32( *(s32*)m_StackFrame.pArguments );
}

void xsc_vm_core::Exec_aaddr        ( void )
{
    s32 Offset = Operand16();
    Push_s32( (s32)m_StackFrame.pArguments + Offset );
}

void xsc_vm_core::Exec_laddr        ( void )
{
    s32 Offset = Operand16();
    Push_s32( (s32)m_StackFrame.pLocals + Offset );
}

void xsc_vm_core::Exec_faddr        ( void )
{
    s32 FieldIndex = Operand16();
    Push_s32( Pop_s32() + m_StackFrame.pModule->m_pFieldRef[FieldIndex].pFieldDef->FieldByteOffset );
}

void xsc_vm_core::Exec_iconst       ( void )
{
    s32 Index = Operand16();
    Push_s32( m_StackFrame.pModule->m_pConstInt[Index] );
}

void xsc_vm_core::Exec_fconst       ( void )
{
    s32 Index = Operand16();
    Push_f32( m_StackFrame.pModule->m_pConstFlt[Index] );
}

void xsc_vm_core::Exec_iload        ( void )
{
    Push_s32( *((s32*)Pop_s32()) );
}

void xsc_vm_core::Exec_fload        ( void )
{
    Push_f32( *((f32*)Pop_s32()) );
}

void xsc_vm_core::Exec_cload        ( void )
{
    s32 Index = Operand16();
    s32 Size = m_StackFrame.pModule->m_pClassRef[Index].pClassDef->ByteSize;
    s32 a = Pop_s32();
    x_memcpy( (void*)(m_StackFrame.SP), (void*)a, Size );
    m_StackFrame.SP += Size;
}

void xsc_vm_core::Exec_istore       ( void )
{
    s32 a = Pop_s32();
    *((s32*)a) = *((s32*)(m_StackFrame.SP-4));
}

void xsc_vm_core::Exec_fstore       ( void )
{
    s32 a = Pop_s32();
    *((f32*)a) = *((f32*)(m_StackFrame.SP-4));
}

void xsc_vm_core::Exec_cstore       ( void )
{
    s32 Index = Operand16();
    s32 Size = m_StackFrame.pModule->m_pClassRef[Index].pClassDef->ByteSize;
    s32 a = Pop_s32();
    s32 b = Pop_s32();
    x_memcpy( (void*)a, (void*)b, Size );
}


void xsc_vm_core::Exec_iadd         ( void )
{
    Push_s32( Pop_s32() + Pop_s32() );
}

void xsc_vm_core::Exec_idiv         ( void )
{
    s32 v2 = Pop_s32();
    s32 v1 = Pop_s32();
    Push_s32( v1/v2 );
}

void xsc_vm_core::Exec_imod         ( void )
{
    s32 v2 = Pop_s32();
    s32 v1 = Pop_s32();
    Push_s32( v1%v2 );
}

void xsc_vm_core::Exec_imul         ( void )
{
    s32 v2 = Pop_s32();
    s32 v1 = Pop_s32();
    Push_s32( v1*v2 );
}

void xsc_vm_core::Exec_ineg         ( void )
{
    Push_s32( -Pop_s32() );
}

void xsc_vm_core::Exec_isub         ( void )
{
    s32 v2 = Pop_s32();
    s32 v1 = Pop_s32();
    Push_s32( v1-v2 );
}


void xsc_vm_core::Exec_fadd         ( void )
{
    Push_f32( Pop_f32() + Pop_f32() );
}

void xsc_vm_core::Exec_fdiv         ( void )
{
    f32 v2 = Pop_f32();
    f32 v1 = Pop_f32();
    Push_f32( v1/v2 );
}

void xsc_vm_core::Exec_fmod         ( void )
{
    f32 v2 = Pop_f32();
    f32 v1 = Pop_f32();
    Push_f32( x_fmod(v1,v2) );
}

void xsc_vm_core::Exec_fmul         ( void )
{
    f32 v2 = Pop_f32();
    f32 v1 = Pop_f32();
    Push_f32( v1*v2 );
}

void xsc_vm_core::Exec_fneg         ( void )
{
    Push_f32( -Pop_f32() );
}

void xsc_vm_core::Exec_fsub         ( void )
{
    f32 v2 = Pop_f32();
    f32 v1 = Pop_f32();
    Push_f32( v1-v2 );
}


void xsc_vm_core::Exec_bit_and      ( void )
{
    Push_s32( Pop_s32() & Pop_s32() );
}

void xsc_vm_core::Exec_bit_or       ( void )
{
    Push_s32( Pop_s32() | Pop_s32() );
}

void xsc_vm_core::Exec_log_and      ( void )
{
    Push_s32( Pop_s32() && Pop_s32() );
}

void xsc_vm_core::Exec_log_or       ( void )
{
    Push_s32( Pop_s32() || Pop_s32() );
}

void xsc_vm_core::Exec_not          ( void )
{
    Push_s32( !Pop_s32() );
}

void xsc_vm_core::Exec_shl          ( void )
{
    s32 v2 = Pop_s32();
    s32 v1 = Pop_s32();
    Push_s32( v1<<v2 );
}

void xsc_vm_core::Exec_shr          ( void )
{
    s32 v2 = Pop_s32();
    s32 v1 = Pop_s32();
    Push_s32( v1>>v2 );
}

void xsc_vm_core::Exec_xor          ( void )
{
    Push_s32( Pop_s32() ^ Pop_s32() );
}


void xsc_vm_core::Exec_pop          ( void )
{
    s32 Count = Operand16();
    m_StackFrame.SP -= Count;
}


void xsc_vm_core::Exec_vret         ( void )
{
    s32 ArgsSize = m_StackFrame.pMethod->ArgumentsSize;
    (void)ArgsSize;
    PopStackFrame();
}

void xsc_vm_core::Exec_iret         ( void )
{
    s32 RetVal = *(s32*)(m_StackFrame.SP-4);

    PopStackFrame();
    Push_s32( RetVal );
}

void xsc_vm_core::Exec_fret         ( void )
{
    f32 RetVal = *(f32*)(m_StackFrame.SP-4);

    PopStackFrame();
    Push_f32( RetVal );
}

void xsc_vm_core::Exec_cret         ( void )
{
    s32     Index   = Operand16();
    s32     Size    = m_StackFrame.pModule->m_pClassRef[Index].pClassDef->ByteSize;
    void*   pRetSrc = (void*)(m_StackFrame.SP-Size);
    PopStackFrame();
    x_memmove( (void*)m_StackFrame.SP, pRetSrc, Size );
    m_StackFrame.SP += Size;
}

void xsc_vm_core::Exec_rret         ( void )
{
    ASSERT( 0 );
}

//==============================================================================
