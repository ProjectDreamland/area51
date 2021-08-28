//////////////////////////////////////////////////////////////////
//
// IOPLIBSN.H
//
// Contains definitions for global constructor/destructor code,
//

#ifdef __cplusplus
extern "C" {
#endif

#define SN_CALL_CTORS asm ("jal __do_global_ctors")
#define SN_CALL_DTORS asm ("jal __do_global_dtors")

void _exit(int);

void exit(int);

#ifdef __cplusplus
}
#endif