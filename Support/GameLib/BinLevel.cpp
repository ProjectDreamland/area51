
#include "BinLevel.hpp"
#include <stdio.h>
#include "Auxiliary\MiscUtils\Property.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Objects\Render\RenderInst.hpp"
#include "UI\ui_manager.hpp"
#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/NetobjMgr.hpp"
#include "Configuration/GameConfig.hpp"
//=========================================================================
// VARIABLES
//=========================================================================
// HACKOMOTRON - need to stop creating player objects that are in the level export
u64         g_PlayerGuid = 0;

bin_level   g_BinLevelMgr;

//=========================================================================
// GENERAL FUNCTIONS
//=========================================================================

//=========================================================================

bin_level::bin_level( void )
{
#ifdef X_EDITOR
    m_bDirty = FALSE;
#endif // X_EDITOR
    NullData( TRUE );
    m_bWantsToSave = FALSE;
    m_bWantsToLoad = FALSE;
}

//=========================================================================

bin_level::~bin_level( void )
{
//    ClearData( TRUE );
}

//=========================================================================

void bin_level::NullData( xbool bClearDictionary )
{
    m_BitStream.Kill();    

    if( bClearDictionary )
    {
        m_pDictionary = NULL;
    }

    m_nObjects = 0;
    m_pObject = NULL;

    m_nProperties = 0;
    m_pProperties = NULL;

    m_bIsRuntimeDynamicData = FALSE;
}


//=========================================================================

void bin_level::ClearData( xbool bClearDictionary )
{
    //delete all arrays
    x_free( m_pObject );
    m_pObject = NULL;
    x_free( m_pProperties );
    m_pProperties = NULL;

#ifndef X_EDITOR
    //we only free this ourselves on PS2 side, since m_BitStream.Kill()
    //will free the data on PC side
    x_free( m_BitStream.GetDataPtr() );
#endif // X_EDITOR

    if( bClearDictionary )
    {
        if( m_pDictionary )
        {
            delete m_pDictionary;
            m_pDictionary = NULL;
        }
    }

    //set all members Null and empty
    NullData( bClearDictionary );
} 

//=============================================================================

xbool bin_level::LoadLevel( const char* levelName, const char* dictionaryName, const char* pLoadOrderName )
{
    MEMORY_OWNER("bin_level::LoadLevel()");
 
    PreloadDataFiles(pLoadOrderName);
    xtimer DeltaTime;
    xbool  SuppressNetObjects = FALSE;

    DeltaTime.Start();

    {
    //MEMORY_OWNER("_LoadData");
    if( !LoadData( levelName, dictionaryName ) )
        return FALSE;
    }

    if (!m_pDictionary)
        return FALSE;

#ifndef X_EDITOR
    // get current pct loaded
    f32 StartPct;
    StartPct = g_UiMgr->GetPercentLoaded();

    // calculate percent per object loaded
    f32 PctPerObject;
    PctPerObject = ( (95.0f - StartPct) / m_nObjects);

    if( g_NetworkMgr.IsClient() )
        SuppressNetObjects = TRUE;
#endif // X_EDITOR

    for (s32 i = 0; i < m_nObjects; i++)
    {
        //MEMORY_OWNER("_loop1_a");
//        m_BitStream.SetCursor( m_pObject[i].iStartBitStream );

        // HACKOMOTRON
        // The player object is exported in some levels - it shouldn't be!
        if( x_strcmp( m_pDictionary->GetString(m_pObject[i].TypeIndex), "Player" ) == 0)
        {
            // don't create the player object if we already have one
            if( g_PlayerGuid != 0 )
            {
                object* pObject = g_ObjMgr.GetObjectByGuid(g_PlayerGuid);
                if (pObject)
                {
                    //MEMORY_OWNER("_loop1_b");
                    pObject->LoadStart();
                    
                    //set all the objects properties
                    for (s32 k = m_pObject[i].iProperty; k < (m_pObject[i].iProperty + m_pObject[i].nProperty); k++)
                    {
                        //MEMORY_OWNER("_loop1_c");
                        AddPropertyToObject( m_pProperties[k], pObject);
                    }
                    
                    {
                    //MEMORY_OWNER("_loop1_d");
                    pObject->LoadEnd();
                    }

                    // player has been successfully restored
                    continue;
                } 
            }
            // store the guid of this player
            g_PlayerGuid = m_pObject[i].Guid;
        }
        // END HACKOMOTRON

        //now create the object
        guid ObjGuid = m_pObject[i].Guid;
        {
            //MEMORY_OWNER("_loop1_e");

            const char*        pTypeName = m_pDictionary->GetString( m_pObject[i].TypeIndex );
            const object_desc* pDesc     = g_ObjMgr.GetDescFromName( pTypeName );

            if( (pDesc->GetType() == object::TYPE_PICKUP) && !SuppressNetObjects )
            {
                // Create a net pickup
                g_ObjMgr.ReserveGuid( ObjGuid );
                CREATE_NET_OBJECT( *pDesc, netobj::TYPE_PICKUP );

#if !defined( X_EDITOR )
                ASSERT( g_NetworkMgr.IsServer() );
#endif
            }
            else
            {
                // Either not a pickup, or a suppressed pickup so just create it
                g_ObjMgr.CreateObject( *pDesc, ObjGuid );
            }
        }

        object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);
        if (pObject)
        {
            //MEMORY_OWNER("_loop2_a");
            pObject->LoadStart();
            
            //set all the objects properties
            for (s32 k = m_pObject[i].iProperty; k < (m_pObject[i].iProperty + m_pObject[i].nProperty); k++)
            {
                //MEMORY_OWNER("AddPropertyToObject()");
                AddPropertyToObject( m_pProperties[k], pObject);
            }
            {
                //MEMORY_OWNER("_loop2_c");
                pObject->LoadEnd();
            }
#if !defined( X_EDITOR )
            //
            // We don't really load the player, just allowing it so that the bitstream cursor
            // stays current. Now we'll drop the player.
            //
            if ( pObject->GetType() == object::TYPE_PLAYER )
            {
                g_ObjMgr.DestroyObject( ObjGuid );
                ASSERTS( FALSE, "We loaded a player, which should not have been exported, see mreed" );
            }

            // CJ: We don't really want the pickups on the client so nuke them now
            if( (pObject->GetType() == object::TYPE_PICKUP) && SuppressNetObjects )
            {
                g_ObjMgr.DestroyObject( ObjGuid );
            }
#endif
        }
        else
        {
            ASSERTS( 0, "Error loading couldn't create an object" );
        }

    }

#ifndef X_EDITOR
    // make sure the progress bar is showing 95% loaded now.
    //g_UiMgr->SetPercentLoaded( 95.0f );
#endif // X_EDITOR

    // We don't need this data anymore...
    ClearData( TRUE );

    return TRUE;
}

//=========================================================================

xbool bin_level::LoadData( const char* pFile, const char* pDictionary )
{
    MEMORY_OWNER( "bin_level::LoadData()" );

    //first clear any existing data
    ClearData( TRUE );

    X_FILE* fpTemplate;
    if( !(fpTemplate = x_fopen( pFile, "rb" )))
    {
        ASSERT(FALSE);
        return FALSE;
    }

    u16 uVersion = 0;
    s32 nBitStreamSize = 0;
    s32 nDictSize = 0;

    //read the version number
    x_fread( &uVersion , sizeof(u16), 1, fpTemplate );

    //read the dictionary size
    x_fread( &nDictSize , sizeof(s32), 1, fpTemplate );

    //read the object list size
    x_fread( &m_nObjects , sizeof(s32), 1, fpTemplate );

    //read the property list size
    x_fread( &m_nProperties , sizeof(s32), 1, fpTemplate );

    //read the bitstream size
    x_fread( &nBitStreamSize , sizeof(s32), 1, fpTemplate );

    //do mallocs
    //load object data
    m_pObject = (obj_entry*)x_malloc(sizeof(obj_entry)*m_nObjects);
    ASSERT(m_pObject);
    x_memset( m_pObject, 0, sizeof(obj_entry)*m_nObjects );

    //load property data
    m_pProperties = (prop_entry*)x_malloc(sizeof(prop_entry)*m_nProperties);
    ASSERT(m_pProperties);
    x_memset( m_pProperties, 0, sizeof(prop_entry)*m_nProperties );

    //read the object list data
    x_fread( m_pObject , sizeof(obj_entry), m_nObjects, fpTemplate );

    //read the property list data
    x_fread( m_pProperties , sizeof(prop_entry), m_nProperties, fpTemplate );

    //read the bitstream
    byte* pDataStream = (byte*)x_malloc(sizeof(byte)*nBitStreamSize);
    x_fread( pDataStream, sizeof(byte), nBitStreamSize, fpTemplate);  
    m_BitStream.Init(pDataStream, nBitStreamSize);

    //close file
    x_fclose( fpTemplate );

    {
        MEMORY_OWNER( "DICTIONARY" );
        X_FILE* fpDict;
        if( !(fpDict = x_fopen( pDictionary, "rb" )))
        {
            ASSERT(FALSE);
            return FALSE;
        }
        m_pDictionary = new dictionary;
        m_pDictionary->Load(fpDict, nDictSize);

        // mreed: ugly, but we need this in the dictionary since it's not being added through
        // export any more. GameApp's savegame looks for the entry in the dictionary because the
        // player's guid is dynamic...
        g_BinLevelMgr.m_pDictionary->Add( "Player" );


        x_fclose( fpDict );
    }

    return TRUE;
}

//=============================================================================

void bin_level::AddPropertyToObject( prop_entry& pe, object* pObject )
{
    MEMORY_OWNER( xfs( "OBJECT DATA '%s'", pObject->GetTypeDesc().GetTypeName() ) );
    prop_query pq;
    char       cData[256];

    switch(pe.PropType)
    {
    case PROP_TYPE_EXTERNAL:
        {
            MEMORY_OWNER_DETAIL("PROP_TYPE_EXTERNAL");
            m_BitStream.ReadString(cData);
            pq.WQueryExternal( m_pDictionary->GetString(pe.NameIndex), cData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_BUTTON:  
        {
            MEMORY_OWNER_DETAIL("PROP_TYPE_BUTTON");
            m_BitStream.ReadString(cData);
            pq.WQueryButton( m_pDictionary->GetString(pe.NameIndex), cData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_FILENAME:     
        {
            MEMORY_OWNER_DETAIL("PROP_TYPE_FILENAME");
            m_BitStream.ReadString(cData);
            pq.WQueryFileName( m_pDictionary->GetString(pe.NameIndex), cData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_ENUM:  
        {
            MEMORY_OWNER_DETAIL("PROP_TYPE_ENUM");
            m_BitStream.ReadString(cData);
            pq.WQueryEnum( m_pDictionary->GetString(pe.NameIndex), cData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_STRING:   
        {
            MEMORY_OWNER_DETAIL("PROP_TYPE_STRING");
            m_BitStream.ReadString(cData);
            pq.WQueryString( m_pDictionary->GetString(pe.NameIndex), cData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_FLOAT:  
        {
            MEMORY_OWNER_DETAIL("PROP_TYPE_FLOAT");
            f32 fData;
            m_BitStream.ReadF32(fData);
            pq.WQueryFloat( m_pDictionary->GetString(pe.NameIndex), fData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_INT:       
        {
            MEMORY_OWNER_DETAIL("PROP_TYPE_INT");
            s32 nData;
            m_BitStream.ReadS32(nData);
            pq.WQueryInt( m_pDictionary->GetString(pe.NameIndex), nData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_BOOL:
        {
            MEMORY_OWNER_DETAIL("PROP_TYPE_BOOL");
            xbool bData;
            bData = m_BitStream.ReadFlag();
            pq.WQueryBool( m_pDictionary->GetString(pe.NameIndex), bData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_VECTOR3:
        {
            MEMORY_OWNER_DETAIL("PROP_TYPE_VECTOR3");
            vector3 v3Data;
            m_BitStream.ReadVector(v3Data);
            pq.WQueryVector3( m_pDictionary->GetString(pe.NameIndex), v3Data);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_ANGLE:
        {
            MEMORY_OWNER_DETAIL("PROP_TYPE_ANGLE");
            radian rData;
            m_BitStream.ReadF32(rData);
            pq.WQueryAngle( m_pDictionary->GetString(pe.NameIndex), rData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_ROTATION:  			
        {
            MEMORY_OWNER_DETAIL("PROP_TYPE_ROTATION");
            radian3 r3Data;
            m_BitStream.ReadRadian3(r3Data);
            pq.WQueryRotation( m_pDictionary->GetString(pe.NameIndex), r3Data);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_BBOX:      
        {
            MEMORY_OWNER_DETAIL("PROP_TYPE_BBOX");
            bbox bbData;
            m_BitStream.ReadVector(bbData.Min);
            m_BitStream.ReadVector(bbData.Max);
            pq.WQueryBBox( m_pDictionary->GetString(pe.NameIndex), bbData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_COLOR:
        {
            MEMORY_OWNER_DETAIL("PROP_TYPE_COLOR");
            xcolor xcData;
            m_BitStream.ReadColor(xcData);
            pq.WQueryColor( m_pDictionary->GetString(pe.NameIndex), xcData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_GUID:     
        {
            MEMORY_OWNER_DETAIL("PROP_TYPE_GUID");
            u64 gData;
            m_BitStream.ReadU64(gData);
            pq.WQueryGUID( m_pDictionary->GetString(pe.NameIndex), gData);
            pObject->OnProperty(pq);
        }
	    break;
	}
}

//=============================================================================

xbool bin_level::SaveRuntimeDynamicData( void )
{    
    m_bWantsToSave = FALSE;

    //reset data streams
    ClearData( TRUE );
    
    m_pDictionary = new dictionary;
       
    //allocate bitstream
    m_BitStream.Init(128);
    
    xarray<guid> m_GuidArray;
    xarray<s32>  m_PropCount;
    
    m_nProperties = 0;

    //collect all existing dynamic objects
    for( object_desc* pNode = object_desc::GetFirstType(); 
                      pNode; 
                      pNode = object_desc::GetNextType( pNode )  )
    {        
        if( pNode->IsDynamic() )
        {            
            s32     iCount = 0;
            slot_id SlotID = g_ObjMgr.GetFirst( pNode->GetType() );
            
            while( SlotID != SLOT_NULL )
            {                
                object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );                
                SlotID = g_ObjMgr.GetNext( SlotID );
                
                ASSERT(pObject);

                //make sure we have a valid, non-persistent object that does not have the no_runtime save flag
                if( pObject &&  
                    ((pObject->GetAttrBits() & object::ATTR_NO_RUNTIME_SAVE) == 0) )
                {                    
                    if (x_strcmp(pObject->GetTypeDesc().GetTypeName(), pNode->GetTypeName()) == 0)
                    {                        
                        iCount++;
                        m_GuidArray.Append(pObject->GetGuid());
                        
                        xarray<prop_container> tempProperties;                        
                        pObject->OnCopy( tempProperties );                                             
                        m_nProperties += tempProperties.GetCount();
                        m_PropCount.Append(tempProperties.GetCount());                        
                    }
                }
            }

            if ( iCount > 0 )
            {
                x_DebugMsg("Runtime Data Saved %d Dynamic Objects of Type(%s)\n",iCount,pNode->GetTypeName());
            }
        }
    }
    
    m_nObjects = m_GuidArray.GetCount();
    
    //alloc object data
    m_pObject = (obj_entry*)x_malloc(sizeof(obj_entry)*m_nObjects);    
    ASSERT(m_pObject);
    x_memset( m_pObject, 0, sizeof(obj_entry)*m_nObjects );
    
    //alloc property data
    m_pProperties = (prop_entry*)x_malloc(sizeof(prop_entry)*m_nProperties);    
    ASSERT(m_pProperties);
    x_memset( m_pProperties, 0, sizeof(prop_entry)*m_nProperties );
    
    s32 currentPropertyIndex = 0;
    s32 currentObjectIndex = 0;

    for (s32 i = 0; i < m_GuidArray.GetCount(); i++)
    {        
        object* pObject = g_ObjMgr.GetObjectByGuid(m_GuidArray.GetAt(i));
        
        if (pObject)
        {            
            //save this object
            m_pObject[ currentObjectIndex].Guid      = pObject->GetGuid();            
            m_pObject[ currentObjectIndex].TypeIndex = m_pDictionary->Add( pObject->GetTypeDesc().GetTypeName() );            
            xarray<prop_container> ObjProperties;            
            pObject->OnCopy( ObjProperties );            
            ASSERT( ObjProperties.GetCount() == m_PropCount.GetAt(i));
            m_pObject[ currentObjectIndex ].nProperty = ObjProperties.GetCount( );            
            m_pObject[ currentObjectIndex ].iProperty = currentPropertyIndex;
            for(s32 propCount= 0; propCount < ObjProperties.GetCount(); propCount++ )
            {                
                ASSERT( currentPropertyIndex < m_nProperties );
                m_pProperties[ currentPropertyIndex  ].NameIndex = m_pDictionary->Add( ObjProperties.GetAt( propCount ).GetName() );
                m_pProperties[ currentPropertyIndex  ].PropType = ObjProperties.GetAt( propCount ).GetType();
                currentPropertyIndex++;
                AddDataToBitStream( ObjProperties.GetAt( propCount ) );                
            }

            currentObjectIndex++;
        }
    }

    if ( currentObjectIndex > 0 )
    {
        x_DebugMsg("Saved %d objects with %d properties total!\n",currentObjectIndex, currentPropertyIndex);
    }
    
    m_bIsRuntimeDynamicData = TRUE;
    
    return TRUE;
}

//=============================================================================

xbool bin_level::LoadRuntimeDynamicData( void )
{
    m_bWantsToLoad = FALSE;
    
    if (!m_bIsRuntimeDynamicData)
    {
        //no dynamic data stored
        return FALSE;
    }

    if (!m_pDictionary)
    {
        //no dictionary, serious problem
        return FALSE;
    }

    //delete all existing dynamic objects
    for( object_desc* pNode = object_desc::GetFirstType(); 
                      pNode; 
                      pNode = object_desc::GetNextType( pNode )  )
    {
        if( pNode->IsDynamic())
        {
            slot_id SlotID = g_ObjMgr.GetFirst( pNode->GetType() );
            while( SlotID != SLOT_NULL )
            {
                object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
                SlotID = g_ObjMgr.GetNext( SlotID );
                ASSERT(pObject);
                if( pObject )
                {
                    if (x_strcmp(pObject->GetTypeDesc().GetTypeName(), pNode->GetTypeName()) == 0)
                    {
                        g_ObjMgr.DestroyObjectEx(pObject->GetGuid(), TRUE);
                    }
                }   
            }   
        }
    }
        
    //reset the bitstream cursor
    m_BitStream.SetCursor( 0 );

    s32 nObjectsLoaded = 0;
    s32 nPropertiesLoaded = 0;
    for (s32 i = 0; i < m_nObjects; i++)
    {
        //create the object
        guid ObjGuid = m_pObject[i].Guid;
        g_ObjMgr.CreateObject( m_pDictionary->GetString(m_pObject[i].TypeIndex), m_pObject[i].Guid );

        object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);
        if (pObject)
        {
            pObject->LoadStart();
            
            //set all the objects properties
            for (s32 k = m_pObject[i].iProperty; k < (m_pObject[i].iProperty + m_pObject[i].nProperty); k++)
            {
                AddPropertyToObject( m_pProperties[k], pObject);
                nPropertiesLoaded++;
            }
            nObjectsLoaded++;
            
            pObject->LoadEnd();
        }        
    }
    
    if ( nObjectsLoaded > 0 )
    {
        x_DebugMsg("Loaded %d objects with %d properties total!\n",nObjectsLoaded, nPropertiesLoaded);
    }

    return TRUE;
}

//=============================================================================

void bin_level::SetRigidColor( const char* pFileName )
{
    // loop through all of the objects, and if the need to have rigid
    // colors associated with them, then do that now
    s32 i;
    for( i = 0; (object::type)i < object::TYPE_END_OF_LIST; i++ )
    {
        const object_desc* pDesc = g_ObjMgr.GetTypeDesc( (object::type)i );
        if( pDesc && pDesc->IsBurnVertexLighting() )
        {
            slot_id SlotID = g_ObjMgr.GetFirst( (object::type)i );
            while( SlotID != SLOT_NULL )
            {
                object*         pObject = g_ObjMgr.GetObjectBySlot( SlotID );
                render_inst*    pInst   = pObject->GetRenderInstPtr();

                if( pInst )
                    pInst->LoadColorTable( pFileName );

                SlotID = g_ObjMgr.GetNext( SlotID );
            }
        }
    }
}

//=============================================================================

void bin_level::PreloadDataFiles ( const char* pLoadOrderName )
{
    (void)pLoadOrderName;
    /*
    X_FILE* fpTemplate;
    if( !(fpTemplate = x_fopen( pLoadOrderName, "rb" ) ) )
    {
        ASSERT(FALSE);
        return;// FALSE;
    }
    */
}

//=========================================================================

void bin_level::AddDataToBitStream( prop_container& pc )
{
    
    switch(pc.GetType())
    {
    case PROP_TYPE_FILENAME:     
        {
            char cData[X_MAX_PATH];
            pc.GetFileName(cData);
            m_BitStream.WriteString(cData);
        }
	    break;
    case PROP_TYPE_EXTERNAL:
        {
            char cData[X_MAX_PATH];
            pc.GetExternal(cData);
            m_BitStream.WriteString(cData);
        }
	    break;
    case PROP_TYPE_BUTTON:  
        {
            char cData[X_MAX_PATH];
            pc.GetButton(cData);
            m_BitStream.WriteString(cData);
        }
	    break;    
    case PROP_TYPE_ENUM:  
        {
            
            char cData[X_MAX_PATH];
            pc.GetEnum(cData);
            m_BitStream.WriteString(cData);
            
        }
	    break;    
    case PROP_TYPE_STRING:   
        {
            char cData[X_MAX_PATH];
            pc.GetString(cData);
            m_BitStream.WriteString(cData);
        }
	    break;
    case PROP_TYPE_FLOAT:  
        {
            f32 fData;
            pc.GetFloat(fData);
            m_BitStream.WriteF32(fData);
        }
	    break;
    case PROP_TYPE_INT:       
        {
            s32 nData;
            pc.GetInt(nData);
            m_BitStream.WriteS32(nData);
        }
	    break;
    case PROP_TYPE_BOOL:
        {
            xbool bData;
            pc.GetInt(bData);
            m_BitStream.WriteFlag(bData);
        }
	    break;
    case PROP_TYPE_VECTOR3:
        {
            vector3 v3Data;
            pc.GetVector3(v3Data);
            m_BitStream.WriteVector(v3Data);
        }
	    break;
    case PROP_TYPE_ANGLE:
        {
            radian rData;
            pc.GetAngle(rData);
            m_BitStream.WriteF32(rData);
        }
	    break;
    case PROP_TYPE_ROTATION:  			
        {
            radian3 r3Data;
            pc.GetRotation(r3Data);
            m_BitStream.WriteRadian3(r3Data);
        }
	    break;
    case PROP_TYPE_BBOX:      
        {
            bbox bbData;
            pc.GetBBox(bbData);
            m_BitStream.WriteVector(bbData.Min);
            m_BitStream.WriteVector(bbData.Max);
        }
	    break;
    case PROP_TYPE_COLOR:
        {
            xcolor xcData;
            pc.GetColor(xcData);
            m_BitStream.WriteColor(xcData);
        }
	    break;
    case PROP_TYPE_GUID:     
        {
            guid gData;
            pc.GetGUID(gData);
            m_BitStream.WriteU64(gData);
        }
	    break;
	}
    
}

//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
#ifdef X_EDITOR
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================

//=========================================================================
// EDITOR STRUCTURES & CLASSES
//=========================================================================

//data elements used to generate Data Stream

struct EditorObjData
{
    char                    cType[X_MAX_PATH];
    xarray<prop_container>  Properties;
};

//=========================================================================


xbool bin_level::EditorCreateGameData(  xarray<guid> lstGuidsToExport  )
{
    //first clear any existing data
    ClearData( TRUE );

    m_pDictionary = new dictionary;
    
    //allocate bitstream
    m_BitStream.Init(128);

    m_nObjects = lstGuidsToExport.GetCount();
    //  Walk through the list of guids for objects to export
    s32 objectCount;
    for(objectCount = 0; objectCount < lstGuidsToExport.GetCount(); objectCount++)
    {
        object *tempObject = g_ObjMgr.GetObjectByGuid( lstGuidsToExport[objectCount] );
        if ( tempObject )
        {
            xarray<prop_container> tempProperties;
            tempObject->OnCopy( tempProperties );
            
            for( s32 t=0; t<tempProperties.GetCount(); t++ )
            {
                if( (tempProperties[t].GetTypeFlags() & PROP_TYPE_DONT_EXPORT) )
                {
                    tempProperties.Delete(t);
                    t--;
                }
            }

            m_nProperties += tempProperties.GetCount();
        }       
    }
    
    //alloc object data
    m_pObject = (obj_entry*)x_malloc(sizeof(obj_entry)*lstGuidsToExport.GetCount());
    ASSERT(m_pObject);
    x_memset( m_pObject, 0, sizeof(obj_entry)*lstGuidsToExport.GetCount() );

    //alloc property data
    m_pProperties = (prop_entry*)x_malloc(sizeof(prop_entry)*m_nProperties);
    ASSERT(m_pProperties);
    x_memset( m_pProperties, 0, sizeof(prop_entry)*m_nProperties );

    EditorObjData ObjData;

    //now we need to update the bitstream and dictionary and pointer arrays
    // time to walk through all the objects again 

    s32 currentPropertyIndex = 0;
    for(objectCount = 0; objectCount < lstGuidsToExport.GetCount(); objectCount++)
    {
        object *tempObject = g_ObjMgr.GetObjectByGuid( lstGuidsToExport[ objectCount ] );
        
        if ( tempObject )
        {
            m_pObject[ objectCount].Guid      = lstGuidsToExport[ objectCount ];
            m_pObject[ objectCount].TypeIndex = m_pDictionary->Add( tempObject->GetTypeDesc().GetTypeName() );
            xarray<prop_container> tempProperties;
            tempObject->OnCopy( tempProperties );

            for( s32 t=0; t<tempProperties.GetCount(); t++ )
            {
                if( (tempProperties[t].GetTypeFlags() & PROP_TYPE_DONT_EXPORT) )
                {
                    tempProperties.Delete(t);
                    t--;
                }
            }

            m_pObject[ objectCount ].nProperty = tempProperties.GetCount( );
            m_pObject[ objectCount ].iProperty = currentPropertyIndex;

            s32 propCount;
            for(propCount= 0; propCount < tempProperties.GetCount(); propCount++ )
            {
                m_pProperties[ currentPropertyIndex  ].NameIndex = m_pDictionary->Add( tempProperties.GetAt( propCount ).GetName() );
                m_pProperties[ currentPropertyIndex  ].PropType = tempProperties.GetAt( propCount ).GetType();
                currentPropertyIndex++;
                AddDataToBitStream( tempProperties.GetAt( propCount ) );
            }
        }
    }
    m_nProperties = currentPropertyIndex;
    return TRUE;
}

//=========================================================================

xbool bin_level::EditorSaveData( const char* pFile, const char* pDictionary, xarray<guid> lstGuidsToExport )
{
    x_try;


    EditorCreateGameData( lstGuidsToExport );

    X_FILE* fpDict;
    if( !(fpDict = x_fopen( pDictionary, "wb" )))
    {
        ASSERT(FALSE);
        return FALSE;
    }
    s32 nDictSize = m_pDictionary->Save(fpDict);
    x_fclose( fpDict );

    X_FILE* fpTemplate;
    if( !(fpTemplate = x_fopen( pFile, "wb" )))
    {
        ASSERT(FALSE);
        return FALSE;
    }

    u16 uVersion = 1;
    s32 nBitStreamSize = m_BitStream.GetNBytesUsed();

    //write the version number
    x_fwrite( &uVersion , sizeof(u16), 1, fpTemplate );

    //write the dictionary size
    x_fwrite( &nDictSize , sizeof(s32), 1, fpTemplate );

    //write the object list size
    x_fwrite( &m_nObjects , sizeof(s32), 1, fpTemplate );

    //write the property list size
    x_fwrite( &m_nProperties , sizeof(s32), 1, fpTemplate );

    //write the bitstream size
    x_fwrite( &nBitStreamSize , sizeof(s32), 1, fpTemplate );

    
    //read the object list data
    x_fwrite( m_pObject , sizeof(obj_entry), m_nObjects, fpTemplate );
    
    //read the property list data
    x_fwrite( m_pProperties , sizeof(prop_entry), m_nProperties, fpTemplate );
    
    //write the bitstream
    byte* pDataStream = m_BitStream.GetDataPtr();

    x_fwrite( pDataStream, sizeof(byte), nBitStreamSize + 1, fpTemplate);

    //close file
    x_fclose( fpTemplate );

    return TRUE;

    x_catch_display;

    return FALSE;
}

//-----------------// END EDITOR SPECIFIC CODE
#endif // X_EDITOR
//-----------------//






