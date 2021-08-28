//==============================================================================
//  DecalDefinition.cpp
//
//  Copyright (c) 2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class defines the behavior of a decal. Which bitmap does it use? How
//  big is it? Does it wrap around walls? What blending does it use? etc.
//==============================================================================

#include "DecalDefinition.hpp"
#include "DecalMgr.hpp"

//==============================================================================
// Implementation
//==============================================================================

decal_definition::decal_definition( void ) :
    m_MinSize       ( 50.0f, 50.0f ),
    m_MaxSize       ( 50.0f, 50.0f ),
    m_MinRoll       ( R_0 ),
    m_MaxRoll       ( R_0 ),
    m_Color         ( 128, 128, 128, 255 ),
    m_MaxVisible    ( 20 ),
    m_FadeTime      ( 2.0f ),
    m_Flags         ( DECAL_FLAG_NO_CLIP ),
    m_BlendMode     ( DECAL_BLEND_ADD ),
    m_Handle        ( HNULL )
{
    m_Name[0]       = '\0';
    m_BitmapName[0] = '\0';
}

//==============================================================================

decal_definition::decal_definition( fileio& File )
{
    (void)File;

    m_Handle = HNULL;
}

//==============================================================================

decal_definition::~decal_definition( void )
{
    ASSERT( m_Handle.IsNull() );
    m_Handle = HNULL;
}

//==============================================================================

void decal_definition::FileIO( fileio& File )
{
    File.Static( m_Name,       32  );
    File.Static( m_MinSize         );
    File.Static( m_MaxSize         );
    File.Static( m_MinRoll         );
    File.Static( m_MaxRoll         );
    File.Static( m_Color           );
    File.Static( m_BitmapName, 256 );
    File.Static( m_MaxVisible      );
    File.Static( m_FadeTime        );
    File.Static( m_Flags           );
    File.Static( m_BlendMode       );

    if ( m_MinRoll > m_MaxRoll )
    {
        // this shouldn't happen, but just to be safe
        radian Temp = m_MinRoll;
        m_MinRoll   = m_MaxRoll;
        m_MaxRoll   = Temp;
    }
}

//==============================================================================

vector2 decal_definition::RandomSize( void ) const
{
    f32 WidthT;
    f32 HeightT;
    
    WidthT = x_frand(0.0f,1.0f);
    if ( m_Flags & DECAL_FLAG_KEEP_SIZE_RATIO )
    {
        HeightT = WidthT;
    }
    else
    {
        HeightT = x_frand(0.0f,1.0f);
    }

    return vector2( m_MinSize.X+WidthT*(m_MaxSize.X-m_MinSize.X),
                    m_MinSize.Y+HeightT*(m_MaxSize.Y-m_MinSize.Y) );
}

//==============================================================================

radian decal_definition::RandomRoll( void ) const
{
    return x_frand( m_MinRoll, m_MaxRoll );
}