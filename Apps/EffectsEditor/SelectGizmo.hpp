#ifndef __SELGIZMO_HPP
#define __SELGIZMO_HPP

#include "ManipulatorMgr.h"
#include "Manipulator.h"
#include "ManipTranslate.h"

//============================================================================
// Gizmo for translating
struct gizmo_xlate
{
    CManipTranslate*    m_pTrans;

    vector3             m_StartPos;
    s32                 m_PointCount;

    gizmo_xlate         ( );
    ~gizmo_xlate        ( );

    void AddPoint       ( vector3& Point )      { m_StartPos += Point; m_PointCount++; }
    void CalcAvg        ( void )                { m_StartPos /= (f32)m_PointCount; m_PointCount = 0; }
};

//============================================================================
// Gizmo for scaling
struct gizmo_scale
{
    CManipTranslate*    m_pScale;

    vector3             m_StartScale;

    gizmo_scale         ( f32 ScaleX, f32 ScaleY, f32 ScaleZ );
    ~gizmo_scale        ( );
};


#endif
