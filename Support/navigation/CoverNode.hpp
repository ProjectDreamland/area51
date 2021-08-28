#ifndef COVER_NODE_HPP
#define COVER_NODE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\Obj_mgr.hpp"
#include "miscutils\PropertyEnum.hpp"
#include "Nav_Map.hpp"
#include "inventory\inventory2.hpp"

//=========================================================================
// Cover Node
//=========================================================================
class cover_node : public object
{
public:
    enum eCoverAnimPackageType
    {
        COVER_PACKAGE_NONE = -1,
        COVER_PACKAGE_CIVILIAN,
        COVER_PACKAGE_SOLDIER,
        COVER_PACKAGE_GRUNT,
        COVER_PACKAGE_LEAPER,
        COVER_PACKAGE_COUNT,
    };

    enum eCoverWeaponType
    {
        COVER_WEAPON_NONE,
        COVER_WEAPON_SMP,

// KSS -- TO ADD NEW WEAPON
        COVER_WEAPON_SHT,

        COVER_WEAPON_SNI,
        COVER_WEAPON_GAS,
        COVER_WEAPON_EGL,
        COVER_WEAPON_MHG,
        COVER_WEAPON_MSN,
        COVER_WEAPON_BBG,
        COVER_WEAPON_TRA,
        COVER_WEAPON_MUT,

        COVER_WEAPON_COUNT
    };

public:

    CREATE_RTTI( cover_node, object, object )

                                cover_node      ( void );
    virtual                    ~cover_node      ( void );

    virtual const object_desc&  GetTypeDesc     ( void ) const ;

    virtual void                OnMove          ( const vector3& NewPos );      
    virtual void                OnTransform     ( const matrix4& L2W    ); 
    virtual void                OnAdvanceLogic  ( f32 DeltaTime         ); 

    virtual	void	            OnEnumProp	    ( prop_enum& rList );
    virtual	xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual s32                 OnValidateProperties( xstring& ErrorMsg );
#endif

    virtual s32                 GetMaterial     ( void ) const ;
    virtual bbox                GetLocalBBox    ( void ) const ;
    virtual void                OnRender        ( void );
    virtual void                OnActivate      ( xbool Flag );            
    virtual xbool               IsActive        ( void )    { return m_bActive; }

#ifndef X_RETAIL
    virtual void                OnDebugRender   ( void );
#endif // X_RETAIL

    inline  xbool               IsOccupied      ( void );

            xbool               IsCoverFromLocation( const vector3& targetLocation );

            void                InvalidateNode  ( void );
    void                        ReserveNode     ( guid NewUser ) ;
    xbool                       IsReserved      ( guid Requester ) ;

    xbool                       HasValidAnims2  ( object::type characterType, const char* logicalName, inven_item WeaponItem );

    anim_group::handle&         GetAnimGroupHandle      ( s32 PackageType );
    const char*                 GetAnimGroupName( object::type characterType, const char* logicalName );
    const char*                 GetAnimGroupNameByType( s32 packageType );
    radian                      GetNPCFacing    ( void );
    guid                        GetCheckPoint   ( void )        { return m_CheckPoint; } 
    guid                        GetNextStickyNode( void )   { return m_NextStickyNode; }

    s32                         GetShootWeight  ( void )    { return m_ShootWeight; }
    s32                         GetGrenadeWeight( void )    { return m_GrenadeWeight; }
    f32                         GetMinDelay     ( void )    { return m_ActionDelayMin; }
    f32                         GetMaxDelay     ( void )    { return m_ActionDelayMax; }

    s32                         GetNumValidCoverPackages( void );
    xbool                       GetIsCoverToAll ( void )    { return m_bCoverToAll; }

protected:

    enum flags
    {
        FLAGS_DIRTY_PLANES = (1<<0),
        FLAGS_OCCUPIED     = (1<<1),
        FLAGS_DEBUG_RENDER = (1<<2),
    };

protected:

    void                        ComputeVerts        ( vector3* pVert ) const;
    void                        ComputePlanes       ( void );
    void                        ValidateWeaponNPC   ( void );
    const char*                 GetIdleAnimNameForWeapon( eCoverWeaponType weaponType );
    eCoverAnimPackageType       GetAnimPackageType  ( object::type characterType, const char* logicalName );
    eCoverWeaponType            GetWeaponType2      ( inven_item WeaponItem );

    radian                      GetXFOV             ( void );
    void                        SetXFOV             ( radian XFOV );
    radian                      GetYFOV             ( void );
    void                        SetYFOV             ( radian YFOV );

protected:

    plane           m_Plane[6];
    xtick           m_ReserveTimer;
    guid            m_CheckPoint;       // the point we use when checking to see if we can throw grenades
    guid            m_Destructable;     // our destructable, if destructable goes so do we.
    guid            m_ReservedGuid;
    guid            m_NextStickyNode;   // Useful from grunts, this guid will be set as next sticky 
    f32             m_Width;            // Width of the window use for the frustrum
    f32             m_Height;           // Height of the window use for the frustrum
    f32             m_MaxDistance;      // Distance for the view
    f32             m_MinDistance;      // min distance, if closer than this, not a valid cover.
    f32             m_ActionDelayMin;
    f32             m_ActionDelayMax;
    s32             m_ShootWeight;
    s32             m_GrenadeWeight;
    //s32             m_MaxIndices[3*5];  // Indices to check bbox agains the different planes of the view
    u32             m_Flags;
    xbool           m_HasValidatedWeaponNPC;
    xbool           m_ValidWeaponNPC[COVER_PACKAGE_COUNT][COVER_WEAPON_COUNT];
    xbool           m_bFirstReservation;
    xbool           m_bCoverToAll;
    xbool           m_bActive;
    
    anim_group::handle          m_hSoldierAnimGroup;
    anim_group::handle          m_hCivilianAnimGroup;
    anim_group::handle          m_hGruntAnimGroup;
    anim_group::handle          m_hLeaperAnimGroup;
};

//=========================================================================

inline
xbool cover_node::IsOccupied( void )
{
    return (m_Flags&FLAGS_OCCUPIED)!=0 ;
}

//=========================================================================

//=========================================================================
// END
//=========================================================================
#endif
