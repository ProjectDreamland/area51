#include "mss.h"
#include "stdlib.h"

static void __cdecl MSS_cleanup(void)
{
  AIL_shutdown();
};

int __cdecl MSS_auto_cleanup(void)
{
  atexit(MSS_cleanup);
  return(0);
}