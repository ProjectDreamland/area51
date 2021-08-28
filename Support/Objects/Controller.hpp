#ifndef __CONTROLLER_HPP__
#define __CONTROLLER_HPP__

//=========================================================================
// INCLUDES
//=========================================================================

#include "Entropy.hpp"
#include "Tracker.hpp"

//=========================================================================
// CLASS
//=========================================================================

class controller : public tracker
{
//=====================================================================
// DEFINES
//=====================================================================
public:

    // Defines
    enum defines
    {
        MAX_OBJECTS = 4
    } ;

//=====================================================================
// PUBLIC BASE CLASS FUNCTIONS
//=====================================================================
public:

    CREATE_RTTI( controller, tracker, object )
    
                            controller      ( void );
    virtual s32             GetMaterial     ( void ) const { return MAT_TYPE_FLESH; }
    virtual void            OnEnumProp      ( prop_enum& List );
    virtual xbool           OnProperty      ( prop_query& I );
    virtual void            OnMove          ( const vector3& NewPos   );        
    virtual void            OnTransform     ( const matrix4& L2W      ); 

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//=====================================================================
// PRIVATE BASE CLASS FUNCTIONS
//=====================================================================
protected:

    struct object_info
    {
        object_info(void) { Guid=0; bPivot=FALSE; Pivot.Identity(); }
        guid        Guid;
        xbool       bPivot;
        matrix4     Pivot;
    };


protected:
    virtual void            OnInit              ( void );     
    virtual void            OnAdvanceLogic	    ( f32 DeltaTime );
            void            MoveObject          ( s32 Index, const matrix4& L2W ) const;

//=====================================================================
// DATA
//=====================================================================

protected:

    object_info     m_ObjectInfo[MAX_OBJECTS] ;    // List of objects to track
};

//=========================================================================
// END
//=========================================================================
#endif
