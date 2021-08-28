//===========================================================================
//
//  ViewportCamera.hpp
//
//  A camera class to be used for the viewports in 3D editing tools\
//
//===========================================================================

#ifndef __VIEWPORTCAMERA_HPP__
#define __VIEWPORTCAMERA_HPP__

//------------------------------------------------------------------------------+
//  INCLUDES
//------------------------------------------------------------------------------+


//------------------------------------------------------------------------------+
//  Class Description
//------------------------------------------------------------------------------+

#define CAM_HISTORY_SIZE    128

class ViewportCamera
{
    //--------------------------+
    // Constructor/Destructor   |
    //--------------------------+
    public:

         ViewportCamera( void );
        ~ViewportCamera( void );

    //--------------------------+
    // Functions - Public       |
    //--------------------------+
    public:
        const   vector3&    GetPosition     ( void ) const;
        const   quaternion& GetRotation     ( void ) const;

        const   vector3     GetTargetPos    ( void ) const;

        const   s32         GetUndoCount    ( void ) const;
        const   s32         GetRedoCount    ( void ) const;

                void        Navigate_Begin  ( void );
                void        Navigate_End    ( void );
                void        Navigate_Cancel ( void );

                void        Navigate_Undo   ( void );
                void        Navigate_Redo   ( void );

                void        Pan             ( f32 DeltaX, f32 DeltaY );
                void        Fly             ( f32 DeltaX, f32 DeltaY );

                void        Look            ( f32 DeltaX, f32 DeltaY );
                void        Orbit           ( f32 DeltaX, f32 DeltaY );
                void        OrbitPoint      ( f32 DeltaX, f32 DeltaY, const vector3& PivotPoint );
    
                void        Zoom            ( f32 DeltaY );
                void        ZoomRegion      ( f32 NormLeft, f32 NormTop, f32 NormRight, f32 NormBottom );
                void        ZoomExtents     ( const bbox& World_Aligned_Bounds );

    //--------------------------+
    // Data - Protected         |
    //--------------------------+
    protected:
        xbool       m_bIsOrtho;

        // Current Camera Settings
        vector3     m_Pos;          // Position of the Camera in World Space (V2W)
        quaternion  m_Rot;          // Camera's orientation
        f32         m_Focus;        // The length from the camera to it's theoretical focal point
        radian      m_FOV;          // Field-of-View

        // Last Camera Settings - Used for Navigate_Begin(), Navigate_End(), Navigate_Cancel()
        vector3     m_LastPos;
        quaternion  m_LastRot;
        f32         m_LastFocus;

        // Camera History - Used for circular Undo/Redo array
        struct  camera_info
        {
            vector3     m_Pos;
            quaternion  m_Rot;
            f32         m_Focus;
        };

        s32             m_HistoryIndex;     // Base Index into CamHistory array (ie The first Undo entry)
        s32             m_UndoCount;        // Number of Undo's, which start at the HistoryIndex
        s32             m_RedoCount;        // Number of Redo's, which start after the last Undo entry

        camera_info     m_CamHistory[ CAM_HISTORY_SIZE ];    // Must be a power of 2!
};

//------------------------------------------------------------------------------+
//------------------------------------------------------------------------------+

#endif  //#ifndef __VIEWPORTCAMERA_HPP__
