
#include "TemplateMgr.hpp"
#include <stdio.h>
#include "Auxiliary\MiscUtils\Property.hpp"
#include "Parsing\TextIn.hpp"
#include "Parsing\TextOut.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/NetObj.hpp"

//=========================================================================

#ifdef cgalley
#define LOGGING_ENABLED 0
#else
#define LOGGING_ENABLED 0
#endif

//=========================================================================
// VARIABLES
//=========================================================================

static const u16 kTemplateFileVersion = 2;

template_mgr g_TemplateMgr;

//=========================================================================
// GENERAL FUNCTIONS
//=========================================================================

//=========================================================================

template_mgr::template_mgr( void ) :
m_pDictionary(NULL)
{
#ifdef X_EDITOR
    m_lstBlueprints.SetCapacity( 20 );
    m_lstBlueprints.SetGrowAmount( 20 );
#endif // X_EDITOR
    NullData( );

    // Some bounds for the xarrays.  May need to change these when weapons / etc are moved over
    // to template system.
}

//=========================================================================

template_mgr::~template_mgr( void )
{
//    ClearData( );
}

//=========================================================================

void template_mgr::NullData( )
{
    m_BitStream.Kill();    

    m_pDictionary = NULL;
    
    m_nTemplates = 0;
    m_pTemplate = NULL;

    m_nObjects = 0;
    m_pObject = NULL;

    m_nProperties = 0;
    m_pProperties = NULL;
}

//=========================================================================

void template_mgr::DisplayTemplates( void )
{
    x_DebugMsg("DisplayTemplates: %d\n",m_nTemplates);
    for( s32 i=0; i<m_nTemplates; i++ )
    {
        s32 NI = m_pTemplate[i].NameIndex;
        const char* pName = m_pDictionary->GetString(NI);
        x_DebugMsg("%3d] %s\n",i,pName);
        CLOG_MESSAGE( LOGGING_ENABLED, "template_mgr::DisplayTemplates", pName );
    }
}

//=========================================================================

void template_mgr::ClearData( )
{
    //delete all arrays
    x_free( m_pTemplate );
    x_free( m_pObject );
    x_free( m_pProperties );

    if (m_pDictionary)
    {
        delete m_pDictionary;
        m_pDictionary = NULL;
    }

    //set all members Null and empty
    NullData( );
}

//=========================================================================

xbool template_mgr::LoadData( const char* pFile, const char* pDictionary )
{
    MEMORY_OWNER( "template_mgr::LoadData()" );

    //first clear any existing data
    ClearData( );

    X_FILE* fpTemplate;
    if( !(fpTemplate = x_fopen( pFile, "rb" )))
    {
        ASSERT(FALSE);
        return FALSE;
    }

    u16 uVersion = kTemplateFileVersion;
    s32 nBitStreamSize = 0;
    s32 nDictSize = 0;

    //read the version number
    x_fread( &uVersion , sizeof(u16), 1, fpTemplate );
    if ( uVersion != kTemplateFileVersion )
    {
        x_throw( "Template file version doesn't match app." );
    }

    //read the dictionary size
    x_fread( &nDictSize , sizeof(s32), 1, fpTemplate );

    //read the template list size
    x_fread( &m_nTemplates , sizeof(s32), 1, fpTemplate );

    //read the object list size
    x_fread( &m_nObjects , sizeof(s32), 1, fpTemplate );

    //read the property list size
    x_fread( &m_nProperties , sizeof(s32), 1, fpTemplate );

    //read the bitstream size
    x_fread( &nBitStreamSize , sizeof(s32), 1, fpTemplate );

    //do mallocs

    //load template data
    m_pTemplate = (template_entry*)x_malloc(sizeof(template_entry)*m_nTemplates);
    ASSERT(m_pTemplate);
    x_memset( m_pTemplate, 0, sizeof(template_entry)*m_nTemplates );

    //load object data
    m_pObject = (obj_entry*)x_malloc(sizeof(obj_entry)*m_nObjects);
    ASSERT(m_pObject);
    x_memset( m_pObject, 0, sizeof(obj_entry)*m_nObjects );

    //load property data
    m_pProperties = (prop_entry*)x_malloc(sizeof(prop_entry)*m_nProperties);
    ASSERT(m_pProperties);
    x_memset( m_pProperties, 0, sizeof(prop_entry)*m_nProperties );

    //read the template list data
    x_fread( m_pTemplate , sizeof(template_entry), m_nTemplates, fpTemplate );

    //read the object list data
    x_fread( m_pObject , sizeof(obj_entry), m_nObjects, fpTemplate );

    //read the property list data
    x_fread( m_pProperties , sizeof(prop_entry), m_nProperties, fpTemplate );

    //read the bitstream
    byte* pDataStream = (byte*)x_malloc(sizeof(byte)*nBitStreamSize);
    x_fread( pDataStream, sizeof(byte), nBitStreamSize, fpTemplate);  
    m_BitStream.Init(pDataStream, nBitStreamSize);
    m_BitStream.SetOwnsData( TRUE );

    //close file
    x_fclose( fpTemplate );

    X_FILE* fpDict;
    if( !(fpDict = x_fopen( pDictionary, "rb" )))
    {
        ASSERT(FALSE);
        return FALSE;
    }
    m_pDictionary = new dictionary;
    m_pDictionary->Load(fpDict, nDictSize);
    x_fclose( fpDict );

    DisplayTemplates();

    return FALSE;
}

//=========================================================================

xbool template_mgr::IsTemplateAvailable( const char* pName )
{
    if (!m_pDictionary)
        return FALSE;

    s32 index = m_pDictionary->Find(pName);
    if (index == -1)
        return FALSE;

    return TRUE;
}

//=========================================================================

xbool template_mgr::CreateTemplate( const char* pName, const vector3& Pos, const radian3& Rot, u16 Zone1, u16 Zone2, guid ObjectGuid )
{
    if (!m_pDictionary)
        return FALSE;

    s32 index = m_pDictionary->Find(pName);
    if (index == -1)
        return FALSE;
    
    for (s32 i = 0; i < m_nTemplates; i++)
    {
        if (m_pTemplate[i].NameIndex == index)
        {
            //we found it, set the bitstream cursor to the appropriate section
            m_BitStream.SetCursor(m_pTemplate[i].iStartBitStream);

            xarray<guid> Objects;
            Objects.SetCapacity(m_pTemplate[i].nObjects);

            //set up overall template bbox
            bbox Bounds;
            xbool bInit = FALSE;

            //now iterate through
            for (s32 j = m_pTemplate[i].iObject; j < (m_pTemplate[i].iObject + m_pTemplate[i].nObjects); j++)
            {
                //now create the object
                const char* pObjectString = m_pDictionary->GetString( m_pObject[j].TypeIndex) ;

                guid ObjGuid = ObjectGuid;
                if (  ObjGuid == NULL_GUID )
                {
                    ObjGuid = g_ObjMgr.CreateObject( pObjectString );
                }
                else
                {
                    g_ObjMgr.CreateObject( pObjectString, ObjGuid ) ;
                }
                object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);
                if (pObject)
                {
                    if (!pObject->GetTypeDesc().IsDynamic())
                    {
                        //THIS MUST BE A DYNAMIC OBJECT TYPE
                        g_ObjMgr.DestroyObject(ObjGuid);
                        ASSERT(FALSE);
                    }
                    else if( pObject->GetAttrBits() & object::ATTR_EDITOR_TEMP_OBJECT )
                    {
                        //THIS CAN NOT BE A TEMP OBJECT
                        g_ObjMgr.DestroyObject(ObjGuid);
                        ASSERT(FALSE);
                    }
                    else
                    {
                        Objects.Append(ObjGuid);

                        //set all the objects properties
                        for (s32 k = m_pObject[j].iProperty; k < (m_pObject[j].iProperty + m_pObject[j].nProperty); k++)
                        {
                            AddPropertyToObject( m_pProperties[k], pObject);
                        }

                        //now get the bbox of the object
                        if (!bInit)
                        {
                            Bounds = pObject->GetBBox();
                            bInit = TRUE;
                        }
                        else
                        {
                            Bounds.AddVerts(&(pObject->GetBBox().Min),1);
                            Bounds.AddVerts(&(pObject->GetBBox().Max),1);
                        }                    
                    }
                }
            }

            vector3 ptInvereted = Bounds.GetCenter();
            ptInvereted.Negate();
            vector3 delta(Pos - m_pTemplate[i].AnchorPos);

            //now tranform & rotate the objects
            for (s32 a = 0; a < Objects.GetCount(); a++)
            {
                object* pObject = g_ObjMgr.GetObjectByGuid(Objects.GetAt(a));
                if (pObject)
                {
                    //compute Matrix
                    matrix4 L2W = pObject->GetL2W();

                    //first we need to translate the each object to the origin
                    L2W.Translate(ptInvereted);

                    //now we transform
                    L2W.Rotate(Rot);

                    //now we translate back
                    L2W.Translate(Bounds.GetCenter());

                    pObject->OnTransform(L2W);   

                    //now move
                    pObject->OnMoveRel(delta);

                    pObject->SetZone1( Zone1 );
                    pObject->SetZone2( Zone2 );
                }
            }

            if (Objects.GetCount() > 0)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

//=========================================================================

xbool template_mgr::IsSingleTemplate( const char* pName )
{
    if (!m_pDictionary)
        return FALSE;

    s32 index = m_pDictionary->Find(pName);
    if (index == -1)
        return FALSE;

    s32 i;
    for (i=0;i<m_nTemplates;i++)
    {
        if (m_pTemplate[i].NameIndex == index)
        {
            if (m_pTemplate[i].nObjects == 1)
                return TRUE;
        }
    }
    return FALSE;
}

//=========================================================================


guid template_mgr::CreateSingleTemplate( const char* pName, const vector3& Pos, const radian3& Rot, u16 Zone1, u16 Zone2 )
{
    guid CreatedObjectGuid = NULL_GUID;

    if (!m_pDictionary)
        return NULL_GUID;

    s32 index = m_pDictionary->Find(pName);
    if (index == -1)
        return NULL_GUID;
    
    for (s32 i = 0; i < m_nTemplates; i++)
    {
        if (m_pTemplate[i].NameIndex == index)
        {
            //we found it, set the bitstream cursor to the appropriate section
            m_BitStream.SetCursor(m_pTemplate[i].iStartBitStream);

            ASSERTS( m_pTemplate[i].nObjects == 1, "You can only call this routine if there is exactly one object in your template" );
            if ( m_pTemplate[i].nObjects != 1 )
                return NULL_GUID;

            //set up overall template bbox
            bbox Bounds;
            xbool bInit = FALSE;

            //now iterate through
            for (s32 j = m_pTemplate[i].iObject; j < (m_pTemplate[i].iObject + m_pTemplate[i].nObjects); j++)
            {
                //now create the object
                const char* pObjectString = m_pDictionary->GetString( m_pObject[j].TypeIndex );

                const object_desc* pDesc     = g_ObjMgr.GetDescFromName( pObjectString );
#ifndef X_EDITOR
                if( (pDesc->GetType() == object::TYPE_PICKUP) )
                {
                    // Create a net pickup
                    CreatedObjectGuid = CREATE_NET_OBJECT( *pDesc, netobj::TYPE_PICKUP );
                }
                else
#endif
                {
                    CreatedObjectGuid = g_ObjMgr.CreateObject( *pDesc );
                }

                object* pObject   = g_ObjMgr.GetObjectByGuid(CreatedObjectGuid);

                if( pObject )
                {
                    if (!pObject->GetTypeDesc().IsDynamic())
                    {
                        //THIS MUST BE A DYNAMIC OBJECT TYPE
                        g_ObjMgr.DestroyObject(CreatedObjectGuid);
                        CreatedObjectGuid = NULL_GUID;
                        ASSERT(FALSE);
                    }
                    else if( pObject->GetAttrBits() & object::ATTR_EDITOR_TEMP_OBJECT )
                    {
                        //THIS CAN NOT BE A TEMP OBJECT
                        g_ObjMgr.DestroyObject(CreatedObjectGuid);
                        CreatedObjectGuid = NULL_GUID;
                        ASSERT(FALSE);
                    }
                    else if ( pObject->GetType() == object::TYPE_PLAYER )
                    {
                        // mreed: we don't create players from templates any more. We are still allowing the
                        // creation and just deleting because the side effects of creation are necessary for
                        // dependent code in blueprint_bag.
                        g_ObjMgr.DestroyObject( CreatedObjectGuid );
                        CreatedObjectGuid = NULL_GUID;
                    }
                    else
                    {
                        //set all the objects properties
                        for (s32 k = m_pObject[j].iProperty; k < (m_pObject[j].iProperty + m_pObject[j].nProperty); k++)
                        {
                            AddPropertyToObject( m_pProperties[k], pObject);
                        }

                        //now get the bbox of the object
                        if (!bInit)
                        {
                            Bounds = pObject->GetBBox();
                            bInit = TRUE;
                        }
                        else
                        {
                            Bounds.AddVerts(&(pObject->GetBBox().Min),1);
                            Bounds.AddVerts(&(pObject->GetBBox().Max),1);
                        }                    
                    }
                }
            }

            vector3 ptInvereted = Bounds.GetCenter();
            ptInvereted.Negate();
            vector3 delta(Pos - m_pTemplate[i].AnchorPos);

            //now tranform & rotate the objects
            if ( CreatedObjectGuid != NULL_GUID )
            {
                object* pObject = g_ObjMgr.GetObjectByGuid(CreatedObjectGuid);
                if (pObject)
                {
                    //compute Matrix
                    matrix4 L2W = pObject->GetL2W();

                    //first we need to translate the each object to the origin
                    L2W.Translate(ptInvereted);

                    //now we transform
                    L2W.Rotate(Rot);

                    //now we translate back
                    L2W.Translate(Bounds.GetCenter());

                    pObject->OnTransform(L2W);   

                    //now move
                    pObject->OnMoveRel(delta);

                    pObject->SetZone1( Zone1 );
                    pObject->SetZone2( Zone2 );
                }
            }
            return CreatedObjectGuid;
        }
    }

    return CreatedObjectGuid;
}

//=========================================================================

void template_mgr::ApplyTemplateToObject( const char* pName, guid GUID )
{
    if( !m_pDictionary )
        return;

    s32 index = m_pDictionary->Find( pName );
    if( index == -1 )
        return;
    
    for (s32 i = 0; i < m_nTemplates; i++)
    {
        if (m_pTemplate[i].NameIndex == index)
        {
            //we found it, set the bitstream cursor to the appropriate section
            m_BitStream.SetCursor(m_pTemplate[i].iStartBitStream);

            ASSERTS( m_pTemplate[i].nObjects == 1, "You can only call this routine if there is exactly one object in your template" );
            if ( m_pTemplate[i].nObjects != 1 )
                return;

            //now iterate through
            for( s32 j = m_pTemplate[i].iObject; j < (m_pTemplate[i].iObject + m_pTemplate[i].nObjects); j++ )
            {
                object* pObject = g_ObjMgr.GetObjectByGuid( GUID );

                if( pObject )
                {
                    if( !pObject->GetTypeDesc().IsDynamic() )
                    {
                        // THIS MUST BE A DYNAMIC OBJECT TYPE
                        g_ObjMgr.DestroyObject(GUID);
                        GUID = NULL_GUID;
                        ASSERT(FALSE);
                    }
                    else if( pObject->GetAttrBits() & object::ATTR_EDITOR_TEMP_OBJECT )
                    {
                        // THIS CAN NOT BE A TEMP OBJECT
                        g_ObjMgr.DestroyObject(GUID);
                        GUID = NULL_GUID;
                        ASSERT(FALSE);
                    }
                    else if ( pObject->GetType() == object::TYPE_PLAYER )
                    {
                        // mreed: we don't create players from templates any more. We are still allowing the
                        // creation and just deleting because the side effects of creation are necessary for
                        // dependent code in blueprint_bag.
                        g_ObjMgr.DestroyObject( GUID );
                        GUID = NULL_GUID;
                        ASSERT( FALSE );
                    }
                    else
                    {
                        //set all the objects properties
                        for (s32 k = m_pObject[j].iProperty; k < (m_pObject[j].iProperty + m_pObject[j].nProperty); k++)
                        {
                            AddPropertyToObject( m_pProperties[k], pObject);
                        }
                    }
                }
            }
        }
    }
}

//=========================================================================

void template_mgr::AddPropertyToObject( prop_entry& pe, object* pObject )
{
//    CLOG_MESSAGE( LOGGING_ENABLED, "template_mgr::AddPropertyToObject", "%08x '%s:%s' - %s", (u32)pObject, pObject->GetTypeDesc().GetTypeName(), pObject->GetName(), m_pDictionary->GetString(pe.NameIndex) );

    prop_query pq;
    switch(pe.PropType)
    {
    case PROP_TYPE_EXTERNAL:
        {
            char cData[256];
            m_BitStream.ReadString(cData);
            pq.WQueryExternal( m_pDictionary->GetString(pe.NameIndex), cData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_BUTTON:  
        {
            char cData[256];
            m_BitStream.ReadString(cData);
            pq.WQueryButton( m_pDictionary->GetString(pe.NameIndex), cData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_FILENAME:     
        {
            char cData[256];
            m_BitStream.ReadString(cData);
            pq.WQueryFileName( m_pDictionary->GetString(pe.NameIndex), cData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_ENUM:  
        {
            char cData[256];
            m_BitStream.ReadString(cData);
            pq.WQueryEnum( m_pDictionary->GetString(pe.NameIndex), cData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_STRING:   
        {
            char cData[256];
            m_BitStream.ReadString(cData);
            pq.WQueryString( m_pDictionary->GetString(pe.NameIndex), cData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_FLOAT:  
        {
            f32 fData;
            m_BitStream.ReadF32(fData);
            pq.WQueryFloat( m_pDictionary->GetString(pe.NameIndex), fData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_INT:       
        {
            s32 nData;
            m_BitStream.ReadS32(nData);
            pq.WQueryInt( m_pDictionary->GetString(pe.NameIndex), nData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_BOOL:
        {
            xbool bData;
            bData = m_BitStream.ReadFlag();
            pq.WQueryBool( m_pDictionary->GetString(pe.NameIndex), bData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_VECTOR3:
        {
            vector3 v3Data;
            m_BitStream.ReadVector(v3Data);
            pq.WQueryVector3( m_pDictionary->GetString(pe.NameIndex), v3Data);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_ANGLE:
        {
            radian rData;
            m_BitStream.ReadF32(rData);
            pq.WQueryAngle( m_pDictionary->GetString(pe.NameIndex), rData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_ROTATION:  			
        {
            radian3 r3Data;
            m_BitStream.ReadRadian3(r3Data);
            pq.WQueryRotation( m_pDictionary->GetString(pe.NameIndex), r3Data);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_BBOX:      
        {
            bbox bbData;
            m_BitStream.ReadVector(bbData.Min);
            m_BitStream.ReadVector(bbData.Max);
            pq.WQueryBBox( m_pDictionary->GetString(pe.NameIndex), bbData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_COLOR:
        {
            xcolor xcData;
            m_BitStream.ReadColor(xcData);
            pq.WQueryColor( m_pDictionary->GetString(pe.NameIndex), xcData);
            pObject->OnProperty(pq);
        }
	    break;
    case PROP_TYPE_GUID:     
        {
            u64 gData;
            m_BitStream.ReadU64(gData);
            pq.WQueryGUID( m_pDictionary->GetString(pe.NameIndex), gData);
            pObject->OnProperty(pq);
        }
	    break;
	}
}

//=========================================================================

//-----------------// BEGIN EDITOR SPECIFIC CODE
#ifdef X_EDITOR
//-----------------//

//=========================================================================
// EDITOR STRUCTURES & CLASSES
//=========================================================================

//data elements used to generate Data Stream

struct EditorObjData
{
    char                    cType[MAX_PATH];
    xarray<prop_container>  Properties;
};

struct EditorBPData 
{
    char                    cName[MAX_PATH];
    vector3                 AnchorPos;        
    xarray<EditorObjData>   Objects;
};

//=========================================================================
// EDITOR FUNCTIONS
//=========================================================================

guid template_mgr::EditorCreateSingleTemplateFromPath( const char* pName, const vector3& Pos, const radian3& Rot , u16 Zone1, u16 Zone2 )
{
    if (x_strlen(pName) <= 0)
    {
        x_DebugMsg("ERROR!!! something has tried to create a template with no name specified!\n");
        return NULL;
    }

    x_strcpy(m_PreGamePathing, pName);
    EditorCreateGameData(TRUE);
    return CreateSingleTemplate( pName, Pos, Rot, Zone1, Zone2);
}

//=========================================================================

xbool template_mgr::EditorCreateTemplateFromPath( const char* pName, const vector3& Pos, const radian3& Rot , u16 Zone1, u16 Zone2, guid ObjectGuid )
{
    if (x_strlen(pName) <= 0)
    {
        x_DebugMsg("ERROR!!! something has tried to create a template with no name specified!\n");
        return FALSE;
    }

    x_strcpy(m_PreGamePathing, pName);
    EditorCreateGameData(TRUE);
    return CreateTemplate( pName, Pos, Rot, Zone1, Zone2, ObjectGuid );
}

//=========================================================================

xbool template_mgr::EditorBuildTemplateArray( void )
{
    x_try;

    m_lstBlueprints.Clear();

    //add blueprint templates to dictionary for each dynamic object
    u32 objectTypeCount;
    for(objectTypeCount = 0; objectTypeCount < object::TYPE_END_OF_LIST; objectTypeCount++)
    {
        slot_id SlotID = g_ObjMgr.GetFirst((object::type)objectTypeCount);

        if( SlotID != SLOT_NULL )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot(SlotID);
            if( pObject == NULL )
                continue;

            if( pObject->GetTypeDesc().IsDynamic() == FALSE )
                continue;

            while(SlotID != SLOT_NULL)
            {
                object* pObject = g_ObjMgr.GetObjectBySlot(SlotID) ;

                //add to list
                pObject->EditorPreGame();

                // Check next
                SlotID = g_ObjMgr.GetNext(SlotID) ;
            }
        }
    }

    //walk the dictionary, and add all paths to the blueprint array
    //...
    for (s32 i = 0; i < g_TemplateStringMgr.GetCount(); i++)
    {
        const char* pPath = g_TemplateStringMgr.GetString(i);
        BlueprintReference BPRef;
        x_strcpy(BPRef.cFullPath, pPath);
        m_lstBlueprints.Append(BPRef);
    }

    return TRUE;

    x_catch_display;

    //damn we had an error
    m_lstBlueprints.Clear();

    return FALSE;
}

//=========================================================================

//NOTE: Only set bPreGameDetermination to TRUE if called from object EditorPreGame
xbool template_mgr::EditorCreateGameData( xbool bPreGameDetermination )
{
    xbool bNonDynamicObjectEncountered = FALSE;

    if (!bPreGameDetermination)
    {
        EditorBuildTemplateArray();

        //first clear any existing data
        ClearData( );

        //lets get our counts first so we correctly allocate memory
        m_nTemplates = m_lstBlueprints.GetCount();
    }
    else
    {
        //first clear any existing data
        ClearData( );

        //pre-game data build phase
        m_nTemplates = 1; 
    }

    m_pDictionary = new dictionary;

    //allocate bitstream
    m_BitStream.Init(128);
    
    //lets create some temporary data structures
    xarray<EditorBPData> lstData;
    for (s32 i = 0; i < m_nTemplates; i++)
    {
        text_in Blueprint;
        
        x_try;
        EditorBPData BPData;

        if (!bPreGameDetermination)
        {
            BlueprintReference& BPRef = m_lstBlueprints.GetAt(i);
            x_strcpy(BPData.cName, BPRef.cFullPath);
            Blueprint.OpenFile( BPRef.cFullPath );
        }
        else
        {
            //pre-game build phase
            x_strcpy(BPData.cName, m_PreGamePathing);
            Blueprint.OpenFile( m_PreGamePathing );
        }

        while (Blueprint.ReadHeader())
        {
            if( x_stricmp( Blueprint.GetHeaderName(), "Anchor" ) == 0 )
            {
                //anchor Position
                Blueprint.ReadFields();
                if (!Blueprint.GetVector3("Position",BPData.AnchorPos))
                {
                    x_throw(xfs("Create Template Data Stream Error\nError Reading Blueprint Anchor for %s",BPData.cName));
                }
            }
            else if( x_stricmp( Blueprint.GetHeaderName(), "Object" ) == 0 )
            {
                //new object
                EditorObjData ObjData;

                Blueprint.ReadFields();
                if (Blueprint.GetString("SType",ObjData.cType)) //New Way
                {
                    //ok now read in all the properties
/*
                    s32 iPropertyCount = LoadPropertyData(Blueprint, ObjData.Properties);
*/
//since I can't get this to work with prop_containers, I will do this the really cheesy way
                    
                    guid ObjGuid = g_ObjMgr.CreateObject( ObjData.cType );
                    object *pObjTemp = g_ObjMgr.GetObjectByGuid(ObjGuid);

                    if (pObjTemp)
                    {
                        if (!pObjTemp->GetTypeDesc().IsDynamic())
                        {
                            if (!bNonDynamicObjectEncountered)
                            {
                                //only throw one major warning
                                bNonDynamicObjectEncountered = TRUE;
                                x_try;
                                x_throw(xfs("Create Template Data Stream Error\nThere is 1 or more non-dynamic object(s) in Blueprint %s",BPData.cName));
                                x_catch_display;
                            }
                            x_DebugMsg("WARNING: There is a non-dynamic object in Template %s\n",BPData.cName);
                        }

                        pObjTemp->OnLoad(Blueprint);
                        pObjTemp->OnCopy(ObjData.Properties);
            
                        for( s32 t=0; t<ObjData.Properties.GetCount(); t++ )
                        {
                            if( (ObjData.Properties[t].GetTypeFlags() & PROP_TYPE_DONT_EXPORT) )
                            {
                                ObjData.Properties.Delete(t);
                                t--;
                            }
                        }

                        g_ObjMgr.DestroyObjectEx( ObjGuid, TRUE );
                        m_nProperties += ObjData.Properties.GetCount();
                    }       
                }
                else
                {
                    x_throw(xfs("Create Template Data Stream Error\nError Reading Object in Blueprint %s",BPData.cName));
                }

                BPData.Objects.Append(ObjData);
                m_nObjects++; //increment object count
            }
            else
            {
                x_throw(xfs("Create Template Data Stream Error\nBad Header in Blueprint %s",BPData.cName));
            }
        }        

        lstData.Append(BPData);

        x_catch_display;
        
        Blueprint.CloseFile();
    }
    
    //alloc template data
    m_pTemplate = (template_entry*)x_malloc(sizeof(template_entry)*m_nTemplates);
    ASSERT(m_pTemplate);
    x_memset( m_pTemplate, 0, sizeof(template_entry)*m_nTemplates );

    //alloc object data
    m_pObject = (obj_entry*)x_malloc(sizeof(obj_entry)*m_nObjects);
    ASSERT(m_pObject);
    x_memset( m_pObject, 0, sizeof(obj_entry)*m_nObjects );

    //alloc property data
    m_pProperties = (prop_entry*)x_malloc(sizeof(prop_entry)*m_nProperties);
    ASSERT(m_pProperties);
    x_memset( m_pProperties, 0, sizeof(prop_entry)*m_nProperties );

    //now we need to update the bitstream and dictionary and pointer arrays
    s16 iCurrentObjectIndex = 0;
    s16 iCurrentPropertyIndex = 0;
    for ( i = 0; i < lstData.GetCount(); i++)
    {
        EditorBPData& BPData = lstData.GetAt(i);

        //set all template data
        m_pTemplate[i].iStartBitStream = m_BitStream.GetCursor();
        m_pTemplate[i].AnchorPos = BPData.AnchorPos;
        m_pTemplate[i].nObjects = BPData.Objects.GetCount();
        m_pTemplate[i].NameIndex = m_pDictionary->Add(BPData.cName);
        m_pTemplate[i].iObject = iCurrentObjectIndex;
       
        //now set all object data
        for (s32 j = 0; j < BPData.Objects.GetCount(); j++)
        {
            EditorObjData& ObjData = BPData.Objects.GetAt(j);

            m_pObject[j+iCurrentObjectIndex].TypeIndex = m_pDictionary->Add(ObjData.cType);
            m_pObject[j+iCurrentObjectIndex].nProperty = ObjData.Properties.GetCount();
            m_pObject[j+iCurrentObjectIndex].iProperty = iCurrentPropertyIndex;

            //now set all the properties
            for (s32 k = 0; k < ObjData.Properties.GetCount(); k++)
            {
                prop_container& pc = ObjData.Properties.GetAt(k);

                m_pProperties[k+iCurrentPropertyIndex].NameIndex = m_pDictionary->Add(pc.GetName());
                m_pProperties[k+iCurrentPropertyIndex].PropType = (u16)pc.GetType();

                //add this data to the bitstream
                AddDataToBitStream(pc);
            }

            //update the property index
            iCurrentPropertyIndex += m_pObject[j+iCurrentObjectIndex].nProperty;
        }

        //update the object index
        iCurrentObjectIndex += m_pTemplate[i].nObjects;
    }

    return TRUE;
}

//=========================================================================

void template_mgr::AddDataToBitStream( prop_container& pc )
{
    switch(pc.GetType())
    {
    case PROP_TYPE_FILENAME:     
        {
            char cData[MAX_PATH];
            pc.GetFileName(cData);
            m_BitStream.WriteString(cData);
        }
	    break;
    case PROP_TYPE_EXTERNAL:
        {
            char cData[MAX_PATH];
            pc.GetExternal(cData);
            m_BitStream.WriteString(cData);
        }
	    break;
    case PROP_TYPE_BUTTON:  
        {
            char cData[MAX_PATH];
            pc.GetButton(cData);
            m_BitStream.WriteString(cData);
        }
	    break;    
    case PROP_TYPE_ENUM:  
        {
            char cData[MAX_PATH];
            pc.GetEnum(cData);
            m_BitStream.WriteString(cData);
        }
	    break;    
    case PROP_TYPE_STRING:   
        {
            char cData[MAX_PATH];
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

//=========================================================================

xbool template_mgr::EditorSaveData( const char* pFile, const char* pDictionary )
{
    x_try;

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

    u16 uVersion = kTemplateFileVersion;
    s32 nBitStreamSize = m_BitStream.GetNBytesUsed();

    //write the version number
    x_fwrite( &uVersion , sizeof(u16), 1, fpTemplate );

    //write the dictionary size
    x_fwrite( &nDictSize , sizeof(s32), 1, fpTemplate );

    //write the template list size
    x_fwrite( &m_nTemplates , sizeof(s32), 1, fpTemplate );

    //write the object list size
    x_fwrite( &m_nObjects , sizeof(s32), 1, fpTemplate );

    //write the property list size
    x_fwrite( &m_nProperties , sizeof(s32), 1, fpTemplate );

    //write the bitstream size
    x_fwrite( &nBitStreamSize , sizeof(s32), 1, fpTemplate );

    //write the template list data
    x_fwrite( m_pTemplate , sizeof(template_entry), m_nTemplates, fpTemplate );

    //write the template list data
    x_fwrite( m_pObject , sizeof(obj_entry), m_nObjects, fpTemplate );

    //write the template list data
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
