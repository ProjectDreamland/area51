#include "StdAfx.h"
#include "x_files.hpp"
#include "WorldEditor.hpp"
#include "Obj_Mgr/obj_mgr.hpp"
#include "Objects/PlaySurface.hpp"
#include "Objects/AnimSurface.hpp"
#include "Objects/PropSurface.hpp"
#include "Objects/Door.hpp"
#include "Objects/Pickup.hpp"
#include "..\Apps\EDRscDesc\RSCDesc.hpp"
#include "..\editor\Project.hpp"

extern char *g_CommandLine;

struct TextureData {
    u16     uCount;
    xstring xstrName;
};

// This actually creates the dfs script file. This is called when ExportToLevel() is performed
void world_editor::CreateDFSFile(const char* pFilename)
{
    X_FILE*         OutFile;
    xarray<xstring> FileList;
    s32             i,j;

    OutFile = x_fopen(pFilename, "wt" ); 
    if(OutFile == NULL )
    {
        x_DebugMsg( "Could not open file \"%s\".  Could be write protected\n" );
        return;
    }
    
    for( i = 0; i < object::TYPE_END_OF_LIST; i++ )
    {
        slot_id SlotId;
        for( SlotId = g_ObjMgr.GetFirst( (object::type)i ); SlotId != SLOT_NULL;  SlotId = g_ObjMgr.GetNext( SlotId ) )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( SlotId );
            
            //  Make sure we got a valid object and if not, let them know since
            //  an object in the list is a bug that should be tracked down
            if( pObject == NULL )
            {
                x_DebugMsg( "WARNING:  Invalid object type:  %s   slot_id: %d", 
                                g_ObjMgr.GetNameFromType( (object::type)i), 
                                SlotId          );
                continue;
            }

            //  If it doesn't have a resource name then we don't care about it
            if (!(pObject->GetTypeDesc().QuickResourceName() ) )
            {
                continue;
            }

            //  If the object type is a Rigid Geom then we need to get it's
            //  associated rigid_geom file and then get the textures from the
            //  rigid geom and add them all to the list
            if( x_strcmp(pObject->GetTypeDesc().QuickResourceName(),"RigidGeom") == 0 )
            {

                // Check to see what type of object it is since our silly object
                // hiearchy doesn't have a base "renderable" type or something
                // similiar
                rigid_inst* RigidInst = NULL;
                rigid_geom* RigidGeom = NULL;

                if( pObject->IsKindOf( play_surface::GetRTTI() ) ||
                    pObject->IsKindOf( anim_surface::GetRTTI() ) ||
                    pObject->IsKindOf( prop_surface::GetRTTI() ) )
                {
                    play_surface* playSurface = &play_surface::GetSafeType( *pObject );
                    RigidInst = &playSurface->GetRigidInst();
                }
                else if( pObject->IsKindOf( pickup::GetRTTI() ) )
                {
                    pickup* pickUp = &pickup::GetSafeType( *pObject );
                    RigidInst = &pickUp->GetRigidInst();
                }
                else if(pObject->IsKindOf( door::GetRTTI() ) )
                {
                    door* aDoor = &door::GetSafeType( *pObject );
                    RigidInst = &aDoor->GetRigidInst();
                }

                if( RigidInst )
                {
                    RigidGeom = RigidInst->GetRigidGeom();
                    if( RigidGeom == NULL )
                    {
                        x_DebugMsg("Rigid Inst with no rigid geom?\n" );
                        continue;
                    }
                    prop_query  Query;
                    char        Name[MAX_PATH];
                    Query.RQueryExternal( pObject->GetTypeDesc().QuickResourcePropertyName(), Name );
                    pObject->OnProperty( Query );

                    // At this time rigidName should contain the name of the rigid geom file.  Now we check
                    // to see if that rigid geom file is in our list and if it is, we mark to add it to the
                    // list of resources to load

                    FileList.Append(Name);

                    for ( s32 j = 0; j < RigidGeom->m_nTextures; j++ )
                    {
                        FileList.Append( RigidGeom->GetTextureName( j ) );
                    }
                }
            }
            else
            {
                x_DebugMsg("Object of type %s\n",pObject->GetTypeDesc().QuickResourceName());
            }
        }   
    }  

    // Remove duplicated names
    for( i = 0; i < FileList.GetCount(); i++ )
    {
        for (j = i+1; j < FileList.GetCount() - 1; j++ )
        {
            if (x_strcmp(FileList[j],FileList[i])==0)
            {
                FileList.Delete(j);
                j--;
            }
        }
    }

    for (i = 0; i < FileList.GetCount() ; i++ )
    {
        x_fprintf(OutFile,"%s\\%s\\%s\n",g_Settings.GetReleasePath(),"ps2",FileList[i]);
    }


    x_fclose( OutFile );
}

//=============================================================================
// Build the 'fs.txt' file and prep to use dfstool to export all the files to
// a single, hopefully ordered, dfs file.
//
// We need to mirror the initial instance creation process when the game is started.
//
//=============================================================================
xbool world_editor::BuildDFS(const char* pFilename)
{
    CreateDFSFile(pFilename);
    return TRUE;
}


//=============================================================================
void world_editor::MakeRegisteredRigidList(xarray<RigidGeomData> &RigidGeoms)
{

    xarray<xstring>         lstRigidGeoms;
    s32 i ;
    for ( i =0; i < g_RescDescMGR.GetRscDescCount(); i++)
    {
        rsc_desc_mgr::node &Node = g_RescDescMGR.GetRscDescIndex(i);
        if (x_stricmp(Node.pDesc->GetType(),"RigidGeom") == 0 )
        {
            lstRigidGeoms.Append(Node.pDesc->GetName());
        }
    }
    
    
    for( i = 0; i < lstRigidGeoms.GetCount(); i++)
    {
        RigidGeomData rgd;
        rgd.uCount = 0;
        rgd.bRegistered = TRUE;
        rgd.xstrName = xstring(lstRigidGeoms.GetAt(i));
        rgd.nVerts = rgd.nPolys = rgd.nMaterials = rgd.nTextures = rgd.nMeshes = rgd.nSubMeshes = -1;
        RigidGeoms.Append(rgd);
    }



}

#if 0

        ///----- OLD
        xstrOutputFile.Format( "%s\\%s\\%s.level", pReleasePath, pPlatformString, pFileName );
        level Level;
        Level.Open( xstrOutputFile, FALSE );
        
        for( i=0; i<nGuid; i++ )
        {
            Level.Save( lstGuidsToExport[i] );
            if (m_pHandler) m_pHandler->SetProgress( ++Progress );
        }
        Level.Close();
        
        //=============================================================================
        xstrOutputFile.Format( "%s\\%s\\%s.load", pReleasePath, pPlatformString, pFileName );
         
        g_WorldEditor.CreateResourceLoadList( xstrOutputFile );
        

        //=============================================================================
        
        g_BinLevelMgr.EditorSaveData(xstrOutputFile, xstrDictionary,lstGuidsToExport );

    //---- ORIGINAL
    xarray<xstring>         listOfResources;
    xarray<RigidGeomData>   rigidGeoms;
    xarray<TextureData>     textures;


    
    MakeRegisteredRigidList( rigidGeoms );
    
    int objectTypeCount;
 
    for( objectTypeCount = 0; objectTypeCount < object::TYPE_END_OF_LIST; objectTypeCount++ )
    {
        slot_id slotID;
        for( slotID = g_ObjMgr.GetFirst( (object::type)objectTypeCount ); slotID != SLOT_NULL;  slotID = g_ObjMgr.GetNext( slotID ) )
        {
            object* currentObject = g_ObjMgr.GetObjectBySlot( slotID );
            
            //  Make sure we got a valid object and if not, let them know since
            //  an object in the list is a bug that should be tracked down
            if( currentObject == NULL )
            {
                x_DebugMsg( "WARNING:  Invalid object type:  %s   slot_id: %d", 
                                g_ObjMgr.GetNameFromType( (object::type)objectTypeCount), 
                                slotID          );
                continue;
            }

            //  If it doesn't have a resource name then we don't care about it
            if (!(currentObject->GetTypeDesc().QuickResourceName() ) )
            {
                continue;
            }

            //  If the object type is a Rigid Geom then we need to get it's
            //  associated rigid_geom file and then get the textures from the
            //  rigid geom and add them all to the list
            if( x_strcmp(currentObject->GetTypeDesc().QuickResourceName(),"RigidGeom") == 0 )
            {

                // Check to see what type of object it is since our silly object
                // hiearchy doesn't have a base "renderable" type or something
                // similiar
                rigid_inst* rigidInst = NULL;
                rigid_geom* rigidGeom = NULL;

                if( currentObject->IsKindOf( play_surface::GetRTTI() ) ||
                    currentObject->IsKindOf( anim_surface::GetRTTI() ) ||
                    currentObject->IsKindOf( prop_surface::GetRTTI() ) )
                {
                    play_surface* playSurface = &play_surface::GetSafeType( *currentObject );
                    rigidInst = &playSurface->GetRigidInst();
                }
                else if(    currentObject->IsKindOf( boost_pickup::GetRTTI()    ) ||
                            currentObject->IsKindOf( defence_pickup::GetRTTI()  ) ||
                            currentObject->IsKindOf( health_pickup::GetRTTI()   ) ||
                            currentObject->IsKindOf( pickup::GetRTTI() )                )
                {
                    pickup* pickUp = &pickup::GetSafeType( *currentObject );
                    rigidInst = &pickUp->GetRigidInst();
                }
                else if(currentObject->IsKindOf( door::GetRTTI() ) )
                {
                    door* aDoor = &door::GetSafeType( *currentObject );
                    rigidInst = &aDoor->GetRigidInst();
                }

                if( rigidInst != NULL )
                {
                    rigidGeom = rigidInst->GetRigidGeom();
                    if( rigidGeom == NULL )
                    {
                        x_DebugMsg("Rigid Inst with no rigid geom?\n" );
                        continue;
                    }
                    prop_query propQuery;
                    char rigidName[MAX_PATH];
                    propQuery.RQueryExternal( currentObject->GetTypeDesc().QuickResourcePropertyName(), & rigidName[0] );
                    currentObject->OnProperty( propQuery );

                    // At this time rigidName should contain the name of the rigid geom file.  Now we check
                    // to see if that rigid geom file is in our list and if it is, we mark to add it to the
                    // list of resources to load



                    //handle rigidgeom data
                    xbool found = FALSE;
                    s32 rigidCount;
                    for( rigidCount = 0; rigidCount < rigidGeoms.GetCount(); rigidCount++)
                    {
                        RigidGeomData& rigidGeomData = rigidGeoms.GetAt(rigidCount);
                        if ( x_strcmp(rigidGeomData.xstrName, rigidName) == 0)
                        {
                            
                            if (rigidGeom && rigidGeomData.uCount == 0)
                            {
                                rigidGeomData.uCount++;
                                rigidGeomData.nMaterials  = rigidGeom->m_nMaterials;
                                rigidGeomData.nTextures   = rigidGeom->m_nTextures;
                                rigidGeomData.nMeshes     = rigidGeom->m_nMeshes;
                                rigidGeomData.nSubMeshes  = rigidGeom->m_nSubMeshs;
                            }
                            found = TRUE;
                        }
                    }

                    //  if we don't find it then we need to add it
                    if (!found)
                    {
                        //append new one
                        RigidGeomData rigidGeomData;
                        rigidGeomData.uCount = 1;
                        rigidGeomData.bRegistered = FALSE;
                        rigidGeomData.xstrName = xstring(rigidName);
                        if (rigidGeom)
                        {
                            rigidGeomData.nMaterials    = rigidGeom->m_nMaterials;
                            rigidGeomData.nTextures     = rigidGeom->m_nTextures;
                            rigidGeomData.nMeshes       = rigidGeom->m_nMeshes;
                            rigidGeomData.nSubMeshes    = rigidGeom->m_nSubMeshs;
                        }
                        else
                        {
                            rigidGeomData.nMaterials = rigidGeomData.nTextures = rigidGeomData.nMeshes = rigidGeomData.nSubMeshes = -1;
                        }
                        
                        rigidGeoms.Append(rigidGeomData);
                    }


                }
         

                
//=============================================================================

                if (rigidInst)
                {
                    s32 nTex = rigidInst->GetTextureCount( ); 
                    for (s32 j = 0; j < nTex; j++)
                    {
                        xbool foundTex = FALSE;
                        const char* pName = rigidInst->GetTexture(j);
                        for( s32 k = 0; k < textures.GetCount(); k++)
                        {
                            TextureData& td = textures.GetAt(k);
                            if (x_strcmp(td.xstrName, pName)==0)
                            {
                                td.uCount++;
                                foundTex = TRUE;
                            }
                        }
                        
                        if (!foundTex)
                        {
                            TextureData td;
                            td.uCount = 1;
                            td.xstrName = xstring(pName);
                            textures.Append(td);
                        }
                    }
                }            
                
    ::MessageBox( NULL, Script, "Resulting DFS File",MB_ICONWARNING|MB_OK );
    return Script.SaveFile("dfstool-script.txt");
#endif
