//==============================================================================
//  
//  xsc_vm_instructions.hpp
//  
//==============================================================================

#ifndef XSC_VM_INSTRUCTIONS_HPP
#define XSC_VM_INSTRUCTIONS_HPP

//==============================================================================
//  Instructions
//==============================================================================

enum vm_instruction
{
    vm_nop,                     // no operation                     1 byte
    vm_assert,                  // assert exception                 1 byte
    vm_break,                   // break exception                  1 byte

    vm_ba,                      // Branch Always                    3 bytes (Relative offset)
    vm_bf,                      // Branch False                     3 bytes (Relative offset)
    vm_bt,                      // Branch True                      3 bytes (Relative offset)

    vm_itof,                    // cast int to float                1 byte
    vm_ftoi,                    // cast float to int                1 byte

    vm_icmp_eq,                 // compare equal int                1 byte
    vm_icmp_ge,                 // compare greater or equal int     1 byte
    vm_icmp_gt,                 // compare greater int              1 byte
    vm_icmp_le,                 // compare less or equal int        1 byte
    vm_icmp_lt,                 // compare less than int            1 byte
    vm_icmp_ne,                 // compare not equal int            1 byte

    vm_fcmp_eq,                 // compare equal float              1 byte
    vm_fcmp_ge,                 // compare greater or equal float   1 byte
    vm_fcmp_gt,                 // compare greater float            1 byte
    vm_fcmp_le,                 // compare less or equal float      1 byte
    vm_fcmp_lt,                 // compare less than float          1 byte
    vm_fcmp_ne,                 // compare not equal float          1 byte

    vm_idup,                    // Duplicate top stack item int     1 byte
    vm_fdup,                    // Duplicate top stack item float   1 byte
    vm_cdup,                    // Duplicate top stack item class   3 bytes (ClassRef index)

    vm_invoke,                  // Invoke instance method           3 bytes (MethodRef index)
    vm_invokenative,            // Invoke native method             3 bytes (MethodRef index)
    vm_invokestatic,            // Invoke static method             3 bytes (MethodRef index)
    vm_invokevirtual,           // Invoke virtual method            3 bytes (MethodRef index)

    vm_this,                    // Load this pointer                1 byte
    vm_aaddr,                   // Load argument address            3 bytes (Argument byte index)
    vm_laddr,                   // Load local address               3 bytes (Local byte index)
	vm_faddr,					// Add field address				3 bytes (FieldRef index)

    vm_iconst,                  // Load Int Constant                3 bytes (Const index)
    vm_fconst,                  // Load Flt Constant                3 bytes (Const index)
    vm_iload,                   // Load Int from address on stack   1 byte
    vm_fload,                   // Load Flt from address on stack   1 byte
    vm_cload,                   // Load Class from address on stack 3 bytes (ClassRef index)
    vm_istore,                  // Store Int to address on stack    1 byte
    vm_fstore,                  // Store Flt to address on stack    1 byte
    vm_cstore,                  // Store Class to address on stack  3 bytes (ClassRef index)

    vm_iadd,                    // Add top 2 stack Int              1 byte
    vm_idiv,                    // Div top 2 stack Int              1 byte
    vm_imod,                    // Mod top 2 stack Int              1 byte
    vm_imul,                    // Mul top 2 stack Int              1 byte
    vm_ineg,                    // Neg top stack Int                1 byte
    vm_isub,                    // Sub top 2 stack Int              1 byte

    vm_fadd,                    // Add top 2 stack Int              1 byte
    vm_fdiv,                    // Div top 2 stack Int              1 byte
    vm_fmod,                    // Mod top 2 stack Int              1 byte
    vm_fmul,                    // Mul top 2 stack Int              1 byte
    vm_fneg,                    // Neg top stack Int                1 byte
    vm_fsub,                    // Sub top 2 stack Int              1 byte

    vm_bit_and,                 // Bitwise And top 2 stack Int      1 byte
    vm_bit_or,                  // Bitwise Or top 2 stack Int       1 byte
    vm_log_and,                 // Logical And top 2 stack Int      1 byte
    vm_log_or,                  // Logical Or top 2 stack Int       1 byte
    vm_not,                     // Not top stack Int                1 byte
    vm_shl,                     // Shift Left                       1 byte
    vm_shr,                     // Shift Right                      1 byte
    vm_xor,                     // Xor top 2 stack Int              1 byte

    vm_pop,                     // Pop n bytes from stack           3 bytes (Number of bytes)

    vm_vret,                    // Return Void                      1 byte
    vm_iret,                    // Return Int                       1 byte
    vm_fret,                    // Return Flt                       1 byte
    vm_cret,                    // Return Class                     3 bytes (ClassRef index)
    vm_rret,                    // Return Reference                 1 byte

    vm_last_opcode              // Marker
};

//==============================================================================
#endif // XSC_VM_INSTRUCTIONS_HPP
//==============================================================================
