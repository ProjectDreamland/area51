///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  notepad_object.hpp
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef notepad_object_hpp
#define notepad_object_hpp

///////////////////////////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Obj_mgr\obj_mgr.hpp"
#include "x_types.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////
// CLASS
///////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_NOTE_LENGTH     256

class notepad_object : public object
{
public:

    CREATE_RTTI( notepad_object, object, object )

                            notepad_object        ( void );
                            ~notepad_object       ( void );
                            
    virtual         bbox    GetLocalBBox    ( void ) const;
    virtual         s32     GetMaterial     ( void ) const { return MAT_TYPE_NULL; }

    virtual         void    OnEnumProp      ( prop_enum& List );
    virtual         xbool   OnProperty      ( prop_query&        I    );
    virtual         void    OnMove( const vector3& newPos );

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

protected:

    virtual         void    OnInit          ( void );
    virtual         void    OnRender        ( void );
    virtual         void    OnImport        ( text_in& TextIn );

protected:
    char                m_Note[MAX_NOTE_LENGTH+1];   
    s32                 m_nCurrentLength;
    xcolor              m_crText;
    xcolor              m_crNote;
    s32                 m_nCharsPerLine;
    f32                 m_fMaxRenderDist;              
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////////////////////////////
#endif//notepad_object_hpp