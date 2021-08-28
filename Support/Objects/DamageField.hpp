#ifndef damage_field_HPP
#define damage_field_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"
#include "..\Support\TriggerEx\Affecters\object_affecter.hpp"

//=========================================================================
// CLASS
//=========================================================================
class damage_field : public object
{
public:

    enum damage_field_targets
    {
        DF_TARGET_NONE            = 0,
        DF_TARGET_PLAYER          = BIT( 0 ),    
        DF_TARGET_CHARACTERS      = BIT( 1 ),    
        DF_TARGET_PROPS           = BIT( 2 ),    
        INVALID_DF_TARGET         = -1
    } ;

    enum damage_field_spatial_types
    {
        SPATIAL_TYPES_INVALID = -1,
        SPATIAL_TYPE_AXIS_CUBE,
        SPATIAL_TYPE_SPHERICAL,
        SPATIAL_TYPES_END
    };

    CREATE_RTTI( damage_field, object, object )
    
                                damage_field        ( void );
    virtual bbox                GetLocalBBox        ( void ) const;
    virtual s32                 GetMaterial         ( void ) const { return MAT_TYPE_NULL; }
    virtual void                OnEnumProp          ( prop_enum& List );
    virtual xbool               OnProperty          ( prop_query& I );

    virtual const object_desc&  GetTypeDesc         ( void ) const;
    static  const object_desc&  GetObjectType       ( void );

#if !defined( CONFIG_RETAIL )
            void                OnRenderSpatial     ( void );
#endif // !defined( CONFIG_RETAIL )

	virtual			void	    OnColCheck			( void );
    virtual         void        OnColNotify         ( object& Object );
    virtual         void        OnActivate          ( xbool Flag );  
    virtual         void        OnAdvanceLogic      ( f32 DeltaTime );      
    
    xbool           m_DrawActivationIcon;                       // Debug functionality

    damage_field_spatial_types  GetSpatialType      ( void ) { return m_SpatialType; }
                    void        SetSpatialType      ( damage_field_spatial_types SpatialType ) { m_SpatialType = SpatialType; }
                    void        SetSpatialTargets   ( s32 SpatialTargets ) { m_SpatialTargets = SpatialTargets; }
                    s32         GetSpatialTargets   ( void ) { return m_SpatialTargets; }
                    void        SetActive           ( xbool bActive ) { m_bActive = bActive; }
                    xbool       IsActive            ( void ) { return m_bActive; }
                    void        SetDoingDamage      ( xbool bDoingDamage ) { m_bDoingDamage = bDoingDamage; }
                    void        SetDoAttenuate      ( xbool bDoAttenuate ) { m_bDoAttenuate = bDoAttenuate; }
                    void        SetDimension        ( s32 DimensionIndex, f32 Value ) { if ( IN_RANGE( 0, DimensionIndex, 2 ) ) m_Dimensions[DimensionIndex] = Value; }
                    f32         GetDimension        ( s32 DimensionIndex ) { if ( IN_RANGE( 0, DimensionIndex, 2 ) ) return m_Dimensions[DimensionIndex]; else return 0.0;}
                    void        SetGenericPainType  ( generic_pain_type GenericPainType ) { m_GenericPainType = GenericPainType; }
                    void        SetTimeDelay        ( f32 TimeDelay ) { m_TimeDelay = TimeDelay; }


protected:

                    void        LogicCheckOnActivate( void );
                    xbool       QueryPlayerInVolume ( xbool bDoDamage );
                    xbool       QueryNpcInVolume    ( xbool bDoDamage );
                    xbool       QueryPropsInVolume  ( xbool bDoDamage );
                    xbool       QueryObjectInVolume ( object* pObject, xbool bDoDamage );

    damage_field_spatial_types      m_SpatialType;
    s32                             m_SpatialTargets;
    xbool                           m_bActive;
    xbool                           m_bDoingDamage;
    xbool                           m_bDoAttenuate;
    f32                             m_Dimensions[3];

    generic_pain_type               m_GenericPainType;   

    f32                             m_TimeDelay;
    f32                             m_TimeSinceLastDamage;

    object_affecter                 m_DamageAnchorAffecter;
    f32                             m_YOffset;

    static enum_table<damage_field_spatial_types>   m_SpatialTypeList; 
};

//=========================================================================
// END
//=========================================================================
#endif

