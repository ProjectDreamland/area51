#ifndef __LADDER_FIELD_HPP__
#define __LADDER_FIELD_HPP__

#include "..\Object.hpp"




class ladder_field : public object
{
public:

    enum ladder_states
    {
        STATE_INVALID = -1,
        STATE_WAITING_TO_BE_MOUNTED,
        STATE_MOUNTED,
        STATE_MAX
    };
    
    struct collision
    {
        struct quad
        {
            vector3 m_Pos[4] ;
        } ;

        quad    m_FrontQuad ;
        quad    m_TopQuad ;
        quad    m_BackQuad ;
    } ;

public:
    // Run Time Type info.
    CREATE_RTTI( ladder_field, object, object );

    // Construction / Destruction
	                ladder_field();
    virtual         ~ladder_field();

    // object description.
    virtual const   object_desc&  GetTypeDesc           ( void ) const;
    static  const   object_desc&  GetObjectType         ( void );
    
    // Object overloads
    virtual         s32             GetMaterial             ( void ) const { return MAT_TYPE_CONCRETE;}
	virtual         void	        OnEnumProp			    ( prop_enum& rList );
	virtual			xbool	        OnProperty			    ( prop_query& rPropQuery );

#ifndef X_RETAIL
    virtual         void            OnDebugRender           ( void );
    virtual         void            OnColRender             ( xbool bRenderHigh );
#endif // X_RETAIL

    virtual         bbox            GetLocalBBox            ( void ) const;
                    void            ComputeCollision        ( collision& Collision, f32 CylinderDiameter, f32 CylinderHeight ) ;
    virtual         void            OnColCheck              ( void );
    virtual         void            OnColNotify             ( object& rObject );
    virtual         void            OnInit                  ( void );
    virtual void                    OnPolyCacheGather       ( void );
                                    
    // Ladder field utilities.      
                    xbool           DoesCylinderIntersect   ( const vector3& Pos, f32 Height, f32 Radius ) const ;
                    const vector3   GetDimensions           ( void ) const ;
                    f32             GetTop                  ( void ) const ;
                    f32             GetBottom               ( void ) const ;

                    void            RenderLadderField       ( void );

protected:
    vector3         m_Dimensions;
};



#endif