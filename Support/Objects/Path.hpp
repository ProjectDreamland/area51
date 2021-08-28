#ifndef __PATH_HPP__
#define __PATH_HPP__

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"

//=========================================================================
// CLASS
//=========================================================================

class path : public object
{
//=====================================================================
// DEFINES
//=====================================================================
public:

    // General flags
    enum flags
    {
        // Key "property present" flags
        FLAG_KEY_DELTA_TIME    = (1<<0),    // Keys have "delta time" property
        FLAG_KEY_POSITION      = (1<<1),    // Keys have "position" property
        FLAG_KEY_ROTATION      = (1<<2),    // Keys have "rotation" propery
        FLAG_KEY_FIELD_OF_VIEW = (1<<3),    // Keys have "field of view" property
        FLAG_KEY_ROLL          = (1<<4),    // Keys have "roll" property
        FLAG_KEY_COLOR         = (1<<5),    // Keys have "color" property
        FLAG_KEY_OBJECT_GUID   = (1<<6),    // Keys have "guid" property
        FLAG_KEY_AUTO_YAW      = (1<<7),    // Key  rotation YAW is generated from the path
        FLAG_KEY_AUTO_PITCH    = (1<<8),    // Key  rotation PITCH is generated from the path
        
        // Editor only flags (not exported)
        FLAG_KEYS_LOCKED       = (1<<31),    // Keys do not get updated during OnMove or OnTransform
    } ;

    // Available curves
    enum curve_type
    {
        CURVE_TYPE_LINEAR,      // Linear curve thru points
        CURVE_TYPE_SPLINE       // Spline curve thru points
    } ;

//=====================================================================
// STRUCTURES
//=====================================================================

    // Key
    struct key
    {
        // Flags
        enum flags
        {
            FLAG_SELECTED               = (1<<0),   // Key is selected in editor
            FLAG_CORNER                 = (1<<1),   // Key is a corner (no curve)
            FLAG_DEACTIVATE_TRACKER     = (1<<2),   // Tracker deactivates at this key
            FLAG_ACTIVATE_OBJECT        = (1<<3),   // Object activates at this key
            FLAG_DEACTIVATE_OBJECT      = (1<<4),   // Object deactivates at this key

            FLAG_RENDER_OBJECT_EVENT    = (1<<5),   // Used in the editor when object event is triggered
        } ;

        // Data
        vector3     m_Position ;    // World position
        quaternion  m_Rotation ;    // World rotation
        guid        m_ObjectGuid ;  // Object guid
mutable u32         m_Flags ;       // General flags
        f32         m_DeltaTime ;   // Time since last key
        radian      m_FieldOfView ; // Camera field of view
        radian      m_Roll ;        // Camera roll
        xcolor      m_Color ;       // Color

        // Functions
        
        // Constructor
        key() ;

        // Enumerates properties
        void    OnEnumProp  ( prop_enum& List, path& Path, s32 iKey, u32 PresentFlags, u32 NotPresentFlags ) ;

        // Evaluates properties
        xbool   OnProperty  ( prop_query& I, path& Path, s32 iKey ) ;

        // Sets up defaults
        void    SetDefaults ( void ) ;

        // Accumulated multi-selection key values
        void    MultiSelect ( const key& Key ) ;

        // Interpolates between keys
        void    Interpolate( const path& Path,
                                   s32   iKeyA, f32 KeyATime,
                                   s32   iKeyB, f32 KeyBTime,
                                   f32   T,
                                   f32   PrevTime, 
                                   f32   CurrTime );

        // Render key
        void    OnRender    ( path& Path, s32 iKey ) ;
    } ;


//=====================================================================
// PUBLIC BASE CLASS FUNCTIONS
//=====================================================================
public:

    CREATE_RTTI( path, object, object )
    
                            path                ( void );
    virtual bbox            GetLocalBBox        ( void ) const ;
    virtual s32             GetMaterial         ( void ) const { return MAT_TYPE_FLESH; }

    virtual void            OnEnumProp          ( prop_enum& List );
    virtual xbool           OnProperty          ( prop_query& I );
            void            ValidatePosition    ( void );
#if defined( X_EDITOR )
    virtual void            OnSave              ( text_out& TextOut );
#endif // defined( X_EDITOR )
    virtual void            OnLoad              ( text_in&  TextIn  );
    virtual void            OnMove              ( const vector3& NewPos );        
    virtual void            OnTransform         ( const matrix4& L2W    ); 

    virtual void            OnActivate          ( xbool Flag );            

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

#if !defined( CONFIG_RETAIL )
    void                    RenderPath          ( void );
#endif // !defined( CONFIG_RETAIL )

//=====================================================================
// PRIVATE BASE CLASS FUNCTIONS
//=====================================================================

protected:
    virtual void            OnInit              ( void );     
    virtual void            OnColCheck          ( void );
    virtual void            OnColNotify         ( object& Object ) ;

#if !defined( CONFIG_RETAIL )
            void            RenderSegments      ( f32    StartTime, 
                                                  f32    EndTime, 
                                                  s32    nSegments, 
                                                  xbool  bLines,
                                                  xcolor Color ) ;

    virtual void            OnRender            ( void );
#endif // !defined( CONFIG_RETAIL )
            
    
//=====================================================================
// PRIVATE FUNCTIONS
//=====================================================================
private:

            // Key functions
            void            CreateKey           ( s32 iRefKey = -1, xbool bAfterRefKey = TRUE ) ;
            void            DeleteKey           ( s32 Index ) ;
            void            SelectKey           ( s32 Index ) ;

            void            AutoGenerateRotation( void );

#ifdef X_EDITOR

            // Returns length (in cm) of key segment
            f32             GetKeyLength        ( s32 iKey ) const;
            
            // Returns selected key range
            s32             GetKeysInfo         ( xbool bSelectedOnly, xbool bSkipKey0, s32& iStartKey, s32& iEndKey ) const;
            
            // Returns selected key range and total time + length of segment (skips key0)
            s32             GetKeysInfo         ( xbool bSelectedOnly,
                                                  s32&  iStartKey, 
                                                  s32&  iEndKey,
                                                  f32&  TotalTime,
                                                  f32&  TotalLength );

            // Operations                                                  
            void            ScaleTime           ( xbool bSelectedOnly, f32 TotalTime );
            void            SmoothSpeed         ( xbool bSelectedOnly );
            void            EqualSpacing        ( xbool bSelectedOnly );

#endif  //#ifdef X_EDITOR


//=====================================================================
// PUBLIC FUNCTIONS
//=====================================================================

public:

            // Gets flags
            u32             GetFlags            ( void ) const { return m_Flags ; }

            // Returns key count
            s32             GetKeyCount         ( void ) const { return m_Keys.GetCount() ; }

            // Returns key
            key&            GetKey              ( s32 iKey )  { return m_Keys[iKey] ; }
            
            // Returns key
            const key&      GetKey              ( s32 iKey ) const { return m_Keys[iKey] ; }

            // Returns the curve type
            curve_type      GetCurveType        ( void ) const { return m_CurveType ; } 

            // Returns path icon selected key
            s32             GetPathIconSelectedKey  ( void ) const;

            // Sets icon position to key
            void            SetPathIconPosition     ( s32 iKey );
            
            // Returns selected key count
            s32             GetSelectedKeyCount ( void ) const ;
            
            // Returns key time
            f32             GetKeyTime          ( s32 iKey ) const ;

            // Returns key tangent for spline
            vector3         GetKeyTangent       ( s32 iKey ) const ;

            // Returns interpolated key, given previous and current time
            void            GetInterpKey        ( f32 PrevTime, f32 CurrTime, key& Key ) const;

            // Returns total time of all keys
            f32             GetTotalTime        ( xbool bSelectedOnly ) const;

            // Forces update of bounds
            void            UpdateBounds        ( void ) ;

            // Is the path on or off (trackers should obey this)
            xbool           IsPathOn            ( void );

//=====================================================================
// DATA
//=====================================================================

protected:

    u32                 m_Flags ;           // General flags
    curve_type          m_CurveType ;       // Type of curve
    xarray<key>         m_Keys ;            // List of keys
    u8                  m_bPathOn:1;        // 
    
    
friend struct key;    
};

//=========================================================================
// END
//=========================================================================
#endif
