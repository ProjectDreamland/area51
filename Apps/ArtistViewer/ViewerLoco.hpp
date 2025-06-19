//==============================================================================
//
//  File:           ViewerLoco.hpp
//
//  Description:    Viewer locomotion class
//
//  Author:         Stephen Broumley
//
//  Date:           Started August 1st, 2003 (C) Inevitable Entertainment Inc.
//
//==============================================================================

#ifndef __VIEWER_LOCO_HPP__
#define __VIEWER_LOCO_HPP__

//=========================================================================
// INCLUDE
//=========================================================================
#include "Loco\Loco.hpp"
#include "Config.hpp"



//=========================================================================
// LOCO STATES
//=========================================================================

struct viewer_loco_play_anim : public loco_play_anim
{
                     viewer_loco_play_anim   ( loco& Loco );
};

//=========================================================================

struct viewer_loco_idle : public loco_idle
{
                     viewer_loco_idle       ( loco& Loco );
};

//=========================================================================

struct viewer_loco_move : public loco_move
{
                     viewer_loco_move       ( loco& Loco );
};

//=========================================================================
// LOCOMOTION
//=========================================================================

class viewer_loco : public loco
{
    
// Construction / destruction
public:

	// Constructs a viewer_loco object.
	viewer_loco();

	// Destroys a viewer_loco object, handles cleanup and de-allocation.
	virtual ~viewer_loco();

// Public functions
private:
    void    FindAnim( loco::anim_type Type, const char* pNotThis, const char* pFindThis ) ;

// Public functions
public:

            // Checks to make sure property is present
            void CheckForProperty ( const geom*             pGeom,
                                    const char*             pSectionName, 
                                    const char*             pPropertyName,
                                    geom::property::type    PropertyType );

            // Initialize
    virtual void        OnInit( const geom*         pGeom, 
                                const char*         pAnimFileName, 
                                config_options::object&     Object ) ;

            // Move style functions
            xbool       IsValidIdleStyle   ( move_style Style ) ;
            xbool       IsValidMotionStyle ( move_style Style ) ;
    virtual xbool       IsValidMoveStyle   ( move_style Style ) ;
    virtual move_style  GetValidMoveStyle  ( move_style Style );
    virtual void        SetMoveStyle       ( move_style Style ) ;

            // Query functions
            xbool       HasIdleAnims    ( void ) const ;
            xbool       HasMoveAnims    ( void ) const ;
            xbool       HasLipSyncAnims ( void ) const ;
            xbool       HasWarnings     ( void ) const ;

            // Events
            void        SendEvents      ( object* pOwner );

            // Render
            void        Render          ( void );
            
// Private data
protected:

    config_options::object*                     m_pConfigObject ;
    viewer_loco_play_anim               m_PlayAnim;
    viewer_loco_idle                    m_Idle;
    viewer_loco_move                    m_Move;
    xbool                               m_bHasIdleAnims ;
    xbool                               m_bHasMoveAnims ;
    xbool                               m_bHasLipSyncAnims ;
    xbool                               m_bWarnings;
    
// Friends
    friend class viewer_object ;
};

//=========================================================================

#endif // __VIEWER_LOCO_HPP__

