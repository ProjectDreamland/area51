//==============================================================================
//  
//  e_Singleton.hpp -- singleton interface
//
//==============================================================================

#ifndef _SINGLETON_HPP_
#define _SINGLETON_HPP_

    template< class Type >struct singleton_t
    {
        static void Activate( void );
        static void Release ( void );
        static Type* me;
    };

#endif
