#include "controller.hpp"

namespace fx_core
{

#define CONTROLLER_VER      1


//============================================================================
// The registration list for all controller types
static ctrl_reg* s_RegistrationList[256] = { 0 };
static s32       s_RegCount = 0;
static u32       s_SerialNum = 0;


//============================================================================
// Define the ctrl_reg constructor
ctrl_reg::ctrl_reg( const char* pTypeName, ctrl_factory_fn FactoryFn )
{
    m_pTypeName = pTypeName;
    m_pFactoryFn = FactoryFn;

    s_RegistrationList[s_RegCount++] = this;
}

//============================================================================
// CONTROLLER REGISTRATIONS
//============================================================================
controller*         s_LinearKeyFactory( void )  { return (controller*)new ctrl_linear; }
static ctrl_reg     s_LinearKeyReg              ( LINEAR_KEY, s_LinearKeyFactory );

controller*         s_SmoothKeyFactory( void )  { return (controller*)new ctrl_smooth; }
static ctrl_reg     s_SmoothKeyReg              ( SMOOTH_KEY, s_SmoothKeyFactory );

//============================================================================
// Create a controller by type
controller* MakeController( const char* pType )
{
    for ( s32 i = 0; i < s_RegCount; i++ )
    {
        if ( x_strcmp(s_RegistrationList[i]->m_pTypeName, pType) == 0 )
        {
            return (controller*)(*s_RegistrationList[i]->m_pFactoryFn)();
        }
    }

    return NULL;
}


//============================================================================
// CONTROLLERS 
//============================================================================

//============================================================================
// The base class constructor
controller::controller()
{
    m_SerialNum = s_SerialNum++;
    m_InType    = controller::CLAMP;
    m_OutType   = controller::CLAMP;
    m_Scalar    = 1.0f;
}

//============================================================================
// The base class constructor
ctrl_linear::ctrl_linear()
{
    m_SerialNum = s_SerialNum++;
    m_InType    = controller::CLAMP;
    m_OutType   = controller::CLAMP;

    m_IsSmooth  = false;
}

//============================================================================
// Keys
void key::SaveData( igfmgr& Igf )
{
    hfield  CurGrp;
    s32     i;

    CurGrp = Igf.AddGroup( "Key" );
    Igf.EnterGroup( CurGrp );

    Igf.AddS32( "Time", m_Time, "Key Time" );
    for ( i = 0; i < m_nVals; i++ )
    {
        Igf.AddF32( "F32", m_pData[i] );
    }
    Igf.ExitGroup();
}

//============================================================================
void key::LoadData( igfmgr& Igf )
{
    // we're already inside the key group, just load the data
    m_Time = Igf.GetS32( "Time" );

    // find the first float
    Igf.Find( "F32" );

    for ( s32 i = 0; i < m_nVals; i++ )
    {
        m_pData[i] = Igf.GetF32();
        Igf.Next();
    }

}


//============================================================================
key* key::CopyOf( void )
{
    key* pNew = new key(m_nVals);

    x_memmove( pNew->m_pData, m_pData, m_nVals * sizeof(f32) );
    pNew->m_Time = m_Time;

    // return the new key
    return pNew;
}

//============================================================================
// THE BASE KEYFRAME CONTROLLER

//============================================================================
// Destructor
ctrl_key::~ctrl_key()
{
    s32 i, j;

    j = m_Keys.GetCount();

    for ( i = 0; i < j; i++ )
    {
        delete m_Keys[i];
    }
}

//============================================================================
// Set the value at a time (does not support interframe times, obviously)
void ctrl_key::SetValue( s32 T, key* pKey ) 
{
    // search the array for the appropriate location for this key
    if ( !m_Keys.GetCount() )
        m_Keys.Append( pKey );
    else
    {
        s32 i, j = m_Keys.GetCount();

        for ( i = 0; i < j; i++ )
        {
            // make sure time is valid
            ASSERT( m_Keys.GetAt(i)->GetKeyTime() != 0xDEADBEEF );

            // figure out where to put it
            if ( m_Keys.GetAt(i)->GetKeyTime() == T )
            {
                // first, nuke the existing key
                delete m_Keys.GetAt(i);
                // replace it with the new key
                m_Keys.SetAt( i, pKey );
                // all done
                return;
            }
            else
            if ( T < m_Keys.GetAt(i)->GetKeyTime() )
            {
                m_Keys.Insert( i, pKey );
                return;
            }
        }
        // check the only other possible case
        if ( T > m_Keys.GetAt(j-1)->GetKeyTime() )
        {
            m_Keys.Insert( j, pKey );
            return;
        }
    
        // should never make it here!
        ASSERT( FALSE );
    }
}

//============================================================================
// Get the key based on the index in the key array
key* ctrl_key::GetKeyByIndex( s32 Idx ) const
{
    ASSERT( m_Keys.GetAt(Idx) );
    ASSERT( m_Keys.GetAt(Idx)->GetKeyTime() != 0xDEADBEEF );

    return m_Keys.GetAt(Idx);
}

//============================================================================
// Count how many channels are actually non-static
s32 ctrl_key::GetOptimalKeyCount( s32* pChannels )
{
    s32 i, j;
    s32 NonStaticCount = 0;

    for ( i = 0; i < m_nFloats; i++ )
    {
        // initialize to zero
        pChannels[i] = 0;

        // check this value in each key
        for ( j = 0; j < m_Keys.GetCount()-1; j++ )
        {
            f32* pKeys1 = m_Keys[j]->GetKeyValue();
            f32* pKeys2 = m_Keys[j+1]->GetKeyValue();
            
            if ( pKeys1[i] != pKeys2[i] )
            {
                pChannels[i] = 1;
                NonStaticCount++;
                break;
            }
        }
    }

    return NonStaticCount;
}

//============================================================================
// Get the first and last key in the array
void ctrl_key::GetBookends( key** FirstKey, key** LastKey ) const
{
    if ( m_Keys.GetCount() )
    {
        *FirstKey = m_Keys.GetAt(0);
        *LastKey = m_Keys.GetAt( m_Keys.GetCount()-1 );
    }
    else
    {
        *FirstKey = NULL;
        *LastKey = NULL;
    }        
}

//============================================================================
// Get the Min and Max T values that are keyed

s32 ctrl_key::GetMinT( void ) const
{
    key* pKeyFirst;
    key* pKeyLast;
    GetBookends( &pKeyFirst, &pKeyLast );
    return pKeyFirst->GetKeyTime();
}

s32 ctrl_key::GetMaxT( void ) const
{
    key* pKeyFirst;
    key* pKeyLast;
    GetBookends( &pKeyFirst, &pKeyLast );
    return pKeyLast->GetKeyTime();
}

//============================================================================
// Delete the key at the specified index
xbool       ctrl_key::DeleteKeyByIndex    ( s32 Idx )
{
    if ( (Idx < 0) ||
         (Idx >= m_Keys.GetCount()) )
         return FALSE;

    delete m_Keys[Idx];
    m_Keys.Delete( Idx, 1 );

    return TRUE;
}

//============================================================================
// Delete the key at time T if there's a key there.
xbool       ctrl_key::DeleteKey           ( s32 T   )
{
    s32 i, j;

    j = m_Keys.GetCount();

    for ( i = 0 ; i < j; i++ )
    {
        if ( m_Keys.GetAt(i)->GetKeyTime() == T )
        {
            delete m_Keys[i];
            m_Keys.Delete( i, 1 );
            return TRUE;
        }
    }

    return FALSE;
}

//============================================================================
// Copy a keyframed controller
controller* ctrl_key::CopyOf( void ) const
{
    ctrl_key* pNew = new ctrl_key;

    pNew->SetNumFloats( m_nFloats );
    pNew->m_InType = m_InType;
    pNew->m_OutType = m_OutType;
    pNew->m_Scalar = m_Scalar;

    // duplicate all my keys
    s32 i, j;
    j = m_Keys.GetCount();

    for ( i = 0; i < j; i++ )
    {
        key* pKey =     m_Keys.GetAt(i);
        key* pNewKey =  pKey->CopyOf();

        pNew->SetValue( pKey->GetKeyTime(), pNewKey );
    }

    return pNew;
}


s32 ctrl_key::GetNumFloats( void ) const
{
    return m_nFloats;
}

void ctrl_key::SaveData( igfmgr& Igf ) const
{
    s32     i;

    // save all key data
    Igf.AddS32  ( "Count",    m_Keys.GetCount(),  "Total number of keys"          );
    Igf.AddS32  ( "Floats",   m_nFloats,          "Number of floats per key"      );
    Igf.AddS32  ( "Version",  CONTROLLER_VER,     "Version of the controller"     );
    Igf.AddS32  ( "SerNum",   m_SerialNum,        "Serial number"                 );
    Igf.AddS32  ( "InType",   m_InType,           "Out of bounds type - inbound"  );
    Igf.AddS32  ( "OutType",  m_OutType,          "Out of bounds type - outbound" );
    Igf.AddF32  ( "Scalar",   m_Scalar,           "Scalar multiplier"             );

    // save each key
    for ( i = 0; i < m_Keys.GetCount(); i++ )
    {
        m_Keys[i]->SaveData( Igf );
    }
}

void ctrl_key::LoadData( igfmgr& Igf )
{
    // start with the basics
    m_nFloats =     Igf.GetS32( "Floats" );
    m_SerialNum =   Igf.GetS32( "SerNum" );
    m_InType =      (out_of_range)Igf.GetS32( "InType" );
    m_OutType =     (out_of_range)Igf.GetS32( "OutType");
    
    if ( Igf.Find("Scalar") )
        m_Scalar = Igf.GetF32();
    else
        m_Scalar = 1.0f;
    
    // boost the current serial number if you can
    s_SerialNum = MAX( s_SerialNum, m_SerialNum );

    // load the keys
    s32 Count =     Igf.GetS32( "Count"  );
    s32 i;

    hfield KeyGrp = Igf.Find( "Key" );

    for ( i = 0; i < Count; i++ )
    {
        Igf.EnterGroup( KeyGrp );
        key *pNewKey = new key( m_nFloats );
        pNewKey->LoadData( Igf );
        
        // add the key to this controller
        SetValue( pNewKey->GetKeyTime(), pNewKey );

        // next key
        Igf.ExitGroup();
        KeyGrp = Igf.Next();
    }
}

//============================================================================

void ctrl_key::ExportData( export::fx_controllerhdr& ContHdr,
                           xstring&                  Type,
                           xbytestream&              Stream )
{

}



//============================================================================
// THE LINEAR KEYFRAME CONTROLLER

controller* ctrl_linear::CopyOf( void ) const
{
    ctrl_linear* pNew = new ctrl_linear;

    // set the smooth value to be the same
    pNew->m_IsSmooth = m_IsSmooth;

    pNew->SetNumFloats( m_nFloats );
    pNew->m_InType = m_InType;
    pNew->m_OutType = m_OutType;
    pNew->m_Scalar = m_Scalar;

    // duplicate all my keys
    s32 i, j;
    j = m_Keys.GetCount();

    for ( i = 0; i < j; i++ )
    {
        key* pKey =     m_Keys.GetAt(i);
        key* pNewKey =  pKey->CopyOf();

        pNew->SetValue( pKey->GetKeyTime(), pNewKey );
    }

    return pNew;
}

controller* ctrl_smooth::CopyOf( void ) const
{
    return ctrl_key::CopyOf();
}

//============================================================================
// Get a linearly-interpolated value (returns true if T is a keyframe)
xbool ctrl_linear::GetValue( f32 T, f32* pVals ) const
{
    s32                         i, j;
    s32                         Range;
    s32                         FirstKeyTime, LastKeyTime;
    key*                        pFirst;
    key*                        pLast;
    controller::out_of_range    InType, OutType;

    j = m_Keys.GetCount();

    // Handle the extremeties as first-chance exit opportunity
    if ( j )
    {
        // if there's only one key, then CLAMP is the only option
        if ( j == 1 )
        {
            InType  = controller::CLAMP;
            OutType = controller::CLAMP;
        }
        else
        {
            InType  = m_InType;
            OutType = m_OutType;
        }

        // Get the the first and last key...for use in out-of-range calculation
        pFirst          = m_Keys[0];
        pLast           = m_Keys[ m_Keys.GetCount() - 1 ];

        FirstKeyTime    = pFirst->GetKeyTime();
        LastKeyTime     = pLast->GetKeyTime();

        // Get the time range
        Range   = LastKeyTime - FirstKeyTime;

        //=========================================
        // CALCULATE CORRECTED TIME BELOW KEY RANGE
        //=========================================
        if ( (T < m_Keys[0]->GetKeyTime()) || (j==1) ) 
        {
            switch ( InType )
            {
                case controller::CLAMP:
                {
                    x_memmove( pVals, m_Keys[0]->GetKeyValue(), m_nFloats * sizeof(f32) );
                    return TRUE;
                }

                case controller::LOOP:
                {
                    f32 NewT = x_fmod( T - (f32)FirstKeyTime, (f32)Range );

                    // This is a hack so the editor shows first frame on start frame of range
                    // This does not affect playback....just how it looks when you pick that exact frame
                    if( NewT == 0.0f )
                    {
                        NewT = -(f32)Range;
                    }

                    return GetValue( (f32)LastKeyTime + NewT, pVals );
                }

                case controller::PINGPONG:
                {
                    f32 NewT    = x_fmod( T - (f32)FirstKeyTime, (f32)Range );
                    s32 Cycle   = s32( ( T - (f32)FirstKeyTime ) / (f32)Range );

                    if ( Cycle % 2 ) // Ping
                        return GetValue( (f32)LastKeyTime  + NewT, pVals );
                    else            // Pong
                        return GetValue( (f32)FirstKeyTime - NewT, pVals );
                }
            }
        }
        else
        //=========================================
        // CALCULATE CORRECTED TIME ABOVE KEY RANGE
        //=========================================
        if ( T > m_Keys[j-1]->GetKeyTime() )
        {
            switch ( OutType )
            {
                case controller::CLAMP:
                {
                    x_memmove( pVals, m_Keys[j-1]->GetKeyValue(), m_nFloats * sizeof(f32) );
                    return TRUE;
                }

                case controller::LOOP:
                {
                    f32 NewT = x_fmod( T - (f32)LastKeyTime, (f32)Range );

                    // This is a hack so the editor shows last frame on end frame of range
                    // This does not affect playback....just how it looks when you pick that exact frame
                    if( NewT == 0.0f )
                    {
                        NewT = (f32)Range;
                    }

                    return GetValue( (f32)FirstKeyTime + NewT, pVals );
                }

                case controller::PINGPONG:
                {
                    // We subtract 1 for NewT because 1 frame after the range is an offset of 0 into the anim
                    f32 NewT    = x_fmod( T - (f32)LastKeyTime, (f32)Range );
                    s32 Cycle   = s32( ( T - (f32)LastKeyTime ) / (f32)Range );

                    if ( Cycle % 2 ) // Ping
                        return GetValue( (f32)FirstKeyTime + NewT, pVals );
                    else             // Pong
                        return GetValue( (f32)LastKeyTime  - NewT, pVals );

                }
            }
        }

    }
    else
        return FALSE;

    //=========================================
    // INTERPOLATE
    //=========================================
    for ( i = 0; i < j; i++ )
    {
        // If we're on the target key, just use those key values directly
        if ( m_Keys[i]->GetKeyTime() == T )
        {
            // return the straight value, no need to interpolate
            x_memmove( pVals, m_Keys[i]->GetKeyValue(), m_nFloats * sizeof(f32) );
            return TRUE;
        }
        else
        if ( i < (j-1) )
        {
            // Get the time values for this key and the next key
            s32 KeyTime1    = m_Keys[i]->GetKeyTime();
            s32 KeyTime2    = m_Keys[i+1]->GetKeyTime();

            if ( (T > KeyTime1) && (T < KeyTime2) )
            {
                // Calculate normalized time between the two keys (ie 0.0 = KeyTime1 AND 1.0 = KeyTime2 )
                f32 NormTime    = (f32)( T - KeyTime1 ) / ( KeyTime2 - KeyTime1 );

                // Get the values of the two keys
                f32*    pVal1   = m_Keys[i]->GetKeyValue();
                f32*    pVal2   = m_Keys[i+1]->GetKeyValue();

                if( m_IsSmooth )
                {
                    //-----------------------------------------------------------------------------
                    // Smooth hermite interpolation (Real Time Rendering 2nd Ed - pg 492)
                    //-----------------------------------------------------------------------------

                    // Get the values of the keys on each side of the two we're checking...for tangent calculations
                    f32*    pValPrev;
                    f32*    pValNext;

                    if( i == 0 )    { pValPrev = pVal1;                      }
                    else            { pValPrev = m_Keys[i-1]->GetKeyValue(); }

                    if( i+2 == j )  { pValNext = pVal2;                      }
                    else            { pValNext = m_Keys[i+2]->GetKeyValue(); }

                    // Pre-Calculate powers of NormTime
                    f32 t2  = NormTime * NormTime;
                    f32 t3  = NormTime * t2;

                    // Pre-Calculate Hermite spline terms
                    f32 ValTerm1    = ( 2*t3) - (3*t2) + 1;
                    f32 ValTerm2    = (-2*t3) + (3*t2);
                    f32 TanTerm1    = t3 - (2*t2) + NormTime;
                    f32 TanTerm2    = t3 - t2;

                    // Get tangency info for each key
                    f32* pTangent1  = new f32[ m_nFloats ];
                    f32* pTangent2  = new f32[ m_nFloats ];

                    GetTangent( i,   pTangent1 );
                    GetTangent( i+1, pTangent2 );

                    // Calculate the final values
                    for ( s32 k = 0; k < m_nFloats; k++ )
                    {
                        pVals[k]        =   ( pVal1[k]     * ValTerm1 ) + ( pVal2[k]     * ValTerm2 )
                                          + ( pTangent1[k] * TanTerm1 ) + ( pTangent2[k] * TanTerm2 );
                    }

                    delete[] pTangent1;
                    delete[] pTangent2;
                }
                else
                {
                    //-----------------------------------------------------------------------------
                    // Linear interpolation
                    //-----------------------------------------------------------------------------

                    for ( s32 k = 0; k < m_nFloats; k++ )
                    {
                        pVals[k] = pVal1[k] + ( (pVal2[k] - pVal1[k]) * NormTime );
                    }
                }

                return TRUE;
            }
        }
    }

    return FALSE;
}

//============================================================================
void ctrl_linear::ExportData( export::fx_controllerhdr& ContHdr,
                              xstring&                  Type,
                              xbytestream&              Stream )
{

    if ( m_IsSmooth )
        Type.Format( "SMOOTH KEY" );
    else
        Type.Format( "LINEAR KEY" );

    s32             i, Count;
    xbytestream     TmpStream;
    key*            FirstKey;
    key*            LastKey;
    s32             FirstKeyTime, LastKeyTime;
    s32             Channels[30];

    // figure out which channels are good to export
    ContHdr.NOutputValues = GetOptimalKeyCount( Channels );

    TmpStream.Clear();

    GetBookends( &FirstKey, &LastKey );
    Count = m_Keys.GetCount();

    FirstKeyTime = FirstKey->GetKeyTime();
    LastKeyTime  = LastKey ->GetKeyTime();

    ContHdr.DataBegin = FirstKeyTime / 30.0f;
    ContHdr.DataEnd   = LastKeyTime  / 30.0f;
    ContHdr.LeadIn    = m_InType;
    ContHdr.LeadOut   = m_OutType;
    //ContHdr.NOutputValues = GetNumFloats();
        
    TmpStream.Append( (const u8*)(&ContHdr), sizeof(export::fx_controllerhdr) );
    
    // add the number of keys
    TmpStream.Append( (const u8*)(&Count), sizeof(s32) );

    //---------------------------------------------------------------------

    // write the times first
    for ( i = 0; i < m_Keys.GetCount(); i++ )
    {
        s32 KeyTime = m_Keys[i]->GetKeyTime();
        f32 FT = MAX( 0.0f, (f32)(KeyTime - FirstKeyTime) / (LastKeyTime - FirstKeyTime) );  
        TmpStream.Append( (const byte*)&FT, sizeof(f32) );
    }

    // write the values
    for ( s32 j = 0; j < GetNumFloats(); j++ )
    {
        if ( Channels[j] )
        {
            for ( i = 0; i < m_Keys.GetCount(); i++ )
            {
                key*    pKey =  m_Keys[i];
                f32*    pData = pKey->GetKeyValue();
                s32     T =     pKey->GetKeyTime();
                f32     TmpVal;

                TmpVal = pData[j] * m_Scalar;

                TmpStream.Append( (const byte*)&(TmpVal), sizeof(f32) );

                // if smooth, add in the velocity data
                if ( m_IsSmooth )
                {
                    f32* pTangent;
                    pTangent    = new f32[ GetNumFloats() ];
                    GetTangent( i, pTangent );

                    TmpVal = pTangent[j] * m_Scalar;

                    TmpStream.Append( (const byte*)&(TmpVal), sizeof(f32) );

                    delete[] pTangent;
                }
            }
        }
    }

    ContHdr.TotalSize = TmpStream.GetLength() / sizeof(s32);
    TmpStream.Replace( 0, (const u8*)&ContHdr, sizeof(export::fx_controllerhdr) );
    Stream.Append( TmpStream );

}

//============================================================================
void ctrl_linear::GetTangent( s32 KeyIndex, f32* pVals ) const
{
    s32 NumKeys = m_Keys.GetCount();

    // We shouldn't try to get a tangent if the controller is not in "smooth" mode
    ASSERT( m_IsSmooth );

    // Make sure the KeyIndex is in valid range
    ASSERT( KeyIndex >= 0 );
    ASSERT( KeyIndex < NumKeys );

    // If the start & end key values are "equal", then we want to "weld" them to smooth the tangency
    xbool   WeldKeys    = true;

    f32*    pStart  = m_Keys[0]->GetKeyValue();
    f32*    pEnd    = m_Keys[NumKeys-1]->GetKeyValue();

    for ( s32 k = 0; k < m_nFloats; k++ )
    {
        if( x_abs( pStart[k] - pEnd[k] ) > 0.001f )
        {
            WeldKeys = false;
            break;
        }
   }

    // Figure out which keys to use for our "next" and "previous" keys
    s32 iPrev;
    s32 iNext;

    if( KeyIndex == 0 )
    {
        if( WeldKeys )  { iPrev = NumKeys - 2;  }   // Second to last key
        else            { iPrev = 0;            }   // First key
    }
    else                { iPrev = KeyIndex - 1; }   // Previous key

    if( KeyIndex == (NumKeys - 1) )
    {
        if( WeldKeys)   { iNext = 1;            }   // Second key
        else            { iNext = NumKeys - 1;  }   // Last key
    }
    else                { iNext = KeyIndex + 1; }   // Next key

    // Get the values of the two keys
    f32*    pValPrev    = m_Keys[iPrev]->GetKeyValue();
    f32*    pValNext    = m_Keys[iNext]->GetKeyValue();

    // Calculate the tangent values (Chord from prev to next key divided by 2..3dsMax uses 6?!)
    for( s32 i = 0; i < GetNumFloats(); i++ )
    {
        pVals[i]    = ( pValNext[i] - pValPrev[i] ) * 0.5f;
    }
}

//============================================================================
void ctrl_linear::LoadData  ( igfmgr& Igf )
{
    if ( Igf.Find("Smooth") )
        m_IsSmooth = Igf.GetBool();
    
    ctrl_key::LoadData( Igf );
}

//============================================================================
void ctrl_linear::SaveData  ( igfmgr& Igf ) const
{
    hfield  CtrlGrp;

    CtrlGrp = Igf.AddGroup( "Controller" );
    Igf.EnterGroup( CtrlGrp );
    {    
        ctrl_key::SaveData( Igf );

        Igf.AddString( "Type", GetType() );
        Igf.AddBool( "Smooth", m_IsSmooth );
        
        Igf.ExitGroup();
    }
}

//============================================================================
s32 ctrl_linear::GetNumFloats( void ) const
{
    return m_nFloats;
}

//============================================================================
// THE smooth KEYFRAME CONTROLLER

s32 ctrl_smooth::GetNumFloats( void ) const
{
    return m_nFloats;
}

void ctrl_smooth::SaveData    ( igfmgr& fp ) const
{
    // write the stuff

}

void ctrl_smooth::LoadData    ( igfmgr& fp )
{
}

void ctrl_smooth::ExportData( export::fx_controllerhdr& ContHdr,
                           xstring&                  Type,
                           xbytestream&              Stream )
{
    ctrl_key::ExportData( ContHdr, Type, Stream );
}

//============================================================================
// Get a smooth-interpolated value
xbool ctrl_smooth::GetValue( f32 T, f32* pVals ) const
{
    return TRUE;
}

//============================================================================

s32 controller::OutOfRangeType_FromString( const char* pString )
{
         if( x_strstr( pString, "Clamp"    ) )      { return CLAMP;    }
    else if( x_strstr( pString, "Loop"     ) )      { return LOOP;     }
    else if( x_strstr( pString, "PingPong" ) )      { return PINGPONG; }

    ASSERT( 0 );
    return 0;
}

//============================================================================

const char* controller::OutOfRangeType_ToString( s32 OutOfRangeType )
{
    switch( OutOfRangeType )
    {
        case controller::CLAMP:         { return "Clamp";    break; }
        case controller::LOOP:          { return "Loop";     break; }
        case controller::PINGPONG:      { return "PingPong"; break; }
    }

    ASSERT( 0 );
    return "";
}

} // namespace fx_core
