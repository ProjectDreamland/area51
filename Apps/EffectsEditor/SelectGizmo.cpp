#include <stdafx.h>

#include "ManipulatorMgr.h"
#include "Manipulator.h"
#include "SelectGizmo.hpp"


//============================================================================
// Extern for the manip. mgr
extern CManipulatorMgr g_ManipulatorMgr;


//============================================================================
// Gizmo for translating (ctor)
gizmo_xlate::gizmo_xlate( )
{
    m_StartPos.Zero();
    m_pTrans = g_ManipulatorMgr.NewManipTranslate();
    m_PointCount = 0;
}

//============================================================================
// Gizmo for translating (dtor)
gizmo_xlate::~gizmo_xlate( )
{
    g_ManipulatorMgr.DeleteManipulator( (CManipulator*)m_pTrans );
}
    
//============================================================================
// Gizmo for scaling (ctor)
gizmo_scale::gizmo_scale( f32 ScaleX, f32 ScaleY, f32 ScaleZ )
{
}

//============================================================================
// Gizmo for scaling (dtor)
gizmo_scale::~gizmo_scale( )
{
}

