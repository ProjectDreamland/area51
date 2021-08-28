#include "x_types.hpp"

#define EXCEPT_REGISTER_COUNT   45          // Includes GPR and IOP extra registers
#define EXCEPT_STACK_SIZE       32
#define EXCEPT_LOG_PORT         49152
#define EXCEPT_LOG_SERVER       "216.201.187.178"
//#define EXCEPT_LOG_SERVER       "10.0.1.161"

typedef struct s_iop_exception
{
      u32   gpr[EXCEPT_REGISTER_COUNT];
      u32   version;
      u32   offset;
      char  module[32];
} iop_exception;

typedef enum
{
    EXCEPT_IOP,
    EXCEPT_EE,
    EXCEPT_ASSERT,
    EXCEPT_LOG,
} exception_type;

typedef struct s_ee_exception
{
    u32     stat;
    u32     cause;
    u32     epc;
    u32     bva;
    u32     bpa;
    u32     gpr[EXCEPT_REGISTER_COUNT];
    u32     RawStack[EXCEPT_STACK_SIZE];
    u32     CookedStack[EXCEPT_STACK_SIZE];
} ee_exception;

typedef struct s_assert_exception
{
    s32     LineNumber;
    u32     StackPointer;
    char    Filename[128];
    char    Expression[128];
    char    Message[128];
    u32     RawStack[EXCEPT_STACK_SIZE];
    u32     CookedStack[EXCEPT_STACK_SIZE];
} assert_exception;

typedef struct s_log_exception
{
    char    Filename[128];
    u32     LineNumber;
    char    LogMessage[256];
} log_exception;

typedef struct s_exception_packet
{
    exception_type          Type;
    u32                     Time[4];    // hh:mm:ss:ms
    char                    DebugVersion[64];
    char                    DebugMessage[64];
    u32                     LocalIP;
    union
    {
        iop_exception       Iop;
        ee_exception        EE;
        assert_exception    Assert;
        log_exception       Log;
    };
} exception_packet;

enum
{
    EXCEPT_REQ_FIND   = 0x2000,
    EXCEPT_REQ_WRITE,
};

typedef struct s_except_write
{
    s32 Type;
    s32 Sequence;
    exception_packet Data;
} except_write;

typedef struct s_except_find
{
    s32     Type;
    s32 Sequence;
    s32     Address;
    s32     Port;
} except_find;

//-----------------------------------------------------------------------------
// Exception handler replies
//-----------------------------------------------------------------------------
enum
{
    EXCEPT_ERR_OK = 0,
    EXCEPT_ERR_ERROR
};

typedef struct s_except_find_reply
{
    s32     Address;
    s32     Port;
} except_find_reply;

typedef struct s_except_write_reply
{
    s32     Status;                     // Status MUST always be the first field
    s32     Length;
} except_write_reply;


typedef struct s_except_request
{
    union
    {
        except_find     Find;
        except_write    Write;
    };
} except_request;

typedef struct s_except_reply
{
    s32 Sequence;
    union
    {
        except_find_reply Find;
        except_write_reply Write;
    };
} except_reply;