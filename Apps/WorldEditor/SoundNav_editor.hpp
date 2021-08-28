///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SoundNav_editor.hpp
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef SOUNDNAV_EDITOR_HPP
#define SOUNDNAV_EDITOR_HPP

#include "..\WorldEditor\EditorDoc.h"
class soundnav_editor
{
public:
    soundnav_editor(void) : m_FirstSelectedObject(-1) { s_This = this; }
    ~soundnav_editor() { s_This = NULL;   }

    static soundnav_editor*  GetSoundSoundEditor(void) { if(!s_This) new soundnav_editor; return s_This; }

    xbool IsObjectNavNodePlaceHolder(guid aGuid );

    xbool NavNodePlaceHolderCTRLSelected(guid aGuid  );

    void  Render(void);
    void  ResetNavNodePlaceHolder( void );

    void  LoadNavMap( const char* fileName );
    void  SaveNavMap( const char* fileName );

    guid  CreateSoundNode( void );
    guid  CreateReciverNode( void );
    

    void SetNavTestStart( guid thisNode );
    void SetNavTestEnd  ( guid thisNode );

    void CalcPath(void);

    void PrintDebugString(const char *thisString) { OutputDebugString(thisString); }
    

protected:
    
    static soundnav_editor*  s_This;

    s16    m_FirstSelectedObject;

};


#endif//SOUNDNAV_EDITOR_HPP
 
