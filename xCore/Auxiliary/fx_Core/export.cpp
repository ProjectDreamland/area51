#include "x_bytestream.hpp"
#include "controller.hpp"
#include "element.hpp"
#include "effect.hpp"
#include "export.hpp"
#include "TextureMgr.hpp"
#include "errorlog.hpp"

extern fx_core::error_log g_ErrorLog;

namespace fx_core
{

const char EXPORT_VER[5]    = "FX02";

const char* GetExportVersion( void )
{
    return EXPORT_VER;
}

//==============================================================================

void SwapColorEndian( xcolor& Color )
{
    u8 B = Color.B;
    u8 G = Color.G;
    u8 R = Color.R;
    u8 A = Color.A;

    Color.B = A;
    Color.G = R;
    Color.R = G;
    Color.A = B;
}

//==============================================================================
// Build the data from the provided effect
void export::ConstructData( effect* pEffect, s32 ExportTarget )
{
    fx_datahdr  DataHdr;

    s32         i, PadData;

    // Flag all textures that are being used
    pEffect->FlagAllTextures();
    
    //=========================================================================
    // Create the Data Header

    DataHdr.MasterCopy  = -1;                           // unused field
    DataHdr.Flags       = 0;                            // init to zero
    DataHdr.NSAValues   = 0;                            // Fixed up later
    DataHdr.NElements   = pEffect->GetNumElements();
    DataHdr.NBitmaps    = g_pTextureMgr->GetBitmapCount();

    if ( pEffect->m_Instanceable )
        DataHdr.Flags |= ( 1 << 0 );

    // Figure out how many controllers have key values
    DataHdr.NControllers = pEffect->GetOptimalNumControllers();//pEffect->GetNumControllers();

    // Figure out how many elements are flagged for export
    DataHdr.NElements = 0;
    for( i=0 ; i<pEffect->GetNumElements() ; i++ )
    {
        element* pElem = pEffect->GetElement( i );
        if ( pElem->ShouldExport() )
            DataHdr.NElements++;
    }

    //-------------------------------------------------------------------------

    PadData = 0x43434343;
    for ( i = 0; i < DataHdr.NControllers; i++ )
        m_ExportData.Append( (const byte*)&PadData, sizeof(s32) );

    PadData = 0x45454545;
    for ( i = 0; i < DataHdr.NElements; i++ )
        m_ExportData.Append( (const byte*)&PadData, sizeof(s32) );

    PadData = 0x42424242;
    for ( i = 0; i < DataHdr.NBitmaps; i++ )
    {
        m_ExportData.Append( (const byte*)&PadData, sizeof(s32) );  // Diffuse
        m_ExportData.Append( (const byte*)&PadData, sizeof(s32) );  // Alpha
    }
    //=========================================================================
    

    //=========================================================================
    // Controller Data header and keys

    fx_controllerhdr ContHdr;
    s32 Offset = 0;

    //for( i=0 ; i<pEffect->GetNumControllers() ; i++ )
    for( i=0 ; i<pEffect->GetTotalNumControllers() ; i++ )
    {
        xstring Type;
        controller* pCont = pEffect->GetController( i );

        // only export controllers with keys
        if ( strcmp(pCont->GetType(), LINEAR_KEY) == 0 )
        {
            s32 Tmp[30];    // dummy storage for optimal key count call
            ctrl_key* pKeyController = (ctrl_key*)pCont;

            //if ( pKeyController->GetKeyCount() > 1 )
            if ( pKeyController->GetOptimalKeyCount(Tmp) )
            {
                // where the data is in the staging area
                ContHdr.OutputIndex = Offset;
                pCont->m_ExpIdx = Offset;
                Offset += pKeyController->GetOptimalKeyCount(Tmp);//pKeyController->GetNumFloats();

                // let the controller export its own self
                pKeyController->ExportData( ContHdr, Type, m_ExportData );

                // update data header
                DataHdr.NSAValues += ContHdr.NOutputValues;
 
                s32 Len = Type.GetLength() + 1;
                m_Additional.Append( (const byte*)((const char*)Type), Len );
            }
        }
    }
    //=========================================================================

    
    //=========================================================================
    // Element Data
    fx_elementhdr   ElemHdr;
    for( i=0 ; i<pEffect->GetNumElements() ; i++ )
    {
        xstring Type;
        element* pElem = pEffect->GetElement( i );

        // check the export flag for this element
        if ( pElem->ShouldExport() )
        {
            pElem->ExportData( ElemHdr, Type, m_ExportData, ExportTarget );
        
            s32 Len = Type.GetLength() + 1;
            m_Additional.Append( (const byte*)((const char*)Type), Len );
        }
    }
    //=========================================================================

    //=========================================================================
    // Bitmaps
    g_pTextureMgr->ExportNames( m_Additional, ExportTarget );
    //=========================================================================


    //=========================================================================    
    // Write out the string data
    // Starting with ALL textures
    
    DataHdr.TotalSize = (m_ExportData.GetLength() + sizeof(fx_datahdr)) / sizeof(s32);
    m_ExportData.Insert( 0, (const byte*)&DataHdr, sizeof(fx_datahdr) );

    s32 Magic;
    
    char* pMagic = (char*)&Magic;
    pMagic[0] = EXPORT_VER[0];
    pMagic[1] = EXPORT_VER[1];
    pMagic[2] = EXPORT_VER[2];
    pMagic[3] = EXPORT_VER[3];

    if( ExportTarget == EXPORT_TARGET_GCN )
        Magic = ENDIAN_SWAP_32( Magic );
    m_ExportData.Insert( 0, (const byte*)&Magic, sizeof(s32) );

    s32 StringLengths = m_Additional.GetLength();
    m_ExportData.Append( (const byte*)&StringLengths, sizeof(s32) );
    m_ExportData.Append( m_Additional );    
}

//==============================================================================
// Save the data
xbool export::SaveData( const char* pFilename, s32 ExportTarget )
{
    xbool Success = m_ExportData.SaveFile( pFilename );
    if( !Success )
        g_ErrorLog.Append( xfs("Error - Saving effect \"%s\"", pFilename) );

    // export the textures
    g_pTextureMgr->ExportXBMPs( pFilename, ExportTarget );

    return TRUE;
}

} // namespace fx_core
