#ifndef __AI_MGR_HPP__
#define __AI_MGR_HPP__

#include "..\Navigation\Nav_Map.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"



class ai_mgr : public prop_interface
{
public:
    ai_mgr();
    ~ai_mgr();
    virtual void        OnEnumProp                      ( prop_enum&    List );
    virtual xbool       OnProperty                      ( prop_query&   I    );

    inline xbool        GetShowNavSpine                 ( void );
    inline f32          GetOverlapConstructionScale     ( void );
    inline xbool        GetShowConnectionIDs            ( void );
    inline xbool        GetShowGridColoring             ( void );
    inline xbool        GetShowTacticalOverlay          ( void );
    inline xbool        GetRenderConnectionsBright      ( void );    

    void    Init();
    void    Reset();

protected:
    xbool   m_bShowNavSpine; 
    xbool   m_bShowGridColoring;
    xbool   m_bShowTacticalOverlay;
    xbool   m_bRenderConnectionsBright;
    f32     m_OverlapReductionAmt;      // Amount to scale overlap area by.  Must be <1.0f

    xbool   m_bShowConnectionIDs;
};


extern ai_mgr   g_AIMgr;

//=============================================================================
//
//  Inline functions
//
//=============================================================================

inline xbool ai_mgr::GetShowNavSpine()
{
    return m_bShowNavSpine;
}

//=============================================================================

inline f32 ai_mgr::GetOverlapConstructionScale()
{
    return m_OverlapReductionAmt;
}

//=============================================================================

inline
xbool ai_mgr::GetShowConnectionIDs()
{
    return m_bShowConnectionIDs;
}

//=============================================================================

inline
xbool ai_mgr::GetShowGridColoring()
{
    return m_bShowGridColoring;
}

//=============================================================================

inline
xbool ai_mgr::GetShowTacticalOverlay()
{
    return m_bShowTacticalOverlay;
}

//=============================================================================

inline
xbool ai_mgr::GetRenderConnectionsBright()
{
    return m_bRenderConnectionsBright;
}

//=============================================================================


//=============================================================================

#endif // __AI_MGR_HPP__