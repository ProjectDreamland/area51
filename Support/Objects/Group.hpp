#ifndef __GROUP_HPP__
#define __GROUP_HPP__

//=========================================================================
// INCLUDES
//=========================================================================

#include "Object.hpp"
#include "Obj_mgr\Obj_mgr.hpp"
#include "TriggerEx\Affecters\object_affecter.hpp"

#define GROUP_MAX_CHILDREN              100

//=========================================================================
// CLASS
//=========================================================================

class group : public object
{
    //=====================================================================
    // DEFINES
    //=====================================================================
public:


    //=====================================================================
    // PUBLIC BASE CLASS FUNCTIONS
    //=====================================================================
public:

    CREATE_RTTI( group, object, object )

             group          ( void );
    virtual ~group          ( void );
    virtual bbox            GetLocalBBox    ( void ) const { return bbox( vector3(0,0,0), 100 ); }
    virtual s32             GetMaterial     ( void ) const { return MAT_TYPE_NULL; }
    virtual void            OnEnumProp      ( prop_enum& List );
    virtual xbool           OnProperty      ( prop_query& I );

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    //=====================================================================
    // PUBLIC INTERNAL FUNCTIONS
    //
    //  AddGuid - returns TRUE if guid was either added, or already existed
    //            returns FALSE if the guid needed to be added, but the group is full
    //
    //  RemoveGuid - returns TRUE if the guid was removed
    //               returns FALSE if the guid was not in the group
    //
    //=====================================================================
public:
            xbool           AddGuid             ( guid ChildGuid );                            
            xbool           RemoveGuid          ( guid ChildGuid );
            xbool           ContainsGuid        ( guid ChileGuid );

    //TODO xbool           AddGlobal           ( const char* pName );
    //TODO xbool           RemoveGlobal        ( const char* pName );

            s32             GetNumChildren      (             object::type Type = object::TYPE_NULL );
            guid            GetChild            ( s32 iChild, object::type Type = object::TYPE_NULL );
            guid            GetRandomChild      (             object::type Type = object::TYPE_NULL );

    virtual void            OnActivate          ( xbool Flag );  
    virtual void            OnPain              ( const pain& Pain ) ;  // Tells object to recieve pain

    //=====================================================================
    // PRIVATE BASE CLASS FUNCTIONS
    //=====================================================================
protected:
    virtual void            OnInit              ( void ); 
    

#ifndef X_RETAIL
    virtual void            OnDebugRender       ( void );
#endif // X_RETAIL

    //=====================================================================
    // PRIVATE INTERNAL FUNCTIONS
    //=====================================================================
protected:
            const char*     GetObjectName       ( guid gObj );
            void            ValidateChildren    ( void );
            void            DestroyAll          ( void );
            xbool           HasLivingActors     ( void );
    //=====================================================================
    // DATA
    //=====================================================================
protected:

    struct child 
    {
        // A child can be referred to by either a global variable
        // or a static variable
        //
        //  This is the hastily written v1.0, so we'll cache the
        //  object type in the child struct to make the brute-force
        //  type sensitive inquiries a little less brute.
        //
        object_affecter         m_Object;
        object::type            m_Type;         
    };


    xarray<child>       m_Child;
};

//=========================================================================
// END
//=========================================================================
#endif
