#ifndef __SPAWNEROBJECT_HPP__
#define __SPAWNEROBJECT_HPP__

#include "..\Object.hpp"
#include "MiscUtils\SimpleUtils.hpp"
//#include "..\Support\Globals\Global_Variables_Manager.hpp"

class spawner_object : public object
{
public:

    // Run Time Type info.
    CREATE_RTTI( spawner_object, object, object );

    // Construction / Destruction
	                spawner_object();
    virtual         ~spawner_object();

    // object description.
    virtual const   object_desc&    GetTypeDesc         ( void ) const;
    static  const   object_desc&    GetObjectType       ( void );
    
    // Object overloads
    virtual         void            OnInit              ( void );
    virtual         s32             GetMaterial         ( void ) const { return MAT_TYPE_CONCRETE;}
	virtual         void	        OnEnumProp			( prop_enum& rList );
	virtual			xbool	        OnProperty			( prop_query& rPropQuery );

#ifndef X_RETAIL
                    void            OnDebugRender        ( void );
#endif // X_RETAIL

    virtual         void            OnAdvanceLogic      ( f32 DeltaTime );
    virtual         bbox            GetLocalBBox        ( void ) const{ return bbox( vector3(0,50,0), 50 ); }
    virtual         void            OnActivate          ( xbool Flag );            

    // Called when the spawned object has been killed.
                    void            OnSpawnedObjectKill ( object* pObject );
                    
    // Actually spawn the object.                    
                    void            SpawnObject         ( void );


    // Need to maintain the dictionary.
#ifdef X_EDITOR
    virtual         void                EditorPreGame   ( void );
#endif // X_EDITOR

protected:
//    global_var_interface    m_GVarInterface;
    s32                     m_MaxInWorldAtATime;
    s32                     m_MaxAbleToCreate;
    xbool                   m_bActive;
    f32                     m_fDelayBetweenSpawns;
    f32                     m_Timer;

    s32                     m_TotalObjectsSpawned;
    s32                     m_CurrentAliveSpawnedObjects;
    s32                     m_TemplateID;    

    guid                    m_ActivateObject;                           // Guid of object to activate.
    guid                    m_SpawnGroup;
};



#endif