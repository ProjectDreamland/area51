//==============================================================================
//
//  xsc_vm_disasm.cpp
//
//==============================================================================

#include "xsc_vm_core.hpp"
#include "xsc_vm_module.hpp"
#include "xsc_vm_instructions.hpp"

//==============================================================================
//  Defines
//==============================================================================

struct disasm_data
{
    s32             Opcode;
    const char*     Instruction;
    s32             Args;
};

enum disasm_args
{
    DA_NULL,
    DA_OFFSET,
    DA_COUNT,
    DA_METHODREF,
    DA_CLASSREF,
    DA_FIELDREF,
    DA_CONST_INT,
    DA_CONST_FLT,
};

disasm_data DisasmData[] =
{
    { vm_nop,           "nop",              0               },  // no operation                         1 byte
    { vm_assert,        "assert",           0               },  // assert exception                     1 byte
    { vm_break,         "break",            0               },  // break exception                      1 byte

    { vm_ba,            "ba",               DA_OFFSET       },  // Branch Always                        3 bytes (Relative offset)
    { vm_bf,            "bf",               DA_OFFSET       },  // Branch False                         3 bytes (Relative offset)
    { vm_bt,            "bt",               DA_OFFSET       },  // Branch True                          3 bytes (Relative offset)
    
    { vm_itof,          "itof",             0               },  // cast int to float                    1 byte
    { vm_ftoi,          "ftoi",             0               },  // cast float to int                    1 byte

    { vm_icmp_eq,       "icmp_eq",          0               },  // compare equal int                    1 byte
    { vm_icmp_ge,       "icmp_ge",          0               },  // compare greater or equal int         1 byte
    { vm_icmp_gt,       "icmp_gt",          0               },  // compare greater int                  1 byte
    { vm_icmp_le,       "icmp_le",          0               },  // compare less or equal int            1 byte
    { vm_icmp_lt,       "icmp_lt",          0               },  // compare less than int                1 byte
    { vm_icmp_ne,       "icmp_ne",          0               },  // compare not equal int                1 byte

    { vm_fcmp_eq,       "fcmp_eq",          0               },  // compare equal float                  1 byte
    { vm_fcmp_ge,       "fcmp_ge",          0               },  // compare greater or equal float       1 byte
    { vm_fcmp_gt,       "fcmp_gt",          0               },  // compare greater float                1 byte
    { vm_fcmp_le,       "fcmp_le",          0               },  // compare less or equal float          1 byte
    { vm_fcmp_lt,       "fcmp_lt",          0               },  // compare less than float              1 byte
    { vm_fcmp_ne,       "fcmp_ne",          0               },  // compare not equal float              1 byte

    { vm_idup,          "idup",             0               },  // Duplicate top stack item int         1 byte
    { vm_fdup,          "fdup",             0               },  // Duplicate top stack item float       1 byte
    { vm_cdup,          "cdup",             DA_CLASSREF     },  // Duplicate top stack item class       3 bytes (ClassRef index)

    { vm_invoke,        "invoke",           DA_METHODREF    },  // Invoke instance method               3 bytes (MethodRef index)
    { vm_invokenative,  "invokenative",     DA_METHODREF    },  // Invoke native method                 3 bytes (MethodRef index)
    { vm_invokestatic,  "invokestatic",     DA_METHODREF    },  // Invoke static method                 3 bytes (MethodRef index)
    { vm_invokevirtual, "invokevirtual",    DA_METHODREF    },  // Invoke virtual method                3 bytes (MethodRef index)

    { vm_this,          "this",             0               },  // Load this pointer                    1 byte
    { vm_aaddr,         "aaddr",            DA_COUNT        },  // Load argument address                3 bytes (Argument byte index)
    { vm_laddr,         "laddr",            DA_COUNT        },  // Load local address                   3 bytes (Local byte index)
    { vm_faddr,         "faddr",            DA_FIELDREF     },  // Add field address to top of stack    3 bytes (FieldRef index)

    { vm_iconst,        "iconst",           DA_CONST_INT    },  // Load Const Int                       3 bytes (Const index)
    { vm_fconst,        "fconst",           DA_CONST_FLT    },  // Load Const Float                     3 bytes (Const index)
    { vm_iload,         "iload",            0               },  // Load Int from address                1 byte
    { vm_fload,         "fload",            0               },  // Load Flt from address                1 byte
    { vm_cload,         "cload",            DA_CLASSREF     },  // Load Class from address              3 bytes (ClassRef index)
    { vm_istore,        "istore",           0               },  // Store Int to address                 1 byte
    { vm_fstore,        "fstore",           0               },  // Store Flt to address                 1 byte
    { vm_cstore,        "cstore",           DA_CLASSREF     },  // Store Class to address               3 bytes (ClassRef index)

    { vm_iadd,          "iadd",             0               },  // Add top 2 stack Int                  1 byte
    { vm_idiv,          "idiv",             0               },  // Div top 2 stack Int                  1 byte
    { vm_imod,          "imod",             0               },  // Mod top 2 stack Int                  1 byte
    { vm_imul,          "imul",             0               },  // Mul top 2 stack Int                  1 byte
    { vm_ineg,          "ineg",             0               },  // Neg top stack Int                    1 byte
    { vm_isub,          "isub",             0               },  // Sub top 2 stack Int                  1 byte

    { vm_fadd,          "fadd",             0               },  // Add top 2 stack Float                1 byte
    { vm_fdiv,          "fdiv",             0               },  // Div top 2 stack Float                1 byte
    { vm_fmod,          "fmod",             0               },  // Mod top 2 stack Float                1 byte
    { vm_fmul,          "fmul",             0               },  // Mul top 2 stack Float                1 byte
    { vm_fneg,          "fneg",             0               },  // Neg top stack Float                  1 byte
    { vm_fsub,          "fsub",             0               },  // Sub top 2 stack Float                1 byte

    { vm_bit_and,       "bit_and",          0               },  // Bitwise And top 2 stack Int          1 byte
    { vm_bit_or,        "bit_or",           0               },  // Bitwise Or top 2 stack Int           1 byte
    { vm_log_and,       "log_and",          0               },  // Logical And top 2 stack Int          1 byte
    { vm_log_or,        "log_or",           0               },  // Logical Or top 2 stack Int           1 byte
    { vm_not,           "not",              0               },  // Not top stack Int                    1 byte
    { vm_shl,           "shl",              0               },  // Shift Left                           1 byte
    { vm_shr,           "shr",              0               },  // Shift Right                          1 byte
    { vm_xor,           "xor",              0               },  // Xor top 2 stack Int                  1 byte

    { vm_pop,           "pop",              DA_COUNT        },  // Pop n bytes from stack               3 bytes (Number of bytes)
    
    { vm_vret,          "vret",             0               },  // Return Void                          1 byte
    { vm_iret,          "iret",             0               },  // Return Int                           1 byte
    { vm_fret,          "fret",             0               },  // Return Flt                           1 byte
    { vm_cret,          "cret",             DA_CLASSREF     },  // Return Class                         3 bytes (ClassRef index)
    { vm_rret,          "rret",             0               },  // Return Reference                     1 byte
};

//==============================================================================
//  ValidateDisasmTable
//==============================================================================

void xsc_vm_core::ValidateDisasmTable( void ) const
{
    // Loop through table validating entries
    for( s32 i=0 ; i<(s32)((sizeof(DisasmData)/sizeof(disasm_data))) ; i++ )
    {
        ASSERT( DisasmData[i].Opcode == i );
    }
}

//==============================================================================
//  Disasm
//==============================================================================

xstring xsc_vm_core::Disasm( const xsc_vm_module* pModule, s32& IP )
{
    ASSERT( (IP >= 0) && (IP < pModule->m_pHeader->MethodsBytes) );

    xstring s;
    const u8*       pMethod = pModule->m_pMethods;
    s32             Opcode  = (s32)pMethod[IP];
    ASSERT( (Opcode >= vm_nop) && (Opcode < vm_last_opcode) );
    s32             Operand;
    disasm_data&    dd      = DisasmData[Opcode];

    s.AddFormat( "%06d  %-15s", IP, dd.Instruction );
    IP++;
    switch( dd.Args )
    {
    case DA_OFFSET:
        Operand = (s16)(((s16)pMethod[IP+0])<<8) + (s16)(((s16)pMethod[IP+1]));
        IP += 2;
        s.AddFormat( "%d (%d)", Operand, IP+Operand );
        break;
    case DA_COUNT:
        Operand = (((s32)pMethod[IP+0])<<8) + (((s32)pMethod[IP+1]));
        IP += 2;
        s.AddFormat( "%d", Operand );
        break;
    case DA_METHODREF:
        Operand = (((s32)pMethod[IP+0])<<8) + (((s32)pMethod[IP+1]));
        IP += 2;
        s.AddFormat( "%d (%s.%s)", Operand, &pModule->m_pConstStr[pModule->m_pMethodRef[Operand].ClassNameOffset],
                     &pModule->m_pConstStr[pModule->m_pMethodRef[Operand].MethodNameOffset] );
        break;
    case DA_CLASSREF:
        Operand = (((s32)pMethod[IP+0])<<8) + (((s32)pMethod[IP+1]));
        IP += 2;
        s.AddFormat( "%d (%s)", Operand, &pModule->m_pConstStr[pModule->m_pClassRef[Operand].ClassNameOffset] );
        break;
    case DA_FIELDREF:
        Operand = (((s32)pMethod[IP+0])<<8) + (((s32)pMethod[IP+1]));
        IP += 2;
        s.AddFormat( "%d (%s)", Operand, &pModule->m_pConstStr[pModule->m_pFieldRef[Operand].FieldNameOffset] );
        break;
    case DA_CONST_INT:
        Operand = (((s32)pMethod[IP+0])<<8) + (((s32)pMethod[IP+1]));
        IP += 2;
        s.AddFormat( "%d (%d)", Operand, pModule->m_pConstInt[Operand] );
        break;
    case DA_CONST_FLT:
        Operand = (((s32)pMethod[IP+0])<<8) + (((s32)pMethod[IP+1]));
        IP += 2;
        s.AddFormat( "%d (%f)", Operand, pModule->m_pConstFlt[Operand] );
        break;
    }

    // Return disassembled line
    return s;
}

//==============================================================================
