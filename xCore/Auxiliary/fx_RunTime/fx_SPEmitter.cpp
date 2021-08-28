//==============================================================================
//
//  fx_SPEmitter.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "fx_SPEmitter.hpp"
#include "x_context.hpp"
#include "e_Draw.hpp"
#include "e_Engine.hpp"
#include "..\Support\Render\Render.hpp"

#ifdef TARGET_GCN
#include "dolphin/gx.h"
#endif

//==============================================================================
//  DAMN LINKER!
//==============================================================================

s32 fx_SPEmitter;

//==============================================================================
//  FUNCTIONS
//==============================================================================

s32 SPEmitterMemoryFn( const fx_element_def& ElementDef )
{
    fx_edef_spemitter& EmitterDef = (fx_edef_spemitter&)ElementDef;

    s32 nParticles = EmitterDef.NParticles;

    u32 Size = sizeof( fx_spemitter );

    Size += sizeof(fx_sparticle) * nParticles;
    Size += sizeof(vector4     ) * nParticles;
    Size += sizeof(vector4     ) * nParticles;

    Size = ALIGN_16( Size );
    Size += (EmitterDef.Flags & SPE_REVERSE_MODE) ? sizeof(vector3     ) * nParticles : 0;

    Size = ALIGN_16( Size );
    Size += sizeof(vector3) * nParticles;

    Size = ALIGN_16( Size );
    Size += sizeof(u32) * nParticles;

    Size = ALIGN_16( Size );
    Size += (EmitterDef.Flags & SPE_VELOCITY_ORIENTED) ? 0 : sizeof(vector2) * nParticles;

#ifdef TARGET_PS2
    return ALIGN_16( Size );
#else
    // The extra 15 here is because the allocator on the PC is not 16 byte aligned but the preceeding
    // code assumes 16 byte alignment, the 15 gives room for any error.
    return ALIGN_16( Size + 15 );
#endif
}

//==============================================================================

void fx_spemitter::Initialize( const fx_element_def* pElementDef, 
                                     f32*            pInput )
{
    // Let the base class take care of the basics.
    fx_element::Initialize( pElementDef, pInput );

    //
    // Now, on to the sprite particle emitter specific details.
    //

    s32                i;
    fx_edef_spemitter& EmitterDef = (fx_edef_spemitter&)(*pElementDef);

    //
    // TO DO - Do a better job with this stuff in here.
    //

    if( EmitterDef.Flags & SPE_BURST_MODE )
        m_EmitGap = (EmitterDef.TimeStop - EmitterDef.TimeStart) / EmitterDef.NParticles;
    else
        m_EmitGap = EmitterDef.LifeSpan / EmitterDef.NParticles;

    m_NActive      = 0;
    m_PCursor      = 0;
    m_EmitClock    = 0;
    m_EmitCycle    = EmitterDef.LifeSpan;
    m_Emitting     = TRUE;
    m_PrevL2WReady = FALSE;
    m_Color.Set( 255, 255, 255, (u8)0 );

    s32 nParticles = EmitterDef.NParticles;
    u32 t = (u32)(this+1);
    m_pParticles    = (fx_sparticle*)t; t += sizeof(fx_sparticle) * nParticles;
    m_pPositions    = (vector4*     )t; t += sizeof(vector4     ) * nParticles;
    m_pVelocities   = (vector4*     )t; t += sizeof(vector4     ) * nParticles;
    
    t = ALIGN_16( t );
    m_pStartPos     = (EmitterDef.Flags & SPE_REVERSE_MODE) ? (vector3*)t : (vector3*)0;
    t += (EmitterDef.Flags & SPE_REVERSE_MODE) ? sizeof(vector3) * nParticles : 0;

    t = ALIGN_16( t );
    m_pStartVel     = (vector3*)t;
    t += sizeof(vector3) * nParticles;

    t = ALIGN_16( t );
    m_pColors       = (u32*)t;
    t += sizeof(u32) * nParticles;

    t = ALIGN_16( t );
    m_pRotAndScales = (EmitterDef.Flags & SPE_VELOCITY_ORIENTED) ? (vector2*)0 : (vector2*)t;
    t += (EmitterDef.Flags & SPE_VELOCITY_ORIENTED) ? 0 : sizeof(vector2) * nParticles;

    vector4     InactivePos( 0.0f, 0.0f, 0.0f, 0.0f );
    InactivePos.GetIW() = 0x8000;

    for( i = 0; i < nParticles; i++ )
    {
        fx_sparticle& P = m_pParticles[i];
        m_pPositions[i] = InactivePos;
        P.EmitTime      = i * m_EmitGap;
//      P.AgeRate       = 1.0f / EmitterDef.LifeSpan;
        P.StartSpin     = (EmitterDef.MinSpinRate == EmitterDef.MaxSpinRate) 
                          ? EmitterDef.MinSpinRate
                          : x_frand( R_0, R_360 );
    }
}

//==============================================================================

void fx_spemitter::AdvanceLogic( const fx_effect_base* pEffect, f32 DeltaTime )
{
    EmissionLogic( pEffect, DeltaTime );
    ParticleLogic( pEffect, DeltaTime );
}

//==============================================================================

static s32 s_Seed = 15827;

inline f32 fast_rand( f32 Min, f32 Max )
{
    s_Seed = s_Seed * 214013 + 2531011;
    f32 r = (f32)((s_Seed >> 16) & X_RAND_MAX);
    return( ((r / (f32)X_RAND_MAX) * (Max-Min)) + Min );
}

void fx_spemitter::EmissionLogic( const fx_effect_base* pEffect, f32 DeltaTime )
{
    CONTEXT( "fx_spemitter::EmissionLogic" );

    const matrix4&     L2W = GetL2W( pEffect );
    matrix4            StartVelL2W;    
    fx_edef_spemitter& EmitterDef = (fx_edef_spemitter&)(*m_pElementDef);

    if( m_Emitting && !pEffect->IsSuspended() )
    {
        // Time to stop m_Emitting?
        if( pEffect->GetAge() >= EmitterDef.TimeStop )
            m_Emitting = FALSE;

        if( EmitterDef.Flags & SPE_WORLD_SPACE )
        {
            StartVelL2W = L2W;
            StartVelL2W.ClearTranslation();
        }

        // Read some values for the loop
        const vector3p& VMin            = EmitterDef.MinVelocity;
        const vector3p& VMax            = EmitterDef.MaxVelocity;
        s32             nActive         = m_NActive;
        s32             iCursor         = m_PCursor;
        const u32       EmitterDefFlags = EmitterDef.Flags;
        vector3         p;
        vector3         v;

        // Kill old particles and emit new ones.
        m_EmitClock += DeltaTime;
        while( m_Emitting && (m_pParticles[iCursor].EmitTime < m_EmitClock) )
        {
            fx_sparticle& P    = m_pParticles[iCursor];

            // Due to floating point error, the particles will sometimes have
            // a sliver of life left in them and thus still active.  Time for
            // an early retirement!
            if( m_pPositions[iCursor].GetIW() != 0x8000 )
            {
                nActive -= 1;
            }

            // Emit this particle.
            m_pPositions[iCursor].GetIW() = 0x0000;
            P.SpinRate = fast_rand( EmitterDef.MinSpinRate, EmitterDef.MaxSpinRate );
            P.Age      = m_EmitClock - m_pParticles[iCursor].EmitTime;

            if( EmitterDefFlags & SPE_EMIT_FROM_VOLUME )
            {
                f32 x = fast_rand( -0.5f, 0.5f );
                f32 y = fast_rand( -0.5f, 0.5f );
                f32 z = fast_rand( -0.5f, 0.5f );
                p.Set( x, y, z );
            }
            else
            {
                p.Zero();
            }

            v.Set( fast_rand( VMin.X, VMax.X ),
                   fast_rand( VMin.Y, VMax.Y ),
                   fast_rand( VMin.Z, VMax.Z ) );

            if( EmitterDefFlags & SPE_WORLD_SPACE )
            {
                v = StartVelL2W * v;

                if( (m_PrevL2WReady) &&
                    (DeltaTime > 0.0f) && 
                    (DeltaTime > (m_EmitGap * 1.25f)) )
                {
                    // It is very likely that more than one particle will be 
                    // emitted during this logic.  To prevent "clumping", we
                    // must interpolate the point from which the particles 
                    // emit.  We have the previous L2W and the current L2W.

                    f32     t  = P.Age / DeltaTime;
                    vector3 P1 = m_PreviousL2W * p;
                    vector3 P2 =           L2W * p;
                    p = (P1 * t) + (P2 * (1.0f - t));
                }
                else
                {
                    p = L2W * p;
                }
            }

            if( EmitterDefFlags & SPE_REVERSE_MODE )
            {
                m_pStartPos[iCursor] = p;
            }

            m_pStartVel[iCursor] = v;
            m_pPositions [iCursor] = p;
            m_pVelocities[iCursor] = v;

            nActive += 1;
            iCursor += 1;

            if( iCursor >= EmitterDef.NParticles )
            {
                iCursor    = 0;
                m_EmitClock -= m_EmitCycle;

                if( EmitterDefFlags & SPE_BURST_MODE )
                {
                    m_Emitting = FALSE;
                }
            }
        }

        // Write back out the cache variables
        m_NActive = nActive;
        m_PCursor = iCursor;

        if( EmitterDefFlags & SPE_WORLD_SPACE )
        {
            m_PrevL2WReady = TRUE;
            m_PreviousL2W  = L2W;
        }
    }
}

//==============================================================================

#ifdef TARGET_PS2

inline
static xcolor LerpColor( const xcolor& Color1, const xcolor& Color2, f32 Factor, const xcolor& Color3 )
{
    ASSERT( (Factor >= 0.0f) && (Factor <= 1.0f) );

    register s32    factor = (s32)(Factor * 32767);
    register u32    color1 = *(u32*)&Color1;
    register u32    color2 = *(u32*)&Color2;
    register u32    color3 = *(u32*)&Color3;
    register u32    tmp1;
    register u32    tmp2;

    asm
    ("
        li          tmp1, 0x7fff
        pcpyh       tmp1, tmp1              # Duplicate the 7fff value over the lower 4 halfwords
        pcpyh       tmp2, factor            # Duplicate the factor over the lower 4 halfwords
        psubuh      tmp1, tmp1, tmp2        # 7fff - factor in all 4 lower halfwords
        pextlh      tmp2, tmp1, tmp2        # Interleave tmp and factor components

        pextlb      color1, $0, color1      # Extend c1 to halfwords
        pextlb      color2, $0, color2      # Extend c2 to halfwords
        pextlh      tmp1, color1, color2    # Interleave c1 and c2 components

        phmadh      tmp1, tmp1, tmp2        # Multiply and Add for the lerp
        psraw       tmp1, tmp1, 15          # Normalize

        pextlb      tmp2, $0, color3        # Extend c3 to halfwords
        pextlh      tmp2, $0, tmp2          # Extend c3 to words

        phmadh      tmp1, tmp1, tmp2        # Multiply (c1 lerp c2) * c3
        psraw       tmp1, tmp1, 8           # Normalize
        ppach       tmp1, $0, tmp1          # Pack result
        ppacb       tmp1, $0, tmp1          # Pack result
    "
    :
      "=&r tmp1"(tmp1),
      "=&r tmp2"(tmp2)
    :
      "r color1"(color1),
      "r color2"(color2),
      "r color3"(color3),
      "r factor"(factor)
    );

/*
        pextlb      color3, $0, color3      # Extend c3 to halfwords

        pmfhl.sh    tmp
        pmulth      tmp, tmp, color3        # * Color 3
        pmfhl.sh    tmp
        ppacb       tmp, $0, tmp
*/

    return (xcolor)tmp1;
}

#endif

//==============================================================================

void fx_spemitter::ParticleLogic( const fx_effect_base* pEffect, f32 DeltaTime )
{
    CONTEXT( "fx_spemitter::ParticleLogic" );

    s32                 i;
    f32                 MaxRadius;
    s32                 BoxSkip    = 0;
    fx_edef_spemitter&  EmitterDef = (fx_edef_spemitter&)(*m_pElementDef);
    vector3             Gravity3( 0.0f, EmitterDef.Gravity, 0.0f );

    m_BBox.Clear();

    MaxRadius = 0.0f;

    if( m_NActive == 0 )
    {
        m_BBox = pEffect->GetL2W() * m_Translate;
        return;
    }

    f32     UniScale  = pEffect->GetUniformScale();
#ifndef TARGET_PS2
    vector4 ElementColor( m_Color.R / 255.0f,
                          m_Color.G / 255.0f,
                          m_Color.B / 255.0f,
                          m_Color.A / 255.0f );
#endif

    if( EmitterDef.Flags & SPE_SCALE_SPRITE_SIZE )
        UniScale *= GetUniformScale();

    // Get Max frames from the EmitterDef such that 0 <= Max < EmitterDef.NKeyFrames-1
    f32 Max = EmitterDef.NKeyFrames - 1.00001f;
    if( Max < 0.0f )
        Max = 0.0f;

    // Get render buffer pointers
    vector4*    pPosition       = m_pPositions;
    u32*        pColor          = m_pColors;
    vector4*    pVelocity       = m_pVelocities;
    vector2*    pRotAndScale    = m_pRotAndScales;

//    vector4     Gravity4( 0.0f, EmitterDef.Gravity, 0.0f, 0.0f );
    vector4     InactivePos( 0.0f, 0.0f, 0.0f, 0.0f );
    InactivePos.GetIW() = 0x8000;

    // Advance the logic on all of the particles.
    for( i = 0; i < EmitterDef.NParticles; i++ )
    {
        fx_sparticle& P = m_pParticles[i];
        P.Age += DeltaTime;     // P.Age += P.AgeRate * DeltaTime;

        if( m_pPositions[i].GetIW() != 0x8000 )
        {
            if( P.Age >= EmitterDef.LifeSpan )
            {
                m_NActive -= 1;

                *pPosition++ = InactivePos;
                pColor++;
                pVelocity++;
                pRotAndScale++;
            }
            else
            {
//                f32 Age       = (EmitterDef.Flags & SPE_REVERSE_MODE) ? 
//                                (EmitterDef.LifeSpan - P.Age) :
//                                (P.Age);
//                f32 Factor    = Age * Age * 0.5f;

                // Figure out which keyframes to use and a blend factor.
                // TO DO - Rig the parametric age to be the frames.
                f32 Frame  = (P.Age / EmitterDef.LifeSpan) * Max;
                s32 Lo     = (s32)Frame;
                s32 Hi     = Lo + 1;
                f32 LoMix  = Hi - Frame;
                f32 HiMix  = Frame - Lo;

                if( Max == 0 )
                {
                    Hi    = 0;
                    LoMix = 1.0f;
                    HiMix = 0.0f;
                }

                // Calculate Color
                xcolor Color;
#ifndef TARGET_PS2
                Color.R = (u8)(((EmitterDef.Key[Lo].Color.R * LoMix)  + 
                                (EmitterDef.Key[Hi].Color.R * HiMix)) *
                                (ElementColor.GetX()));

                Color.G = (u8)(((EmitterDef.Key[Lo].Color.G * LoMix)  + 
                                (EmitterDef.Key[Hi].Color.G * HiMix)) *
                                (ElementColor.GetY()));

                Color.B = (u8)(((EmitterDef.Key[Lo].Color.B * LoMix)  + 
                                (EmitterDef.Key[Hi].Color.B * HiMix)) *
                                (ElementColor.GetZ()));

                Color.A = (u8)(((EmitterDef.Key[Lo].Color.A * LoMix)  + 
                                (EmitterDef.Key[Hi].Color.A * HiMix)) *
                                (ElementColor.GetW()));
#else
                Color = LerpColor( EmitterDef.Key[Lo].Color, EmitterDef.Key[Hi].Color, Frame-Lo, m_Color );
#endif

#ifdef TARGET_PS2
                // TODO: Remove this code
                Color.R = (Color.R==255) ? 0x80 : (Color.R>>1);
                Color.G = (Color.G==255) ? 0x80 : (Color.G>>1);
                Color.B = (Color.B==255) ? 0x80 : (Color.B>>1);
                Color.A = (Color.A==255) ? 0x80 : (Color.A>>1);
#endif
                *pColor++ = (Color.A << 24) | (Color.B << 16) | (Color.G << 8) | Color.R;

                // Calculate Scale
                f32 Scale = ((EmitterDef.Key[Lo].Scale * LoMix) + (EmitterDef.Key[Hi].Scale * HiMix));

                vector4 v;
                vector3 p;

                if( EmitterDef.Flags & SPE_REVERSE_MODE )
                {

                    f32 Age    = (EmitterDef.LifeSpan - P.Age);
                    f32 Factor = Age * Age * 0.5f;

                    vector3 sp = m_pStartPos[i];
                    vector3 sv = m_pStartVel[i];

                    // Calc position
                    p = sp + sv * ((EmitterDef.Acceleration * Factor) + Age) - Gravity3 * Factor;

                    // Calc velocity
                    v = sv + sv * (EmitterDef.Acceleration * P.Age) - Gravity3 * P.Age;
                    v.GetW() = Scale;

                    *pPosition = p;
                    *pVelocity = v;
                }
                else
                {
                    // Update position
                    p = *(vector3*)pPosition;
                    p += *(vector3*)pVelocity * DeltaTime;
                    *(vector3*)pPosition = p;
                    ((s32*)pPosition)[3] = 0x0000;

                    // Update velocity
                    v = *pVelocity;
                    v += m_pStartVel[i] * (EmitterDef.Acceleration * DeltaTime);
                    v.GetY() -= EmitterDef.Gravity * DeltaTime;
                    v.GetW() = Scale;
                    *pVelocity = v;
                }

                pPosition++;
                pVelocity++;

                // Velocity & Scale or Rotation & Scale
                if( !(EmitterDef.Flags & SPE_VELOCITY_ORIENTED) )
                {
                    radian Rotation = P.StartSpin + P.SpinRate * P.Age;
                    Rotation = x_ModAngle2( Rotation ); // TODO: Optimize this?
                    pRotAndScale->Set( Rotation, Scale );
                    pRotAndScale++;
                }

                if( BoxSkip-- <= 0 )
                {
                    m_BBox    += p;
                    MaxRadius  = MAX( MaxRadius, Scale * UniScale * 0.75f );
                    BoxSkip    = m_NActive >> 2;
                }
            }
        }
        else
        {
            pPosition++;
            pColor++;
            pVelocity++;
            pRotAndScale++;
        }
    }

    if( m_NActive == 0 )
    {
        m_BBox = pEffect->GetL2W() * m_Translate;
        return;
    }

    if( !(EmitterDef.Flags & SPE_WORLD_SPACE) )
    {
        m_BBox.Transform( GetL2W( pEffect ) );
    }
    m_BBox.Inflate( MaxRadius, MaxRadius, MaxRadius );
}

//==============================================================================
#ifdef TARGET_GCN
//==============================================================================

void SPEmitterGCNSetup( const xbitmap* pAlpha, xbool SubMode )
{
    if( pAlpha == NULL )
        return;

    if( SubMode )
    {   
        GXSetTevOrder   ( GX_TEVSTAGE0, GCNTEXCoord[ 0 ], GX_TEXMAP1, GX_COLOR0A0 );   
        GXSetTevOrder   ( GX_TEVSTAGE1, GCNTEXCoord[ 0 ], GX_TEXMAP0, GX_COLOR0A0 );   

        GXSetTevSwapMode( GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP1 );
        GXSetTevSwapMode( GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0 );

        GXSetTevColorOp( GX_TEVSTAGE0,
                         GX_TEV_ADD,           
                         GX_TB_ZERO,           
                         GX_CS_SCALE_1,        
                         TRUE,                 
                         GX_TEVPREV );         

        GXSetTevAlphaOp( GX_TEVSTAGE0,
                         GX_TEV_ADD,           
                         GX_TB_ZERO,           
                         GX_CS_SCALE_1,        
                         TRUE,                 
                         GX_TEVPREV );        

        GXSetTevColorOp( GX_TEVSTAGE1,
                         GX_TEV_ADD,           
                         GX_TB_ZERO,           
                         GX_CS_SCALE_1,        
                         TRUE,                 
                         GX_TEVPREV );         

        GXSetTevAlphaOp( GX_TEVSTAGE1,
                         GX_TEV_ADD,           
                         GX_TB_ZERO,           
                         GX_CS_SCALE_1,        
                         TRUE,                 
                         GX_TEVPREV );        

        //==---------------------
        //  Stage 0 override
        //==---------------------
        GXSetTevColorIn( GX_TEVSTAGE0,
                         GX_CC_ZERO,
                         GX_CC_TEXA,
                         GX_CC_RASA,
                         GX_CC_ZERO );
        GXSetTevAlphaIn( GX_TEVSTAGE0,
                         GX_CA_ZERO,
                         GX_CA_ZERO,
                         GX_CA_ZERO,
                         GX_CA_ONE );       // We can actually ignore alpha
                                            // if we are doing a subtractive blend
                                            // since gcn hardware doesn't use it


        //==---------------------
        //  Stage 1
        //==---------------------
        GXSetTevColorIn( GX_TEVSTAGE1,
                         GX_CC_ZERO,
                         GX_CC_CPREV,
                         GX_CC_TEXC,
                         GX_CC_ZERO );
        GXSetTevAlphaIn( GX_TEVSTAGE1,
                         GX_CA_ZERO,
                         GX_CA_ZERO,
                         GX_CA_ZERO,
                         GX_CA_ONE );           
    }
    else
    {    
        // Activate alpha texture on stage 1.
        vram_Activate( *pAlpha, 1 );

        //==-----------------------------------------
        //  Setup TEV 1
        //  
        //  Use swap1 to get A from R.    
        //==-----------------------------------------   
        GXSetTevOrder   ( GX_TEVSTAGE1, GCNTEXCoord[ 0 ], GX_TEXMAP1, GX_COLOR0A0 );   
        GXSetTevSwapMode( GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1 );

        GXSetTevColorOp( GX_TEVSTAGE1,
                         GX_TEV_ADD,           
                         GX_TB_ZERO,           
                         GX_CS_SCALE_1,        
                         TRUE,                 
                         GX_TEVPREV );         

        GXSetTevAlphaOp( GX_TEVSTAGE1,
                         GX_TEV_ADD,           
                         GX_TB_ZERO,           
                         GX_CS_SCALE_1,        
                         TRUE,                 
                         GX_TEVPREV );        

        //==---------------------
        //  Color stage
        //==---------------------
        // Pass-thru
        GXSetTevColorIn( GX_TEVSTAGE1,
                         GX_CC_ZERO,
                         GX_CC_ZERO,
                         GX_CC_ZERO,
                         GX_CC_CPREV );        

        //==---------------------
        //  Alpha stage
        //==---------------------
        GXSetTevAlphaIn( GX_TEVSTAGE1,
                         GX_CA_ZERO,
                         GX_CA_TEXA,
                         GX_CA_RASA,
                         GX_CA_ZERO );        
    }

    GXSetNumTevStages(2);

    GXSetAlphaCompare( GX_ALWAYS,
                       0,
                       GX_AOP_AND,
                       GX_ALWAYS, 
                       0 );

    GXSetZCompLoc( FALSE ); 
}

//==============================================================================
#endif // TARGET_GCN
//==============================================================================

void fx_spemitter::Render( const fx_effect_base* pEffect ) const
{
    CONTEXT( "fx_spemitter::Render" );

    fx_edef_spemitter& EmitterDef = (fx_edef_spemitter&)(*m_pElementDef);

    if( m_NActive == 0 )
        return;

    if( (m_NActive + fx_mgr::GetSpritesThisFrame()) > fx_mgr::GetSpriteBudget() )
    {
        return;
    }
    else
    {
        fx_mgr::AddSpritesThisFrame( m_NActive );
    }

    f32 UniScale = pEffect->GetUniformScale();

    const xbitmap* pDiffuse = NULL;
    const xbitmap* pAlpha   = NULL;

    pEffect->GetBitmaps( EmitterDef.BitmapIndex, pDiffuse, pAlpha );

    if( EmitterDef.Flags & SPE_SCALE_SPRITE_SIZE )
        UniScale *= GetUniformScale();

    // render it using the render system rather than entropy's draw
    s32 BlendMode = render::BLEND_MODE_NORMAL;
    switch( EmitterDef.CombineMode )
    {
    default:    ASSERT( FALSE ); // Fall through.
    case -1:    BlendMode = render::BLEND_MODE_SUBTRACTIVE;     break;
    case  0:    BlendMode = render::BLEND_MODE_NORMAL;          break;
    case  1:    BlendMode = render::BLEND_MODE_ADDITIVE;        break;
    case  9:    BlendMode = render::BLEND_MODE_SUBTRACTIVE;     break;
    case 10:    BlendMode = render::BLEND_MODE_NORMAL;          break;
    case 11:    BlendMode = render::BLEND_MODE_ADDITIVE;        break;
    case 19:    BlendMode = render::BLEND_MODE_SUBTRACTIVE;     break;
    case 20:    BlendMode = render::BLEND_MODE_NORMAL;          break;
    case 21:    BlendMode = render::BLEND_MODE_ADDITIVE;        break;
    }

    if( IN_RANGE( -1, EmitterDef.CombineMode, 1 ) )
    {
        render::SetDiffuseMaterial( *pDiffuse, BlendMode, EmitterDef.ReadZ );
    }

    if( IN_RANGE( 9, EmitterDef.CombineMode, 11 ) )
    {
        render::SetGlowMaterial( *pDiffuse, BlendMode, EmitterDef.ReadZ );
    }

    if( IN_RANGE( 19, EmitterDef.CombineMode, 21 ) )
    {
        render::SetDiffuseMaterial( *pDiffuse, BlendMode, EmitterDef.ReadZ );
    //  render::SetDistortionMaterial( BlendMode, EmitterDef.ReadZ );
    }

    if( EmitterDef.Flags & SPE_VELOCITY_ORIENTED )
    {
        matrix4* pL2W  = NULL;
        matrix4* pVL2W = (matrix4*)smem_BufferAlloc( sizeof(matrix4) );
        pVL2W->Identity();
        if( (EmitterDef.Flags & SPE_WORLD_SPACE) == 0 )
        {
            pL2W  = (matrix4*)smem_BufferAlloc( sizeof(matrix4) );
            *pL2W = GetL2W( pEffect );

            // TODO: I think this is wrong, the velocities should really just be 
            //       rotated by the L2W matrix and not translated.
            pVL2W->Setup( m_Scale, m_Rotate, vector3(0,0,0) );
        }

        render::RenderVelocitySprites( EmitterDef.NParticles, 
                                       UniScale * 0.5f, 
                                       pL2W, 
                                       pVL2W, 
                                       m_pPositions, 
                                       m_pVelocities, 
                                       m_pColors );
    }
    else
    {
        matrix4* pL2W = NULL;
        if( (EmitterDef.Flags & SPE_WORLD_SPACE) == 0 )
        {
            pL2W  = (matrix4*)smem_BufferAlloc( sizeof(matrix4) );
            *pL2W = GetL2W( pEffect );
        }

        render::Render3dSprites( EmitterDef.NParticles, 
                                 UniScale * 0.5f, 
                                 pL2W, 
                                 m_pPositions, 
                                 m_pRotAndScales, 
                                 m_pColors );
    }

    // *************** //
    // DEBUG RENDERING //
    // *************** //

#ifdef DEBUG_FX
    s32 i;
//    fx_sparticle* pParticle = (fx_sparticle*)(this+1);

    if( !FXDebug.ElementReserved )
        return;

    if( FXDebug.ElementSpriteCenter )
    {
        if( EmitterDef.Flags & SPE_WORLD_SPACE )
        {
            // WORLD MOVEMENT //

            for( i = 0; i < EmitterDef.NParticles; i++ )
            {
//                const fx_sparticle& P = pParticle[i];
                if( m_pPositions[i].GetIW() != 0x8000 )
                {
                    draw_Point( *(vector3*)&m_pPositions[i], XCOLOR_WHITE );
                }
            }
        }
        else
        {
            // LOCAL MOVEMENT //

            const matrix4& L2W = GetL2W( pEffect );

            for( i = 0; i < EmitterDef.NParticles; i++ )
            {
//                const fx_sparticle& P = pParticle[i];
                if( m_pPositions[i].GetIW() != 0x8000 )
                {
                    draw_Point( L2W * *(vector3*)&m_pPositions[i], XCOLOR_WHITE );
                }
            }
        }        
    }

    if( FXDebug.ElementWire )
    {
        if( EmitterDef.Flags & SPE_VELOCITY_ORIENTED )
        {
            // *** VELOCITY ORIENTED *** //
            draw_Begin( DRAW_QUADS, DRAW_NO_ZWRITE | DRAW_CULL_NONE | DRAW_WIRE_FRAME | DRAW_CULL_NONE );
            draw_ClearL2W();

            vector3 LineOfSight = eng_GetView()->GetViewZ();
            vector3 Velocity;
            vector3 Right;
            vector3 Up;
            vector3 Fore;
            vector3 Aft;

            const matrix4& PL2W = GetL2W( pEffect );
                  matrix4  VL2W;

            if( !(EmitterDef.Flags & SPE_WORLD_SPACE) )
            {
                VL2W.Setup( m_Scale, m_Rotate, vector3(0,0,0) );
                VL2W = pEffect->GetL2W() * VL2W;
            }

            for( i = 0; i < EmitterDef.NParticles; i++ )
            {
//                const fx_sparticle& P = pParticle[i];

                if( m_pPositions[i].GetIW() != 0x8000 )
                {
                    f32 Scale  = m_pVelocities[i].GetW() * UniScale;
//                    f32 DeltaV = EmitterDef.Acceleration * P.Age;

                    Velocity         = *(vector3*)&m_pVelocities[i];

                    if( !(EmitterDef.Flags & SPE_WORLD_SPACE) )
                        Velocity = VL2W * Velocity;

                    Velocity.Normalize();

                    Right  = Velocity;
                    Up     = LineOfSight.Cross( Right );
                    Right *= Scale;
                    Up    *= Scale;

                    if( EmitterDef.Flags & SPE_WORLD_SPACE )
                    {
                        Fore   = *(vector3*)&m_pPositions[i] + Right;
                        Aft    = *(vector3*)&m_pPositions[i] - Right;
                    }
                    else
                    {
                        Fore   = PL2W * *(vector3*)&m_pPositions[i]; 
                        Aft    = Fore;              
                        Fore  += Right;             
                        Aft   -= Right;             
                    }

                    draw_Vertex( Fore - Up );
                    draw_Vertex( Aft  - Up );
                    draw_Vertex( Aft  + Up );
                    draw_Vertex( Fore + Up ); 
                }
            }
            draw_End();
        }
        else
        {
            vector2 WH;
            vector2 UV0( 0, 0 );
            vector2 UV1( 1, 1 );

            draw_Begin( DRAW_SPRITES, DRAW_WIRE_FRAME | DRAW_CULL_NONE );

            if( EmitterDef.Flags & SPE_WORLD_SPACE )
            {
                // *** SPIN ORIENTED / WORLD MOVEMENT *** //

                for( i = 0; i < EmitterDef.NParticles; i++ )
                {
//                    const fx_sparticle& P = pParticle[i];
                    if( m_pPositions[i].GetIW() != 0x8000 )
                    {
                        f32 S = m_pRotAndScales[i].Y * UniScale;
                        WH( S, S );
                        draw_SpriteUV( *(vector3*)&m_pPositions[i], WH, UV0, UV1, XCOLOR_BLUE, m_pRotAndScales[i].X );
                    }
                }
            }
            else
            {
                // *** SPIN ORIENTED / LOCAL MOVEMENT *** //

                const matrix4& L2W = GetL2W( pEffect );

                for( i = 0; i < EmitterDef.NParticles; i++ )
                {
//                    const fx_sparticle& P = pParticle[i];
                    if( m_pPositions[i].GetIW() != 0x8000 )
                    {
                        f32 S = m_pRotAndScales[i].Y * UniScale;
                        WH( S, S );
                        draw_SpriteUV( L2W * *(vector3*)&m_pPositions[i], WH, UV0, UV1, XCOLOR_BLUE, m_pRotAndScales[i].X );
                    }
                }
            } 
            
            draw_End();
        }
    }

    if( FXDebug.ElementSpriteCount )
    {
        draw_Label( pEffect->GetL2W() * m_Translate, XCOLOR_YELLOW, "%d", m_NActive );
    }

    fx_element::Render( pEffect );
#endif // DEBUG_FX
}

//==============================================================================

xbool fx_spemitter::IsFinished( const fx_effect_base* pEffect ) const
{
    return( (m_NActive == 0) && fx_element::IsFinished( pEffect ) );
}

//==============================================================================

void fx_spemitter::Reset( void )
{
    s32                i;
    fx_edef_spemitter& EmitterDef = (fx_edef_spemitter&)(*m_pElementDef);

    fx_element::Reset();

    //
    // TO DO - Do a better job with this stuff in here.
    //

    m_NActive       = 0;
    m_PCursor       = 0;
    m_EmitClock     = 0;
    m_EmitCycle     = EmitterDef.LifeSpan;
    m_Emitting      = TRUE;
    m_PrevL2WReady  = FALSE;

//    fx_sparticle* pParticle = (fx_sparticle*)(this + 1);

    for( i = 0; i < EmitterDef.NParticles; i++ )
    {
//        fx_sparticle& P = pParticle[i];
        m_pPositions[i].GetIW() = 0x8000;
    }
}

//==============================================================================

#undef new
REGISTER_FX_ELEMENT_CLASS( fx_spemitter, "SPEMITTER", SPEmitterMemoryFn );

//==============================================================================
