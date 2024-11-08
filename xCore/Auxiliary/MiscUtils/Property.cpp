#include "Property.hpp"
#include "Parsing\TextIn.hpp"
#include "Parsing\TextOut.hpp"
#include <stdio.h>

//=========================================================================
// FUNCTIONS
//=========================================================================

extern const char*  k_EnumEndStringConst    = "LAST_ENUM_STRING_90210";
extern const s32    k_MaxEnumInTable        = 10000;
char                g_EnumStringOut[255]    = {0};

prop_query              g_PropQuery;
xarray<prop_container>  g_PropContainer;
prop_enum               g_PropEnum;

//=========================================================================

void prop_query::ParseString( const char* pString )
{
    s32 i,j;

    m_nIndices = 0;

    // Find the length of the string
    for( j=i=0; pString[j]; i++, j++ )
    {
        u8  C = pString[j];
        m_PropName[i] = C;

        // Decode array entry. Take the number out as well
        if( C == '[' )
        {
            s32 Total=0;

            // Decode the number
            while( (C = pString[++j]) != ']' )
            {
                ASSERT( (C >= '0') && (C <= '9') );

                // Accumulate digit.
                Total = (10 * Total) + (C - '0');
            }

            // Roll back j
            j--;

            // Set the number 
            ASSERT( m_nIndices < 16 );
            ASSERT( i != j ); // This usually happens if the string pass has no number in the [12] such []
            m_Index[ m_nIndices++ ] = Total;
        }
    }

    // make the end char
    m_PropName[i] = 0;
    m_PropNameLen = i;
}

//=========================================================================

#if defined( X_EDITOR )

void prop_interface::OnSave( text_out& TextOut )
{
    prop_enum       List;
    prop_query      Query;
    s32             i;
    s32             nProp=0;

    // First lests enumerate all the properties
    OnEnumProp( List );

    // Do a quick pass and find out how many properties are we going to save
    for( i=0; i<List.GetCount(); i++ )
    {
        if( List[i].GetType() & PROP_TYPE_DONT_SAVE )
            continue;

        if( List[i].GetType() & PROP_TYPE_READ_ONLY )
            continue;

        if( List[i].GetType() & PROP_TYPE_HEADER )
            continue;

        if( (List[i].GetType() & PROP_TYPE_BASIC_MASK) == PROP_TYPE_BUTTON )
            continue;

// we need to save this now for renderInst
//        if( (List[i].GetType() & PROP_TYPE_BASIC_MASK) == PROP_TYPE_EXTERNAL )
//            continue;

        nProp++;
    }

    // Okay now lets go for it
    TextOut.AddHeader( "Properties", nProp );
    for( i=0; i<List.GetCount(); i++ )
    {
        if( List[i].GetType() & PROP_TYPE_DONT_SAVE )
            continue;

        if( List[i].GetType() & PROP_TYPE_READ_ONLY )
            continue;

        if( List[i].GetType() & PROP_TYPE_HEADER )
            continue;

        if( (List[i].GetType() & PROP_TYPE_BASIC_MASK) == PROP_TYPE_BUTTON )
            continue;

        TextOut.AddField( "Name:s", List[i].GetName() );

        switch( List[i].GetType() & PROP_TYPE_BASIC_MASK )
        {
        case PROP_TYPE_FLOAT    :
            {
                f32 Float;

                TextOut.AddField( "Type:s", "FLOAT" );

                OnProperty( Query.RQueryFloat( List[i].GetName(), Float ) );

                TextOut.AddField( "Value:s", (const char*)xfs("%g", Float) );
                break;
            }
        case PROP_TYPE_INT      :
            {
                s32 Int;

                TextOut.AddField( "Type:s", "INT" );

                OnProperty( Query.RQueryInt( List[i].GetName(), Int ) );

                TextOut.AddField( "Value:s", (const char*)xfs("%d", Int) );
                break;
            }
        case PROP_TYPE_BOOL     :
            {
                xbool Bool;

                TextOut.AddField( "Type:s", "BOOL" );

                OnProperty( Query.RQueryBool( List[i].GetName(), Bool ) );

                TextOut.AddField( "Value:s", (const char*)xfs("%d", Bool) );
                break;
            }
        case PROP_TYPE_VECTOR2  :
            {
                vector2 Vector2;
                
                TextOut.AddField( "Type:s", "VECTOR2" );

                OnProperty( Query.RQueryVector2( List[i].GetName(), Vector2 ) );

                TextOut.AddField( "Value:s", (const char*)xfs("%f %f", Vector2.X, Vector2.Y ) );
                break;
            }
        case PROP_TYPE_VECTOR3  :  
            {
                vector3 Vector3;

                TextOut.AddField( "Type:s", "VECTOR3" );

                OnProperty( Query.RQueryVector3( List[i].GetName(), Vector3 ) );

                TextOut.AddField( "Value:s", (const char*)xfs("%f %f %f", Vector3.GetX(), Vector3.GetY(), Vector3.GetZ() ) );
                break;
            }
        case PROP_TYPE_ROTATION :
            {
                radian3 Rotation;

                TextOut.AddField( "Type:s", "ROTATION" );

                OnProperty( Query.RQueryRotation( List[i].GetName(), Rotation ) );

                TextOut.AddField( "Value:s", (const char*)xfs("%f %f %f", 
                    RAD_TO_DEG(Rotation.Roll), 
                    RAD_TO_DEG(Rotation.Pitch), 
                    RAD_TO_DEG(Rotation.Yaw) ) );
                break;
            }
        case PROP_TYPE_ANGLE    :
            {
                radian Angle;

                TextOut.AddField( "Type:s", "ANGLE" );

                OnProperty( Query.RQueryAngle( List[i].GetName(), Angle ) );

                TextOut.AddField( "Value:s", (const char*)xfs("%g", RAD_TO_DEG(Angle) ) );
                break;
            }
        case PROP_TYPE_BBOX     :
            {
                bbox BBox;

                TextOut.AddField( "Type:s", "BBOX" );

                OnProperty( Query.RQueryBBox( List[i].GetName(), BBox ) );

                TextOut.AddField( "Value:s", (const char*)xfs("%g %g %g %g %g %g", 
                    BBox.Min.GetX(), BBox.Min.GetY(), BBox.Min.GetZ(),
                    BBox.Max.GetX(), BBox.Max.GetY(), BBox.Max.GetZ()) );
                break;
            }
        case PROP_TYPE_GUID     :
            {
                guid Guid;

                TextOut.AddField( "Type:s", "GUID" );

                OnProperty( Query.RQueryGUID( List[i].GetName(), Guid ) );

                TextOut.AddField( "Value:s", (const char*)xfs("%d %d", 
                    (s32)Guid.Guid, (s32)(Guid.Guid>>32) ));
                break;
            }
        case PROP_TYPE_COLOR    :
            {
                xcolor Color;
                const f32 K = 1/255.0f;

                TextOut.AddField( "Type:s", "COLOR" );

                OnProperty( Query.RQueryColor( List[i].GetName(), Color ) );

                TextOut.AddField( "Value:s", (const char*)xfs("%f %f %f %f", 
                    Color.R*K, Color.G*K, Color.B*K, Color.A*K ) );
                break;
            }
        case PROP_TYPE_STRING   :
            {
                char String[256];
                String[0] = 0;
                TextOut.AddField( "Type:s", "STRING" );

                OnProperty( Query.RQueryString( List[i].GetName(), String ) );

                TextOut.AddField( "Value:s", String );
                break;
            }
        case PROP_TYPE_ENUM     :
            {
                char Enum[256];
                Enum[0] = 0;

                TextOut.AddField( "Type:s", "ENUM" );

                OnProperty( Query.RQueryEnum( List[i].GetName(), Enum ) );

                TextOut.AddField( "Value:s", Enum );
                break;
            }
        case PROP_TYPE_BUTTON   :
            {
                ASSERT(0);
                break;   
            }
        case PROP_TYPE_EXTERNAL :
            {
                char Enum[256];
                Enum[0] = 0;

                TextOut.AddField( "Type:s", "EXTERNAL" );

                OnProperty( Query.RQueryExternal( List[i].GetName(), Enum ) );

                TextOut.AddField( "Value:s", Enum );
                break;   
            }
        case PROP_TYPE_FILENAME :
            {
                char FileName[256];
                FileName[0] = 0;

                TextOut.AddField( "Type:s", "FILENAME" );

                OnProperty( Query.RQueryFileName( List[i].GetName(), FileName ) );

                TextOut.AddField( "Value:s", FileName );
                break;
            }
        }

        // End of line
        TextOut.AddEndLine();
    }
}

//=========================================================================

void prop_interface::OnSave( const char* pFileName )
{
    text_out TextOut;

    TextOut.OpenFile( pFileName );
    x_try;
    OnSave( TextOut );
    x_catch_begin;
    TextOut.CloseFile();
    x_catch_end_ret;

    TextOut.CloseFile();
}

#endif // defined( X_EDITOR )

//=========================================================================

void prop_interface::OnLoad( text_in& TextIn )
{
    //prop_container     Prop;
    s32                i;

    TextIn.ReadHeader();

    if( x_strcmp( TextIn.GetHeaderName(), "Properties" ) != 0 )
        x_throw( xfs("Unable to load the properties in the file.\nCould not find the right header TextIn %s",TextIn.GetFileName()) );

    // Okay now lets go for it
    s32 Count = TextIn.GetHeaderCount();
    for( i=0; i<Count; i++ )
    {
        TextIn.ReadFields();

        char Name [256];
        char Type [256];
        char Value[256];
        TextIn.GetField( "Name:s",  Name   );
        TextIn.GetField( "Type:s",  Type   );
        TextIn.GetField( "Value:s", Value  );

        if( x_strcmp( Type, "FLOAT") == 0 )
        {
            f32 Float = x_atof( Value );
            OnProperty( g_PropQuery.WQueryFloat( Name, Float ) );
        }
        else if( x_strcmp( Type, "INT") == 0 )
        {
            s32 Int = x_atoi( Value );
            OnProperty( g_PropQuery.WQueryInt( Name, Int ) );
        }
        else if( x_strcmp( Type, "BOOL") == 0 )
        {
            xbool Bool = x_atoi( Value );
            OnProperty( g_PropQuery.WQueryBool( Name, Bool ) );
        }
        else if( x_strcmp( Type, "VECTOR2" ) == 0 )
        {
            vector2 Vector2;
            sscanf( Value, "%f %f", &Vector2.X, &Vector2.Y );
            OnProperty( g_PropQuery.WQueryVector2( Name, Vector2 ) );
        }
        else if( x_strcmp( Type, "VECTOR3") == 0 )
        {
            vector3 Vector3;
            sscanf( Value, "%f %f %f", &Vector3.GetX(), &Vector3.GetY(), &Vector3.GetZ() );
            OnProperty( g_PropQuery.WQueryVector3( Name, Vector3 ) );
        }
        else if( x_strcmp( Type, "ROTATION") == 0 )
        {
            radian3 Rotation;
            sscanf( Value, "%f %f %f", &Rotation.Roll, &Rotation.Pitch, &Rotation.Yaw );

            Rotation.Roll  = DEG_TO_RAD(Rotation.Roll);
            Rotation.Pitch = DEG_TO_RAD(Rotation.Pitch);
            Rotation.Yaw   = DEG_TO_RAD(Rotation.Yaw);

            OnProperty( g_PropQuery.WQueryRotation( Name, Rotation ) );
        }
        else if( x_strcmp( Type, "ANGLE") == 0 )
        {
            radian Angle = x_atof( Value );
            Angle        = DEG_TO_RAD(Angle);
            OnProperty( g_PropQuery.WQueryAngle( Name, Angle ) );
        }
        else if( x_strcmp( Type, "BBOX") == 0 )
        {
            bbox BBox;
            sscanf( Value, "%f %f %f %f %f %f", 
                &BBox.Min.GetX(), &BBox.Min.GetY(), &BBox.Min.GetZ(),
                &BBox.Max.GetX(), &BBox.Max.GetY(), &BBox.Max.GetZ() );
            OnProperty( g_PropQuery.WQueryBBox( Name, BBox ) );
        }
        else if( x_strcmp( Type, "GUID") == 0 )
        {
            s32 A, B;
            guid Guid;
            sscanf( Value, "%d %d", &A, &B );
            Guid.Guid = ((u32)A) | (((u64)B)<<32);
            OnProperty( g_PropQuery.WQueryGUID( Name, Guid ) );
        }
        else if( x_strcmp( Type, "COLOR") == 0 )
        {
            f32 R, G, B, A;
            xcolor Color;
            sscanf( Value, "%f %f %f %f", &R, &G, &B, &A );

            Color.SetfRGBA( R, G, B, A );

            OnProperty( g_PropQuery.WQueryColor( Name, Color ) );
        }
        else if( x_strcmp( Type, "STRING") == 0 )
        {
            OnProperty( g_PropQuery.WQueryString( Name, Value ) );
        }
        else if( x_strcmp( Type, "ENUM") == 0 )
        {
            OnProperty( g_PropQuery.WQueryEnum( Name, Value ) );
        }
        else if( x_strcmp( Type, "EXTERNAL") == 0 )
        {
            OnProperty( g_PropQuery.WQueryExternal( Name, Value ) );
        }
        else if( x_strcmp( Type, "FILENAME") == 0 )
        {
            OnProperty( g_PropQuery.WQueryFileName( Name, Value ) );
        }
        else
        {
        //  ASSERT( FALSE );
        }
    }
}

//=========================================================================

void prop_interface::OnLoad( const char* pFileName )
{
    text_in TextIn;

    TextIn.OpenFile( pFileName );
    x_try;
        OnLoad( TextIn );
    x_catch_begin;
        TextIn.CloseFile();
    x_catch_end_ret;

    TextIn.CloseFile();
}

//=========================================================================

void prop_interface::OnCopy( xarray<prop_container>& Container )
{
    prop_enum          List;
    prop_query         Query;
    s32                i;
    s32                nItems = 0;

    // First lets count all the properties
    prop_enum_counter Counter;
    OnEnumProp( Counter );

    // Now lets enumerate all the properties
    List.SetCapacity( Counter.GetCount() );
    OnEnumProp( List );

    // Do a quick pass and find out how many properties are we going to save
    for( i=0; i<List.GetCount(); i++ )
    {
        if( List[i].GetType() & (PROP_TYPE_DONT_COPY | PROP_TYPE_HEADER | PROP_TYPE_READ_ONLY) )
            continue;

        if( (List[i].GetType() & PROP_TYPE_BASIC_MASK) == PROP_TYPE_BUTTON )
            continue;

        nItems++;
    }

    // Set capacity!
    Container.SetCapacity( nItems );

    // Now do the actual copy
    for( i=0; i<List.GetCount(); i++ )
    {
        if( List[i].GetType() & (PROP_TYPE_DONT_COPY | PROP_TYPE_HEADER | PROP_TYPE_READ_ONLY) )
            continue;

        if( (List[i].GetType() & PROP_TYPE_BASIC_MASK) == PROP_TYPE_BUTTON )
            continue;

        //
        // Do the actual copy
        //
        prop_container& Cont = Container.Append();
        Cont.InitPropEnum( List[i] );
        OnProperty( Query.RQuery( Cont ) );        
    }
}

//=========================================================================

void prop_interface::OnPaste( const xarray<prop_container>& Container )
{
    prop_query         Query;

    // Do a quick pass and find out how many properties are we going to save
    for( s32 i=0; i<Container.GetCount(); i++ )
    {
        OnProperty( Query.WQuery( Container[i] ) );        
    }
}

//=========================================================================
//=========================================================================
//=========================================================================
#ifndef MAKE_PROP_QUERY_INLINE
//=========================================================================
//=========================================================================
//=========================================================================
            prop_query::prop_query    ( void ) { m_pData = NULL; m_RootPath[0]=0; m_RootLength=0;}

prop_type   prop_query::GetQueryType  ( void )            const { return ClearType(m_Type); }
prop_type   prop_query::ClearType     ( u32        Type ) const { return (prop_type)(Type&PROP_TYPE_BASIC_MASK); }
prop_type   prop_query::ClearType     ( prop_type  Type ) const { return (prop_type)(((u32)Type)&PROP_TYPE_BASIC_MASK); }

prop_query& prop_query::RQueryFloat   ( const char* pName, f32&        Data ) { GenericQuery( TRUE, PROP_TYPE_FLOAT,    pName, Data ); return *this; }
prop_query& prop_query::RQueryInt     ( const char* pName, s32&        Data ) { GenericQuery( TRUE, PROP_TYPE_INT,      pName, Data ); return *this; }
prop_query& prop_query::RQueryBool    ( const char* pName, xbool&      Data ) { GenericQuery( TRUE, PROP_TYPE_BOOL,     pName, Data ); return *this; }
prop_query& prop_query::RQueryGUID    ( const char* pName, guid&       Data ) { GenericQuery( TRUE, PROP_TYPE_GUID,     pName, Data ); return *this; }
prop_query& prop_query::RQueryVector2 ( const char* pName, vector2&    Data ) { GenericQuery( TRUE, PROP_TYPE_VECTOR2,  pName, Data ); return *this; }
prop_query& prop_query::RQueryVector3 ( const char* pName, vector3&    Data ) { GenericQuery( TRUE, PROP_TYPE_VECTOR3,  pName, Data ); return *this; }
prop_query& prop_query::RQueryBBox    ( const char* pName, bbox&       Data ) { GenericQuery( TRUE, PROP_TYPE_BBOX,     pName, Data ); return *this; }
prop_query& prop_query::RQueryAngle   ( const char* pName, radian&     Data ) { GenericQuery( TRUE, PROP_TYPE_ANGLE,    pName, Data ); return *this; }
prop_query& prop_query::RQueryRotation( const char* pName, radian3&    Data ) { GenericQuery( TRUE, PROP_TYPE_ROTATION, pName, Data ); return *this; }
prop_query& prop_query::RQueryColor   ( const char* pName, xcolor&     Data ) { GenericQuery( TRUE, PROP_TYPE_COLOR,    pName, Data ); return *this; }
prop_query& prop_query::RQueryString  ( const char* pName, char*       Data ) { RStringQuery(       PROP_TYPE_STRING,   pName, Data ); return *this; }
prop_query& prop_query::RQueryEnum    ( const char* pName, char*       Data ) { RStringQuery(       PROP_TYPE_ENUM,     pName, Data ); return *this; }
prop_query& prop_query::RQueryExternal( const char* pName, char*       Data ) { RStringQuery(       PROP_TYPE_EXTERNAL, pName, Data ); return *this; }
prop_query& prop_query::RQueryFileName( const char* pName, char*       Data ) { RStringQuery(       PROP_TYPE_FILENAME, pName, Data ); return *this; }
prop_query& prop_query::RQueryButton  ( const char* pName, char*       Data ) { RStringQuery(       PROP_TYPE_BUTTON,   pName, Data ); return *this; }

prop_query& prop_query::WQueryFloat   ( const char* pName, const f32&        Data ) { GenericQuery( FALSE, PROP_TYPE_FLOAT,    pName, Data ); return *this; }
prop_query& prop_query::WQueryInt     ( const char* pName, const s32&        Data ) { GenericQuery( FALSE, PROP_TYPE_INT,      pName, Data ); return *this; }
prop_query& prop_query::WQueryBool    ( const char* pName, const xbool&      Data ) { GenericQuery( FALSE, PROP_TYPE_BOOL,     pName, Data ); return *this; }
prop_query& prop_query::WQueryGUID    ( const char* pName, const guid&       Data ) { GenericQuery( FALSE, PROP_TYPE_GUID,     pName, Data ); return *this; }
prop_query& prop_query::WQueryVector2 ( const char* pName, const vector2&    Data ) { GenericQuery( FALSE, PROP_TYPE_VECTOR2,  pName, Data ); return *this; }
prop_query& prop_query::WQueryVector3 ( const char* pName, const vector3&    Data ) { GenericQuery( FALSE, PROP_TYPE_VECTOR3,  pName, Data ); return *this; }
prop_query& prop_query::WQueryBBox    ( const char* pName, const bbox&       Data ) { GenericQuery( FALSE, PROP_TYPE_BBOX,     pName, Data ); return *this; }
prop_query& prop_query::WQueryAngle   ( const char* pName, const radian&     Data ) { GenericQuery( FALSE, PROP_TYPE_ANGLE,    pName, Data ); return *this; }
prop_query& prop_query::WQueryRotation( const char* pName, const radian3&    Data ) { GenericQuery( FALSE, PROP_TYPE_ROTATION, pName, Data ); return *this; }
prop_query& prop_query::WQueryColor   ( const char* pName, const xcolor&     Data ) { GenericQuery( FALSE, PROP_TYPE_COLOR,    pName, Data ); return *this; }
prop_query& prop_query::WQueryString  ( const char* pName, const char*       Data ) { WStringQuery(        PROP_TYPE_STRING,   pName, Data ); return *this; }
prop_query& prop_query::WQueryEnum    ( const char* pName, const char*       Data ) { WStringQuery(        PROP_TYPE_ENUM,     pName, Data ); return *this; }
prop_query& prop_query::WQueryExternal( const char* pName, const char*       Data ) { WStringQuery(        PROP_TYPE_EXTERNAL, pName, Data ); return *this; }
prop_query& prop_query::WQueryFileName( const char* pName, const char*       Data ) { WStringQuery(        PROP_TYPE_FILENAME, pName, Data ); return *this; }
prop_query& prop_query::WQueryButton  ( const char* pName, const char*       Data ) { WStringQuery(        PROP_TYPE_BUTTON,   pName, Data ); return *this; }

xbool prop_query::VarFloat     ( const char* pPropName, f32&     Data, f32    Min, f32    Max ){ if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_FLOAT, Data, Min, Max ); m_MinFloat = Min; m_MaxFloat = Max; return TRUE; }
xbool prop_query::VarInt       ( const char* pPropName, s32&     Data, s32    Min, s32    Max ){ if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_INT,   Data, Min, Max ); m_MinInt   = Min; m_MaxInt   = Max; return TRUE; }
xbool prop_query::VarAngle     ( const char* pPropName, radian&  Data, radian Min, radian Max ){ if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_ANGLE, Data, Min, Max ); m_MinFloat = Min; m_MaxFloat = Max; return TRUE; }
xbool prop_query::VarBool      ( const char* pPropName, xbool&   Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_BOOL,  Data, 0,   1   ); return TRUE; }
xbool prop_query::VarGUID      ( const char* pPropName, guid&    Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_GUID,     Data ); return TRUE; }
xbool prop_query::VarVector2   ( const char* pPropName, vector2& Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_VECTOR2,  Data ); return TRUE; }
xbool prop_query::VarVector3   ( const char* pPropName, vector3& Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_VECTOR3,  Data ); return TRUE; }
xbool prop_query::VarBBox      ( const char* pPropName, bbox&    Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_BBOX,     Data ); return TRUE; }
xbool prop_query::VarRotation  ( const char* pPropName, radian3& Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_ROTATION, Data ); return TRUE; }
xbool prop_query::VarColor     ( const char* pPropName, xcolor&  Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVar( PROP_TYPE_COLOR,    Data ); return TRUE; }
xbool prop_query::VarString    ( const char* pPropName, char*    Data, s32 MaxStrLen )         { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVarString( Data, MaxStrLen ); return TRUE; }
xbool prop_query::VarEnum      ( const char* pPropName, char*    Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVarString( Data, 256 );       return TRUE; }
xbool prop_query::VarExternal  ( const char* pPropName, char*    Data, s32 MaxStrLen )         { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVarString( Data, MaxStrLen ); return TRUE; }
xbool prop_query::VarFileName  ( const char* pPropName, char*    Data, s32 MaxStrLen )         { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVarString( Data, MaxStrLen ); return TRUE; }
xbool prop_query::VarButton    ( const char* pPropName, char*    Data )                        { if( IsVar( pPropName ) == FALSE ) return FALSE; GenericVarString( Data, 256 );       return TRUE; }

f32     prop_query::GetVarFloat   ( f32    MinVal, f32    MaxVal ) { f32     Data; GetGenericVar( PROP_TYPE_FLOAT,    Data, MinVal, MaxVal ); m_MinFloat = MinVal; m_MaxFloat = MaxVal; return Data; }
s32     prop_query::GetVarInt     ( s32    MinVal, s32    MaxVal ) { s32     Data; GetGenericVar( PROP_TYPE_INT,      Data, MinVal, MaxVal ); m_MinInt   = MinVal; m_MaxInt   = MaxVal; return Data; }
radian  prop_query::GetVarAngle   ( radian MinVal, radian MaxVal ) { radian  Data; GetGenericVar( PROP_TYPE_ANGLE,    Data, MinVal, MaxVal ); m_MinFloat = MinVal; m_MaxFloat = MaxVal; return Data; }
xbool   prop_query::GetVarBool    ( void )                         { xbool   Data; GetGenericVar( PROP_TYPE_BOOL,     Data ); return Data; }
guid    prop_query::GetVarGUID    ( void )                         { guid    Data; GetGenericVar( PROP_TYPE_GUID,     Data ); return Data; }
vector2 prop_query::GetVarVector2 ( void )                         { vector2 Data; GetGenericVar( PROP_TYPE_VECTOR2,  Data ); return Data; }
vector3 prop_query::GetVarVector3 ( void )                         { vector3 Data; GetGenericVar( PROP_TYPE_VECTOR3,  Data ); return Data; }
bbox    prop_query::GetVarBBox    ( void )                         { bbox    Data; GetGenericVar( PROP_TYPE_BBOX,     Data ); return Data; }
radian3 prop_query::GetVarRotation( void )                         { radian3 Data; GetGenericVar( PROP_TYPE_ROTATION, Data ); return Data; }
xcolor  prop_query::GetVarColor   ( void )                         { xcolor  Data; GetGenericVar( PROP_TYPE_COLOR,    Data ); return Data; }
const char* prop_query::GetVarFileName( void ) { ASSERT( PROP_TYPE_FILENAME == GetQueryType() ); return (char*)m_pData; }
const char* prop_query::GetVarString  ( void ) { ASSERT( PROP_TYPE_STRING   == GetQueryType() ); return (char*)m_pData; }
const char* prop_query::GetVarEnum    ( void ) { ASSERT( PROP_TYPE_ENUM     == GetQueryType() ); return (char*)m_pData; }
const char* prop_query::GetVarExternal( void ) { ASSERT( PROP_TYPE_EXTERNAL == GetQueryType() ); return (char*)m_pData; }

void prop_query::SetVarFloat   ( f32            Data ) { SetGenericVar( PROP_TYPE_FLOAT,    Data ); }
void prop_query::SetVarInt     ( s32            Data ) { SetGenericVar( PROP_TYPE_INT,      Data ); }
void prop_query::SetVarBool    ( xbool          Data ) { SetGenericVar( PROP_TYPE_BOOL,     Data ); }
void prop_query::SetVarGUID    ( guid           Data ) { SetGenericVar( PROP_TYPE_GUID,     Data ); }
void prop_query::SetVarVector2 ( const vector2& Data ) { SetGenericVar( PROP_TYPE_VECTOR2,  Data ); }
void prop_query::SetVarVector3 ( const vector3& Data ) { SetGenericVar( PROP_TYPE_VECTOR3,  Data ); }
void prop_query::SetVarBBox    ( const bbox&    Data ) { SetGenericVar( PROP_TYPE_BBOX,     Data ); }
void prop_query::SetVarAngle   ( radian         Data ) { SetGenericVar( PROP_TYPE_ANGLE,    Data ); }
void prop_query::SetVarRotation( const radian3& Data ) { SetGenericVar( PROP_TYPE_ROTATION, Data ); }
void prop_query::SetVarColor   ( xcolor         Data ) { SetGenericVar( PROP_TYPE_COLOR,    Data ); }
void prop_query::SetVarFileName( const char*   pData, s32 MaxStrLen ) { ASSERT( PROP_TYPE_FILENAME == GetQueryType() ); x_strsavecpy( (char*)m_pData, pData, MaxStrLen ); m_MaxStringLength = MaxStrLen; }
void prop_query::SetVarString  ( const char*   pData, s32 MaxStrLen ) { ASSERT( PROP_TYPE_STRING   == GetQueryType() ); x_strsavecpy( (char*)m_pData, pData, MaxStrLen ); m_MaxStringLength = MaxStrLen; }
void prop_query::SetVarButton  ( const char*   pData )                { ASSERT( PROP_TYPE_BUTTON   == GetQueryType() ); x_strsavecpy( (char*)m_pData, pData, 256       ); m_MaxStringLength = 256;       }
void prop_query::SetVarEnum    ( const char*   pData )                { ASSERT( PROP_TYPE_ENUM     == GetQueryType() ); x_strsavecpy( (char*)m_pData, pData, 256       ); m_MaxStringLength = 256;       }
void prop_query::SetVarExternal( const char*   pData, s32 MaxStrLen ) { ASSERT( PROP_TYPE_EXTERNAL == GetQueryType() ); x_strsavecpy( (char*)m_pData, pData, MaxStrLen ); m_MaxStringLength = MaxStrLen; }

//=========================================================================

s32 prop_query::PushPath( const char* pRootPath )
{
    s32 i;
    s32 OldID = m_RootLength;

    for( i=0; pRootPath[i]; i++ )
    {
        // Can we push it or not?
        // TODO: We can only push a path if this property also has that path
        //       so for right now we will fail to push it but restoring the length.
        //       In reality should return a -1 or something like that.
        if( m_PropName[m_RootLength+i] != pRootPath[i] )
        {
            m_RootLength = OldID;
            return OldID;
        }

        m_RootPath[m_RootLength+i]=pRootPath[i];
    }

    ASSERT( i<128 );
    m_RootLength += i;
    m_RootPath[m_RootLength]=0;
    
    return OldID;
}

//=========================================================================

void prop_query::PopPath( s32 iPath )
{
    m_RootLength = iPath;
    m_RootPath[m_RootLength]=0;
}

//=========================================================================

xbool prop_query::IsVar( const char* pString )
{
    // Super fast string compare
    for( s32 i=m_PropNameLen; i>=m_RootLength; --i )
    {
        if( pString[i-m_RootLength] != m_PropName[i] ) return FALSE;
    }

    return TRUE;
}

//=========================================================================

xbool prop_query::IsSimilarPath( const char* pPath )
{
    return x_stristr( &m_PropName[m_RootLength], pPath ) != NULL;
}

//=========================================================================

xbool prop_query::IsBasePath( const char* pPath )
{
    const char* pStr1 = &m_PropName[m_RootLength];
    const char* pStr2 = pPath;
    ASSERT( pStr1 && pStr2 );

    s32 C1, C2;

    do
    {
        C1 = (s32)(*(pStr1++));
        if( (C1 >= 'A') && (C1 <= 'Z') )
            C1 -= ('A' - 'a');

        C2 = (s32)(*(pStr2++));
        if( (C2 >= 'A') && (C2 <= 'Z') )
            C2 -= ('A' - 'a');

    } while( C1 && C2 && (C1 == C2) );

    // If we made it to the end of pStr2 then we are good.
    if( C2 == 0 )
        return TRUE;

    return FALSE;
}

//=========================================================================


xbool prop_query::IsRead( void )
{
    return m_bRead;
}

//=========================================================================

s32 prop_query::GetIndex( s32 Number )
{
    ASSERT( Number >= 0 );
    if( Number >= m_nIndices )
        return 0;
    return m_Index[Number];
}

//=========================================================================

prop_query& prop_query::RQuery( prop_container& Container )
{
    ParseString( Container.GetName() );
    m_bRead       = TRUE;
    m_Type        = Container.GetType();
    m_DataSize    = Container.GetDataSize();
    m_pData       = Container.GetRawData();
    m_RootPath[0] = 0;
    m_RootLength  = 0;

    return *this;
}

//=========================================================================

prop_query& prop_query::RQuery( const char* pName, prop_container& Container )
{
    Container.SetName( pName );
    return RQuery( Container );
}

//=========================================================================

prop_query& prop_query::WQuery( const prop_container& Container )
{
    ParseString( Container.GetName() );
    m_bRead       = FALSE;
    m_Type        = Container.GetType();
    m_DataSize    = Container.GetDataSize();
    m_pData       = (void*)Container.GetRawData();
    m_RootPath[0] = 0;
    m_RootLength  = 0;

    return *this;
}
//=========================================================================
//=========================================================================
#endif //MAKE_PROP_QUERY_
//=========================================================================
//=========================================================================

//=========================================================================
//=========================================================================
#ifndef MAKE_PROP_CONTAINER_INLINE
//=========================================================================
//=========================================================================

prop_container::prop_container   ( void ) { m_Name[0]=0; m_Type=0; m_Data[0]=0x0; }

void prop_container::InitPropEnum( const prop_enum::node& EnumNode )
{
    x_strcpy( m_Name, EnumNode.GetName() );
    m_Type   = EnumNode.GetType();
};

void prop_container::InitFloat   ( const char* pName, const f32&      Data ) { InitGeneric( pName, PROP_TYPE_FLOAT,    (const void*)&Data ); }
void prop_container::InitBool    ( const char* pName, const xbool&    Data ) { InitGeneric( pName, PROP_TYPE_BOOL,     (const void*)&Data ); }
void prop_container::InitInt     ( const char* pName, const s32&      Data ) { InitGeneric( pName, PROP_TYPE_INT,      (const void*)&Data ); }
void prop_container::InitGUID    ( const char* pName, const guid&     Data ) { InitGeneric( pName, PROP_TYPE_GUID,     (const void*)&Data ); }
void prop_container::InitVector2 ( const char* pName, const vector2&  Data ) { InitGeneric( pName, PROP_TYPE_VECTOR2,  (const void*)&Data ); }
void prop_container::InitVector3 ( const char* pName, const vector3&  Data ) { InitGeneric( pName, PROP_TYPE_VECTOR3,  (const void*)&Data ); }
void prop_container::InitBBox    ( const char* pName, const bbox&     Data ) { InitGeneric( pName, PROP_TYPE_BBOX,     (const void*)&Data ); }
void prop_container::InitAngle   ( const char* pName, const radian&   Data ) { InitGeneric( pName, PROP_TYPE_ANGLE,    (const void*)&Data ); }
void prop_container::InitRotation( const char* pName, const radian3&  Data ) { InitGeneric( pName, PROP_TYPE_ROTATION, (const void*)&Data ); }
void prop_container::InitColor   ( const char* pName, const xcolor&   Data ) { InitGeneric( pName, PROP_TYPE_COLOR,    (const void*)&Data ); }
void prop_container::InitString  ( const char* pName, const char*     Data ) { InitGeneric( pName, PROP_TYPE_STRING,   (const void*)Data );  }
void prop_container::InitEnum    ( const char* pName, const char*     Data ) { InitGeneric( pName, PROP_TYPE_ENUM,     (const void*)Data );  }
void prop_container::InitExternal( const char* pName, const char*     Data ) { InitGeneric( pName, PROP_TYPE_EXTERNAL, (const void*)Data );  }

void prop_container::InitFileName( const char* pName, const char*     Data ) { InitGeneric( pName, PROP_TYPE_FILENAME, (const void*)Data );  }
void prop_container::InitButton  ( const char* pName, const char*     Data ) { InitGeneric( pName, PROP_TYPE_BUTTON,   (const void*)Data );  }

void prop_container::GetFloat    ( f32&        Data ) const { GetGeneric( (void*)&Data ); }
void prop_container::GetInt      ( s32&        Data ) const { GetGeneric( (void*)&Data ); }
void prop_container::GetGUID     ( guid&       Data ) const { GetGeneric( (void*)&Data ); }
void prop_container::GetBool     ( xbool&      Data ) const { GetGeneric( (void*)&Data ); }
void prop_container::GetVector2  ( vector2&    Data ) const { GetGeneric( (void*)&Data ); }
void prop_container::GetVector3  ( vector3&    Data ) const { GetGeneric( (void*)&Data ); }
void prop_container::GetBBox     ( bbox&       Data ) const { GetGeneric( (void*)&Data ); }
void prop_container::GetAngle    ( radian&     Data ) const { GetGeneric( (void*)&Data ); }
void prop_container::GetRotation ( radian3&    Data ) const { GetGeneric( (void*)&Data ); }
void prop_container::GetColor    ( xcolor&     Data ) const { GetGeneric( (void*)&Data ); }
void prop_container::GetString   ( char*       Data ) const { GetGeneric( (void*) Data ); }
void prop_container::GetEnum     ( char*       Data ) const { GetGeneric( (void*) Data ); }
void prop_container::GetExternal ( char*       Data ) const { GetGeneric( (void*) Data ); }
void prop_container::GetFileName ( char*       Data ) const { GetGeneric( (void*) Data ); }
void prop_container::GetButton   ( char*       Data ) const { GetGeneric( (void*) Data ); }

void prop_container::SetFloat    ( const f32&        Data ) { SetGeneric( (const void*)&Data ); }
void prop_container::SetInt      ( const s32&        Data ) { SetGeneric( (const void*)&Data ); }
void prop_container::SetGUID     ( const guid&       Data ) { SetGeneric( (const void*)&Data ); }
void prop_container::SetBool     ( const xbool&      Data ) { SetGeneric( (const void*)&Data ); }
void prop_container::SetVector2  ( const vector2&    Data ) { SetGeneric( (const void*)&Data ); }
void prop_container::SetVector3  ( const vector3&    Data ) { SetGeneric( (const void*)&Data ); }
void prop_container::SetBBox     ( const bbox&       Data ) { SetGeneric( (const void*)&Data ); }
void prop_container::SetAngle    ( const radian&     Data ) { SetGeneric( (const void*)&Data ); }
void prop_container::SetRotation ( const radian3&    Data ) { SetGeneric( (const void*)&Data ); }
void prop_container::SetColor    ( const xcolor&     Data ) { SetGeneric( (const void*)&Data ); }
void prop_container::SetString   ( const char*       Data ) { SetGeneric( (const void*) Data ); }
void prop_container::SetEnum     ( const char*       Data ) { SetGeneric( (const void*) Data ); }
void prop_container::SetExternal ( const char*       Data ) { SetGeneric( (const void*) Data ); }
void prop_container::SetFileName ( const char*       Data ) { SetGeneric( (const void*) Data ); }
void prop_container::SetButton   ( const char*       Data ) { SetGeneric( (const void*) Data ); }

const char* prop_container::GetName     ( void ) const { return m_Name; }
u32         prop_container::GetType     ( void ) const { return (m_Type  & PROP_TYPE_BASIC_MASK); }
u32         prop_container::GetTypeFlags( void ) const { return m_Type; }
void*       prop_container::GetRawData( void )       { return m_Data; }
void*       prop_container::GetRawData( void ) const { return (void*)m_Data; }
void        prop_container::SetName   ( const char* pName ) { ASSERT(pName); x_strcpy( m_Name, pName); }

//=========================================================================

s32 prop_container::GetDataSize( void ) const
{
    switch( m_Type & PROP_TYPE_BASIC_MASK )
    {
    case PROP_TYPE_FLOAT:       return sizeof(f32);
    case PROP_TYPE_VECTOR2:     return sizeof(vector2);
    case PROP_TYPE_VECTOR3:     return sizeof(vector3);
    case PROP_TYPE_INT:         return sizeof(s32);
    case PROP_TYPE_BOOL:        return sizeof(xbool);
    case PROP_TYPE_ROTATION:    return sizeof(radian3);
    case PROP_TYPE_ANGLE:       return sizeof(radian);
    case PROP_TYPE_BBOX:        return sizeof(bbox);
    case PROP_TYPE_GUID:        return sizeof(guid);
//    case PROP_TYPE_TRANSFORM:   return sizeof(matrix4);
    case PROP_TYPE_COLOR:       return sizeof(xcolor);
    case PROP_TYPE_FILENAME:    return x_strlen(m_Data);
    case PROP_TYPE_STRING:      return x_strlen(m_Data);
    case PROP_TYPE_ENUM:        return x_strlen(m_Data);
    case PROP_TYPE_BUTTON:      return x_strlen(m_Data);
    case PROP_TYPE_EXTERNAL:    return x_strlen(m_Data);
    default: 
        { 
            x_throw( "Internal error: Unkown property type" );
        }
    }

    return 0;
}

//=========================================================================

void prop_container::InitGeneric( const char* pName, prop_type Type, const void* pData )
{
    // Validate property name
    for( s32 i=0; (m_Name[i] = pName[i]); i++ )
    {
        if( i >= 126 ) 
            x_throw( xfs( "The property name [%s] is too long max 127 chars", pName));
    }

    // copy the type
    m_Type = Type;
    SetGeneric( pData );
}

//=========================================================================

void prop_container::SetGeneric( const void* pData )
{
    switch( m_Type & PROP_TYPE_BASIC_MASK )
    {
    case PROP_TYPE_FLOAT:       x_memmove( m_Data, pData, sizeof(f32) );         break;
    case PROP_TYPE_VECTOR2:     x_memmove( m_Data, pData, sizeof(vector2) );     break;
    case PROP_TYPE_VECTOR3:     x_memmove( m_Data, pData, sizeof(vector3) );     break;
    case PROP_TYPE_INT:         x_memmove( m_Data, pData, sizeof(s32) );         break;
    case PROP_TYPE_BOOL:        x_memmove( m_Data, pData, sizeof(xbool) );       break;
    case PROP_TYPE_ROTATION:    x_memmove( m_Data, pData, sizeof(radian3) );     break;
    case PROP_TYPE_ANGLE:       x_memmove( m_Data, pData, sizeof(radian) );      break;
    case PROP_TYPE_BBOX:        x_memmove( m_Data, pData, sizeof(bbox) );        break;
    case PROP_TYPE_GUID:        x_memmove( m_Data, pData, sizeof(guid) );        break;
//    case PROP_TYPE_TRANSFORM:   x_memmove( m_Data, pData, sizeof(matrix4) );     break;
    case PROP_TYPE_COLOR:       x_memmove( m_Data, pData, sizeof(xcolor) );      break;
    case PROP_TYPE_FILENAME:    x_strcpy( m_Data, (const char*)pData );         break;
    case PROP_TYPE_STRING:      x_strcpy( m_Data, (const char*)pData );         break;
    case PROP_TYPE_ENUM:        x_strcpy( m_Data, (const char*)pData );         break;
    case PROP_TYPE_EXTERNAL:    x_strcpy( m_Data, (const char*)pData );         break;
    case PROP_TYPE_BUTTON:      x_strcpy( m_Data, (const char*)pData );         break;
    default:
        x_throw( "Internal error: Unkown property type" );
    }
}

//=========================================================================

void prop_container::GetGeneric( void* pData ) const
{
    switch( m_Type & PROP_TYPE_BASIC_MASK )
    {
    case PROP_TYPE_FLOAT:       x_memmove( pData, m_Data, sizeof(f32) );         break;
    case PROP_TYPE_VECTOR2:     x_memmove( pData, m_Data, sizeof(vector2) );     break;
    case PROP_TYPE_VECTOR3:     x_memmove( pData, m_Data, sizeof(vector3) );     break;
    case PROP_TYPE_INT:         x_memmove( pData, m_Data, sizeof(s32) );         break;
    case PROP_TYPE_BOOL:        x_memmove( pData, m_Data, sizeof(xbool) );       break;
    case PROP_TYPE_ROTATION:    x_memmove( pData, m_Data, sizeof(radian3) );     break;
    case PROP_TYPE_ANGLE:       x_memmove( pData, m_Data, sizeof(radian) );      break;
    case PROP_TYPE_BBOX:        x_memmove( pData, m_Data, sizeof(bbox) );        break;
    case PROP_TYPE_GUID:        x_memmove( pData, m_Data, sizeof(guid) );        break;
//    case PROP_TYPE_TRANSFORM:   x_memmove( pData, m_Data, sizeof(matrix4) );     break;
    case PROP_TYPE_COLOR:       x_memmove( pData, m_Data, sizeof(xcolor) );      break;
    case PROP_TYPE_FILENAME:    x_strcpy( (char*)pData, m_Data ) ;              break;
    case PROP_TYPE_STRING:      x_strcpy( (char*)pData, m_Data ) ;              break;
    case PROP_TYPE_ENUM:        x_strcpy( (char*)pData, m_Data ) ;              break;
    case PROP_TYPE_EXTERNAL:    x_strcpy( (char*)pData, m_Data ) ;              break;
    case PROP_TYPE_BUTTON:      x_strcpy( (char*)pData, m_Data ) ;              break;
    default:
        x_throw( "Internal error: Unkown property type" );
    }
}
//=========================================================================
//=========================================================================
//=========================================================================
#endif //MAKE_PROP_CONTAINER_
//=========================================================================
//=========================================================================
//=========================================================================

//=========================================================================
//=========================================================================
//=========================================================================
#ifndef MAKE_PROP_ENUM_INLINE
//=========================================================================
//=========================================================================
//=========================================================================

const char* prop_enum::node::GetName       ( void ) const      { return m_Name; }
u32         prop_enum::node::GetType       ( void ) const      { return m_Type; }
const char* prop_enum::node::GetComment    ( void ) const      { return m_pComment; }
void        prop_enum::node::SetFlags      ( u32 Flags )       { m_Type |= Flags; }

prop_enum::node::node( void )
{
    //m_Enum[0] = 0;
    m_Name[0] = 0;
}

s32 prop_enum::PushPath( const char* pPath )
{
    ASSERT( pPath );
    s32 i;
    s32 Old = m_iRootPath;

    for( i=0; (m_RootPath[m_iRootPath+i] = pPath[i]); i++ )
    {
        ASSERT( i<256);
    }    

    m_iRootPath += i;
    return Old;
}

void prop_enum::PopPath( s32 iPath )
{
    m_iRootPath = iPath;
    m_RootPath[m_iRootPath]=0;
}


const char* prop_enum::GetRootPath( void )
{
    return m_RootPath;
}

prop_enum::prop_enum       ( void )
{
    m_RootPath[0]=0;
    m_iRootPath=0;
}

// ******
// RELEASE BUILD COMPILER ERROR
// 11/6/02 - Removed conditional statement in the following lines as they were
// causing the compiler to barf.
//

#ifdef USE_PROPERTY_HELP_STRINGS

void prop_enum::_PropEnumBool    ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_BOOL    ); }
void prop_enum::_PropEnumInt     ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_INT     ); }
void prop_enum::_PropEnumFloat   ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_FLOAT   ); }
void prop_enum::_PropEnumVector2 ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_VECTOR2 ); }
void prop_enum::_PropEnumVector3 ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_VECTOR3 ); }
void prop_enum::_PropEnumRotation( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_ROTATION); }
void prop_enum::_PropEnumAngle   ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_ANGLE   ); }
void prop_enum::_PropEnumBBox    ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_BBOX    ); }
void prop_enum::_PropEnumGuid    ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_GUID    ); }
void prop_enum::_PropEnumColor   ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_COLOR   ); }
void prop_enum::_PropEnumString  ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_STRING  ); }
void prop_enum::_PropEnumButton  ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_BUTTON  ); }
void prop_enum::_PropEnumHeader  ( const char* pName, const char* pComment, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_HEADER  ); }

#else

void prop_enum::_PropEnumBool    ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_BOOL    ); }
void prop_enum::_PropEnumInt     ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_INT     ); }
void prop_enum::_PropEnumFloat   ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_FLOAT   ); }
void prop_enum::_PropEnumVector2 ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_VECTOR2 ); }
void prop_enum::_PropEnumVector3 ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_VECTOR3 ); }
void prop_enum::_PropEnumRotation( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_ROTATION); }
void prop_enum::_PropEnumAngle   ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_ANGLE   ); }
void prop_enum::_PropEnumBBox    ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_BBOX    ); }
void prop_enum::_PropEnumGuid    ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_GUID    ); }
void prop_enum::_PropEnumColor   ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_COLOR   ); }
void prop_enum::_PropEnumString  ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_STRING  ); }
void prop_enum::_PropEnumButton  ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_BUTTON  ); }
void prop_enum::_PropEnumHeader  ( const char* pName, u32 Flags ) { m_lList.Append().Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_HEADER  ); }

#endif


s32              prop_enum::GetCount  ( void )       { return m_lList.GetCount(); }
void             prop_enum::Clear     ( void )       {        m_lList.Clear(); }
prop_enum::node& prop_enum::operator[]( s32 Index )  { return m_lList[Index]; }


s32 prop_enum::node::GetEnumCount( void ) const
{
    s32 nEnums = 0;
    for( s32 i=0; m_String[i]; i++ )
    {
        for( ; m_String[i]; i++ );
        nEnums++;
    }
    return nEnums;
}

const char* prop_enum::node::GetEnumType( s32 Index ) const
{
    s32 nEnums = 0;
    for( s32 i=0; m_String[i]; i++ )
    {
        if( nEnums == Index ) return &m_String[i];
        for( ; m_String[i]; i++ );
        nEnums++;
    }
    return NULL;
}

#ifdef USE_PROPERTY_HELP_STRINGS
void prop_enum::_PropEnumEnum    ( const char* pName, const char* pEnum, const char* pComment, u32 Flags )
#else
void prop_enum::_PropEnumEnum    ( const char* pName, const char* pEnum, u32 Flags )
#endif
{
    ASSERT( pEnum );

    node&   Node = m_lList.Append();

#ifdef USE_PROPERTY_HELP_STRINGS
    Node.Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_ENUM );
#else
    Node.Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_ENUM );
#endif

    s32 Len = 0;
    while( (pEnum[Len] != 0) || (pEnum[Len+1] != 0) )
        Len++;
    Node.m_String.SetLength( Len+1 );
    x_memmove( &Node.m_String[0], pEnum, Len+2 );
}

#ifdef USE_PROPERTY_HELP_STRINGS
void prop_enum::_PropEnumFileName( const char* pName, const char* pExt, const char* pComment, u32 Flags )
#else
void prop_enum::_PropEnumFileName( const char* pName, const char* pExt, u32 Flags )
#endif
{
    node& Node = m_lList.Append();

#ifdef USE_PROPERTY_HELP_STRINGS
    Node.Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_FILENAME );
#else
    Node.Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_FILENAME );
#endif
    ASSERT( pExt );
    ASSERT( x_strlen(pExt) < X_MAX_PATH );
    Node.m_String = pExt;
}

#ifdef USE_PROPERTY_HELP_STRINGS
void prop_enum::_PropEnumExternal( const char* pName, const char* TypeInfo, const char* pComment, u32 Flags )
#else
void prop_enum::_PropEnumExternal( const char* pName, const char* TypeInfo, u32 Flags )
#endif
{
    ASSERT( TypeInfo );

    node&   Node = m_lList.Append();

#ifdef USE_PROPERTY_HELP_STRINGS
    Node.Set( (const char*)xfs("%s%s",m_RootPath,pName), pComment, Flags|PROP_TYPE_EXTERNAL );
#else
    Node.Set( (const char*)xfs("%s%s",m_RootPath,pName), "", Flags|PROP_TYPE_EXTERNAL );
#endif

    s32 Len = 0;
    while( (TypeInfo[Len] != 0) || (TypeInfo[Len+1] != 0) )
        Len++;
    Node.m_String.SetLength( Len+1 );
    x_memmove( &Node.m_String[0], TypeInfo, Len+2 );
}

//=========================================================================


void prop_enum::node::Set( const char* pString, const char* pComment, u32 aFlags )
{
    ASSERT( pString );
    ASSERT( x_strlen( pString ) < 128 );
    x_strcpy( m_Name, pString );
    m_Type     = aFlags;
    m_pComment = pComment;
}

//=========================================================================
//=========================================================================
//=========================================================================
#endif// MAKE_PROP_ENUM_
//=========================================================================
//=========================================================================
//=========================================================================
