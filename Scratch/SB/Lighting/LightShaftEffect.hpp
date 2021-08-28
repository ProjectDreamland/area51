//==============================================================================
//  
//  LightShaftEffect.hpp
//
//==============================================================================

#ifndef __LIGHT_SHAFT_EFFECT_HPP__
#define __LIGHT_SHAFT_EFFECT_HPP__


//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_Files.hpp"


//==============================================================================
//  CLASSES
//==============================================================================
class light_shaft_effect
{
// Functions
public:

        // Constructor/destructor
        light_shaft_effect();
        ~light_shaft_effect();

        // Functions
        void    Init    ( const char* pTextBMP, const char* pFogBMP );
        void    Kill    ( void );
        void    Update  ( f32 DeltaTime );
        void    Render  ( f32 Scale ) const;

// Data
private:
        xbitmap     m_TextBMP;          // Text texture
        xbitmap     m_FogBMP;           // Fog texture
        
        f32         m_Position;         // Current position of light shafts
        f32         m_HorizSpeed;       // Speed of light shafts
        f32         m_PixelWidth;       // Source pixel width of light shafts
        f32         m_HorizScale;       // Horizontal shaft spread amount
        f32         m_VertScale;        // Vertical shaft spread amount
        f32         m_Alpha;            // Alpha of each layer
        s32         m_nPasses;          // # of passes

        radian      m_FogAngle;         // Current fog rotation
        radian      m_FogRotSpeed;      // Speed of rotation
        f32         m_FogAlpha;         // Alpha of each layer
        f32         m_FogSize;          // Max size of fog layer
        f32         m_FogZoom;          // Current fog zoom
        f32         m_FogZoomSpeed;     // Speed of zoom
        s32         m_nFogPasses;       // # of passes
};


//==============================================================================
#endif // __LIGHT_SHAFT_EFFECT_HPP__
//==============================================================================
