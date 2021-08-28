#ifndef LEVEL_HPP
#define LEVEL_HPP

#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Parsing\TextIn.hpp"
#include "Parsing\TextOut.hpp"

//=============================================================================

class level
{
public:

                level           ( void );
               ~level           ( void );
               
    void        Open            ( const char* pFileName, xbool DoLoad = TRUE );
    void        Close           ( void );
    void        Load            ( void );
    void        Save            ( guid Guid );
    void        SetRigidColor   ( const char* pFileName );

protected:

    xbool       m_DoLoad;
    text_out    m_TextOut;
    text_in     m_TextIn;
};

//=============================================================================


#endif
