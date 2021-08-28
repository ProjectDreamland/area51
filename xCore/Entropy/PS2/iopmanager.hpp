#ifndef __IOPMANAGER_HPP
#define __IOPMANAGER_HPP

#include "x_target.hpp"
#include "x_types.hpp"
#include "x_threads.hpp"

#if !defined(X_SECTION)
#define X_SECTION(x) __attribute__((section("."#x)))
#endif
#if defined(TARGET_PS2_IOP) || !defined(TARGET_PS2)
#error "This should only be compiled for EE side IOP support. Check your dependencies"
#endif

#include "sifdev.h"

class iop_manager
{
public:
        void                Init            ( void ) X_SECTION(ctors);
        void                Kill            ( void ) X_SECTION(dtors);
        void                Update          ( void ) X_SECTION(update);
        s32                 LoadModule      ( const char* pFilename,const char* Params=NULL, s32 ParamLength=0, xbool AllowFail=FALSE ) X_SECTION(update);
        void                UnloadModule    ( s32 id )  X_SECTION(update);
        void                Reboot          ( xbool LoadModules = TRUE, xbool ShowSplash = FALSE, const char* pKernelName = IOP_IMAGE_FILE );
private:
        const char*         KernelError     ( s32 ErrorCode );
        void                FindFile        ( char* pResult, const char* pFilename );

        xthread*            m_pPoweroffThread;
        xmutex              m_Lock;
};

extern iop_manager g_IopManager;

#endif