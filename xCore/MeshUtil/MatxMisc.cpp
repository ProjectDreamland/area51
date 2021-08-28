
#include "MATXMisc.hpp"
#include "TextIn.hpp"

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

matx_misc::matx_misc( void )
{
    x_memset( this, 0, sizeof(*this) );
}

//=========================================================================

matx_misc::~matx_misc( void )
{    
    if(m_pXRef) delete[]m_pXRef;
    x_memset( this, 0, sizeof(*this) );
}


//=========================================================================

xbool Matx_HasHeader(const char *pFileName, const char *pHeader)
{
    text_in File;

    //
    // Open the file
    //
    File.OpenFile( pFileName );

    while( File.ReadHeader() == TRUE )
    {
#ifdef AUX_DEBUG_OUTPUT
        x_DebugMsg("Read header %s\n", File.GetHeaderName());
#endif
        if( x_stricmp( File.GetHeaderName(), pHeader ) == 0 )
        {
            return TRUE;
        }
        File.SkipToNextHeader();
    }
    return FALSE;
}

//=========================================================================

xbool matx_misc::Load( const char* pFileName )
{
    text_in File;

    //
    // Open the file
    //
    File.OpenFile( pFileName );

    while( File.ReadHeader() == TRUE )
    {
        if( x_stricmp( File.GetHeaderName(), "XRefs" ) == 0 )
        {
            m_nXRefs = File.GetHeaderCount();
    
            m_pXRef  = new xref[ m_nXRefs ];
            ASSERT( m_pXRef );
            x_memset( m_pXRef, 0, sizeof(xref)*m_nXRefs );

            // Read each of the fields in the file
            for( s32 i=0; i<File.GetHeaderCount(); i++ )
            {
                xref& XRef = m_pXRef[i];

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "ObjectName:s",  XRef.ObjectName);
                File.GetField( "FileName:s",    XRef.FileName);

                if( !File.GetField( "IsChild:d", &XRef.IsChild ) )
                    XRef.IsChild = FALSE;

                File.GetField( "Scale:fff",    &XRef.Scale.GetX(), &XRef.Scale.GetY(), &XRef.Scale.GetZ() );
                File.GetField( "Rot:ffff",     &XRef.Rotation.X, &XRef.Rotation.Y, &XRef.Rotation.Z, &XRef.Rotation.W );
                File.GetField( "Pos:fff",      &XRef.Position.GetX(), &XRef.Position.GetY(), &XRef.Position.GetZ() );
                File.GetField( "BBox:ffffff",  &XRef.BBox.Min.GetX(), &XRef.BBox.Min.GetY(), &XRef.BBox.Min.GetZ(),
                                               &XRef.BBox.Max.GetX(), &XRef.BBox.Max.GetY(), &XRef.BBox.Max.GetZ());
            }
        }
        else
        if( x_stricmp( File.GetHeaderName(), "Light" ) == 0 )
        {
            m_nLights = File.GetHeaderCount();
    
            m_pLight  = new light[ m_nLights ];
            ASSERT( m_pLight );
            x_memset( m_pLight, 0, sizeof(light)*m_nLights );

            // Read each of the fields in the file
            for( s32 i=0; i<File.GetHeaderCount(); i++ )
            {
                light& Light = m_pLight[i];

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "Name:s", Light.Name );
                File.GetField( "Type:d", &Light.Type );
                File.GetField( "Pos:fff",      &Light.Position.GetX(), &Light.Position.GetY(), &Light.Position.GetZ() );
                
                f32 C[4];
                File.GetField( "Color:ffff",   &C[0], &C[1], &C[2], &C[3] );
                Light.Color.R = (byte)(255*C[0]);
                Light.Color.G = (byte)(255*C[1]);
                Light.Color.B = (byte)(255*C[2]);
                Light.Color.A = (byte)(255*C[3]);

                File.GetField( "Intensity:f", &Light.Intensity );
                File.GetField( "AttenRange:ff", &Light.AttenuationStart, &Light.AttenuationEnd );
            }
        }
        else File.SkipToNextHeader();
    }

    //
    // Did we rich the end of file successfully?
    //
    if( File.IsEOF() == FALSE )
        return FALSE;

    return TRUE;
}

//=========================================================================
