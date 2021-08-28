//=========================================================================
//
// PS2_DRAW.CPP
//
//=========================================================================
#include "e_engine.hpp"
#include "ps2_spad.hpp"
#include "ps2_misc.hpp"

#define OFFSET_X    (2048-(512/2))
#define OFFSET_Y    (2048-(512/2))
#define OFFSET_X_3D (2048)
#define OFFSET_Y_3D (2048)

//==========================================================================
//#define FORCE_CLIPPING

#define NUM_VERTICES                (SPAD.GetUsableSize()/sizeof(vert))             // Number of vertices in spad buffer
#define TRIGGER_POINTS              ((NUM_VERTICES/1)*1)                            // Vertex indicies to trigger buffer dispatch
#define TRIGGER_LINES               ((NUM_VERTICES/2)*2)                            // ...
#define TRIGGER_LINE_STRIPS         ((NUM_VERTICES/2)*2)                            // ...
#define TRIGGER_TRIANGLES           ((NUM_VERTICES/3)*3)                            // ...
#define TRIGGER_TRIANGLE_STRIPS     ((NUM_VERTICES/3)*3)                            // ...
#define TRIGGER_QUADS               ((NUM_VERTICES/4)*4)                            // ...
#define TRIGGER_RECTS               ((NUM_VERTICES/4))                              // ...
#define NUM_SPRITES                 ((s32)(SPAD.GetUsableSize()/sizeof(sprite)))    // Number of sprites in spad buffer

typedef void (*fnptr_dispatch)( void );

//==========================================================================
// Structures
//==========================================================================

struct batch_header
{
    dmatag  DMA;
    giftag  PRIMGIF;
    s64     prim;
    s64     pad;
    giftag  REGGIF;
};

struct gsvert
{
    s64     T;  // Texture coordinates
    s64     C;  // Color
    s64     P;  // Position
};

struct vert
{
    vector3 Pos;
    gsvert  GS;
    vector2 UV;
    u32     Color;
} PS2_ALIGNMENT(16);

struct sprite
{
    vector3     Position;
    vector2     WH;
    xcolor      Color;
    vector2     UV0;
    vector2     UV1;
    radian      Rotation;
    xbool       IsRotated;
} PS2_ALIGNMENT(16);

struct point_prim
{
    s64 RGBA;
    s64 XYZ;
};

struct line_prim
{
    s64 RGBA0;
    s64 XYZ0;
    s64 RGBA1;
    s64 XYZ1;
};

struct quad_prim
{
    s64 UV0;
    s64 RGBA0;
    s64 XYZ0;
    s64 UV1;
    s64 RGBA1;
    s64 XYZ1;
    s64 UV2;
    s64 RGBA2;
    s64 XYZ2;
    s64 UV3;
    s64 RGBA3;
    s64 XYZ3;
};

struct tri_prim
{
    s64 UV0;
    s64 RGBA0;
    s64 XYZ0;
    s64 UV1;
    s64 RGBA1;
    s64 XYZ1;
    s64 UV2;
    s64 RGBA2;
    s64 XYZ2;
    s64 PAD;
};

struct zbuffer_clear
{
    dmatag   DMA;       // DMA tag
    giftag   PRIMGIF;   // GIF for setting PRIM register
    s64      prim;      // PRIM register
    s64      pad;
    giftag   GIF;
    s64      XYZ0;
    s64      XYZ1;
    s64      XYZ2;
    s64      XYZ3;
};

//==========================================================================
// Helper template for adding primitives to the display list. This becomes
// necessary when using MFIFO because the display list can only contain
// a small number of primitives and we need to have those broken apart
// by separate dma tags. In this case, the overhead for having a template
// is pretty minimal, because there will only be a few versions of this
// class and it is all localized to this cpp file.
//==========================================================================

template <class T>
class batch_mgr
{
public:
            batch_mgr       ( void );
    void    SetPrimReg      ( s64           Prim );
    void    SetRegGiftag    ( const giftag& GIF );
    void    Begin           ( void );
    void    End             ( void );
    T&      NewPrimitive    ( void );

    byte*   m_pStart;
    byte*   m_pEnd;
    byte*   m_pCurrent;
    s32     m_nPrimsAdded;
    s32     m_nPrimsAvail;
    s64     m_PrimReg;
    giftag  m_RegGiftag;

#ifdef X_ASSERT
    xbool   m_bInBegin;
#endif
};

//==========================================================================

template<class T>
batch_mgr<T>::batch_mgr( void )
{
#ifdef X_ASSERT
    m_bInBegin = FALSE;
#endif
}

//==========================================================================

template<class T>
void batch_mgr<T>::SetPrimReg( s64 PrimReg )
{
    m_PrimReg = PrimReg;
}

//==========================================================================

template<class T>
void batch_mgr<T>::SetRegGiftag( const giftag& Giftag )
{
    m_RegGiftag = Giftag;
}

//==========================================================================

template<class T>
void batch_mgr<T>::Begin( void )
{
    ASSERT( TRUE == (m_bInBegin = !m_bInBegin) );

    // figure out how many primitives we can store in this batch
    s32 nBytesAvail = DLIST.GetAvailable();
    if( nBytesAvail <= (s32)(sizeof(batch_header) + sizeof(T)) )
    {
        DLIST.Flush();
        nBytesAvail = DLIST.GetAvailable();
        ASSERT( nBytesAvail >= (s32)(sizeof(batch_header) + sizeof(T)) );
    }
    m_nPrimsAvail = (nBytesAvail-sizeof(batch_header)) / sizeof(T);

    // allocate space for the primitives to be added
    m_nPrimsAdded = 0;
    s32 nBytes = sizeof(batch_header) + (m_nPrimsAvail * sizeof(T));
    m_pStart   = (byte*)DLIST.Alloc( nBytes );
    m_pEnd     = m_pStart + nBytes;
    m_pCurrent = m_pStart + sizeof(batch_header);   // skip the header
}

//==========================================================================

template<class T>
void batch_mgr<T>::End( void )
{
    ASSERT( FALSE == (m_bInBegin = !m_bInBegin) );

    if( m_nPrimsAdded == 0 )
    {
        // No primitives added? Return ALL of the space to the dlist...
        DLIST.Dealloc( m_pEnd-m_pStart );
    }
    else
    {
        // Primitives added? Return unused space to the dlist. (It can
        // survive returning zero bytes, so don't worry about checking that
        // case)
        DLIST.Dealloc( m_pEnd-m_pCurrent );

        // Fill in the giftag
        batch_header* pBatchHeader = (batch_header*)m_pStart;
        pBatchHeader->DMA.SetCont( m_pCurrent-m_pStart-sizeof(dmatag) );
        pBatchHeader->DMA.MakeDirect();
        pBatchHeader->PRIMGIF.Build( GIF_MODE_REGLIST, 2, 1, 0, 0, 0, 0 );
        pBatchHeader->PRIMGIF.Reg( GIF_REG_PRIM, GIF_REG_NOP ) ;
        pBatchHeader->prim         = m_PrimReg;
        pBatchHeader->pad          = 0;
        pBatchHeader->REGGIF       = m_RegGiftag;
        pBatchHeader->REGGIF.NLOOP = m_nPrimsAdded;
    }
}

//==========================================================================

template<class T>
T& batch_mgr<T>::NewPrimitive( void )
{
    if( m_nPrimsAvail > 1000 )
    {
        BREAK;
    }

    ASSERT( m_bInBegin );
    
    // flush the current batch and start a new one if necessary
    if( m_nPrimsAdded == m_nPrimsAvail )
    {
        End();
        Begin();
    }

    // allocate space for the primitive
    m_nPrimsAdded++;
    T* pC = (T*)m_pCurrent;
    m_pCurrent += sizeof(T);
    ASSERT( m_pCurrent <= m_pEnd );

    return (*pC);
}

//==========================================================================
// Data
//==========================================================================

static vert*            s_BufferPos;
static fnptr_dispatch   s_pfnDispatch;          // Dispatch Function
static s32              s_nVertsInBuffer;       // Num of verts currently in
static s32              s_nTrigger;             // Num of verts to trigger dispatch

static s32              s_nSprites;

static xbool            s_Initialized = TRUE;  // Becomes TRUE when Initialized
static xbool            s_bBegin = FALSE;       // TRUE when between begin/end

static draw_primitive   s_Primitive;            // Primitive Type, see enum draw_primitive
static u32              s_Flags;                // Flags, see defines
static xbool            s_Is2D;                 // TRUE for 2D mode
static xbool            s_IsTextured;           // TRUE for Textured mode
static xbool            s_IsAlpha;              // TRUE when using alpha
static xbool            s_IsAdditive;           // TRUE when using alpha in additive mode
static xbool            s_IsSubtractive;        // TRUE when using alpha in subtractive mode
static xbool            s_IsWire;               // TRUE when using wireframe
static xbool            s_IsSTQ;                // TRUE when needs stq in transform
static xbool            s_IsZBuffered;
static xbool            s_IsZWrite;             // TRUE when writing to the zbuffer
static xbool            s_UClamp;
static xbool            s_VClamp;
static xbool            s_IsGouraud  = TRUE;

static matrix4          s_L2W PS2_ALIGNMENT(64);    // L2W matrix for draw
static matrix4          s_W2S PS2_ALIGNMENT(64);
static matrix4          s_L2S PS2_ALIGNMENT(64);
static matrix4          s_W2V PS2_ALIGNMENT(64);
static matrix4          s_V2S PS2_ALIGNMENT(64);

static const vector2*   s_pUVs;                 // Pointer to UV array
static s32              s_nUVs;                 // Number of elements
static s32              s_sUVs;                 // Stride of elements

static const xcolor*    s_pColors;              // Pointer to Color array
static s32              s_nColors;              // Number of elements
static s32              s_sColors;              // Stride of elements

static const vector3*   s_pVerts;               // Poitner to vertex array
static s32              s_nVerts;               // Number of elements
static s32              s_sVerts;               // Stride of elements

static vector2          s_UV;                   // Current UV
static u32              s_Color;                // Current Color in ps2 format

static s32              s_ZBias=0;
static s32              ZBIAS_SCALAR = 1024;

static batch_mgr<point_prim>     s_PointBatchMgr;
static batch_mgr<line_prim>      s_LineBatchMgr;
static batch_mgr<quad_prim>      s_QuadBatchMgr;
static batch_mgr<tri_prim>       s_TriBatchMgr;

//==========================================================================

void draw_SetZBias( s32 Bias )
{
    ASSERT( (Bias>=0) && (Bias<=16) );
    s_ZBias = (Bias*ZBIAS_SCALAR);
}

//==========================================================================

static xcolor draw_ColorToPS2( const xcolor In, xbool Textured )
{
    xcolor C;
    if( Textured )
    {
        C.R = (In.R==255) ? 128 : (In.R>>1);
        C.G = (In.G==255) ? 128 : (In.G>>1);
        C.B = (In.B==255) ? 128 : (In.B>>1);
    }
    else
    {
        C.R = In.R;
        C.G = In.G;
        C.B = In.B;
    }
    C.A = (In.A==255) ? 128 : (In.A>>1);
    return C;
}

//==========================================================================

static
void draw_UpdateMatrices( void )
{
    const view* pView = eng_GetView();
    s_W2V = pView->GetW2V();
    s_V2S = pView->GetV2S();
    s_W2S = pView->GetW2S();
    s_L2S = s_W2S*s_L2W;
}

//==========================================================================

static inline void LoadMatrixIntoRegisters(matrix4& m0)
{
    asm __volatile__ ("
        lqc2 vf4, 0(%0)
        lqc2 vf5,16(%0)
        lqc2 vf6,32(%0)
        lqc2 vf7,48(%0)
        " : : "r" (&m0) );
}

//==========================================================================

struct veci4
{
    s32 X,Y,Z;
    f32 W;
} PS2_ALIGNMENT(16);

static
void draw_TransformVerts( vert* pBuffer, matrix4& M, s32 NVerts )
{

    if( s_Is2D )
    {
        s32      C = NVerts;
        vert*    pV = pBuffer;

        while( C-- )
        {
            // Build binary u64s for GS
            s32 SPX = (s32)((pV->Pos.GetX()+OFFSET_X)*16);
            s32 SPY = (s32)((pV->Pos.GetY()+OFFSET_Y)*16);
            s32 SPZ = (s32)pV->Pos.GetZ() ;

            // Keep vert z? (NOTE: 0x01 means no clipping needed)
            if (s_Flags & DRAW_2D_KEEP_Z)
            {
                // transform the fog z into screen space
                vector4 V = s_V2S * vector4( 0.0f, 0.0f, pV->Pos.GetZ(), 1.0f );
                V.GetZ() /= V.GetW();
                SPZ = (s32)(V.GetZ()*16.0f);
                pV->GS.P = SCE_GS_SET_XYZ( SPX, SPY, SPZ ) | 0x01;
            }
            else
            {
                pV->GS.P = SCE_GS_SET_XYZ( SPX, SPY, 0x00FFFFFF ) | 0x01;
            }

            //pV->GS.C = pV->Color;
            //pV->GS.T = SCE_GS_SET_UV( ((s32)(pV->UV.X*16)), ((s32)(pV->UV.Y*16)) );
        
            // Build texturing STQ
            ((f32*)(&pV->GS.T))[0] = pV->UV.X; 
            ((f32*)(&pV->GS.T))[1] = pV->UV.Y; 

            // Build color and Q
            ((s32*)(&pV->GS.C))[0] = pV->Color;
            ((f32*)(&pV->GS.C))[1] = 1.0f;

            if( (SPX|SPY)&0xFFFF0000 )
                pV->GS.P &= ~((u64)1);

            #ifdef FORCE_CLIPPING
            pV->GS.P &= ~((u64)1);
            #endif

            pV++;
        }
    }
    else
    {
        s32      C;
        vert*    pV;
        volatile veci4    SP;

        pV   = pBuffer;
        C    = NVerts;
        LoadMatrixIntoRegisters( M );

        if( !s_IsSTQ )
        {
            while( C-- )
            {
                asm __volatile__ ("
                    lqc2            vf8, 0(%0)
                    vmulaw.xyzw     ACC, vf7,vf0
                    vmadday.xyzw    ACC, vf5,vf8
                    vmaddaz.xyzw    ACC, vf6,vf8
                    vmaddx.xyzw     vf12,vf4,vf8
                    vdiv            Q,vf0w,vf12w
	                vwaitq
	                vmulq.xyz       vf12,vf12,Q
                    vftoi4.xyz      vf12,vf12
                    sqc2            vf12,0(%1)
                    ": : "r" (&pV[0].Pos), "r" (&SP));


                // Build binary u64s for GS
                pV->GS.C = pV->Color;
                pV->GS.P = SCE_GS_SET_XYZ( SP.X, SP.Y, SP.Z+s_ZBias ) | 0x01;
            
                if( ((SP.X|SP.Y)&0xFFFF0000) || (SP.Z<0) )
                    pV->GS.P &= ~((u64)1);

                #ifdef FORCE_CLIPPING
                pV->GS.P &= ~((u64)1);
                #endif

                pV++;
            }
        }
        else
        {
            while( C-- )
            {
                asm __volatile__ ("
                    lqc2            vf8, 0(%0)
                    vmulaw.xyzw     ACC, vf7,vf0
                    vmadday.xyzw    ACC, vf5,vf8
                    vmaddaz.xyzw    ACC, vf6,vf8
                    vmaddx.xyzw     vf12,vf4,vf8
                    vdiv            Q,vf0w,vf12w
	                vwaitq
	                vmulq.xyz       vf12,vf12,Q
                    vmulq.w         vf12,vf0,Q
                    vftoi4.xyz      vf12,vf12
                    sqc2            vf12,0(%1)
                    ": : "r" (&pV[0].Pos), "r" (&SP) );

                // Build texturing STQ
                ((f32*)(&pV->GS.T))[0] = pV->UV.X * SP.W; 
                ((f32*)(&pV->GS.T))[1] = pV->UV.Y * SP.W; 

                // Build color and Q
                ((s32*)(&pV->GS.C))[0] = pV->Color;
                ((f32*)(&pV->GS.C))[1] = SP.W;

                // Build position
                pV->GS.P = SCE_GS_SET_XYZ( SP.X, SP.Y, SP.Z+s_ZBias ) | 0x01;
            
                if( ((SP.X|SP.Y)&0xFFFF0000) || (SP.Z<0) )
                    pV->GS.P &= ~((u64)1);

                #ifdef FORCE_CLIPPING
                pV->GS.P &= ~((u64)1);
                #endif

                // Move to next vert
                pV++;
            }
        }
    }
}
//==========================================================================

static inline void LoadSpriteMatrixIntoRegisters(matrix4& m0, matrix4& m1)
{
    asm __volatile__ ("
        lqc2 vf4,  0(%0)
        lqc2 vf5, 16(%0)
        lqc2 vf6, 32(%0)
        lqc2 vf7, 48(%0)
        lqc2 vf8,  0(%1)
        lqc2 vf9, 16(%1)
        lqc2 vf10,32(%1)
        lqc2 vf11,48(%1)
        " : : "r" (&m0), "r" (&m1) );
}

//==========================================================================

static inline void draw_sincos( radian Angle, f32& Sine, f32& Cosine )
{
    #define I_360   (1<<16)
    #define I_90    (I_360/4)
    #define I_180   (I_360/2)
    #define I_270   (I_90*3)

    s32 IAngle = ((s32)(Angle*(I_360/R_360)))&(I_360-1);
    s32 si,ci;

    if( IAngle >= I_180 )
    {
        if( IAngle >= I_270 ) { si =  IAngle - I_360; ci =  IAngle - I_270; }
        else                  { si = -IAngle + I_180; ci =  IAngle - I_270; }
    }
    else
    {
        if( IAngle >= I_90 )  { si = -IAngle + I_180; ci = -IAngle + I_90;  }
        else                  { si = IAngle;          ci = -IAngle + I_90;  }
    }

    f32 sq  = si*(R_360/I_360);
    f32 cq  = ci*(R_360/I_360);
    f32 sq2 = sq*sq;
    f32 cq2 = cq*cq;
    Sine   = (((0.00813767f*sq2) - 0.1666666f)*sq2 + 1)*sq;
    Cosine = (((0.00813767f*cq2) - 0.1666666f)*cq2 + 1)*cq;

    #undef I_360
    #undef I_90
    #undef I_180
    #undef I_270
}

//==========================================================================

static
void ClipToGuardband( vector3* pDstPos,
                      vector2* pDstUV,
                      const vector3* pSrcPos,
                      const vector2* pSrcUV,
                      s32& NDstVerts,
                      s32 NSrcVerts )
{
    //static const f32 kClipLeft   = (f32)(10<<4);
    //static const f32 kClipRight  = (f32)(4086<<4);
    //static const f32 kClipTop    = (f32)(10<<4);
    //static const f32 kClipBottom = (f32)(4086<<4);
    static const f32 kClipLeft   = (f32)(10);
    static const f32 kClipRight  = (f32)(4086);
    static const f32 kClipTop    = (f32)(10);
    static const f32 kClipBottom = (f32)(4086);

    vector3 TempPos0[4+4+1];
    vector3 TempPos1[4+4+1];
    vector2 TempUV0[4+4+1];
    vector2 TempUV1[4+4+1];

    // copy the src verts into our temp buffer and duplicate the last one
    s32 iVert;
    for ( iVert = 0; iVert < NSrcVerts; iVert++ )
    {
        TempPos0[iVert] = pSrcPos[iVert] * (1.0f/16.0f);
        TempUV0[iVert]  = pSrcUV[iVert];
    }
    TempPos0[NSrcVerts] = pSrcPos[0];
    TempUV0[NSrcVerts]  = pSrcUV[0];
    
    vector3* pSP = TempPos0;
    vector2* pSU = TempUV0;
    vector3* pDP = TempPos1;
    vector2* pDU = TempUV1;

    // clip the 4 edges
    for ( s32 Edge = 0; Edge < 4; Edge++ )
    {
        NDstVerts = 0;
        for ( iVert = 0; iVert < NSrcVerts; iVert++ )
        {
            xbool AddVert         = FALSE;
            xbool AddIntersection = FALSE;
            f32   T = 0.0f;
            switch ( Edge )
            {
            case 0:
                {
                    xbool bIn     = pSP[iVert].GetX()   >= kClipLeft-0.001f;
                    xbool bNextIn = pSP[iVert+1].GetX() >= kClipLeft-0.001f;
                    if ( bIn )
                        AddVert = TRUE;
                    if ( bIn != bNextIn )
                    {
                        AddIntersection = TRUE;
                        T = (kClipLeft - pSP[iVert].GetX()) / (pSP[iVert+1].GetX() - pSP[iVert].GetX());
                    }
                }
                break;
            case 1:
                {
                    xbool bIn     = pSP[iVert].GetX()   <= kClipRight+0.001f;
                    xbool bNextIn = pSP[iVert+1].GetX() <= kClipRight+0.001f;
                    if ( bIn )
                        AddVert = TRUE;
                    if ( bIn != bNextIn )
                    {
                        AddIntersection = TRUE;
                        T = (kClipRight - pSP[iVert].GetX()) / (pSP[iVert+1].GetX() - pSP[iVert].GetX());
                    }
                }
                break;
            case 2:
                {
                    xbool bIn     = pSP[iVert].GetY()   >= kClipTop-0.001f;
                    xbool bNextIn = pSP[iVert+1].GetY() >= kClipTop-0.001f;
                    if ( bIn )
                        AddVert = TRUE;
                    if ( bIn != bNextIn )
                    {
                        AddIntersection = TRUE;
                        T = (kClipTop - pSP[iVert].GetY()) / (pSP[iVert+1].GetY() - pSP[iVert].GetY());
                    }
                }
                break;
            case 3:
                {
                    xbool bIn     = pSP[iVert].GetY()   <= kClipBottom+0.001f;
                    xbool bNextIn = pSP[iVert+1].GetY() <= kClipBottom+0.001f;
                    if ( bIn )
                        AddVert = TRUE;
                    if ( bIn != bNextIn )
                    {
                        AddIntersection = TRUE;
                        T = (kClipBottom - pSP[iVert].GetY()) / (pSP[iVert+1].GetY() - pSP[iVert].GetY());
                    }
                }
                break;
            }

            if ( AddVert && NDstVerts < 8 )
            {
                pDP[NDstVerts] = pSP[iVert];
                pDU[NDstVerts] = pSU[iVert];
                NDstVerts++;
            }

            if ( AddIntersection && NDstVerts < 8 )
            {
                ASSERT( (T > -0.1f) && (T < 1.1f) );
                T = MINMAX( 0.0f, T, 1.0f );
                vector3 NewPos = pSP[iVert] + T * (pSP[iVert+1]-pSP[iVert]);
                vector2 NewUV  = pSU[iVert] + T * (pSU[iVert+1]-pSU[iVert]);
                pDP[NDstVerts] = NewPos;
                pDU[NDstVerts] = NewUV;
                NDstVerts++;
            }
        }

        // duplicate the last vert
        pDP[NDstVerts] = pDP[0];
        pDU[NDstVerts] = pDU[0];
        
        // swap dst and src
        vector3* Temp3 = pDP;
        pDP = pSP;
        pSP = Temp3;
        vector2* Temp2 = pDU;
        pDU = pSU;
        pSU = Temp2;
        NSrcVerts = NDstVerts;

        // bail if we're completely culled
        if ( NSrcVerts == 0 )
            break;

        if ( NDstVerts >= 8 )
            break;
    }

    // copy into the new array    
    for ( iVert = 0; iVert < NSrcVerts; iVert++ )
    {
        ASSERT( pSP[iVert].GetX() >= kClipLeft-5.0f && pSP[iVert].GetX() < kClipRight+5.0f );
        ASSERT( pSP[iVert].GetY() >= kClipTop-5.0f && pSP[iVert].GetY() < kClipBottom+5.0f );
        pDstPos[iVert] = pSP[iVert] * 16.0f;
        pDstUV[iVert]  = pSU[iVert];
    }
    ASSERT( NDstVerts <= 8 );
}

//==========================================================================
s32 SpriteZShift = 22;
void draw_DispatchSprites( void )
{
    s32 j;

    if( s_nSprites==0 )
        return;

    // Build header info
    s64 PrimReg = SCE_GS_SET_PRIM( GIF_PRIM_TRIANGLESTRIP,
                                   1,               // shading method (flat, gouraud)
                                   s_IsTextured,    // texture mapping (off, on)
                                   0,               // fogging (off, on)
                                   s_IsAlpha,       // alpha blending (off, on)
                                   0,               // 1 pass anti-aliasing (off, on)
                                   0,               // tex-coord spec method (STQ, UV)
                                   0,               // context (1 or 2)
                                   0);              // fragment value control (normal, fixed)
    s_QuadBatchMgr.SetPrimReg( PrimReg );
    s_QuadBatchMgr.Begin();
    if( s_Is2D )
    {
        for( j=0; j<s_nSprites; j++ )
        {
            sprite* pSprite = ((sprite*)SPAD.GetUsableStartAddr()) + j;
            f32 w = pSprite->WH.X*16*0.5f;
            f32 h = pSprite->WH.Y*16*0.5f;
            s32 ScreenX = (s32)((pSprite->Position.GetX()+OFFSET_X)*16);
            s32 ScreenY = (s32)((pSprite->Position.GetY()+OFFSET_Y)*16);

            s32 V0X,V1X;
            s32 V0Y,V1Y;

            // Construct points v0 and v1
            if( pSprite->IsRotated )
            {
                f32 s,c;
                draw_sincos( -pSprite->Rotation, s, c );
                V0X = (s32)(c*w - s*h);
                V0Y = (s32)(s*w + c*h);
                V1X = (s32)(c*w + s*h);
                V1Y = (s32)(s*w - c*h);
            }
            else
            {
                ScreenX += (s32)w;
                ScreenY += (s32)h;
                V0X = (s32)w;
                V0Y = (s32)h;
                V1X = (s32)w;
                V1Y = (s32)-h;
            }

            s32 SX[4];
            s32 SY[4];
            SX[0] = (s32)(ScreenX - V0X);
            SY[0] = (s32)(ScreenY - V0Y);
            SX[1] = (s32)(ScreenX - V1X); 
            SY[1] = (s32)(ScreenY - V1Y);
            SX[2] = (s32)(ScreenX + V0X);
            SY[2] = (s32)(ScreenY + V0Y);
            SX[3] = (s32)(ScreenX + V1X);
            SY[3] = (s32)(ScreenY + V1Y);

            s32 Clip = SX[0] | SY[0] | SX[1] | SY[1] | SX[2] | SY[2] | SX[3] | SY[3];
            if( Clip & 0xFFFF0000 )
                continue;

            u64 C = (u64)pSprite->Color | (((u64)0x3f800000)<<32);

            {
                quad_prim& Quad = s_QuadBatchMgr.NewPrimitive();

                if (s_Flags & DRAW_2D_KEEP_Z)
                {
                    s32 SPZ = (s32)pSprite->Position.GetZ() ;
                    Quad.XYZ0 = SCE_GS_SET_XYZ(SX[0],SY[0],SPZ);
                    Quad.XYZ1 = SCE_GS_SET_XYZ(SX[1],SY[1],SPZ);
                    Quad.XYZ2 = SCE_GS_SET_XYZ(SX[3],SY[3],SPZ);
                    Quad.XYZ3 = SCE_GS_SET_XYZ(SX[2],SY[2],SPZ);
                }
                else
                {
                    // BW - PLEASE NOTE WHEN DOING A RE-WRITE (some poor bastard will have to)
                    //
                    // The lower right corner of the quad for a sprite needs to be 'adjusted' to
                    // take in to account the crappyness of the crap crap sony hardware. Otherwise
                    // a large sprite will be triangle skewed. Normally this is not an issue since
                    // bilinear interpolation is on, but with a large narrow sprite as required for
                    // movie playback (without bilinear), it is very noticable.
                    Quad.XYZ0 = SCE_GS_SET_XYZ(SX[0],SY[0],0x00FFFFFF);
                    Quad.XYZ1 = SCE_GS_SET_XYZ(SX[1],SY[1],0x00FFFFFF);
                    Quad.XYZ2 = SCE_GS_SET_XYZ(SX[3]-1,SY[3]+1,0x00FFFFFF);
                    Quad.XYZ3 = SCE_GS_SET_XYZ(SX[2]-1,SY[2]+1,0x00FFFFFF);
                }

                Quad.RGBA0 = C;
                Quad.RGBA1 = C;
                Quad.RGBA2 = C;
                Quad.RGBA3 = C;

                Quad.UV0 = SCE_GS_SET_ST( reinterpret_cast<u32&>(pSprite->UV0.X), reinterpret_cast<u32&>(pSprite->UV0.Y) );
                Quad.UV1 = SCE_GS_SET_ST( reinterpret_cast<u32&>(pSprite->UV0.X), reinterpret_cast<u32&>(pSprite->UV1.Y) );
                Quad.UV2 = SCE_GS_SET_ST( reinterpret_cast<u32&>(pSprite->UV1.X), reinterpret_cast<u32&>(pSprite->UV0.Y) );
                Quad.UV3 = SCE_GS_SET_ST( reinterpret_cast<u32&>(pSprite->UV1.X), reinterpret_cast<u32&>(pSprite->UV1.Y) );
            }
        }
    }
    else
    {
        f32     ProjX0,ProjX1;
        f32     ProjY0,ProjY1;
        f32     NearZ,FarZ,OneOverNFZ;
        matrix4 V2S;

        // For multi-screen shot fixup
        const view* pView = eng_GetView();
        s32 X0, Y0, X1, Y1;
        pView->GetViewport( X0, Y0, X1, Y1 );
        s32 ScreenW = 512;
        s32 ScreenH = 448;
#ifdef X_RETAIL
        f32 OffsetX = (OFFSET_X_3D - ScreenW/2);
        f32 OffsetY = (OFFSET_Y_3D - ScreenH/2) - (512-448)/2;
#else // X_RETAIL
        s32 ShotW = eng_ScreenShotSize() * ScreenW;
        s32 ShotH = eng_ScreenShotSize() * ScreenH;
        f32 OffsetX = (OFFSET_X_3D - ShotW/2) +
                      ShotW * (1.0f-((f32)(eng_ScreenShotX()+1) / (f32)eng_ScreenShotSize()));
        f32 OffsetY = (OFFSET_Y_3D - ShotH/2) - (512-448)/2 +
                      ShotH * (1.0f-((f32)(eng_ScreenShotY()+1) / (f32)eng_ScreenShotSize()));
#endif // X_RETAIL

        // Prepare to transform
        LoadSpriteMatrixIntoRegisters( s_V2S, s_W2V );
        V2S = pView->GetV2S();
        f32 ZConst0 = 16.0f*V2S(2,2);
        f32 ZConst1 = 16.0f*V2S(3,2);
        pView->GetProjection(ProjX0,ProjX1,ProjY0,ProjY1);
        ProjX0 = (ProjX0 + OffsetX)*16.0f;
        ProjY0 = (ProjY0 + OffsetY)*16.0f;
        ProjX1 *= 16.0f;
        ProjY1 *= 16.0f;
        pView->GetZLimits(NearZ,FarZ);
        OneOverNFZ = 1.0f / (FarZ-NearZ);

        s32     V0X,V0Y;
        s32     V1X,V1Y;
        vector3 Center;

        for( j=0; j<s_nSprites; j++ )
        {

            sprite* pSprite = ((sprite*)SPAD.GetUsableStartAddr()) + j;

            // Transform sprite center into viewspace
            asm __volatile__ ("
                lqc2            vf12, 0(%0)
                vmulaw.xyzw     ACC, vf11,vf0
                vmadday.xyzw    ACC, vf9,vf12
                vmaddaz.xyzw    ACC, vf10,vf12
                vmaddx.xyzw     vf12,vf8,vf12
                sqc2            vf12,0(%1)
                ": : "r" (&pSprite->Position), "r" (&Center) );

            // Check if in front of view
            if( (Center.GetZ()<NearZ) || (Center.GetZ()>FarZ) ) 
                continue;

            // Project pos onto screen
            f32 OneOverZ = 1.0f / Center.GetZ();
            s32 ScreenX = (s32)(ProjX0 + Center.GetX()*ProjX1 * OneOverZ);
            s32 ScreenY = (s32)(ProjY0 + Center.GetY()*ProjY1 * OneOverZ);
            f32 w       = pSprite->WH.X * ProjX1 * OneOverZ * 0.5f;
            f32 h       = pSprite->WH.Y * ProjY1 * OneOverZ * 0.5f;
            s32 Z       = (s32)( ZConst0 + ZConst1*OneOverZ ) + s_ZBias;

            // Construct points v0 and v1
            if( pSprite->IsRotated )
            {
                f32 s,c;
                draw_sincos( -pSprite->Rotation, s, c );
                V0X = (s32)(c*w - s*h);
                V0Y = (s32)(s*w + c*h);
                V1X = (s32)(c*w + s*h);
                V1Y = (s32)(s*w - c*h);
            }
            else
            {
                V0X = (s32)w;
                V0Y = (s32)h;
                V1X = (s32)w;
                V1Y = (s32)-h;
            }

            s32 SX[4];
            s32 SY[4];
            SX[0] = ScreenX + V0X;
            SY[0] = ScreenY + V0Y;
            SX[1] = ScreenX + V1X; 
            SY[1] = ScreenY + V1Y;
            SX[2] = ScreenX - V0X;
            SY[2] = ScreenY - V0Y;
            SX[3] = ScreenX - V1X;
            SY[3] = ScreenY - V1Y;

            s32 Clip = SX[0] | SY[0] | SX[1] | SY[1] | SX[2] | SY[2] | SX[3] | SY[3];
            if( Clip & 0xFFFF0000 )
            {
                vector3 SrcPos[4];
                vector2 SrcUV[4];
                vector3 DstPos[8];
                vector2 DstUV[8];

                SrcPos[0].Set( SX[0], SY[0], Z );
                SrcPos[1].Set( SX[1], SY[1], Z );
                SrcPos[2].Set( SX[2], SY[2], Z );
                SrcPos[3].Set( SX[3], SY[3], Z );
                SrcUV[0].Set( pSprite->UV0.X, pSprite->UV0.Y );
                SrcUV[1].Set( pSprite->UV1.X, pSprite->UV0.Y );
                SrcUV[2].Set( pSprite->UV1.X, pSprite->UV1.Y );
                SrcUV[3].Set( pSprite->UV0.X, pSprite->UV1.Y );

                s32 NDstVerts;
                ClipToGuardband( DstPos, DstUV, SrcPos, SrcUV, NDstVerts, 4 );
                if ( NDstVerts != 0 )
                {
                    // now add the clipped triangles (Note that I'm using quads, but
                    // just duplicating the last vert. This way I won't need to break
                    // the batch mechanism when we hit a clipped sprite, which should
                    // be pretty rare.)
                    s64 C = (s64)pSprite->Color | (((u64)0x3f800000)<<32);
                    for ( s32 iFanVert = 1; iFanVert < NDstVerts-1; iFanVert++ )
                    {
                        quad_prim& QuadPrim = s_QuadBatchMgr.NewPrimitive();

                        QuadPrim.UV0 = SCE_GS_SET_ST( reinterpret_cast<u32&>(DstUV[0].X),
                                                      reinterpret_cast<u32&>(DstUV[0].Y) );
                        QuadPrim.UV1 = SCE_GS_SET_ST( reinterpret_cast<u32&>(DstUV[iFanVert].X),
                                                      reinterpret_cast<u32&>(DstUV[iFanVert].Y) );
                        QuadPrim.UV2 = SCE_GS_SET_ST( reinterpret_cast<u32&>(DstUV[iFanVert+1].X),
                                                      reinterpret_cast<u32&>(DstUV[iFanVert+1].Y) );
                        QuadPrim.UV3 = QuadPrim.UV2;

                        QuadPrim.RGBA0 = C;
                        QuadPrim.RGBA1 = C;
                        QuadPrim.RGBA2 = C;
                        QuadPrim.RGBA3 = C;

                        QuadPrim.XYZ0 = SCE_GS_SET_XYZ((s32)DstPos[0].GetX(),
                                                       (s32)DstPos[0].GetY(),
                                                       (s32)DstPos[0].GetZ());
                        QuadPrim.XYZ1 = SCE_GS_SET_XYZ((s32)DstPos[iFanVert].GetX(),
                                                       (s32)DstPos[iFanVert].GetY(),
                                                       (s32)DstPos[iFanVert].GetZ());
                        QuadPrim.XYZ2 = SCE_GS_SET_XYZ((s32)DstPos[iFanVert+1].GetX(),
                                                       (s32)DstPos[iFanVert+1].GetY(),
                                                       (s32)DstPos[iFanVert+1].GetZ());
                        QuadPrim.XYZ3 = QuadPrim.XYZ2;
                    }
                }
            }
            else
            {
                u64 C = (u64)pSprite->Color | (((u64)0x3f800000)<<32);

                quad_prim& QuadPrim = s_QuadBatchMgr.NewPrimitive();

                QuadPrim.XYZ0 = SCE_GS_SET_XYZ(SX[0],SY[0],Z);
                QuadPrim.XYZ1 = SCE_GS_SET_XYZ(SX[1],SY[1],Z);
                QuadPrim.XYZ2 = SCE_GS_SET_XYZ(SX[3],SY[3],Z);
                QuadPrim.XYZ3 = SCE_GS_SET_XYZ(SX[2],SY[2],Z);

                QuadPrim.RGBA0 = C;
                QuadPrim.RGBA1 = C;
                QuadPrim.RGBA2 = C;
                QuadPrim.RGBA3 = C;

                QuadPrim.UV0 = SCE_GS_SET_ST( reinterpret_cast<u32&>(pSprite->UV0.X),
                                              reinterpret_cast<u32&>(pSprite->UV0.Y) );
                QuadPrim.UV1 = SCE_GS_SET_ST( reinterpret_cast<u32&>(pSprite->UV0.X),
                                              reinterpret_cast<u32&>(pSprite->UV1.Y) );
                QuadPrim.UV2 = SCE_GS_SET_ST( reinterpret_cast<u32&>(pSprite->UV1.X),
                                              reinterpret_cast<u32&>(pSprite->UV0.Y) );
                QuadPrim.UV3 = SCE_GS_SET_ST( reinterpret_cast<u32&>(pSprite->UV1.X),
                                              reinterpret_cast<u32&>(pSprite->UV1.Y) );
            }
        }
    }

    s_QuadBatchMgr.End();
    
    s_nSprites = 0;
}

//==========================================================================

static
void draw_DispatchPoints( void )
{
    // Build header info
    s64 PrimReg = SCE_GS_SET_PRIM( GIF_PRIM_POINT,
                                   0,    // shading method (flat, gouraud)
                                   0,    // texture mapping (off, on)
                                   0,    // fogging (off, on)
                                   0,    // alpha blending (off, on)
                                   0,    // 1 pass anti-aliasing (off, on)
                                   0,    // tex-coord spec method (STQ, UV)
                                   0,    // context (1 or 2)
                                   0 );  // fragment value control (normal, fixed)
    s_PointBatchMgr.SetPrimReg( PrimReg );

  
    // Copy data from spad to output
    s_PointBatchMgr.Begin();

    vert* pV  = (vert*)SPAD.GetUsableStartAddr();
    s32 C = s_nVertsInBuffer;
    while( C-- )
    {
        if( pV->GS.P & 0x01 )
        {
            point_prim& P = s_PointBatchMgr.NewPrimitive();
            P.RGBA = pV->GS.C;
            P.XYZ  = pV->GS.P;
        }
        pV++;
    }

    s_PointBatchMgr.End();
}

//==============================================================================

inline xcolor ColorLerp( const xcolor& C0, const xcolor& C1, f32 T )
{
    xcolor XC;
    ASSERT( (T>=0.0f) && (T<=1.0f) );
    XC.R = (u8)((f32)C0.R + T*((f32)C1.R-(f32)C0.R));
    XC.G = (u8)((f32)C0.G + T*((f32)C1.G-(f32)C0.G));
    XC.B = (u8)((f32)C0.B + T*((f32)C1.B-(f32)C0.B));
    XC.A = (u8)((f32)C0.A + T*((f32)C1.A-(f32)C0.A));
    return XC;
}

//==========================================================================

xbool ClipLine( vert* pInV, vert* pOutV )
{
    // Transform from local to world
    pOutV[0].Pos = s_L2W.Transform(pInV[0].Pos);
    pOutV[1].Pos = s_L2W.Transform(pInV[1].Pos);
    pOutV[0].Color = pInV[0].Color;
    pOutV[1].Color = pInV[1].Color;

    // Get view planes
    const view* pView = eng_GetView();
    const plane* pPlane = pView->GetViewPlanes(view::WORLD);

    for( s32 i=0; i<6; i++ )
    {
        f32 D0,D1;
        D0 = pPlane[i].Distance(pOutV[0].Pos);
        D1 = pPlane[i].Distance(pOutV[1].Pos);
        if( (D0<0) && (D1<0) ) return FALSE;
        if( (D0>=0) && (D1>=0) ) continue;
        f32 T = (0-D0)/(D1-D0);
        ASSERT( (T>=0.0f) && (T<=1.0f) );
        if( D0>D1 )
        {
            pOutV[1].Pos = pOutV[0].Pos + T*(pOutV[1].Pos-pOutV[0].Pos);
            pOutV[1].Color = ColorLerp( pOutV[0].Color, pOutV[1].Color, T );        
        }
        else
        {
            pOutV[0].Pos = pOutV[0].Pos + T*(pOutV[1].Pos-pOutV[0].Pos);
            pOutV[0].Color = ColorLerp( pOutV[0].Color, pOutV[1].Color, T );        
        }
    }

    // Transform and build verts
    draw_TransformVerts( pOutV, s_W2S, 2 );

    return TRUE;
}

//==========================================================================

s32 ClipTriangle( vert* pInV, vert* pOutV )
{
    vert TempV[10];
    vert* pSrc = pOutV;
    vert* pDst = TempV;
    s32   nSrcPoints;
    s32   nDstPoints;
    s32 i;

    // Transform from local to world
    nSrcPoints = 3;
    for( i=0; i<3; i++ )
    {
        pSrc[i].Pos     = s_L2W.Transform( pInV[i].Pos );
        pSrc[i].Color   = pInV[i].Color;
        pSrc[i].UV      = pInV[i].UV;
    }

    // Get view planes
    const view* pView = eng_GetView();
    const plane* pPlane = pView->GetViewPlanes(view::WORLD);

    for( s32 i=0; i<6; i++ )
    {
        s32 p0,p1;
        f32 D0,D1;
        nDstPoints = 0;

        // Loop through points in this ngon
        for( p0=0; p0<nSrcPoints; p0++ )
        {
            p1 = (p0+1)%nSrcPoints;
            D0 = pPlane[i].Distance(pSrc[p0].Pos);
            D1 = pPlane[i].Distance(pSrc[p1].Pos);

            // Check if we need to add point to front
            if( D0>=0 )
            {
                pDst[nDstPoints].Pos    = pSrc[p0].Pos;
                pDst[nDstPoints].Color  = pSrc[p0].Color;
                pDst[nDstPoints].UV     = pSrc[p0].UV;
                nDstPoints++;
            }

            // Check if edge spans clipping plane
            if( ((D0>=0)&&(D1<0)) || ((D0<0)&&(D1>=0)) )
            {
                // Compute point at crossing and add
                f32 t = (0-D0)/(D1-D0);
                ASSERT( (t>=0.0f) && (t<=1.0f) );
                pDst[nDstPoints].Pos    = pSrc[p0].Pos + t*(pSrc[p1].Pos-pSrc[p0].Pos);
                pDst[nDstPoints].UV     = pSrc[p0].UV  + t*(pSrc[p1].UV-pSrc[p0].UV);
                pDst[nDstPoints].Color  = ColorLerp( pSrc[p0].Color, pSrc[p1].Color, t );
                nDstPoints++;
            }
        }

        if( nDstPoints==0 ) return 0;
        vert* pTemp = pSrc;
        pSrc = pDst;
        pDst = pTemp;
        nSrcPoints = nDstPoints;
    }

    // Transform and build verts
    draw_TransformVerts( pOutV, s_W2S, nSrcPoints );

    return nSrcPoints;
}

//==========================================================================

static
void draw_DispatchLines( void )
{
    // Build header info
    s64 PrimReg = SCE_GS_SET_PRIM( GIF_PRIM_LINE,
                                   1,    // shading method (flat, gouraud)
                                   0,    // texture mapping (off, on)
                                   0,    // fogging (off, on)
                                   s_IsAlpha,   // alpha blending (off, on)
                                   0,    // 1 pass anti-aliasing (off, on)
                                   0,    // tex-coord spec method (STQ, UV)
                                   0,    // context (1 or 2)
                                   0 );  // fragment value control (normal, fixed)
    s_LineBatchMgr.SetPrimReg( PrimReg );

    // Copy data from spad to output
    s_LineBatchMgr.Begin();

    vert* pV  = (vert*)SPAD.GetUsableStartAddr();
    s32 C = s_nVertsInBuffer;
    while( C>=2 )
    {
        if( (u32)pV[0].GS.P & (u32)pV[1].GS.P & 0x01 )
        {
            line_prim& LinePrim = s_LineBatchMgr.NewPrimitive();
            LinePrim.RGBA0 = pV[0].GS.C;
            LinePrim.XYZ0  = pV[0].GS.P;
            LinePrim.RGBA1 = pV[1].GS.C;
            LinePrim.XYZ1  = pV[1].GS.P;
        }
        else
        {
            vert V[2];
            if( ClipLine(pV,V) )
            {
                line_prim& LinePrim = s_LineBatchMgr.NewPrimitive();
                LinePrim.RGBA0 = V[0].GS.C;
                LinePrim.XYZ0  = V[0].GS.P;
                LinePrim.RGBA1 = V[1].GS.C;
                LinePrim.XYZ1  = V[1].GS.P;
            }
        }
        pV+=2;
        C-=2;
    }

    s_LineBatchMgr.End();
}

//==========================================================================

static
void draw_DispatchQuadsAndRects( void )
{
    // Build header info
    s64 PrimReg = SCE_GS_SET_PRIM( GIF_PRIM_TRIANGLESTRIP,
                                   1,               // shading method (flat, gouraud)
                                   s_IsTextured,    // texture mapping (off, on)
                                   0,               // fogging (off, on)
                                   s_IsAlpha,       // alpha blending (off, on)
                                   0,               // 1 pass anti-aliasing (off, on)
                                   0,               // tex-coord spec method (STQ, UV)
                                   0,               // context (1 or 2)
                                   0 );             // fragment value control (normal, fixed)
    s_QuadBatchMgr.SetPrimReg( PrimReg );

    // Copy data from spad to output
    s_QuadBatchMgr.Begin();

    vert* pV  = (vert*)SPAD.GetUsableStartAddr();
    s32 C = s_nVertsInBuffer;
    if( s_Primitive == DRAW_RECTS )
    {
        while( C>=2 )
        {
            if( (u32)pV[0].GS.P & (u32)pV[1].GS.P & 0x01 )
            {
                s32 X0 = (pV[0].GS.P>> 0)&0xFFFF;
                s32 Y0 = (pV[0].GS.P>>16)&0xFFFF;
                s32 X1 = (pV[1].GS.P>> 0)&0xFFFF;
                s32 Y1 = (pV[1].GS.P>>16)&0xFFFF;
                s32 U0 = (pV[0].GS.T>> 0)&0xFFFF;
                s32 V0 = (pV[0].GS.T>>16)&0xFFFF;
                s32 U1 = (pV[1].GS.T>> 0)&0xFFFF;
                s32 V1 = (pV[1].GS.T>>16)&0xFFFF;

                quad_prim& QuadPrim = s_QuadBatchMgr.NewPrimitive();
                QuadPrim.UV0   = SCE_GS_SET_UV(U0,V0);
                QuadPrim.RGBA0 = pV[0].GS.C;
                QuadPrim.XYZ0  = SCE_GS_SET_XYZ(X0,Y0,0);
                QuadPrim.UV1   = SCE_GS_SET_UV(U0,V1);
                QuadPrim.RGBA1 = pV[0].GS.C;
                QuadPrim.XYZ1  = SCE_GS_SET_XYZ(X0,Y1,0);
                QuadPrim.UV2   = SCE_GS_SET_UV(U1,V0);
                QuadPrim.RGBA2 = pV[0].GS.C;
                QuadPrim.XYZ2  = SCE_GS_SET_XYZ(X1,Y0,0);
                QuadPrim.UV3   = SCE_GS_SET_UV(U1,V1);
                QuadPrim.RGBA3 = pV[0].GS.C;
                QuadPrim.XYZ3  = SCE_GS_SET_XYZ(X1,Y1,0);
            }
            pV+=2;
            C-=2;
        }
    }
    else
    if( s_Primitive == DRAW_QUADS )
    {
        while( C>=4 )
        {
            if( (u32)pV[0].GS.P & (u32)pV[1].GS.P & (u32)pV[2].GS.P & 0x01 )
            {
                quad_prim& QuadPrim = s_QuadBatchMgr.NewPrimitive();
                QuadPrim.UV0   = pV[0].GS.T;
                QuadPrim.RGBA0 = pV[0].GS.C;
                QuadPrim.XYZ0  = pV[0].GS.P;
                QuadPrim.UV1   = pV[1].GS.T;
                QuadPrim.RGBA1 = pV[1].GS.C;
                QuadPrim.XYZ1  = pV[1].GS.P;
                QuadPrim.UV2   = pV[3].GS.T;
                QuadPrim.RGBA2 = pV[3].GS.C;
                QuadPrim.XYZ2  = pV[3].GS.P;
                QuadPrim.UV3   = pV[2].GS.T;
                QuadPrim.RGBA3 = pV[2].GS.C;
                QuadPrim.XYZ3  = pV[2].GS.P;
            }
            pV+=4;
            C-=4;
        }
    }
    else
    {
        ASSERT(FALSE);
    }

    s_QuadBatchMgr.End();
}

//==========================================================================

static
void draw_DispatchWireTriangles( void )
{
    s64 PrimReg = SCE_GS_SET_PRIM( GIF_PRIM_LINE,
                                   1,           // shading method (flat, gouraud)
                                   0,           // texture mapping (off, on)
                                   0,           // fogging (off, on)
                                   s_IsAlpha,   // alpha blending (off, on)
                                   0,           // 1 pass anti-aliasing (off, on)
                                   0,           // tex-coord spec method (STQ, UV)
                                   0,           // context (1 or 2)
                                   0 );         // fragment value control (normal, fixed)
    s_LineBatchMgr.SetPrimReg( PrimReg );

    // Copy data from spad to output
    s_LineBatchMgr.Begin();

    vert* pV  = (vert*)SPAD.GetUsableStartAddr();
    s32 C = s_nVertsInBuffer;
    while( C>=3 )
    {
        if( (u32)pV[0].GS.P & (u32)pV[1].GS.P & (u32)pV[2].GS.P & 0x01 )
        {
            line_prim& Line0 = s_LineBatchMgr.NewPrimitive();
            Line0.RGBA0 = pV[0].GS.C;
            Line0.XYZ0  = pV[0].GS.P;
            Line0.RGBA1 = pV[1].GS.C;
            Line0.XYZ1  = pV[1].GS.P;

            line_prim& Line1 = s_LineBatchMgr.NewPrimitive();
            Line1.RGBA0 = pV[1].GS.C;
            Line1.XYZ0  = pV[1].GS.P;
            Line1.RGBA1 = pV[2].GS.C;
            Line1.XYZ1  = pV[2].GS.P;

            line_prim& Line2 = s_LineBatchMgr.NewPrimitive();
            Line2.RGBA0 = pV[2].GS.C;
            Line2.XYZ0  = pV[2].GS.P;
            Line2.RGBA1 = pV[0].GS.C;
            Line2.XYZ1  = pV[0].GS.P;
        }
        else
        {
            vert V[10];
            s32 NVerts;
            NVerts = ClipTriangle(pV,V);
            if( NVerts )
            {
                s32 OldI = NVerts-1;
                for( s32 i=0; i<NVerts; i++ )
                {
                    line_prim& Line = s_LineBatchMgr.NewPrimitive();
                    Line.RGBA0 = V[OldI].GS.C;
                    Line.XYZ0  = V[OldI].GS.P;
                    Line.RGBA1 = V[i].GS.C;
                    Line.XYZ1  = V[i].GS.P;
                    OldI = i;
                }
            }
        }
        pV+=3;
        C-=3;
    }

    s_LineBatchMgr.End();
}
//==========================================================================

static
void draw_DispatchTriangles( void )
{
    // Build header info
    s64 PrimReg = SCE_GS_SET_PRIM( GIF_PRIM_TRIANGLE,
                                   s_IsGouraud,         // shading method (flat, gouraud)
                                   s_IsTextured,        // texture mapping (off, on)
                                   0,                   // fogging (off, on)
                                   s_IsAlpha,           // alpha blending (off, on)
                                   0,                   // 1 pass anti-aliasing (off, on)
                                   0,                   // tex-coord spec method (STQ, UV)
                                   eng_GetGSContext(),  // context (1 or 2)
                                   0 );                 // fragment value control (normal, fixed)
    s_TriBatchMgr.SetPrimReg( PrimReg );

    // Copy data from spad to output
    s_TriBatchMgr.Begin();

    vert* pV  = (vert*)SPAD.GetUsableStartAddr();
    s32 C = s_nVertsInBuffer;
    while( C>=3 )
    {
        if( (u32)pV[0].GS.P & (u32)pV[1].GS.P & (u32)pV[2].GS.P & 0x01 )
        {
            tri_prim& Tri = s_TriBatchMgr.NewPrimitive();
            Tri.UV0   = pV[0].GS.T;
            Tri.RGBA0 = pV[0].GS.C;
            Tri.XYZ0  = pV[0].GS.P;
            Tri.UV1   = pV[1].GS.T;
            Tri.RGBA1 = pV[1].GS.C;
            Tri.XYZ1  = pV[1].GS.P;
            Tri.UV2   = pV[2].GS.T;
            Tri.RGBA2 = pV[2].GS.C;
            Tri.XYZ2  = pV[2].GS.P;
            Tri.PAD   = 0;
        }
        else
        {
            vert V[10];
            s32 NVerts;
            NVerts = ClipTriangle(pV,V);
            if( NVerts )
            {
                for( s32 i=1; i<NVerts-1; i++ )
                {
                    tri_prim& Tri = s_TriBatchMgr.NewPrimitive();
                    Tri.UV0   = V[0].GS.T;
                    Tri.RGBA0 = V[0].GS.C;
                    Tri.XYZ0  = V[0].GS.P;
                    Tri.UV1   = V[i].GS.T;
                    Tri.RGBA1 = V[i].GS.C;
                    Tri.XYZ1  = V[i].GS.P;
                    Tri.UV2   = V[i+1].GS.T;
                    Tri.RGBA2 = V[i+1].GS.C;
                    Tri.XYZ2  = V[i+1].GS.P;
                    Tri.PAD   = 0;
                }
            }

        }
        pV+=3;
        C-=3;
    }

    s_TriBatchMgr.End();
}

//==========================================================================

static
void draw_DispatchTriangleStrips( void )
{
    // Build header info
    s64 PrimReg = SCE_GS_SET_PRIM( GIF_PRIM_TRIANGLE,
                                   1,               // shading method (flat, gouraud)
                                   s_IsTextured,    // texture mapping (off, on)
                                   0,               // fogging (off, on)
                                   s_IsAlpha,       // alpha blending (off, on)
                                   0,               // 1 pass anti-aliasing (off, on)
                                   0,               // tex-coord spec method (STQ, UV)
                                   0,               // context (1 or 2)
                                   0 );             // fragment value control (normal, fixed)
    s_TriBatchMgr.SetPrimReg( PrimReg );

    // Copy data from spad to output
    s_TriBatchMgr.Begin();

    vert* pV  = (vert*)SPAD.GetUsableStartAddr();
    s32 C = s_nVertsInBuffer;
    while( C>=3 )
    {
        if( (u32)pV[0].GS.P & (u32)pV[1].GS.P & (u32)pV[2].GS.P & 0x01 )
        {
            tri_prim& Tri = s_TriBatchMgr.NewPrimitive();
            Tri.UV0  = pV[0].GS.T;
            Tri.RGBA0 = pV[0].GS.C;
            Tri.XYZ0  = pV[0].GS.P;
            Tri.UV1   = pV[1].GS.T;
            Tri.RGBA1 = pV[1].GS.C;
            Tri.XYZ1  = pV[1].GS.P;
            Tri.UV2   = pV[2].GS.T;
            Tri.RGBA2 = pV[2].GS.C;
            Tri.XYZ2  = pV[2].GS.P;
            Tri.PAD   = 0;
        }
        else
        {
            vert V[10];
            s32 NVerts;
            NVerts = ClipTriangle(pV,V);
            if( NVerts )
            {
                for( s32 i=1; i<NVerts-1; i++ )
                {
                    tri_prim& Tri = s_TriBatchMgr.NewPrimitive();
                    Tri.UV0   = V[0].GS.T;
                    Tri.RGBA0 = V[0].GS.C;
                    Tri.XYZ0  = V[0].GS.P;
                    Tri.UV1   = V[i].GS.T;
                    Tri.RGBA1 = V[i].GS.C;
                    Tri.XYZ1  = V[i].GS.P;
                    Tri.UV2   = V[i+1].GS.T;
                    Tri.RGBA2 = V[i+1].GS.C;
                    Tri.XYZ2  = V[i+1].GS.P;
                    Tri.PAD   = 0;
                }
            }
        }
        pV+=1;
        C-=1;
    }

    s_TriBatchMgr.End();
}

//==========================================================================

static
void draw_MainDispatch( void )
{
    if( s_nVertsInBuffer == 0 )
        return;

    if( 1 )
    {
        // Transform verts
        draw_TransformVerts( (vert*)SPAD.GetUsableStartAddr(), s_L2S, s_nVertsInBuffer );

        // Do actual dispatch
        s_pfnDispatch();
    }

    // Clear vert buffer
    s_BufferPos = (vert*)SPAD.GetUsableStartAddr();
    s_nVertsInBuffer = 0;
}

//==========================================================================

void draw_Begin( draw_primitive Primitive, u32 Flags )
{
    ASSERT( s_Initialized );
    ASSERT( !s_bBegin );

    // Confirm we are in a render context
    ASSERT( eng_InBeginEnd() );

    // Save primitive and flags
    s_Primitive   = Primitive;
    s_Flags       = Flags;
    s_Is2D        = (Flags & (DRAW_2D | DRAW_2D_KEEP_Z)        )?(TRUE):(FALSE);
    s_IsTextured  = (Flags & DRAW_TEXTURED  )?(TRUE):(FALSE);
    s_IsAlpha     = (Flags & DRAW_USE_ALPHA )?(TRUE):(FALSE);
    s_IsAdditive  = (Flags & DRAW_BLEND_ADD )?(TRUE):(FALSE);
    s_IsSubtractive=(Flags & DRAW_BLEND_SUB )?(TRUE):(FALSE);
    s_IsWire      = (Flags & DRAW_WIRE_FRAME)?(TRUE):(FALSE);
    s_UClamp      = (Flags & DRAW_U_CLAMP)   ?(TRUE):(FALSE);
    s_VClamp      = (Flags & DRAW_V_CLAMP)   ?(TRUE):(FALSE);
    s_IsZBuffered = (Flags & DRAW_NO_ZBUFFER)?(FALSE):(TRUE);
    s_IsZWrite    = (Flags & DRAW_NO_ZWRITE) ?(FALSE):(TRUE);
    s_IsSTQ       = FALSE;

    if( s_IsWire )
        s_IsTextured = FALSE;

    // Set internal state from primitive type
    switch( s_Primitive )
    {
    case DRAW_POINTS:
        s_pfnDispatch = draw_DispatchPoints;
        s_nTrigger = TRIGGER_POINTS;
        break;
    case DRAW_LINES:
        s_pfnDispatch = draw_DispatchLines;
        s_nTrigger = TRIGGER_LINES;
        break;
    case DRAW_RECTS:
        s_pfnDispatch = draw_DispatchQuadsAndRects;
        s_nTrigger = TRIGGER_RECTS;
        ASSERT( s_Is2D );
        break;
    case DRAW_QUADS:
        s_pfnDispatch = draw_DispatchQuadsAndRects;
        s_nTrigger = TRIGGER_QUADS;
        break;
    case DRAW_TRIANGLES:
        if( s_IsWire )
        {
            s_pfnDispatch = draw_DispatchWireTriangles;
            s_nTrigger = TRIGGER_TRIANGLES;
            s_IsSTQ = FALSE;
        }
        else
        {
            s_pfnDispatch = draw_DispatchTriangles;
            s_nTrigger = TRIGGER_TRIANGLES;
            s_IsSTQ = s_IsTextured;
        }
        break;
    case DRAW_TRIANGLE_STRIPS:
        s_pfnDispatch = draw_DispatchTriangleStrips;
        s_nTrigger = TRIGGER_TRIANGLES;
        s_IsSTQ = s_IsTextured;
        break;
    case DRAW_SPRITES:
        s_nSprites = 0;
        break;
    default:
        ASSERT( FALSE && "Primitive not implemented");
        break;
    }

    if( (Flags & DRAW_CUSTOM_MODE) == 0 )
    {
        gsreg_Begin( 4 );
        
        if( s_IsSubtractive )
        {
            // Set render states for SUBTRACTIVE
            gsreg_SetAlphaBlend( ALPHA_BLEND_MODE(C_ZERO,C_SRC,A_SRC,C_DST) );
        }
        else if( s_IsAdditive )
        {
            // Set render states for ADDITIVE
            gsreg_SetAlphaBlend( ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_SRC,C_DST) );
        }
        else if( s_IsAlpha )
        {
            // Set render states for ALPHA
            gsreg_SetAlphaBlend( ALPHA_BLEND_MODE(C_SRC,C_DST,A_SRC,C_DST) );
        }
        else
        {
            gsreg_SetAlphaBlend( ALPHA_BLEND_MODE(C_ZERO,C_ZERO,A_SRC,C_SRC) );
        }

    
        // Set render states for ZBUFFER
        gsreg_SetZBufferTest( s_IsZBuffered ? ZBUFFER_TEST_GEQUAL : ZBUFFER_TEST_ALWAYS );
      
        // Set render states for ZWRITE
        gsreg_SetZBufferUpdate(s_IsZWrite);
 
        // Set render states for WIREFRAME
        if( Flags & DRAW_WIRE_FRAME )
        {

        }
        else
        {

        }
        
        gsreg_SetClamping( s_UClamp, s_VClamp );

        // Clear Texture if not in textured mode
        if( !s_IsTextured )
        {
        }
        
        gsreg_End();
    }

    // Set in begin state
    s_bBegin = TRUE;

    // let everyone know that the scratchpad is now in use
    SPAD.Lock();

    // Clear vert buffer
    s_BufferPos = (vert*)SPAD.GetUsableStartAddr();
    s_nVertsInBuffer = 0;

    // Clear initial values
    s_UV.Zero();
    s_Color = XCOLOR_WHITE;

    s_nVerts = 0;
    s_nUVs = 0;
    s_nColors = 0;
    s_pVerts = NULL;
    s_pUVs = NULL;
    s_pColors = NULL;

    // Be sure matrices are up to date
    if( !s_Is2D )
        draw_UpdateMatrices();
}

//==========================================================================

static
void draw_Flush( void )
{
    // Flush any drawing we have queued up
    draw_MainDispatch();

    // Watch for leftover sprites
    if( s_Primitive == DRAW_SPRITES )
        draw_DispatchSprites();
}

//==========================================================================

void draw_End( void )
{
    ASSERT( s_bBegin );

    draw_Flush();

    // Clear in begin state
    s_bBegin = FALSE;

    // And we are finished with scratchpad...
    SPAD.Unlock();

    // Restore zbuffer if we turned it off
    if( !s_IsZBuffered || !s_IsZWrite )
    {
        gsreg_Begin( 2 );
        gsreg_SetZBufferTest( ZBUFFER_TEST_GEQUAL );
        gsreg_SetZBufferUpdate( TRUE );
        gsreg_End();
    }
}

//==========================================================================

void draw_SetL2W( const matrix4& L2W )
{
    s_L2W = L2W;
    s_L2S = s_W2S*s_L2W;
}

//==========================================================================

void draw_ClearL2W( void )
{
    s_L2W.Identity();
    s_L2S = s_W2S*s_L2W;
}
 
//==========================================================================

void draw_SetTexture( const xbitmap& Bitmap )
{
    draw_Flush();
    vram_Activate( Bitmap );
}

//==========================================================================

void draw_SetTexture( void )
{
    vram_Activate();
}

//==========================================================================

void draw_DisableBilinear( void )
{
    gsreg_Begin( 2 );
    gsreg_SetClamping( TRUE );
    gsreg_SetMipEquation( FALSE, 1.0f, 0, MIP_MAG_POINT, MIP_MIN_POINT );
    gsreg_End();
}

//==========================================================================

void draw_EnableBilinear( void )
{
}

//==========================================================================

void draw_UV( const vector2& UV )
{
    ASSERT( s_bBegin );
    ASSERT( s_Primitive != DRAW_SPRITES );

    s_UV = UV;
}

void draw_UV( f32 U, f32 V )
{
    ASSERT( s_bBegin );
    ASSERT( s_Primitive != DRAW_SPRITES );

    s_UV.X = U;
    s_UV.Y = V;
}

//==========================================================================

void draw_Color( const xcolor& Color )
{
    ASSERT( s_bBegin );
    ASSERT( s_Primitive != DRAW_SPRITES );
    xcolor c2( Color.B, Color.G, Color.R, Color.A );
    
    // NOTE: this will correctly convert 1.0 on PC to 1.0 on PS2    (JP)
    #if 1
    if( s_IsTextured )
    {
        c2.R = (c2.R == 255) ? 128 : (c2.R >> 1);
        c2.G = (c2.G == 255) ? 128 : (c2.G >> 1);
        c2.B = (c2.B == 255) ? 128 : (c2.B >> 1);
    }
    c2.A = (c2.A == 255) ? 128 : (c2.A >> 1);
    #else
    if( s_IsTextured )
    {
        c2.R >>= 1;
        c2.G >>= 1;
        c2.B >>= 1;
    }
    c2.A >>= 1;
    #endif

    s_Color = (u32)c2;
}

void draw_Color( f32 R, f32 G, f32 B, f32 A )
{
    ASSERT( s_bBegin );
    ASSERT( s_Primitive != DRAW_SPRITES );
    f32 S = ( s_IsTextured )?(128):(255);
    s_Color  = ((s32)(R*S))<< 0;
    s_Color |= ((s32)(G*S))<< 8;
    s_Color |= ((s32)(B*S))<<16;
    s_Color |= ((s32)(A*128))<<24;
}

//==========================================================================

void draw_Vertex( const vector3& Vertex )
{
    ASSERT( s_bBegin );
    ASSERT( s_Primitive != DRAW_SPRITES );

    // Setup vertex in buffer
    s_BufferPos->UV    = s_UV;
    s_BufferPos->Color = s_Color;
    s_BufferPos->Pos   = Vertex;

    // Advance buffer pointer
    s_BufferPos++;
    s_nVertsInBuffer++;

    // Check if it is time to dispatch this buffer
    if( s_nVertsInBuffer == s_nTrigger )
        draw_MainDispatch();
}

void draw_Vertex( f32 X, f32 Y, f32 Z )
{
    ASSERT( s_bBegin );
    ASSERT( s_Primitive != DRAW_SPRITES );

    // Setup vertex in buffer
    s_BufferPos->UV    = s_UV;
    s_BufferPos->Color = s_Color;
    s_BufferPos->Pos.Set(X,Y,Z);

    // Advance buffer pointer
    s_BufferPos++;
    s_nVertsInBuffer++;

    // Check if it is time to dispatch this buffer
    if( s_nVertsInBuffer == s_nTrigger )
        draw_MainDispatch();
}
//==========================================================================

void draw_UVs( const vector2* pUVs,    s32 Count, s32 Stride = sizeof(vector2) )
{
    ASSERT( s_bBegin );
    ASSERT( s_Primitive != DRAW_SPRITES );

    s_pUVs = pUVs;
    s_nUVs = Count;
    s_sUVs = Stride;
}

//==========================================================================

void draw_Colors( const xcolor*  pColors, s32 Count, s32 Stride = sizeof(xcolor ) )
{
    ASSERT( s_bBegin );
    ASSERT( s_Primitive != DRAW_SPRITES );

    s_pColors = pColors;
    s_nColors = Count;
    s_sColors = Stride;
}

//==========================================================================

void draw_Verts( const vector3* pVerts,  s32 Count, s32 Stride = sizeof(vector3) )
{
    ASSERT( s_bBegin );
    ASSERT( s_Primitive != DRAW_SPRITES );

    s_pVerts = pVerts;
    s_nVerts = Count;
    s_sVerts = Stride;
}


//==========================================================================
void    draw_Vertex_Composite ( const vector3& Vertex, const vector2& UV, const xcolor& Color )
{
    u32 Color_Raw = (((Color.A+1)>>8) + (Color.A >> 1))<<24;
  
    if( s_IsTextured )
    {
        Color_Raw |=    (((Color.B+1)>>8) + (Color.B >> 1))<<16     |
                        (((Color.G+1)>>8) + (Color.G >> 1))<<8      |
                        (((Color.R+1)>>8) + (Color.R >> 1));
    }
    
    // Setup vertex in buffer
    s_BufferPos->UV    = UV;
    s_BufferPos->Color = Color_Raw;
    s_BufferPos->Pos   = Vertex;
    
    // Advance buffer pointer
    s_BufferPos++;
    s_nVertsInBuffer++;
}

inline void    draw_Vertex_Composite_Textured ( const vector3& Vertex, const vector2& UV, const xcolor& Color )
{
    u32 Color_Raw =    
                        (((Color.B+1)>>8) + (Color.B >> 1))<<16     |
                        (((Color.G+1)>>8) + (Color.G >> 1))<<8      |
                        (((Color.R+1)>>8) + (Color.R >> 1))         |
                        (((Color.A+1)>>8) + (Color.A >> 1))<<24;
    
    // Setup vertex in buffer
    s_BufferPos->UV    = UV;
    s_BufferPos->Color = Color_Raw;
    s_BufferPos->Pos   = Vertex;
    
    // Advance buffer pointer
    s_BufferPos++;
    s_nVertsInBuffer++;
}

inline void    draw_Vertex_Composite_UnTextured ( const vector3& Vertex, const vector2& UV, const xcolor& Color )
{
    u32 Color_Raw = (((Color.A+1)>>8) + (Color.A >> 1))<<24;
    
    // Setup vertex in buffer
    s_BufferPos->UV    = UV;
    s_BufferPos->Color = Color_Raw;
    s_BufferPos->Pos   = Vertex;
    
    // Advance buffer pointer
    s_BufferPos++;
    s_nVertsInBuffer++;
}

void    draw_Vertex_Ptr_Block ( 
                           const xcolor*  pRGB,
                           const vector2* pUV,
                           const vector3* pVert,
                           const xbool*   pKick,
                           s32            NumVerts )
{
    s32  Uncheck = ( s_nVertsInBuffer + NumVerts ) - s_nTrigger;
    s32  j;

    if (Uncheck>0)
        Uncheck = NumVerts-Uncheck;
    else 
        Uncheck = NumVerts;
   
    if (s_IsTextured)
    {
        for(  j=0; j < Uncheck ; j++ )
        {
            if( *pKick )
            {
                draw_Vertex_Composite_Textured( pVert[ -2 ], pUV  [ -2 ], pRGB [ -2 ] );
                draw_Vertex_Composite_Textured( pVert[ -1 ], pUV  [ -1 ], pRGB [ -1 ] );
                draw_Vertex_Composite_Textured( pVert[  0 ], pUV  [  0 ], pRGB [  0 ] );
            }
            
            pRGB++;
            pUV++;
            pVert++;
            pKick++;
        }
    }
    else
    {
       for(  j=0; j < Uncheck ; j++ )
        {
            if( *pKick )
            {
                draw_Vertex_Composite_UnTextured( pVert[ -2 ], pUV  [ -2 ], pRGB [ -2 ] );
                draw_Vertex_Composite_UnTextured( pVert[ -1 ], pUV  [ -1 ], pRGB [ -1 ] );
                draw_Vertex_Composite_UnTextured( pVert[  0 ], pUV  [  0 ], pRGB [  0 ] );
            }
            
            pRGB++;
            pUV++;
            pVert++;
            pKick++;
        }
    }

    if ( Uncheck < NumVerts )
    {
        for( ; j < NumVerts ; j++ )
        {
            if( *pKick )
            {
                draw_Vertex_Composite( pVert[ -2 ], pUV  [ -2 ], pRGB [ -2 ] );

                if( s_nVertsInBuffer == s_nTrigger ) 
                    draw_MainDispatch();

                draw_Vertex_Composite( pVert[ -1 ], pUV  [ -1 ], pRGB [ -1 ] );

                 if( s_nVertsInBuffer == s_nTrigger ) 
                    draw_MainDispatch();

                draw_Vertex_Composite( pVert[  0 ], pUV  [  0 ], pRGB [  0 ] );

                 if( s_nVertsInBuffer == s_nTrigger ) 
                    draw_MainDispatch();

                break;
            }
            
            pRGB++;
            pUV++;
            pVert++;
            pKick++;
        }
        
        for( ; j < NumVerts ; j++ )
        {
            if( *pKick )
            {
                draw_Vertex_Composite( pVert[ -2 ], pUV  [ -2 ], pRGB [ -2 ] );
                draw_Vertex_Composite( pVert[ -1 ], pUV  [ -1 ], pRGB [ -1 ] );
                draw_Vertex_Composite( pVert[  0 ], pUV  [  0 ], pRGB [  0 ] );
            }
            
            pRGB++;
            pUV++;
            pVert++;
            pKick++;
        }
    }
}

void    draw_Vertex_Scale_Ptr_Block   ( 
                                 const xcolor*  pRGB,
                                 const vector2* pUV,
                                 const vector3* pVert,
                                 const xbool*   pKick,
                                 f32            Scale,
                                 s32            NumVerts )
{
    s32  Uncheck = ( s_nVertsInBuffer + NumVerts ) - s_nTrigger;
    s32  j;

    if (Uncheck>0)
        Uncheck = NumVerts-Uncheck;
    else 
        Uncheck = NumVerts;
    
      if (s_IsTextured)
    {
        for(  j=0; j < Uncheck ; j++ )
        {
            if( *pKick )
            {
                draw_Vertex_Composite_Textured( pVert[ -2 ], pUV  [ -2 ]*Scale, pRGB [ -2 ] );
                draw_Vertex_Composite_Textured( pVert[ -1 ], pUV  [ -1 ]*Scale, pRGB [ -1 ] );
                draw_Vertex_Composite_Textured( pVert[  0 ], pUV  [  0 ]*Scale, pRGB [  0 ] );
            }
            
            pRGB++;
            pUV++;
            pVert++;
            pKick++;
        }
    }
    else
    {
       for(  j=0; j < Uncheck ; j++ )
        {
            if( *pKick )
            {
                draw_Vertex_Composite_UnTextured( pVert[ -2 ], pUV  [ -2 ]*Scale, pRGB [ -2 ] );
                draw_Vertex_Composite_UnTextured( pVert[ -1 ], pUV  [ -1 ]*Scale, pRGB [ -1 ] );
                draw_Vertex_Composite_UnTextured( pVert[  0 ], pUV  [  0 ]*Scale, pRGB [  0 ] );
            }
            
            pRGB++;
            pUV++;
            pVert++;
            pKick++;
        }
    }
    
    if ( Uncheck < NumVerts )
    {
        for( ; j < NumVerts ; j++ )
        {
            if( *pKick )
            {
                draw_Vertex_Composite( pVert[ -2 ], pUV  [ -2 ]*Scale, pRGB [ -2 ] );

                if( s_nVertsInBuffer == s_nTrigger ) 
                    draw_MainDispatch();

                draw_Vertex_Composite( pVert[ -1 ], pUV  [ -1 ]*Scale, pRGB [ -1 ] );

                 if( s_nVertsInBuffer == s_nTrigger ) 
                    draw_MainDispatch();

                draw_Vertex_Composite( pVert[  0 ], pUV  [  0 ]*Scale, pRGB [  0 ] );

                 if( s_nVertsInBuffer == s_nTrigger ) 
                    draw_MainDispatch();

                break;
            }
            
            pRGB++;
            pUV++;
            pVert++;
            pKick++;
        }
        
        for( ; j < NumVerts ; j++ )
        {
            if( *pKick )
            {
                draw_Vertex_Composite( pVert[ -2 ], pUV  [ -2 ]*Scale, pRGB [ -2 ] );
                draw_Vertex_Composite( pVert[ -1 ], pUV  [ -1 ]*Scale, pRGB [ -1 ] );
                draw_Vertex_Composite( pVert[  0 ], pUV  [  0 ]*Scale, pRGB [  0 ] );
            }
            
            pRGB++;
            pUV++;
            pVert++;
            pKick++;
        }
    }
}

//==========================================================================

void draw_Index( s32 Index )
{
    ASSERT( s_bBegin );
    ASSERT( s_Primitive != DRAW_SPRITES );

    if( Index == -1 )
    {
        draw_MainDispatch();
    }
    else
    {
        // Setup vertex in buffer
        if( s_pUVs )
        {
            ASSERT( Index < s_nUVs );
            s_BufferPos->UV = s_pUVs[Index];
        }
        else
        {
            s_BufferPos->UV = s_UV;
        }

        // Get Color
        if( s_pColors )
        {
            ASSERT( Index < s_nColors );
            xcolor C = s_pColors[Index];
            
            C.R ^= C.B;
            C.B ^= C.R;
            C.R ^= C.B;

            if( s_IsTextured )
            {
                C.R >>= 1;
                C.G >>= 1;
                C.B >>= 1;
            }
            C.A >>= 1;

            s_BufferPos->Color = C;
        }
        else
        {
            s_BufferPos->Color = s_Color;
        }

        // Setup vertex in buffer
        ASSERT( Index < s_nVerts );
        s_BufferPos->Pos = s_pVerts[Index];

        // Advance buffer pointer
        s_BufferPos++;
        s_nVertsInBuffer++;

        // Check if it is time to dispatch this buffer
        if( s_nVertsInBuffer == s_nTrigger )
            draw_MainDispatch();
    }
}

//==========================================================================

void draw_Execute( const s16* pIndices, s32 NIndices )
{
    s32     i;

    ASSERT( s_bBegin );
    ASSERT( s_Primitive != DRAW_SPRITES );

    // Loop through indices supplied
    for( i=0; i<NIndices; i++ )
    {
        draw_Index( pIndices[i] );
    }
}

//==========================================================================

void draw_Init( void )
{
    s_Initialized = TRUE;

    giftag GIF;

    // set up the point batch
    GIF.Build( GIF_MODE_REGLIST, 2, 1, 0, 0, 0, 1 );
    GIF.Reg( GIF_REG_RGBAQ, GIF_REG_XYZ2 );
    s_PointBatchMgr.SetRegGiftag( GIF );

    // set up the line batch
    GIF.Build( GIF_MODE_REGLIST, 4, 1, 0, 0, 0, 1 );
    GIF.Reg( GIF_REG_RGBAQ, GIF_REG_XYZ2, GIF_REG_RGBAQ, GIF_REG_XYZ2 );
    s_LineBatchMgr.SetRegGiftag( GIF );

    // set up the quad batch
    GIF.Build( GIF_MODE_REGLIST, 12, 1, 0, 0, 0, 1 );
    GIF.Reg( GIF_REG_ST,  GIF_REG_RGBAQ, GIF_REG_XYZ3, 
             GIF_REG_ST,  GIF_REG_RGBAQ, GIF_REG_XYZ3, 
             GIF_REG_ST,  GIF_REG_RGBAQ, GIF_REG_XYZ2, 
             GIF_REG_ST,  GIF_REG_RGBAQ, GIF_REG_XYZ2 );
    s_QuadBatchMgr.SetRegGiftag( GIF );

    // set up the tri batch
    GIF.Build( GIF_MODE_REGLIST, 10, 1, 0, 0, 0, 1 );
    GIF.Reg( GIF_REG_ST,  GIF_REG_RGBAQ, GIF_REG_XYZ2, 
             GIF_REG_ST,  GIF_REG_RGBAQ, GIF_REG_XYZ2, 
             GIF_REG_ST,  GIF_REG_RGBAQ, GIF_REG_XYZ2, 
             GIF_REG_NOP );
    s_TriBatchMgr.SetRegGiftag( GIF );
}

//==========================================================================

void draw_Kill( void )
{
    s_Initialized = FALSE;
}

//==========================================================================

void    draw_Sprite     ( const vector3& Position,  // Hot spot (2D Left-Top), (3D Center)
                          const vector2& WH,        // (2D pixel W&H), (3D World W&H)
                          const xcolor&  Color )
{
    ASSERT( s_bBegin );
    ASSERT( s_Primitive == DRAW_SPRITES );

    sprite* p = ((sprite*)SPAD.GetUsableStartAddr()) + s_nSprites;


    p->IsRotated = FALSE;
    p->Position  = Position;
    p->WH        = WH;
    p->Color.R   = Color.B >>1;
    p->Color.G   = Color.G >>1;
    p->Color.B   = Color.R >>1;
    p->Color.A   = Color.A >>1;
    p->Rotation  = 0.0f;
    p->UV0.Set( 0.0f, 0.0f );
    p->UV1.Set( 1.0f, 1.0f );

    s_nSprites++;
    if( s_nSprites == NUM_SPRITES )
        draw_DispatchSprites();
}

//==========================================================================

void    draw_SpriteUV   ( const vector3& Position,  // Hot spot (2D Left-Top), (3D Center)
                          const vector2& WH,        // (2D pixel W&H), (3D World W&H)
                          const vector2& UV0,       // Upper Left   UV  [0.0 - 1.0]
                          const vector2& UV1,       // Bottom Right UV  [0.0 - 1.0]
                          const xcolor&  Color )
{
    ASSERT( s_bBegin );
    ASSERT( s_Primitive == DRAW_SPRITES );

    sprite* p = ((sprite*)SPAD.GetUsableStartAddr()) + s_nSprites;

    p->IsRotated = FALSE;
    p->Position  = Position;
    p->WH        = WH;
    p->UV0       = UV0;
    p->UV1       = UV1;
    p->Color.R   = Color.B >>1;
    p->Color.G   = Color.G >>1;
    p->Color.B   = Color.R >>1;
    p->Color.A   = Color.A == 255 ? 128 : (Color.A>>1);
    p->Rotation  = 0.0f;

    s_nSprites++;
    if( s_nSprites == NUM_SPRITES )
        draw_DispatchSprites();
}

//==========================================================================

void    draw_SpriteUV   ( const vector3& Position,  // Hot spot (3D Center)
                          const vector2& WH,        // (3D World W&H)
                          const vector2& UV0,       // Upper Left   UV  [0.0 - 1.0]
                          const vector2& UV1,       // Bottom Right UV  [0.0 - 1.0]
                          const xcolor&  Color,     //
                                radian   Rotate )
{
    ASSERT( s_bBegin );
    ASSERT( s_Primitive == DRAW_SPRITES );

    sprite* p = ((sprite*)SPAD.GetUsableStartAddr()) + s_nSprites;

    p->IsRotated = TRUE;
    p->Position  = Position;
    p->WH        = WH;
    p->UV0       = UV0;
    p->UV1       = UV1;
    p->Color.R   = Color.B >>1;
    p->Color.G   = Color.G >>1;
    p->Color.B   = Color.R >>1;
    p->Color.A   = Color.A >>1;
    p->Rotation  = Rotate;

    s_nSprites++;
    if( s_nSprites == NUM_SPRITES )
        draw_DispatchSprites();
}


//==========================================================================

void    draw_OrientedQuad(const vector3& Pos0,
                          const vector3& Pos1,
                          const vector2& UV0,
                          const vector2& UV1,
                          const xcolor&  Color0,
                          const xcolor&  Color1,
                                f32      Radius )
{
    (void)Color0;
    (void)Color1;

    ASSERT( s_bBegin );
    ASSERT( s_Primitive == DRAW_TRIANGLES );

    vector3 Dir = Pos1 - Pos0;
    if( !Dir.SafeNormalize() )
        return;

    vector3 CrossDir = Dir.Cross( eng_GetView()->GetPosition() - Pos0 );
    if( !CrossDir.SafeNormalize() )
        return;

    CrossDir *= Radius;

    draw_Color( Color1 );
    draw_UV( UV1.X, UV1.Y );    draw_Vertex( Pos1 + CrossDir );
    draw_UV( UV1.X, UV0.Y );    draw_Vertex( Pos1 - CrossDir );
    draw_Color( Color0 );
    draw_UV( UV0.X, UV0.Y );    draw_Vertex( Pos0 - CrossDir );
    draw_UV( UV0.X, UV0.Y );    draw_Vertex( Pos0 - CrossDir );
    draw_UV( UV0.X, UV1.Y );    draw_Vertex( Pos0 + CrossDir );
    draw_Color( Color1 );
    draw_UV( UV1.X, UV1.Y );    draw_Vertex( Pos1 + CrossDir );
}

//==========================================================================

void    draw_OrientedQuad(const vector3& Pos0,
                          const vector3& Pos1,
                          const vector2& UV0,
                          const vector2& UV1,
                          const xcolor&  Color0,
                          const xcolor&  Color1,
                                f32      Radius0,
                                f32      Radius1)
{
    ASSERT( s_bBegin );
    ASSERT( s_Primitive == DRAW_TRIANGLES );

    vector3 Dir = Pos1 - Pos0;
    if( !Dir.SafeNormalize() )
        return;

    vector3 CrossDir = Dir.Cross( eng_GetView()->GetPosition() - Pos0 );
    if( !CrossDir.SafeNormalize() )
        return;

    vector3 Cross0 = CrossDir * Radius0;
    vector3 Cross1 = CrossDir * Radius1;

    draw_Color( Color1 );
    draw_UV( UV1.X, UV1.Y );    draw_Vertex( Pos1 + Cross1 );
    draw_UV( UV1.X, UV0.Y );    draw_Vertex( Pos1 - Cross1 );
    draw_Color( Color0 );
    draw_UV( UV0.X, UV0.Y );    draw_Vertex( Pos0 - Cross0 );
    draw_UV( UV0.X, UV0.Y );    draw_Vertex( Pos0 - Cross0 );
    draw_UV( UV0.X, UV1.Y );    draw_Vertex( Pos0 + Cross0 );
    draw_Color( Color1 );
    draw_UV( UV1.X, UV1.Y );    draw_Vertex( Pos1 + Cross1 );
}

//==========================================================================

void    draw_OrientedStrand (const vector3* pPosData,
                                   s32      NumPts,
                             const vector2& UV0,
                             const vector2& UV1,
                             const xcolor&  Color0,
                             const xcolor&  Color1,
                                   f32      Radius )
{
    s32 i;
    vector3 quad[6];        //  storage for a quad, plus an extra edge for averaging
    vector2 uv0, uv1;

    uv0 = UV0;
    uv1 = UV1;

    ASSERT( s_bBegin );
    ASSERT( s_Primitive == DRAW_TRIANGLES );

    for ( i = 0; i < NumPts - 1; i++ )
    {
        vector3 Dir = pPosData[i+1] - pPosData[i];
        if( !Dir.SafeNormalize() )
            Dir(0,1,0);
        
        vector3 CrossDir = Dir.Cross( eng_GetView()->GetPosition() - pPosData[i] );
        if( !CrossDir.SafeNormalize() )
            CrossDir(1,0,0);

        CrossDir *= Radius;

        if ( i == 0 )
        {
            // first set, no point averaging necessary
            quad[ 0 ] =     pPosData[ i ] + CrossDir;
            quad[ 1 ] =     pPosData[ i ] - CrossDir;
            quad[ 2 ] =     pPosData[ i + 1 ] + CrossDir;
            quad[ 3 ] =     pPosData[ i + 1 ] - CrossDir;
        }
        else
        {
            vector3 tq1, tq2;

            tq1 = pPosData[ i ] + CrossDir;
            tq2 = pPosData[ i ] - CrossDir;

            // second set...average verts
            quad[ 2 ] =     ( quad[2] + tq1 ) / 2.0f;
            quad[ 3 ] =     ( quad[3] + tq2 ) / 2.0f;
            quad[ 4 ] =     pPosData[ i + 1 ] + CrossDir;
            quad[ 5 ] =     pPosData[ i + 1 ] - CrossDir;            
        }

        // render q0, q1, q2, and q3 then shift all of them down
        if ( i > 0 )
        {
            draw_Color( Color1 );
            draw_UV( uv1.X, uv1.Y );    draw_Vertex( quad[2] );
            draw_UV( uv1.X, uv0.Y );    draw_Vertex( quad[3] );
            draw_Color( Color0 );
            draw_UV( uv0.X, uv0.Y );    draw_Vertex( quad[1] );
            draw_UV( uv0.X, uv0.Y );    draw_Vertex( quad[1] );
            draw_UV( uv0.X, uv1.Y );    draw_Vertex( quad[0] );
            draw_Color( Color1 );
            draw_UV( uv1.X, uv1.Y );    draw_Vertex( quad[2] );

            // cycle the UV's
            uv0.X = uv1.X;
            uv1.X += ( UV1.X - UV0.X );

            quad[0] = quad[2];
            quad[1] = quad[3];
            quad[2] = quad[4];
            quad[3] = quad[5];
        }
        
    }

    // last edge...
    draw_Color( Color1 );
    draw_UV( UV1.X, UV1.Y );    draw_Vertex( quad[2] );
    draw_UV( UV1.X, UV0.Y );    draw_Vertex( quad[3] );
    draw_Color( Color0 );
    draw_UV( UV0.X, UV0.Y );    draw_Vertex( quad[1] );
    draw_UV( UV0.X, UV0.Y );    draw_Vertex( quad[1] );
    draw_UV( UV0.X, UV1.Y );    draw_Vertex( quad[0] );
    draw_Color( Color1 );
    draw_UV( UV1.X, UV1.Y );    draw_Vertex( quad[2] );

}

//==============================================================================

void draw_SetZBuffer( const irect& Rect, f32 Z )
{
    // Make sure Z is valid
    ASSERT(Z >= 0.0f) ;
    ASSERT(Z <= 1.0f) ;

    // Turn off writing the ARGB
    eng_PushGSContext(0);
    gsreg_Begin( 3 );
    gsreg_SetFBMASK( 0xFFFFFFFF );
    gsreg_SetZBufferTest( ZBUFFER_TEST_ALWAYS );
    gsreg_SetZBufferUpdate( TRUE );
    gsreg_End();
    eng_PopGSContext();

    eng_PushGSContext(1);
    gsreg_Begin( 3 );
    gsreg_SetFBMASK( 0xFFFFFFFF );
    gsreg_SetZBufferTest( ZBUFFER_TEST_ALWAYS );
    gsreg_SetZBufferUpdate( TRUE );
    gsreg_End();
    eng_PopGSContext();

    {
        DLPtrAlloc( pZClear, zbuffer_clear );

        // Build header info
        pZClear->DMA.SetCont( sizeof(zbuffer_clear) - sizeof(dmatag) );
        pZClear->DMA.MakeDirect();
        pZClear->PRIMGIF.Build( GIF_MODE_REGLIST, 1, 1, 0, 0, 0, 0 );
        pZClear->PRIMGIF.Reg( GIF_REG_PRIM ) ;
        pZClear->prim = SCE_GS_SET_PRIM(GIF_PRIM_TRIANGLESTRIP,
                                        1,               // shading method (flat, gouraud)
                                        0,               // texture mapping (off, on)
                                        0,               // fogging (off, on)
                                        0,               // alpha blending (off, on)
                                        0,               // 1 pass anti-aliasing (off, on)
                                        0,               // tex-coord spec method (STQ, UV)
                                        0,               // context (1 or 2)
                                        0);              // fragment value control (normal, fixed)
        pZClear->GIF.Build( GIF_MODE_REGLIST, 4, 1, 0, 0, 0, 1 );
        pZClear->GIF.Reg( GIF_REG_XYZ2, GIF_REG_XYZ2, GIF_REG_XYZ2, GIF_REG_XYZ2 );

        s32 X0 = ((Rect.l+OFFSET_X)*16)&0xFFFF;
        s32 Y0 = ((Rect.t+OFFSET_Y)*16)&0xFFFF;
        s32 X1 = ((Rect.r+OFFSET_X)*16)&0xFFFF;
        s32 Y1 = ((Rect.b+OFFSET_Y)*16)&0xFFFF;
        s32 iZ = (s32)((1.0f - Z) * 0x00FFFFFF) ;   // 24bit z buffer (note: PS2 Z buffer is backwards!)
        pZClear->XYZ0 = SCE_GS_SET_XYZ(X0,Y0,iZ);
        pZClear->XYZ1 = SCE_GS_SET_XYZ(X0,Y1,iZ);
        pZClear->XYZ2 = SCE_GS_SET_XYZ(X1,Y0,iZ);
        pZClear->XYZ3 = SCE_GS_SET_XYZ(X1,Y1,iZ);
    }

    // Restore states
    eng_PushGSContext(0);
    gsreg_Begin( 2 );
    gsreg_SetFBMASK( 0x00000000 );
    gsreg_SetZBufferTest( ZBUFFER_TEST_GEQUAL );
    gsreg_End();
    eng_PopGSContext();

    eng_PushGSContext(1);
    gsreg_Begin( 2 );
    gsreg_SetFBMASK( 0x00000000 );
    gsreg_SetZBufferTest( ZBUFFER_TEST_GEQUAL );
    gsreg_End();
    eng_PopGSContext();
}

//==============================================================================

void draw_ClearZBuffer( const irect& Rect )
{
    // Set Z to far plane
    draw_SetZBuffer(Rect, 1.0f) ;
}

//==============================================================================

void draw_FillZBuffer( const irect& Rect )
{
    // Set Z to near plane
    draw_SetZBuffer(Rect, 0.0f) ;
}

//==============================================================================

void ps2draw_SetAlpha( xbool IsAlpha )
{
    s_IsAlpha = IsAlpha;
}

//==============================================================================

void ps2draw_SetGouraud( xbool IsGouraud )
{
    s_IsGouraud = IsGouraud;
}

//==============================================================================

void draw_SpriteImmediate( const vector2& Position,  // Hot spot (2D Left-Top), (2D Center)
                           const vector2& WH,        // (2D pixel W&H), (3D World W&H)
                           const vector2& UV0,       // Upper Left   UV  [0.0 - 1.0]
                           const vector2& UV1,       // Bottom Right UV  [0.0 - 1.0]
                           const xcolor & Color )
{
    ASSERT( s_bBegin );

    // convert the color to ps2-friendly values
    xcolor C2 = draw_ColorToPS2( Color, TRUE );

    // convert the position to a ps2-friendly position
    s32 SX0 = (s32)(Position.X*16.0f) + (OFFSET_X<<4);
    s32 SY0 = (s32)(Position.Y*16.0f) + (OFFSET_Y<<4);
    s32 SX1 = SX0 + (s32)(WH.X*16.0f);
    s32 SY1 = SY0 + (s32)(WH.Y*16.0f);

    // convert the uv's into ps2-friendly format
    f32 fU0 = UV0.X;    f32 fV0 = UV0.Y;
    f32 fU1 = UV1.X;    f32 fV1 = UV1.Y;
    s32 U0 = reinterpret_cast<u32&>(fU0);
    s32 V0 = reinterpret_cast<u32&>(fV0);
    s32 U1 = reinterpret_cast<u32&>(fU1);
    s32 V1 = reinterpret_cast<u32&>(fV1);

    // draw the sprite
    ASSERT( s_Is2D );
    gsreg_Begin( 6 );
    gsreg_Set( SCE_GS_PRIM,  SCE_GS_SET_PRIM( GIF_PRIM_SPRITE, 0, 1, 0, s_IsAlpha, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( C2.R, C2.G, C2.B, C2.A, 0x3f800000 ) );
    gsreg_Set( SCE_GS_ST,    SCE_GS_SET_ST( U0, V0 ) );
    gsreg_Set( SCE_GS_XYZ2,  SCE_GS_SET_XYZ2( SX0, SY0, 0 ) );
    gsreg_Set( SCE_GS_ST,    SCE_GS_SET_ST( U1, V1 ) );
    gsreg_Set( SCE_GS_XYZ2,  SCE_GS_SET_XYZ2( SX1, SY1, 0 ) );
    gsreg_End();
}

//==============================================================================

void draw_RectImmediate( const irect& Rect,
                         xcolor       Color,
                         xbool        IsWire )
{
    // begin the drawing mode
    draw_Begin( DRAW_QUADS, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER|DRAW_NO_ZWRITE);

    // convert the color to ps2-friendly color
    xcolor C = draw_ColorToPS2( Color, FALSE );

    // convert the rect into a ps2-friendly rect
    irect Rect2 = Rect;
    Rect2.l = (Rect2.l + OFFSET_X)<<4;
    Rect2.t = (Rect2.t + OFFSET_X)<<4;
    Rect2.r = (Rect2.r + OFFSET_X)<<4;
    Rect2.b = (Rect2.b + OFFSET_X)<<4;

    // render the rect
    ASSERT( s_Is2D );
    if( IsWire )
    {
        gsreg_Begin( 6 );
        gsreg_Set( SCE_GS_PRIM,  SCE_GS_SET_PRIM( GIF_PRIM_LINESTRIP, 0, 0, 0, TRUE, 0, 0, 0, 0 ) );
        gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( C.R, C.G, C.B, C.A, 0x3f800000 ) );
        gsreg_Set( SCE_GS_XYZ2,  SCE_GS_SET_XYZ2( Rect2.l, Rect2.t, 0 ) );
        gsreg_Set( SCE_GS_XYZ2,  SCE_GS_SET_XYZ2( Rect2.l, Rect2.b, 0 ) );
        gsreg_Set( SCE_GS_XYZ2,  SCE_GS_SET_XYZ2( Rect2.r, Rect2.b, 0 ) );
        gsreg_Set( SCE_GS_XYZ2,  SCE_GS_SET_XYZ2( Rect2.r, Rect2.t, 0 ) );
        gsreg_End();
    }
    else
    {
        gsreg_Begin( 4 );
        gsreg_Set( SCE_GS_PRIM,  SCE_GS_SET_PRIM( GIF_PRIM_SPRITE, 0, 0, 0, TRUE, 0, 0, 0, 0 ) );
        gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( C.R, C.G, C.B, C.A, 0x3f800000 ) );
        gsreg_Set( SCE_GS_XYZ2,  SCE_GS_SET_XYZ2( Rect2.l, Rect2.t, 0 ) );
        gsreg_Set( SCE_GS_XYZ2,  SCE_GS_SET_XYZ2( Rect2.r, Rect2.b, 0 ) );
        gsreg_End();
    }

    draw_End();
}

//==============================================================================

void draw_GouraudRectImmediate( const irect&   Rect,
                                const xcolor&  c1,
                                const xcolor&  c2,
                                const xcolor&  c3,
                                const xcolor&  c4,
                                xbool          DoAdditive )
{
    if (DoAdditive)
        draw_Begin( DRAW_QUADS, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER|DRAW_NO_ZWRITE|DRAW_BLEND_ADD );
    else
        draw_Begin( DRAW_QUADS, DRAW_2D|DRAW_USE_ALPHA|DRAW_NO_ZBUFFER|DRAW_NO_ZWRITE);

    // convert the color to ps2-friendly colors
    xcolor C1 = draw_ColorToPS2( c1, FALSE );
    xcolor C2 = draw_ColorToPS2( c2, FALSE );
    xcolor C3 = draw_ColorToPS2( c3, FALSE );
    xcolor C4 = draw_ColorToPS2( c4, FALSE );

    // convert the rect into a ps2-friendly rect
    irect Rect2 = Rect;
    Rect2.l = (Rect2.l + OFFSET_X)<<4;
    Rect2.t = (Rect2.t + OFFSET_X)<<4;
    Rect2.r = (Rect2.r + OFFSET_X)<<4;
    Rect2.b = (Rect2.b + OFFSET_X)<<4;

    // draw the rect
    ASSERT( s_Is2D );
    gsreg_Begin( 9 );
    gsreg_Set( SCE_GS_PRIM,  SCE_GS_SET_PRIM( GIF_PRIM_TRIANGLESTRIP, 1, 0, 0, s_IsAlpha, 0, 0, 0, 0 ) );
    
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( C1.R, C1.G, C1.B, C1.A, 0x3f800000 ) );
    gsreg_Set( SCE_GS_XYZ3,  SCE_GS_SET_XYZ3( Rect2.l, Rect2.t, 0 ) );
    
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( C2.R, C2.G, C2.B, C2.A, 0x3f800000 ) );
    gsreg_Set( SCE_GS_XYZ3,  SCE_GS_SET_XYZ3( Rect2.l, Rect2.b, 0 ) );
    
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( C4.R, C4.G, C4.B, C4.A, 0x3f800000 ) );
    gsreg_Set( SCE_GS_XYZ2,  SCE_GS_SET_XYZ2( Rect2.r, Rect2.t, 0 ) );
    
    gsreg_Set( SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( C3.R, C3.G, C3.B, C3.A, 0x3f800000 ) );
    gsreg_Set( SCE_GS_XYZ2,  SCE_GS_SET_XYZ2( Rect2.r, Rect2.b, 0 ) );

    gsreg_End();

    draw_End();
}

//==============================================================================
