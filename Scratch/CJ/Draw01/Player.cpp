//==============================================================================
//
//  Player.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "aux_Bitmap.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

struct poly
{
    vector3 p[3];
    xcolor  Color;
};

xarray<poly>    g_World;

vector3 g_PlayerCenter(100.0f,50.0f,-25.0f);
f32     g_PlayerRadius = 25.0f;

//==============================================================================
//  FUNCTIONS
//==============================================================================

void World_Init( void )
{
    g_World.Clear();
}

//=========================================================================

void World_AddPoly( vector3& p0, vector3& p1, vector3& p2, xcolor Color )
{
    poly& p = g_World.Append();
    p.p[0] = p0;
    p.p[1] = p1;
    p.p[2] = p2;
    p.Color = Color;
}

//=========================================================================

void World_AddBox( bbox& Box, xcolor Color )
{
    vector3 p0( Box.Min.GetX(), Box.Min.GetY(), Box.Min.GetZ() );
    vector3 p1( Box.Max.GetX(), Box.Min.GetY(), Box.Min.GetZ() );
    vector3 p2( Box.Max.GetX(), Box.Min.GetY(), Box.Max.GetZ() );
    vector3 p3( Box.Min.GetX(), Box.Min.GetY(), Box.Max.GetZ() );

    vector3 p4( Box.Min.GetX(), Box.Max.GetY(), Box.Min.GetZ() );
    vector3 p5( Box.Max.GetX(), Box.Max.GetY(), Box.Min.GetZ() );
    vector3 p6( Box.Max.GetX(), Box.Max.GetY(), Box.Max.GetZ() );
    vector3 p7( Box.Min.GetX(), Box.Max.GetY(), Box.Max.GetZ() );

    World_AddPoly( p0, p1, p2, Color );
    World_AddPoly( p2, p3, p0, Color );

    World_AddPoly( p6, p5, p4, Color );
    World_AddPoly( p4, p7, p6, Color );

    World_AddPoly( p0, p4, p5, Color );
    World_AddPoly( p5, p1, p0, Color );

    World_AddPoly( p1, p5, p6, Color );
    World_AddPoly( p6, p2, p1, Color );

    World_AddPoly( p2, p6, p7, Color );
    World_AddPoly( p7, p3, p2, Color );

    World_AddPoly( p3, p7, p4, Color );
    World_AddPoly( p4, p0, p3, Color );
}

//=========================================================================

void World_Render( void )
{
    random r(0);

    eng_Begin( "World" );

    draw_Begin( DRAW_TRIANGLES, 0 );

    for( s32 i=0 ; i<g_World.GetCount() ; i++ )
    {
        poly& p = g_World[i];

        draw_Color( r.color() );
//        draw_Color( xcolor( 255,255,255 ) );
        draw_Vertex( p.p[0] );
        draw_Vertex( p.p[1] );
        draw_Vertex( p.p[2] );
    }

    draw_End();

    draw_SetL2W( matrix4( vector3(1,2,1), radian3(0,0,0), g_PlayerCenter ) );
    draw_Sphere( vector3(0,0,0), g_PlayerRadius, XCOLOR_WHITE );

    eng_End();
}

//=========================================================================
//==============================================================================
