#include "CoverNode.hpp"
#include "entropy\e_draw.hpp"
#include "Entropy\e_ScratchMem.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "ng_node2.hpp"
#include "Characters\Character.hpp"
#include "objects\SuperDestructible.hpp"
#include "objects\DestructibleObj.hpp"

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif

const f32 k_MinPreferedReserveTime = 0.0f;
const f32 k_MinReserveTime = 8.0f;

//=========================================================================
// EDITOR SPECIAL CASE
//=========================================================================
#ifdef X_EDITOR

static  cover_node::eCoverAnimPackageType   s_CoverNodeDebugPackage = cover_node::COVER_PACKAGE_NONE;
static  cover_node::eCoverWeaponType        s_CoverNodeDebugWeapon  = cover_node::COVER_WEAPON_NONE;
extern  xbool                               g_game_running ;

//==============================================================================

// Update this if you add new package types. It's used for drawing the cover node debug anim
typedef enum_pair<cover_node::eCoverAnimPackageType> cover_node_package_enum_pair;
static cover_node_package_enum_pair s_CoverNodePackageEnumPair[] = 
{
    cover_node_package_enum_pair("ALL",                 cover_node::COVER_PACKAGE_NONE      ),
    cover_node_package_enum_pair("CIVILIAN",            cover_node::COVER_PACKAGE_CIVILIAN  ),
    cover_node_package_enum_pair("SOLDIER",             cover_node::COVER_PACKAGE_SOLDIER   ),
    cover_node_package_enum_pair("GRUNT",               cover_node::COVER_PACKAGE_GRUNT     ),
    cover_node_package_enum_pair("LEAPER",              cover_node::COVER_PACKAGE_LEAPER    ),
    cover_node_package_enum_pair( k_EnumEndStringConst, cover_node::COVER_PACKAGE_COUNT) //**MUST BE LAST**//
};
enum_table<cover_node::eCoverAnimPackageType>  s_CoverNodePackageEnumTable(s_CoverNodePackageEnumPair);

//==============================================================================

// Update this if you add new weapon types. It's used for drawing the cover node debug anim
typedef enum_pair<cover_node::eCoverWeaponType> cover_node_weapon_enum_pair;
static cover_node_weapon_enum_pair s_CoverNodeWeaponEnumPair[] = 
{
    cover_node_weapon_enum_pair("ALL",   cover_node::COVER_WEAPON_NONE ),
    cover_node_weapon_enum_pair("SMP",   cover_node::COVER_WEAPON_SMP ),
    cover_node_weapon_enum_pair("SHT",   cover_node::COVER_WEAPON_SMP ),
    cover_node_weapon_enum_pair("SNI",   cover_node::COVER_WEAPON_SNI ),
    cover_node_weapon_enum_pair("GAS",   cover_node::COVER_WEAPON_GAS ),
    cover_node_weapon_enum_pair("EGL",   cover_node::COVER_WEAPON_EGL ),
    cover_node_weapon_enum_pair("MHG",   cover_node::COVER_WEAPON_MHG ),
    cover_node_weapon_enum_pair("MSN",   cover_node::COVER_WEAPON_MSN ),
    cover_node_weapon_enum_pair("BBG",   cover_node::COVER_WEAPON_BBG ),
    cover_node_weapon_enum_pair("MUT",   cover_node::COVER_WEAPON_MUT ),

    cover_node_weapon_enum_pair( k_EnumEndStringConst, cover_node::COVER_WEAPON_COUNT) //**MUST BE LAST**//
};
enum_table<cover_node::eCoverWeaponType>  s_CoverNodeWeaponEnumTable(s_CoverNodeWeaponEnumPair);

#endif


//=========================================================================
// DEBUG RENDER ANIMATION FUNCTIONS
//=========================================================================

#ifdef X_EDITOR

void PrepD3DForStencil( void )
{
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1  );
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE      );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE     );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1  );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE      );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE     );

    g_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,  TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,    D3DCMP_ALWAYS );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILMASK,    0xFFFFFFFF );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,    D3DSTENCILOP_KEEP );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL,   D3DSTENCILOP_KEEP );
    g_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,    D3DSTENCILOP_REPLACE  );

    g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0x0 );
    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ONE );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ZERO );
}

void RenderFullScreenQuad( void )
{
    const view* pView = eng_GetView();
    matrix4 L2W;
    L2W.Identity();
    L2W.Scale( vector3(1000,1000,50) );
    L2W.RotateX( pView->GetViewZ().GetPitch() );
    L2W.RotateY( pView->GetViewZ().GetYaw() );
    L2W.Translate( pView->GetPosition() );
    draw_SetL2W(L2W);
    draw_Vertex(+1,+1,1);
    draw_Vertex(+1,-1,1);
    draw_Vertex(-1,+1,1);
    draw_Vertex(-1,+1,1);
    draw_Vertex(+1,-1,1);
    draw_Vertex(-1,-1,1);
    draw_End();
    draw_ClearL2W();
}

void RenderFrustum( const vector3* pInnerFrame, const vector3* pOuterFrame, xbool bInside )
{
    s32 i,j;

    vector3 PTA[4] = {pInnerFrame[0],pInnerFrame[1],pInnerFrame[2],pInnerFrame[3]};
    vector3 PTB[4] = {pOuterFrame[0],pOuterFrame[1],pOuterFrame[2],pOuterFrame[3]};
    vector3 CenterA = (PTA[0] + PTA[1] + PTA[2] + PTA[3]) / 4.0f;
    vector3 CenterB = (PTB[0] + PTB[1] + PTB[2] + PTB[3]) / 4.0f;


    // Setup the render arrays
    //                           0      1      2      3      4      5      6      7
    vector3 VertexList[8] = {PTA[3],PTA[2],PTA[1],PTA[0],PTB[3],PTB[2],PTB[1],PTB[0]};
    s16     IndexList[] = {0,4,1, 1,4,5, 
                           1,5,2, 2,5,6, 
                           2,6,3, 3,6,7, 
                           3,7,0, 0,7,4,
                           0,1,2, 0,2,3, 
                           4,7,6, 4,6,5 };

    {
        if( bInside )
        {
            // Clear stencil buffer
            {
                draw_Begin( DRAW_TRIANGLES );
                draw_Color(xcolor(64,64,64,0));
                PrepD3DForStencil();
                g_pd3dDevice->SetRenderState( D3DRS_STENCILREF,     0xFFFFFFFF );
                g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
                RenderFullScreenQuad();
            }

            // Render Frustum
            {
                draw_Begin( DRAW_TRIANGLES );
                draw_Color(xcolor(64,64,64,0));
                PrepD3DForStencil();
                g_pd3dDevice->SetRenderState( D3DRS_STENCILREF,     0x0 );
                g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
                draw_Verts( (vector3*)VertexList, sizeof(VertexList)/sizeof(vector3) );
                draw_Execute( IndexList, sizeof(IndexList)/sizeof(s16) );
                draw_End();
            }
        }
        else
        {
            // Clear stencil buffer
            {
                draw_Begin( DRAW_TRIANGLES );
                draw_Color(xcolor(64,64,64,0));
                PrepD3DForStencil();
                g_pd3dDevice->SetRenderState( D3DRS_STENCILREF,     0x0 );
                g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
                RenderFullScreenQuad();
            }

            // Render Front
            {
                draw_Begin( DRAW_TRIANGLES );
                draw_Color(xcolor(64,64,64,0));
                PrepD3DForStencil();
                g_pd3dDevice->SetRenderState( D3DRS_STENCILREF,     0xFFFFFFFF );
                g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
                draw_Verts( (vector3*)VertexList, sizeof(VertexList)/sizeof(vector3) );
                draw_Execute( IndexList, sizeof(IndexList)/sizeof(s16) );
                draw_End();
            }

            // Clear Back
            {
                draw_Begin( DRAW_TRIANGLES );
                draw_Color(xcolor(0,0,0,0));
                PrepD3DForStencil();
                g_pd3dDevice->SetRenderState( D3DRS_STENCILREF,     0x0 );
                g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
                draw_Verts( (vector3*)VertexList, sizeof(VertexList)/sizeof(vector3) );
                draw_Execute( IndexList, sizeof(IndexList)/sizeof(s16) );
                draw_End();
            }
        }

        // Darken outside of frustum
        {
            draw_Begin( DRAW_TRIANGLES );
            draw_Color(xcolor(64,64,64,255));
    	    
            PrepD3DForStencil();
            g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,       FALSE );
            g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE,   0x0F );
            g_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,        D3DCMP_EQUAL );
            g_pd3dDevice->SetRenderState( D3DRS_STENCILREF,         0 );
            g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
            g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,           D3DBLEND_ZERO );
            g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,          D3DBLEND_SRCCOLOR );
            g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,           D3DCULL_CW );

            RenderFullScreenQuad();
        }

        // Illuminate inside of frustum
        {
            draw_Begin( DRAW_TRIANGLES );
            draw_Color(xcolor(45,45,45,255));
    	    
            PrepD3DForStencil();
            g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,       FALSE );
            g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE,   0x0F );
            g_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,        D3DCMP_EQUAL );
            g_pd3dDevice->SetRenderState( D3DRS_STENCILREF,         0xFFFFFFFF );
            g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
            g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,           D3DBLEND_ONE );
            g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,          D3DBLEND_ONE );
            g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,           D3DCULL_CW );

            RenderFullScreenQuad();
        }

        g_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,  FALSE );
        g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
        g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
        g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0x0F );
    }
    
    // Render cage
    {
        draw_Line( CenterA, CenterB, XCOLOR_GREEN );
        for( i=0; i<4; i++ )
        {
            draw_Line( PTA[i], PTB[i], XCOLOR_GREEN );
            draw_Line( PTB[i], PTB[(i+1)&3], XCOLOR_GREEN );
        }

        f32 Dist = (CenterA - CenterB).Length();
        //x_DebugMsg("----------------------\n%f\n",Dist);
        f32 D = 0;
        while( 1 )
        {
            x_DebugMsg("%f\n",D);
            f32 T = D / Dist;
            vector3 P[4];
            for( j=0; j<4; j++ )
                P[j] = PTA[j] + T*(PTB[j] - PTA[j]);

            for( s32 j=0; j<4; j++ )
            {
                draw_Line( P[j], P[(j+1)&3], XCOLOR_GREEN );
            }

            if( D==Dist )
                break;

            D += 200.0f;
            if( D>Dist ) D = Dist;
        }
    }
}

// Draws a lit quad
void draw_LitQuad( const vector3& A, 
                   const vector3& B, 
                   const vector3& C, 
                   const vector3& D,
                   const vector3& LightDir,
                         f32      LightDirI,
                         f32      LightAmbI,
                         xcolor   LightColor ) 
{
    // Compute lighting
    vector3 N  = (A - C).Cross(B - A) ;
    N.Normalize() ;
    f32 Dot = N.Dot(LightDir) * LightDirI ;
    if (Dot < 0)
        Dot = 0 ;
    Dot += LightAmbI ;
    if (Dot > 1)
        Dot = 1 ;

    // Compute color
    LightColor.R = (u8)((f32)LightColor.R * Dot) ;
    LightColor.G = (u8)((f32)LightColor.G * Dot) ;
    LightColor.B = (u8)((f32)LightColor.B * Dot) ;

    // Draw quad
    draw_Color( LightColor );
    draw_Vertex(D) ;
    draw_Vertex(C) ;
    draw_Vertex(B) ;
    draw_Vertex(A) ;
}

//=========================================================================

// Draws a solid, lit, bbox
void draw_LitSolidBBox( const bbox&    BBox,
                        const matrix4& L2W,
                        const vector3& WorldLightDir,
                              f32      LightDirI,
                              f32      LightAmbI,
                              xcolor   LightColor )
{
    // Compute corner points in local space
    vector3 P[8];
    P[0].GetX() = BBox.Min.GetX();    P[0].GetY() = BBox.Min.GetY();    P[0].GetZ() = BBox.Min.GetZ(); 
    P[1].GetX() = BBox.Min.GetX();    P[1].GetY() = BBox.Min.GetY();    P[1].GetZ() = BBox.Max.GetZ(); 
    P[2].GetX() = BBox.Min.GetX();    P[2].GetY() = BBox.Max.GetY();    P[2].GetZ() = BBox.Min.GetZ(); 
    P[3].GetX() = BBox.Min.GetX();    P[3].GetY() = BBox.Max.GetY();    P[3].GetZ() = BBox.Max.GetZ(); 
    P[4].GetX() = BBox.Max.GetX();    P[4].GetY() = BBox.Min.GetY();    P[4].GetZ() = BBox.Min.GetZ(); 
    P[5].GetX() = BBox.Max.GetX();    P[5].GetY() = BBox.Min.GetY();    P[5].GetZ() = BBox.Max.GetZ(); 
    P[6].GetX() = BBox.Max.GetX();    P[6].GetY() = BBox.Max.GetY();    P[6].GetZ() = BBox.Min.GetZ(); 
    P[7].GetX() = BBox.Max.GetX();    P[7].GetY() = BBox.Max.GetY();    P[7].GetZ() = BBox.Max.GetZ(); 

    // Setup light direction in local space
    matrix4 W2L ;
    W2L = L2W ;
    W2L.InvertSRT() ;
    vector3 LightDir = W2L.RotateVector(WorldLightDir) ;
    LightDir.Normalize() ;

    // Draw bbox
    draw_SetL2W(L2W) ;
    draw_Begin( DRAW_QUADS, (LightColor.A == 255) ? 0 : DRAW_USE_ALPHA );
    draw_LitQuad(P[0], P[2], P[6], P[4], LightDir, LightDirI, LightAmbI, LightColor) ; // F
    draw_LitQuad(P[1], P[5], P[7], P[3], LightDir, LightDirI, LightAmbI, LightColor) ; // B
    draw_LitQuad(P[4], P[6], P[7], P[5], LightDir, LightDirI, LightAmbI, LightColor) ; // R
    draw_LitQuad(P[0], P[1], P[3], P[2], LightDir, LightDirI, LightAmbI, LightColor) ; // L
    draw_LitQuad(P[2], P[3], P[7], P[6], LightDir, LightDirI, LightAmbI, LightColor) ; // T
    draw_LitQuad(P[0], P[4], P[5], P[1], LightDir, LightDirI, LightAmbI, LightColor) ; // B
    draw_End();
    draw_ClearL2W() ;
}

//=========================================================================

// Renders solid,lit bbox for bone
void RenderBone ( const  vector3& A, 
                  const  vector3& B, 
                  f32    FloorY, 
                  xcolor Color,
                  xcolor ShadowColor )
{
    vector3 AxisX, AxisY, AxisZ, Pos;

    // Setup rotation
    AxisZ = B - A ;
    AxisZ.Normalize() ;
    AxisX = v3_Cross(AxisZ, vector3(0,1,0)) ;
    AxisX.Normalize() ;
    AxisY = v3_Cross(AxisX, AxisZ) ;
    AxisY.Normalize() ;

    // Setup translation
    Pos = (A + B) * 0.5f ;

    // Create L2W
    matrix4 L2W ;
    L2W.Identity() ;
    L2W.SetColumns( AxisX, AxisY, AxisZ ) ;
    L2W.SetTranslation( Pos );

    // Create bbox
    f32  L = (B - A).Length() * 0.5f ;
    f32  S = 6.0f ;
    bbox BBox ;
    BBox.Min.GetX() = -S ;
    BBox.Min.GetY() = -S ;
    BBox.Min.GetZ() = -L ;
    BBox.Max.GetX() = S ;
    BBox.Max.GetY() = S ;
    BBox.Max.GetZ() = L ;

    // Render bbox
    draw_LitSolidBBox(BBox, L2W, vector3(1,1,1), 0.8f, 0.3f, Color) ;

    // Compute shadow matrix (just force Y to FloorY)
    L2W(0,1) = 0.0f ;
    L2W(1,1) = 0.0f ;
    L2W(2,1) = 0.0f ;
    L2W(3,1) = FloorY ;

    // Render shadow
    draw_LitSolidBBox(BBox, L2W, vector3(1,1,1), 0.0f, 1.0f, ShadowColor) ;
}

//=========================================================================

// Render specified frame of animation
void RenderFrame( const matrix4& L2W, const anim_group& AnimGroup, s32 iAnim, f32 Frame, xcolor Color )
{
    s32 i ;

    // Setup shadow info
    xcolor ShadowColor = Color ;
    f32    FloorY      = L2W.GetTranslation().GetY() + 10.0f ;
    ShadowColor.R >>= 3 ;
    ShadowColor.G >>= 3 ;
    ShadowColor.B >>= 3 ;

    // Lookup bone count
    s32 nBones = AnimGroup.GetNBones() ;

    // Lookup anim info
    const anim_info& AnimInfo = AnimGroup.GetAnimInfo(iAnim) ;

    // Allocate temporary memory for keys and matrices
    smem_StackPushMarker();
    byte* pData = smem_StackAlloc( nBones * ( sizeof(anim_key) + sizeof(matrix4) ) );
    if (!pData)
    {
        smem_StackPopToMarker();
        return ;
    }

    // Setup ptrs
    anim_key* pKeys     = (anim_key*)pData ;
    matrix4*  pMatrices = (matrix4*)(pData + ( nBones * sizeof(anim_key) ) );

    // Compute matrices in world space
    AnimInfo.GetInterpKeys(Frame, pKeys, nBones) ;

    // Make anim translation relative to frame0 just like loco
    anim_key Key0;
    s32 iMotionProp = AnimInfo.GetPropChannel("MotionProp");
    if ( iMotionProp != -1 )
        AnimInfo.GetPropRawKey( iMotionProp, 0, Key0 );
    else
        AnimInfo.GetRawKey(0, 0, Key0) ;
        
    pKeys[0].Translation.GetX() -= Key0.Translation.GetX() ;
    pKeys[0].Translation.GetZ() -= Key0.Translation.GetZ() ;

    // Compute final matrices
    AnimGroup.ComputeBonesL2W( L2W, pKeys, nBones, pMatrices, FALSE ) ;
   
    // Render bones and joints
    for( i=0; i<nBones; i++ )
    {
        // Lookup joint
        vector3 BP = pMatrices[i].GetTranslation() ;
        vector3 PP = BP;

        // Compute parent joint?
        s32 iParent = AnimGroup.GetBone(i).iParent ;
        if( iParent != -1 )
            PP = pMatrices[ iParent ].GetTranslation() ;

        // Draw bone
        RenderBone(BP, PP, FloorY, Color, ShadowColor) ;
    }

    // Free alloced memory
    smem_StackPopToMarker();
}

//===============================================================================

// Renders cover anim
void RenderAnim( const matrix4&             L2W, 
                       anim_group::handle   hAnimGroup, 
                       s32                  iAnim )
{
    // Lookup anim info
    const anim_group& AnimGroup = *hAnimGroup.GetPointer() ;
    const anim_info&  AnimInfo  = AnimGroup.GetAnimInfo(iAnim) ;
    
    // Default color
    xcolor Color = XCOLOR_GREEN ;

    // Game running?
    if (g_game_running)
    {
        // Lookup frame
        static f32 PrevFrame = 0 ;
        f32 Frame = (f32)x_TicksToSec(g_ObjMgr.GetGameTime()) * (f32)AnimInfo.GetFPS() ;
        Frame = x_fmod(Frame,(f32)(AnimInfo.GetNFrames() - 1)) ;
        
        // Look for fire events
        for (s32 i = 0 ; i < AnimInfo.GetNEvents() ; i++)
        {
            if (AnimInfo.IsEventActive(i, Frame, PrevFrame))
            {
                // Get event info
                const anim_event& Event     = AnimInfo.GetEvent(i) ;
                const char*       EventType = Event.GetType() ;

                // Weapon event?
                if(x_strcmp(EventType, "Weapon") == 0)
                {
                    Color = XCOLOR_RED ;
                    break ;
                }
            }
        }

        // Render
        RenderFrame(L2W, AnimGroup, iAnim, Frame, Color) ;

        PrevFrame = Frame ;
    }
    else
    {
        // Render frame 0
        RenderFrame(L2W, AnimGroup, iAnim, 0, XCOLOR_GREEN) ;
        draw_Label(L2W.GetTranslation(), XCOLOR_WHITE, "%s\n\n%s", hAnimGroup.GetName(), AnimInfo.GetName()) ;

        // Look for the 1st fire frame
        for (s32 i = 0 ; i < AnimInfo.GetNEvents() ; i++)
        {
            // Get event info
            const anim_event& Event     = AnimInfo.GetEvent(i) ;
            const char*       EventType = Event.GetType() ;

            // Weapon event?
            if(x_strcmp(EventType, "Weapon") == 0)
            {
                // Render weapon fire
                RenderFrame(L2W, AnimGroup, iAnim, (f32)Event.StartFrame(), XCOLOR_RED) ;
                break ;                
            }
        }
    }
}

#endif  // #ifdef X_EDITOR



//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct cover_node_desc : public object_desc
{
        cover_node_desc( void ) : object_desc( 
            object::TYPE_COVER_NODE, 
            "Cover Object", 
            "AI",
#ifdef X_RETAIL
            object::ATTR_NEEDS_LOGIC_TIME 
#else
            object::ATTR_NEEDS_LOGIC_TIME | 
            object::ATTR_RENDERABLE
#endif
            ,           
            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_IS_DYNAMIC )   {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new cover_node; } 

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        if( Object.IsActive() )
        {
            EditorIcon_Draw( EDITOR_ICON_COVER_NODE, Object.GetL2W(), FALSE, XCOLOR_GREEN );
            return -1;            
        }
        else
        {
            EditorIcon_Draw( EDITOR_ICON_COVER_NODE, Object.GetL2W(), FALSE, XCOLOR_GREY );
            return -1;            
        }
    }

#endif // X_EDITOR

} s_cover_node_desc;

//===============================================================================
/*
cover_node::cover_pair cover_node::s_PairTable[] = 
{
     cover_pair( "CROUCH_COVER",      COVER_CROUCH ),
     cover_pair( "STRAFE_COVER",      COVER_STRAFE ),
         
     cover_pair( k_EnumEndStringConst, COVER_INVALID ),  
};

cover_node::cover_table cover_node::s_EnumTable( s_PairTable );
*/
//===============================================================================

cover_node::cover_node() :
    m_ReserveTimer(0),
    m_ReservedGuid(0),
    m_Flags( FLAGS_DIRTY_PLANES ),
    m_bFirstReservation(TRUE)
{
        m_Width             = 1000.0f;
        m_Height            = 1000.0f;
        m_MaxDistance       = 1000.0f;
        m_MinDistance       = 100.0f;
        m_HasValidatedWeaponNPC = FALSE;
        m_ShootWeight       = -1;
        m_GrenadeWeight     = -1;
        m_ActionDelayMin    = -1.0f;
        m_ActionDelayMax    = -1.0f;
        m_Destructable      = 0;
        m_bCoverToAll       = FALSE;
        m_bActive           = TRUE;
        m_NextStickyNode    = 0;
}

//===============================================================================

cover_node::~cover_node()
{
}

//===============================================================================

const object_desc& cover_node::GetTypeDesc( void ) const
{
    return s_cover_node_desc;
}

//===============================================================================

s32 cover_node::GetMaterial( void ) const
{
    return MAT_TYPE_NULL ;
}

//===============================================================================

void cover_node::ComputeVerts( vector3* pVert ) const
{
    f32 W = m_Width/2;
    f32 H = m_Height/2;

    pVert[0].Set( -W,  H, m_MaxDistance );
    pVert[1].Set( -W, -H, m_MaxDistance );
    pVert[2].Set(  W, -H, m_MaxDistance );
    pVert[3].Set(  W,  H, m_MaxDistance );
}

//===============================================================================

void cover_node::ComputePlanes( void ) 
{
    if( m_Flags & FLAGS_DIRTY_PLANES )
    {
        vector3 Vertex[5];

        // Compute the vertex
        ComputeVerts( Vertex );
        GetL2W().Transform( Vertex, Vertex, 4 );

        // Compute the planes.  The planes face inward
        m_Plane[0].Setup( GetPosition(), Vertex[0], Vertex[1] );
        m_Plane[1].Setup( GetPosition(), Vertex[1], Vertex[2] );
        m_Plane[2].Setup( GetPosition(), Vertex[2], Vertex[3] );
        m_Plane[3].Setup( GetPosition(), Vertex[3], Vertex[0] );
        m_Plane[4].Setup( Vertex[0], Vertex[3], Vertex[2] ); // Far plane
        m_Plane[5].Setup( Vertex[0], Vertex[1], Vertex[2] ); // Near plane

        vector3 MinPt(0,0,m_MinDistance);
        MinPt = GetL2W() * MinPt;
        m_Plane[5].ComputeD( MinPt );

        // Now we can clear our flag
        m_Flags &= ~FLAGS_DIRTY_PLANES;
    }
}

//=========================================================================================================

xbool cover_node::IsCoverFromLocation( const vector3& targetLocation )
{
    if( GetIsCoverToAll() )
    {
        return TRUE;
    }

    ComputePlanes();

    // Is point inside all 6 sides of the frustum?
    for( s32 i=0; i<6; i++ )
    {
        if( m_Plane[i].Distance( targetLocation ) < 0 )
            return FALSE;
    }

    // Point was inside all planes
    return TRUE;   
}

//===============================================================================

bbox cover_node::GetLocalBBox( void ) const
{
    return bbox( vector3(-50, -50, -50), vector3(50,50,50) );
}

//===============================================================================

void cover_node::OnAdvanceLogic( f32 DeltaTime )
{
    (void)DeltaTime;
    if( m_Destructable )
    {
        object *tempObj = g_ObjMgr.GetObjectByGuid(m_Destructable);
        // if our destructable no longer exists, neither do we.
        if( !tempObj )
        {
            g_ObjMgr.DestroyObject( GetGuid() );
            return;
        }
        else if ( tempObj->IsKindOf(super_destructible_obj::GetRTTI()) )
        {
            super_destructible_obj& superDestructableObj = super_destructible_obj::GetSafeType( *tempObj );
            // if our super destructable is destroyed, so are we
            if( superDestructableObj.IsDestroyed() )
            {                
                g_ObjMgr.DestroyObject( GetGuid() );
                return;
            }
        }
        else if ( tempObj->IsKindOf(destructible_obj::GetRTTI()) )
        {
            destructible_obj& destructableObj = destructible_obj::GetSafeType( *tempObj );
            // if our super destructable is destroyed, so are we
            if( destructableObj.IsDestroyed() )
            {                
                g_ObjMgr.DestroyObject( GetGuid() );
                return;
            }
        }
    }
}

//===============================================================================

void cover_node::OnMove( const vector3& NewPos )
{
    object::OnMove( NewPos );
    m_Flags |= FLAGS_DIRTY_PLANES;
}


//===============================================================================

void cover_node::OnTransform( const matrix4& L2W )
{
    object::OnTransform( L2W );
    m_Flags |= FLAGS_DIRTY_PLANES;
}

//===============================================================================

void cover_node::OnRender( void )
{
#ifdef X_EDITOR
        
    // Render line and bbox
    if(m_ReservedGuid && m_ReservedGuid != GetGuid())
    {
        object* pObject = g_ObjMgr.GetObjectByGuid(m_ReservedGuid);
        if(pObject)
            draw_Line(pObject->GetBBox().GetCenter(), GetBBox().GetCenter(), XCOLOR_RED);
    }
  
    // Render anims if selected
    if( GetAttrBits() & object::ATTR_EDITOR_SELECTED )
    {
        // Compute L2W
        matrix4 L2W = GetL2W() ;
        L2W.PreRotateY( R_180 ) ;

        // Snap to ground
        vector3 Start( GetPosition() ) ;
        vector3 End  ( Start );
        Start.GetY() += 10.0f;   // sometimes the root can be below the ground, so nudge it up a bit
        End.GetY()   -= 200.0f;  // give it 2 meters to hit the ground
        g_CollisionMgr.RaySetup( 0, Start, End );
        g_CollisionMgr.CheckCollisions(object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_LIVING);
        if (g_CollisionMgr.m_nCollisions)
            Start = g_CollisionMgr.m_Collisions[0].Point ;
        L2W.SetTranslation(Start) ;

        // Render animations
        for ( s32 i = COVER_PACKAGE_NONE+1 ; i < COVER_PACKAGE_COUNT ; i++)
        {
            // Skip this package?
            if( ( s_CoverNodeDebugPackage != COVER_PACKAGE_NONE ) && ( s_CoverNodeDebugPackage != i ) )
                continue;

            // Lookup anim group
            anim_group::handle& hAnimGroup = GetAnimGroupHandle(i) ;
            const anim_group* pAnimGroup = hAnimGroup.GetPointer() ;
            if (!pAnimGroup)
                continue ;

            // Get reference to make code easier to read
            const anim_group& AnimGroup = *pAnimGroup ;

            // Check all anims
            for ( s32 j = 0 ; j < AnimGroup.GetNAnims() ; j++)
            {
                // Render fire anim
                const char* pName = AnimGroup.GetAnimInfo(j).GetName() ;

                // Skip cycle shooting
                if ( x_stristr(pName, "COVER_SHOOT_BEGIN") )
                    continue;
                if ( x_stristr(pName, "COVER_SHOOT_END") )
                    continue;
                if ( x_stristr(pName, "COVER_SHOOT_CYCLE") )
                    continue;

                // Look for cover shoot or exit cover
                if ( x_stristr(pName, "COVER_SHOOT") == NULL &&
                     x_stristr(pName, "COVER_EXIT") == NULL )
                    continue;

                // Use correct weapon?
                if (s_CoverNodeDebugWeapon != COVER_WEAPON_NONE )
                {
                    // Must have weapon name in anim name
                    if (x_stristr(pName, s_CoverNodeWeaponEnumTable.GetString( s_CoverNodeDebugWeapon )) == NULL )
                        continue;
                }

                // Render anim
                RenderAnim(L2W, hAnimGroup, j) ;
            }
        }
    }
#endif

#if defined( TARGET_PS2 ) && (!defined ( X_RETAIL ) || defined(X_QA))

    if(character::s_bDebugInGame)
    {
        // Without this, the debug drawing is corrupt
        eng_WriteToBackBuffer();

        // Compute cover node render bbox
        bbox BBox ;
        BBox.Min = GetPosition() - vector3(25.0f, 0.0f, 25.0f) ;
        BBox.Max = GetPosition() + vector3(25.0f, 25.0f, 25.0f) ;
        draw_ClearL2W();

        // Is the cover node reserved?
        xcolor Color = XCOLOR_GREEN;
        if( GetGuid() != m_ReservedGuid )
        {
            // Is cover reserver using the cover node?
            object* pObject = g_ObjMgr.GetObjectByGuid( m_ReservedGuid );
            if( ( pObject ) && ( pObject->IsKindOf( character::GetRTTI() ) ) )
            {
                // Is character using this cover node?
                character* pCharacter = (character*)pObject;
                if( pCharacter->GetCurrentCover() == GetGuid() )
                {
                    // Draw a line from player to cover node            
                    draw_Line( BBox.GetCenter(), pCharacter->GetBBox().GetCenter(), XCOLOR_RED );

                    // Draw bbox in red
                    Color = XCOLOR_RED;
                }
            }
        }

        // Draw cover node
        draw_BBox( BBox, Color );
    }
#endif  //TARGET_PS2 && (!X_RETAIL || X_QA)

}

//===============================================================================

#ifndef X_RETAIL
void cover_node::OnDebugRender( void )
{
    // don't render frustum if is cover to all.
    if( GetIsCoverToAll() )
    {
        return;
    }
    //
    // Render the hold only when selected
    //
    //xcolor  ViewColor( XCOLOR_RED ) ;

    vector3 vDebugDirection( 0.f , 10.f , 300.f );
    vDebugDirection.RotateY( GetL2W().GetRotation().Yaw );
    draw_Line( GetPosition() + vector3( 0.f , 10.f , 0.f ) , GetPosition() + vDebugDirection , XCOLOR_GREEN );

    //
    // Render frustum
    //
    #ifdef X_EDITOR
    {
        s32 i;
        vector3 InnerVertex[4];
        vector3 OuterVertex[4];
        ComputeVerts( OuterVertex );

        GetL2W().Transform( OuterVertex, OuterVertex, 4 );
        for( i=0; i<4; i++ )
        {
            vector3 Dir = OuterVertex[i] - GetPosition();
            f32 Len = Dir.Length();
            Dir.NormalizeAndScale( Len * (m_MinDistance/m_MaxDistance) );
            InnerVertex[i] = GetPosition() + Dir;
        }
        RenderFrustum( InnerVertex, OuterVertex, IsCoverFromLocation( eng_GetView()->GetPosition() ) );
    }
    #endif
}
#endif // X_RETAIL

//===============================================================================

void cover_node::OnActivate( xbool Flag )
{
    m_bActive = Flag;
}

//===============================================================================
radian cover_node::GetNPCFacing()
{
    return GetL2W().GetRotation().Yaw;
}

//===============================================================================

xbool cover_node::HasValidAnims2( object::type characterType, const char* logicalName, inven_item WeaponItem )
{
    if( !m_HasValidatedWeaponNPC )
    {
        ValidateWeaponNPC();
        m_HasValidatedWeaponNPC = TRUE;
    }

    s32 packageType = GetAnimPackageType( characterType, logicalName );
    s32 weaponType = GetWeaponType2( WeaponItem );

    if( packageType >= 0 &&
        packageType < COVER_PACKAGE_COUNT &&
        weaponType >= 0 &&
        weaponType < COVER_WEAPON_COUNT )
    {
        return m_ValidWeaponNPC[packageType][weaponType];
    }
    else
    {
        return FALSE;
    }
}

//===============================================================================

void cover_node::ValidateWeaponNPC()
{
    s32 c;
    for(c=0;c<COVER_PACKAGE_COUNT;c++)
    {
        const char*animGroupName = GetAnimGroupNameByType(c);
        if( animGroupName != NULL )
        {
            anim_group::handle groupHandle;
            groupHandle.SetName( animGroupName );
            if( groupHandle.GetIndex() != -1 )
            {            
                anim_group *ourAnimGroup = groupHandle.GetPointer();
                if( ourAnimGroup != NULL )
                {
                    s32 d;
                    for(d=0;d<COVER_WEAPON_COUNT;d++)
                    {
                        if( ourAnimGroup->GetAnimIndex(GetIdleAnimNameForWeapon((eCoverWeaponType)d)) >= 0 )
                        {
                            m_ValidWeaponNPC[c][d] = TRUE;
                        }
                        else
                        {
                            m_ValidWeaponNPC[c][d] = FALSE;
                        }
                    }   
                    continue;
                }
            }
        }
        //something didn't work out, set all of them to false.
        s32 d;
        for(d=0;d<COVER_WEAPON_COUNT;d++)
        {
            m_ValidWeaponNPC[c][d] = FALSE;
        }
    }
}

//===============================================================================

const char* cover_node::GetIdleAnimNameForWeapon( eCoverWeaponType weaponType )
{
    switch( weaponType )
    {
    case COVER_WEAPON_SMP:
        return "SMP_COVER_IDLE";
        break;
    case COVER_WEAPON_SHT:
        return "SHT_COVER_IDLE";
        break;
    case COVER_WEAPON_SNI:
        return "SNI_COVER_IDLE";
        break;
    case COVER_WEAPON_GAS:
        return "GAS_COVER_IDLE";
        break;
    case COVER_WEAPON_EGL:
        return "EGL_COVER_IDLE";
        break;
    case COVER_WEAPON_MHG:
        return "MHG_COVER_IDLE";
        break;
    case COVER_WEAPON_MSN:
        return "MSN_COVER_IDLE";
        break;
    case COVER_WEAPON_BBG:
        return "SMP_COVER_IDLE"; // KSS -- FIXME -- wrong cover
        break;
    default:
        return "COVER_IDLE";
        break;
    }
}    

//===============================================================================

cover_node::eCoverWeaponType cover_node::GetWeaponType2( inven_item WeaponItem )
{
    switch( WeaponItem )
    {    
    case INVEN_WEAPON_SMP:
        return COVER_WEAPON_SMP;
        break;

// KSS -- TO ADD NEW WEAPON
    case INVEN_WEAPON_SHOTGUN:
        return COVER_WEAPON_SHT;
        break;
    case INVEN_WEAPON_SNIPER_RIFLE:
        return COVER_WEAPON_SNI;
        break;
    case INVEN_WEAPON_DESERT_EAGLE:
        return COVER_WEAPON_EGL;
        break;
    case INVEN_WEAPON_MESON_CANNON:
        return COVER_WEAPON_MSN;
        break;
    case INVEN_WEAPON_BBG:
        return COVER_WEAPON_SMP; // KSS -- FIXME -- wrong cover for BBG
        break;
    case INVEN_WEAPON_TRA:
        return COVER_WEAPON_TRA;
        break;
    case INVEN_WEAPON_MUTATION:
        return COVER_WEAPON_MUT;
        break;
    default:
        return COVER_WEAPON_NONE;
        break;
    }
}

//=========================================================================================================

void cover_node::InvalidateNode()
{
    ReserveNode(GetGuid());
}

//=========================================================================================================

void cover_node::ReserveNode( guid NewUser ) 
{ 
    m_bFirstReservation = FALSE;
    m_ReserveTimer = g_ObjMgr.GetGameTime();
    m_ReservedGuid = NewUser;
}

//=========================================================================================================

xbool cover_node::IsReserved( guid Requester ) 
{ 
    //if first time used
    if (m_bFirstReservation)
        return FALSE;

    f32 TimePassed = g_ObjMgr.GetGameDeltaTime( m_ReserveTimer );

    //if used by same requester, then allow use after shorter wait
    if (m_ReservedGuid == Requester)
    {
        return ( TimePassed < k_MinPreferedReserveTime);
    }

    //no reserved guid
    if (!m_ReservedGuid)
    {
        return FALSE;
    }

/*    //if reservation holder is no longer in existence, then allow use
    if (!g_ObjMgr.GetObjectByGuid(m_ReservedGuid))
    {
        m_ReservedGuid = NULL_GUID;
        return FALSE;
    }*/

    //reserves for 8 seconds
    return ( TimePassed < k_MinReserveTime );
}

//=========================================================================================================

anim_group::handle& cover_node::GetAnimGroupHandle( s32 PackageType )
{
    switch( PackageType )
    {
    default:
        ASSERTS(0," You need to add  your new cover package enum here!");
    
    case COVER_PACKAGE_SOLDIER:
        return m_hSoldierAnimGroup ;
    
    case COVER_PACKAGE_CIVILIAN:
        return m_hCivilianAnimGroup ;

    case COVER_PACKAGE_GRUNT:
        return m_hGruntAnimGroup ;

    case COVER_PACKAGE_LEAPER:
        return m_hLeaperAnimGroup ;

    }
}

//=========================================================================================================

const char* cover_node::GetAnimGroupName( object::type characterType, const char* logicalName )
{
    switch( characterType )
    {
    case object::TYPE_GRUNT:
        // distinguish between leaper and grunt
        if( x_strcmp(logicalName,"Leaper") == 0 )
        {
            return m_hLeaperAnimGroup.GetName();
        }
        else if ( x_strcmp(logicalName,"Grunt_Scientist") == 0 )
        {
            return NULL;
        }
        else
        {        
            return m_hGruntAnimGroup.GetName();
        }
        break;
    case object::TYPE_FRIENDLY_SOLDIER:
    case object::TYPE_BLACK_OPPS:
    case object::TYPE_HAZMAT:
        return m_hSoldierAnimGroup.GetName();
        break;
    case object::TYPE_FRIENDLY_SCIENTIST:
        return m_hCivilianAnimGroup.GetName();
        break;
    }
    return NULL;
}

//=========================================================================================================

cover_node::eCoverAnimPackageType cover_node::GetAnimPackageType( object::type characterType, const char* logicalName )
{
    switch( characterType )
    {
    case object::TYPE_GRUNT:
        if( x_strcmp(logicalName,"Leaper") == 0 )
        {
            return COVER_PACKAGE_LEAPER;
        }
        else if ( x_strcmp(logicalName,"Grunt_Scientist") == 0 )
        {
            return COVER_PACKAGE_NONE;
        }
        else
        {        
            return COVER_PACKAGE_GRUNT;
        }
        break;
    case object::TYPE_FRIENDLY_SOLDIER:
    case object::TYPE_BLACK_OPPS:
    case object::TYPE_HAZMAT:
        return COVER_PACKAGE_SOLDIER;
        break;
    case object::TYPE_FRIENDLY_SCIENTIST:
        return COVER_PACKAGE_CIVILIAN;
        break;
    default:
        return COVER_PACKAGE_NONE;
        break;
    }
}

//=========================================================================================================

const char* cover_node::GetAnimGroupNameByType( s32 packageType )
{
    switch( packageType )
    {
    case COVER_PACKAGE_CIVILIAN:
        return m_hCivilianAnimGroup.GetName();
        break;
    case COVER_PACKAGE_SOLDIER:
        return m_hSoldierAnimGroup.GetName();
        break;
    case COVER_PACKAGE_GRUNT:
        return m_hGruntAnimGroup.GetName();
        break;
    case COVER_PACKAGE_LEAPER:
        return m_hLeaperAnimGroup.GetName();
        break;
    default:
        return NULL;
        break;
    }
}

//=========================================================================================================

s32 cover_node::GetNumValidCoverPackages( void )
{
    if( !m_HasValidatedWeaponNPC )
    {
        ValidateWeaponNPC();
        m_HasValidatedWeaponNPC = TRUE;
    }

    s32 nGroups = 0;
    s32 i,j;
    for (i=0;i<COVER_PACKAGE_COUNT;i++)
    {
        for (j=0;j<COVER_WEAPON_COUNT;j++)
        {
            if ( m_ValidWeaponNPC[i][j] )
            {
                nGroups++;
                break;
            }
        }
    }
    return nGroups;
}

//=========================================================================================================

void cover_node::OnEnumProp( prop_enum& rPropList )
{
    object::OnEnumProp  ( rPropList ) ;
    rPropList.PropEnumHeader ( "Cover Node", "Information for the cover node." , PROP_TYPE_HEADER ) ;
    rPropList.PropEnumGuid   ( "Cover Node\\Next Sticky", "The covernode or group grunts will go to after leaving this one.", 0 ) ;
    
    // Animation file
    rPropList.PropEnumExternal("Cover Node\\Civilian Cover Anim Package", "Resource\0anim\0", "Resource File", PROP_TYPE_MUST_ENUM);
    rPropList.PropEnumExternal("Cover Node\\Soldier Cover Anim Package", "Resource\0anim\0", "Resource File", PROP_TYPE_MUST_ENUM);
    rPropList.PropEnumExternal("Cover Node\\Grunt Cover Anim Package", "Resource\0anim\0", "Resource File", PROP_TYPE_MUST_ENUM);
    rPropList.PropEnumExternal("Cover Node\\Leaper Cover Anim Package", "Resource\0anim\0", "Resource File", PROP_TYPE_MUST_ENUM);

#ifdef X_EDITOR
    rPropList.PropEnumEnum   ( "Cover Node\\Debug Package", s_CoverNodePackageEnumTable.BuildString(), "Which anim package to draw debug blockman", PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT );
    rPropList.PropEnumEnum   ( "Cover Node\\Debug Weapon",  s_CoverNodeWeaponEnumTable.BuildString(),  "Which weapon to draw debug blockman", PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT );
#endif  // #ifdef X_EDITOR

    rPropList.PropEnumFloat  ( "Cover Node\\Max Distance", "How far from the point of cover this node is still valid.", 0 ) ;
    rPropList.PropEnumFloat  ( "Cover Node\\Min Distance", "How close from the point of cover this node still valid.", 0 ) ;
    rPropList.PropEnumFloat  ( "Cover Node\\XFOV",         "Angle of horizontal field of view", PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT ) ;
    rPropList.PropEnumFloat  ( "Cover Node\\YFOV",         "Angle of vertical field of view", PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT ) ;
    rPropList.PropEnumFloat  ( "Cover Node\\Width",        "Width of frustum at MaxDistance", PROP_TYPE_DONT_SHOW ) ;
    rPropList.PropEnumFloat  ( "Cover Node\\Height",       "Height of the frustum at MaxDistance", PROP_TYPE_DONT_SHOW ) ;
    rPropList.PropEnumGuid   ( "Cover Node\\Check Marker", "Marker of the position used by the NPC when making checks such as do we have a clear grenade throw.", 0 ) ;
    rPropList.PropEnumGuid   ( "Cover Node\\Destructable Object", "destructable or super destructable we are attached to, if it's destroyed so are we.", 0 ) ;

    rPropList.PropEnumBool   ( "Cover Node\\DebugRender", "Turns on/off the debugging features", PROP_TYPE_DONT_SAVE ) ;
    rPropList.PropEnumBool   ( "Cover Node\\Cover From Everywhere", "If true this cover is valid regardless of where the enemy is", PROP_TYPE_EXPOSE ) ;
    rPropList.PropEnumBool   ( "Cover Node\\Is Active", "Is this cover node active for use", 0 ) ;
    rPropList.PropEnumInt    ( "Cover Node\\Shoot Percent", "Chance we will shoot, if negative will use cover state values.", PROP_TYPE_EXPOSE ) ;
    rPropList.PropEnumInt    ( "Cover Node\\Grenade Percent", "Chance we will throw a grenade, if negative will use cover state values.", PROP_TYPE_EXPOSE ) ;
    rPropList.PropEnumFloat  ( "Cover Node\\MinTimeBetweenCombatPopup", "Min time we stay down while fighting.", PROP_TYPE_EXPOSE ) ;
    rPropList.PropEnumFloat  ( "Cover Node\\MaxTimeBetweenCombatPopup", "Max time we stay down while fighting.", PROP_TYPE_EXPOSE ) ;
}

//=============================================================================

xbool cover_node::OnProperty( prop_query& rPropQuery )
{
    //
    // When ever we get here we are going to assume we are changing something
    // that will affect our frusturm. This function is not performace critical
    // so we can get away with it.
    //
    m_Flags |= FLAGS_DIRTY_PLANES;

    // Animation
    if (rPropQuery.VarInt("Cover Node\\Shoot Percent", m_ShootWeight))
    {
        return TRUE;
    }
    else if (rPropQuery.VarInt("Cover Node\\Grenade Percent", m_GrenadeWeight))
    {
        return TRUE;
    }
    else if (rPropQuery.VarFloat("Cover Node\\MinTimeBetweenCombatPopup", m_ActionDelayMin))
    {
        return TRUE;
    }
    else if (rPropQuery.VarFloat("Cover Node\\MaxTimeBetweenCombatPopup", m_ActionDelayMax))
    {
        return TRUE;
    }
    else if (rPropQuery.IsVar( "Cover Node\\Civilian Cover Anim Package"))
    {
        if (rPropQuery.IsRead())
        {        
            rPropQuery.SetVarExternal( m_hCivilianAnimGroup.GetName() , RESOURCE_NAME_SIZE);
        }
        else
        {
            xstring String = rPropQuery.GetVarExternal();
            if( !String.IsEmpty() )
            {
                // Clear?
                if( String == "<null>" )
                {
                    // Clear anim group and name
                    m_hCivilianAnimGroup.SetName( "" );
                }
                else
                {            
                    m_hCivilianAnimGroup.SetName( rPropQuery.GetVarExternal() );
                }
            }
        }
        return TRUE;
    }
    else if (rPropQuery.IsVar( "Cover Node\\Soldier Cover Anim Package"))
    {
        if (rPropQuery.IsRead())
        {        
            rPropQuery.SetVarExternal( m_hSoldierAnimGroup.GetName() , RESOURCE_NAME_SIZE);
        }
        else
        {
            xstring String = rPropQuery.GetVarExternal();
            if( !String.IsEmpty() )
            {
                // Clear?
                if( String == "<null>" )
                {
                    // Clear anim group and name
                    m_hSoldierAnimGroup.SetName( "" );
                }
                else
                {            
                    m_hSoldierAnimGroup.SetName( rPropQuery.GetVarExternal() );
                }
            }
        }
        return TRUE;
    }
    else if (rPropQuery.IsVar( "Cover Node\\Grunt Cover Anim Package"))
    {
        if (rPropQuery.IsRead())
        {        
            rPropQuery.SetVarExternal( m_hGruntAnimGroup.GetName(), RESOURCE_NAME_SIZE);
        }
        else
        {
            xstring String = rPropQuery.GetVarExternal();
            if( !String.IsEmpty() )
            {
                // Clear?
                if( String == "<null>" )
                {
                    // Clear anim group and name
                    m_hGruntAnimGroup.SetName( "" );
                }
                else
                {            
                    m_hGruntAnimGroup.SetName( rPropQuery.GetVarExternal() );
                }
            }
        }
        return TRUE;
    }

    else if (rPropQuery.IsVar( "Cover Node\\Leaper Cover Anim Package"))
    {
        if (rPropQuery.IsRead())
        {        
            rPropQuery.SetVarExternal( m_hLeaperAnimGroup.GetName(), RESOURCE_NAME_SIZE);
        }
        else
        {
            xstring String = rPropQuery.GetVarExternal();
            if( !String.IsEmpty() )
            {
                // Clear?
                if( String == "<null>" )
                {
                    // Clear anim group and name
                    m_hLeaperAnimGroup.SetName( "" );
                }
                else
                {            
                    m_hLeaperAnimGroup.SetName( rPropQuery.GetVarExternal() );
                }
            }
        }
        return TRUE;
    }

#ifdef X_EDITOR
    // Debug package
    if ( SMP_UTIL_IsEnumVar<cover_node::eCoverAnimPackageType, cover_node::eCoverAnimPackageType>(rPropQuery, "Cover Node\\Debug Package", 
         s_CoverNodeDebugPackage, s_CoverNodePackageEnumTable) )
    {
        return TRUE;
    }

    // Debug weapon
    if ( SMP_UTIL_IsEnumVar<cover_node::eCoverWeaponType, cover_node::eCoverWeaponType>(rPropQuery, "Cover Node\\Debug Weapon", 
         s_CoverNodeDebugWeapon, s_CoverNodeWeaponEnumTable) )
    {
        return TRUE;
    }
#endif  // #ifdef X_EDITOR

    //
    // Handle the properties
    //
    if ( rPropQuery.VarFloat( "Cover Node\\Width", m_Width, 0.0f, 50000.0f ) )
    {
        SetXFOV(GetXFOV());

        // Force bounds to update
        SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSFORM );
        return TRUE;
    }
    else if ( rPropQuery.VarFloat( "Cover Node\\Height", m_Height, 0.0f, 50000.0f ) )
    {
        SetYFOV(GetYFOV());

        // Force bounds to update
        SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSFORM );
        return TRUE;
    }
    else if (rPropQuery.IsVar( "Cover Node\\Max Distance"))
    {
        if (rPropQuery.IsRead())
        {        
            rPropQuery.SetVarFloat( m_MaxDistance );
        }
        else
        {
            radian XFOV = GetXFOV();
            radian YFOV = GetYFOV();
            m_MaxDistance = rPropQuery.GetVarFloat();
            if( m_MaxDistance < m_MinDistance ) m_MaxDistance = m_MinDistance + 1.0f;
            if( m_MaxDistance > 50000.0f ) m_MaxDistance = 50000.0f;
            SetXFOV(XFOV);
            SetYFOV(YFOV);
            SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSFORM );
        }

        return TRUE;
    }
    else if ( rPropQuery.VarFloat( "Cover Node\\Distance", m_MaxDistance, 0.0f, 50000.0f ) )
    {
        // Force bounds to update
        SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSFORM );
        return TRUE;
    }
    else if ( rPropQuery.VarFloat( "Cover Node\\Min Distance", m_MinDistance, 0.0f, 50000.0f) )
    {
        if( m_MinDistance >= m_MaxDistance )
            m_MinDistance = m_MaxDistance - 1.0f;
        if( m_MinDistance < 0 ) m_MinDistance = 1;
        return TRUE;
    }
    else if (rPropQuery.IsVar( "Cover Node\\XFOV"))
    {
        if (rPropQuery.IsRead())
        {        
            radian XFOV = GetXFOV();
            rPropQuery.SetVarFloat( RAD_TO_DEG(XFOV) );
        }
        else
        {
            radian XFOV = DEG_TO_RAD(rPropQuery.GetVarFloat());
            SetXFOV( XFOV );
        }
        return TRUE;
    }
    else if (rPropQuery.IsVar( "Cover Node\\YFOV"))
    {
        if (rPropQuery.IsRead())
        {        
            radian YFOV = GetYFOV();
            rPropQuery.SetVarFloat( RAD_TO_DEG(YFOV) );
        }
        else
        {
            radian YFOV = DEG_TO_RAD(rPropQuery.GetVarFloat());
            SetYFOV( YFOV );
        }
        return TRUE;
    }
    else if ( rPropQuery.VarGUID( "Cover Node\\Next Sticky", m_NextStickyNode) )
    {
        return TRUE;
    }
    else if ( rPropQuery.VarGUID( "Cover Node\\Check Marker", m_CheckPoint) )
    {
        return TRUE;
    }
    else if ( rPropQuery.VarGUID( "Cover Node\\Destructable Object", m_Destructable) )
    {
        return TRUE;
    }
    else if ( rPropQuery.IsVar( "Cover Node\\DebugRender" ) )
    {
        if( rPropQuery.IsRead() )        
        {
            rPropQuery.SetVarBool( (m_Flags&FLAGS_DEBUG_RENDER)!=0 );
        }
        else
        {
            if( rPropQuery.GetVarBool() )
            {
                m_Flags |= FLAGS_DEBUG_RENDER;
            }
            else
            {
                m_Flags &= ~FLAGS_DEBUG_RENDER;
            }
        }
        return TRUE;
    }
    else if ( rPropQuery.VarBool("Cover Node\\Is Active", m_bActive) )
    {
        return TRUE;
    }
    else if ( rPropQuery.VarBool("Cover Node\\Cover From Everywhere", m_bCoverToAll) )
    {
        return TRUE;
    }

    return object::OnProperty(rPropQuery);
}

//=============================================================================

#ifdef X_EDITOR

static
s32 ValidateCoverAnimGroup( xstring&            ErrorMsg, 
                            anim_group::handle& hAnimGroup, 
                            const char*         pCoverType,
                            const char*         pCharacterName )
{
    // Lookup anim group
    const anim_group* pAnimGroup = hAnimGroup.GetPointer();
    if( !pAnimGroup )
        return 0;
        
    // Lookup group name
    const char* pAnimGroupName = hAnimGroup.GetName();
    if( !pAnimGroupName )
        return 0;         
    
    // Check for correct assigned character type
    if( x_stristr( pAnimGroupName, pCharacterName ) == FALSE )
    {
        // Animation group does not have enough bones - report the error!
        ErrorMsg += "ERROR: Assigned ";
        ErrorMsg += pCoverType;
        ErrorMsg += " anim package [";
        ErrorMsg += pAnimGroupName;
        ErrorMsg += "] ";
        ErrorMsg += xfs( "[%d Bones]", pAnimGroup->GetNBones() );
        ErrorMsg += " is in the wrong NPC slot!";
        return 1;
    }
    
    // Loop through all game objects and check for matching bone assignments
    for( s32 Type = object::TYPE_NULL; Type < object::TYPE_END_OF_LIST; Type++ )
    {
        // Lookup first slot
        slot_id SlotID = g_ObjMgr.GetFirst( (object::type)Type );
        while( SlotID != SLOT_NULL )
        {
            // Lookup object
            object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
            
            // Is this a character?
            if( ( pObject ) && ( pObject->IsKindOf( character::GetRTTI() ) ) )
            {
                // Convert to character
                character* pCharacter = (character*)pObject;
                
                // Lookup geom info
                const geom* pSkinGeom     = pCharacter->GetGeomPtr();
                const char* pSkinGeomName = pCharacter->GetSkinInst().GetSkinGeomName();
                
                // Does skin geom match anim group?
                if( ( pSkinGeom ) && ( pSkinGeomName ) && ( x_stristr( pSkinGeomName, pCharacterName ) ) )
                {
                    // Make sure there are enough bones in the animation
                    if( pSkinGeom->m_nBones > pAnimGroup->GetNBones() )
                    {
                        // Animation group does not have enough bones - report the error!
                        ErrorMsg += "ERROR: Assigned ";
                        ErrorMsg += pCoverType;
                        ErrorMsg += " anim package [";
                        ErrorMsg += pAnimGroupName;
                        ErrorMsg += "] ";
                        ErrorMsg += xfs( "[%d Bones]", pAnimGroup->GetNBones() );
                        ErrorMsg += " does not match skingeom [";
                        ErrorMsg += pSkinGeomName;
                        ErrorMsg += "] ";
                        ErrorMsg += xfs( "[%d Bones]", pSkinGeom->m_nBones );
                        ErrorMsg += ". Bone counts MUST match!! - check assignment/and/or anims\n";
                        return 1;
                    }
                }
            }
            
            // Check next object of this type
            SlotID = g_ObjMgr.GetNext( SlotID );
        }
    }
    
    // No errors
    return 0;
}

//=============================================================================

s32 cover_node::OnValidateProperties( xstring& ErrorMsg )
{
    // Make sure we call base class to get errors
    s32 nErrors = object::OnValidateProperties( ErrorMsg );

    // Check anim groups against skin geoms of npcs
    // NOTE: This is hard coded for A51 skingeom resource names
    nErrors += ValidateCoverAnimGroup( ErrorMsg, m_hSoldierAnimGroup,   "MILITARY", "_MIL"     );
    nErrors += ValidateCoverAnimGroup( ErrorMsg, m_hCivilianAnimGroup,  "CIVILIAN", "_CIV"     );
    nErrors += ValidateCoverAnimGroup( ErrorMsg, m_hGruntAnimGroup,     "GRUNT",    "_GRUNT"   );
    nErrors += ValidateCoverAnimGroup( ErrorMsg, m_hLeaperAnimGroup,    "LEAPER",   "_LEAPER"  );
    
    // Loop through all anim packages
    for( s32 i = COVER_PACKAGE_NONE + 1 ; i < COVER_PACKAGE_COUNT ; i++ )
    {
        // Lookup anim group if present
        anim_group::handle& hAnimGroup = GetAnimGroupHandle( i );
        const anim_group* pAnimGroup = hAnimGroup.GetPointer();
        if( !pAnimGroup )
            continue;

        // Loop through all animations (skip bind pose) in anim group
        for( s32 j = 1 ; j < pAnimGroup->GetNAnims() ; j ++ )
        {
            // Lookup anim
            const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( j );
            
            // These are temp for now until the artists do these anims
            if( x_strcmp( AnimInfo.GetName(), "EGL_COVER_GRENADE" ) == 0 )
                continue;
            if( x_strcmp( AnimInfo.GetName(), "EGL_COVER_SHOOT" ) == 0 )
                continue;
            if( x_strcmp( AnimInfo.GetName(), "SMP_COVER_GRENADE" ) == 0 )
                continue;
            if( x_strcmp( AnimInfo.GetName(), "SMP_COVER_SHOOT" ) == 0 )
                continue;
            if( x_stristr( AnimInfo.GetName(), "MSN_" ) )
                continue;
            if( x_stristr( AnimInfo.GetName(), "SNI_" ) )
                continue;
            if( x_stristr( AnimInfo.GetName(), "EXIT" ) )
                continue;
            if( x_stristr( AnimInfo.GetName(), "DEATH" ) )
                continue;
            
            // Make sure motion prop is there
            if( AnimInfo.GetPropChannel( "MotionProp" ) == -1 )
            {
                // Add to errors
                nErrors++;
                ErrorMsg += "ERROR: Animation [";
                ErrorMsg += AnimInfo.GetName();
                ErrorMsg += "] in package [";
                ErrorMsg += hAnimGroup.GetName();
                ErrorMsg += "] does not have a motion prop!\n";
            }
        }            
    }
    
    return nErrors;
}
#endif

//=============================================================================

radian cover_node::GetXFOV( void )
{
    if( m_Width<0 )
        m_Width = 0;
    return x_atan2( m_Width/2, m_MaxDistance )*2;
}

//=============================================================================

void cover_node::SetXFOV( radian XFOV)
{
    if( XFOV<0 ) XFOV = 0;
    if( XFOV>180 ) XFOV = 180;
    m_Width = m_MaxDistance * x_tan(XFOV/2) * 2;
}

//=============================================================================

radian cover_node::GetYFOV( void )
{
    if( m_Height<0 )
        m_Height = 0;
    return x_atan2( m_Height/2, m_MaxDistance )*2;
}

//=============================================================================

void cover_node::SetYFOV( radian YFOV)
{
    if( YFOV<0 ) YFOV = 0;
    if( YFOV>180 ) YFOV = 180;
    m_Height = m_MaxDistance * x_tan(YFOV/2) * 2;
}

//=============================================================================

