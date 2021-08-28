
#include "LightMgr.hpp"
#include "e_ScratchMem.hpp"

//=========================================================================
// The global object
//=========================================================================

light_mgr   g_LightMgr;

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

light_mgr::light_mgr( void ) :
    m_FirstLink             ( -1 ),
    m_NFadingLights         ( 0 ),
    m_NDynamicLights        ( 0 ),
    m_NCharLights           ( 0 ),
    m_bInCollection         ( FALSE ),
    m_NSpadLights           ( 0 ),
    m_nNonCharLightsInSpad  ( 0 ),
    m_pSpadLights           ( NULL ),
    m_NCollectedLights      ( 0 )

{
    x_memset( m_FadingLights, 0, sizeof(fading_light) * MAX_FADING_LIGHTS );
}

//=========================================================================

light_mgr::~light_mgr( void )
{
}

//=========================================================================

void light_mgr::AddFadingLight( const vector3& Pos, const xcolor& C, f32 Radius, f32 Intensity, f32 FadeTime )
{
    // early return
    if ( FadeTime <= 0.0f )
        return;

    // add a light to the linked list of lights
    s32 iLight = AddLight();

    // now we can fill in the light data
    m_FadingLights[iLight].Pos            = Pos;
    m_FadingLights[iLight].Radius         = Radius;
    m_FadingLights[iLight].StartColor     = C;
    m_FadingLights[iLight].CurrentColor   = C;
    m_FadingLights[iLight].FadeTime       = FadeTime;
    m_FadingLights[iLight].ElapsedTime    = 0.0f;
    m_FadingLights[iLight].Valid          = TRUE;
    m_FadingLights[iLight].InterpolationT = 0.0f;
    m_FadingLights[iLight].Intensity      = Intensity;
}

//=========================================================================

void light_mgr::AddDynamicLight( const vector3& Pos, const xcolor& C, f32 Radius, f32 Intensity, xbool CharOnly )
{
    if ( CharOnly )
    {
        if ( m_NCharLights >= MAX_CHAR_LIGHTS )
        {
            //ASSERT( FALSE );
            return;
        }
        m_CharLights[m_NCharLights].Pos       = Pos;
        m_CharLights[m_NCharLights].Radius    = Radius;
        m_CharLights[m_NCharLights].Intensity = Intensity;
        m_CharLights[m_NCharLights].Color     = C;
        m_NCharLights++;
    }
    else
    {
        if ( m_NDynamicLights >= MAX_DYNAMIC_LIGHTS )
        {
            //ASSERT( FALSE );
            return;
        }
        m_DynamicLights[m_NDynamicLights].Pos       = Pos;
        m_DynamicLights[m_NDynamicLights].Radius    = Radius;
        m_DynamicLights[m_NDynamicLights].Intensity = Intensity;
        m_DynamicLights[m_NDynamicLights].Color     = C;
        m_NDynamicLights++;
    }
}

//=========================================================================

void light_mgr::OnUpdate( f32 DeltaTime )
{
    for ( s32 iLight = 0; iLight < MAX_FADING_LIGHTS; iLight++ )
    {
        if ( m_FadingLights[iLight].Valid )
        {
            m_FadingLights[iLight].ElapsedTime += DeltaTime;
            
            if ( m_FadingLights[iLight].ElapsedTime >= m_FadingLights[iLight].FadeTime )
            {
                RemoveLight( iLight );
            }
            else
            {
                f32 t = m_FadingLights[iLight].ElapsedTime / m_FadingLights[iLight].FadeTime;
                f32 R = (f32)m_FadingLights[iLight].StartColor.R;
                f32 G = (f32)m_FadingLights[iLight].StartColor.G;
                f32 B = (f32)m_FadingLights[iLight].StartColor.B;
            
                m_FadingLights[iLight].InterpolationT = t;
                m_FadingLights[iLight].CurrentColor.R = (u8)(R*(1.0f-t));
                m_FadingLights[iLight].CurrentColor.G = (u8)(G*(1.0f-t));
                m_FadingLights[iLight].CurrentColor.B = (u8)(B*(1.0f-t));
            }
        }
    }
}

//=========================================================================

s32 light_mgr::AddLight( void )
{
    // find the first available light, remembering the light with the least
    // amount of time left in case we need to kick one out.
    s32 iLight;
    s32 WeakestI       = 100000000;
    s32 LightToReplace = -1;

    for ( iLight = 0; iLight < MAX_FADING_LIGHTS; iLight++ )
    {
        if ( !m_FadingLights[iLight].Valid )
            break;

        // grab the light intensity
        s32 R = m_FadingLights[iLight].CurrentColor.R;
        s32 G = m_FadingLights[iLight].CurrentColor.G;
        s32 B = m_FadingLights[iLight].CurrentColor.B;
        s32 I = R*R + G*G + B*B;

        if ( (LightToReplace == -1) || (I < WeakestI) )
        {
            LightToReplace = iLight;
            WeakestI       = I;
        }
    }

    // was there room in the array?
    if ( iLight == MAX_FADING_LIGHTS )
    {
        // replace an old light, the old one was already in the linked list,
        // so no need to fix up any links

        // NOTE: We should rarely hit this, but if we do and it starts to kick out
        // any vital lights, we could start merging lights together (sounds like fun!)
        ASSERT( LightToReplace != -1 );
        iLight = LightToReplace;
    }
    else
    {
        //#### TODO: If this linked-list business starts hitting the cache pretty
        // hard, insert into the middle of the list so that we at least move
        // forward through the cache.

        // add the new light to the start of the linked list
        if ( m_FirstLink != -1 )
        {
            m_FadingLights[m_FirstLink].PrevLink = iLight;
        }

        m_FadingLights[iLight].NextLink = m_FirstLink;
        m_FadingLights[iLight].PrevLink = -1;
        m_FirstLink                     = iLight;
        m_NFadingLights++;
    }

    ASSERT( (iLight >= 0) && (iLight < MAX_FADING_LIGHTS) );
    return iLight;
}

//=========================================================================

void light_mgr::RemoveLight( s32 LightIndex )
{
    ASSERT( (LightIndex >= 0) && (LightIndex < MAX_FADING_LIGHTS) );
    ASSERT( m_FadingLights[LightIndex].Valid );
    
    // invalidate the light
    m_FadingLights[LightIndex].Valid = FALSE;

    // patch up the linked list indices
    s32 Prev = m_FadingLights[LightIndex].PrevLink;
    s32 Next = m_FadingLights[LightIndex].NextLink;
    if ( Prev != -1 )   m_FadingLights[Prev].NextLink = Next;
    if ( Next != -1 )   m_FadingLights[Next].PrevLink = Prev;
    if ( LightIndex == m_FirstLink )
    {
        ASSERT( Prev == -1 );
        m_FirstLink = Next;
    }

    m_NFadingLights--;
}

//=========================================================================

void light_mgr::ReduceCollectedLights( s32 MaxLightCount )
{
    // if there were more than the max lights intersecting, start cutting
    // some of them out (maybe we can merge them instead? sounds CPU intensive,
    // as if it weren't already bad enough!)

    // ideally, we'd calculate the lights intensity at the corners of the
    // collected objects bbox at the very least, but if we're getting inside
    // this while loop, the artists have probably placed too many dynamic
    // lights anyway
    while ( m_NCollectedLights > MaxLightCount )
    {
        f32 LowestI    = F32_MAX;
        s32 LightToCut = -1;

        for ( s32 i = 0; i < m_NCollectedLights; i++ )
        {
            s32 LightIndex = m_CollectedLights[i];
            f32 I;
            if ( LightIndex & 0x80000000 )
            {
                LightIndex &= 0x7FFFFFFF;
        
                dynamic_light& Light = m_DynamicLights[LightIndex];
                f32 R = Light.Color.R*Light.Intensity;
                f32 G = Light.Color.G*Light.Intensity;
                f32 B = Light.Color.B*Light.Intensity;
                I     = R*R+G*G+B*B;
            }
            else
            {
                fading_light& Light = m_FadingLights[LightIndex];
                f32 R = Light.CurrentColor.R*Light.Intensity;
                f32 G = Light.CurrentColor.G*Light.Intensity;
                f32 B = Light.CurrentColor.B*Light.Intensity;
                I     = R*R+G*G+B*B;
            }

            if ( I < LowestI )
            {
                LowestI    = I;
                LightToCut = i;
            }
        }

        if ( LightToCut==-1 )
        {
            ASSERT( FALSE );
            LightToCut = 0;
        }

        // remove a light
        m_NCollectedLights--;
        x_memmove( &m_CollectedLights[LightToCut], &m_CollectedLights[LightToCut+1], sizeof(s32)*(m_NCollectedLights-LightToCut) );
    }
}

//=========================================================================

s32 SpadLightSortFn( const void* pA, const void* pB )
{
    light_mgr::spad_light* pLightA = (light_mgr::spad_light*)pA;
    light_mgr::spad_light* pLightB = (light_mgr::spad_light*)pB;

    if( pLightA->CharOnly > pLightB->CharOnly )
        return 1;
    if( pLightA->CharOnly < pLightB->CharOnly )
        return -1;
    if ( pLightA->Score > pLightB->Score )
        return 1;
    if ( pLightA->Score < pLightB->Score )
        return -1;

    return 0;
}

//=========================================================================

void light_mgr::BeginLightCollection( void )
{
    static const s32 kMaxLightsInSpad = 8192 / sizeof(spad_light);

    ASSERT( !m_bInCollection );
  
    // Allocate space for the spad lights.
    s32 nLights = m_NFadingLights+m_NDynamicLights+m_NCharLights;
    smem_StackPushMarker();
    m_pSpadLights = (spad_light*)smem_StackAlloc( sizeof(spad_light)*MIN(nLights,kMaxLightsInSpad) );
    ASSERT( m_pSpadLights );
    
    // Add the optimized lights to scratchpad
    nLights = 0;

    // add the dynamic lights
    s32 i;
    for ( i = 0; (i<m_NDynamicLights)&&(nLights<kMaxLightsInSpad); i++ )
    {
        dynamic_light& Light = m_DynamicLights[i];
        m_pSpadLights[nLights].Pos       = Light.Pos;
        m_pSpadLights[nLights].Radius    = Light.Radius;
        m_pSpadLights[nLights].Intensity = Light.Intensity;
        m_pSpadLights[nLights].Color     = Light.Color;
        m_pSpadLights[nLights].Score     = x_sqr(Light.Color.R*Light.Intensity) +
                                           x_sqr(Light.Color.G*Light.Intensity) +
                                           x_sqr(Light.Color.B*Light.Intensity);
        m_pSpadLights[nLights].CharOnly  = FALSE;
        nLights++;
    }

    // add the fading lights
    s32 CurrLink = m_FirstLink;
    while ( (CurrLink!=-1) && (nLights<kMaxLightsInSpad) )
    {
        fading_light& Light = m_FadingLights[CurrLink];
        m_pSpadLights[nLights].Pos       = Light.Pos;
        m_pSpadLights[nLights].Radius    = Light.Radius;
        m_pSpadLights[nLights].Intensity = Light.Intensity;
        m_pSpadLights[nLights].Color     = Light.CurrentColor;
        m_pSpadLights[nLights].Score     = x_sqr(Light.CurrentColor.R*Light.Intensity) +
                                           x_sqr(Light.CurrentColor.G*Light.Intensity) +
                                           x_sqr(Light.CurrentColor.B*Light.Intensity);
        m_pSpadLights[nLights].CharOnly  = FALSE;
        nLights++;
        
        CurrLink = Light.NextLink;
    }
    
    // We've collected everything but the character lights now. These are the
    // lights that will be used for all non-skinned geometry.
    m_nNonCharLightsInSpad = nLights;

    // add the character lights
    for ( i = 0; (i<m_NCharLights) && (nLights<kMaxLightsInSpad); i++ )
    {
        dynamic_light& Light = m_CharLights[i];
        m_pSpadLights[nLights].Pos       = Light.Pos;
        m_pSpadLights[nLights].Radius    = Light.Radius;
        m_pSpadLights[nLights].Intensity = Light.Intensity;
        m_pSpadLights[nLights].Color     = Light.Color;
        m_pSpadLights[nLights].Score     = x_sqr(Light.Color.R*Light.Intensity) +
                                           x_sqr(Light.Color.G*Light.Intensity) +
                                           x_sqr(Light.Color.B*Light.Intensity);
        m_pSpadLights[nLights].CharOnly  = TRUE;
        nLights++;
    }

    // sort the lights based on their score...this will mean that lights
    // with a higher intensity and color will get precedence when it comes
    // time to whittle them down to a nice number for the hardware
    x_qsort( m_pSpadLights, nLights, sizeof(spad_light), SpadLightSortFn );

    m_NSpadLights   = nLights;
    m_bInCollection = TRUE;
}

//=========================================================================

void light_mgr::EndLightCollection( void )
{
    ASSERT( m_bInCollection );

    // free up the spad lights
    smem_StackPopToMarker();
    
    m_NSpadLights          = 0;
    m_nNonCharLightsInSpad = 0;
    m_pSpadLights          = NULL;
    m_bInCollection        = FALSE;
}

//=========================================================================

void light_mgr::ResetAfterException( void )
{
    m_bInCollection = FALSE;
}

//=========================================================================

s32 light_mgr::CollectLights( const bbox& WorldBBox, s32 MaxLightCount )
{
    ASSERT( m_bInCollection );
    ASSERT( m_pSpadLights );

/*
    //#### this code is temporarily disabled until the vector3 optimizations are finished
    #ifdef TARGET_PS2
    // load the bbox into vu0 registers
    u128 temp1 = 0;
    u128 temp2 = 0;
    u32  temp3 = (u32)&WorldBBox;
    asm __volatile__
    ("
        #// load min
        mtsab   %2, 0
        lq      %0, 0(%2)
        lq      %1, 16(%2)
        qfsrv   %0, %1, %0
        qmtc2   %0, vf01

        #// load max
        addiu   %1, %2, 12
        mtsab   %1, 0
        lq      %0, 0(%1)
        lq      %1, 16(%1)
        qfsrv   %0, %1, %0
        qmtc2   %0, vf02

    " : "+r" (temp1), "+r" (temp2), "+r" (temp3) );
    #endif
    */

    s32 i;
    m_NCollectedLights = 0;
    for ( i = 0; (i < m_nNonCharLightsInSpad) && (m_NCollectedLights<MaxLightCount); i++ )
    {
        ASSERT( !m_pSpadLights[i].CharOnly );

        xbool bIntersects;

        // intersection test
        //#### this code is temporarily disabled until the vector3 optimizations are finished
        /*
        #ifdef TARGET_PS2
        
        // the math will be something like this (using vectors):
        // temp1 = min-center
        // temp2 = center-max
        // temp1 = MAX(temp1,0)
        // temp2 = MAX(temp2,0)
        // temp1 = temp1.x*temp1.x, temp1.y*temp1.y, temp1.z*temp1.z
        // temp2 = temp2.x*temp2.x, temp2.y*temp1.y, temp2.z*temp2.z
        // add components of temp1 and temp2
        // compare result to r*r
        ASSERT( (((u32)&m_pSpadLights[i].Pos) & 0xf) == 0 );
        f32 ftemp1;
        f32 ftemp2;
        asm
        ("
            lqc2        vf03, 0x00(%3)      # load (posx,posy,posz,radius)
            vsub.xyz    vf04, vf01, vf03    # vf04  = min-center
            vsub.xyz    vf05, vf03, vf02    # vf05  = center-max
            vmax.xyz    vf04, vf04, vf00    # vf04  = MAX(vf04, 0)
            vmax.xyz    vf05, vf05, vf00    # vf05  = MAX(vf05, 0)
            vmul.xyz    vf04, vf04, vf04    # vf04  = (x*x,y*y,z*z)
            vmul.xyz    vf05, vf05, vf05    # vf05  = (x*x,y*y,z*z)
            vadd.xyz    vf06, vf04, vf05    # vf06  = vf04+vf05
            vaddy.x     vf06, vf06, vf06y   # vf06x = vf06x+vf06y
            vaddz.x     vf06, vf06, vf06z   # vf06x = vf06x+vf06z
            vmul.w      vf03, vf03, vf03    # vf03w = r*r
            vaddw.x     vf03, vf00, vf03    # vf03x = vf03w
            qmfc2       %0,   vf06          # temp = d*d
            mtc1        %0,   %1            # ftemp1 = d*d
            qmfc2       %0,   vf03          # temp = r*r
            mtc1        %0,   %2            # ftemp2 = r*r
            .set noreorder
            c.le.s      %1,   %2            # iff(d*d<=r*r)
            nop
            bc1t        wastrue
            addi        %0,   $0,   1       # BDS (true==1)
            addi        %0,   $0,   0       # (false==0)
        wastrue:
            .set reorder

        " : "=r" (bIntersects), "=f" (ftemp1), "=f" (ftemp2) : "r" (&m_pSpadLights[i]) );
        
        ASSERT( bIntersects == WorldBBox.Intersect(m_pSpadLights[i].Pos, m_pSpadLights[i].Radius) );

        #else
        */
        
        bIntersects = WorldBBox.Intersect(m_pSpadLights[i].Pos, m_pSpadLights[i].Radius);
        
        //#endif
        
        if ( bIntersects )
            m_CollectedLights[m_NCollectedLights++] = i;
    }

    return m_NCollectedLights;
}

//=========================================================================

void light_mgr::GetCollectedLight( s32 Index, vector3& Pos, f32& Radius, xcolor& C )
{
    ASSERT( Index < m_NCollectedLights );
    ASSERT( m_bInCollection );
    ASSERT( m_pSpadLights );
    
    spad_light& Light = m_pSpadLights[m_CollectedLights[Index]];
    Pos    = Light.Pos;
    Radius = Light.Radius;
    C.R    = (u8)MIN(255.0f,Light.Color.R*Light.Intensity);
    C.G    = (u8)MIN(255.0f,Light.Color.G*Light.Intensity);
    C.B    = (u8)MIN(255.0f,Light.Color.B*Light.Intensity);
    C.A    = (u8)MIN(255.0f,Light.Color.A*Light.Intensity);
}

//=========================================================================

xbool light_mgr::CalcDirLight( dir_light* pDst,
                              const matrix4& L2W,
                              const bbox& B,
                              const vector3& Pos,
                              f32 Radius,
                              f32 Intensity,
                              xcolor& C )
{
    bbox Box = B;
    Box.Transform(L2W);
    if ( Box.Intersect(Pos, Radius) )
    {
        //
        // convert the point light into a directional light
        //
        
        // to make things easier and avoid having the bounding box grow to an
        // inaccurate shape, move the light into the bbox's local space.
        vector3 Temp = Pos - L2W.GetTranslation();
        vector3 LPos( Temp.GetX()*L2W(0,0) + Temp.GetY()*L2W(0,1) + Temp.GetZ()*L2W(0,2),
                      Temp.GetX()*L2W(1,0) + Temp.GetY()*L2W(1,1) + Temp.GetZ()*L2W(1,2),
                      Temp.GetX()*L2W(2,0) + Temp.GetY()*L2W(2,1) + Temp.GetZ()*L2W(2,2) );

        // find the distance from the light to the bounding box, and use that to adjust
        // the directional intensity
        f32 ftemp;
        f32 dist = 0.0f;
        if ( LPos.GetX() > B.Max.GetX() )
        {
            ftemp  = LPos.GetX() - B.Max.GetX();
            dist += ftemp*ftemp;
        }

        if ( LPos.GetX() < B.Min.GetX() )
        {
            ftemp  = B.Min.GetX() - LPos.GetX();
            dist += ftemp*ftemp;
        }

        if ( LPos.GetY() > B.Max.GetY() )
        {
            ftemp  = LPos.GetY() - B.Max.GetY();
            dist += ftemp*ftemp;
        }

        if ( LPos.GetY() < B.Min.GetY() )
        {
            ftemp  = B.Min.GetY() - LPos.GetY();
            dist += ftemp*ftemp;
        }

        if ( LPos.GetZ() > B.Max.GetZ() )
        {
            ftemp  = LPos.GetZ() - B.Max.GetZ();
            dist += ftemp*ftemp;
        }

        if ( LPos.GetZ() < B.Min.GetZ() )
        {
            ftemp  = B.Min.GetZ() - LPos.GetZ();
            dist += ftemp*ftemp;
        }

        // now we can calculate the lights intensity
        f32 radius_sqr = Radius * Radius;
        
        if ( radius_sqr == 0.0f )
            radius_sqr = 1.0f;
        
        f32 I = 1.0f - dist / radius_sqr;
        I     = MAX( I, 0.0f );
        I    *= Intensity;

        if ( I > 0.0f )
        {
            // and the direction of the light should just be based on the
            // bounding box's center point
            vector3 BoxPos = B.GetCenter() + L2W.GetTranslation();
            vector3 LDir   = BoxPos - Pos;
            if ( !LDir.SafeNormalize() )
            {
                // this can happen if the light happens to be placed at the center of
                // the object (odd case but still possible)
                LDir.Set( 0.7071f, -0.7071f, 0.0f );
            }

            // now we can set up the light
            pDst->Col.R = (u8)(MIN(I*(f32)C.R, 255.0f));
            pDst->Col.G = (u8)(MIN(I*(f32)C.G, 255.0f));
            pDst->Col.B = (u8)(MIN(I*(f32)C.B, 255.0f));
            pDst->Col.A = 0;
            pDst->Dir   = LDir;
            
            return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================

s32 light_mgr::CollectCharLights( const matrix4& L2W, const bbox& B, s32 MaxLightCount )
{
    m_NCollectedLights = 0;

    // walk the list collecting fading lights that may intersect
    s32 CurrLink = m_FirstLink;
    while ( (CurrLink != -1) && (m_NCollectedLights < MAX_COLLECTED_LIGHTS) )
    {
        if ( CalcDirLight( &m_CollectedCharLights[m_NCollectedLights],
                           L2W,
                           B,
                           m_FadingLights[CurrLink].Pos,
                           m_FadingLights[CurrLink].Radius,
                           m_FadingLights[CurrLink].Intensity,
                           m_FadingLights[CurrLink].CurrentColor ) )
        {
            m_NCollectedLights++;
        }
        CurrLink = m_FadingLights[CurrLink].NextLink;
    }

    // now go through all of the normal character lights
    for ( s32 iCharLight = 0;
          (iCharLight < m_NCharLights) && (m_NCollectedLights < MAX_COLLECTED_LIGHTS);
          iCharLight++ )
    {
        if ( CalcDirLight( &m_CollectedCharLights[m_NCollectedLights],
                           L2W,
                           B,
                           m_CharLights[iCharLight].Pos,
                           m_CharLights[iCharLight].Radius,
                           m_CharLights[iCharLight].Intensity,
                           m_CharLights[iCharLight].Color ) )
        {
            m_NCollectedLights++;
        }
    }

    // now go through all of the dynamic lights
    for ( s32 iDynamicLight = 0;
          (iDynamicLight < m_NDynamicLights) && (m_NCollectedLights < MAX_COLLECTED_LIGHTS);
          iDynamicLight++ )
    {
        if ( CalcDirLight( &m_CollectedCharLights[m_NCollectedLights],
                           L2W,
                           B,
                           m_DynamicLights[iDynamicLight].Pos,
                           m_DynamicLights[iDynamicLight].Radius,
                           m_DynamicLights[iDynamicLight].Intensity,
                           m_DynamicLights[iDynamicLight].Color ) )
        {
            m_NCollectedLights++;
        }
    }

    // reduce the lights if necessary
    while ( m_NCollectedLights > MaxLightCount )
    {
        s32 iLightToDrop = -1;
        s32 LowestI = 255*255+255*255+255*255+1;
        
        for ( s32 iLight = 0; iLight < m_NCollectedLights; iLight++ )
        {
            // grab the light intensity
            s32 R = m_CollectedCharLights[iLight].Col.R;
            s32 G = m_CollectedCharLights[iLight].Col.G;
            s32 B = m_CollectedCharLights[iLight].Col.B;
            s32 I = R*R + G*G + B*B;

            if ( (iLightToDrop == -1) || (I < LowestI) )
            {
                iLightToDrop = iLight;
                LowestI      = I;
            }
        }

        // remove a light
        ASSERT( iLightToDrop >= 0 );
        m_NCollectedLights--;
        x_memmove( &m_CollectedCharLights[iLightToDrop],
                   &m_CollectedCharLights[iLightToDrop+1],
                   sizeof(s32)*(m_NCollectedLights-iLightToDrop) );
    }

    return m_NCollectedLights;
}

//=========================================================================

void light_mgr::GetCollectedCharLight( s32 Index, vector3& Dir, xcolor& C )
{
    ASSERT( (Index >= 0) && (Index < m_NCollectedLights) );
    Dir = m_CollectedCharLights[Index].Dir;
    C   = m_CollectedCharLights[Index].Col;
}

//=========================================================================

void light_mgr::GetLight( s32 Index, vector3& Pos, f32& Radius, xcolor& C ) const
{
    ASSERT( m_bInCollection );
    ASSERT( (Index >= 0) && (Index < m_NSpadLights) );

    spad_light& Light = m_pSpadLights[Index];
    Pos    = Light.Pos;
    Radius = Light.Radius;
    C.R    = (u8)MIN(255.0f,Light.Color.R*Light.Intensity);
    C.G    = (u8)MIN(255.0f,Light.Color.G*Light.Intensity);
    C.B    = (u8)MIN(255.0f,Light.Color.B*Light.Intensity);
    C.A    = (u8)MIN(255.0f,Light.Color.A*Light.Intensity);
}

//=========================================================================
