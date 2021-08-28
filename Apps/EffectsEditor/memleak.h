#ifndef __MEMLEAK_H
#define __MEMLEAK_H

#include <crtdbg.h>


class CMemDebug
{

public:
    CMemDebug();
    ~CMemDebug();

    void    Go      ( void )    {}           // just to prevent linker from throwing it all away
    
};


extern CMemDebug __MemDebug;




#endif