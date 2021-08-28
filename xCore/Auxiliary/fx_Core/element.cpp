//============================================================================
// INCLUDES
//============================================================================

#include "controller.hpp"
#include "element.hpp"
#include "element_sprite.hpp"
#include "element_spemitter.hpp"
#include "element_mesh.hpp"
#include "effect.hpp"

namespace fx_core
{

#define BASE_VER    2

//============================================================================
// The registration list for all element types
static elem_reg* s_RegistrationList[256] = { 0 };
static s32       s_RegCount = 0;
static u32       s_SerialNum = 0;


//============================================================================
// Define the elem_reg constructor
elem_reg::elem_reg( const char* pTypeName, elem_factory_fn FactoryFn )
{
    m_pTypeName = pTypeName;
    m_pFactoryFn = FactoryFn;

    s_RegistrationList[s_RegCount++] = this;
}

//============================================================================
// Create a element by type
element* MakeElement( const char* pType )
{
    for ( s32 i = 0; i < s_RegCount; i++ )
    {
        if ( x_stricmp(s_RegistrationList[i]->m_pTypeName, pType) == 0 )
        {
            return (element*)(*s_RegistrationList[i]->m_pFactoryFn)();
        }
    }

    return NULL;
}


//============================================================================
// DATA
//============================================================================

//============================================================================
// element
//============================================================================

element::element()
{
    m_IsSelected        = FALSE;
    m_ShowTrajectory    = false;

    m_pEffect           = NULL;
    m_Export            = true;

    m_IsImmortal        = true;
    m_LifeStartFrame    = 0;
    m_LifeStopFrame     = 60;

    m_ZRead             = TRUE;

    m_Hide              = FALSE;
}

//============================================================================

element::~element()
{
    // controllers are deleted by the effect
}

//============================================================================

void element::MakeDuplicateName( const char* pOriginalName )
{
    // Extract the base name...which is the name without numbers at the end
    char    BaseName[64];
    s32     BaseLength  = 0;
    s32     LastChar    = x_strlen( pOriginalName ) - 1;

    for( s32 i = LastChar; i > -1; i-- )
    {
        if( pOriginalName[i] != '0' &&
            pOriginalName[i] != '1' &&
            pOriginalName[i] != '2' &&
            pOriginalName[i] != '3' &&
            pOriginalName[i] != '4' &&
            pOriginalName[i] != '5' &&
            pOriginalName[i] != '6' &&
            pOriginalName[i] != '7' &&
            pOriginalName[i] != '8' &&
            pOriginalName[i] != '9' )
        {
            BaseLength = i + 1;
            break;
        }
    }

    x_strncpy( BaseName, pOriginalName, BaseLength );
    BaseName[BaseLength] = '\0';

    // Append a unique number onto the base name
    s32     suffix = 0;
    char    NewName[64];

    do 
    {
        suffix++;
        x_sprintf( NewName, "%s%-3.3d", BaseName, suffix );
    }
    while( m_pEffect->FindElementByName(NewName) != NULL );

    // Assign the final name
    SetElementID( NewName );
}

//============================================================================

void element::AddKey_SRT( s32 T, const vector3& Scale, 
                                 const radian3& Rotation, 
                                 const vector3& Translation )
{

    // allocate the keys
    key* pKeyScale =        new key( 3 );
    key* pKeyRotation =     new key( 3 );
    key* pKeyTranslation =  new key( 3 );

    // fill out the keys
    pKeyScale->SetKey       ( T, (f32*)&Scale       );
    pKeyRotation->SetKey    ( T, (f32*)&Rotation    );
    pKeyTranslation->SetKey ( T, (f32*)&Translation );    

    // add the keys to the controller
    m_pScale->SetValue        ( T, pKeyScale          );
    m_pRotation->SetValue     ( T, pKeyRotation       );
    m_pTranslation->SetValue  ( T, pKeyTranslation    );
    
}

//============================================================================

void element::AddKey_Pos( s32 T, const vector3& Pos )
{
    // allocate the key
    key* pKeyTranslation =  new key( 3 );

    // fill out the key
    pKeyTranslation->SetKey ( T, (f32*)&Pos );    

    // add the key to the controller
    m_pTranslation->SetValue  ( T, pKeyTranslation    );
}

//============================================================================

void element::AddKey_Rotation( s32 T, const radian3& Rotation )
{
    // allocate the key
    key* pKeyRotation =     new key( 3 );

    // fill out the key
    pKeyRotation->SetKey    ( T, (f32*)&Rotation    );

    // add the key to the controller
    m_pRotation->SetValue     ( T, pKeyRotation       );
}

//============================================================================

void element::AddKey_Scale( s32 T, const vector3& Scale )
{

    // allocate the key
    key* pKeyScale =        new key( 3 );

    // fill out the key
    pKeyScale->SetKey       ( T, (f32*)&Scale       );

    // add the key to the controller
    m_pScale->SetValue        ( T, pKeyScale          );

}

//============================================================================

void element::AddKey_Color( s32 T, const xcolor& Color )
{
    // allocate the key
    key* pKeyColor = new key( 3 );
    key* pKeyAlpha = new key( 1 );

    // fill out the key
    f32 tmp[4];
    tmp[0] = Color.R;
    tmp[1] = Color.G;
    tmp[2] = Color.B;
    tmp[3] = Color.A;

    pKeyColor->SetKey ( T, tmp     );
    pKeyAlpha->SetKey ( T, &tmp[3] );

    // add the key to the controller
    m_pColor->SetValue  ( T, pKeyColor    );
    m_pAlpha->SetValue  ( T, pKeyAlpha    );

}

//============================================================================

void element::Translate( const vector3& Delta )
{
    s32 Count = m_pTranslation->GetKeyCount(); // m_pXfrm->GetNodeCount();
    s32 i;

    for ( i = 0; i < Count; i++ )
    {
        key* PosKey = m_pTranslation->GetKeyByIndex( i );

        f32* pPos = PosKey->GetKeyValue();

        pPos[0] += Delta.GetX();
        pPos[1] += Delta.GetY();
        pPos[2] += Delta.GetZ();
    }    
}

//============================================================================

void element::Rotate( const radian3& Delta )
{
    s32 Count = m_pRotation->GetKeyCount();
    s32 i;

    for ( i = 0; i < Count; i++ )
    {
        key* RotKey = m_pRotation->GetKeyByIndex( i );

        f32* pRot = RotKey->GetKeyValue();

        pRot[0] += Delta.Pitch;
        pRot[1] += Delta.Yaw;
        pRot[2] += Delta.Roll;
    }
}

//============================================================================

void element::Scale( const vector3& Delta )
{
    s32 Count = m_pScale->GetKeyCount();
    s32 i;

    for ( i = 0; i < Count; i++ )
    {
        key* ScaleKey = m_pScale->GetKeyByIndex( i );

        f32* pScale = ScaleKey->GetKeyValue();

        pScale[0] += Delta.GetX();
        pScale[1] += Delta.GetY();
        pScale[2] += Delta.GetZ();
    }
}

//============================================================================

void element::ChangeColorKeys( s32 DeltaR, s32 DeltaG, s32 DeltaB, s32 DeltaA )
{
    s32 Count = m_pColor->GetKeyCount();
    s32 i;

    // do the color first
    for ( i = 0; i < Count; i++ )
    {
        key* ColorKey = m_pColor->GetKeyByIndex( i );

        f32* pColor = ColorKey->GetKeyValue();

        pColor[0] += DeltaR;
        pColor[1] += DeltaG;
        pColor[2] += DeltaB;
    }

    // do the alpha separately
    Count = m_pAlpha->GetKeyCount();

    for ( i = 0; i < Count; i++ )
    {
        key* AlphaKey = m_pAlpha->GetKeyByIndex( i );

        f32* pAlpha = AlphaKey->GetKeyValue();

        pAlpha[0] += DeltaA;
    }
}

//============================================================================

xbool element::ExistsAtTime( f32 T ) const
{
    return( (T >= m_LifeStartFrame) && (m_IsImmortal || (T <= m_LifeStopFrame)) );
}

//============================================================================

xbool element::GetLocalBBoxAtTime( f32 T, bbox& BBox ) const
{
    if( ExistsAtTime( T ) )
    {
        BBox.Set( vector3(0.0f,0.0f,0.0f), 0.5f );
        return TRUE;
    }

    return FALSE;
}

//============================================================================

xbool element::GetWorldBBoxAtTime( f32 T, bbox& BBox ) const
{
    if( ExistsAtTime( T ) )
    {
        matrix4 L2W;

        VERIFY( GetLocalBBoxAtTime( T, BBox ) );
        VERIFY( GetL2WAtTime      ( T, L2W  ) );
        BBox.Transform( L2W );

        return TRUE;
    }

    return FALSE;
}

//============================================================================

xbool element::GetScaleAtTime( f32 T, vector3& Scale ) const
{
    f32 ScaleVals[3];

    if ( m_pScale->GetValue( T, ScaleVals ) )    
    {
        Scale.Set( ScaleVals[0], ScaleVals[1], ScaleVals[2] );
        return TRUE;
    }
    
    return FALSE;

}

//============================================================================

xbool element::GetRotationAtTime( f32 T, vector3& Rot ) const
{
    f32 RotVals[3];

    if ( m_pRotation->GetValue( T, RotVals ) )    
    {
        Rot.Set( RotVals[0], RotVals[1], RotVals[2] );
        return TRUE;
    }
    
    return FALSE;

}

//============================================================================

xbool element::GetPositionAtTime( f32 T, vector3& Point ) const
{
    f32 Pos[3];

    if ( m_pTranslation->GetValue( T, Pos ) )    
    {
        Point.Set( Pos[0], Pos[1], Pos[2] );
        return TRUE;
    }
    
    return FALSE;

}

//============================================================================

xbool element::GetL2WAtTime( f32 T, matrix4& L2W ) const
{
    if( ExistsAtTime( T ) )
    {
        f32     Pos  [3];
        f32     Scale[3];
        f32     Rot  [3];

        // Evaluate SRT controllers at T
        m_pScale->GetValue          ( T, Scale  );
        m_pRotation->GetValue       ( T, Rot    );
        m_pTranslation->GetValue    ( T, Pos    );

        // Setup L2W
        L2W.Identity ();
        L2W.Scale    ( vector3(Scale[0],Scale[1],Scale[2]) );
        L2W.Rotate   ( radian3(  Rot[0],  Rot[1],  Rot[2]) );
        L2W.Translate( vector3(  Pos[0],  Pos[1],  Pos[2]) );

        return TRUE;
    }

    return FALSE;
}

//============================================================================

xbool element::GetColorAtTime( f32 T, xcolor& Color ) const
{
    if( ExistsAtTime( T ) )
    {
        f32     Colors[3];
        f32     Alpha;

        // Evaluate color controllers at T
        m_pColor->GetValue          ( T, Colors );
        m_pAlpha->GetValue          ( T, &Alpha );

        // Clamp color values to range
        Colors[0] = MINMAX( 0.0f, Colors[0], 255.0f );
        Colors[1] = MINMAX( 0.0f, Colors[1], 255.0f );
        Colors[2] = MINMAX( 0.0f, Colors[2], 255.0f );
        Alpha     = MINMAX( 0.0f,     Alpha, 255.0f );

        // Create xcolor from the key
        Color.Set( (u8)Colors[0], (u8)Colors[1], (u8)Colors[2], (u8)Alpha );

        return TRUE;
    }

    return FALSE;
}

//============================================================================

s32 element::GetBookends( s32& FirstKey, s32& LastKey )
{
    FirstKey = 99999;
    LastKey = -99999;

    key* pKeys1[5];
    key* pKeys2[5];

    m_pScale->GetBookends      ( &pKeys1[0], &pKeys2[0] );
    m_pRotation->GetBookends   ( &pKeys1[1], &pKeys2[1] );
    m_pTranslation->GetBookends( &pKeys1[2], &pKeys2[2] );
    m_pColor->GetBookends      ( &pKeys1[3], &pKeys2[3] );
    m_pAlpha->GetBookends      ( &pKeys1[4], &pKeys2[4] );

    for ( s32 i = 0 ; i < 5 ; i++ )
    {
        if ( pKeys1[i] )
            FirstKey = MIN( FirstKey, pKeys1[i]->GetKeyTime() );

        if ( pKeys2[i] )
            LastKey = MAX( LastKey, pKeys2[i]->GetKeyTime() );
    }

    s32 MaxKeys = m_pScale->GetKeyCount();

    MaxKeys = MAX( MaxKeys, m_pRotation->GetKeyCount() );
    MaxKeys = MAX( MaxKeys, m_pTranslation->GetKeyCount() );
    MaxKeys = MAX( MaxKeys, m_pColor->GetKeyCount() );
    MaxKeys = MAX( MaxKeys, m_pAlpha->GetKeyCount() );

    return MaxKeys;
}

//============================================================================

void element::SetSelected( xbool IsSelected )
{
    m_IsSelected = IsSelected;
}

xbool element::IsSelected( void ) const
{
    return m_IsSelected;
}


//============================================================================

void element::RenderBBox( f32 T ) const
{
    if( m_pEffect->RenderBBoxesEnabled() )
    {
        bbox BBox;
        if( GetLocalBBoxAtTime( T, BBox ) ) // Returns FALSE if the object doesn't exist at that time
        {
            draw_BBox( BBox, IsSelected() ? XCOLOR_WHITE : XCOLOR_GREY );
        }
    }
}

//============================================================================

void element::RenderTrajectory( void ) const
{
    if( m_ShowTrajectory )
    {
        s32 NumKeys = m_pTranslation->GetKeyCount();

        if( NumKeys > 1 )
        {
            // Reset the L2W Matrix
            matrix4 m;
            m.Identity();
            draw_SetL2W( m );

            s32 StartFrame  = m_pTranslation->GetKeyByIndex( 0         )->GetKeyTime();
            s32 EndFrame    = m_pTranslation->GetKeyByIndex( NumKeys-1 )->GetKeyTime();

            f32 PosA[3];
            f32 PosB[3];

            s32 i;
            s32 KeyTime;

            // Draw the trajectory as line segments
            for( i = StartFrame; i < EndFrame; i+=2 )
            {
                m_pTranslation->GetValue( f32( i ), PosA );
                m_pTranslation->GetValue( f32(i+1), PosB );

                draw_Line( vector3(PosA[0],PosA[1],PosA[2]), vector3(PosB[0],PosB[1],PosB[2]) );
            }

            // Draw the keyframes as points
            for( i = 0; i < NumKeys; i++ )
            {
                KeyTime = m_pTranslation->GetKeyByIndex( i )->GetKeyTime();
                m_pTranslation->GetValue( f32( KeyTime ), PosA );

                draw_Point( vector3(PosA[0],PosA[1],PosA[2]) );
            }
        }
    }
}


//============================================================================

void element::ExportData( export::fx_elementhdr& ElemHdr,
                          xstring& Type,
                          xbytestream& Stream, 
                          s32 ExportTarget )
{
    //s32     FirstKey, LastKey, j;
    s32 j;

    // Find the extents of life    
    /*
    if ( GetBookends( FirstKey, LastKey ) > 0 )
    {
        ElemHdr.TimeStart = FirstKey / 30.0f;
        ElemHdr.TimeStop  = LastKey  / 30.0f;
    }
    else
    {
        s32 MinT, MaxT;
        g_pMainFrame->GetMinMaxTime( MinT, MaxT );
        ElemHdr.TimeStart = MinT / 30.0f;
        ElemHdr.TimeStop  = MaxT / 30.0f;
    }*/
    ElemHdr.TimeStart = (f32)(m_LifeStartFrame / 30.0f);
    ElemHdr.TimeStop  = (f32)(m_LifeStopFrame  / 30.0f);

    ElemHdr.ReadZ = m_ZRead;
    
    if( m_IsImmortal )
    {
        ElemHdr.TimeStop = F32_MAX;
    }

    // Set default SRTCA values
    ElemHdr.Scale[0]     = 1.0f;
    ElemHdr.Scale[1]     = 1.0f;
    ElemHdr.Scale[2]     = 1.0f;

    ElemHdr.Rotate[0]    = 1.0f;
    ElemHdr.Rotate[1]    = 0.0f;
    ElemHdr.Rotate[2]    = 0.0f;

    ElemHdr.Translate[0] = 0.0f;
    ElemHdr.Translate[1] = 0.0f;
    ElemHdr.Translate[2] = 0.0f;

    ElemHdr.Color[0]     = 1.0f;
    ElemHdr.Color[1]     = 1.0f;
    ElemHdr.Color[2]     = 1.0f;
    ElemHdr.Color[3]     = 1.0f;

    // controller slots
    s32 StartIdx;
    s32 Channels[3];

    //-------------------------------------------------------------------------
    // Scale
    //-------------------------------------------------------------------------
    StartIdx = m_pScale->m_ExpIdx;    
    if ( m_pScale->GetOptimalKeyCount(Channels) ) //m_pScale->GetKeyCount()
    {
        for ( j = 0; j < 3; j++ )
        {
            if ( Channels[j] )
                ElemHdr.CtrlOffsets[j] = StartIdx++;
            else
            {
                ElemHdr.CtrlOffsets[j] = -1;
                key* pKey = m_pScale->GetKeyByIndex(0);
                f32* pData = pKey->GetKeyValue();
                ElemHdr.Scale[j] = pData[j];
            }             
        }            
    }
    else
    {
        for ( j = 0; j < 3; j++ )
            ElemHdr.CtrlOffsets[j] = -1;

        if ( m_pScale->GetKeyCount() )
        {
            key* pKey = m_pScale->GetKeyByIndex(0);
            f32* pData = pKey->GetKeyValue();

            ElemHdr.Scale[0] = pData[0];
            ElemHdr.Scale[1] = pData[1];
            ElemHdr.Scale[2] = pData[2];
        }
    }

    //-------------------------------------------------------------------------
    // Rotation
    //-------------------------------------------------------------------------
    StartIdx = m_pRotation->m_ExpIdx;    
    if ( m_pRotation->GetOptimalKeyCount(Channels) ) //m_pScale->GetKeyCount()
    {
        for ( j = 0; j < 3; j++ )
        {
            if ( Channels[j] )
                ElemHdr.CtrlOffsets[j+3] = StartIdx++;
            else
            {
                ElemHdr.CtrlOffsets[j+3] = -1;
                key* pKey = m_pRotation->GetKeyByIndex(0);
                f32* pData = pKey->GetKeyValue();
                ElemHdr.Rotate[j] = pData[j];
            }             
        }            
    }
    else
    {
        for ( j = 0; j < 3; j++ )
            ElemHdr.CtrlOffsets[j+3] = -1;

        if ( m_pRotation->GetKeyCount() )
        {
            key* pKey = m_pRotation->GetKeyByIndex(0);
            f32* pData = pKey->GetKeyValue();

            ElemHdr.Rotate[0] = pData[0];
            ElemHdr.Rotate[1] = pData[1];
            ElemHdr.Rotate[2] = pData[2];
        }
    }

    //-------------------------------------------------------------------------
    // Translation
    //-------------------------------------------------------------------------
    StartIdx = m_pTranslation->m_ExpIdx;    
    if ( m_pTranslation->GetOptimalKeyCount(Channels) ) //m_pScale->GetKeyCount()
    {
        for ( j = 0; j < 3; j++ )
        {
            if ( Channels[j] )
                ElemHdr.CtrlOffsets[j+6] = StartIdx++;
            else
            {
                ElemHdr.CtrlOffsets[j+6] = -1;
                key* pKey = m_pTranslation->GetKeyByIndex(0);
                f32* pData = pKey->GetKeyValue();
                ElemHdr.Translate[j] = pData[j];
            }             
        }            
    }
    else
    {
        for ( j = 0; j < 3; j++ )
            ElemHdr.CtrlOffsets[j+6] = -1;

        if ( m_pTranslation->GetKeyCount() )
        {
            key* pKey = m_pTranslation->GetKeyByIndex(0);
            f32* pData = pKey->GetKeyValue();

            ElemHdr.Translate[0] = pData[0];
            ElemHdr.Translate[1] = pData[1];
            ElemHdr.Translate[2] = pData[2];
        }
    }

    //-------------------------------------------------------------------------
    // Color
    //-------------------------------------------------------------------------
    StartIdx = m_pColor->m_ExpIdx;    
    if ( m_pColor->GetOptimalKeyCount(Channels) ) //m_pScale->GetKeyCount()
    {
        for ( j = 0; j < 3; j++ )
        {
            if ( Channels[j] )
                ElemHdr.CtrlOffsets[j+9] = StartIdx++;
            else
            {
                ElemHdr.CtrlOffsets[j+9] = -1;
                key* pKey = m_pColor->GetKeyByIndex(0);
                f32* pData = pKey->GetKeyValue();
                ElemHdr.Color[j] = pData[j];
            }             
        }            
    }
    else
    {
        for ( j = 0; j < 3; j++ )
            ElemHdr.CtrlOffsets[j+9] = -1;

        if ( m_pColor->GetKeyCount() )
        {
            key* pKey = m_pColor->GetKeyByIndex(0);
            f32* pData = pKey->GetKeyValue();

            ElemHdr.Color[0] = pData[0] / 255.0f;
            ElemHdr.Color[1] = pData[1] / 255.0f;
            ElemHdr.Color[2] = pData[2] / 255.0f;
        }
    }
    
    //-------------------------------------------------------------------------
    // Alpha
    //-------------------------------------------------------------------------
    StartIdx = m_pAlpha->m_ExpIdx;    
    if ( m_pAlpha->GetOptimalKeyCount(Channels) ) //m_pScale->GetKeyCount()
    {
        if ( Channels[0] )
            ElemHdr.CtrlOffsets[12] = StartIdx;
        else
        {
            ElemHdr.CtrlOffsets[12] = -1;
            key* pKey = m_pAlpha->GetKeyByIndex(0);
            f32* pData = pKey->GetKeyValue();
            ElemHdr.Color[3] = *pData;
        }             
    }
    else
    {
        ElemHdr.CtrlOffsets[12] = -1;

        if ( m_pAlpha->GetKeyCount() )
        {
            key* pKey = m_pAlpha->GetKeyByIndex(0);
            f32* pData = pKey->GetKeyValue();

            ElemHdr.Color[3] = pData[0] / 255.0f;
        }
    }

    //---------------------------------------------------------------------
    Stream.Append( (const u8*)&ElemHdr, sizeof(export::fx_elementhdr) );

    // strcpy( ID, "SPRITE" );
    // ElemHdr.BitmapIndex = 0;

    if( !m_CustomType.IsEmpty() )
    {
        Type = m_CustomType + '~' + Type;
    }
}

  
//============================================================================

xbool element::OnPropertyChanged( s32 T, xstring& Field, xstring& Value )
{
    xbool Dirty = FALSE;

    // Strip out the header information from the Field, so we can check the property strings
    s32     PropLength  = Field.GetLength() - ( 17 + m_ID.GetLength() );
    xstring Property    = Field.Right( PropLength );

    // Non-Animatable Properties
    if( x_strcmp( Property, "Object\\Name" ) == 0 )
    {
        m_ID    = Value;
        Dirty   = TRUE;
    }
    else if( x_strcmp( Property, "Object\\Custom Type" ) == 0 )
    {
        m_CustomType = Value;
        Dirty        = TRUE;
    }
    else if( x_strcmp( Property, "Object\\Immortal" ) == 0 )
    {
        m_IsImmortal    = x_strcmp( Value, "true" ) == 0 ? TRUE : FALSE;
        Dirty           = TRUE;
    }
    else if( x_strcmp( Property, "Object\\Start Frame" ) == 0 )
    {
        m_LifeStartFrame = x_atoi( Value );
        Dirty            = TRUE;
    }
    else if( x_strcmp( Property, "Object\\Stop Frame" ) == 0 )
    {
        m_LifeStopFrame = x_atoi( Value );
        Dirty           = TRUE;
    }
    else if( x_strcmp( Property, "Object\\Show Trajectory" ) == 0 )
    {
        m_ShowTrajectory = x_strcmp( Value, "true" ) == 0 ? TRUE : FALSE;
        Dirty            = TRUE;
    }

    else if( x_strcmp( Property, "Transform\\Position" ) == 0 )
    {
             if( x_strcmp( Value, "Smooth" ) == 0 )     { ((ctrl_linear*)m_pTranslation)->SetSmooth( true  ); }
        else if( x_strcmp( Value, "Linear" ) == 0 )     { ((ctrl_linear*)m_pTranslation)->SetSmooth( false ); }

        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Transform\\Position\\In Type" ) == 0 )
    {
        controller::out_of_range    ORT = (controller::out_of_range)controller::OutOfRangeType_FromString( Value );
        m_pTranslation->SetInType( ORT );

        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Transform\\Position\\Out Type" ) == 0 )
    {
        controller::out_of_range    ORT = (controller::out_of_range)controller::OutOfRangeType_FromString( Value );
        m_pTranslation->SetOutType( ORT );

        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Transform\\Rotation" ) == 0 )
    {
             if( x_strcmp( Value, "Smooth" ) == 0 )     { ((ctrl_linear*)m_pRotation)->SetSmooth( true  ); }
        else if( x_strcmp( Value, "Linear" ) == 0 )     { ((ctrl_linear*)m_pRotation)->SetSmooth( false ); }

        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Transform\\Rotation\\In Type" ) == 0 )
    {
        controller::out_of_range    ORT = (controller::out_of_range)controller::OutOfRangeType_FromString( Value );
        m_pRotation->SetInType( ORT );

        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Transform\\Rotation\\Out Type" ) == 0 )
    {
        controller::out_of_range    ORT = (controller::out_of_range)controller::OutOfRangeType_FromString( Value );
        m_pRotation->SetOutType( ORT );

        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Transform\\Scale" ) == 0 )
    {
             if( x_strcmp( Value, "Smooth" ) == 0 )     { ((ctrl_linear*)m_pScale)->SetSmooth( true  ); }
        else if( x_strcmp( Value, "Linear" ) == 0 )     { ((ctrl_linear*)m_pScale)->SetSmooth( false ); }

        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Transform\\Scale\\In Type" ) == 0 )
    {
        controller::out_of_range    ORT = (controller::out_of_range)controller::OutOfRangeType_FromString( Value );
        m_pScale->SetInType( ORT );

        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Transform\\Scale\\Out Type" ) == 0 )
    {
        controller::out_of_range    ORT = (controller::out_of_range)controller::OutOfRangeType_FromString( Value );
        m_pScale->SetOutType( ORT );

        Dirty = TRUE;
    }

    else if( x_strcmp( Property, "Material\\Z Read" ) == 0 )
    {
        m_ZRead = x_strcmp( Value, "true" ) == 0 ? TRUE : FALSE;
        Dirty   = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Color" ) == 0 )
    {
        if( x_strcmp( Value, "Smooth" ) == 0 )
        {
            ((ctrl_linear*)m_pColor)->SetSmooth( true );
            ((ctrl_linear*)m_pAlpha)->SetSmooth( true );
        }
        else if( x_strcmp( Value, "Linear" ) == 0 )
        {
            ((ctrl_linear*)m_pColor)->SetSmooth( false );
            ((ctrl_linear*)m_pAlpha)->SetSmooth( false );
        }

        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Color\\In Type" ) == 0 )
    {
        controller::out_of_range    ORT = (controller::out_of_range)controller::OutOfRangeType_FromString( Value );

        m_pColor->SetInType( ORT ); // Temporary until we split UI for color & alpha
        m_pAlpha->SetInType( ORT ); // Temporary until we split UI for color & alpha

        Dirty = TRUE;
    }
    else if( x_strcmp( Property, "Material\\Color\\Out Type" ) == 0 )
    {
        controller::out_of_range    ORT = (controller::out_of_range)controller::OutOfRangeType_FromString( Value );

        m_pColor->SetOutType( ORT ); // Temporary until we split UI for color & alpha
        m_pAlpha->SetOutType( ORT ); // Temporary until we split UI for color & alpha

        Dirty = TRUE;
    }

    // Animatable Properties
    else if ( m_pEffect->IsAnimateModeOn() == TRUE )
    {
        if( x_strcmp( Property, "Transform\\Position\\XYZ" ) == 0 )
        {
            vector3 Pos;
            String2V3( Value, Pos );
            AddKey_Pos( T, Pos );
            Dirty = TRUE;
        }

        else if( x_strcmp( Property, "Transform\\Rotation\\XYZ" ) == 0 )
        {
            vector3 RotVals;
            radian3 Rotation;
            String2V3( Value, RotVals );

            // values are in degrees, convert to radian
            Rotation.Pitch = DEG_TO_RAD(RotVals.GetX());
            Rotation.Yaw = DEG_TO_RAD(RotVals.GetY());
            Rotation.Roll = DEG_TO_RAD(RotVals.GetZ());

            AddKey_Rotation( T, Rotation );
            Dirty = TRUE;
        }

        else if( x_strcmp( Property, "Transform\\Scale\\XYZ" ) == 0 )
        {
            vector3 Scale;
            String2V3( Value, Scale );
            AddKey_Scale( T, Scale );
            Dirty = TRUE;
        }

        else if( x_strcmp( Property, "Material\\Color\\RGBA" ) == 0 )
        {
            xcolor Color;
            String2Color( Value, Color );
            AddKey_Color( T, Color );
            Dirty = TRUE;
        }
    }
    else
    {
        f32 Pos[3];
        f32 Rot[3];
        f32 ScaleF[3];
        f32 Col[3];
        f32 Alpha;

        m_pTranslation->GetValue( (f32)T, Pos    );
        m_pRotation->GetValue   ( (f32)T, Rot    );
        m_pScale->GetValue      ( (f32)T, ScaleF );
        m_pColor->GetValue      ( (f32)T, Col    );
        m_pAlpha->GetValue      ( (f32)T, &Alpha );

        if( x_strcmp( Property, "Transform\\Position\\XYZ" ) == 0 )
        {
            vector3 Position;
            String2V3( Value, Position );
            vector3 Delta = Position - vector3( Pos[0], Pos[1], Pos[2] );

            Translate( Delta );
            Dirty = TRUE;
        }

        else if( x_strcmp( Property, "Transform\\Rotation\\XYZ" ) == 0 )
        {
            radian3 Rotation;
            vector3 RotVals;
            String2V3( Value, RotVals );

            // values are in degrees, convert to radian
            Rotation.Pitch = DEG_TO_RAD(RotVals.GetX());
            Rotation.Yaw = DEG_TO_RAD(RotVals.GetY());
            Rotation.Roll = DEG_TO_RAD(RotVals.GetZ());

            radian3 Delta = Rotation - radian3( Rot[0], Rot[1], Rot[2] );

            Rotate( Delta );
            Dirty = TRUE;
        }

        else if( x_strcmp( Property, "Transform\\Scale\\XYZ" ) == 0 )
        {
            vector3 _Scale;
            String2V3( Value, _Scale );
            vector3 Delta = _Scale - vector3( ScaleF[0], ScaleF[1], ScaleF[2] );

            Scale( Delta );
            Dirty = TRUE;
        }

        else if( x_strcmp( Property, "Material\\Color\\RGBA" ) == 0 )
        {
            xcolor Color;
            String2Color( Value, Color );
            
            s32     DeltaR = Color.R - (s32)Col[0];
            s32     DeltaG = Color.G - (s32)Col[1];
            s32     DeltaB = Color.B - (s32)Col[2];
            s32     DeltaA = Color.A - (s32)Alpha;

            ChangeColorKeys( DeltaR, DeltaG, DeltaB, DeltaA );
            Dirty = TRUE;
        }
    }

    return Dirty;
}

//============================================================================
void element::Save( igfmgr& Igf )
{
    // save the base data for the element
    // namely, controller serial numbers and other basic data
    Igf.AddString( "ID",        m_ID );
    Igf.AddS32   ( "BASE_VER",  BASE_VER, "Version number of the base element" );
    Igf.AddString( "Custom",    m_CustomType );

    Igf.AddS32( "S", m_pScale->GetSerNum(), "Scale controller serial number" );
    Igf.AddS32( "R", m_pRotation->GetSerNum(), "Rotation controller serial number" );
    Igf.AddS32( "T", m_pTranslation->GetSerNum(), "Translation controller serial number" );
    Igf.AddS32( "C", m_pColor->GetSerNum(), "Color controller serial number" );
    Igf.AddS32( "A", m_pAlpha->GetSerNum(), "Alpha controller serial number" );

    Igf.AddBool( "Export",          m_Export, "Should this element be exported?" );
    Igf.AddBool( "Immortal",        m_IsImmortal        );
    Igf.AddS32 ( "Start",           m_LifeStartFrame    );
    Igf.AddS32 ( "Stop",            m_LifeStopFrame     );
    Igf.AddBool( "ShowTrajectory",  m_ShowTrajectory    );
    Igf.AddBool( "ZRead",           m_ZRead             );

}

//============================================================================
void element::Load( igfmgr& Igf )
{
    s32             Ver = 0;

    m_ID =          Igf.GetString( "ID" );
    m_CustomType =  Igf.GetString( "Custom" );

    // put the serial number of the controller in place of an actual pointer value
    // then the effect will fixup the pointer after the load
    m_pScale =      (ctrl_linear*)Igf.GetS32( "S" );
    m_pRotation =   (ctrl_linear*)Igf.GetS32( "R" );
    m_pTranslation =(ctrl_linear*)Igf.GetS32( "T" );
    m_pColor =      (ctrl_linear*)Igf.GetS32( "C" );
    m_pAlpha =      (ctrl_linear*)Igf.GetS32( "A" );

    // load or set default
    if( Igf.Find( "BASE_VER" ) )
        Ver = Igf.GetS32();

    if( Ver == 0 )
    {
        m_Export            = TRUE;
        m_IsImmortal        = FALSE;
        m_LifeStartFrame    = 0;
        m_LifeStopFrame     = 60;
        m_ShowTrajectory    = FALSE;
        m_ZRead             = TRUE;
    }
    else
    {
        // Version 1+
        m_Export            = Igf.GetBool(    "Export"    );
        m_IsImmortal        = Igf.GetBool(    "Immortal"  );
        m_LifeStartFrame    = Igf.GetS32 (    "Start"     );
        m_LifeStopFrame     = Igf.GetS32 (    "Stop"      );
        m_ShowTrajectory    = Igf.GetBool("ShowTrajectory");

        // Version 2+
        m_ZRead             = (Ver > 1 ) ? Igf.GetBool( "ZRead" ) : true;
    }
}


//============================================================================
// Utility fuunctions
//============================================================================
s32 String2V3    ( const char* pStr, vector3& V3   )
{
    s32             i;
    const char*     p[3];               // bookmark 3 places along the string
    xbool           Watching = TRUE;    // watching for numeric value (otherwise watching for white space)
    s32             Pos = 0;

    for ( i = 0; i < x_strlen(pStr); i++ )
    {
        if ( Watching == TRUE )
        {
            if ( x_strchr("-0123456789.", pStr[i]) != NULL )
            {
                Watching = FALSE;
                p[Pos++] = &pStr[i];
            }
        }
        else
        {
            if ( x_strchr(" ,", pStr[i]) != NULL )
            {
                Watching = TRUE;
            }
        }
    }

    if ( Pos > 0 )      V3.GetX() = x_atof( p[0] );
    if ( Pos > 1 )      V3.GetY() = x_atof( p[1] );
    if ( Pos > 2 )      V3.GetZ() = x_atof( p[2] );

    return Pos;
}

//============================================================================

s32 String2Color ( const char* pStr, xcolor& Color )
{
    s32             i;
    const char*     p[4];               // bookmark 4 places along the string
    xbool           Watching = TRUE;    // watching for numeric value (otherwise watching for white space)
    s32             Pos = 0;

    for ( i = 0; i < x_strlen(pStr); i++ )
    {
        if ( Watching == TRUE )
        {
            if ( x_strchr("-0123456789", pStr[i]) != NULL )
            {
                Watching = FALSE;
                p[Pos++] = &pStr[i];
            }
        }
        else
        {
            if ( x_strchr(" ,", pStr[i]) != NULL )
            {
                Watching = TRUE;
            }
        }
    }

    if ( Pos > 0 )      Color.R = x_atoi( p[0] );
    if ( Pos > 1 )      Color.G = x_atoi( p[1] );
    if ( Pos > 2 )      Color.B = x_atoi( p[2] );
    if ( Pos > 3 )      Color.A = x_atoi( p[3] );

    return Pos;

}

//============================================================================

xbool element::GetProperty( s32 Idx, s32 T, xcolor& UIColor, xstring& Name, xstring& Value, xbool& IsDisabled, base::prop_type& Type )
{
    f32     Pos  [3];
    f32     Scale[3];
    f32     Rot  [3];
    f32     Color[3];
    f32     Alpha;

    m_pTranslation->GetValue   ( (f32)T, Pos    );
    m_pRotation->GetValue      ( (f32)T, Rot    );
    m_pScale->GetValue         ( (f32)T, Scale  );
    m_pColor->GetValue         ( (f32)T, Color  );
    m_pAlpha->GetValue         ( (f32)T, &Alpha );

    xcolor  HeaderColor ( 119, 128, 144 );
    xcolor  ItemColor   ( 176, 176, 176 );

    switch( Idx )
    {
        case 0:
            //============================================================================
            Name.Format( "Object" );
            Value.Format( "" );
            Type        = PROP_HEADER;
            UIColor     = HeaderColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 1:
            //============================================================================
            Name.Format( "Object\\Name" );
            Value.Format( "%s", (const char*)m_ID );
            Type        = PROP_STRING;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 2:
            //============================================================================
            Name.Format( "Object\\Custom Type" );
            Value.Format( "%s", (const char*)m_CustomType );
            Type        = PROP_STRING;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 3:
            //============================================================================
            Name.Format( "Object\\Immortal" );
            Value.Format( "%s", m_IsImmortal == TRUE ? "true" : "false" );
            Type        = PROP_BOOL;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 4:
            //============================================================================
            Name.Format( "Object\\Start Frame" );
            Value.Format( "%d", m_LifeStartFrame );
            Type        = PROP_FLOAT;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 5:
            //============================================================================
            Name.Format( "Object\\Stop Frame" );
            Type        = PROP_FLOAT;
            UIColor     = ItemColor;

            if( m_IsImmortal )
            {
                Value       = "";
                IsDisabled  = TRUE;
            }
            else
            {
                Value.Format( "%d", m_LifeStopFrame );
                IsDisabled  = FALSE;
            }

            return TRUE;
        case 6:
            //============================================================================
            Name.Format( "Object\\Show Trajectory" );
            Value.Format( "%s", m_ShowTrajectory == TRUE ? "true" : "false" );
            Type        = PROP_BOOL;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 7:
            //============================================================================
            Name.Format( "Transform" );
            Value.Format( "" );
            Type        = PROP_HEADER;
            UIColor     = HeaderColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 8:
            //============================================================================
            Name.Format( "Transform\\Position" );
            if( ((ctrl_linear*)m_pTranslation)->IsSmooth() )    { Value.Format( "%s", "Smooth" ); }
            else                                                { Value.Format( "%s", "Linear" ); }
            Type        = PROP_CONTROLLERTYPE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 9:
            //============================================================================
            Name.Format( "Transform\\Position\\In Type" );
            Value = controller::OutOfRangeType_ToString( m_pTranslation->GetInType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 10:
            //============================================================================
            Name.Format( "Transform\\Position\\Out Type" );
            Value = controller::OutOfRangeType_ToString( m_pTranslation->GetOutType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 11:
            //============================================================================
            Name.Format( "Transform\\Position\\XYZ" );
            Value.Format( "%g, %g, %g", Pos[0], Pos[1], Pos[2] );
            Type        = PROP_V3;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 12:
            //============================================================================
            Name.Format( "Transform\\Rotation" );
            if( ((ctrl_linear*)m_pRotation)->IsSmooth() )       { Value.Format( "%s", "Smooth" ); }
            else                                                { Value.Format( "%s", "Linear" ); }
            Type        = PROP_CONTROLLERTYPE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 13:
            //============================================================================
            Name.Format( "Transform\\Rotation\\In Type" );
            Value = controller::OutOfRangeType_ToString( m_pRotation->GetInType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 14:
            //============================================================================
            Name.Format( "Transform\\Rotation\\Out Type" );
            Value = controller::OutOfRangeType_ToString( m_pRotation->GetOutType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 15:
            //============================================================================
            Name.Format( "Transform\\Rotation\\XYZ" );
            Value.Format( "%g, %g, %g", RAD_TO_DEG(Rot[0]), RAD_TO_DEG(Rot[1]), RAD_TO_DEG(Rot[2]) );
            Type        = PROP_V3;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 16:
            //============================================================================
            Name.Format( "Transform\\Scale" );
            if( ((ctrl_linear*)m_pScale)->IsSmooth() )          { Value.Format( "%s", "Smooth" ); }
            else                                                { Value.Format( "%s", "Linear" ); }
            Type        = PROP_CONTROLLERTYPE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 17:
            //============================================================================
            Name.Format( "Transform\\Scale\\In Type" );
            Value = controller::OutOfRangeType_ToString( m_pScale->GetInType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 18:
            //============================================================================
            Name.Format( "Transform\\Scale\\Out Type" );
            Value = controller::OutOfRangeType_ToString( m_pScale->GetOutType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 19:
            //============================================================================
            Name.Format( "Transform\\Scale\\XYZ" );
            Value.Format( "%g, %g, %g", Scale[0], Scale[1], Scale[2] );
            Type        = PROP_V3;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 20:
            //============================================================================
            Name.Format( "Material" );
            Value.Format( "" );
            Type        = PROP_HEADER;
            UIColor     = HeaderColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 21:
            //============================================================================
            Name.Format( "Material\\Color" );
            if( ((ctrl_linear*)m_pColor)->IsSmooth() )          { Value.Format( "%s", "Smooth" ); }
            else                                                { Value.Format( "%s", "Linear" ); }
            Type        = PROP_CONTROLLERTYPE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 22:
            //============================================================================
            Name.Format( "Material\\Color\\In Type" );
            Value = controller::OutOfRangeType_ToString( m_pColor->GetInType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 23:
            //============================================================================
            Name.Format( "Material\\Color\\Out Type" );
            Value = controller::OutOfRangeType_ToString( m_pColor->GetOutType() );
            Type        = PROP_LOOPMODE;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 24:
            //============================================================================
            Name.Format( "Material\\Color\\RGBA" );
            Value.Format( "%d, %d, %d, %d", (u8)Color[0], (u8)Color[1], (u8)Color[2], (u8)Alpha );
            Type        = PROP_COLOR;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;
        case 25:
            //============================================================================
            Name.Format( "Material\\Z Read" );
            Value.Format( "%s", m_ZRead == TRUE ? "true" : "false" );
            Type        = PROP_BOOL;
            UIColor     = ItemColor;
            IsDisabled  = FALSE;
            return TRUE;

        default:
            return FALSE;
    }
   
    // keep the compilers happy
    return FALSE;
}

} // namespace fx_core
