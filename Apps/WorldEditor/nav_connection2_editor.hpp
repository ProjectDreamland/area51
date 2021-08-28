#ifndef NAV_CONNECTION2_EDITOR_HPP
#define NAV_CONNECTION2_EDITOR_HPP

//=========================================================================
//  INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "navigation\ng_connection2.hpp"

class nav_connection2_anchor;

//=========================================================================
//  CLASS
//=========================================================================
class nav_connection2_editor : public object
{
public:

    CREATE_RTTI( nav_connection2_editor, object, object )

    enum flags
    {
        FLAG_NONE           = 0x00000000,
        FLAG_MASK_PERSONAL  = 0xFF000000,       // We reserve the upper 8 bits
        FLAG_MASK_SHARED    = 0x00FFFFFF,       // Bits that we don't enum

        FLAG_DYING          = BIT(30),          // OnKill has been called
        FLAG_MOVING         = BIT(29),          // OnMove is in process
        FLAG_ANCHORS_DIRTY  = BIT(28),          // One or both anchors have run their OnMove        
        FLAG_CORNERS_DIRTY  = BIT(27),          // Render corners are dirty

        FLAG_ALL            = 0xFFFFFFFF
    };
/*
    enum AI_hints
    {
        HINT_NONE       = 0x00000000,
        HINT_DARK       = BIT(0),       //  Connection is mostly concealed in Darkness
        HINT_COVER      = BIT(1),       //  Path has significant cover
        HINT_WALL       = BIT(2),       //  connection is on a wall
        HINT_CEILING    = BIT(3),       //  Connection is on the ceiling
        HINT_SNEAKY     = BIT(4),       //  Connection is considered a sneaky path
        HINT_JUMPER     = BIT(5),       //  Connection is a jump
        HINT_ORIENTATION_CHANGE = BIT(6),// Connection requires orientation change
        HINT_ONE_WAY    = BIT(7),
        HINT_PATROL_ROUTE= BIT(8),       //  Is this part of a patrol route

        HINT_ALL        = 0xFFFFFFFF
   
    };
*/

                            nav_connection2_editor               ( void );
    virtual                 ~nav_connection2_editor              ( );
    virtual bbox            GetLocalBBox                        ( void ) const;
    virtual s32             GetMaterial                         ( void ) const { return MAT_TYPE_NULL; }

    virtual void            Reset                               ( void );

    virtual void            OnInit                              ( void );     

    virtual void            OnEnumProp                          ( prop_enum& List );
    virtual xbool           OnProperty                          ( prop_query& I );

    virtual void            OnMove                              ( const vector3& NewPos );

    virtual void            RecalcPosition                      ( xbool callOnMove  = true );
    
    virtual void            ScaleWidth                          ( f32 scaleValue );
    virtual void            SetWidth                            ( f32 Width );

    virtual void            SetAnchors                          ( guid startAnchor, guid endAnchor );
    virtual void            SetAnchor                           ( s32 iAnchor, guid gAnchor );

    virtual guid            GetStartAnchor                      ( void ) { return m_Anchor[0]; }
    virtual guid            GetEndAnchor                        ( void ) { return m_Anchor[1]; }
            xbool           HasAnchor                           ( guid testAnchor)       { return( testAnchor == m_Anchor[0] || testAnchor == m_Anchor[1] ); }

    virtual void            MoveAnchor                          ( s32 iAnchor, const vector3& Pos );

    virtual void            SetClearLineOfSight                 ( xbool clearLOS ) { m_ClearLineOfSight = clearLOS;  }
    virtual xbool           GetClearLineOfSight                 ( void ) { return m_ClearLineOfSight;  }

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );
    
    virtual void            SetIndexInSavedList( s32 connectionIndex ) { m_IndexInSavedList = connectionIndex;    }
    virtual s32             GetIndexInSavedList( void )         { return m_IndexInSavedList;  }

    virtual f32             GetWidth            ( void );
    virtual f32             GetLength           ( void );
    virtual radian          GetYaw              ( void );
    
    virtual u32             GetFlags            ( void )         { return m_Flags;    }
    virtual void            SetFlags            ( u32 flags)     { m_Flags = flags;   }

    xbool                   IsDying             ( void )        { return m_Flags & FLAG_DYING; }
    xbool                   IsMoving            ( void )        { return m_Flags & FLAG_MOVING; }

    void                    SetAnchorsDirty     ( void )        { m_Flags |= (FLAG_ANCHORS_DIRTY | FLAG_CORNERS_DIRTY); }
    void                    SetCornersDirty     ( void )        { m_Flags |= (FLAG_ANCHORS_DIRTY | FLAG_CORNERS_DIRTY); }
    
    virtual vector3*        GetRenderCorners    ( xbool bForceUpdate = TRUE );

    nav_connection2_anchor* GetAnchor           ( s32 iAnchor );

    virtual plane           GetPlane            ( void );

    virtual vector3         GetAnchorPosition   ( s32 iAnchor );

    xbool                   IsPointInConnection ( const vector3& pointToCheck, f32 bufferAmount );

    virtual void            RenderNavConnection ( void );
            
            void            SetInitiallyEnabled ( xbool bYesNo );
            void            SetEnabled          ( xbool bYesNo );
            xbool           GetInitiallyEnabled ( void );
            xbool           GetEnabled          ( void );
    
protected:

    /*
        NOTE ON RENDER CORNERS:

                     1_________0
                    /        /
                  /        /
                /        /
              /        /
           5 ----------  4             /^\
                                        |
                                        |   +Y
                     3_________2
                    /        /
                  /        /
                /        /
              /        /
           7 ----------  6


            0,1,2,3 are at Anchor[0] end
            4,5,6,7 are at Anchor[1] end
    */

    void                    BuildCorners        ( void );
    void                    Draw_Volume         ( xcolor aColor );


    virtual void            OnRender            ( void );    
    virtual void            OnColCheck          ( void );
    virtual void            OnColNotify         ( object& Object );
    virtual void            OnKill              ( void );

#ifndef X_RETAIL
    virtual void            OnDebugRender       ( void );
#endif // X_RETAIL

//=========================================================================
//  DATA MEMBERS
//=========================================================================

//  Members to be written out to file

    f32             m_Width;
    guid            m_Anchor[2];
    u32             m_Flags;

    vector3         m_CollisionCorners[8];
    vector3         m_RenderCorners[8];
    bbox            m_CollisionBBox;        // For selection picking
    bbox            m_LocalRenderBBox;      // For render culling 
    
//  Members to be calculated based on permanent data

    f32             m_Length;               // length calculated on the 2 end points
    vector3         m_AnchorPosition[2];    // world-space positions of endpoints
    xbool           m_ClearLineOfSight;
    s32             m_IndexInSavedList;     // used when building navmap
    xbool           m_bIsEnabled;

};

//=========================================================================
//  END
//=========================================================================
#endif