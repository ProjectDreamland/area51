///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AI_editor.hpp
//
//      pass through class to shield the editorview from any game specific stuff
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef AI_EDITOR_HPP
#define AI_EDITOR_HPP

#include "..\WorldEditor\EditorDoc.h"
#include "nav_connection2_editor.hpp"

class ai_editor
{
public:

    enum eSetFlagMode
    {
        k_NoChange = 0,
        k_Toggle,
        k_AllTrue,
        k_AllFalse
    };

    enum ClipPlanes
    {
        CLIP_LEFT = 0,
        CLIP_RIGHT,
        CLIP_TOP,
        CLIP_BOTTOM,
        CLIP_MAX
    };



    ai_editor(void);
    ~ai_editor() { s_This = NULL;   }

    static ai_editor*  GetAIEditor(void) { if(!s_This) new ai_editor; return s_This; }


    void    Render              (void);
    
    void    LoadNavMap          ( const char* fileName );
    void    SaveNavMap          ( const char* fileName );

    void    CreateNavMap        ( void );
    void    CleanNavMap         ( void );
    
    void    UpgradeNavObjects   ( void );        // Upgrade from old connection/nodes to connection2 and anchors

    xbool   UpdateAI            ( void );
    guid    CreatePlayer        ( void );
    xbool   DoesPlayerExist     ( void );
    

    // START OF UNUSED
    void    SetNavTestStart     ( guid thisNode );
    void    SetNavTestEnd       ( guid thisNode );
    void    CalcPath            ( void );
    
    void    CheckAllNavConnections( void );
    //void    ConnectAllNavNodes( void );
    //void    ConnectAllSelectedNavNodes( void );
    //xbool   ObjectSelected( guid ObjGuid );
    //void    SetObjectSelected( guid ObjGuid ) { m_LastObjectSelected = ObjGuid;  }
    
    //xbool IsInConnectionMode( void )    { return m_InConnectionMode; }
    //void SetInConnectionMode(xbool newStatus ) { m_InConnectionMode = newStatus;  }

    void    SetFlagsInSelectedObjects   (u32 Flags, eSetFlagMode setFlagMode );
    void    SetGridIDs( void );
    
    void    ClearTestPath   ( void );
    void    ComputeTestPath ( const vector3& MouseRayStart, const vector3& MouseRayEnd );
    void    RenderTestPath  ( void );

protected:

    xbool   TestConnectionsForOverlap( nav_connection2_editor*  pA,                 // IN:  Connection 1
                                       nav_connection2_editor*  pB,                 // IN:  Connection 2
                                       vector3*                 pVertBucket,        // IN:  Storage for verts
                                       s32                      nMaxVerts,          // IN:  Storage bucket size
                                       s32&                     nOutputVerts    );  // OUT: # of verts written to bucket
    
    s32     ClipConnections          ( nav_connection2_editor*  pA,                 // IN:  Connection 1
                                       nav_connection2_editor*  pB,                 // IN:  Connection 2
                                       vector3*                 pVertBucket,        // IN:  Storage for verts
                                       s32                      nMaxVerts );        // IN:  Storage bucket size

                                                                                    // NOTE: A similar function exists in
                                                                                    // nav_map, but the nav_map isn't built when
                                                                                    // the ai_editor needs to do clipping, so we
                                                                                    // have a seperate version here that works with
                                                                                    // nav_connection2_editor ptrs instead.
                                                                                    // We still use the raw clipper in the nav_map
                                                                                    // once we have generated our verts
                                       
    static ai_editor*  s_This;

    u16    m_FirstSelectedObject;

    xbool  m_InConnectionMode;

    guid   m_LastObjectSelected;
    
    // Test pathing members    
    xbool               m_bTestPathValid;
    f32                 m_TestPathRadius;
    pathing_hints       m_PathingHints;
    path_find_struct    m_PathFindStruct;
};


#endif//AI_EDITOR_HPP
 
