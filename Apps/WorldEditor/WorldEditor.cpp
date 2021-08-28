#include <io.h>
#include <stdio.h>
#include "StdAfx.h"
#include "WorldEditor.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "Render\LightMgr.hpp"
#include "Parsing\TextIn.hpp"
#include "Parsing\TextOut.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Objects\PlaySurface.hpp"
#include "PlaySurfaceMgr\PlaySurfaceMgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "blueprint_anchor.hpp"
#include "ai_editor.hpp"
#include "..\Editor\Lighting.hpp"
#include "..\Editor\Project.hpp"
#include "Gamelib\Level.hpp"
#include "GameLib\binlevel.hpp"
#include "transaction_mgr.hpp"
#include "transaction_layer_data.hpp"
#include "transaction_bpref_data.hpp"
#include "transaction_object_data.hpp"
#include "transaction_file_data.hpp"
#include "transaction_selection_data.hpp"
#include "transaction_zone_data.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "InputMgr\InputMgr.hpp"
#include "Objects\PlaySurface.hpp"
#include "Objects\PropSurface.hpp"
#include "Objects\AnimSurface.hpp"
#include "Objects\SuperDestructible.hpp"
#include "Objects\door.hpp"
#include "Objects\ParticleEmiter.hpp"
#include "Objects\Portal.hpp"
#include "Objects\Player.hpp"
#include "Objects\ClothObject.hpp"
#include "Objects\Corpse.hpp"
#include "Objects\TeamProp.hpp"
#include "Objects\LoreObject.hpp"
//#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\TriggerEx\TriggerEx_Manager.hpp"
#include "..\Support\ZoneMgr\ZoneMgr.hpp"
#include "..\Support\Templatemgr\TemplateMgr.hpp"
#include "..\Support\Objects\ParticleEmiter.hpp"
#include "..\Support\Tracers\TracerMgr.hpp"
#include "PhysicsMgr\PhysicsMgr.hpp"
#include "Font\font.hpp"
#include "Music_Mgr\Music_Mgr.hpp"
#include "MusicStateMgr\MusicStateMgr.hpp"
#include "ConversationMgr\ConversationMgr.hpp"

#include "Decals\DecalMgr.hpp"
#include "static_decal.hpp"
#include "GenericDialog\GenericDialog.hpp"

#include "..\Apps\FxEditor\FxEditor.hpp"
#include "..\Apps\DecalEditor\DecalEditor.hpp"
#include "GameTextMgr\GameTextMgr.hpp"
#include <io.h>

#include "Objects\AlienGlob.hpp"
#include "nav_connection2_editor.hpp"
#include "nav_connection2_anchor.hpp"
#include "Objects\LevelSettings.hpp"

//=========================================================================
// Global Declaration
//=========================================================================

world_editor     g_WorldEditor;

xbool       g_AimAssist_Render_Reticle      = FALSE;
xbool       g_AimAssist_Render_Bullet       = FALSE;
xbool       g_AimAssist_Render_Turn         = FALSE;
xbool       g_AimAssist_Render_Bullet_Angle = FALSE;
xbool       g_AimAssist_Render_Player_Pills = FALSE;

xbool   g_game_running      = FALSE;
xbool   g_level_loading     = FALSE;
xbool   g_UnlimitedAmmo     = TRUE;
xbool   g_RenderShadows     = TRUE;
xbool   g_first_person      = FALSE;
xbool   g_GameLogicDebug    = TRUE;
xbool   g_bBloodEnabled     = TRUE;
xbool   g_bRagdollsEnabled  = TRUE;
xbool   g_RenderBoneBBoxes  = FALSE;

s32             g_Difficulty = 1; // Start out on Medium difficulty
const char* DifficultyText[] = { "Easy", "Medium", "Hard" };

#define PROP_COLOR_ZONE                 xcolor(180, 220, 180)
#define PROP_COLOR_1TEMPOBJ             xcolor(220, 180, 180)
#define PROP_COLOR_TEMPOBJS             xcolor(220, 180, 220)    
#define PROP_COLOR_1OBJECT              xcolor(220, 220, 180)
#define PROP_COLOR_BP_MOD               xcolor(180, 180, 220)
#define PROP_COLOR_MULTI_OBJS           xcolor(200, 180, 220)
#define PROP_COLOR_MULTI_SAME_OBJS      xcolor(200, 220, 180)

//=========================================================================
//GLOBAL VARIABLE USED FOR ANONYMOUS REGISTERATION OF THE FX SYSTEM
//=========================================================================

extern s32 g_fx_link;
extern s32 g_stringbin_link;
extern s32 g_decal_link;
extern s32 g_font_link;
extern s32 s_PS2MemorySize;
extern xbool g_bAutoLoad;

//=========================================================================

//=========================================================================
//=========================================================================
// editor_layer
//=========================================================================
//=========================================================================

editor_layer::editor_layer( ) 
{ 
    //=========================================================================
    //anonymous linking variable for fx subsystems..
    g_fx_link   = 0;

    //=========================================================================
    // Link the binary string system.
    g_stringbin_link    = 0;

    //=========================================================================
    // Link the decal system.
    g_decal_link    = 0;

    //=========================================================================
    // Link the font system.
    g_font_link     =0;

    IsDirty     = FALSE; 
    IsLoaded    = FALSE; 
    IsEditable  = TRUE; 
    IsNull      = FALSE; 
}

//=========================================================================

xbool editor_layer::AddObject( editor_object_ref Object ) 
{
    if (Objects.GetCount()>0)
    {
        for (s32 iXaIndex = 0; iXaIndex < Objects.GetCount(); iXaIndex++)
		{
            editor_object_ref& ObjRef = Objects.GetAt(iXaIndex);
            if (ObjRef.Guid == Object.Guid)
            {
                //error it exists
                return FALSE;
            }
        }
	}
    
    Objects.Append(Object);
    return TRUE;
}

//=========================================================================

xbool editor_layer::RemoveObject( guid ObjectGuid ) 
{
    if (Objects.GetCount()>0)
    {
        for (s32 iXaIndex = 0; iXaIndex < Objects.GetCount(); iXaIndex++)
		{
            editor_object_ref& ObjRef = Objects.GetAt(iXaIndex);
            if (ObjRef.Guid == ObjectGuid)
            {
                Objects.Delete(iXaIndex);
                return TRUE;
            }
        }
	}
    return FALSE;
}

//=========================================================================

xbool editor_layer::AddResource( xstring ResourceName ) 
{
    if (Resources.GetCount()>0)
    {
        for (s32 iXaIndex = 0; iXaIndex < Resources.GetCount(); iXaIndex++)
		{
            xstring& xstrRes = Resources.GetAt(iXaIndex);
            if (x_stricmp(xstrRes, ResourceName) == 0)
            {
                //error it exists
                return FALSE;
            }
        }
	}

    Resources.Append(ResourceName);
    return TRUE;
}

//=========================================================================

xbool editor_layer::RemoveResource( xstring ResourceName ) 
{
    if (Resources.GetCount()>0)
    {
        for (s32 iXaIndex = 0; iXaIndex < Resources.GetCount(); iXaIndex++)
		{
            xstring& xstrRes = Resources.GetAt(iXaIndex);
            if (x_stricmp(xstrRes, ResourceName) == 0)
            {
                //found it
                Resources.Delete(iXaIndex);
                return TRUE;
            }
        }
	}
    return FALSE;
}

//=========================================================================

xbool editor_layer::AddBlueprint( const editor_blueprint_ref& Blueprint ) 
{
    if (Blueprints.GetCount()>0)
    {
        for (s32 iXaIndex = 0; iXaIndex < Blueprints.GetCount(); iXaIndex++)
		{
            editor_blueprint_ref& BlueprintCheck = Blueprints.GetAt(iXaIndex);
            if (Blueprint.Guid == BlueprintCheck.Guid)
            {
                //exists already
                return FALSE;
            }
        }
	}

    Blueprints.Append(Blueprint);
    return TRUE;
}

//=========================================================================

xbool editor_layer::RemoveBlueprint( guid BlueprintGuid ) 
{
    if (Blueprints.GetCount()>0)
    {
        for (s32 iXaIndex = 0; iXaIndex < Blueprints.GetCount(); iXaIndex++)
		{
            editor_blueprint_ref& Blueprint = Blueprints.GetAt(iXaIndex);
            if (Blueprint.Guid == BlueprintGuid)
            {
                Blueprints.Delete(iXaIndex);
                return TRUE;
            }
        }
	}
    return FALSE;
}


//=========================================================================
//=========================================================================
// editor_blueprint_ref
//=========================================================================
//=========================================================================

vector3 editor_blueprint_ref::GetAnchorPosition( void )
{
    vector3 Position( 0.0f, 0.0f, 0.0f );

    object *pObject = g_ObjMgr.GetObjectByGuid( Anchor );
    if( pObject )
    {
        Position = pObject->GetPosition();
    }

    return Position;
}

bbox editor_blueprint_ref::ComputeBBox( void )
{
    bbox B;

    // Loop through all the blueprints objects
    for( s32 i=0; i <= ObjectsInBlueprint.GetCount(); i++ )
    {
        guid Guid;
        
        // Get GUID of this object, index 0 will be the anchor
        if( i == 0 )
            Guid = Anchor;
        else
            Guid = ObjectsInBlueprint.GetAt( i-1 );

        // Get object form GUID
        object *pObject = g_ObjMgr.GetObjectByGuid( Guid );
        if( pObject )
        {
            // Grow the BBox
            if( i == 0 )
                B = pObject->GetBBox();
            else
                B += pObject->GetBBox();
        }
    }

    // Return the BBox
    return B;
}

//=========================================================================
//=========================================================================
// editor_zone_ref
//=========================================================================
//=========================================================================

//=========================================================================

editor_zone_ref::editor_zone_ref( void )
{
    MinPlayers      =  0;
    MaxPlayers      = 32;
    EnvMap[0]       = '\0';
    FogMap[0]       = '\0';
    QuickFog        = FALSE;
    SndAbsorption   = 1.0f;
}

//=========================================================================

void editor_zone_ref::OnEnumProp  ( prop_enum&    List )
{
    List.PropEnumHeader  ( "Zone",                  "Zone Properties", 0 );
    List.PropEnumString  ( "Zone\\Name",            "Name of the Zone", PROP_TYPE_READ_ONLY );
    List.PropEnumInt     ( "Zone\\Id",              "Internal ID of the Zone", PROP_TYPE_READ_ONLY );
    List.PropEnumString  ( "Zone\\Layer",           "Currently assigned Layer", PROP_TYPE_READ_ONLY );
    List.PropEnumInt     ( "Zone\\VertCount",       "Number of RigidInst Verts in this zone", PROP_TYPE_READ_ONLY );
    List.PropEnumInt     ( "Zone\\InstanceCount",   "Number of objects in this zone", PROP_TYPE_READ_ONLY );
    List.PropEnumFloat   ( "Zone\\SndAbsorption",   "Sound absorption coefficient for the zone, how much sound is allowed through 0 -> 1", 0 );
    List.PropEnumInt     ( "Zone\\MinPlayers",      "Minimum number of players needed in game for zone to stay active.", 0 );
    List.PropEnumInt     ( "Zone\\MaxPlayers",      "Maximum number of players allowed in game for zone to stay active.", 0 );
    List.PropEnumExternal( "Zone\\EnvironmentMap",  "Resource\0envmap", "Cube environment map resource that is used for this zone.", 0 );
    List.PropEnumExternal( "Zone\\FogMap",          "Resource\0xbmp",   "Fog map resource that specifies the fog falloff for this zone.", 0 );
    List.PropEnumBool    ( "Zone\\QuickFog",        "Fog transitions into this zone will be immediate instead of gradual.", 0 );
}

//=========================================================================

xbool editor_zone_ref::OnProperty  ( prop_query&   I    )
{
    if( I.IsVar( "Zone\\Name" ) && I.IsRead())
    {
        I.SetVarString( Name, MAX_PATH );
    }
    else if( I.IsVar( "Zone\\Id" ) && I.IsRead())
    {
        I.SetVarInt( Id );
    }
    else if( I.IsVar( "Zone\\Layer" ) && I.IsRead())
    {
        I.SetVarString( Layer, MAX_PATH );
    }
    else if( I.IsVar( "Zone\\SndAbsorption" ))
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( SndAbsorption );
        }
        else
        {
            SndAbsorption = I.GetVarFloat();
            if( SndAbsorption < 0.01f )
                SndAbsorption = 0.01f;
        }
    }
    else if( I.VarInt( "Zone\\MinPlayers", MinPlayers, 0, 32 ) )
    {
    }
    else if( I.VarInt( "Zone\\MaxPlayers", MaxPlayers, 0, 32 ) )
    {
    }
    else if( I.VarExternal( "Zone\\EnvironmentMap", EnvMap, 256) )
    {
    }
    else if( I.VarExternal( "Zone\\FogMap", FogMap, 256) )
    {
    }
    else if( I.VarBool( "Zone\\QuickFog", QuickFog ) )
    {
    }
    else if( I.IsVar( "Zone\\VertCount" ) && I.IsRead())
    {
        s32 nVertCount = 0;
        for( s32 i = 0; i < (s32)object::TYPE_END_OF_LIST; i++ )
        {
            slot_id SlotID = g_ObjMgr.GetFirst( ( object::type)i );

            while( SlotID != SLOT_NULL )
            {
                // Better exit one of this object if we have an slot for it
                object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
                SlotID = g_ObjMgr.GetNext(SlotID);

                if (pObject && (( pObject->GetZone1() == Id ) || ( pObject->GetZone2() == Id )) )
                {
                    rigid_inst* pRigidInst = g_WorldEditor.GetRigidInstForObject(pObject);
                    if (pRigidInst)
                    {
                        rigid_geom* pGeom = pRigidInst->GetRigidGeom();
                        if (pGeom)
                        {
                            nVertCount += pGeom->GetNVerts();
                        }
                    }
                }
            }
        }

        I.SetVarInt( nVertCount );
    }
    else if( I.IsVar( "Zone\\InstanceCount" ) && I.IsRead())
    {
        s32 nInstanceCount = 0;
        for( s32 i = 0; i < (s32)object::TYPE_END_OF_LIST; i++ )
        {
            slot_id SlotID = g_ObjMgr.GetFirst( ( object::type)i );

            while( SlotID != SLOT_NULL )
            {
                // Better exit one of this object if we have an slot for it
                object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
                SlotID = g_ObjMgr.GetNext(SlotID);

                if (pObject && (( pObject->GetZone1() == Id ) || ( pObject->GetZone2() == Id )) )
                {
                    nInstanceCount += 1;
                }
            }
        }

        I.SetVarInt( nInstanceCount );
    }    
    else
    {   
        return FALSE;        
    }

    return TRUE;
}

//=========================================================================
//=========================================================================
// editor_dif_prop_list
//=========================================================================
//=========================================================================

//helper function
xbool IsIgnoredTypeProp( const char* pString )
{
    // Which ones don't we want to save
    // HARDCODED per Tomas's request
    // This may introduce a bug we saw earlier but we can't remember what
    if( x_strcmp("Base\\Position", pString) == 0)
        return TRUE;

    if( x_strcmp("Base\\Rotation", pString) == 0)
        return TRUE;

    if( x_strcmp("Base\\ZoneInfo", pString) == 0)
        return TRUE;

    return FALSE;
}

//=========================================================================

void editor_dif_prop_list::OnSave( text_out& TextOut )
{
    s32             i;
    s32             nProp=0;

    // Do a quick pass and find out how many properties are we going to save
    for( i=0; i < m_ObjProps.GetCount(); i++ )
    {
        prop_container& pc = m_ObjProps.GetAt(i);

        if( pc.GetTypeFlags() & PROP_TYPE_DONT_SAVE )
            continue;

        if( pc.GetTypeFlags() & PROP_TYPE_READ_ONLY )
            continue;

        if( pc.GetTypeFlags() & PROP_TYPE_HEADER )
            continue;

        if( pc.GetType() == PROP_TYPE_BUTTON )
            continue;

        if (IsIgnoredTypeProp(pc.GetName()))
            continue;

        nProp++;
    }

    TextOut.AddHeader("BP_OverwriteData",1);
    TextOut.AddGuid("Guid", m_ObjGuid);
    TextOut.AddEndLine();

    // Okay now lets go for it
    TextOut.AddHeader( "Properties", nProp );
    for( i=0; i < m_ObjProps.GetCount(); i++ )
    {
        prop_container& pc = m_ObjProps.GetAt(i);

        if( pc.GetTypeFlags() & PROP_TYPE_DONT_SAVE )
            continue;

        if( pc.GetTypeFlags() & PROP_TYPE_READ_ONLY )
            continue;

        if( pc.GetTypeFlags() & PROP_TYPE_HEADER )
            continue;

        if( pc.GetType()  == PROP_TYPE_BUTTON )
            continue;

        if (IsIgnoredTypeProp(pc.GetName()))
            continue;

        TextOut.AddField( "Name:s", pc.GetName() );

        switch( pc.GetType() )
        {
        case PROP_TYPE_FLOAT    :
            {
                f32 Float;
                pc.GetFloat(Float);
                TextOut.AddField( "Type:s", "FLOAT" );
                TextOut.AddField( "Value:s", (const char*)xfs("%g", Float) );
                break;
            }
        case PROP_TYPE_INT      :
            {
                s32 Int;
                pc.GetInt(Int);
                TextOut.AddField( "Type:s", "INT" );
                TextOut.AddField( "Value:s", (const char*)xfs("%d", Int) );
                break;
            }
        case PROP_TYPE_BOOL     :
            {
                xbool Bool;
                pc.GetBool(Bool);
                TextOut.AddField( "Type:s", "BOOL" );
                TextOut.AddField( "Value:s", (const char*)xfs("%d", Bool) );
                break;
            }
        case PROP_TYPE_VECTOR3  :  
            {
                vector3 Vector3;
                pc.GetVector3(Vector3);
                TextOut.AddField( "Type:s", "VECTOR3" );
                TextOut.AddField( "Value:s", (const char*)xfs("%f %f %f", Vector3.GetX(), Vector3.GetY(), Vector3.GetZ() ) );
                break;
            }
        case PROP_TYPE_ROTATION :
            {
                radian3 Rotation;
                pc.GetRotation(Rotation);
                TextOut.AddField( "Type:s", "ROTATION" );
                TextOut.AddField( "Value:s", (const char*)xfs("%f %f %f", 
                    RAD_TO_DEG(Rotation.Roll), 
                    RAD_TO_DEG(Rotation.Pitch), 
                    RAD_TO_DEG(Rotation.Yaw) ) );
                break;
            }
        case PROP_TYPE_ANGLE    :
            {
                radian Angle;
                pc.GetAngle(Angle);
                TextOut.AddField( "Type:s", "ANGLE" );
                TextOut.AddField( "Value:s", (const char*)xfs("%g", RAD_TO_DEG(Angle) ) );
                break;
            }
        case PROP_TYPE_BBOX     :
            {
                bbox BBox;
                pc.GetBBox(BBox);
                TextOut.AddField( "Type:s", "BBOX" );
                TextOut.AddField( "Value:s", (const char*)xfs("%g %g %g %g %g %g", 
                    BBox.Min.GetX(), BBox.Min.GetY(), BBox.Min.GetZ(),
                    BBox.Max.GetX(), BBox.Max.GetY(), BBox.Max.GetZ()) );
                break;
            }
        case PROP_TYPE_GUID     :
            {
                guid Guid;
                pc.GetGUID(Guid);
                TextOut.AddField( "Type:s", "GUID" );
                TextOut.AddField( "Value:s", (const char*)xfs("%d %d", 
                    (s32)Guid.Guid, (s32)(Guid.Guid>>32) ));
                break;
            }
        case PROP_TYPE_COLOR    :
            {
                xcolor Color;
                pc.GetColor(Color);
                const f32 K = 1/255.0f;
                TextOut.AddField( "Type:s", "COLOR" );
                TextOut.AddField( "Value:s", (const char*)xfs("%f %f %f %f", 
                    Color.R*K, Color.G*K, Color.B*K, Color.A*K ) );
                break;
            }
        case PROP_TYPE_STRING   :
            {
                char String[256];
                pc.GetString(String);
                TextOut.AddField( "Type:s", "STRING" );
                TextOut.AddField( "Value:s", String );
                break;
            }
        case PROP_TYPE_ENUM     :
            {
                char Enum[256];
                pc.GetEnum(Enum);
                TextOut.AddField( "Type:s", "ENUM" );
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
                pc.GetExternal(Enum);
                TextOut.AddField( "Type:s", "EXTERNAL" );
                TextOut.AddField( "Value:s", Enum );
                break;   
            }
        case PROP_TYPE_FILENAME :
            {
                char FileName[256];
                pc.GetFileName(FileName);
                TextOut.AddField( "Type:s", "FILENAME" );
                TextOut.AddField( "Value:s", FileName );
                break;
            }
        }

        // End of line
        TextOut.AddEndLine();
    }
}

//=========================================================================

void editor_dif_prop_list::OnLoad( text_in& TextIn )
{
    m_ObjGuid = 0;
    m_ObjProps.Clear();
    
    x_try;

    TextIn.ReadHeader();
    if (x_stricmp( TextIn.GetHeaderName(), "BP_OverwriteData" ) == 0)
    {
        TextIn.ReadFields();
        guid BPGuid;
        if (TextIn.GetGuid("Guid",BPGuid) && (BPGuid != 0)) 
        {
            m_ObjGuid = BPGuid;

            s32                i;

            TextIn.ReadHeader();

            if( x_strcmp( TextIn.GetHeaderName(), "Properties" ) != 0 )
                x_throw( "Unable to load the properties for the blueprint mod." );

            // Okay now lets go for it
            s32 Count = TextIn.GetHeaderCount();
            for( i=0; i < Count; i++ )
            {
                TextIn.ReadFields();

                char Name [256];
                char Type [256];
                char Value[256];
                TextIn.GetField( "Name:s",  Name   );
                TextIn.GetField( "Type:s",  Type   );
                TextIn.GetField( "Value:s", Value  );

                if (!IsIgnoredTypeProp(Name))
                {
                    prop_container Container;

                    if( x_strcmp( Type, "FLOAT") == 0 )
                    {
                        f32 Float = x_atof( Value );
                        Container.InitFloat( Name, Float );
                    }
                    else if( x_strcmp( Type, "INT") == 0 )
                    {
                        s32 Int = x_atoi( Value );
                        Container.InitInt( Name, Int );
                    }
                    else if( x_strcmp( Type, "BOOL") == 0 )
                    {
                        xbool Bool = x_atoi( Value );
                        Container.InitBool( Name, Bool );
                    }
                    else if( x_strcmp( Type, "VECTOR3") == 0 )
                    {
                        vector3 Vector3;
                        sscanf( Value, "%f %f %f", &Vector3.GetX(), &Vector3.GetY(), &Vector3.GetZ() );
                        Container.InitVector3( Name, Vector3 );
                    }
                    else if( x_strcmp( Type, "ROTATION") == 0 )
                    {
                        radian3 Rotation;
                        sscanf( Value, "%f %f %f", &Rotation.Roll, &Rotation.Pitch, &Rotation.Yaw );

                        Rotation.Roll  = DEG_TO_RAD(Rotation.Roll);
                        Rotation.Pitch = DEG_TO_RAD(Rotation.Pitch);
                        Rotation.Yaw   = DEG_TO_RAD(Rotation.Yaw);

                        Container.InitRotation( Name, Rotation );
                    }
                    else if( x_strcmp( Type, "ANGLE") == 0 )
                    {
                        radian Angle = x_atof( Value );
                        Angle        = DEG_TO_RAD(Angle);
                        Container.InitAngle( Name, Angle );
                    }
                    else if( x_strcmp( Type, "BBOX") == 0 )
                    {
                        bbox BBox;
                        sscanf( Value, "%f %f %f %f %f %f", 
                            &BBox.Min.GetX(), &BBox.Min.GetY(), &BBox.Min.GetZ(),
                            &BBox.Max.GetX(), &BBox.Max.GetY(), &BBox.Max.GetZ() );
                        Container.InitBBox( Name, BBox );
                    }
                    else if( x_strcmp( Type, "GUID") == 0 )
                    {
                        s32 A, B;
                        guid Guid;
                        sscanf( Value, "%d %d", &A, &B );
                        Guid.Guid = ((u32)A) | (((u64)B)<<32);
                        Container.InitGUID( Name, Guid );
                    }
                    else if( x_strcmp( Type, "COLOR") == 0 )
                    {
                        f32 R, G, B, A;
                        xcolor Color;
                        sscanf( Value, "%f %f %f %f", &R, &G, &B, &A );

                        Color.SetfRGBA( R, G, B, A );

                        Container.InitColor( Name, Color );
                    }
                    else if( x_strcmp( Type, "STRING") == 0 )
                    {
                        Container.InitString( Name, Value );
                    }
                    else if( x_strcmp( Type, "ENUM") == 0 )
                    {
                        Container.InitEnum( Name, Value );
                    }
                    else if( x_strcmp( Type, "EXTERNAL") == 0 )
                    {
                        Container.InitExternal( Name, Value );
                    }
                    else if( x_strcmp( Type, "FILENAME") == 0 )
                    {
                        Container.InitFileName( Name, Value );
                    }
                    else
                    {
                        ASSERT( 0 );
                    }

                    m_ObjProps.Append(Container);
                }
            }
        }
    }
    x_catch_display;
}

//=========================================================================
//=========================================================================
// world_editor
//=========================================================================
//=========================================================================

world_editor::world_editor() 
{
    m_guidsSelectedObjs.Clear();
    m_guidsSelectedObjs.SetCount(0);

    // Clear ray cast overlap guid vars and pre-allocate max size array
    m_RayCastOverlapGuids.SetCount(MAX_COLLISION_MGR_COLLISIONS) ;
    m_RayCastOverlapGuids.SetLocked(TRUE) ;
    m_iRayCastOverlapGuid = 0;

    AddLayer(GetDefaultLayer( ), FALSE);
    AddLayer(GetGlobalLayer( ), FALSE);
    m_xstrActiveLayer = GetDefaultLayer( );
    m_xstrActiveLayerPath = "\\";
    m_pHandler = NULL;
    m_pUndoSelectData = NULL;
    m_pCurrentUndoEntry = NULL;
    m_SelectedZone = 0;
    m_CapturedGuid.Guid = 0;
    ClearTemporaryObjects();
    ClearHoverSelection();
    m_bInHoverSelectMode = FALSE;
    m_bHoverSelectForProperty = FALSE;
    m_xstrHoverProperty.Clear();
    m_bLogGuids = FALSE;
    
    m_NullLayer.IsNull = TRUE;

    m_bIgnoreReadOnlyChecksForDebug = FALSE;

    //autobuild info
    m_bAutoBuilding  = FALSE;
    m_bForceShutdown = FALSE;
    m_bRefreshRsc    = FALSE;   

    m_LastSelectedGuidA = 0;
    m_LastSelectedGuidB = 0;

    m_DisableDirtyTracking = 0;
}

//=========================================================================

world_editor::~world_editor() 
{
    ClearUndoList( );
    ClearUndoEntry( );
}

//=========================================================================
// Munge Name Functions
//=========================================================================

//=========================================================================

void world_editor::GetDisplayNameForObject( guid ObjectGuid, xstring& xstrName )
{
    object *pObject = g_ObjMgr.GetObjectByGuid(ObjectGuid);
    xstring xstrType;

    if (pObject) 
    {
        
        if (x_strlen(pObject->GetName()))
        {
            xstrName.Format("%s", pObject->GetName());
        }
        else
        {
            xstrName.Format("(%s)",pObject->GetTypeDesc().GetTypeName());
        } 
        
/*
        if (pObject->GetType() == object::TYPE_ZONE_PORTAL)
        {
            prop_query pqRead;
            char cName[MAX_PORTAL_NAME];
            pqRead.RQueryString( "Base\\Name", &cName[0]);
            pObject->OnProperty( pqRead );
            xstrName.Format("%s",cName);
        }
        else
        {
            xstrName.Format("%s(%s)",pObject->GetTypeDesc().GetTypeName(),guid_ToString(ObjectGuid));
        }
*/
    }
    else
    {
        xstrName = xstring("UNDEFINED");
    }
}

//=========================================================================

void world_editor::GetDisplayNameForBlueprint( const editor_blueprint_ref& Blueprint, xstring& xstrName )
{
    xstrName.Format("[%s]%s(%d Objs)",
        (const char*)Blueprint.ThemeName,
        (const char*)Blueprint.RelativePath,
        Blueprint.ObjectsInBlueprint.GetCount());
}

//=========================================================================

void world_editor::GetDisplayNameForBlueprint( guid BlueprintGuid, xstring& xstrName )
{
    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(i);
        for (s32 j = 0; j < eLayer.Blueprints.GetCount(); j++)
		{
            editor_blueprint_ref& Blueprint = eLayer.Blueprints.GetAt(j);
            if (Blueprint.Guid == BlueprintGuid)
            {
                GetDisplayNameForBlueprint(Blueprint, xstrName);
                return;
            }
        }
    }
    xstrName.Format("Unknown Blueprint(%s)",(const char*)guid_ToString(BlueprintGuid));
}

//=========================================================================
// Prop interface handling
//=========================================================================

void world_editor::OnEnumProp( prop_enum& List )
{
    x_try;

    xbool bCanGuidDebug   = FALSE;
    m_bObjectGroupChanges = FALSE;
    m_TempPropGuid        = 0;

    //show zone properties rather than object properties
    if (m_SelectedZone != 0)
    {
        if (m_pHandler) m_pHandler->AlterGridColor(PROP_COLOR_ZONE);

        for( s32 i=0; i < m_listZones.GetCount(); i++ )
        {       
            editor_zone_ref &Zone = m_listZones.GetAt(i);
            if ( Zone.Id == m_SelectedZone )
            {
                Zone.OnEnumProp(List);
            }
        }
        return;
    }

    object *pObject = NULL;
    if (m_guidLstTempObject.GetCount() > 0)
    {
        //do OnEnumProp of temporary objects
        if (m_guidLstTempObject.GetCount()==1)
        {
            if (m_pHandler) m_pHandler->AlterGridColor(PROP_COLOR_1TEMPOBJ);

            //one object selected
            guid& ObjectGuid = m_guidLstTempObject.GetAt(0);
            pObject = g_ObjMgr.GetObjectByGuid(ObjectGuid);
        }
        else
        {
            if (m_pHandler) m_pHandler->AlterGridColor(PROP_COLOR_TEMPOBJS);

            //multiple objects selected
            List.PropEnumHeader  ( "Multiple Objects",            "You have multiple placement objects selected.", 0 );
            List.PropEnumVector3 ( "Multiple Objects\\Position",  "Position of the object in world space", 0 );
            List.PropEnumRotation( "Multiple Objects\\Rotation",  "Rotation of the object in world space", 0 );
        }
    }
    else //do OnEnumProp of select objects
    {
        if (m_guidsSelectedObjs.GetCount()>0)
        {
            List.PropEnumHeader("EditorOptions", "Editor side only options for the selected object.", 0);
            List.PropEnumButton("EditorOptions\\MoveToZone", "Option to move the object to a specified zone.", 0);
        }

        if (m_guidsSelectedObjs.GetCount()<=0)
        {
            //nothing selected
        }
        else if (m_guidsSelectedObjs.GetCount()==1)
        {
            if (m_pHandler) m_pHandler->AlterGridColor(PROP_COLOR_1OBJECT);

            //one object selected
            guid& ObjectGuid = m_guidsSelectedObjs.GetAt(0);
            pObject = g_ObjMgr.GetObjectByGuid(ObjectGuid);

            if (pObject && pObject->GetTypeDesc().IsDynamic())
            {
                bCanGuidDebug = TRUE;
            }
        }
        else
        {
            editor_blueprint_ref BPEditRef;
            xbool bEditBlueprint = FALSE;
            //check if we have 1 editable blueprint selected
            if (IsOneBlueprintSelected( BPEditRef ))
            {
                //yes, does it only have one object?
                if (BPEditRef.ObjectsInBlueprint.GetCount() == 1)
                {
                    guid ObjToEdit = BPEditRef.ObjectsInBlueprint.GetAt(0);
                    if (ObjToEdit)
                    {
                        pObject = g_ObjMgr.GetObjectByGuid(ObjToEdit);
                        bEditBlueprint = TRUE;
                 
                        if (pObject && pObject->GetTypeDesc().IsDynamic())
                        {
                            bCanGuidDebug = TRUE;
                        }
                        
                        if (m_pHandler) m_pHandler->AlterGridColor(PROP_COLOR_BP_MOD);    
                    }
                }
            }
            
            if (!bEditBlueprint)
            {
                //must determine if all selected objects are the same type
                char cObjType[MAX_PATH];
                cObjType[0]=0;
                xbool bAllMatch = TRUE;
                for (s32 i = 0; i < m_guidsSelectedObjs.GetCount(); i++)
                {
                    guid& ObjectGuid = m_guidsSelectedObjs.GetAt(i);
                    pObject = g_ObjMgr.GetObjectByGuid(ObjectGuid);
                    if (pObject)
                    {
                        //hack to handle tasklist, trigger crashes on multiselect
                        if (pObject->GetType() == object::TYPE_TRIGGER_EX || 
                            pObject->GetType() == object::TYPE_TRIGGER ||
                            pObject->GetType() == object::TYPE_SPATIAL_TRIGGER || 
                            pObject->GetType() == object::TYPE_VIEWABLE_SPATIAL_TRIGGER || 
                            pObject->GetType() == object::TYPE_CHARACTER_TASK )
                        {
                            //pretend they don't all match
                            bAllMatch = FALSE;
                            break;
                        }

                        if (x_strlen(cObjType)==0) 
                        {
                            x_strcpy(cObjType, pObject->GetTypeDesc().GetTypeName());
                        }
                        else if (x_strcmp(cObjType, pObject->GetTypeDesc().GetTypeName()) != 0)
                        {
                            //they don't all match
                            bAllMatch = FALSE;
                            break;
                        }
                    }
                }

                if (bAllMatch)
                {
                    if (m_pHandler) m_pHandler->AlterGridColor(PROP_COLOR_MULTI_SAME_OBJS);    

                    List.PropEnumHeader  ( "Multiple Objects",            "Currently you have multiple objects of the same type selected.", 0 );
                    m_bObjectGroupChanges = TRUE;
                }
                else
                {
                    if (m_pHandler) m_pHandler->AlterGridColor(PROP_COLOR_MULTI_OBJS);    

                    pObject = NULL; //must set to null

                    //multiple objects selected
                    List.PropEnumHeader  ( "Multiple Objects",            "Currently you have a mix of objects and blueprints selected.", 0 );
                }
                List.PropEnumVector3 ( "Multiple Objects\\Position",  "Position of the object in world space", 0 );
                List.PropEnumRotation( "Multiple Objects\\Rotation",  "Rotation of the object in world space", 0 );
            }
        }
    }

    //if pObject was set, then we enum for it
    if (pObject) 
    {
        if (bCanGuidDebug)
        {
            m_TempPropGuid = pObject->GetGuid();
        }
        pObject->OnEnumProp( List );
    }

    x_catch_display;
}

//=========================================================================

xbool world_editor::ExternalProperties( prop_query& I, prop_interface& PropInterface )
{
    if( !I.IsRead() )
    {
        if (m_pHandler)
        {
            xstring xstrType;
            xstring xstrEnumName;

            DetermineExternalType( &PropInterface, I.GetName(), xstrType, xstrEnumName, &I );

            if (!xstrType.IsEmpty())
            {
                xstring xstrValue;
                if (m_pHandler->HandleExternal(xstrType, xstrValue))
                {
                    prop_query pq;
                    pq.WQueryExternal( xstrEnumName, xstrValue );

		            xbool bSuccess = PropInterface.OnProperty(pq);

                    return bSuccess;
                }
            }
        }
    }
    else
    {
        return PropInterface.OnProperty( I );
    }

    return FALSE;
}

//=========================================================================

xbool world_editor::OnProperty( prop_query& I )
{
    x_try;

    //show zone properties rather than object properties
    if (m_SelectedZone != 0)
    {
        for( s32 i=0; i < m_listZones.GetCount(); i++ )
        {       
            editor_zone_ref &Zone = m_listZones.GetAt(i);
            if ( Zone.Id == m_SelectedZone )
            {
                if( I.GetQueryType() == PROP_TYPE_EXTERNAL )
                {
                    return ExternalProperties( I, Zone );
                }
                else
                {
                    return Zone.OnProperty(I);
                }
            }
        }
    }

    //handle special editor properties
    if( I.IsVar( "EditorOptions\\MoveToZone" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Pick Zone" );
        }
        else
        {
            if (m_pHandler)
                m_pHandler->ChangeSelectObjectsZone();
        }
        
        return TRUE;
    }

    object *pObject = NULL;
    xbool bIsTempObjects = FALSE;

    if (m_guidLstTempObject.GetCount() > 0)
    {
        //do OnProperty of temporary objects
        bIsTempObjects = TRUE;
        if (m_guidLstTempObject.GetCount()==1)
        {
            //one object selected
            guid& ObjectGuid = m_guidLstTempObject.GetAt(0);
            pObject = g_ObjMgr.GetObjectByGuid(ObjectGuid);
        }
        else
        {
            //multiple objects selected
            if( I.IsVar( "Multiple Objects\\Position" ) )
            {
                if( I.IsRead() )
                {
                    bbox Bounds = GetTemporaryObjectsBounds();
                    I.SetVarVector3( Bounds.GetCenter() );
                }
                else
                {
                    MoveTemporaryObjects( I.GetVarVector3() );
                }
                return TRUE;
            }
            else if( I.IsVar( "Multiple Objects\\Rotation" ) )
            {
                if( I.IsRead() )
                {
                    I.SetVarRotation( radian3(0,0,0) );
                }
                else
                {
                    radian3 r3;
                    r3.Roll = I.GetVarRotation().Roll;
                    r3.Pitch = I.GetVarRotation().Pitch;
                    r3.Yaw = I.GetVarRotation().Yaw;

                    RotateTemporaryObjects( r3 );
                }
                return TRUE;
            }
        }
    }
    else //do OnProperty of select objects
    {
        if (m_guidsSelectedObjs.GetCount()<=0)
        {
            //nothing selected
            return FALSE;
        }
        else if (m_guidsSelectedObjs.GetCount()==1)
        {
            //one object selected
            guid& ObjectGuid = m_guidsSelectedObjs.GetAt(0);
            pObject = g_ObjMgr.GetObjectByGuid(ObjectGuid);
        }
        else
        {
            editor_blueprint_ref BPEditRef;
            xbool bEditBlueprint = FALSE;
            
            //check if we have 1 editable blueprint selected
            if (IsOneBlueprintSelected( BPEditRef ))
            {
                //yes, does it only have one object?
                if (BPEditRef.ObjectsInBlueprint.GetCount() == 1)
                {
                    guid ObjToEdit = BPEditRef.ObjectsInBlueprint.GetAt(0);
                    if (ObjToEdit)
                    {
                        pObject = g_ObjMgr.GetObjectByGuid(ObjToEdit);
                        if( pObject )
                        {
                            xbool IsActor = pObject->IsKindOf( actor::GetRTTI() );
                            bEditBlueprint = TRUE;

                            // Lookup blue print reference
                            editor_blueprint_ref* pBlueprintRef = NULL;
                            GetBlueprintRefContainingObject2( ObjToEdit, &pBlueprintRef );

                            // Lookup anchor
                            object* pAnchor = g_ObjMgr.GetObjectByGuid( BPEditRef.Anchor );
                            if( ( pAnchor ) && ( pBlueprintRef ) )
                            {
                                // SB: Fixes editing a blue-print that contains a single object
                                ASSERTS( g_level_loading == FALSE, "Should only ever come here from editing!" );
                                
                                // Transform properties should all be applied to the anchor -
                                // the single objects transform relative to the anchor should just be maintained!
                                if(     ( I.IsVar( "Base\\Position" ) )
                                    ||  ( I.IsVar( "Base\\Rotation" ) ) )
                                {
                                    // Lookup anchor transform
                                    matrix4 AnchorL2W = pAnchor->GetL2W();
                                
                                    // Writing to anchor object?
                                    if( I.IsRead() == FALSE )
                                    {
                                        // Begin undo
                                        xbool bCommitUndo = ( m_pCurrentUndoEntry == NULL );
                                        if( bCommitUndo )
                                        {
                                            SetCurrentUndoEntry( new transaction_entry( "Property Moving Blueprint with single object" ) );
                                        }
                                                                            
                                        // Create undo data for blue-print reference (contains orientation)
                                        transaction_bpref_data* pRefData = new transaction_bpref_data(
                                            transaction_bpref_data::BPREF_EDIT,
                                            pBlueprintRef->Guid );
                                                                            
                                        // Create undo data for anchor
                                        transaction_object_data* pAnchorData = new transaction_object_data(
                                            transaction_object_data::OBJ_EDIT,
                                            pAnchor->GetGuid(), pAnchor->GetTypeDesc().GetTypeName() );

                                        // Create undo data for object
                                        transaction_object_data* pObjectData = new transaction_object_data(
                                            transaction_object_data::OBJ_EDIT,
                                            pObject->GetGuid(), pObject->GetTypeDesc().GetTypeName() );

                                        // Store old properties for undo                                        
                                        pRefData->StoreReferenceData( transaction_data::TRANSACTION_OLD_STATE );
                                        pAnchorData->StoreProperties( transaction_data::TRANSACTION_OLD_STATE );
                                        pObjectData->StoreProperties( transaction_data::TRANSACTION_OLD_STATE );

                                        // Compute objects L2W relative to anchor
                                        matrix4 ObjectL2W    = pObject->GetL2W();
                                        matrix4 ObjectRelL2W = m4_InvertRT( AnchorL2W ) * ObjectL2W;
                                    
                                        // Update anchor position?
                                        if( I.IsVar( "Base\\Position" ) )
                                            AnchorL2W.SetTranslation( I.GetVarVector3() );
                                        
                                        // Update anchor rotation?
                                        if( I.IsVar( "Base\\Rotation" ) )
                                            AnchorL2W.SetRotation( I.GetVarRotation() );
                                    
                                        // Update anchor and object
                                        pAnchor->OnTransform( AnchorL2W );
                                        pObject->OnTransform( AnchorL2W * ObjectRelL2W );
                                        
                                        // Need to update blue print reference orientation also!
                                        pBlueprintRef->Transform = AnchorL2W;
                                        pBlueprintRef->Transform.ClearTranslation(); // Shouldn't contain translation!
                                        
                                        // Store new properties for undo
                                        pRefData->StoreReferenceData( transaction_data::TRANSACTION_NEW_STATE );
                                        pAnchorData->StoreProperties( transaction_data::TRANSACTION_NEW_STATE );
                                        pObjectData->StoreProperties( transaction_data::TRANSACTION_NEW_STATE );
                                        
                                        // End undo
                                        m_pCurrentUndoEntry->AddCommitStep( pRefData );
                                        m_pCurrentUndoEntry->AddCommitStep( pAnchorData );
                                        m_pCurrentUndoEntry->AddCommitStep( pObjectData );
                                        if( bCommitUndo )
                                        {
                                            CommitCurrentUndoEntry();
                                        }                                            
                                                                                
                                        // Mark the layer as dirty
                                        SetObjectsLayerAsDirty( pAnchor->GetGuid() );
                                        SetObjectsLayerAsDirty( pObject->GetGuid() );
                                    }
                                    else
                                    {
                                        // Read anchor position into UI?
                                        if( I.IsVar( "Base\\Position" ) )
                                            I.SetVarVector3( AnchorL2W.GetTranslation() );

                                        // Read anchor rotation into UI?
                                        if( I.IsVar( "Base\\Rotation" ) )
                                            I.SetVarRotation( AnchorL2W.GetRotation() );
                                    }
                                    
                                    // Done
                                    return TRUE;
                                }
                            }                                
                        }
                    }
                }
            }

            if (!bEditBlueprint)
            {
                //multiple objects selected
                if( I.IsVar( "Multiple Objects\\Position" ) )
                {
                    if( I.IsRead() )
                    {
                        bbox Bounds = GetSelectedObjectsBounds();
                        I.SetVarVector3( Bounds.GetCenter() );
                    }
                    else
                    {
                        if (!m_pCurrentUndoEntry)
                        {
                            //not already in an undo wrapper
                            SetCurrentUndoEntry(new transaction_entry(xfs("Property Moving %d Object(s)",GetSelectedCount())));
                            MoveSelectedObjectsToCenterPt( I.GetVarVector3() );
                            CommitCurrentUndoEntry();
                        }
                        else
                        {
                            MoveSelectedObjectsToCenterPt( I.GetVarVector3() );
                        }
                    }
                    return TRUE;
                }
                else if( I.IsVar( "Multiple Objects\\Rotation" ) )
                {
                    if( I.IsRead() )
                    {
                        I.SetVarRotation( radian3(0,0,0) );
                    }
                    else
                    {
                        radian3 r3;
                        r3.Roll = I.GetVarRotation().Roll;
                        r3.Pitch = I.GetVarRotation().Pitch;
                        r3.Yaw = I.GetVarRotation().Yaw;

                        if (!m_pCurrentUndoEntry)
                        {
                            //not already in an undo wrapper
                            SetCurrentUndoEntry(new transaction_entry(xfs("Property Rotate %d Objects",GetSelectedCount())));
                            RotateSelectedObjects( r3 );
                            CommitCurrentUndoEntry();
                        }
                        else
                        {
                            RotateSelectedObjects( r3 );
                        }
                    }
                    return TRUE;
                }
                else if (m_bObjectGroupChanges)
                {
                    //ok they have a bunch of the same type objects selected              
                    if (I.IsRead())
                    {
                        //read properties only can read the first object
                        guid& ObjectGuid = m_guidsSelectedObjs.GetAt(0);
                        pObject = g_ObjMgr.GetObjectByGuid(ObjectGuid);
                        return pObject->OnProperty(I);
                    }
                    else
                    {
                        //must do the write to all objects
                        xbool bSuccess = TRUE;
                        prop_query pq;
                        xstring xstrValue;

                        if (I.GetQueryType() == PROP_TYPE_EXTERNAL)
                        {
                            xstring xstrType;
                            xstring xstrEnumName;
                            guid& ObjectGuid = m_guidsSelectedObjs.GetAt(0);
                            pObject = g_ObjMgr.GetObjectByGuid(ObjectGuid);
                            DetermineExternalType(pObject, I.GetName(), xstrType, xstrEnumName, &I );
                            if (!xstrType.IsEmpty())
                            {
                                if (m_pHandler && m_pHandler->HandleExternal(xstrType, xstrValue))
                                {
                                    pq.WQueryExternal( xstrEnumName, xstrValue );
                                }
                                else
                                {
                                    return TRUE;
                                }
                            }
                            else
                            {
                                x_DebugMsg("Error while trying to determine external type.\n");   
                                return FALSE;
                            }
                        }
                        else
                        {
                            pq = I;
                        }

                        //set up undo entry
                        guid& ObjectGuid = m_guidsSelectedObjs.GetAt(0);
                        SetCurrentUndoEntry(new transaction_entry(xfs("Edit Multiple Objects::%s",g_WorldEditor.GetObjectTypeName(ObjectGuid))));

                        for (s32 i = 0; i < m_guidsSelectedObjs.GetCount(); i++)
                        {
                            guid& ObjectGuid = m_guidsSelectedObjs.GetAt(i);
                            pObject = g_ObjMgr.GetObjectByGuid(ObjectGuid);

                            //UNDO for object change
                            transaction_object_data* pObjData = new transaction_object_data(
                                           transaction_object_data::OBJ_EDIT,
                                           pObject->GetGuid(), pObject->GetTypeDesc().GetTypeName() );
                            pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);

                            if (pObject && pObject->OnProperty(pq))
                            {
                                //set layer as dirty
                                SetObjectsLayerAsDirty(pObject->GetGuid());
                            }
                            else //success
                            {
                                bSuccess = FALSE;
                            }

                            //complete undo
                            pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                            AddStepToCurrentUndoEntry(pObjData);                    
                        }
                        CommitCurrentUndoEntry();

                        return bSuccess;
                    }
                }
            }
        }    
    }

    //if pObject was set, then we OnProperty for it
    if (pObject) 
    {
        if ( I.GetQueryType() == PROP_TYPE_EXTERNAL )
        {
            if( !I.IsRead() )
            {
                if (m_pHandler)
                {
                    xstring xstrType;
                    xstring xstrEnumName;

                    DetermineExternalType( pObject, I.GetName(), xstrType, xstrEnumName, &I );

                    if (!xstrType.IsEmpty())
                    {
                        xstring xstrValue;
                        if (m_pHandler->HandleExternal(xstrType, xstrValue))
                        {
                            prop_query pq;
                            pq.WQueryExternal( xstrEnumName, xstrValue );

                            //prepare undo ptrs
                            transaction_object_data* pObjData = NULL;
                            if ( !I.IsRead() && !bIsTempObjects)
                            {
                                //set layer as dirty
                                SetObjectsLayerAsDirty(pObject->GetGuid());

                                if (!m_pCurrentUndoEntry)
                                {
                                    //set up undo entry
                                    SetCurrentUndoEntry(new transaction_entry(xfs("Object Props Edit::%s",I.GetName())));

                                    //UNDO for object change
                                    pObjData = new transaction_object_data(
                                                   transaction_object_data::OBJ_EDIT,
                                                   pObject->GetGuid(), pObject->GetTypeDesc().GetTypeName() );
                                    pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);
                                }
                            }

		                    xbool bSuccess = (pObject->OnProperty(pq));

                            if (bSuccess)
                            {
                                //HACK
                                if (pObject->GetType() == object::TYPE_ZONE_PORTAL)
                                {
                                    //ok, gotta update zone numbers
                                    if (x_strcmp(I.GetName(),"Portal\\1st Zone") == 0)
                                    {
                                        zone_portal* pPortal = &zone_portal::GetSafeType( *pObject );
                                        if (pPortal)
                                        {
                                            pPortal->SetZone1(GetZoneId(xstrValue));
                                        }
                                    }
                                    else if (x_strcmp(I.GetName(),"Portal\\2nd Zone") == 0)
                                    {
                                        zone_portal* pPortal = &zone_portal::GetSafeType( *pObject );
                                        if (pPortal)
                                        {
                                            pPortal->SetZone2(GetZoneId(xstrValue));
                                        }
                                    }

                                    //now we need to update children with correct zones
                                    UpdateAllChildrenOfPortal(pObject->GetGuid());
                                }

                                //complete undo
                                if (pObjData)
                                {
                                    pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                                    AddStepToCurrentUndoEntry(pObjData);
                                    CommitCurrentUndoEntry();
                                }
                            }
                            else
                            {
                                //failure so cleanup undo
                                if (!bIsTempObjects && InTransaction())
                                {
                                    if (pObjData)
                                    {
                                        ClearUndoEntry();
                                    }
                                }
                            }

                            return bSuccess;
                        }
                    }
                }
            }
            else
            {
                //HACK
                if (pObject->GetType() == object::TYPE_ZONE_PORTAL)
                {
                    if (x_strcmp(I.GetName(),"Portal\\1st Zone") == 0)
                    {
                        zone_portal* pPortal = &zone_portal::GetSafeType( *pObject );
                        if (pPortal)
                        {
                            I.SetVarExternal( GetZoneForId(pPortal->GetZone1()), MAX_PATH );
                        }
                    }
                    else if (x_strcmp(I.GetName(),"Portal\\2nd Zone") == 0)
                    {
                        zone_portal* pPortal = &zone_portal::GetSafeType( *pObject );
                        if (pPortal)
                        {
                            I.SetVarExternal( GetZoneForId(pPortal->GetZone2()), MAX_PATH );
                        }
                    }
                }
            }

        }

        //prepare undo ptrs
        transaction_object_data* pObjData = NULL;
        if ( !I.IsRead() && !bIsTempObjects)
        {
            //set layer as dirty
            SetObjectsLayerAsDirty(pObject->GetGuid());

            if (!m_pCurrentUndoEntry)
            {
                //set up undo entry
                SetCurrentUndoEntry(new transaction_entry(xfs("Object Props Edit::%s",I.GetName())));

                //UNDO for object change
                pObjData = new transaction_object_data(
                               transaction_object_data::OBJ_EDIT,
                               pObject->GetGuid(), pObject->GetTypeDesc().GetTypeName() );
                pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);
            }
        }

        xbool NameChange = FALSE;
        char cOldName[MAX_OBJECT_NAME_LENGTH];
        if( !I.IsRead() && I.IsVar( "Base\\Name" ) )
        {
                NameChange = TRUE;
                prop_query pqRead;
                pqRead.RQueryString( "Base\\Name", &cOldName[0]);
                pObject->OnProperty( pqRead );
        }

        xbool bSuccess = pObject->OnProperty( I );

        if (bSuccess)
        {
            if (NameChange)
            {
                //if( pObject->GetType() == object::TYPE_ZONE_PORTAL )
                {
                    //name change, we need to update layers
                    char cNewName[MAX_OBJECT_NAME_LENGTH];
                    prop_query pqRead;
                    pqRead.RQueryString( "Base\\Name", &cNewName[0]);
                    pObject->OnProperty( pqRead );
                    RepathPortalChildren(cOldName, cNewName);              
                    if (m_pHandler) m_pHandler->ForceLayerUpdate();               
                }
            }

            //complete undo
            if (pObjData)
            {
                pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                AddStepToCurrentUndoEntry(pObjData);
                CommitCurrentUndoEntry();
            }

            //special case for the trigger view
            if (pObject->GetType() == object::TYPE_TRIGGER_EX)
            {
                if( !I.IsRead() ) 
                {
                    if (m_pHandler) m_pHandler->RefreshTriggerView();
                }
            }
        }



        /*
        xbool bPortalNameChange = FALSE;
        char cOldName[MAX_PORTAL_NAME];
        if (pObject->GetType() == object::TYPE_ZONE_PORTAL)
        {
            if( !I.IsRead() && I.IsVar( "Base\\Name" ) )
            {
                bPortalNameChange = TRUE;
                prop_query pqRead;
                pqRead.RQueryString( "Base\\Name", &cOldName[0]);
                pObject->OnProperty( pqRead );
            }
        }

        xbool bSuccess = pObject->OnProperty( I );

        if (bSuccess)
        {
            //ok, was this a portal object?
            if (bPortalNameChange)
            {
                //name change, we need to update layers
                char cNewName[MAX_PORTAL_NAME];
                prop_query pqRead;
                pqRead.RQueryString( "Base\\Name", &cNewName[0]);
                pObject->OnProperty( pqRead );
                RepathPortalChildren(cOldName, cNewName);              
                if (m_pHandler) m_pHandler->ForceLayerUpdate();               
            }

            //complete undo
            if (pObjData)
            {
                pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                AddStepToCurrentUndoEntry(pObjData);
                CommitCurrentUndoEntry();
            }

            //special case for the trigger view
            if (pObject->GetType() == object::TYPE_TRIGGER_EX)
            {
                if( !I.IsRead() ) 
                {
                    if (m_pHandler) m_pHandler->RefreshTriggerView();
                }
            }
        }
        */
        else
        {
            //failure so cleanup undo
            if (!bIsTempObjects && InTransaction())
            {
                if (pObjData)
                {
                    ClearUndoEntry();
                }
            }
        }

        return bSuccess;
    }

    x_catch_display;

    return FALSE;
}

//=========================================================================

void world_editor::DetermineExternalType( prop_interface* pPropInterface, const char* pName, xstring& xstrType, 
                                         xstring& xstrEnumName, prop_query* PropQuery )
{
    //damn externals, gotta get its enum so we can determine type
    if (pPropInterface)
    {
        prop_enum List;

        pPropInterface->OnEnumProp( List );
        for( s32 i = 0; i < List.GetCount(); i++)
        {
            prop_enum::node& enData = List[i];

            char       Temp[MAX_PATH];
            prop_query MangledName;
            xstrEnumName = enData.GetName();
            MangledName.RQueryExternal( enData.GetName(), Temp );           

            if( x_stricmp(MangledName.GetName(), pName) == 0 )
            {
                //need to check if were using an array of elements and see if were indexing into
                //the proper array, also needs to address multiarray elements.
                
                if ( PropQuery != NULL )
                {
                    xbool Match = TRUE;
                    
                    for (s32 i = 0; i < PropQuery->GetIndexCount(); i++)
                    {
                        if (MangledName.GetIndex(i) != PropQuery->GetIndex(i)) 
                        {
                            Match = FALSE;
                            break;
                        }
                    }
                    
                    if (!Match)
                    {
                        continue;
                    }
                }
                
                //found it
                if (enData.GetEnumCount() >= 2)
                {
                    xstrType = xstring(enData.GetEnumType(1));
                }
                break;
            }
        }
    }
}

//=========================================================================

void world_editor::SetExternalHandler( editor_handler* pHandler )
{
    m_pHandler = pHandler;
}

//=========================================================================

extern s32 g_Changelist;

f32 world_editor::GetVersionNumber( void )
{
    return (f32)g_Changelist;
}

//=========================================================================

s32 world_editor::GetTotalObjectCount( void )
{
    s32 nCount = 0;
    //loop through all layers
    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(i);

        nCount += eLayer.Objects.GetCount();

        xarray<editor_blueprint_ref> lstBPRef;
        if (GetBlueprintsInLayer( eLayer.Name, lstBPRef ))
        {
            for (s32 k=0; k < lstBPRef.GetCount(); k++)
            {
                editor_blueprint_ref& BPRef = lstBPRef.GetAt(k);
                nCount += (BPRef.ObjectsInBlueprint.GetCount() + 1); //add 1 for anchor
            }
        }
    }
    return nCount;
}

//=========================================================================

void world_editor::AdvanceLogic( f32 DeltaTime )
{
    g_InputMgr.Update( DeltaTime );

    d3deng_SetMouseMode( MOUSE_MODE_NEVER );

    g_ObjMgr.AdvanceAllLogic(DeltaTime);

    render::Update( DeltaTime );
    
    g_LightMgr.OnUpdate( DeltaTime );
    
    //old trigger manager
//    g_TriggerMgr.OnUpdate( DeltaTime );

    g_TriggerExMgr.OnUpdate( DeltaTime );

    g_DecalMgr.OnUpdate( DeltaTime );

    g_TracerMgr.OnUpdate( DeltaTime );

    // Limit dynamic dead bodies before advancing physics so that it doesn't get overloaded 
    // and or/run out of constraints
    corpse::LimitCount();
    
    g_PhysicsMgr.Advance( DeltaTime );

    g_ConverseMgr.Update( DeltaTime );

    g_MusicStateMgr.Update();

    g_MusicMgr.Update( DeltaTime );

	g_AudioMgr.Update( DeltaTime );

    g_GameTextMgr.Update( DeltaTime );

    g_AlienGlobMgr.Advance( DeltaTime );
}

//=========================================================================

s32 world_editor::GetObjectCount( void )
{
    s32 nCount = 0;
    for (s32 i = object::TYPE_NULL + 1; i < object::TYPE_END_OF_LIST; i++)
    {
        nCount += g_ObjMgr.GetNumInstances( (object::type)i );
    }
    return nCount;
}

//=========================================================================
// Generic object functions
//=========================================================================

bbox world_editor::GetObjectsBoundingBox( guid ObjectGuid )
{
    object *pObject = g_ObjMgr.GetObjectByGuid(ObjectGuid);
    if ( pObject )
    {
        //special case for lights
        if (pObject->GetType() == object::TYPE_LIGHT)
        {
            return bbox(pObject->GetPosition(), 40.0f);
        }
        else if(pObject->GetType() == object::TYPE_SND_EMITTER )
        {
            return bbox(pObject->GetPosition(), 100.0f);
        }
        else
        {
            return pObject->GetBBox();
        }
    }

    // Construct an empty
    bbox BBox( vector3(0.0f,0.0f,0.0f), 0.0f );
    return BBox;
}

//=========================================================================

bbox world_editor::GetBlueprintsBoundingBox( editor_blueprint_ref& Blueprint )
{
    bbox Bounds;
    x_try;
    BOOL bInit = FALSE;
    for( s32 i=0; i < Blueprint.ObjectsInBlueprint.GetCount(); i++ )
    {       
        object *pObject = g_ObjMgr.GetObjectByGuid( Blueprint.ObjectsInBlueprint.GetAt(i) );
        if( pObject )
        {
            bbox ObjBounds;
            //special case for lights
            if (pObject->GetType() == object::TYPE_LIGHT)
            {
                ObjBounds = bbox(pObject->GetPosition(), 40.0f);
            }
            else
            {
                ObjBounds = pObject->GetBBox();
            }

            if (!bInit)
            {
                Bounds = ObjBounds;
                bInit = TRUE;
            }
            else
            {
                bbox objBounds = ObjBounds;
                Bounds.AddVerts(&(objBounds.Min),1);
                Bounds.AddVerts(&(objBounds.Max),1);
            }
        }
    }

    x_catch_display;
    return Bounds;
}

//=========================================================================

bbox world_editor::GetBlueprintsBoundingBox( guid BPGuid )
{
    x_try;

    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(i);
        for (s32 j = 0; j < eLayer.Blueprints.GetCount(); j++)
		{
            editor_blueprint_ref& Blueprint = eLayer.Blueprints.GetAt(j);
            if (Blueprint.Guid == BPGuid)
            {
                return GetBlueprintsBoundingBox( Blueprint );
            }
        }
    }

    x_catch_display;
    return bbox();
}

//=========================================================================
// Layer Loading and Unloading
//=========================================================================

xbool world_editor::PreInitialize()
{
    //clear internal selection list
    g_ObjMgr.Clear();

    // Create permanent objects
    g_ObjMgr.CreateObject("god") ;

    m_guidsSelectedObjs.Clear();
    m_listZones.Clear();
    ClearLayers();
    AddLayer(GetDefaultLayer( ), FALSE);
    AddLayer(GetGlobalLayer( ), FALSE);
    m_xstrActiveLayer = GetDefaultLayer( );
    m_xstrActiveLayerPath = "\\";
    m_CapturedGuid.Guid = 0;
    g_TracerMgr.Kill();
    g_Font.Kill();
    g_RscMgr.UnloadAll();
    g_DecalMgr.Kill();
    g_PhysicsMgr.Kill();
    
    ClearHoverSelection();
    m_bInHoverSelectMode = FALSE;
    m_bHoverSelectForProperty = FALSE;
    m_xstrHoverProperty.Clear();
    
    //Clear UNDOS
    transaction_mgr::Transaction()->ClearStack();

    // re-initialize systems that have hard-coded data associated with them
    g_TracerMgr.Init();
    g_Font.Load( xfs( "%s\\%s", g_RscMgr.GetRootDirectory(), "Font_small.xbmp" ) );
    
    g_level_loading = TRUE;
    
    return TRUE;
}

//=========================================================================

xbool world_editor::PostInitialize()
{
    g_level_loading = FALSE;

    m_guidsSelectedObjs.Clear();

    // Loop through all objects and build selection list
    for( s32 i=0; i<obj_mgr::MAX_OBJECTS; i++ )
    {   
        object *pObject = g_ObjMgr.GetObjectBySlot(i);
        if (pObject && pObject->GetAttrBits() & object::ATTR_EDITOR_SELECTED )
        {
            //add this to selection list
            InternalSelectObject(pObject->GetGuid(), FALSE, FALSE);
        }
    }
    
    // Initialize corpses (so that they run their physics to settle on the ground)
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_CORPSE );
    while( SlotID != SLOT_NULL )
    {
        // Lookup corpse
        object* pObj    = g_ObjMgr.GetObjectBySlot(SlotID);
        corpse* pCorpse = &corpse::GetSafeType( *pObj );

        // Initialize
        pCorpse->InitializeEditorPlaced() ;

        // Get next corpse
        SlotID = g_ObjMgr.GetNext( SlotID );
    }
    
    return TRUE;
}

//=========================================================================

void world_editor::ClearLayers( )
{
    m_listLayers.Clear();
    m_xstrActiveLayer = GetDefaultLayer( );
    m_xstrActiveLayerPath = "\\";
}

//=========================================================================

//static s32 s_Debug_StoreCount = 0;

void world_editor::StoreState( xbool bDynamicOnly )
{
    //s_Debug_StoreCount++;
    //ASSERT(s_Debug_StoreCount == 1);

    m_ObjectsState.Clear();

    // delete all dynamic object types
    for( object_desc* pNode = object_desc::GetFirstType(); 
                      pNode; 
                      pNode = object_desc::GetNextType( pNode )  )
    {
        if( !bDynamicOnly || pNode->IsDynamic())
        {
            s32 iCount = 0;
            for( slot_id SlotID = g_ObjMgr.GetFirst( pNode->GetType() ); 
                         SlotID != SLOT_NULL; 
                         SlotID = g_ObjMgr.GetNext( SlotID ) )
            {
                object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
                ASSERT(pObject);

                if( !pObject )
                    continue;

                if (x_strcmp(pNode->GetTypeName(),pObject->GetTypeDesc().GetTypeName()) != 0)
                    continue;

                //only store non-persistent objects
                if (pObject)
                {
                    if (x_strcmp(pObject->GetTypeDesc().GetTypeName(), pNode->GetTypeName()) == 0)
                    {
                        //store this object
                        editor_state_ref ObjState;
                        ObjState.ObjGuid        = pObject->GetGuid();
                        ObjState.pObjectType    = pObject->GetTypeDesc().GetTypeName();
                        pObject->OnCopy( ObjState.ObjProps );
                        m_ObjectsState.Append( ObjState ); 
                        iCount++;
                    }
                }
            }

            if ( iCount > 0 )
            {
                x_DebugMsg("{EDITOR} Editor Stored %d Dynamic Objects of Type(%s)\n",iCount,pNode->GetTypeName());
            }
        }
    }
}

//=========================================================================

void world_editor::ResetState( xbool bDynamicOnly )
{
    DisableDirtyTracking();

    //ASSERT(s_Debug_StoreCount == 1);
    //s_Debug_StoreCount --;

    // delete all dynamic object types
    for( object_desc* pNode = object_desc::GetFirstType(); 
                      pNode; 
                      pNode = object_desc::GetNextType( pNode )  )
    {
        if( !bDynamicOnly || pNode->IsDynamic())
        {
            DeleteAllObjectsOfType( pNode->GetTypeName() );
        }
    }

    //now reset all types
    for (s32 j = 0; j < m_ObjectsState.GetCount(); j++)
    {
        editor_state_ref& ObjState = m_ObjectsState.GetAt(j);
        g_ObjMgr.CreateObject( ObjState.pObjectType, ObjState.ObjGuid );
        object *pObjNew = g_ObjMgr.GetObjectByGuid(ObjState.ObjGuid);

        if (pObjNew)
        {
            pObjNew->OnPaste( ObjState.ObjProps );            
        }
        else
        {
            ASSERT(FALSE);
            LOG_ERROR("Editor", "ResetState failed to recreate an object");
        }
    }
    
    // Reset GOD
    slot_id GodSlot = g_ObjMgr.GetFirst( object::TYPE_GOD ) ;
    if ( GodSlot != NULL )
    {
        object* pGod = g_ObjMgr.GetObjectBySlot( GodSlot ) ;
        if ( pGod )
        {
            pGod->OnActivate(TRUE) ;
        }
    }

    EnableDirtyTracking();
}

//=========================================================================

void world_editor::DeleteAllObjectsOfType( const char* pName )
{
    object::type oType = g_ObjMgr.GetTypeFromName(pName);
    slot_id SlotID = g_ObjMgr.GetFirst( oType );
    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        SlotID = g_ObjMgr.GetNext( SlotID );
        ASSERT(pObject);
        if( pObject )
        {
            if (x_strcmp(pObject->GetTypeDesc().GetTypeName(), pName) == 0)
            {
                DestroyObjectEx(pObject->GetGuid(), TRUE);
            }
        }   
    }   
}

//=========================================================================

xbool world_editor::LoadLayer(const char* pFileName, const char* pName, 
                              xarray<editor_blueprint_placement>& BPFiles,
                              xarray<editor_dif_prop_list>& BPDiffs )
{
    g_PolyCache.InvalidateAllCells();

    xbool bReturn = TRUE;
    //load single layer with full file path
    x_try;

    f32 TimeFileOpen = 0.0f;
    f32 TimeObjLoad =0.0f;
    f32 TimeBPLoad = 0.0f;
    f32 TimeRESLoad = 0.0f;
    f32 TimeObjLayerAdds = 0.0f;

    xtimer LoadTimer;
    LoadTimer.Start();

    BPFiles.Clear();
    BPDiffs.Clear();
    m_guidsSelectedObjs.Clear();

    text_in LayerFile;
    LayerFile.OpenFile( pFileName );
    if (m_pHandler)
    {
        editor_layer& Layer = GetLayerInfo(pName);
        Layer.IsEditable = !m_pHandler->IsFileReadOnly(pFileName);
    }

    TimeFileOpen += LoadTimer.TripSec();

    //AddLayer(pName, FALSE); //now done in editor doc
    f32 fVersion = 0.0f;

    while (LayerFile.ReadHeader())
    {
        if( x_stricmp( LayerFile.GetHeaderName(), "EditorInfo" ) == 0 )
        {
            LoadTimer.TripSec();
            LayerFile.ReadFields();
            LayerFile.GetF32("Version",fVersion);
            
            if (fVersion > GetVersionNumber())
            {
                x_throw(xfs("Layer (%s) was saved with a newer version of the editor, and can not be opened with this version!",pName));
                bReturn = FALSE;
            }

            TimeFileOpen += LoadTimer.TripSec();
        }
        else if( x_stricmp( LayerFile.GetHeaderName(), "OverallObjectData" ) == 0 )
        {
            LayerFile.ReadFields();
            s32 nCount = 0;

            if (LayerFile.GetS32("Count",nCount))
            {
                if (m_pHandler) m_pHandler->SetProgressRange(0,nCount);
                
                for (s32 i =0; i < nCount; i++)
                {
                    if (m_pHandler) m_pHandler->SetProgress(i);

                    LayerFile.ReadHeader();

                    x_try;
                    if (x_stricmp( LayerFile.GetHeaderName(), "Object" ) == 0)
                    {
                        LoadTimer.TripSec();

                        LayerFile.ReadFields();
                        s32 nType = -1;
                        char cType[MAX_PATH];
                        if (LayerFile.GetString("SType",cType)) //New Way
                        {
                            char cLayerPath[MAX_PATH];
                            if (!LayerFile.GetString("LayerPath",cLayerPath)) 
                            {
                                x_throw(xfs("error reading path for object (%s) during load of layer %s.",cType,pName));
                            }

                            guid GuidNewObj;
                            if (LayerFile.GetGuid("Guid",GuidNewObj) && (GuidNewObj.Guid != 0)) 
                            {
                                x_try;

                                if( x_stricmp( cType, zone_portal::GetObjectType().GetTypeName() ) == 0 )
                                {
                                    //don't load portals since we preload them from Portals.ProjectInfo file
                                }
                                else
                                {
                                    g_ObjMgr.CreateObject( cType, GuidNewObj );
                                }

                                x_catch_display;

                                x_try;

                                object *pObjNew = g_ObjMgr.GetObjectByGuid(GuidNewObj);

                                if (pObjNew)
                                {
                                    if (m_bLogGuids)
                                        m_GuidLog.Append(GuidNewObj);

                                    pObjNew->OnLoad(LayerFile);

                                    TimeObjLoad += LoadTimer.TripSec();

                                    //add object to layer
                                    editor_object_ref ObjRef;
                                    ObjRef.Guid = GuidNewObj;

                                    if ( pObjNew->GetTypeDesc().IsGlobalObject())
                                    {
                                        ObjRef.LayerPath = "\\GlobalObjs\\";
                                    }
                                    else
                                    {
                                        ObjRef.LayerPath = cLayerPath;
                                    }

                                    AddObjectToLayer(ObjRef, pName, FALSE);

                                    TimeObjLayerAdds += LoadTimer.TripSec();
                                }
                                else
                                {
                                    //must read next header to clear properties
                                    LayerFile.ReadHeader();
                                    x_throw(xfs("error creating object (%s) during load of layer %s.",cType,pName));
                                }

                                x_catch_display;
                            }
                            else
                            {
                                x_throw(xfs("error getting guid for object (%s) during load of layer %s.",cType,pName));
                            }
                        }
                        else
                        {
                            x_throw(xfs("error reading object type in layer %s.",pName));
                        }
                    }
                    else
                    {
                        x_throw(xfs("error reading object header in layer %s. I read %d object.",pName,i));
                    }
                    x_catch_display;
                }
            }
            else
            {
                x_throw(xfs("error reading object count in layer %s.",pName));
                bReturn = FALSE;
            }
        }
        else if( x_stricmp( LayerFile.GetHeaderName(), "OverallBlueprintData" ) == 0 )
        {
            s32 nCount = LayerFile.GetHeaderCount();

            if (m_pHandler) m_pHandler->SetProgressRange(0,nCount);

            for (s32 i =0; i < nCount; i++)
            {
                LoadTimer.TripSec();

                LayerFile.ReadFields();
                char cTheme[MAX_PATH];
                char cRelPath[MAX_PATH];
                char cLayerPath[MAX_PATH];

                x_try;

                if (fVersion < 0.31f)
                {
                    //this version used full paths to blueprints
                    cTheme[0] = 0;
                    cLayerPath[0] = '\\';
                    cLayerPath[1] = 0;
                    //ok, old way for backwards compatibility, use relative path for full path
                    if (!LayerFile.GetString("FileName",cRelPath))
                    {
                        x_throw(xfs("error reading blueprint filename in layer %s.",pName));
                    }
                }
                else
                {
                    if (!LayerFile.GetString("ThemeName",cTheme))
                    {
                        x_throw(xfs("error reading blueprint theme name. [%s...(layer)%s]",cRelPath,pName));
                    }

                    if (!LayerFile.GetString("RelativePath",cRelPath))
                    {
                        x_throw(xfs("error reading blueprint relative path. [%s...(layer)%s]",cRelPath,pName));
                    }

                    if (!LayerFile.GetString("LayerPath",cLayerPath))
                    {
                        x_throw(xfs("error reading blueprint layer path. [%s...(layer)%s]",cRelPath,pName));
                    }
                }

                vector3 Anchor;
                if (!LayerFile.GetVector3("Anchor",Anchor))
                {
                    x_throw(xfs("error reading blueprint Anchor. [%s...(layer)%s]",cRelPath,pName));
                }

                radian3 r;
                if (!LayerFile.GetRadian3("Rotation",r))
                {
                    x_throw(xfs("error reading blueprint rotation. [%s...(layer)%s]",cRelPath,pName));
                }

                guid PrimaryGuid;
                if (!LayerFile.GetGuid("PrimaryGuid",PrimaryGuid))
                {
                    //don't worry about it here
                    //lets mark this layer dirty so we save primary guid
                    MarkLayerDirty(pName);
                }

                //move to grid positioning
                editor_blueprint_placement BPPlacement;
                BPPlacement.ThemeName = cTheme;
                BPPlacement.RelativePath = cRelPath;
                BPPlacement.LayerPath = cLayerPath;
                BPPlacement.Position = Anchor;
                BPPlacement.PrimaryGuid = PrimaryGuid;
                BPPlacement.Transform.Identity();
                BPPlacement.Transform.Rotate(r);
                
                BPFiles.Append(BPPlacement);

                TimeBPLoad += LoadTimer.TripSec();

                x_catch_display;

                if (m_pHandler) m_pHandler->SetProgress(i);
            }
        }
        else if( x_stricmp( LayerFile.GetHeaderName(), "BlueprintOverrideData" ) == 0 )
        {
            LayerFile.ReadFields();
            s32 nCount = 0;

            if (LayerFile.GetS32("Count",nCount))
            {
                for (s32 i =0; i < nCount; i++)
                {
                    editor_dif_prop_list DiffPropList;
                    DiffPropList.OnLoad(LayerFile);
                    BPDiffs.Append(DiffPropList);
                }
            }

        }
        else if( x_stricmp( LayerFile.GetHeaderName(), "OverallResourceData" ) == 0 )
        {
            s32 nCount = LayerFile.GetHeaderCount();

            if (m_pHandler) m_pHandler->SetProgressRange(0,nCount);

            for (s32 i =0; i < nCount; i++)
            {
                LoadTimer.TripSec();

                LayerFile.ReadFields();
                char cRes[MAX_PATH];

                if (!LayerFile.GetString("Resource",cRes))
                {
                    x_throw(xfs("error reading resource name in layer %s.",pName));
                    bReturn = FALSE;
                }

                AddResourceToLayer(cRes, pName, FALSE);

                TimeBPLoad += LoadTimer.TripSec();
                if (m_pHandler) m_pHandler->SetProgress(i);         
            }
        }
        else
        {
            x_throw(xfs("error reading blueprint header %s",pName));
            bReturn = FALSE;
        }

    }

    LayerFile.CloseFile();

    MarkLayerLoaded(pName, TRUE);

    SetActiveLayer(GetDefaultLayer(), "\\");

    x_DebugMsg("--TIMECHECK-- WorldEditor::LoadLayer(%s) FO:%g, OL:%g, OLA:%g, BL:%g, RL:%g\n",
        pName, TimeFileOpen, TimeObjLoad, TimeObjLayerAdds, TimeBPLoad, TimeRESLoad);

    x_catch_display;

    return bReturn;
}

//=========================================================================

xbool world_editor::UnLoadLayer(const char* pName)
{
    g_PolyCache.InvalidateAllCells();

    ClearSelectedObjectList();
    
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pName);

        //layer found, delete all objects in layer
        for (s32 j = 0; j < Layer.Objects.GetCount(); j++)
		{
            editor_object_ref& ObjRef = Layer.Objects.GetAt(j);
            object* pObjDel = g_ObjMgr.GetObjectByGuid(ObjRef.Guid);
            if (pObjDel)
            { 
                DestroyObjectEx( ObjRef.Guid, TRUE );
            }
        }

        //clear object list
        Layer.Objects.Clear();

        //delete all blueprints in layer, with associated objects
        for (s32 k = 0; k < Layer.Blueprints.GetCount(); k++)
		{
            editor_blueprint_ref& BlueprintRef = Layer.Blueprints.GetAt(k);
            //now delete all objects in blueprint
            for (s32 m = 0; m < BlueprintRef.ObjectsInBlueprint.GetCount(); m++)
	    	{
                guid& ObjectGuid = BlueprintRef.ObjectsInBlueprint.GetAt(m);
                object* pObjDel = g_ObjMgr.GetObjectByGuid(ObjectGuid);

                if (pObjDel)
                { 
                    DestroyObjectEx( ObjectGuid, TRUE );
                }
            }

            //delete the anchor
            object* pAnchorDel = g_ObjMgr.GetObjectByGuid(BlueprintRef.Anchor);
            if (pAnchorDel)
            {
                DestroyObjectEx( BlueprintRef.Anchor, TRUE );
            }
        }                
 
        //clear BP list
        Layer.Blueprints.Clear();

        if (x_stricmp(m_xstrActiveLayer, pName)==0)
        {
            //oh, just removed the active layer
            m_xstrActiveLayer = GetDefaultLayer( );
            m_xstrActiveLayerPath = "\\";
        }

        Layer.IsDirty = FALSE;
        Layer.IsLoaded = FALSE;

        //clear undo stack
        transaction_mgr::Transaction()->ClearStack();

        return TRUE;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return FALSE;
}

//=========================================================================

void world_editor::MarkLayerLoaded(const char* pName, xbool bLoaded )
{
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pName);
        Layer.IsLoaded = bLoaded;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;
}

//=========================================================================

xbool world_editor::IsLayerLoaded(const char* pName )
{
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pName);
        return Layer.IsLoaded;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return FALSE;
}

//=========================================================================

xbool world_editor::SaveLayers(const char* pPath)
{
    xarray<editor_object_ref> TempPortalLst;
    BOOL bGlobalSaved = IsLayerDirty(GetGlobalLayer());

    xbool bReturn = TRUE;

    //save all layers with full dir path
    x_try;

    //loop through all layers
    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(i);

        //Only save if layer is Dirty!!!!
        if ( eLayer.IsDirty )
        {
            xstring FullLayerName = xstring(pPath) + eLayer.Name + xstring(".layer") ;
            xbool bIsEditable = !m_pHandler->IsFileReadOnly( FullLayerName );
            if (bIsEditable)
            {
                s32 nMaxProgress = eLayer.Objects.GetCount() + eLayer.Blueprints.GetCount() + 1;
                s32 nCurrentProgress = 0;
                if (m_pHandler) m_pHandler->SetProgressRange(0,nMaxProgress);

                eLayer.IsDirty = FALSE;

                if (m_pHandler) m_pHandler->SetProgress(++nCurrentProgress);

                xstring xstrSaveFile;
                xstrSaveFile.Format("%s\\%s.layer",pPath,(const char*)eLayer.Name);

                text_out LayerFile;
                LayerFile.OpenFile( xstrSaveFile );

                //Add version number
                LayerFile.AddHeader("EditorInfo",1);
                LayerFile.AddF32("Version", GetVersionNumber());
                LayerFile.AddEndLine();

                //
                // Add Overall object data
                //
                s32 j;
                s32 ObjCount=0;
                for( j = 0; j < eLayer.Objects.GetCount(); j++)
		        {
                    editor_object_ref& ObjRef = eLayer.Objects.GetAt(j);
                    object *pObject = g_ObjMgr.GetObjectByGuid(ObjRef.Guid);
                    ASSERT( pObject );

                    const object_desc& Desc = pObject->GetTypeDesc();

                    // Skip if it is a editor temp object
                    if( Desc.IsTempObject() )
                        continue;

                    ObjCount++;              
                }

                LayerFile.AddHeader("OverallObjectData",1);
                LayerFile.AddS32("Count", ObjCount );
                LayerFile.AddEndLine();

                //
                // get objects
                //
                for ( j = 0; j < eLayer.Objects.GetCount(); j++)
		        {
                    editor_object_ref& ObjRef = eLayer.Objects.GetAt(j);
                    object *pObject = g_ObjMgr.GetObjectByGuid(ObjRef.Guid);
                    if (pObject)
                    {
                        x_try;

                        if ( pObject->GetType() == object::TYPE_ZONE_PORTAL )
                        {
                            //save portals separately
                            TempPortalLst.Append(ObjRef);
                        }

                        const object_desc& Desc = pObject->GetTypeDesc();

                        // Skip if it is a editor temp object
                        if( Desc.IsTempObject() )
                            continue;

                        xstring xstrType = Desc.GetTypeName();
                        
                        if (xstrType.IsEmpty())
                        {
                            x_throw("ERROR: Bad object type found, this object will not be saved!");
                        }

                        LayerFile.AddHeader("Object",1);

                        //new way
                        LayerFile.AddString("SType", xstrType);
                        LayerFile.AddGuid("Guid", pObject->GetGuid());

                        LayerFile.AddString("LayerPath", ObjRef.LayerPath);
                        LayerFile.AddEndLine();
                        pObject->OnSave(LayerFile);     
            
                        x_catch_display;
                    }
                    if (m_pHandler) m_pHandler->SetProgress(++nCurrentProgress);
                }

                //Add Overall blueprints data
                LayerFile.AddHeader("OverallBlueprintData",eLayer.Blueprints.GetCount());

                xarray<editor_dif_prop_list> BPOverrideList;

                //get blueprints
                for (j = 0; j < eLayer.Blueprints.GetCount(); j++)
		        {
                    const editor_blueprint_ref& Blueprint = eLayer.Blueprints.GetAt(j);
                    LayerFile.AddString("ThemeName", Blueprint.ThemeName);
                    LayerFile.AddString("RelativePath", Blueprint.RelativePath);
                    LayerFile.AddString("LayerPath", Blueprint.LayerPath);

                    vector3 anchorPos;  
                    object *pAnchor = g_ObjMgr.GetObjectByGuid(Blueprint.Anchor);
                    if (pAnchor)
                    {
                        anchorPos = pAnchor->GetPosition();
                    }

                    LayerFile.AddVector3("Anchor",anchorPos);
                    radian3 r = Blueprint.Transform.GetRotation();            
                    LayerFile.AddRadian3("Rotation",r);

                    guid PrimaryGuid(0);
                    if (Blueprint.ObjectsInBlueprint.GetCount() == 1)
                    {
                        PrimaryGuid = Blueprint.ObjectsInBlueprint.GetAt(0);
                        editor_dif_prop_list DiffPropList;
                        DiffPropList.m_ObjGuid = PrimaryGuid;
                        if (HasSingleBlueprintChanged(Blueprint, DiffPropList.m_ObjProps))
                        {
                            BPOverrideList.Append(DiffPropList);
                        }
                    }
                    LayerFile.AddGuid("PrimaryGuid",PrimaryGuid);
                    LayerFile.AddEndLine();

                    if (m_pHandler) m_pHandler->SetProgress(++nCurrentProgress);
                }

                //Add any blueprint override data
                LayerFile.AddHeader("BlueprintOverrideData",1);
                LayerFile.AddS32("Count", BPOverrideList.GetCount());
                LayerFile.AddEndLine();

                //get blueprints
                for (j = 0; j < BPOverrideList.GetCount(); j++)
		        {
                    editor_dif_prop_list& DiffPropList = BPOverrideList.GetAt(j);
                    DiffPropList.OnSave(LayerFile);
                }

                //Add Overall resource data
                LayerFile.AddHeader("OverallResourceData",eLayer.Resources.GetCount());

                //get resource
                for (j = 0; j < eLayer.Resources.GetCount(); j++)
		        {
                    const xstring& xstrData = eLayer.Resources.GetAt(j);
                    LayerFile.AddString("Resource", xstrData);
                    LayerFile.AddEndLine();
                }

                //close this layer
                LayerFile.CloseFile();
            }
            else
            {
                bReturn = FALSE;
            }
        }
    }

    //save Portals
    if (bGlobalSaved)
    {
        if (!SavePortals(TempPortalLst))
        {
            //could not save portals mark globals layer as dirty
            //MarkLayerDirty(GetGlobalLayer());
        }
    }

    x_catch_display;

    if (m_pHandler) 
    {
        m_pHandler->SetProgressRange(0,1);
        m_pHandler->SetProgress(0);
    }
    
    return bReturn;
}

//=========================================================================

void world_editor::CollectGuidsToExport( xarray<guid>& lstGuidsToExport )
{
    //loop through all layers
    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(i);

        // Skip collecting guids from this layer if it's not loaded
        if( eLayer.IsLoaded == FALSE )
            continue;

        xarray<guid> lstGuidsInLayer;
        GetObjectsInLayer( eLayer.Name, lstGuidsInLayer );
        //add to export list
        for (s32 j = 0; j < lstGuidsInLayer.GetCount(); j++)
	    {
            object *pObject = g_ObjMgr.GetObjectByGuid(lstGuidsInLayer.GetAt(j));
            if (pObject)
            {
                if (!(pObject->GetAttrBits() & object::ATTR_EDITOR_TEMP_OBJECT) &&
                    !(pObject->GetType()    == object::TYPE_ZONE_PORTAL ) &&
					!(pObject->GetType()	== object::TYPE_PLAYER ) ) // mreed: player info now stored in .info
                {
                    lstGuidsToExport.Append(lstGuidsInLayer.GetAt(j));
                }
            }
        }

        xarray<editor_blueprint_ref> lstBPRef;
        if (GetBlueprintsInLayer( eLayer.Name, lstBPRef ))
        {
            for (s32 k=0; k < lstBPRef.GetCount(); k++)
            {
                editor_blueprint_ref& BPRef = lstBPRef.GetAt(k);
                for (j=0; j < BPRef.ObjectsInBlueprint.GetCount(); j++)
                {
                    guid ObjGuid = BPRef.ObjectsInBlueprint.GetAt(j);
                    object *pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);
                    if (pObject)
                    {
                        if (!(pObject->GetAttrBits() & object::ATTR_EDITOR_TEMP_OBJECT)&&
                            !(pObject->GetType()    == object::TYPE_ZONE_PORTAL ) &&
							!(pObject->GetType()	== object::TYPE_PLAYER ) ) // mreed: player info now stored in .info
                        {
                            lstGuidsToExport.Append(ObjGuid);
                        }
                    }
                }
            }
        }
    }
}

//=========================================================================

void world_editor::CollectPlaySurfacesToExport( xarray<guid>& lstPlaySurfaces )
{
    //loop through all layers
    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(i);
        if( eLayer.IsLoaded == FALSE )
            continue;

        xarray<guid> lstGuidsInLayer;
        GetObjectsInLayer( eLayer.Name, lstGuidsInLayer );
        
        //add to playsurface list
        for (s32 j = 0; j < lstGuidsInLayer.GetCount(); j++)
	    {
            object *pObject = g_ObjMgr.GetObjectByGuid(lstGuidsInLayer.GetAt(j));
            if (pObject)
            {
                if ( pObject->GetType() == object::TYPE_PLAY_SURFACE )
                {
                    lstPlaySurfaces.Append(lstGuidsInLayer.GetAt(j));
                }
            }
        }

        xarray<editor_blueprint_ref> lstBPRef;
        if (GetBlueprintsInLayer( eLayer.Name, lstBPRef ))
        {
            for (s32 k=0; k < lstBPRef.GetCount(); k++)
            {
                editor_blueprint_ref& BPRef = lstBPRef.GetAt(k);
                for (j=0; j < BPRef.ObjectsInBlueprint.GetCount(); j++)
                {
                    guid ObjGuid = BPRef.ObjectsInBlueprint.GetAt(j);
                    object *pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);
                    if (pObject)
                    {
                        if ( pObject->GetType() == object::TYPE_PLAY_SURFACE )
                        {
                            lstPlaySurfaces.Append(ObjGuid);
                        }
                    }
                }
            }
        }
    }
}

//=========================================================================

void world_editor::CollectDecalsToExport( xarray<guid>& lstDecals )
{
    // loop through all layers
    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(i);
        if( eLayer.IsLoaded == FALSE )
            continue;

        xarray<guid> lstGuidsInLayer;
        GetObjectsInLayer( eLayer.Name, lstGuidsInLayer );
        
        //add to decal list
        for (s32 j = 0; j < lstGuidsInLayer.GetCount(); j++)
	    {
            object *pObject = g_ObjMgr.GetObjectByGuid(lstGuidsInLayer.GetAt(j));
            if (pObject)
            {
                if ( pObject->GetType() == object::TYPE_EDITOR_STATIC_DECAL )
                {
                    lstDecals.Append(lstGuidsInLayer.GetAt(j));
                }
            }
        }

        xarray<editor_blueprint_ref> lstBPRef;
        if (GetBlueprintsInLayer( eLayer.Name, lstBPRef ))
        {
            for (s32 k=0; k < lstBPRef.GetCount(); k++)
            {
                editor_blueprint_ref& BPRef = lstBPRef.GetAt(k);
                for (j=0; j < BPRef.ObjectsInBlueprint.GetCount(); j++)
                {
                    guid ObjGuid = BPRef.ObjectsInBlueprint.GetAt(j);
                    object *pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);
                    if (pObject)
                    {
                        if ( pObject->GetType() == object::TYPE_EDITOR_STATIC_DECAL )
                        {
                            lstDecals.Append(ObjGuid);
                        }
                    }
                }
            }
        }
    }
}

//=========================================================================

void world_editor::ExportToLevelTo3dMax( const char* pName )
{
    //
    // Collect all the guids to export
    //
    xarray<guid> lstGuidsToExport;
    CollectGuidsToExport( lstGuidsToExport );
    xarray<guid> lstPlaySurfaceGuids;
    CollectPlaySurfacesToExport( lstPlaySurfaceGuids );
    lstGuidsToExport.Append( lstPlaySurfaceGuids );

    // Get ready the progress bar
    if (m_pHandler) m_pHandler->SetProgressRange( 0, lstGuidsToExport.GetCount() );

    //
    // ASk the lighting system to export the guids
    //
    x_try;

    lighting_ExportTo3DMax( lstGuidsToExport, pName );

    x_catch_display;

    if (m_pHandler) m_pHandler->SetProgress(0);
}

//=========================================================================

void CleanPath( xstring Path )
{
    for( s32 i=0 ; i<Path.GetLength() ; i++ )
    {
        char c = Path.GetAt( i );
        if( c == '/' )
        {
            c = '\\';
            Path.SetAt( i, c );
        }
    }
}

//=========================================================================

extern xbool ExecuteCMD( const xstring& InputStr, xstring& OutputStr );

xbool WriteDFSFile( const char* pDFSFileName, const char* pFileList, xbool bEnableCRC )
{
    xstring Result;
    xstring CMD;

    if( bEnableCRC )
    {
        CMD.Format( "C:\\GameData\\A51\\Apps\\Compilers\\PS2\\dfstool.exe -crc -m -b %s %s", pDFSFileName, pFileList );
    }
    else
    {
        CMD.Format( "C:\\GameData\\A51\\Apps\\Compilers\\PS2\\dfstool.exe -split 1000000000 -m -b %s %s", pDFSFileName, pFileList );
    }
    LOG_MESSAGE( "WriteDFSFile", "%s", (const char*)CMD );
    LOG_FLUSH();

    //system((const char*)CMD);
    ExecuteCMD( CMD, Result );
    return TRUE;
}

//=========================================================================

void ExtractFilesWithExtFromList( xarray<xstring>& DstList, xarray<xstring>& SrcList, const char* pExt )
{
    s32 i=0;
    char PATH_DRIVE[256];
    char PATH_DIR[256];
    char PATH_NAME[256];
    char PATH_EXT[32];

    while( i<SrcList.GetCount() )
    {
        x_splitpath( (const char*)SrcList[i],PATH_DRIVE,PATH_DIR,PATH_NAME,PATH_EXT);

        if( x_stricmp( pExt, PATH_EXT ) == 0 )
        {
            LOG_MESSAGE( "ExtractFilesWithExtFromList", "Extracting: %s", (const char*)SrcList[i] );
            DstList.Append( SrcList[i] );
            SrcList.Delete(i);
        }
        else
        {
            LOG_MESSAGE( "ExtractFilesWithExtFromList", "  Skipping: %s", (const char*)SrcList[i] );
            i++;
        }
    }
}

//=========================================================================

void ExtractFilesWithSubStringFromList( xarray<xstring>& DstList, xarray<xstring>& SrcList, const char* pSubString )
{
    s32 i=0;
    char PATH_DRIVE[256];
    char PATH_DIR[256];
    char PATH_NAME[256];
    char PATH_EXT[32];

    while( i<SrcList.GetCount() )
    {
        x_splitpath( (const char*)SrcList[i],PATH_DRIVE,PATH_DIR,PATH_NAME,PATH_EXT);

        if( x_stristr( PATH_NAME, pSubString )  )
        {
            DstList.Append( SrcList[i] );
            SrcList.Delete(i);
        }
        else
        {
            i++;
        }
    }
}

//=========================================================================

void RemoveDuplicates( xarray<xstring>& ListToModify, xarray<xstring>& ListToCheck )
{
    s32 i = 0;
    while( i<ListToCheck.GetCount() )
    {
        s32 j = 0;
        while( j<ListToModify.GetCount() )
        {
            char Modify[256];
            char Check[256];
            x_strcpy( Modify, (const char*)ListToModify[j] );
            x_strcpy( Check, ( const char*)ListToCheck[i]  );

            // Find it?
            if( x_stricmp( (const char*)ListToModify[j], (const char*)ListToCheck[i] ) == 0 )
            {
                LOG_MESSAGE( "RemoveDuplicates[2]", 
                             "Removing: [%s]",
                             (const char*)ListToModify[j] );

                // Remove it!
                ListToModify.Delete( j );
            }
            else
            {
                j++;
            }
        }

        // Next!
        i++;
    }
}

void RemoveDuplicates( xarray<xstring>& List )
{
    s32 i = 0;
    while( i<List.GetCount() )
    {
        s32 j = i+1;
        while( j<List.GetCount() )
        {
            if( x_stricmp( (const char*)List[j], (const char*)List[i] ) == 0 )
            {
                LOG_MESSAGE( "RemoveDuplicates[1]", 
                    "Removing: [%s]",
                    (const char*)List[j] );

                // Remove it!
                List.Delete( j );
            }
            else
            {
                j++;
            }
        }

        // Next!
        i++;
    }
}

//=========================================================================

s32 GetDependencies( xarray<xstring>& Dest, xarray<xstring>& Source, const char* pPlatformString,platform PlatformType )
{
    s32 Result = 0;

    char* Ext[] = 
    {
        ".rigidgeom",
        ".skingeom",
        ".anim",
        ".decalpkg",
        ".envmap",
        ".fxo"
    };

    // Clear the destination.
    Dest.Clear();

    // For each file...
    for( s32 i=0 ; i<Source.GetCount() ; i++ )
    {
        xbool bContinue = TRUE;

        // Make sure file exists by opening it...
        char RscName[256];
        char RscExt[20];
        xstring Filename;
        xstring Filepath;
        x_splitpath( (const char*)Source[i],NULL,NULL,RscName,RscExt);
        Filepath.Format( "%s\\%s%s", xfs( "%s\\%s",g_Settings.GetReleasePath(),pPlatformString), RscName, RscExt );
        Filename.Format( "%s%s", RscName, RscExt );

        X_FILE* fp = x_fopen((const char*)Filepath,"rb");
        if( fp )
        {
            // Close it!
            x_fclose( fp );

            for( s32 j=0 ; (j<sizeof(Ext)/sizeof(char*)) && bContinue ; j++ )
            {
                // Is it a supported type?
                if( x_stristr( (const char*)Source[i], Ext[j] ) )
                {
                    xarray<xstring> Dependencies;

                    // Done!
                    bContinue = FALSE;

                    x_try;

                    // Now add any dependencies.
                    rsc_desc& RescDesc = g_RescDescMGR.GetRscDescByString( Filename );
                    RescDesc.OnGetFinalDependencies( Dependencies, PlatformType, xfs( "%s\\%s",g_Settings.GetReleasePath(),pPlatformString) );

                    x_catch_begin;

                    rsc_desc& RescDesc = g_RescDescMGR.CreateRscDesc( Filename );
                    RescDesc.OnGetFinalDependencies( Dependencies, PlatformType, xfs( "%s\\%s",g_Settings.GetReleasePath(),pPlatformString) );
                    g_RescDescMGR.DeleteRscDesc( Filename );

                    x_catch_end;

                    Result = Dependencies.GetCount();
                    for( s32 k=0 ; k<Dependencies.GetCount() ; k++ )
                    {
                        xstring         Temp;

                        Temp.Format( "%s\\%s\\%s", g_Settings.GetReleasePath(), pPlatformString, (const char*)Dependencies[k] );
                        CleanPath( Temp );
                        Dest.Append( Temp );
                     }

                    xarray<xstring> SubDependencies;
                    if( GetDependencies( SubDependencies, Dependencies, pPlatformString,PlatformType ) )
                        Dest.Append( SubDependencies );

                }
            }
        }
        else
        {
            LOG_ERROR( "GetDependencies", 
                       "Could not find file: [%s]",
                       (const char*)Filepath
                     );
        }
    }

    return Result;
}

//=========================================================================

void ExtractionSort( xarray<xstring>& Destination, xarray<xstring>& Source )
{
    // Now extract the other resources from the resource list.
    ExtractFilesWithExtFromList( Destination, Source,         ".txt" );
    ExtractFilesWithExtFromList( Destination, Source,        ".xbmp" );
    ExtractFilesWithExtFromList( Destination, Source,   ".rigidgeom" );
    ExtractFilesWithExtFromList( Destination, Source,    ".skingeom" );
    ExtractFilesWithExtFromList( Destination, Source,        ".anim" );
    ExtractFilesWithExtFromList( Destination, Source,    ".decalpkg" );
    ExtractFilesWithExtFromList( Destination, Source,      ".envmap" );
    ExtractFilesWithExtFromList( Destination, Source,  ".rigidcolor" );
    ExtractFilesWithExtFromList( Destination, Source,   ".stringbin" );
    ExtractFilesWithExtFromList( Destination, Source,         ".fxo" );
    ExtractFilesWithExtFromList( Destination, Source,    ".audiopkg" );
    ExtractFilesWithExtFromList( Destination, Source,        ".font" );
}

//=========================================================================

void CreateListFromFile( xarray<xstring>&Destination, const char* pFilename, const char* pExportPath )
{
    LOG_MESSAGE( "CreateListFromFile", "SOURCE FILENAME: %s", pFilename );
    Destination.Clear();
    X_FILE* fp = x_fopen(pFilename,"rb");
    if( fp )
    {
        s32 Size = x_flength(fp);
        char* pB = (char*)x_malloc(Size);
        x_fread(pB,1,Size,fp);
        x_fclose(fp);

        s32 i=0;
        while( i<Size )
        {
            s32 j = i;

            // Advance to the end of the string
            while( (pB[i] != '\n') && (pB[i] != 0) )
                i++;

            ASSERTS( i < Size, xfs( "CreateListFromFile couldn't find the end of a string from file %s, checking that i < Size: %i >= %i", pFilename, i, Size ) );
            if ( i >= Size )
            {
                break;
            }

            pB[i] = 0;
            if( pB[i-1] == 0x0D )
                pB[i-1] = 0;

            xstring FileName;
            xstring Path;
            char RscName[X_MAX_FNAME];
            char RscExt[X_MAX_EXT];
            x_splitpath( pB+j,NULL,NULL,RscName,RscExt);
            FileName.Format( "%s%s", RscName, RscExt );
            if( FileName.GetLength() > 0 )
            {
                Path.Format( "%s\\%s", pExportPath, (const char*)FileName );
                CleanPath( Path );
                LOG_MESSAGE( "CreateListFromFile", "ADDING: %s", Path );
                Destination.Append( Path );
            }
            else
            {
                LOG_ERROR( "CreateListFromFile", "0 length string from '%s'", pFilename );
            }


            i++;
        }
    }
    else
    {
        LOG_MESSAGE( "CreateListFromFile", "Could not open file: [%s]", pFilename );
    }
}

//=========================================================================

void CreateLevelDataList( xarray<xstring>& Destination, const char* pExportPath )
{
    Destination.Clear();

    char* Ext[] = 
    {
        ".nmp.new",
        ".glb",
        ".bin_level",
        ".lev_dict",
//        ".load",
        ".templates",
        ".tmpl_dct",
        ".zone",
        ".playsurface",
        ".decals",
        ".info",
        ".rigidcolor"
    };

    for( s32 i=0; i<sizeof(Ext)/sizeof(char*); i++ )
    {
        char FileName[256];
        x_sprintf(FileName,"%s\\level_data%s",pExportPath,Ext[i]);
        CleanPath( FileName );
        Destination.Append( xstring(FileName) );
    }
}

//=========================================================================

void CreateSlideShowList( xarray<xstring>& Destination, const char* pExportPath )
{
    Destination.Clear();

    slot_id         ID        = g_ObjMgr.GetFirst( object::TYPE_LEVEL_SETTINGS );
    level_settings* pSettings = (level_settings*)g_ObjMgr.GetObjectBySlot( ID );

    // Append in the fog bitmap (WARNING: Hard-coded!!!)
    // We choose not to use the PRELOAD_FILE in this case, because we don't
    // want the game to give up memory for this. Only the load screen.
    xstring FogPath;
    FogPath.Format( "%s\\%s", pExportPath, "UI_LoadScreen_Fog.xbmp" );
    FogPath.MakeUpper();
    CleanPath( FogPath );
    Destination.Append( FogPath );

    // All good?
    if( pSettings && pSettings->IsKindOf(level_settings::GetRTTI()) )
    {
        for( s32 i = 0; i < pSettings->GetNumSlideShowSlides(); i++ )
        {
            const level_settings::slideshow_slide* pSlide = pSettings->GetSlideShowSlide( i );

            if( pSlide->TextureName[0] == 0 )
                continue;

            xstring Path( pSlide->TextureName );
            xbool   AppendExt = TRUE;
            if( Path.GetLength() > 5 )
            {
                xstring TempPath = Path.Right( 5 );
                TempPath.MakeUpper();
                if( TempPath == ".XBMP" )
                    AppendExt = FALSE;
            }
            if( AppendExt )
                Path += ".xbmp";

            xstring FinalPath;
            FinalPath.Format( "%s\\%s", pExportPath, Path );
            FinalPath.MakeUpper();
            CleanPath( FinalPath );
            Destination.Append( FinalPath );
        }
    }
}

//=========================================================================

void CreateSlideShowScript( const char* pFilename )
{
    slot_id         ID        = g_ObjMgr.GetFirst( object::TYPE_LEVEL_SETTINGS );
    level_settings* pSettings = (level_settings*)g_ObjMgr.GetObjectBySlot( ID );

    // All good?
    if( pSettings && pSettings->IsKindOf(level_settings::GetRTTI()) )
    {
        text_out TextOut;

        TextOut.OpenFile( pFilename );

        // List the audio descriptor
        TextOut.AddHeader( "Audio", 1 );
        TextOut.AddString( "descriptor", pSettings->GetSlideShowDescriptor() );
        TextOut.AddEndLine();

        // List the text slide times
        f32 StartTextAnim = (f32)pSettings->GetStartTextAnim() / 30.0f;
        TextOut.AddHeader( "TextAnim", 1 );
        TextOut.AddF32( "start_text_anim", StartTextAnim );
        TextOut.AddEndLine();

        // List the pictures in the slideshow along with their fade times.
        TextOut.AddHeader( "Slides", pSettings->GetNumSlideShowSlides() );
        for( s32 i=0 ; i<pSettings->GetNumSlideShowSlides() ; i++ )
        {
            const level_settings::slideshow_slide* pSlide = pSettings->GetSlideShowSlide(i);

            // strip the path from the xbmp name, but make sure we have
            // a proper extension on there
            char XBMPName[ MAX_PATH ];
            if( pSlide->TextureName[0] != 0 )
            {
                x_splitpath( pSlide->TextureName, NULL, NULL, XBMPName, NULL );
                x_strcat( XBMPName, ".XBMP" );
                TextOut.AddString( "texture", XBMPName );
            }
            else
            {
                TextOut.AddString( "texture", "<NULL>" );
            }

            // convert the frame times into something more meaningful for the game
            f32 StartFadeIn  = (f32)pSlide->StartFadeIn / 30.0f;
            f32 EndFadeIn    = (f32)pSlide->EndFadeIn / 30.0f;
            f32 StartFadeOut = (f32)pSlide->StartFadeOut / 30.0f;
            f32 EndFadeOut   = (f32)pSlide->EndFadeOut / 30.0f;
            EndFadeIn    = MIN(EndFadeOut,EndFadeIn);
            StartFadeOut = MAX(EndFadeIn,StartFadeOut);

            TextOut.AddF32( "start_fade_in", StartFadeIn );
            TextOut.AddF32( "end_fade_in",   EndFadeIn );
            TextOut.AddF32( "start_fade_out", StartFadeOut );
            TextOut.AddF32( "end_fade_out", EndFadeOut );
            TextOut.AddColor( "slide_color", pSlide->SlideColor );
            TextOut.AddEndLine();
        }

        TextOut.CloseFile();
    }
}

//=========================================================================

void DumpListToFile( const char* pFilename, xarray<xstring>& List )
{
    X_FILE* f = x_fopen( pFilename, "wt" );
    if( f )
    {
        for( s32 i=0 ; i<List.GetCount() ; i++ )
        {
            x_fprintf( f, "%s\n", (const char*)List[i] );
        }

        x_fclose( f );
    }
}

//=========================================================================

void CleanDVDFilename( xstring Filename )
{
    Filename.MakeUpper();

    for( s32 i=0 ; i<Filename.GetLength() ; i++ )
    {
        char c = Filename.GetAt( i );
        if( !x_isalpha(c) && !x_isdigit(c) && (c != ':') && (c != '\\') )
            c = '_';
        Filename.SetAt( i, c );
    }
}

//=========================================================================

void CleanDVDFilename( char* pFilename )
{
    if( pFilename )
    {
        while( *pFilename )
        {
            char c = *pFilename;
            c = x_toupper( c );
            if( !x_isalpha(c) && !x_isdigit(c) && (c != ':') && (c != '\\') )
                c = '_';
            *pFilename++ = c;
        }
    }
}

//=========================================================================

void CreateFileList( xarray<xstring>& Destination, const char* pPath, const char* pFilter )
{
    CFileFind finder;
    xstring   Filespec;

    Destination.Clear();
    Filespec.Format( "%s\\%s", pPath, pFilter );
    LOG_MESSAGE( "CreateFileList", "Filespec: %s", (const char*)Filespec );
    CString s = CString( (const char*)Filespec );
    BOOL bWorking = finder.FindFile( s );
    while (bWorking)
    {
        xstring Filepath;
        bWorking = finder.FindNextFile();
        Filepath.Format( "%s", (const char*)finder.GetFilePath() );
        CleanPath( Filepath );
        Destination.Append( Filepath );
        LOG_MESSAGE( "CreateFileList", "file: %s", (const char*)Filepath );
    }
}

//=========================================================================

xbool world_editor::BuildDFSFile( const char* pExportName,
                                  const char* pReleasePath,
                                  const char* pPlatformString,
                                  platform     PlatformType
                                 )
{
    text_out TextOut;

    xarray<xstring> TempList;
    xarray<xstring> Dependencies;
    xarray<xstring> BootData;
    xarray<xstring> PreloadData;
    xarray<xstring> LevelData;
    xarray<xstring> CommonData;
    xarray<xstring> LevelResources;
    xarray<xstring> LevelAudioPackages;
    xarray<xstring> LevelStringbins;
    xarray<xstring> GlobalMusicPackages;
    xarray<xstring> GlobalVoicePackages;
    xarray<xstring> GlobalAmbientPackages;
    xarray<xstring> GlobalHotPackages;
    xarray<xstring> GlobalStringbins;
    xarray<xstring> SlideShowData;

    // Paths on the exporting PC.
    xstring ExportPath;
    xstring ExportFilename;
    xstring BootFileListing;
    xstring BootDataListing;
    xstring PreloadFileListing;
    xstring PreloadDataListing;
    xstring LevelResourceListing;
    xstring LevelDataListing;
    xstring CommonFileListing;
    xstring CommonDataListing;
    xstring SlideShowFileListing;

    xstring LoadScript;         // 'c:\gamedata\a51\release\ps2\LoadScript.txt'
    xstring SlideShowScript;    // 'c:\gamedata\a51\release\ps2\SlideShowScript.txt'
    xstring RootPath;           // 'c:\gamedata\a51\release\ps2\dvd'
    xstring AudioPath;          // 'c:\gamedata\a51\release\ps2\dvd\audio
    xstring MusicFilename;      // 'c:\gamedata\a51\release\ps2\dvd\audio\music.dfs
    xstring VoiceFilename;      // 'c:\gamedata\a51\release\ps2\dvd\audio\voice.dfs
    xstring AmbientFilename;    // 'c:\gamedata\a51\release\ps2\dvd\audio\ambient.dfs
    xstring HotFilename;        // 'c:\gamedata\a51\release\ps2\dvd\audio\hot.dfs   
    xstring StringFilename;     // 'c:\gamedata\a51\release\ps2\dvd\strings\strings.dfs
    xstring MoviePath;          // 'c:\gamedata\a51\release\ps2\dvd\movies
    xstring LevelPath;          // 'c:\gamedata\a51\release\ps2\dvd\levels
    xstring LevelDirectoryPath; // 'c:\gamedata\a51\release\ps2\dvd\levels\blue
    xstring LevelFilePath;      // 'c:\gamedata\a51\release\ps2\dvd\levels\blue\1_2

    // Paths to files on the DVD.
    xstring DVDLevelDFS;
    xstring DVDResourceDFS;
    xstring DVDAudioMusicDFS;
    xstring DVDAudioVoiceDFS;
    xstring DVDAudioAmbientDFS;
    xstring DVDAudioRAMDFS;

    //========================================================
    // Construct all paths and filenames.
    {
        // Extract the export filename.
        char buffer[256];
        x_splitpath( pExportName, NULL, NULL, buffer, NULL );
        ExportFilename = buffer;

        // Generate the export path.
        ExportPath.Format( "%s\\%s", pReleasePath, pPlatformString );
        CleanDVDFilename( ExportPath );

        // Generate the dvd root path.
        RootPath.Format( "%s\\%s\\DVD", pReleasePath, pPlatformString );
        CleanDVDFilename( RootPath );

        // Construct the level dfs paths.
        DVDLevelDFS.Format( "LEVELS\\%s\\%s\\LEVEL",    
                          (const char*)g_Project.m_DFSDirectory, 
                          (const char*)g_Project.m_DFSName  );

        // Construct the resource dfs paths.
        DVDResourceDFS.Format( "LEVELS\\%s\\%s\\RESOURCE", 
                             (const char*)g_Project.m_DFSDirectory, 
                             (const char*)g_Project.m_DFSName );

        // Generate required paths.
        AudioPath.Format         ( "%s\\AUDIO",  (const char*)RootPath );
        MoviePath.Format         ( "%s\\MOVIES", (const char*)RootPath );
        LevelPath.Format         ( "%s\\LEVELS", (const char*)RootPath );
        LevelDirectoryPath.Format( "%s\\%s",     (const char*)LevelPath, (const char*)g_Project.m_DFSDirectory );
        LevelFilePath.Format     ( "%s\\%s",     (const char*)LevelDirectoryPath, (const char*)g_Project.m_DFSName );

        // Generate text file paths!
        BootFileListing.Format      ( "%s\\BootFiles.txt",              (const char*)ExportPath );
        BootDataListing.Format      ( "%s\\BootData.txt",               (const char*)ExportPath );
        CommonFileListing.Format    ( "%s\\CommonFiles.txt",            (const char*)ExportPath );
        CommonDataListing.Format    ( "%s\\CommonData.txt",             (const char*)ExportPath );
        PreloadFileListing.Format   ( "%s\\PreloadFiles.txt",           (const char*)ExportPath );
        PreloadDataListing.Format   ( "%s\\PreloadData.txt",            (const char*)ExportPath );
        LoadScript.Format           ( "%s\\LoadScript.txt",             (const char*)ExportPath );
        SlideShowScript.Format      ( "%s\\SlideShowScript.txt",        (const char*)ExportPath );
        LevelResourceListing.Format ( "%s\\LevelResources.txt",         (const char*)ExportPath );
        LevelDataListing.Format     ( "%s\\LevelData.txt",              (const char*)ExportPath );
        MusicFilename.Format        ( "%s\\AudioMusicResources.txt",    (const char*)ExportPath );
        VoiceFilename.Format        ( "%s\\AudioVoiceResources.txt",    (const char*)ExportPath );
        AmbientFilename.Format      ( "%s\\AudioAmbientResources.txt",  (const char*)ExportPath );
        HotFilename.Format          ( "%s\\AudioHotResources.txt",      (const char*)ExportPath );
        StringFilename.Format       ( "%s\\StringResources.txt",        (const char*)ExportPath );

        // Generate slideshow listing path.
        SlideShowFileListing.Format ( "%sSlideshowFiles.txt", g_Project.GetWorkingPath() );

        // Make sure the directories exist.
        CString s = CString( (const char*)RootPath );
        CreateDirectory( s, NULL );
        s = CString( (const char*)AudioPath );
        CreateDirectory( s, NULL );
        s = CString( (const char*)MoviePath );
        CreateDirectory( s, NULL );
        s = CString( (const char*)LevelPath );
        CreateDirectory( s, NULL );
        s = CString( (const char*)LevelDirectoryPath );
        CreateDirectory( s, NULL );
        s = CString( (const char*)LevelFilePath );
        CreateDirectory( s, NULL );
    }

    //========================================================
    // Build all the lists!
    {
        // Extract the resource filenames from the .load file.
        CreateListFromFile( TempList, xfs("%s\\level_data.load", (const char*)ExportPath ), (const char*)ExportPath );

        // Extract the audio packages from the resource list.
        ExtractFilesWithExtFromList( LevelAudioPackages, TempList, ".audiopkg" );

        // Extract the stringbin files from the resource list.
        ExtractFilesWithExtFromList( LevelStringbins, TempList, ".stringbin" );

        // Now extract the other resources from the resource list.
        ExtractionSort( LevelResources, TempList );

        // After extraction, <Resources> should be empty!
        ASSERT( TempList.GetCount() == 0 );
        if( TempList.GetCount() )
        {
            for( s32 i=0 ; i<TempList.GetCount() ; i++ )
            {
                LOG_WARNING( "BuildDFSFile", "Resource: [%s] not accounted for!", (const char *)TempList[i] );
            }
        }

        // Create the level data list.
        CreateLevelDataList( LevelData, ExportPath );

        // Create the slide show data list, append it to the level data list.
        CreateSlideShowList( SlideShowData, ExportPath );

        // Create the boot data list (get all the dependencies too!)
        CreateListFromFile( TempList, BootFileListing, ExportPath );
        GetDependencies( Dependencies, TempList, pPlatformString,PlatformType );
        if( Dependencies.GetCount() )
            TempList.Append( Dependencies );
        ExtractionSort( BootData, TempList );
        if( TempList.GetCount() )
            BootData.Append( TempList );
        RemoveDuplicates( BootData );

        // Create the preload data list (get all the dependencies too!)
        CreateListFromFile( TempList, PreloadFileListing, ExportPath );
        GetDependencies( Dependencies, TempList, pPlatformString,PlatformType );
        if( Dependencies.GetCount() )
            TempList.Append( Dependencies );
        ExtractionSort( PreloadData, TempList );
        if( TempList.GetCount() )
            PreloadData.Append( TempList );
        RemoveDuplicates( PreloadData );

        // Create the common data list (get all the dependencies too!)
        CreateListFromFile( TempList, CommonFileListing, ExportPath );
        GetDependencies( Dependencies, TempList, pPlatformString,PlatformType );
        if( Dependencies.GetCount() )
            TempList.Append( Dependencies );
        ExtractionSort( CommonData, TempList );
        if( TempList.GetCount() )
            CommonData.Append( TempList );
        RemoveDuplicates( CommonData );

        // Remove duplicate files.
        RemoveDuplicates( PreloadData,    BootData    );
        RemoveDuplicates( LevelResources, BootData    );
        RemoveDuplicates( LevelResources, PreloadData );

        // Remove .anim packages from preload.dfs and append them to level resources.dfs
        TempList.Clear();
        ExtractFilesWithExtFromList( TempList, PreloadData, ".anim" );
        if( TempList.GetCount() )
        {
            LevelResources.Append( TempList );
        }

        // Snag all the audio packages.
        CreateFileList( GlobalHotPackages, ExportPath, "*.audiopkg" );

        // Build the audio file lists.
        ExtractFilesWithSubStringFromList( GlobalMusicPackages,   GlobalHotPackages, "music_" );
        ExtractFilesWithSubStringFromList( GlobalVoicePackages,   GlobalHotPackages, "dx_"    );
        ExtractFilesWithSubStringFromList( GlobalAmbientPackages, GlobalHotPackages, "amb_"   );

        // Snag all the stringbin files.
        CreateFileList( GlobalStringbins, ExportPath, "*.stringbin" );
    }

    //========================================================
    // Build the default load script.
    {
        TextOut.OpenFile( (const char*)LoadScript );
        TextOut.AddHeader( "loadscript", LevelAudioPackages.GetCount() + LevelStringbins.GetCount() + 2 );

        // load the audio dfs
        TextOut.AddString( "command",   "load_dfs" );
        TextOut.AddString( "arguments", (const char*)DVDResourceDFS );
        TextOut.AddEndLine();

        // List the audio packages to load.
        if( LevelAudioPackages.GetCount() )
        {
            //Script.AddFormat( "m: %s\n", DVDAudioRAMDFS );
            for( s32 i=0 ; i<LevelAudioPackages.GetCount() ; i++ )
            {
                char PackageName[ MAX_PATH ];
                x_splitpath( (const char*)LevelAudioPackages[i], NULL, NULL, PackageName, NULL );

                TextOut.AddString( "command",   "load_resource"                   );
                TextOut.AddString( "arguments", xfs( "%s.audiopkg", PackageName ) );
                TextOut.AddEndLine();
            }
            //Script.AddFormat( "u: %s\n", DVDAudioRAMDFS );
        }

        // load the strings dfs
        TextOut.AddString( "command",   "load_dfs" );
        TextOut.AddString( "arguments", (const char*)"STRINGS\\STRINGS" );
        TextOut.AddEndLine();

        // List the stringbins to load.
        if( LevelStringbins.GetCount() )
        {
            for( s32 i=0 ; i<LevelStringbins.GetCount() ; i++ )
            {
                char StringName[ MAX_PATH ];
                x_splitpath( (const char*)LevelStringbins[i], NULL, NULL, StringName, NULL );

                TextOut.AddString( "command",   "load_resource"                   );
                TextOut.AddString( "arguments", xfs( "%s.stringbin", StringName ) );
                TextOut.AddEndLine();
            }
        }

        TextOut.CloseFile();
    }
    
    //========================================================
    // Build the slide show script
    {
        CreateSlideShowScript( SlideShowScript );
    }

    // Add MapList.txt to the preload data list.
    PreloadData.Insert( 0, (xstring)xfs( "%s\\MapList.txt", (const char*)ExportPath )  );

    // Hack for now - put the movies in the preload dfs
    PreloadData.Insert( 0, (xstring)xfs( "%s\\outro.pss",      (const char*)ExportPath )  );
    PreloadData.Insert( 0, (xstring)xfs( "%s\\inevitable.pss", (const char*)ExportPath )  );

    // Add the loadscript to the level data list.
    LevelData.Insert( 0, (xstring)LoadScript );

    // Put the slide show data in the level data.
    for( s32 i = 0; i < SlideShowData.GetCount(); i++ )
        LevelData.Insert( i, SlideShowData[i] );

    // Add the slideshow script to the level data.
    LevelData.Insert( 0, (xstring)SlideShowScript );

    // Add the tweaks to the level data list.
    LevelData.Append( (xstring)xfs( "%s\\Tweak_DamageTable.txt",     (const char*)ExportPath ) );
    LevelData.Append( (xstring)xfs( "%s\\Tweak_MP_DamageTable.txt",  (const char*)ExportPath ) );
    LevelData.Append( (xstring)xfs( "%s\\Tweak_ForceTable.txt",      (const char*)ExportPath ) );
    LevelData.Append( (xstring)xfs( "%s\\Tweak_General.txt",         (const char*)ExportPath ) );
    LevelData.Append( (xstring)xfs( "%s\\Tweak_MP_General.txt",      (const char*)ExportPath ) );
    LevelData.Append( (xstring)xfs( "%s\\Tweak_HitTypeTable.txt",    (const char*)ExportPath ) );
    LevelData.Append( (xstring)xfs( "%s\\Tweak_PainProfile.txt",     (const char*)ExportPath ) );        
    
    //========================================================
    // Write the data out!
    {
        // Now save out the lists!
        DumpListToFile( BootDataListing,      BootData              );
        DumpListToFile( PreloadDataListing,   PreloadData           );
        DumpListToFile( LevelResourceListing, LevelResources        );
        DumpListToFile( LevelDataListing,     LevelData             );
        DumpListToFile( MusicFilename,        GlobalMusicPackages   );
        DumpListToFile( VoiceFilename,        GlobalVoicePackages   );
        DumpListToFile( AmbientFilename,      GlobalAmbientPackages );
        DumpListToFile( HotFilename,          GlobalHotPackages     );
        DumpListToFile( StringFilename,       GlobalStringbins      );
        DumpListToFile( CommonDataListing,    CommonData            );

        // Now build the dfs files.
        xbool bEnableCRC = (PlatformType == PLATFORM_XBOX);
        WriteDFSFile( xfs("%s\\BOOT",     (const char*)RootPath),      BootDataListing,      bEnableCRC );
        WriteDFSFile( xfs("%s\\PRELOAD",  (const char*)RootPath),      PreloadDataListing,   bEnableCRC );
        WriteDFSFile( xfs("%s\\RESOURCE", (const char*)LevelFilePath), LevelResourceListing, bEnableCRC );
        WriteDFSFile( xfs("%s\\LEVEL",    (const char*)LevelFilePath), LevelDataListing,     bEnableCRC );
        WriteDFSFile( xfs("%s\\MUSIC",    (const char*)AudioPath),     MusicFilename,        bEnableCRC );
        WriteDFSFile( xfs("%s\\VOICE",    (const char*)AudioPath),     VoiceFilename,        bEnableCRC );
        WriteDFSFile( xfs("%s\\AMBIENT",  (const char*)AudioPath),     AmbientFilename,      bEnableCRC );
        WriteDFSFile( xfs("%s\\HOT",      (const char*)AudioPath),     HotFilename,          bEnableCRC );
        WriteDFSFile( xfs("%s\\STRINGS",  (const char*)RootPath),      StringFilename,       bEnableCRC );
        WriteDFSFile( xfs("%s\\COMMON",   (const char*)RootPath),      CommonDataListing,    bEnableCRC );
    }

    //========================================================
    // All good!
    return TRUE;
}

//=========================================================================
xbool world_editor::ExportToLevel( const char* pName )
{
    // NOTE: When adding additional files to the level export, please make sure you keep 
    // world_editor::BuildDFSFile() in sync. See notes there.


    x_try;

    ZoneSanityCheck();
    if (!ExportSanityCheck())
    {
        return FALSE;
    }

    //
    // Collect all the guids to export
    //
    xarray<guid> lstGuidsToExport;
    CollectGuidsToExport( lstGuidsToExport );
    xarray<guid> lstPlaySurfaces;
    CollectPlaySurfacesToExport( lstPlaySurfaces );
    xarray<guid> lstDecals;
    CollectDecalsToExport( lstDecals );

    s32 n = 0;
    bbox WorldBBox;
    WorldBBox.Clear();

    //calc world bbox for game objects
    for (n=0; n <lstGuidsToExport.GetCount(); n++)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid(lstGuidsToExport.GetAt(n));
        if (pObj)
        {
            s32 Type = pObj->GetType();
            ASSERTS( Type != object::TYPE_PLAYER, "We shouldn't be exporting players" );
            if( (Type  == object::TYPE_SIMPLE_SND_EMITTER) ||
                (Type  == object::TYPE_SND_EMITTER) )
                continue;

            bbox BBox = pObj->GetBBox();
            WorldBBox += BBox;
        }
    }
    
    //calc world bbox for all playsurfaces
    for (n=0; n <lstPlaySurfaces.GetCount(); n++)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid(lstPlaySurfaces.GetAt(n));
        if (pObj)
        {
            bbox BBox = pObj->GetBBox();
            WorldBBox += BBox;
        }
    }

    //
    //
    //
    s32 nPlatform = g_Settings.GetPlatformCount();
    s32 nGuid     = lstGuidsToExport.GetCount();
  
    x_DebugMsg("EXPORT, begin exporting %d objects\n",nGuid);

    s32 nExportPlatformCount = 0;
    for( n=0; n < nPlatform; n++ )
    {
        // Are we exporting for this platform?
        if( g_Settings.GetPlatfromExportI( n ) == FALSE )
            continue;

        platform     PlatformType   = g_Settings.GetPlatformTypeI  ( n );
        if ((PlatformType != PLATFORM_PC) && (PlatformType != PLATFORM_XBOX))
        {
            nExportPlatformCount++;
        }
    }
    
    if (m_pHandler) m_pHandler->SetProgressRange( 0, (nExportPlatformCount * 7) );
    if (m_pHandler) m_pHandler->SetProgress( 0 );

    //
    // Export level file to all platforms
    //

    s32 Progress = 0;

    for( n=0; n < nPlatform; n++ )
    {
        // Are we exporting for this platform?
        if( g_Settings.GetPlatfromExportI( n ) == FALSE )
            continue;

        const char* pReleasePath    = g_Settings.GetReleasePath();
        const char* pPlatformString = g_Settings.GetPlatformStringI( n );
        platform     PlatformType   = g_Settings.GetPlatformTypeI  ( n );

        // Delete all the level_data.* files
        {
            xstring Filename;
            xbool   Result;
            Filename.Format( "%s\\%s\\level_data.rigidcolor",  pReleasePath, pPlatformString );
            Result = DeleteFile( (const char*)Filename );
            Filename.Format( "%s\\%s\\level_data.level",       pReleasePath, pPlatformString );
            Result = DeleteFile( (const char*)Filename );
            Filename.Format( "%s\\%s\\level_data.load",        pReleasePath, pPlatformString );
            Result = DeleteFile( (const char*)Filename );
            Filename.Format( "%s\\%s\\level_data.decals",      pReleasePath, pPlatformString );
            Result = DeleteFile( (const char*)Filename );
            Filename.Format( "%s\\%s\\level_data.glb",         pReleasePath, pPlatformString );
            Result = DeleteFile( (const char*)Filename );
            Filename.Format( "%s\\%s\\level_data.zone",        pReleasePath, pPlatformString );
            Result = DeleteFile( (const char*)Filename );
            Filename.Format( "%s\\%s\\level_data.playsurface", pReleasePath, pPlatformString );
            Result = DeleteFile( (const char*)Filename );
            Filename.Format( "%s\\%s\\level_data.tmpl_dct",    pReleasePath, pPlatformString );
            Result = DeleteFile( (const char*)Filename );
            Filename.Format( "%s\\%s\\level_data.templates",   pReleasePath, pPlatformString );
            Result = DeleteFile( (const char*)Filename );
            Filename.Format( "%s\\%s\\level_data.lev_dict",    pReleasePath, pPlatformString );
            Result = DeleteFile( (const char*)Filename );
            Filename.Format( "%s\\%s\\level_data.bin_level",   pReleasePath, pPlatformString );
            Result = DeleteFile( (const char*)Filename );
            Filename.Format( "%s\\%s\\level_data.info",        pReleasePath, pPlatformString );
            Result = DeleteFile( (const char*)Filename );
            Filename.Format( "%s\\%s\\level_data.nmp.new",     pReleasePath, pPlatformString );
            Result = DeleteFile( (const char*)Filename );
        }

// CJ: The PC game is working and now needs exported data
//        if( PlatformType != PLATFORM_PC )
        {
            char pFileName[ MAX_PATH ];
            x_splitpath( pName, NULL, NULL, pFileName, NULL );

            xstring xstrOutputFile;

            // NOTE: Color Table MUST be exported BEFORE the objects!
            x_try;
            xstrOutputFile.Format( "%s\\%s\\level_data.rigidcolor", pReleasePath, pPlatformString );

            x_DebugMsg("EXPORT %s - Computing RigidColor Data\n", pPlatformString);       

            lighting_CreateColorTable( PlatformType, lstGuidsToExport, xstrOutputFile);
            x_catch_display;
        
            if (m_pHandler) m_pHandler->SetProgress( ++Progress );
            
            x_DebugMsg("EXPORT %s - Create Level File\n", pPlatformString);       

            // Output the Level file
            xstrOutputFile.Format( "%s\\%s\\level_data.level", pReleasePath, pPlatformString );
            level Level;
            Level.Open( xstrOutputFile, FALSE );
        
            for( s32 i=0; i<nGuid; i++ )
            {
                Level.Save( lstGuidsToExport[i] );
            }
            Level.Close();

            if (m_pHandler) m_pHandler->SetProgress( ++Progress );
            
            //=============================================================================
            // update config.txt
            s32             nLevels = 0;
            FILE*           f       = NULL;
            xarray<xstring> LevelNames;
            xarray<xstring> PathNames;
            xstring         ConfigFile;
            xstring         LevelName;

            // Format the name.
            ConfigFile.Format( "%s\\%s\\MapList.txt", pReleasePath, pPlatformString );
            LevelName.Format( "%s\\%s", g_Project.m_DFSDirectory, g_Project.m_DFSName );

            // Read the current file, if it exists.
            f = fopen( (const char*)ConfigFile, "r" );
            if( f )
            {
                text_in TextIn;

                // Close it!
                fclose( f );

                TextIn.OpenFile( (const char*)ConfigFile );
                TextIn.ReadHeader();
                nLevels = TextIn.GetHeaderCount();

                // Read in the level names.
                for( i=0; i < nLevels; i++ )
                {
                    char l[128]; // Level buffer.
                    TextIn.ReadFields();
                    TextIn.GetString( "Level", l );
                    if( x_stricmp( l, (const char*)LevelName ) != 0 )
                    {
                        LevelNames.Append( (xstring)l );
                    }
                }

                // Close it.
                TextIn.CloseFile();

                // Now nuke the file.
                f = fopen( (const char*)ConfigFile, "w" );
                fclose( f );
            }

            if ( m_pHandler && !m_pHandler->IsFileReadOnly((const char*)ConfigFile) )
            {
                text_out TextOut;
                TextOut.OpenFile( (const char*)ConfigFile );
                TextOut.AddHeader( "Settings", LevelNames.GetCount()+1 );
                for( i=0; i < LevelNames.GetCount(); i++ )
                {
                    TextOut.AddString( "Level", (const char*)LevelNames[i] );
                    TextOut.AddEndLine();
                }
                TextOut.AddString( "Level", (const char*)LevelName );
                TextOut.AddEndLine();
                TextOut.CloseFile();               
            }
            else
            {
                extern xbool g_bAutoBuild;
                if( !g_bAutoBuild )
                    LOG_ERROR("Export","Failed to write level name to config.txt (probably readonly)");
            }

            //=============================================================================
                
            //=============================================================================
            x_DebugMsg("EXPORT %s - Create Rescource Load List\n", pPlatformString);       
            
            xstrOutputFile.Format( "%s\\%s\\level_data.load", pReleasePath, pPlatformString );
         
            g_WorldEditor.CreateResourceLoadList( lstGuidsToExport, lstPlaySurfaces, lstDecals, xstrOutputFile, n );
        
            //=============================================================================
        

            x_try;

            x_DebugMsg("EXPORT %s - Export Globals\n", pPlatformString);       

            //now save the globals
            xstrOutputFile.Format( "%s\\%s\\level_data.glb", pReleasePath, pPlatformString );
            g_VarMgr.SaveGlobals(xstrOutputFile);
            x_catch_display;
       
            if (m_pHandler) m_pHandler->SetProgress( ++Progress );

            x_try;

            //x_DebugMsg("EXPORT %s - Export Particle Data\n", pPlatformString);       

            //now save the particles paths directroy
            //xstrOutputFile.Format( "%s\\%s\\%s.ppd", pReleasePath, pPlatformString, pFileName );
            //GetFxDictionary().Save(xstrOutputFile);
            //x_catch_display;

            //x_try;

            x_DebugMsg("EXPORT %s - Export Zone Information\n", pPlatformString);       

            //save zone info
            BuildZoneList();
            xstrOutputFile.Format( "%s\\%s\\level_data.zone", pReleasePath, pPlatformString );
            g_ZoneMgr.Save(xstrOutputFile);
            x_catch_display;

            if (m_pHandler) m_pHandler->SetProgress( ++Progress );

            // save playsurface info (must be done AFTER the zones have been rebuilt!)
            x_try;
            x_DebugMsg("EXPORT %s - Play Surface Create Color Info\n", pPlatformString);
            lighting_CreatePlaySurfaceColors( PlatformType, lstPlaySurfaces );
            x_DebugMsg("EXPORT %s - Play Surface Info\n", pPlatformString);
            g_PlaySurfaceMgr.RebuildList( lstPlaySurfaces, PlatformType );
            xstrOutputFile.Format( "%s\\%s\\level_data.playsurface", pReleasePath, pPlatformString );
            g_PlaySurfaceMgr.OpenFile(xstrOutputFile, FALSE);
            g_PlaySurfaceMgr.SaveFile( PlatformType );
            g_PlaySurfaceMgr.CloseFile();
            x_DebugMsg("EXPORT %s - Play Surface Kill Color Info\n", pPlatformString);
            lighting_KillPlaySurfaceColors( PlatformType, lstPlaySurfaces );
            x_catch_display;

            // save the static decal info
            x_try;
            x_DebugMsg("EXPORT %s - Static Decal Info\n", pPlatformString);
            xstrOutputFile.Format( "%s\\%s\\level_data.decals", pReleasePath, pPlatformString );
            ExportDecals( xstrOutputFile, lstDecals, PlatformType );
            x_catch_display;

            x_try;

            x_DebugMsg("EXPORT %s - Export NavMap\n", pPlatformString);       

            //now save the ai
            xstrOutputFile.Format( "%s\\%s\\level_data.nmp", pReleasePath, pPlatformString );
            ai_editor::GetAIEditor()->SaveNavMap(xstrOutputFile);
            x_catch_display;
              
            if (m_pHandler) m_pHandler->SetProgress( ++Progress );

            x_try;

            x_DebugMsg("EXPORT %s - Create Template Library\n", pPlatformString);       

            //save template info and dictionary
            if (g_TemplateMgr.EditorCreateGameData())
            {
                xstring xstrDictionary;
                xstrDictionary.Format( "%s\\%s\\level_data.tmpl_dct", pReleasePath, pPlatformString );
                xstrOutputFile.Format( "%s\\%s\\level_data.templates", pReleasePath, pPlatformString );
                g_TemplateMgr.EditorSaveData(xstrOutputFile, xstrDictionary);
            }
            x_catch_display;

            if (m_pHandler) m_pHandler->SetProgress( ++Progress );

            x_try;
        
            x_DebugMsg("EXPORT %s - Binary level File\n", pPlatformString);       
        
            //save template info and dictionary
            xstring xstrDictionary;

            xstrDictionary.Format( "%s\\%s\\level_data.lev_dict",  pReleasePath, pPlatformString );
            xstrOutputFile.Format( "%s\\%s\\level_data.bin_level", pReleasePath, pPlatformString );
        
            g_BinLevelMgr.EditorSaveData(xstrOutputFile, xstrDictionary,lstGuidsToExport );

            x_catch_display;

            x_try;

            //
            //save misc info
            //
            x_DebugMsg("EXPORT %s - World Info File\n", pPlatformString);       

            xstrOutputFile.Format( "%s\\%s\\level_data.info", pReleasePath, pPlatformString );
            text_out InfoTextOut;
            InfoTextOut.OpenFile( xstrOutputFile );
            InfoTextOut.AddHeader("Info",1);

            // World bounding box (a "safe" bbox)
            InfoTextOut.AddBBox("WorldBBox", WorldBBox);
            InfoTextOut.AddEndLine();

            // If we have a player, export his position and rotation
            player* pPlayer = NULL;
            g_ObjMgr.SelectByAttribute( object::ATTR_ALL, object::TYPE_PLAYER );
            select_slot_iterator SlotIter;
            for( SlotIter.Begin(); !SlotIter.AtEnd(); SlotIter.Next() )
            {
                pPlayer = (player*)SlotIter.Get();
                ASSERT( pPlayer );
            }
            SlotIter.End();

            if ( pPlayer )
            {
                InfoTextOut.AddHeader( "PlayerInfo", 1 );
                InfoTextOut.AddField( "Position:fff", 
                                      pPlayer->GetPosition().GetX(), 
                                      pPlayer->GetPosition().GetY(), 
                                      pPlayer->GetPosition().GetZ() );
                InfoTextOut.AddField( "Pitch:f", pPlayer->GetPitch() );
                InfoTextOut.AddField( "Yaw:f", pPlayer->GetYaw() );
                InfoTextOut.AddField( "Zone:d", pPlayer->GetZone1() );
                InfoTextOut.AddGuid( "PlayerGuid", pPlayer->GetGuid() );
                InfoTextOut.AddEndLine();
            }

            InfoTextOut.CloseFile();               

            x_catch_display;

            BuildDFSFile( pName, pReleasePath, pPlatformString, PlatformType );
            if (m_pHandler) m_pHandler->SetProgress( ++Progress );
        }
/*
        else
        {
            char pFileName[ MAX_PATH ];
            x_splitpath( pName, NULL, NULL, pFileName, NULL );

            xstring xstrOutputFile;

            x_try;

            x_DebugMsg("EXPORT %s - Export NavMap\n", pPlatformString);       

            //now save the ai
            xstrOutputFile.Format( "%s\\%s\\%s.nmp", pReleasePath, pPlatformString, pFileName );
            ai_editor::GetAIEditor()->SaveNavMap(xstrOutputFile);
            x_catch_display;
        }
*/
    }
    
    x_catch_display;

    if (m_pHandler) m_pHandler->SetProgress( 0 );

    return TRUE;
}

//=========================================================================

struct debug_zone_details
{
    s32     ObjCount;
    s32     PolyCount;
    s32     VertCount;
    s32     DecalVertCount;
};

static const s32 c_MaxObjCount       = 800;
static const s32 c_MaxVertCount      = 150000;
static const s32 c_MaxPolyCount      = 150000;
static const s32 c_MaxDecalVertCount = 1600;
static const s32 c_WarnVertCount     = 140000;
static const s32 c_WarnPolyCount     = 140000;

xbool world_editor::ExportSanityCheck( void )
{
    xbool bExportReady = TRUE;

    // Basic error checking on the directory and level names.
    {
        CleanDVDFilename( g_Project.m_DFSDirectory );
        if( (x_strlen(g_Project.m_DFSDirectory)==0) || (x_strlen(g_Project.m_DFSDirectory)>8) )
        {
            generic_dialog Dialog;
            Dialog.Execute_OK("Level DFS Directory Name Invalid",
                "The level directory name listed in the project settings must be specified and be 8 characters or less.\nThe level cannot be exported until the directory is correct.");
            return FALSE;
        }

        CleanDVDFilename( g_Project.m_DFSName );
        if( (x_strlen(g_Project.m_DFSName)==0) || (x_strlen(g_Project.m_DFSName)>8) )
        {
            generic_dialog Dialog;
            Dialog.Execute_OK("Level DFS Name Invalid",
                "The level name listed in the project settings must be specified and be 8 characters or less.\nThe level cannot be exported until the level name is correct.");
            return FALSE;
        }
    }

    // Abort if there are bad object properties
    {
        xarray<property_error> Errors;
        if( ValidateObjectProperties( Errors, "Export" ) )
        {
            // TODO: CJ: Turn this back on
            //bExportReady = FALSE;
        }
    }

    //PREP  - Create basic zone structure
    debug_zone_details DZDList[256];
    for( s32 i = 0; i < 256; i++ )
    {
        DZDList[i].ObjCount       = 0;
        DZDList[i].PolyCount      = 0;
        DZDList[i].VertCount      = 0;
        DZDList[i].DecalVertCount = 0;
    }

    //PREP  - Add Detail to Debug Zone Struct
    //CHECK - No renderable objects in zone 0
    for( object_desc* pNode = object_desc::GetFirstType(); 
                      pNode; 
                      pNode = object_desc::GetNextType( pNode )  )
    {
        for( slot_id SlotID = g_ObjMgr.GetFirst( pNode->GetType() ); 
                     SlotID != SLOT_NULL; 
                     SlotID = g_ObjMgr.GetNext( SlotID ) )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
            if( !pObject )
                continue;

            if (x_strcmp(pNode->GetTypeName(),pObject->GetTypeDesc().GetTypeName()) != 0)
                continue;

            // static decals have their own limits, handle those now
            if ( pObject->GetType() == object::TYPE_EDITOR_STATIC_DECAL )
            {
                static_decal& StaticDecal = static_decal::GetSafeType( *pObject );
                
                s32 Zone1 = pObject->GetZone1();
                s32 Zone2 = pObject->GetZone2();
                ASSERT( Zone1 < 256 );
                ASSERT( Zone2 < 256 );

                if( Zone1 != 0 )
                {
                    DZDList[Zone1].DecalVertCount += StaticDecal.GetNVerts();
                }
                else
                {
                    LOG_ERROR("Export", "%s (%s) found in Zone 0 (this is not allowed).",
                        pObject->GetTypeDesc().GetTypeName(), guid_ToString(pObject->GetGuid()));
                }

                if ( Zone2 != 0 )
                {
                    DZDList[Zone2].DecalVertCount += StaticDecal.GetNVerts();
                }
            }

            // grab the number of verts, etc. so this can be added to the details
            s32 nVerts = 0;
            s32 nPolys = 0;
            if (pNode->IsBurnVertexLighting())
            {
                rigid_inst* pRigidInst = GetRigidInstForObject(pObject);
                if (pRigidInst)
                {
                    rigid_geom* pGeom = pRigidInst->GetRigidGeom();
                    if (pGeom)
                    {
                        nVerts = pGeom->GetNVerts();
                        nPolys = pGeom->GetNFaces();
                    }
                }
            }

            // check for objects being in zone zero
            if ( pObject->GetType() == object::TYPE_PLAY_SURFACE )
            {
                if (pObject->GetZone1() != 0)
                {
                    DZDList[pObject->GetZone1()].PolyCount+=nPolys;
                    DZDList[pObject->GetZone1()].VertCount+=nVerts;
                }
                else
                {
                    LOG_ERROR("Export","%s (%s) found in Zone 0 (this is not allowed).",
                        pObject->GetTypeDesc().GetTypeName(), guid_ToString(pObject->GetGuid()));
                }

                if (pObject->GetZone2() != 0)
                {
                    DZDList[pObject->GetZone2()].PolyCount+=nPolys;
                    DZDList[pObject->GetZone2()].VertCount+=nVerts;
                }
            }
            else
            if ( !(pObject->GetAttrBits() & object::ATTR_EDITOR_TEMP_OBJECT) )
            {
                xbool bZoneZeroAllowed = !(pObject->GetAttrBits() & object::ATTR_RENDERABLE)         ||
                                         (pObject->GetAttrBits() & object::ATTR_EDITOR_TEMP_OBJECT) ||
                                         (pObject->GetAttrBits() & object::ATTR_DRAW_2D);

                if (pObject->GetZone1() != 0)
                {
                    DZDList[pObject->GetZone1()].ObjCount++;
                    DZDList[pObject->GetZone1()].PolyCount+=nPolys;
                    DZDList[pObject->GetZone1()].VertCount+=nVerts;
                }
                else if ( !bZoneZeroAllowed )
                {
                    bExportReady = FALSE;
                    LOG_ERROR("Export","%s (%s) found in Zone 0 (this is not allowed).",
                        pObject->GetTypeDesc().GetTypeName(), guid_ToString(pObject->GetGuid()));
                }

                if (pObject->GetZone2() != 0)
                {
                    DZDList[pObject->GetZone2()].ObjCount++;
                    DZDList[pObject->GetZone2()].PolyCount+=nPolys;
                    DZDList[pObject->GetZone2()].VertCount+=nVerts;
                }
            }
        }
    }

    //CHECK - Zone Details
    //      - No more than 800 objects
    //      - Check vert and poly limits
    for( i = 0; i < 256; i++)
    {
        debug_zone_details& DZD = DZDList[i];

        if (DZD.ObjCount > c_MaxObjCount)
        {
            bExportReady = FALSE;
            LOG_ERROR("Export","Zone (%s) has too many objects (%d) where max is (%d).",
                GetZoneForId(i), DZD.ObjCount, c_MaxObjCount);
        }

        if (DZD.DecalVertCount > c_MaxDecalVertCount)
        {
            bExportReady = FALSE;
            LOG_ERROR("Export","Zone (%s) has too many decal verts (%d) where max is (%d).",
                GetZoneForId(i), DZD.DecalVertCount, c_MaxDecalVertCount);
        }

        if (DZD.PolyCount > c_MaxPolyCount)
        {
            bExportReady = FALSE;
            LOG_ERROR("Export","Zone (%s) has too many polys (%d) where max is (%d).",
                GetZoneForId(i), DZD.PolyCount, c_MaxPolyCount);
        }
        else if (DZD.PolyCount > c_WarnPolyCount)
        {
            LOG_WARNING("Export","Zone (%s) has an excessive number of polys (%d).",
                GetZoneForId(i), DZD.PolyCount, c_WarnPolyCount);
        }

        if (DZD.VertCount > c_MaxVertCount)
        {
            bExportReady = FALSE;
            LOG_ERROR("Export","Zone (%s) has too many verts (%d) where max is (%d).",
                GetZoneForId(i), DZD.VertCount, c_MaxVertCount);
        }
        else if (DZD.VertCount > c_WarnVertCount)
        {
            LOG_WARNING("Export","Zone (%s) has an excessive number of verts (%d).",
                GetZoneForId(i), DZD.VertCount, c_WarnVertCount);
        }
    }

    //CHECK - Required objects, do not cancel export for these however
    s32 nInputObjs = g_ObjMgr.GetNumInstances(object::TYPE_INPUT_SETTINGS);   
    if (nInputObjs != 1)
    {
        if (nInputObjs == 0)
        {
            LOG_WARNING("Export","Missing Input Settings Object.");
        }
        else
        {
            LOG_WARNING("Export","Too Many Input Settings Objects.");
        }
    }

    s32 nLevelObjs = g_ObjMgr.GetNumInstances(object::TYPE_LEVEL_SETTINGS);
    if (nLevelObjs != 1)
    {
        if (nLevelObjs == 0)
        {
            LOG_WARNING("Export","Missing Level Settings Object.");
        }
        else
        {
            LOG_WARNING("Export","Too Many Level Settings Objects.");
        }
    }

    s32 nHudObjs = g_ObjMgr.GetNumInstances(object::TYPE_HUD_OBJECT);
    if (nHudObjs != 1)
    {
        if (nHudObjs == 0)
        {
            LOG_WARNING("Export","Missing Hud Settings Object.");
        }
        else
        {
            LOG_WARNING("Export","Too Many Hud Settings Objects.");
        }
    }

    s32 nPlayerObjs = g_ObjMgr.GetNumInstances(object::TYPE_PLAYER);
    s32 nSpawnPoints = g_ObjMgr.GetNumInstances(object::TYPE_SPAWN_POINT);
    if (nPlayerObjs != 1)
    {
        if ((nPlayerObjs == 0) && (nSpawnPoints == 0))
        {
            LOG_WARNING("Export","Missing Spawn Points or Player Object.");
        }
        
        if (nPlayerObjs > 1)
        {
            LOG_WARNING("Export","Too Many Player Objects.");
        }
    }

    //CHECK - Sound Memory
    if (s_PS2MemorySize <= 0)
    {
        extern g_bAutoBuild;
        if( !g_bAutoBuild )
        {
            generic_dialog Dialog;
            Dialog.Execute_OK("Audio Memory Exceeded","Some audio packages will not fit on the PS2!!!");
        }
        LOG_ERROR("Export","Audio Memory Exceeded (%dk).", (s_PS2MemorySize/1024));
    }

    //CHECK - Require at least 1 portal and 2 zones
    s32 nPortalObjs = g_ObjMgr.GetNumInstances(object::TYPE_ZONE_PORTAL);
    if ((m_listZones.GetCount() < 2) || (nPortalObjs <= 0)) 
    {
        bExportReady = FALSE;
        LOG_ERROR("Export","At least 1 portal with 2 zones are required for the game to function.");
    }

    return bExportReady;
}

//=========================================================================
// Blueprints
//=========================================================================

xbool world_editor::GetBlueprintPath( const char* pTheme, const char* pRelPath, xstring &xstrFullPath )
{
    xstrFullPath = g_Project.GetBlueprintDirForTheme(pTheme);
    if (!xstrFullPath.IsEmpty())
    {
        xstrFullPath += xstring(pRelPath);
        return TRUE;
    }
    else
    {
        x_throw(xfs("Could not find theme(%s) for blueprint(%s)",pTheme, pRelPath));
        return FALSE;
    }
}

//=========================================================================

xbool world_editor::GetThemeInfoFromPath( const char* pPath, xstring &xstrTheme, xstring &xstrRelativePath )
{
    xstring xstrFullPath(pPath);
    xstring xstrThemeBPDir(g_Project.GetBlueprintPath());
    
    xstring xstrFullPathComp = xstrFullPath;
    xstrFullPathComp.MakeUpper();
    xstrThemeBPDir.MakeUpper();

    s32 iFind = xstrFullPathComp.Find(xstrThemeBPDir);
    if (iFind != -1)
    {
        //found it, this is in the project
        xstrTheme = g_Project.GetName();
        xstrRelativePath = xstrFullPath.Right(xstrFullPath.GetLength() - xstrThemeBPDir.GetLength());
        return TRUE;
    }

    for (s32 iTheme =0; iTheme < g_Project.GetThemeCount(); iTheme++)
    {
        xstrThemeBPDir = g_Project.GetThemeBlueprintDir(iTheme);
        xstrThemeBPDir.MakeUpper();

        iFind = xstrFullPathComp.Find(xstrThemeBPDir);
        if (iFind != -1)
        {
            //found it
            xstrTheme = g_Project.GetThemeName(iTheme);
            xstrRelativePath = xstrFullPath.Right(xstrFullPath.GetLength() - xstrThemeBPDir.GetLength());
            return TRUE;
        }
    }
    return FALSE;
}

//=========================================================================

void world_editor::SaveSelectedObjectsAsBlueprint(const char* pTheme, const char* pRelPath)
{
    x_try;
    xstring xstrFullPath;
    GetBlueprintPath(pTheme, pRelPath, xstrFullPath);

    //UNDO creation of this object
    transaction_file_data* pFileData = NULL;
    if (m_pCurrentUndoEntry)
    {
        if (transaction_file_data::DoesFileExist(xstrFullPath))
        {
            //file exists, this is an edit
            pFileData = new transaction_file_data(
                            transaction_file_data::TFILE_EDIT,
                            xstrFullPath, xstrFullPath);
            pFileData->StoreFileData(transaction_data::TRANSACTION_OLD_STATE);
        }
        else
        {
            //new file
            pFileData = new transaction_file_data(
                            transaction_file_data::TFILE_CREATE,
                            "", xstrFullPath);
        }
    }

    text_out Blueprint;
    Blueprint.OpenFile( xstrFullPath );

    bbox Bounds = GetSelectedObjectsBounds( );
    vector3 Anchor = Bounds.GetCenter();

    // Loop through all objects and find the anchor
    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        object *pObject = g_ObjMgr.GetObjectByGuid( m_guidsSelectedObjs.GetAt(i) );
        if( pObject && (pObject->GetType() == object::TYPE_EDITOR_BLUEPRINT_ANCHOR))
        {
            //got the anchor, save as origin
            Anchor = pObject->GetPosition();

            Blueprint.AddHeader("Anchor",1);
            Blueprint.AddField("Position:fff", Anchor.GetX(), Anchor.GetY(), Anchor.GetZ());
            Blueprint.AddEndLine();

            break;
        }
    }

    // Loop through all objects and save
    for( s32 j=0; j < m_guidsSelectedObjs.GetCount(); j++ )
    {       
        object *pObject = g_ObjMgr.GetObjectByGuid( m_guidsSelectedObjs.GetAt(j) );
        //if( pObject && !(pObject->GetAttrBits() & object::ATTR_EDITOR_TEMP_OBJECT))
        if( pObject && (pObject->GetType() != object::TYPE_EDITOR_BLUEPRINT_ANCHOR))
        {
            Blueprint.AddHeader("Object",1);

            xstring xstrType;

            xstrType = pObject->GetTypeDesc().GetTypeName();
            Blueprint.AddString("SType", xstrType);
            Blueprint.AddGuid("Guid", m_guidsSelectedObjs.GetAt(j)); //for guid mapping

            Blueprint.AddEndLine();

            pObject->OnSave(Blueprint);        
        }
    }

    Blueprint.CloseFile();

    //UNDO add this data
    if (pFileData)
    {
        pFileData->StoreFileData(transaction_data::TRANSACTION_NEW_STATE);
        m_pCurrentUndoEntry->AddCommitStep(pFileData);
    }

    x_catch_display;
}

//=========================================================================

s32 world_editor::AddBlueprintAsObjects(const char* pTheme, const char* pRelPath, xarray<guid>& lstGuids)
{
    s32 iAdded = 0;
    ClearSelectedObjectList();
    lstGuids.Clear();

    xarray<guid_map> GuidMapList;

    x_try;
    xstring xstrFullPath;
    GetBlueprintPath(pTheme, pRelPath, xstrFullPath);

    text_in Blueprint;
    Blueprint.OpenFile( xstrFullPath );

    while (Blueprint.ReadHeader())
    {
        if( x_stricmp( Blueprint.GetHeaderName(), "Anchor" ) == 0 )
        {
            //ignore
        }
        else if( x_stricmp( Blueprint.GetHeaderName(), "Object" ) == 0 )
        {
            Blueprint.ReadFields();
            s32 nType = -1;
            char cType[MAX_PATH];
            if (Blueprint.GetString("SType",cType)) //New Way
            {
                //ok, we need to sync guids so we can update properties that point to other objects
                guid GuidOldObj;
                guid GuidNewObj = g_ObjMgr.CreateObject( cType );
                if (Blueprint.GetGuid("Guid",GuidOldObj))
                {
                    guid_map GuidMap;
                    GuidMap.NewGuid = GuidNewObj;
                    GuidMap.OldGuid = GuidOldObj;
                    GuidMapList.Append(GuidMap);
                }

                object* pObjNew = g_ObjMgr.GetObjectByGuid(GuidNewObj);

                if (pObjNew)
                {
                    pObjNew->OnLoad(Blueprint);

                    //UNDO creation of this object
                    if (m_pCurrentUndoEntry)
                    {
                        transaction_object_data* pObjData = new transaction_object_data(
                                                            transaction_object_data::OBJ_CREATE,
                                                            GuidNewObj, pObjNew->GetTypeDesc().GetTypeName() );
                        pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                        m_pCurrentUndoEntry->AddCommitStep(pObjData);
                    }

                    //add object to layer
                    if (GuidNewObj != 0)
                    {
                        AddObjectToActiveLayer(GuidNewObj);
                        lstGuids.Append(GuidNewObj);

                        //UNDO of layer change
                        if (m_pCurrentUndoEntry)
                        {
                            transaction_object_data* pObjData = new transaction_object_data(
                                           transaction_object_data::OBJ_LAYER_CHANGE,
                                           GuidNewObj, pObjNew->GetTypeDesc().GetTypeName() );
                            pObjData->StoreLayer(transaction_data::TRANSACTION_NEW_STATE);
                            m_pCurrentUndoEntry->AddCommitStep(pObjData);
                        }
                    }

                    InternalSelectObject(GuidNewObj,FALSE,TRUE);
                    iAdded++;
                }
                else
                {
                    x_throw("error creating blueprint object.");
                }
            }
            else
            {
                x_throw("error reading blueprint object type.");
            }
        }
        else
        {
            x_throw("error reading blueprint object header.");
        }
    }

    Blueprint.CloseFile();

    x_catch_display;

    //ok, now we need to update the guid mappings
    UpdatePointerGuids(GuidMapList);

    return iAdded;
}

//=========================================================================

void world_editor::UpdatePointerGuids( xarray<guid_map>& GuidMapList )
{
    for (s32 i = 0; i < GuidMapList.GetCount(); i++)
    {
        guid_map& GuidMap = GuidMapList.GetAt(i);
        object* pObject = g_ObjMgr.GetObjectByGuid(GuidMap.NewGuid);
        if (pObject)
        {
            //now we need to find if any of the properties are guid props, not base\guid and point to an OldGuid
            prop_enum listProps;
            pObject->OnEnumProp(listProps);

		    for (s32 j = 0; j < listProps.GetCount(); j++)
		    {
                prop_enum::node enData = listProps[j];
			    prop_type type = (prop_type)enData.GetType();
                if (((type & PROP_TYPE_BASIC_MASK) == PROP_TYPE_GUID) && (x_stricmp(enData.GetName(),"Base\\GUID")!=0))
                {
                    prop_query pqRead;
                    guid GuidFromObj;
                    pqRead.RQueryGUID(enData.GetName(), GuidFromObj);
                    if (pObject->OnProperty(pqRead))
                    {
                        //now iterate
                        for (s32 k =0; k < GuidMapList.GetCount(); k++)
                        {
                            guid_map& GuidMapFromList = GuidMapList.GetAt(k);
                            if (GuidMapFromList.OldGuid == GuidFromObj)
                            {
                                //we found one that needs updating
                                prop_query pqWrite;
                                pqWrite.WQueryGUID(enData.GetName(), GuidMapFromList.NewGuid);
                                pObject->OnProperty(pqWrite);
                                xstring s1 = guid_ToString(GuidFromObj);
                                xstring s2 = guid_ToString(GuidMapFromList.NewGuid);
                                xstring s3 = guid_ToString(GuidMap.NewGuid);
                                TRACE(xfs("Updating Blueprint Guid Pointer %s to %s for object %s\n",(const char*)s1, (const char*)s2, (const char*)s3));
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

//=========================================================================

s32 world_editor::AddBlueprint(const char* pTheme, const char* pRelPath, editor_blueprint_ref& BlueprintReference, xbool bAndSelect, xbool bIsDirty )
{
    return AddBlueprintToSpecificLayer(
        pTheme,
        pRelPath,
        GetActiveLayer(),
        GetActiveLayerPath(),
        BlueprintReference,
        bAndSelect,
        bIsDirty,
        guid(0));
}

//=========================================================================

s32 world_editor::AddBlueprintToSpecificLayer(const char* pTheme, const char* pRelPath, 
                                              const char* pLayer, const char* pLayerPath, 
                                              editor_blueprint_ref& BlueprintReference, 
                                              xbool bAndSelect, xbool bIsDirty, guid PrimaryGuid  )
{
    s32 iAdded = 0;
    ClearSelectedObjectList();
    xbool bSuccess = TRUE;
    xbool bPrimaryGuidUsed = FALSE;
    BlueprintReference.Guid.Guid = 0;

    xarray<guid_map> GuidMapList;

    x_try;
    xstring xstrFullPath;
    GetBlueprintPath(pTheme, pRelPath, xstrFullPath);

    text_in Blueprint;
    Blueprint.OpenFile( xstrFullPath );

    while (Blueprint.ReadHeader())
    {
        if( x_stricmp( Blueprint.GetHeaderName(), "Anchor" ) == 0 )
        {
            Blueprint.ReadFields();
            vector3 anchorPos;
            if (Blueprint.GetVector3("Position",anchorPos))
            {
                guid GuidNewObj = g_ObjMgr.CreateObject( blueprint_anchor::GetObjectType() );
                object *pObjNew = g_ObjMgr.GetObjectByGuid(GuidNewObj);
                if (pObjNew)
                {
                    if (m_bLogGuids)
                        m_GuidLog.Append(GuidNewObj);

                    BlueprintReference.Anchor = GuidNewObj;
                    pObjNew->SetAttrBits(pObjNew->GetAttrBits() | object::ATTR_EDITOR_BLUE_PRINT);
                    pObjNew->OnMove(anchorPos);
                    if (bAndSelect)
                        InternalSelectObject(GuidNewObj,FALSE,TRUE);

                    //UNDO set this object
                    if (m_pCurrentUndoEntry)
                    {
                        transaction_object_data* pObjData = new transaction_object_data(
                                                            transaction_object_data::OBJ_CREATE,
                                                            GuidNewObj, pObjNew->GetTypeDesc().GetTypeName() );
                        pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                        m_pCurrentUndoEntry->AddCommitStep(pObjData);
                    }
                }
            }
        }
        else if( x_stricmp( Blueprint.GetHeaderName(), "Object" ) == 0 )
        {
            Blueprint.ReadFields();
            s32 nType = -1;
            char cType[MAX_PATH];
            if (Blueprint.GetString("SType",cType)) //New Way
            {
                //ok, we need to sync guids so we can update properties that point to other objects
                guid GuidOldObj;
                guid GuidNewObj;
                
                if ((!bPrimaryGuidUsed) && (PrimaryGuid.Guid !=0))
                {
                    GuidNewObj = PrimaryGuid;
                    g_ObjMgr.CreateObject(cType, PrimaryGuid);
                    bPrimaryGuidUsed = TRUE;
                }
                else
                {
                    GuidNewObj = g_ObjMgr.CreateObject( cType );
                }
                
                
                if (Blueprint.GetGuid("Guid",GuidOldObj))
                {
                    guid_map GuidMap;
                    GuidMap.NewGuid = GuidNewObj;
                    GuidMap.OldGuid = GuidOldObj;
                    GuidMapList.Append(GuidMap);
                }

                object *pObjNew = g_ObjMgr.GetObjectByGuid(GuidNewObj);
                if (pObjNew)
                {
                    if (m_bLogGuids)
                        m_GuidLog.Append(GuidNewObj);

                    pObjNew->OnLoad(Blueprint);
                    pObjNew->SetAttrBits(pObjNew->GetAttrBits() | object::ATTR_EDITOR_BLUE_PRINT);
                    if (bAndSelect)
                        InternalSelectObject(GuidNewObj,FALSE,TRUE);

                    //add to blueprint ref
                    BlueprintReference.ObjectsInBlueprint.Append(GuidNewObj);

                    iAdded++;

                    //UNDO set this object
                    if (m_pCurrentUndoEntry)
                    {
                        transaction_object_data* pObjData = new transaction_object_data(
                                                            transaction_object_data::OBJ_CREATE,
                                                            GuidNewObj, pObjNew->GetTypeDesc().GetTypeName() );
                        pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                        m_pCurrentUndoEntry->AddCommitStep(pObjData);
                    }
                }
                else
                {
                   x_throw(xfs("error creating blueprint object (%s).  data corrupted.  RELOAD SUGGESTED!",pRelPath));
                   bSuccess = FALSE;
                }
            }
            else if (Blueprint.GetS32("Type",nType)) //old way for backwards compatibility
            {
                x_throw( "Sorry file is too old" );
                /*
                guid GuidNewObj = g_ObjMgr.CreateObject((object::type)nType);
                object *pObjNew = g_ObjMgr.GetObjectByGuid(GuidNewObj);
                if (pObjNew)
                {
                    pObjNew->OnLoad(Blueprint);
                    pObjNew->SetAttrBits(pObjNew->GetAttrBits() | object::ATTR_EDITOR_BLUE_PRINT);
                    if (bAndSelect)
                        InternalSelectObject(GuidNewObj,FALSE);

                    //add to blueprint ref
                    BlueprintReference.ObjectsInBlueprint.Append(GuidNewObj);

                    iAdded++;

                    //UNDO set this object
                    if (m_pCurrentUndoEntry)
                    {
                        transaction_object_data* pObjData = new transaction_object_data(
                                                            transaction_object_data::OBJ_CREATE,
                                                            GuidNewObj, pObjNew->GetType());
                        pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                        m_pCurrentUndoEntry->AddCommitStep(pObjData);
                    }
                }
                else
                {
                   x_throw(xfs("error creating blueprint object (%s).  data corrupted.  RELOAD SUGGESTED!",pRelPath));
                   bSuccess = FALSE;
                }
                */
            }
            else
            {
                x_throw("error reading blueprint object type.");
                bSuccess = FALSE;
            }
        }
        else
        {
            x_throw("error reading blueprint object header.");
            bSuccess = FALSE;
        }
    }

    Blueprint.CloseFile();

    if (bSuccess)
    {
        //generate guid
        if (bPrimaryGuidUsed)
        {
            BlueprintReference.Guid = PrimaryGuid;
        }
        else
        {
            BlueprintReference.Guid = guid_New();
        }
        BlueprintReference.ThemeName = pTheme;
        BlueprintReference.RelativePath = pRelPath;
        BlueprintReference.LayerPath = pLayerPath;

        //add blueprint to layer, if we have a layer to add to
        xstring xstrLayer(pLayer);
        if (!xstrLayer.IsEmpty())
        {
            AddBlueprintToLayer(BlueprintReference, pLayer, bIsDirty);

            if (m_pCurrentUndoEntry)
            {
                transaction_bpref_data* pRefData = new transaction_bpref_data(
                                                        transaction_bpref_data::BPREF_CREATE,
                                                        BlueprintReference.Guid);
                pRefData->StoreReferenceData(transaction_data::TRANSACTION_NEW_STATE);
                m_pCurrentUndoEntry->AddCommitStep(pRefData);
            }
        }
    }

    x_catch_display;

    //ok, now we need to update the guid mappings
    UpdatePointerGuids(GuidMapList);

    return iAdded;
}

//=========================================================================

void world_editor::DeleteBlueprintsWithFile( const char* pTheme, const char* pRelPath, 
                                             xarray<editor_item_descript>& lstItems,
                                             xarray<editor_blueprint_placement>& lstPlacement )
{
    for (s32 iXaIndex = 0; iXaIndex < m_listLayers.GetCount(); iXaIndex++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(iXaIndex);
        for (s32 i = 0 ; i < eLayer.Blueprints.GetCount() ; i++)
        {
            editor_blueprint_ref& BPRef = eLayer.Blueprints.GetAt(i);
            if (x_stricmp(BPRef.ThemeName, pTheme) == 0)
            {
                if (x_stricmp(BPRef.RelativePath, pRelPath) == 0)
                {
                    //delete all objects in blueprint
                    for (s32 j = 0 ; j < BPRef.ObjectsInBlueprint.GetCount(); j++)
                    {
                        guid& ObjectGuid = BPRef.ObjectsInBlueprint.GetAt(j);

                        //make sure object isn't in selection list
                        for (s32 k=0; k < m_guidsSelectedObjs.GetCount(); k++)
                        {
                            if (m_guidsSelectedObjs.GetAt(k) == ObjectGuid) 
                            {
                                //in list already, hmmm, remove it
                                m_guidsSelectedObjs.Delete(k);
                                k = -1; //reset search
                            }
                        }

                        if (m_pCurrentUndoEntry)
                        {
                            object *pObject = g_ObjMgr.GetObjectByGuid( ObjectGuid );
                            if( pObject )
                            {
                                //UNDO for object 
                                transaction_object_data* pObjData = new transaction_object_data(
                                                                   transaction_object_data::OBJ_DELETE,
                                                                   ObjectGuid, pObject->GetTypeDesc().GetTypeName() );
                                pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);
                                m_pCurrentUndoEntry->AddCommitStep(pObjData);
                            }
                        }

                        //delete selected object
                        DestroyObjectEx( ObjectGuid, TRUE );
                    }

                    //delete the anchor object
                    //make sure object isn't in selection list
                    for (s32 m=0; m < m_guidsSelectedObjs.GetCount(); m++)
                    {
                        if (m_guidsSelectedObjs.GetAt(m) == BPRef.Anchor) 
                        {
                            //in list already, hmmm, remove it
                            m_guidsSelectedObjs.Delete(m);
                            break;
                        }
                    }

                    //add item to position list
                    editor_blueprint_placement Placement;
                    Placement.Transform = BPRef.Transform;
                    Placement.ThemeName = BPRef.ThemeName;
                    Placement.RelativePath = BPRef.RelativePath;
                    Placement.LayerPath = BPRef.LayerPath;

                    //get primary guid
                    if (BPRef.ObjectsInBlueprint.GetCount() == 1 )
                    {
                        Placement.PrimaryGuid = BPRef.ObjectsInBlueprint.GetAt(0);
                    }
                    else
                    {
                        Placement.PrimaryGuid = guid(0);
                    }

                    object *pAnchor = g_ObjMgr.GetObjectByGuid( BPRef.Anchor );
                    if (pAnchor)
                    {
                        Placement.Position = pAnchor->GetPosition();

                        if (m_pCurrentUndoEntry)
                        {
                            //UNDO for object anchor
                            transaction_object_data* pObjData = new transaction_object_data(
                                                               transaction_object_data::OBJ_DELETE,
                                                               BPRef.Anchor, pAnchor->GetTypeDesc().GetTypeName() );
                            pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);
                            m_pCurrentUndoEntry->AddCommitStep(pObjData);
                        }
                    }
                    lstPlacement.Append(Placement);

                    //delete selected anchor
                    DestroyObjectEx( BPRef.Anchor, TRUE );

                    //append item to list
                    editor_item_descript Item;
                    Item.Guid = BPRef.Guid;
                    Item.Layer = eLayer.Name;
                    Item.LayerPath = BPRef.LayerPath;
                    lstItems.Append(Item);

                    if (m_pCurrentUndoEntry)
                    {
                        //UNDO for blueprint
                        transaction_bpref_data* pRefData = new transaction_bpref_data(transaction_bpref_data::BPREF_DELETE,
                                                              BPRef.Guid);
                        pRefData->StoreReferenceData(transaction_data::TRANSACTION_OLD_STATE);
                        m_pCurrentUndoEntry->AddCommitStep(pRefData);
                    }

                    //delete blueprint
                    RemoveBlueprintFromLayer(BPRef.Guid, eLayer.Name, TRUE);

                    //reset search 
                    iXaIndex = -1;
                    break;
                }
            }
        }
    }
}

//=========================================================================

void world_editor::SelectAllMatchingBlueprints( const editor_blueprint_ref& SourceBPRef )
{
	for (s32 iXaIndex = 0; iXaIndex < m_listLayers.GetCount(); iXaIndex++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(iXaIndex);
        for (s32 i = 0 ; i < eLayer.Blueprints.GetCount() ; i++)
        {
            editor_blueprint_ref BPRef = eLayer.Blueprints.GetAt(i);
			if (x_stricmp(BPRef.ThemeName, SourceBPRef.ThemeName) == 0)
            {
				if (x_stricmp(BPRef.RelativePath, SourceBPRef.RelativePath) == 0)
                {
					SelectBlueprintObjects( BPRef, TRUE );
				}
			}
		}
	}
}

//=========================================================================

void world_editor::UpdateBlueprintsWithFile( const char* pTheme, const char* pRelPath, 
                                             const char* pNewTheme, const char* pNewRelPath, 
                                             xarray<editor_item_descript>& lstItems )
{
    for (s32 iXaIndex = 0; iXaIndex < m_listLayers.GetCount(); iXaIndex++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(iXaIndex);
        for (s32 i = 0 ; i < eLayer.Blueprints.GetCount() ; i++)
        {
            editor_blueprint_ref BPRef = eLayer.Blueprints.GetAt(i);
            if (x_stricmp(BPRef.ThemeName, pTheme) == 0)
            {
                if (x_stricmp(BPRef.RelativePath, pRelPath) == 0)
                {
                    //add item to list
                    editor_item_descript Item;
                    Item.Guid = BPRef.Guid;
                    Item.Layer = eLayer.Name;
                    Item.LayerPath = BPRef.LayerPath;
                    lstItems.Append(Item);

                    //UNDO save old layer state 
                    transaction_bpref_data* pRefData = NULL;
                    if (m_pCurrentUndoEntry)
                    {
                        pRefData = new transaction_bpref_data(transaction_bpref_data::BPREF_EDIT,
                                                              BPRef.Guid);
                        pRefData->StoreReferenceData(transaction_data::TRANSACTION_OLD_STATE);
                    }

                    //delete blueprint, update name, and re-append
                    RemoveBlueprintFromLayer(BPRef.Guid, eLayer.Name, TRUE);
                    BPRef.ThemeName = pNewTheme;
                    BPRef.RelativePath = pNewRelPath;
                    AddBlueprintToLayer(BPRef,eLayer.Name, TRUE);

                    //UNDO save new layer state
                    if (pRefData)
                    {
                        pRefData->StoreReferenceData(transaction_data::TRANSACTION_NEW_STATE);
                        m_pCurrentUndoEntry->AddCommitStep(pRefData);
                    }

                    //reset search 
                    iXaIndex = -1;
                    break;
                }
            }
        }
    }
}

//=========================================================================

guid world_editor::GetBlueprintGuidContainingObject( guid ObjectGuid )
{
    //make sure layer exists
    if (m_listLayers.GetCount() > 0 )
    {
        for (s32 i = 0; i < m_listLayers.GetCount(); i++)
		{
            editor_layer& eLayer = m_listLayers.GetAt(i);
            for (s32 j = 0; j < eLayer.Blueprints.GetCount(); j++)
		    {
                editor_blueprint_ref& Blueprint = eLayer.Blueprints.GetAt(j);
                //check anchor
                if (Blueprint.Anchor == ObjectGuid)
                {
                    return Blueprint.Guid;
                }

                //check remaining objects
                for (s32 k = 0; k < Blueprint.ObjectsInBlueprint.GetCount(); k++)
		        {
                    if (Blueprint.ObjectsInBlueprint.GetAt(k) == ObjectGuid)
                    {
                        return Blueprint.Guid;
                    }
                }
            }
        }
	}
    return guid(0);
}

//=========================================================================

xbool world_editor::ShatterSelectedBlueprints( xarray<editor_item_descript>& lstItemsAdded, xarray<editor_item_descript>& lstItemsRemoved )
{
    xbool bReturn = FALSE;
    x_try;

    lstItemsAdded.Clear();
    lstItemsRemoved.Clear();

    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        //Get Min object position to snap to
        guid ObjGuid = m_guidsSelectedObjs.GetAt(i);
        object *pObject = g_ObjMgr.GetObjectByGuid( ObjGuid );
        if( pObject )
        {
            if (pObject->GetAttrBits() & object::ATTR_EDITOR_BLUE_PRINT)
            {
                //UNDO save these properties
                transaction_object_data* pObjData = NULL;
                if (m_pCurrentUndoEntry)
                {
                    pObjData = new transaction_object_data(
                                   transaction_object_data::OBJ_EDIT,
                                   ObjGuid, pObject->GetTypeDesc().GetTypeName() );
                    pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);
                }

                //remove blueprint flag
                pObject->SetAttrBits(pObject->GetAttrBits() & ~object::ATTR_EDITOR_BLUE_PRINT);

                //UNDO save these properties
                if (pObjData)
                {
                    //UNDO finish saving of object
                    pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                    m_pCurrentUndoEntry->AddCommitStep(pObjData);
                }

                //this object is a blueprint
                editor_blueprint_ref BPRef;
                if (GetBlueprintRefContainingObject(ObjGuid, BPRef))
                {
                    //make sure this isn't already in list
                    BOOL bAdded = FALSE;
                    for (s32 j = 0; j<lstItemsRemoved.GetCount(); j++)
                    {
                        editor_item_descript& ItemInList = lstItemsRemoved.GetAt(j);
                        if (ItemInList.Guid == BPRef.Guid)
                        {
                            bAdded = TRUE;
                            break;
                        }
                    }

                    if (!bAdded)
                    {
                        //add item to list
                        editor_item_descript Item;
                        Item.IsInBlueprint = TRUE;
                        Item.Guid = BPRef.Guid;
                        Item.Layer = GetLayerContainingBlueprint(BPRef.Guid);
                        Item.LayerPath = BPRef.LayerPath;
                        lstItemsRemoved.Append(Item);
                    }

                    if (!(pObject->GetAttrBits() & object::ATTR_EDITOR_TEMP_OBJECT))
                    {
                        editor_item_descript Obj;
                        Obj.IsInBlueprint = FALSE;
                        Obj.Guid = ObjGuid;
                        Obj.Layer = GetLayerContainingBlueprint(BPRef.Guid);
                        Obj.LayerPath = BPRef.LayerPath;
                        lstItemsAdded.Append(Obj);

                        editor_object_ref ObjRef;
                        ObjRef.Guid = Obj.Guid;
                        ObjRef.LayerPath = Obj.LayerPath;
                        AddObjectToLayer(ObjRef, Obj.Layer, TRUE);

                        //UNDO adding to Layer
                        if (m_pCurrentUndoEntry)
                        {
                            transaction_object_data* pObjData = new transaction_object_data(
                                           transaction_object_data::OBJ_LAYER_CHANGE,
                                           ObjGuid, pObject->GetTypeDesc().GetTypeName() );
                            pObjData->StoreLayer(transaction_data::TRANSACTION_NEW_STATE);
                            m_pCurrentUndoEntry->AddCommitStep(pObjData);
                        }
                    }

                    bReturn = TRUE;                
                }
            }
        }
    }

    //now remove the blueprints
    for (s32 j = 0; j<lstItemsRemoved.GetCount(); j++)
    {
        editor_item_descript& Item = lstItemsRemoved.GetAt(j);

        //store UNDO for BPREFs layer
        {
            if (m_pCurrentUndoEntry)
            {
                transaction_bpref_data* pRefData = new transaction_bpref_data(
                                                        transaction_bpref_data::BPREF_DELETE,
                                                        Item.Guid);
                pRefData->StoreReferenceData(transaction_data::TRANSACTION_OLD_STATE);
                m_pCurrentUndoEntry->AddCommitStep(pRefData);
            }
        }

        RemoveBlueprintFromLayer(Item.Guid, Item.Layer, TRUE);
    }

    x_catch_display;
    return bReturn;
}

//=========================================================================

xbool world_editor::IsBlueprintSelected( )
{
    x_try;

    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        //Get Min object position to snap to
        object *pObject = g_ObjMgr.GetObjectByGuid( m_guidsSelectedObjs.GetAt(i) );
        if( pObject )
        {
            if (pObject->GetAttrBits() & object::ATTR_EDITOR_BLUE_PRINT)
            {
                //at least one blueprint is in the selection list
                return TRUE;
            }
        }
    }

    x_catch_display;
    return FALSE;
}

//=========================================================================

xbool world_editor::IsOneBlueprintSelected( editor_blueprint_ref& BPRef )
{
    x_try;

    BOOL bBPRefInit = FALSE;

    if (m_guidsSelectedObjs.GetCount()<=0) return FALSE;

    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        //Get Min object position to snap to
        guid& ObjGuid = m_guidsSelectedObjs.GetAt(i);
        object *pObject = g_ObjMgr.GetObjectByGuid( ObjGuid );
        if( !pObject ) return FALSE;

        if (pObject->GetAttrBits() & object::ATTR_EDITOR_BLUE_PRINT)
        {
            if (!bBPRefInit)
            {
                if (!GetBlueprintRefContainingObject(ObjGuid,BPRef))
                {
                    return FALSE;
                }
                bBPRefInit = TRUE;
            }
            else
            {
                //this is a blueprint, must be in current bp ref
                BOOL bFound = FALSE;
                for (s32 j = 0; j < BPRef.ObjectsInBlueprint.GetCount(); j++)
                {   
                    if (BPRef.ObjectsInBlueprint.GetAt(j) == ObjGuid)
                    {
                        bFound = TRUE;
                    }
                }

                if (BPRef.Anchor == ObjGuid)
                {
                    bFound = TRUE;
                }
                if (!bFound) return FALSE;
            }            
        }
        else
        {
            return FALSE;
        }
    }

    //this is the right BPRef
    return TRUE;

    x_catch_display;
    return FALSE;
}

//=========================================================================

guid world_editor::CreateBlueprintAnchor( )
{
    guid Guid(0);

    Guid = g_ObjMgr.CreateObject( blueprint_anchor::GetObjectType() );

    //now setup undo for each object
    if (m_pCurrentUndoEntry)
    {
        object *pObject = g_ObjMgr.GetObjectByGuid( Guid );
        if( pObject )
        {
            transaction_object_data* pObjData = new transaction_object_data(
                                                transaction_object_data::OBJ_CREATE,
                                                Guid, pObject->GetTypeDesc().GetTypeName() );
            pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
            m_pCurrentUndoEntry->AddCommitStep(pObjData);
        }
    }

    return Guid;
}

//=========================================================================

xbool world_editor::CanMakeBlueprintFromSelected()
{
    s32 nAnchorCount = 0;
    
    for (s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++)
    {
        guid Guid = m_guidsSelectedObjs.GetAt(i);
        object *pObject = g_ObjMgr.GetObjectByGuid(Guid);
        if (pObject && (pObject->GetType() == object::TYPE_EDITOR_BLUEPRINT_ANCHOR)) 
        {
            nAnchorCount++;
        }

        if (IsGlobalObject(Guid)) 
        {
            //no global objects allowed
            return FALSE;
        }
    }

    //make sure we have 1 anchor in the selection
    return (nAnchorCount == 1);
}

//=========================================================================

xbool world_editor::GetBlueprintByGuid ( guid BPGuid, editor_blueprint_ref& BlueprintReference )
{
    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(i);
        for (s32 j = 0; j < eLayer.Blueprints.GetCount(); j++)
		{
            BlueprintReference = eLayer.Blueprints.GetAt(j);
            if (BlueprintReference.Guid == BPGuid)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

//=========================================================================

xbool world_editor::GetBlueprintRefContainingObject ( guid ObjectGuid, editor_blueprint_ref& BlueprintReference )
{
    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(i);
        for (s32 j = 0; j < eLayer.Blueprints.GetCount(); j++)
		{
            BlueprintReference = eLayer.Blueprints.GetAt(j);

            //check the Anchor
            if (BlueprintReference.Anchor == ObjectGuid)
            {
                return TRUE;
            }            
            
            for (s32 k = 0; k < BlueprintReference.ObjectsInBlueprint.GetCount(); k++)
		    {
                if (BlueprintReference.ObjectsInBlueprint.GetAt(k) == ObjectGuid)
                {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

//=========================================================================

xbool world_editor::GetBlueprintRefContainingObject2( guid ObjectGuid, editor_blueprint_ref** ppBlueprintReference )
{
    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
    {
        editor_layer& eLayer = m_listLayers.GetAt(i);
        for (s32 j = 0; j < eLayer.Blueprints.GetCount(); j++)
        {
            editor_blueprint_ref& Ref = eLayer.Blueprints.GetAt(j);

            //check the Anchor
            if (Ref.Anchor == ObjectGuid)
            {
                *ppBlueprintReference = &Ref;
                return TRUE;
            }            

            for (s32 k = 0; k < Ref.ObjectsInBlueprint.GetCount(); k++)
            {
                if (Ref.ObjectsInBlueprint.GetAt(k) == ObjectGuid)
                {
                    *ppBlueprintReference = &Ref;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

//=========================================================================

xbool world_editor::GetBlueprintRefContainingAnchor ( guid AnchorGuid, editor_blueprint_ref& BlueprintReference )
{
    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(i);
        for (s32 j = 0; j < eLayer.Blueprints.GetCount(); j++)
		{
            BlueprintReference = eLayer.Blueprints.GetAt(j);
            if (BlueprintReference.Anchor == AnchorGuid)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

//=========================================================================

void world_editor::SelectBlueprintObjects( editor_blueprint_ref& BlueprintReference, xbool bAddSelection )
{
    m_LastSelectedGuidA = m_LastSelectedGuidB;
    m_LastSelectedGuidB = BlueprintReference.Anchor;

    //loop through all blueprint object
    for( s32 i=0; i <= BlueprintReference.ObjectsInBlueprint.GetCount(); i++ )
    {       
        guid GuidToCheck;
        if (i < BlueprintReference.ObjectsInBlueprint.GetCount())
        {
            GuidToCheck = BlueprintReference.ObjectsInBlueprint.GetAt(i);
        }
        else
        {
            //check the anchor
            GuidToCheck = BlueprintReference.Anchor;
        }

        if (bAddSelection)
        {
            //ADD TO LIST
            BOOL bInList = FALSE;
            // Loop through all selected objects
            for( s32 j=0; j < m_guidsSelectedObjs.GetCount(); j++ )
            {       
                if (GuidToCheck == m_guidsSelectedObjs.GetAt(j))
                {
                    bInList = TRUE;
                }
            }

            if (!bInList)
            {
                object *pObject = g_ObjMgr.GetObjectByGuid( GuidToCheck );
                if( pObject )
                {
                    //add the object to selection list
                    InternalSelectObject(GuidToCheck, FALSE,TRUE);
                }
            }
        }
        else
        {
            //REMOVE FROM LIST
            xarray<guid> tempArray;
            for( s32 j=0; j < m_guidsSelectedObjs.GetCount(); j++ )
            {       
                if (GuidToCheck != m_guidsSelectedObjs.GetAt(j))
                {
                    //not in blueprint so add it to list
                    tempArray.Append(m_guidsSelectedObjs.GetAt(j));
                }
                else
                {
                    //found the item, so set its flag
                    object *pObject = g_ObjMgr.GetObjectByGuid( GuidToCheck );
                    if( pObject )
                    {
                        pObject->SetAttrBits(pObject->GetAttrBits() & ~object::ATTR_EDITOR_SELECTED);
                    }
                }
            }
            m_guidsSelectedObjs.Clear();
            m_guidsSelectedObjs = tempArray;
        }
    }
}

//=========================================================================

xbool world_editor::HasSingleBlueprintChanged( const editor_blueprint_ref& BPRef, xarray<prop_container>& DifferentProperties )
{
    xbool bReturnVal = FALSE;
    DifferentProperties.Clear();

    if (BPRef.ObjectsInBlueprint.GetCount() != 1)
    {
        return FALSE;
    }

    xstring xstrFullPath;
    GetBlueprintPath(BPRef.ThemeName, BPRef.RelativePath, xstrFullPath);

    text_in Blueprint;
    Blueprint.OpenFile( xstrFullPath );

    while (Blueprint.ReadHeader())
    {
        if( x_stricmp( Blueprint.GetHeaderName(), "Anchor" ) == 0 )
        {
            //skip anchor
            Blueprint.ReadFields();
        }
        else if( x_stricmp( Blueprint.GetHeaderName(), "Object" ) == 0 )
        {
            Blueprint.ReadFields();
            s32 nType = -1;
            char cType[MAX_PATH];
            if (Blueprint.GetString("SType",cType))
            {
                guid GuidBPObj;
                Blueprint.GetGuid("Guid",GuidBPObj);

                //create a temp object to compare against
                guid GuidTempObj = g_ObjMgr.CreateObject( cType );
                object* pTempObj = g_ObjMgr.GetObjectByGuid(GuidTempObj);
                object *pObjToCompare = g_ObjMgr.GetObjectByGuid(BPRef.ObjectsInBlueprint.GetAt(0));
                if (pTempObj)
                {
                    pTempObj->OnLoad(Blueprint);

                    //now compare both objects and see if any properties differ
                    if (pObjToCompare && x_strcmp(pObjToCompare->GetTypeDesc().GetTypeName(), cType) == 0)
                    {
                        //put all the data in prop containers
                        xarray<prop_container> FileProps;
                        xarray<prop_container> ObjectProps;
                        pTempObj->OnCopy( FileProps );
                        pObjToCompare->OnCopy( ObjectProps );

                        //now do the compares, the prop list may not be the same, this is going to get ugly
                        for (s32 i = 0; i < ObjectProps.GetCount(); i++)
                        {
                            prop_container& ObjProp = ObjectProps.GetAt(i);
                            xbool bPropDiff = TRUE;
                            for (s32 j = 0; j < FileProps.GetCount(); j++)
                            {
                                prop_container& FlProp = FileProps.GetAt(j);
                                if (x_strcmp(ObjProp.GetName(), FlProp.GetName()) == 0)
                                {
                                    //ok we matched up   
                                    if (IsIgnoredTypeProp(ObjProp.GetName()))
                                    {
                                        bPropDiff = FALSE;
                                    }
                                    else if (ObjProp.GetTypeFlags() & PROP_TYPE_DONT_SAVE)
                                    {
                                        bPropDiff = FALSE;
                                    }
                                    else if (ObjProp.GetType() != FlProp.GetType())
                                    {
                                        //file types don't match this is bad!!!
                                        x_DebugMsg("While comparing blueprints we found property types with the same name that dont match type!\n");
                                        bPropDiff = FALSE;
                                    }
                                    else
                                    {
                                        //ok, same name, same type...
                                        if (ObjProp.GetDataSize() != FlProp.GetDataSize())
                                        {
                                            //data sizes don't match, definitely not the same data
                                            bPropDiff = TRUE;
                                        }
                                        else
                                        {
                                            //sizes match, must do a thourough compare
                                            switch( ObjProp.GetType() )
                                            {
                                            case PROP_TYPE_FLOAT:   
                                                {
                                                    f32 Dat1;
                                                    f32 Dat2;
                                                    ObjProp.GetFloat( Dat1 );
                                                    FlProp.GetFloat( Dat2 );
                                                    bPropDiff = (Dat1 != Dat2);
                                                }
                                                break;
                                            case PROP_TYPE_VECTOR3:     
                                                {
                                                    vector3 Dat1;
                                                    vector3 Dat2;
                                                    ObjProp.GetVector3( Dat1 );
                                                    FlProp.GetVector3( Dat2 );
                                                    bPropDiff = (Dat1 != Dat2);
                                                }
                                                break;
                                            case PROP_TYPE_INT:         
                                                {
                                                    int Dat1;
                                                    int Dat2;
                                                    ObjProp.GetInt( Dat1 );
                                                    FlProp.GetInt( Dat2 );
                                                    bPropDiff = (Dat1 != Dat2);
                                                }
                                                break;
                                            case PROP_TYPE_BOOL:       
                                                {
                                                    xbool Dat1;
                                                    xbool Dat2;
                                                    ObjProp.GetBool( Dat1 );
                                                    FlProp.GetBool( Dat2 );
                                                    bPropDiff = (Dat1 != Dat2);
                                                }
                                                break;
                                            case PROP_TYPE_ROTATION:    
                                                {
                                                    radian3 Dat1;
                                                    radian3 Dat2;
                                                    ObjProp.GetRotation( Dat1 );
                                                    FlProp.GetRotation( Dat2 );
                                                    bPropDiff = (Dat1 != Dat2);
                                                }
                                                break;
                                            case PROP_TYPE_ANGLE:       
                                                {
                                                    radian Dat1;
                                                    radian Dat2;
                                                    ObjProp.GetAngle( Dat1 );
                                                    FlProp.GetAngle( Dat2 );
                                                    bPropDiff = (Dat1 != Dat2);
                                                }
                                                break;
                                            case PROP_TYPE_BBOX:        
                                                {
                                                    bbox Dat1;
                                                    bbox Dat2;
                                                    ObjProp.GetBBox( Dat1 );
                                                    FlProp.GetBBox( Dat2 );
                                                    bPropDiff = ((Dat1.Min != Dat2.Min) || (Dat1.Max != Dat2.Max));
                                                }
                                                break;
                                            case PROP_TYPE_GUID:       
                                                {
                                                    guid Dat1;
                                                    guid Dat2;
                                                    ObjProp.GetGUID( Dat1 );
                                                    FlProp.GetGUID( Dat2 );
                                                    bPropDiff = (Dat1 != Dat2);
                                                }
                                                break;
                                            case PROP_TYPE_COLOR:    
                                                {
                                                    xcolor Dat1;
                                                    xcolor Dat2;
                                                    ObjProp.GetColor( Dat1 );
                                                    FlProp.GetColor( Dat2 );
                                                    bPropDiff = (Dat1 != Dat2);
                                                }
                                                break;
                                            case PROP_TYPE_FILENAME:    
                                            case PROP_TYPE_STRING:      
                                            case PROP_TYPE_ENUM:        
                                            case PROP_TYPE_BUTTON:      
                                            case PROP_TYPE_EXTERNAL:   
                                                //strings
                                                {
                                                    char String1[256];
                                                    char String2[256];
                                                    ObjProp.GetString( String1 );
                                                    FlProp.GetString( String2 );
                                                    bPropDiff = (x_stricmp(String1,String2) != 0 );
                                                }
                                                break;
                                            default: 
                                                bPropDiff = TRUE; 
                                                break;
                                            }
                                        }
                                    }
                                    break;
                                }
                            }

                            if (bPropDiff)
                            {
                                bReturnVal = TRUE;
                                DifferentProperties.Append(ObjProp);
                            }
                        }
                    }
                    else
                    {
                        LOG_ERROR("Editor", "Blueprint Property Overrides could not be saved");
                    }

                    //destory the temp object
                    DestroyObjectEx(GuidTempObj, TRUE);
                }
            }
        }
    }

    Blueprint.CloseFile();
    return bReturnVal;
}

//=========================================================================
// Zoning - note: id 0 is undefined zone (valid zones are 1 - 255)
//=========================================================================

void world_editor::SelectZone( const char* pZone )
{
    m_SelectedZone = GetZoneId(pZone);
}

//=========================================================================

void world_editor::UnSelectZone( void )
{
    m_SelectedZone = 0;
}

//=========================================================================

const char* world_editor::GetZoneLayer( const char* pZone )
{
    for( s32 i=0; i < m_listZones.GetCount(); i++ )
    {       
        editor_zone_ref &Zone = m_listZones.GetAt(i);
        if (x_stricmp(Zone.Name, pZone)==0)
        {
            return Zone.Layer;
        }
    }
    return "";
}

//=========================================================================

u8 world_editor::GetZoneId( const char* pZone )
{
    for( s32 i=0; i < m_listZones.GetCount(); i++ )
    {       
        editor_zone_ref &Zone = m_listZones.GetAt(i);
        if (x_stricmp(Zone.Name, pZone)==0)
        {
            return Zone.Id;
        }
    }
    return 0;
}

//=========================================================================

u16 world_editor::GetZoneIdForPortal( const char* pPortalName )
{
    xarray<guid> PortalList;
    g_WorldEditor.GetListOfPortals(PortalList);

    //we need to update the zones
    for ( s32 j = 0; j < PortalList.GetCount(); j++ )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid(PortalList.GetAt(j));
        if (pObj && pObj->IsKindOf( zone_portal::GetRTTI()) )
        {
            //this is a portal
            zone_portal* pPortal = (zone_portal*)pObj;
            if (x_strcmp(pPortalName, pPortal->GetName()) == 0)
            {
                u16 ZoneInfo = pPortal->GetZone1();
                ZoneInfo = (ZoneInfo&0xff)|(pPortal->GetZone2()<<8);
                return ZoneInfo;
            }
        }
    }

    return 0;
}

//=========================================================================

const char* world_editor::GetZoneForId( u8 uId )
{
    for( s32 i=0; i < m_listZones.GetCount(); i++ )
    {       
        editor_zone_ref &Zone = m_listZones.GetAt(i);
        if ( Zone.Id == uId )
        {
            return Zone.Name;
        }
    }
    return "";
}

//=========================================================================

xbool world_editor::DoesZoneExist( const char* pZone )
{
    for( s32 i=0; i < m_listZones.GetCount(); i++ )
    {       
        editor_zone_ref &Zone = m_listZones.GetAt(i);
        if (x_stricmp(Zone.Name, pZone)==0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

//=========================================================================

xbool world_editor::DoesZoneExist( u8 uId )
{
    for( s32 i=0; i < m_listZones.GetCount(); i++ )
    {       
        editor_zone_ref &Zone = m_listZones.GetAt(i);
        if ( Zone.Id == uId )
        {
            return TRUE;
        }
    }
    return FALSE;
}

//=========================================================================

u8 world_editor::GetZoneCount( void )
{
    return (u8)m_listZones.GetCount();
}

//=========================================================================

xbool world_editor::CreateZone( const char* pZone, const char* pLayer )
{
    if ((GetZoneCount() < U8_MAX ) && !DoesZoneExist(pZone))
    {
        editor_zone_ref Zone;
        Zone.Name = pZone;
        Zone.Layer = pLayer;
        //get a valid unused zone id
        for (u8 i=1; i <= U8_MAX; i++)
        {
            if (!DoesZoneExist(i))
            {
                //found a good slot
                Zone.Id = i;
                m_listZones.Append(Zone);

                if (m_pCurrentUndoEntry)
                {
                    //UNDO
                    transaction_zone_data* pData = new transaction_zone_data(transaction_zone_data::ZONE_CREATE,"",Zone.Name,Zone.Layer);
                    m_pCurrentUndoEntry->AddCommitStep(pData);
                }

                return TRUE;
            }
        }
    }

    return FALSE;
}

//=========================================================================

xbool world_editor::DeleteZone( const char* pZone )
{
    for( s32 i=0; i < m_listZones.GetCount(); i++ )
    {       
        editor_zone_ref &Zone = m_listZones.GetAt(i);
        if (x_stricmp(Zone.Name, pZone)==0)
        {
            //UNDO now remove layer
            if (m_pCurrentUndoEntry)
            {
                transaction_zone_data* pData = new transaction_zone_data(transaction_zone_data::ZONE_DELETE,Zone.Name,"",Zone.Layer);
                m_pCurrentUndoEntry->AddCommitStep(pData);
            }

            m_listZones.Delete(i);
            return TRUE;
        }
    }
    return FALSE;
}

//=========================================================================

xbool world_editor::DeleteZone( u8 uId )
{
    for( s32 i=0; i < m_listZones.GetCount(); i++ )
    {       
        editor_zone_ref &Zone = m_listZones.GetAt(i);
        if ( Zone.Id == uId )
        {
            //UNDO now remove layer
            if (m_pCurrentUndoEntry)
            {
                transaction_zone_data* pData = new transaction_zone_data(transaction_zone_data::ZONE_DELETE,Zone.Name,"",Zone.Layer);
                m_pCurrentUndoEntry->AddCommitStep(pData);
            }

            m_listZones.Delete(i);
            return TRUE;
        }
    }
    return FALSE;
}

//=========================================================================

xbool world_editor::RenameZone( const char* pOldName, const char* pNewName )
{
    if (!DoesZoneExist(pNewName))
    {
        for( s32 i=0; i < m_listZones.GetCount(); i++ )
        {       
            editor_zone_ref &Zone = m_listZones.GetAt(i);
            if (x_stricmp(Zone.Name, pOldName)==0)
            {
                //UNDO
                if (m_pCurrentUndoEntry)
                {
                    transaction_zone_data* pData = new transaction_zone_data(
                        transaction_zone_data::ZONE_RENAME,Zone.Name,pNewName,Zone.Layer);
                    m_pCurrentUndoEntry->AddCommitStep(pData);
                }
                Zone.Name = pNewName;
                return TRUE;
            }
        }
    }
    return FALSE;
}

//=========================================================================

xbool world_editor::SaveZoneFile( void )
{
    if (m_pHandler && m_pHandler->IsFileReadOnly(xfs("%s\\Zoning.ProjectInfo", g_Project.GetWorkingPath())))
    {
        LOG_WARNING("Editor", "The Zoning.ProjectInfo File could not be saved since it is marked readonly!");
        return FALSE;
    }

    x_try;
    {
        if (m_pHandler) m_pHandler->SetProgressRange(0,m_listZones.GetCount());

        text_out ZoneFile;
        ZoneFile.OpenFile( xfs("%s\\Zoning.ProjectInfo", g_Project.GetWorkingPath()) );

        //Add version number
        ZoneFile.AddHeader("FileInfo",1);
        ZoneFile.AddF32("Version", GetVersionNumber());
        ZoneFile.AddEndLine();

        ZoneFile.AddHeader("ZoneData",m_listZones.GetCount());

        for (s32 i = 0; i < m_listZones.GetCount(); i++)
	    {
            editor_zone_ref &Zone = m_listZones.GetAt(i);

            ZoneFile.AddS32     ("Id",              Zone.Id             );
            ZoneFile.AddString  ("Name",            Zone.Name           );
            ZoneFile.AddString  ("Layer",           Zone.Layer          );
            ZoneFile.AddF32     ("SndAbsorption",   Zone.SndAbsorption  );
            ZoneFile.AddS32     ("MinPlayers",      Zone.MinPlayers     );
            ZoneFile.AddS32     ("MaxPlayers",      Zone.MaxPlayers     );
            ZoneFile.AddString  ("FogMap",          Zone.FogMap         );
            ZoneFile.AddBool    ("QuickFog",        Zone.QuickFog       );
            ZoneFile.AddEndLine (                                       );

            if (m_pHandler) m_pHandler->SetProgress(i+1);         
        }

        ZoneFile.CloseFile();
        return TRUE;
    }
    x_catch_display;
    return FALSE;
}

//=========================================================================

xbool world_editor::LoadZoneFile( void )
{
    ClearZoneList();
    return LoadZoneListFromFile( xfs("%s\\Zoning.ProjectInfo", g_Project.GetWorkingPath()),m_listZones);
}

//=========================================================================

xbool world_editor::LoadZoneListFromFile( const char* pZoneFile, xarray<editor_zone_ref>& ZoneList )
{
    x_try;

    ZoneList.Clear();
    
    text_in ZoneFile;
    ZoneFile.OpenFile( pZoneFile );
    f32 fVersion = 0.0f;

    while (ZoneFile.ReadHeader())
    {
        if( x_stricmp( ZoneFile.GetHeaderName(), "FileInfo" ) == 0 )
        {
            ZoneFile.ReadFields();
            ZoneFile.GetF32("Version",fVersion);
        }
        else if( x_stricmp( ZoneFile.GetHeaderName(), "ZoneData" ) == 0 )
        {
            s32 nCount = ZoneFile.GetHeaderCount();

            if (m_pHandler) m_pHandler->SetProgressRange(0, nCount);

            for (s32 i =0; i < nCount; i++)
            {
                ZoneFile.ReadFields();
                char cName[MAX_PATH];
                char cLayer[MAX_PATH];
                char cFogMap[MAX_PATH];
                xbool QuickFog;
                s32 nId;
                f32 sndAbsp;
                s32 MinPlayers;
                s32 MaxPlayers;

                if (!ZoneFile.GetS32("Id",nId))
                {
                    x_throw("ZoneFile error reading resource ID.");
                }                
                if (!ZoneFile.GetString("Name",cName))
                {
                    x_throw("ZoneFile error reading resource name.");
                }
                if (!ZoneFile.GetString("Layer",cLayer))
                {
                    x_throw("ZoneFile error reading resource layer.");
                }
                if (!ZoneFile.GetF32("SndAbsorption",sndAbsp))
                {
                    sndAbsp = 1.0f;
                }
                if( !ZoneFile.GetS32("MinPlayers", MinPlayers ) )
                {
                    MinPlayers = 0;
                }
                if( !ZoneFile.GetS32("MaxPlayers", MaxPlayers ) )
                {
                    MaxPlayers = 32;
                }
                if (!ZoneFile.GetString("FogMap",cFogMap))
                {
                    cFogMap[0] = '\0';
                }
                if (!ZoneFile.GetBool("QuickFog",QuickFog))
                {
                    QuickFog = FALSE;
                }

                editor_zone_ref Zone;
                Zone.Id             = nId;
                Zone.Name           = cName;
                Zone.Layer          = cLayer;
                Zone.SndAbsorption  = sndAbsp;
                Zone.MinPlayers     = MinPlayers;
                Zone.MaxPlayers     = MaxPlayers;
                x_strcpy( Zone.FogMap, cFogMap );
                Zone.QuickFog       = QuickFog;

                ZoneList.Append(Zone);

                if (m_pHandler) m_pHandler->SetProgress(i);         
            }
        }
    }

    return TRUE;

    x_catch_display;
    return FALSE;
}

//=========================================================================

void world_editor::ClearZoneList( void )
{
    m_SelectedZone = 0;
    m_listZones.Clear();
}

//=========================================================================

void world_editor::GetListOfPortals( xarray<guid>& List )
{
    List.Clear();
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_ZONE_PORTAL );
    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        if( pObject != NULL )
        {
            List.Append(pObject->GetGuid());
        }   
        SlotID = g_ObjMgr.GetNext( SlotID );
    }   
}

//=========================================================================

void world_editor::ZoneSanityCheck( void )
{
    x_DebugMsg("\n====Start Zone Sanity Check====\n");

    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& Layer = m_listLayers.GetAt(i);

        xbool bGlobalsLayer = FALSE;
        if (x_strcmp(Layer.Name, GetGlobalLayer()) == 0 )
        {
            bGlobalsLayer = TRUE;
        }

        //check all objects
        for (s32 j = 0; j < Layer.Objects.GetCount(); j++)
		{
            editor_object_ref& ObjRef = Layer.Objects.GetAt(j);
            xstring xstrPath(ObjRef.LayerPath);
            xstring xstrSubPath = xstrPath.Right(xstrPath.GetLength()-1);
            s32 i = xstrSubPath.Find('\\',0);
            if (i != -1)
            {
                xstring xstrZone = xstrSubPath.Left(i);
                u16 ZoneId = 0;
                
                if (bGlobalsLayer)
                    ZoneId = GetZoneIdForPortal(xstrZone);
                else
                    ZoneId = GetZoneId(xstrZone);

                if (ZoneId != 0)
                {
                    object* pObject = g_ObjMgr.GetObjectByGuid(ObjRef.Guid);
                    if (pObject)
                    {
                        if (pObject->GetZone1() != (ZoneId&0xff))
                        {
                            x_DebugMsg(xfs("ZONE ERROR:  Found object whose path (%s) differs from objects zone info, updating object!\n", (const char*)xstrZone));
                            pObject->SetZone1(ZoneId&0xff);
                            //MarkLayerDirty(Layer.Name);
                        }

                        if (pObject->GetZone2() != (ZoneId>>8))
                        {
                            x_DebugMsg(xfs("ZONE ERROR:  Found object whose path (%s) differs from objects zone info, updating object!\n", (const char*)xstrZone));
                            pObject->SetZone2(ZoneId>>8);
                            //MarkLayerDirty(Layer.Name);
                        }
                    }
                }
            }
        }

        //check all Blueprints
        for ( j = 0; j < Layer.Blueprints.GetCount(); j++)
		{
            editor_blueprint_ref& BPRef = Layer.Blueprints.GetAt(j);
            xstring xstrPath(BPRef.LayerPath);
            xstring xstrSubPath = xstrPath.Right(xstrPath.GetLength()-1);
            s32 i = xstrSubPath.Find('\\',0);
            if (i != -1)
            {
                xstring xstrZone = xstrSubPath.Left(i);
                u16 ZoneId = 0;
                
                if (bGlobalsLayer)
                    ZoneId = GetZoneIdForPortal(xstrZone);
                else
                    ZoneId = GetZoneId(xstrZone);

                if (ZoneId != 0)
                {
                    //check all objects in blueprint
                    for ( s32 k = 0; k < BPRef.ObjectsInBlueprint.GetCount(); k++)
                    {
                        object* pObject = g_ObjMgr.GetObjectByGuid(BPRef.ObjectsInBlueprint.GetAt(k));
                        if (pObject)
                        {
                            if (pObject->GetZone1() != (ZoneId&0xff))
                            {
                                x_DebugMsg(xfs("ZONE ERROR:  Found blueprint whose path (%s) differs from objects zone info, updating object!\n", (const char*)xstrZone));
                                pObject->SetZone1(ZoneId&0xff);
                                //MarkLayerDirty(Layer.Name);
                            }

                            if (pObject->GetZone2() != (ZoneId>>8))
                            {
                                x_DebugMsg(xfs("ZONE ERROR:  Found blueprint whose path (%s) differs from objects zone info, updating object!\n", (const char*)xstrZone));
                                pObject->SetZone2(ZoneId>>8);
                                //MarkLayerDirty(Layer.Name);
                            }
                        }
                    }
                }
            }
        }
    }

    x_DebugMsg("==== End Zone Sanity Check ====\n\n");
}

//=========================================================================

xbool world_editor::SetObjectsZone( guid ObjGuid, u8 Zone1, u8 Zone2 )
{
    object *pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);
    if (pObject)
    {
        //UNDO
        transaction_object_data* pObjData = NULL;
        if (m_pCurrentUndoEntry)
        {
            //UNDO for object change before
            pObjData = new transaction_object_data(
                           transaction_object_data::OBJ_EDIT,
                           ObjGuid, pObject->GetTypeDesc().GetTypeName() );
            pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);
        }

        prop_query pq;
        u16 ZoneInfo = Zone2;
        ZoneInfo = ZoneInfo<<8;
        ZoneInfo+= Zone1;
        pq.WQueryInt( "Base\\ZoneInfo", ZoneInfo );
        return pObject->OnProperty( pq );

        if (m_pCurrentUndoEntry)
        {
            //UNDO for object change after
            pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
            m_pCurrentUndoEntry->AddCommitStep(pObjData);
        }
    }
    return FALSE;
}

//=========================================================================

xbool world_editor::SetBlueprintObjectsZone( guid BPGuid, u8 Zone1, u8 Zone2 )
{
    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(i);
        for (s32 j = 0; j < eLayer.Blueprints.GetCount(); j++)
		{
            editor_blueprint_ref& Blueprint = eLayer.Blueprints.GetAt(j);
            if (Blueprint.Guid == BPGuid)
            {
                BOOL bReturn = TRUE;
                for( s32 k=0; k < Blueprint.ObjectsInBlueprint.GetCount(); k++ )
                {       
                    guid ObjGuid = Blueprint.ObjectsInBlueprint.GetAt(k);
                    if (!SetObjectsZone(ObjGuid, Zone1, Zone2 ))
                    {
                        bReturn = FALSE;
                    }
                }
                return bReturn;
            }
        }
    }
    return FALSE;
}

//=========================================================================

void world_editor::GetZoneList( xarray<xstring>& List )
{
    List.Clear();
    for( s32 i=0; i < m_listZones.GetCount(); i++ )
    {       
        editor_zone_ref &Zone = m_listZones.GetAt(i);
        List.Append(Zone.Name);
    }
}

//=========================================================================

void world_editor::GetZoneListForLayer( const char* pLayer, xarray<xstring>& List )
{
    List.Clear();
    for( s32 i=0; i < m_listZones.GetCount(); i++ )
    {       
        editor_zone_ref &Zone = m_listZones.GetAt(i);
        if (x_stricmp(Zone.Layer, pLayer) == 0)
        {
            List.Append(Zone.Name);
        }
    }
}

//=========================================================================

xbool world_editor::IsPartOfAZone( const char* pPath, const char* pLayer, xstring &Zone )
{
    xstring xstrPath(pPath);

    //do the layer path parsing
    ASSERT(xstrPath.GetAt(0) == '\\');
    xstring xstrSubPath = xstrPath.Right(xstrPath.GetLength()-1);
    s32 i = xstrSubPath.Find('\\',0);
    if (i != -1)
    {
        //now parse the layer path
        Zone = xstrSubPath.Left(i);
        if (DoesZoneExist(Zone))
        {
            return TRUE;
        }
    }
    return FALSE;
}

//=========================================================================

s32 world_editor::GetZoneIndex( u8 uId ) const
{
    for( s32 i=0; i < m_listZones.GetCount(); i++ )
    {       
        editor_zone_ref &Zone = m_listZones.GetAt(i);
        if ( Zone.Id == uId )
        {
            return i;
        }
    }

    ASSERT( 0 );
    return 0;
}

//=========================================================================

void world_editor::BuildZoneList()
{
    //
    // TODO: Do this properly.
    // Hacked Here for testing.
    //
    slot_id ID;
    s32     i;
    s32     nZones   = GetZoneCount();
    s32     nPortals = 0;
    

    // Count portals
    for( ID = g_ObjMgr.GetFirst( object::TYPE_ZONE_PORTAL );ID != SLOT_NULL; ID = g_ObjMgr.GetNext( ID ) )
    {
        nPortals++;
    }

    if( nZones && nPortals )
    {
        // OKay we are ready to start adding
        g_ZoneMgr.Reset();
        g_ZoneMgr.AddStart( nZones, nPortals );

        static bbox BBox[256];
        // Clear all the bbox for a zone
        for( i=0; i<256; i++ )
        {
            BBox[i].Clear();
        }

        // Build zone bboxes (loop throw all the types)
        for( i=0; i<object::TYPE_END_OF_LIST; i++ )
        {
            // SB - Skip lights since these never get exported to the PS2 - they get baked into the vert colors
            if ( i == object::TYPE_LIGHT )
                continue ;
            
            // SB - Skip paths because they are never rendered on the PS2 and the editor has special rendering code
            if ( i == object::TYPE_PATH )
                continue ;

            // Go throw all the obejcts of this type
            for( ID = g_ObjMgr.GetFirst( (object::type)i); ID != SLOT_NULL; ID = g_ObjMgr.GetNext( ID ) )
            {
                // Get the object
                object& Object = *g_ObjMgr.GetObjectBySlot( ID );
                if( &Object == NULL ) 
                    continue;
                
                // Add bbox
                u16  Zone1    = Object.GetZone1();
                u16  Zone2    = Object.GetZone2();

                /////////////////////////////////////
                // Why are we also adding the bbox of the lights?
                ///////////////////////////////////
                if( Object.GetAttrBits() & object::ATTR_SOUND_SOURCE )
                {
                    if( Zone1 ) BBox[ Zone1 ] += bbox( Object.GetPosition(), 100.0f );
                    if( Zone2 ) BBox[ Zone2 ] += bbox( Object.GetPosition(), 100.0f );

                }
                else
                {
                    if( Zone1 ) BBox[ Zone1 ] += Object.GetBBox();
                    if( Zone2 ) BBox[ Zone2 ] += Object.GetBBox();
                }
            }
        }

        // Okay in theory we should have all the bboxes for each zone
        // now we need to add the actual zones into the data base
        for( i=1; i<256; i++ )
        {
            if( DoesZoneExist( (u8)i ) )
            {
                g_ZoneMgr.AddZone( 
                    BBox[ i ], 
                    i, 
                    m_listZones[ GetZoneIndex(i) ].SndAbsorption,
                    m_listZones[ GetZoneIndex(i) ].MinPlayers, 
                    m_listZones[ GetZoneIndex(i) ].MaxPlayers,
                    m_listZones[ GetZoneIndex(i) ].EnvMap,
                    m_listZones[ GetZoneIndex(i) ].FogMap,
                    m_listZones[ GetZoneIndex(i) ].QuickFog );
            }
        }

        // Okay now we are ready to start adding portals
        for( ID = g_ObjMgr.GetFirst( object::TYPE_ZONE_PORTAL );ID != SLOT_NULL; ID = g_ObjMgr.GetNext( ID ) )
        {
            // Get the general object first
            object* pObject = g_ObjMgr.GetObjectBySlot( ID );
            if( pObject == NULL )
                continue;

            // Cast object to a zone portal
            zone_portal&   Portal = zone_portal::GetSafeType( *pObject );                

            // Compute portal edges
            vector3 Edges[4];
            f32     Width  = Portal.GetWidth()/2;
            f32     Height = Portal.GetHeight()/2;
            matrix4 L2W    = Portal.GetL2W();


            Edges[0] = L2W * vector3( -Width, Height, 0 );
            Edges[1] = L2W * vector3( -Width,-Height, 0 );
            Edges[2] = L2W * vector3(  Width,-Height, 0 );
            Edges[3] = L2W * vector3(  Width, Height, 0 );

            plane Plane;
            Plane.Setup( Edges[0], Edges[1], Edges[2] );

            bbox BBox = Portal.GetBBox();
            BBox.AddVerts( &(Edges[0] + Plane.Normal*100), 1 );
            BBox.AddVerts( &(Edges[0] - Plane.Normal*100), 1 );

            // Add the portal into the data base
            g_ZoneMgr.AddPortal( Portal.GetGuid(), BBox, Edges, Portal.GetZone1(), Portal.GetZone2(), Portal.GetSoundOcclusion() );
        }

        // Done adding zones and portals
        g_ZoneMgr.AddEnd();
    }
    

    //now must move the player
    UpdatePlayerZone();
}

//=========================================================================
// Portals
//=========================================================================

xbool world_editor::IsPartOfAPortal( const char* pPath, const char* pLayer, u8 &Zone1, u8 &Zone2 )
{
    if (x_strcmp(pLayer, GetGlobalLayer())==0)
    {
        xstring xstrPath(pPath);
        ASSERT(xstrPath.GetAt(0) == '\\');

        xstring xstrSubPath = xstrPath.Right(xstrPath.GetLength()-1);
        s32 i = xstrSubPath.Find('\\',0);
        if (i != -1)
        {
            xstring xstrPortal = xstrSubPath.Left(i);
            //iterate through all portals and see if this exists
            slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_ZONE_PORTAL );
            while( SlotID != SLOT_NULL )
            {
                object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
                if( pObject != NULL )
                {
                    char cPortalName[MAX_OBJECT_NAME_LENGTH];
                    prop_query pqRead;
                    pqRead.RQueryString( "Base\\Name", &cPortalName[0]);
                    pObject->OnProperty( pqRead );

                    if (x_strcmp(cPortalName, xstrPortal)==0)
                    {
                        //found it
                        zone_portal* pPortal = &zone_portal::GetSafeType( *pObject );
                        if (pPortal)
                        {
                            Zone1 = pPortal->GetZone1();
                            Zone2 = pPortal->GetZone2();
                            return TRUE;
                        }
                    }
                }   
                SlotID = g_ObjMgr.GetNext( SlotID );
            }   
        }
    }
    return FALSE;
}

//=========================================================================

xbool world_editor::CanAddRemovePortals( void )
{
    if (m_pHandler && !m_pHandler->IsFileReadOnly(xfs("%s\\Portals.ProjectInfo", g_Project.GetWorkingPath())))
    {
        return TRUE;
    }
    return FALSE;
}

//=========================================================================

xbool world_editor::SavePortals( xarray<editor_object_ref>& PortalList )
{
    if (!CanAddRemovePortals())
    {
        //readonly portals file
        LOG_WARNING("Editor", "The Portals.ProjectInfo File could not be saved since it is marked readonly! Data could be lost!");
        return FALSE;
    }

    x_try;
    text_out PortalFile;
    PortalFile.OpenFile( xfs("%s\\Portals.ProjectInfo", g_Project.GetWorkingPath()) );
    if (m_pHandler) m_pHandler->SetProgressRange(0,PortalList.GetCount());

    //Add version number
    PortalFile.AddHeader("PortalInfo",1);
    PortalFile.AddF32("FileVersion", GetVersionNumber());
    PortalFile.AddEndLine();

    PortalFile.AddHeader("Portals",  PortalList.GetCount());
    for (s32 i = 0; i < PortalList.GetCount(); i++)
	{
        editor_object_ref& PortalRef = PortalList.GetAt(i);
        PortalFile.AddGuid  ("Guid",      PortalRef.Guid);
        PortalFile.AddString("LayerPath", PortalRef.LayerPath);
        PortalFile.AddEndLine();
        if (m_pHandler) m_pHandler->SetProgress(i+1);
    }
    PortalFile.CloseFile();
    return TRUE;
    x_catch_display;

    return FALSE;
}

//=========================================================================

void world_editor::LoadPortals( void )
{
    LoadPortals(xfs("%s\\Portals.ProjectInfo", g_Project.GetWorkingPath()));
}

//=========================================================================

void world_editor::LoadPortals( const char* pFileName )
{
    x_try;

    text_in PortalFile;
    PortalFile.OpenFile( pFileName );

    f32 fVersion = 0.0f;
    s32 nCurrent = 0;
    while (PortalFile.ReadHeader())
    {
        if( x_stricmp( PortalFile.GetHeaderName(), "PortalInfo" ) == 0 )
        {
            PortalFile.ReadFields();
            PortalFile.GetF32("FileVersion",fVersion);
        }
        else if( x_stricmp( PortalFile.GetHeaderName(), "Portals" ) == 0 )
        {
            s32 nCount = PortalFile.GetHeaderCount();
            if (m_pHandler) m_pHandler->SetProgressRange(0, nCount);

            for (s32 i =0; i < nCount; i++)
            {
                PortalFile.ReadFields();
                char cLayerPath[MAX_PATH];
                if (!PortalFile.GetString("LayerPath",cLayerPath)) 
                {
                    x_throw("error creating portal during load.");
                }

                guid GuidNewObj;
                if (PortalFile.GetGuid("Guid",GuidNewObj) && (GuidNewObj.Guid != 0)) 
                {
                    g_ObjMgr.CreateObject( zone_portal::GetObjectType(), GuidNewObj);
                }
                else
                {
                    x_throw("error creating portal during load.");
                }

                if (m_pHandler) m_pHandler->SetProgress(i);         
            }
        }
    }
    PortalFile.CloseFile();

    x_catch_display;
}

//=========================================================================

xbool world_editor::RenamePortal( guid ObjGuid, const char* pName )
{
    if (x_strlen(pName) < MAX_OBJECT_NAME_LENGTH)
    {
        object *pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);
        if (pObject)
        {
            transaction_object_data* pObjData = NULL;
            if (m_pCurrentUndoEntry)
            {
                //UNDO for object change before
                pObjData = new transaction_object_data(
                               transaction_object_data::OBJ_EDIT,
                               ObjGuid, pObject->GetTypeDesc().GetTypeName() );
                pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);
            }
  
            prop_query pqWrite;
            pqWrite.WQueryString( "Base\\Name", pName);
            pObject->OnProperty( pqWrite );

            if (m_pCurrentUndoEntry)
            {
                //UNDO for object change after
                pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                m_pCurrentUndoEntry->AddCommitStep(pObjData);
            }
            return TRUE;
        }
    }
    return FALSE;
}

//=========================================================================

void world_editor::RepathPortalChildren( const char* pOldName, const char* pNewName )
{
    x_try;
    editor_layer& Layer = GetLayerInfo(GetGlobalLayer());
    for (s32 iObject=0; iObject < Layer.Objects.GetCount(); iObject++)
    {
        editor_object_ref &ObjRef = Layer.Objects.GetAt(iObject);
        xstring xstrPath(ObjRef.LayerPath);
        ASSERT(xstrPath.GetAt(0) == '\\');
        xstring xstrSubPath = xstrPath.Right(xstrPath.GetLength()-1);
        s32 i = xstrSubPath.Find('\\',0);
        if (i != -1)
        {
            if (x_strcmp(pOldName, xstrSubPath.Left(i))==0)
            {
                //UNDO save old layer state 
                transaction_object_data* pObjData = NULL;
                if (m_pCurrentUndoEntry)
                {
                    object* pObject = g_ObjMgr.GetObjectByGuid(ObjRef.Guid);
                    if (pObject)
                    {
                        pObjData = new transaction_object_data(
                                       transaction_object_data::OBJ_LAYER_CHANGE,
                                       ObjRef.Guid, pObject->GetTypeDesc().GetTypeName() );
                        pObjData->StoreLayer(transaction_data::TRANSACTION_OLD_STATE);
                    }
                }

                //part of that portal
                ObjRef.LayerPath = xfs("\\%s\\",pNewName);

                //UNDO save new layer state
                if (pObjData)
                {
                    pObjData->StoreLayer(transaction_data::TRANSACTION_NEW_STATE);
                    m_pCurrentUndoEntry->AddCommitStep(pObjData);
                }           
            }
        }
    }
    for ( s32 iBlueprint=0; iBlueprint < Layer.Blueprints.GetCount(); iBlueprint++)
    {
        editor_blueprint_ref &BPRef = Layer.Blueprints.GetAt(iBlueprint);
        xstring xstrPath(BPRef.LayerPath);
        ASSERT(xstrPath.GetAt(0) == '\\');
        xstring xstrSubPath = xstrPath.Right(xstrPath.GetLength()-1);
        s32 i = xstrSubPath.Find('\\',0);
        if (i != -1)
        {
            if (x_strcmp(pOldName, xstrSubPath.Left(i))==0)
            {
                //UNDO save old layer state 
                transaction_bpref_data* pRefData = NULL;
                if (m_pCurrentUndoEntry)
                {
                    pRefData = new transaction_bpref_data(transaction_bpref_data::BPREF_EDIT,
                                                          BPRef.Guid);
                    pRefData->StoreReferenceData(transaction_data::TRANSACTION_OLD_STATE);
                }

                //part of that portal
                BPRef.LayerPath = xfs("\\%s\\",pNewName);

                //UNDO save new layer state
                if (pRefData)
                {
                    pRefData->StoreReferenceData(transaction_data::TRANSACTION_NEW_STATE);
                    m_pCurrentUndoEntry->AddCommitStep(pRefData);
                }
            }
        }
    }
    x_catch_display;
}

//=========================================================================

void world_editor::UpdateAllChildrenOfPortal( guid PortalGuid )
{
    //must search through global list for children of this portal
    x_try;
    editor_layer& Layer = GetLayerInfo(GetGlobalLayer());
    for (s32 i=0; i < Layer.Objects.GetCount(); i++)
    {
        editor_object_ref &ObjRef = Layer.Objects.GetAt(i);
        u8 Zone1 = 0;
        u8 Zone2 = 0;
        if (IsPartOfAPortal(ObjRef.LayerPath, Layer.Name, Zone1, Zone2))
        {
            SetObjectsZone(ObjRef.Guid, Zone1, Zone2);
        }
    }
    for ( i=0; i < Layer.Blueprints.GetCount(); i++)
    {
        editor_blueprint_ref &BPRef = Layer.Blueprints.GetAt(i);
        u8 Zone1 = 0;
        u8 Zone2 = 0;
        if (IsPartOfAPortal(BPRef.LayerPath, Layer.Name, Zone1, Zone2))
        {
            SetBlueprintObjectsZone(BPRef.Guid, Zone1, Zone2);
        }
    }
    
    x_catch_display;
}

//=========================================================================

void world_editor::UpdatePlayerZone( void )
{
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if( pPlayer != NULL )
    {
        pPlayer->InitZoneTracking();
    }   
}

//=========================================================================
// Object Selections
//=========================================================================

u32 world_editor::GetSelectedCount( )
{
    return m_guidsSelectedObjs.GetCount( );
}

//=========================================================================

xbool world_editor::IsSelectedObjectsEditable( void )
{
    u32 ObjCount = m_guidsSelectedObjs.GetCount();

    if(ObjCount)
    {
        for(u32 ii = 0; ii < ObjCount; ii++)
        {
            guid ObjGuid = g_WorldEditor.GetSelectedObjectsByIndex(ii);
            if(ObjGuid)
            {
                editor_layer& Layer = g_WorldEditor.FindObjectsLayer(ObjGuid,TRUE);
                if (!Layer.IsNull && !Layer.Name.IsEmpty())
                {
                    if(!Layer.IsEditable)
                        return FALSE;
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

//=========================================================================

xbool world_editor::CheckSelectedForDuplicates( void )
{
    for (s32 i = 0; i < m_guidsSelectedObjs.GetCount(); i++)
    {
        object *pObject = g_ObjMgr.GetObjectByGuid(m_guidsSelectedObjs.GetAt(i));
        if (pObject)
        {
            if (CheckForDuplicateObjects(pObject))
            {
                //found a dup
                return TRUE;
            }
        }
    }
    return FALSE;
}

//=========================================================================

void world_editor::GetSelectedList( xarray<guid>& lstObjects )
{
    lstObjects.Clear();
    for (s32 i = 0; i < m_guidsSelectedObjs.GetCount(); i++)
    {
        lstObjects.Append(m_guidsSelectedObjs.GetAt(i));
    }
}

//=========================================================================

guid world_editor::GetSelectedObjectsByIndex( s32 index )
{
    return m_guidsSelectedObjs[index];
}

//=========================================================================

guid world_editor::SelectObjectWithRay( const vector3& Start, const vector3& End, xbool bIncludeIcons )
{
    s32 i ;

    m_SelectedZone = 0;
    guid ObjGuid(0);

    x_try;

    g_CollisionMgr.EditorSelectRay( Start, End, bIncludeIcons );

    // Collision?
    if(g_CollisionMgr.m_nCollisions != 0)
    {
        // Get distance to first object intersection
        vector3 RayDir         = End-Start ;
        f32     RayLen         = RayDir.Length() ;
        f32     DistToFirstObj = RayLen * g_CollisionMgr.m_Collisions[0].T ;

        // Build list of very close objects that were hit
        m_RayCastOverlapGuids.Clear() ;
        for (i = 0 ; i < g_CollisionMgr.m_nCollisions ; i++)
        {
            // Get distance to intersection of hit object
            f32 DistToObj = RayLen  * g_CollisionMgr.m_Collisions[i].T ;
            
            // Add to list if intersection distance to this object is really close to the first object
            if (x_abs(DistToFirstObj - DistToObj) < 20.0f)
            {
                // Add to the list
                m_RayCastOverlapGuids.Append(g_CollisionMgr.m_Collisions[i].ObjectHitGuid) ;
            }
        }

        // Reset guid index if there's just one object in the list
        s32 ObjectHitCount = m_RayCastOverlapGuids.GetCount() ;
        if (ObjectHitCount == 1)
            m_iRayCastOverlapGuid = 0 ;

        // Lookup object guid to use
        ObjGuid = m_RayCastOverlapGuids[m_iRayCastOverlapGuid % ObjectHitCount]  ;
        
        // Goto next object for next time
        if (ObjectHitCount > 1)
        {
            m_iRayCastOverlapGuid++ ;
            m_iRayCastOverlapGuid %= ObjectHitCount ;
        }

        // Get object
        object *pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);
        if (pObject) 
        {
            // SB 10/27/03 - Added for path object so it can perform sub-selection
            if (pObject->GetTypeDesc().IsIconSelect() == FALSE)
                pObject->OnColNotify(*pObject) ;

            if (!(pObject->GetAttrBits() & object::ATTR_EDITOR_BLUE_PRINT))
            {
                //get stats for objects only
                rigid_inst* pRigidInst = GetRigidInstForObject(pObject);
                if (pRigidInst)
                {
                    rigid_geom* pGeom = pRigidInst->GetRigidGeom();
                    if (pGeom)
                    {
                        x_DebugMsg(xfs("RIGIDGEOM selected (%d Verts) (%d Polys) (%d Materials) (%d Textures) (%d Meshes) (%d SubMeshes)\n",
                            pGeom->GetNVerts(), pGeom->GetNFaces(),
                            pGeom->m_nMaterials, pGeom->m_nTextures,
                            pGeom->m_nMeshes, pGeom->m_nSubMeshes));
                    }
                }
            }

            //TODO SET UNDO for pObject

            //make sure this isn't already in list
            xbool bExists = FALSE;
            for (i=0; i < m_guidsSelectedObjs.GetCount(); i++)
            {
                if (m_guidsSelectedObjs.GetAt(i) == ObjGuid) 
                {
                    //in list already, hmmm, remove it
                    bExists = TRUE;
                    m_guidsSelectedObjs.Delete(i);
                    break;
                }
            }

            if (!bExists) 
            {
                if (pObject->GetAttrBits() & object::ATTR_EDITOR_BLUE_PRINT)
                {
                    //Add all blueprints items
                    editor_blueprint_ref BlueprintReference;
                    if (GetBlueprintRefContainingObject(ObjGuid, BlueprintReference))
                    {
                        SelectBlueprintObjects(BlueprintReference,TRUE);
                    }
                }
                else
                {
                    SelectObject(ObjGuid, FALSE);
                }
            }
            else
            {
                if (pObject->GetAttrBits() & object::ATTR_EDITOR_BLUE_PRINT)
                {
                    //remove all blueprints items
                    editor_blueprint_ref BlueprintReference;
                    if (GetBlueprintRefContainingObject(ObjGuid, BlueprintReference))
                    {
                        SelectBlueprintObjects(BlueprintReference,FALSE);
                    }
                }
                //clear selection flag
                pObject->SetAttrBits(pObject->GetAttrBits() & ~object::ATTR_EDITOR_SELECTED);
            }
        }
    }

    x_catch_display;

    return ObjGuid;
}

//=========================================================================

xbool world_editor::GetCollisionPointIgnoreTemp( const vector3& Start, const vector3& End, vector3& CollisionPt )
{
    xbool bCollide = FALSE;
    x_try;

    g_CollisionMgr.RaySetup(0, Start, End);
    g_CollisionMgr.SetMaxCollisions(MAX_COLLISION_MGR_COLLISIONS);
    g_CollisionMgr.CheckCollisions();

    for( s32 i=0; i < g_CollisionMgr.m_nCollisions; i++)
    {
        BOOL bIsTemp = FALSE;
        for (s32 j = 0; j < m_guidLstTempObject.GetCount(); j++)
        {
            if (g_CollisionMgr.m_Collisions[i].ObjectHitGuid == m_guidLstTempObject.GetAt(j))
            {
                bIsTemp= TRUE;
                break;
            }
        }

        //this is it
        if (!bIsTemp)
        {
            bCollide = TRUE;
            CollisionPt = g_CollisionMgr.m_Collisions[i].Point;
            break;
        }
    }

    x_catch_display;
    return bCollide;
}

//=========================================================================

xbool world_editor::GetCollisionPointIgnoreSel( const vector3& Start, const vector3& End, vector3& CollisionPt )
{
    xbool bCollide = FALSE;
    x_try;

    g_CollisionMgr.RaySetup(0, Start, End);
    g_CollisionMgr.SetMaxCollisions(MAX_COLLISION_MGR_COLLISIONS);
    g_CollisionMgr.CheckCollisions( );

    for( s32 i=0; i < g_CollisionMgr.m_nCollisions; i++)
    {
        BOOL bIsSel = FALSE;
        for (s32 j = 0; j < m_guidsSelectedObjs.GetCount(); j++)
        {
            if (g_CollisionMgr.m_Collisions[i].ObjectHitGuid == m_guidsSelectedObjs.GetAt(j))
            {
                bIsSel= TRUE;
                break;
            }
        }

        //this is it
        if (!bIsSel)
        {
            bCollide = TRUE;
            CollisionPt = g_CollisionMgr.m_Collisions[i].Point;
            break;
        }
    }

    x_catch_display;
    return bCollide;
}

//=========================================================================

void world_editor::ClearSelectedObjectList ()
{
    m_SelectedZone = 0;

    x_try;

    //TODO SET UNDO for all objects

    // Loop through all selected objects
    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        object *pObject = g_ObjMgr.GetObjectByGuid( m_guidsSelectedObjs.GetAt(i) );
        if( pObject && pObject->GetAttrBits() & object::ATTR_EDITOR_SELECTED )
        {
            //clear selection flag
            pObject->SetAttrBits(pObject->GetAttrBits() & ~object::ATTR_EDITOR_SELECTED);
        }
    }

    x_catch_display;

    //clear internal selection list
    m_guidsSelectedObjs.Clear();
}

//=========================================================================

void world_editor::SelectObjectsByAnimation(guid CheckGuid)
{

    object *pCheckObject;
    pCheckObject = g_ObjMgr.GetObjectByGuid(CheckGuid);

    if(!pCheckObject)
    {
        return; //error with ptr;
    }
    //check to see if this object has an animation 

    if(!pCheckObject->GetAnimGroupPtr())
    {
        ::AfxMessageBox(xfs("Object has no Animations."));
        return;
    }

    //Now, Select every object of the selected type in every active layer

    ClearSelectedObjectList();

    xarray<xstring> ListLayers;
    xarray<guid> ObjList;
    g_WorldEditor.GetLayerNames ( ListLayers );
    for (int j = 0; j < ListLayers.GetCount(); j++ )
    {
        if (g_WorldEditor.GetAllObjectsInLayer( ListLayers.GetAt(j), ObjList ))
        {
            for (int i = 0; i < ObjList.GetCount(); i++)
            {
                guid& ObjGuid = ObjList.GetAt(i);
                object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);

                if(pObject && pObject->GetAnimGroupPtr() && pObject->IsSelectable() 
                    && pObject->GetAttrBits() &~ object::ATTR_EDITOR_SELECTED)
                {
                    if(strcmp(pObject->GetAnimGroupName(), pCheckObject->GetAnimGroupName()) == 0)
                    {
                        SelectObject(ObjList.GetAt(i), FALSE);                   
                    }
                }
            }
        }
    }
}

//=========================================================================

void world_editor::SelectObjectsByRidgedGeom(guid CheckGuid)
{

    object *pCheckObject;

    pCheckObject = g_ObjMgr.GetObjectByGuid(CheckGuid);
    if(!pCheckObject)
    {
        return; //error with ptr;
    }
    //check to see if this object has an animation 

    if(!pCheckObject->GetGeomPtr())
    {
        ::AfxMessageBox(xfs("Object has no ridged Geom."));
        return;
    }

    //Now, Select every object with the proper Ridged

    ClearSelectedObjectList();

    xarray <xstring> ListLayers;
    xarray <guid> ObjList;
    g_WorldEditor.GetLayerNames ( ListLayers );

    for (int j = 0; j < ListLayers.GetCount(); j++ )
    {
        if (g_WorldEditor.GetAllObjectsInLayer( ListLayers.GetAt(j), ObjList ))
        {
            for (int i = 0; i < ObjList.GetCount(); i++)
            {
                guid& ObjGuid = ObjList.GetAt(i);
                object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);

                if(pObject && pObject->GetGeomPtr() && pObject->IsSelectable() 
                    && pObject->GetAttrBits() &~ object::ATTR_EDITOR_SELECTED)
                {
                    if(strcmp(pObject->GetGeomName(), pCheckObject->GetGeomName()) == 0)
                    {
                        SelectObject(ObjList.GetAt(i), FALSE);                   
                    }
                }
            }
        }
    }
}

//=========================================================================

void world_editor::ShowHideObjectsByRidgedGeom(guid CheckGuid, xbool ShowHide)
{
    object *pCheckObject;
    xarray <xstring> ListLayers;
    xarray <guid> ObjList;

    pCheckObject = g_ObjMgr.GetObjectByGuid(CheckGuid);
    if(!pCheckObject)
    {
        return; //error with ptr;
    }
    //Now, Select every object with the proper Ridged

    ClearSelectedObjectList();

    g_WorldEditor.GetLayerNames ( ListLayers );

    for (int j = 0; j < ListLayers.GetCount(); j++ )
    {
        if (g_WorldEditor.GetAllObjectsInLayer( ListLayers.GetAt(j), ObjList ))
        {
            for (int i = 0; i < ObjList.GetCount(); i++)
            {
                guid& ObjGuid = ObjList.GetAt(i);
                object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);

                if(pObject && pObject->GetGeomPtr())
                {
                    if(strcmp(pObject->GetGeomName(), pCheckObject->GetGeomName()) == 0)
                    {
                        pObject->SetHidden(ShowHide);                   
                    }
                }
            }
        }
    }
}

//=========================================================================

void world_editor::ShowHideObjectsByAnimation(guid CheckGuid, xbool ShowHide)
{

    object *pCheckObject;
    pCheckObject = g_ObjMgr.GetObjectByGuid(CheckGuid);

    if(!pCheckObject)
    {
        return; //error with ptr;
    }

    //Now, Select every object of the selected type in every active layer

    ClearSelectedObjectList();

    xarray<xstring> ListLayers;
    xarray<guid> ObjList;
    g_WorldEditor.GetLayerNames ( ListLayers );
    for (int j = 0; j < ListLayers.GetCount(); j++ )
    {
        if (g_WorldEditor.GetAllObjectsInLayer( ListLayers.GetAt(j), ObjList ))
        {
            for (int i = 0; i < ObjList.GetCount(); i++)
            {
                guid& ObjGuid = ObjList.GetAt(i);
                object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);

                if(pObject && pObject->GetAnimGroupPtr())
                {
                    if(strcmp(pObject->GetAnimGroupName(), pCheckObject->GetAnimGroupName()) == 0)
                    {
                        pObject->SetHidden(ShowHide);                   
                    }
                }
            }
        }
    }
}


//=========================================================================

void world_editor::ThawFreezeObjectsByRidgedGeom(guid CheckGuid, xbool ShowHide)
{
    object *pCheckObject;
    xarray <xstring> ListLayers;
    xarray <guid> ObjList;

    pCheckObject = g_ObjMgr.GetObjectByGuid(CheckGuid);
    if(!pCheckObject)
    {
        return; //error with ptr;
    }
    //Now, Select every object with the proper Ridged

    ClearSelectedObjectList();

    g_WorldEditor.GetLayerNames ( ListLayers );

    for (int j = 0; j < ListLayers.GetCount(); j++ )
    {
        if (g_WorldEditor.GetAllObjectsInLayer( ListLayers.GetAt(j), ObjList ))
        {
            for (int i = 0; i < ObjList.GetCount(); i++)
            {
                guid& ObjGuid = ObjList.GetAt(i);
                object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);

                if(pObject && pObject->GetGeomPtr())
                {
                    if(strcmp(pObject->GetGeomName(), pCheckObject->GetGeomName()) == 0)
                    {
                        pObject->SetSelectable(ShowHide);                   
                    }
                }
            }
        }
    }
}

//=========================================================================

void world_editor::ThawFreezeObjectsByAnimation(guid CheckGuid, xbool ShowHide)
{

    object *pCheckObject;
    pCheckObject = g_ObjMgr.GetObjectByGuid(CheckGuid);

    if(!pCheckObject)
    {
        return; //error with ptr;
    }

    //Now, Select every object of the selected type in every active layer

    ClearSelectedObjectList();

    xarray<xstring> ListLayers;
    xarray<guid> ObjList;
    g_WorldEditor.GetLayerNames ( ListLayers );
    for (int j = 0; j < ListLayers.GetCount(); j++ )
    {
        if (g_WorldEditor.GetAllObjectsInLayer( ListLayers.GetAt(j), ObjList ))
        {
            for (int i = 0; i < ObjList.GetCount(); i++)
            {
                guid& ObjGuid = ObjList.GetAt(i);
                object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);

                if(pObject && pObject->GetAnimGroupPtr())
                {
                    if(strcmp(pObject->GetAnimGroupName(), pCheckObject->GetAnimGroupName()) == 0)
                    {
                        pObject->SetSelectable(ShowHide);                   
                    }
                }
            }
        }
    }
}

//=========================================================================

void world_editor::SelectObjectsByType(guid CheckGuid)
{
    object *pCheckObject;

    pCheckObject = g_ObjMgr.GetObjectByGuid(CheckGuid);
    if(!pCheckObject)
    {
        return;
    }

    //Now, Select every object of the selected type in every active layer

    ClearSelectedObjectList();

    xarray<xstring> ListLayers;
    xarray<guid> ObjList;
    g_WorldEditor.GetLayerNames ( ListLayers );

    for (int j=0; j < ListLayers.GetCount(); j++)
    {
        if (g_WorldEditor.GetAllObjectsInLayer( ListLayers.GetAt(j), ObjList ))
        {
            for (int i=0; i<ObjList.GetCount(); i++)
            {
                guid& ObjGuid = ObjList.GetAt(i);
                object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid) ;
                if(pObject && pObject->IsSelectable() && 
                    pObject->GetType() == pCheckObject->GetType()  
                    && pObject->GetAttrBits() &~ object::ATTR_EDITOR_SELECTED)
                    SelectObject(ObjGuid, FALSE);                   
            }
        }
    }
}

//=========================================================================

void world_editor::SelectObjectsByLayer(const char * pCheckLayer)
{
    //Now, Select every object of the selected type in every active layer

    ClearSelectedObjectList ();

    xarray <xstring> ListLayers;
    xarray <guid> ObjList;
    g_WorldEditor.GetLayerNames ( ListLayers );

    for (int j = 0; j < ListLayers.GetCount(); j++)
    {
        if( strcmp(ListLayers.GetAt( j ),pCheckLayer) == 0)
        {
            if (g_WorldEditor.GetAllObjectsInLayer( ListLayers.GetAt( j ), ObjList ))
            {
                for (int i = 0; i < ObjList.GetCount(); i++)
                {
                    guid& ObjGuid = ObjList.GetAt( i );
                    object* pObject = g_ObjMgr.GetObjectByGuid( ObjGuid ) ;
                    if( pObject && pObject->IsSelectable() && pObject->GetAttrBits() &~ object::ATTR_EDITOR_SELECTED)
                        SelectObject( ObjList.GetAt( i ), FALSE );                   
                }
            }
        }
    }
}

//=========================================================================

void world_editor::SelectObjectsByFolder( xarray <guid> &ObjList )
{
    //Now, Select every object in the List layer

    ClearSelectedObjectList ();

    for (int i = 0; i < ObjList.GetCount(); i++)
    {
        guid& ObjGuid = ObjList.GetAt( i );
        object* pObject = g_ObjMgr.GetObjectByGuid( ObjGuid ) ;
        if(pObject)
        {            
            if(pObject->IsSelectable() && pObject->GetAttrBits() &~ object::ATTR_EDITOR_SELECTED &&
                pObject->GetType() != object::TYPE_ZONE_PORTAL)
                SelectObject( ObjList.GetAt( i ), FALSE );                  
        }
    }
}

//=========================================================================

void world_editor::SelectObjectsAll  ( void )
{
    xarray <xstring> ListLayers;

    ClearSelectedObjectList ();

    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
    {
        editor_layer& Layer = m_listLayers.GetAt(i);

        //select all blueprints in layer
        for (s32 k = 0; k < Layer.Blueprints.GetCount(); k++)
        {
            editor_blueprint_ref& BlueprintRef = Layer.Blueprints.GetAt(k);
            g_WorldEditor.SelectBlueprintObjects(BlueprintRef,TRUE);
        }                
        for (s32 j = 0; j < Layer.Objects.GetCount(); j++)
        {
            editor_object_ref& ObjRef = Layer.Objects.GetAt(j);
            object* pObject = g_ObjMgr.GetObjectByGuid( ObjRef.Guid);

            if(pObject && pObject->GetAttrBits() &~ object::ATTR_EDITOR_SELECTED )
            { 
                SelectObject( ObjRef.Guid, FALSE, TRUE );
            }
        }
    }
}

//=========================================================================

void world_editor::MakeSelectUnSelectObjectsByFolder( xarray <guid> &ObjList, xbool SelectUnSelect )
{
    //Now, Select every object of the selected type in every active layer

    ClearSelectedObjectList ();

    for (int i = 0; i < ObjList.GetCount(); i++)
    {
        guid& ObjGuid = ObjList.GetAt( i );
        object* pObject = g_ObjMgr.GetObjectByGuid( ObjGuid ) ;
        if(pObject && pObject->GetType() != object::TYPE_ZONE_PORTAL)
        {            
            pObject->SetSelectable(SelectUnSelect); 
        }
    }
}

//=========================================================================

void world_editor::MakeSelectUnSelectAllObjects(xbool SelectUnSelect)
{
    //Now, Select every object of the selected type in every active layer
    ClearSelectedObjectList ();

    xarray <xstring> ListLayers;
    xarray <guid> ObjList;
    g_WorldEditor.GetLayerNames ( ListLayers );

    for (int j = 0; j < ListLayers.GetCount(); j++)
    {
        if (g_WorldEditor.GetAllObjectsInLayer( ListLayers.GetAt( j ), ObjList ))
        {
            for (int i = 0; i < ObjList.GetCount(); i++)
            {
                guid& ObjGuid = ObjList.GetAt( i );
                object* pObject = g_ObjMgr.GetObjectByGuid( ObjGuid ) ;
                if( pObject)
                    pObject->SetSelectable(SelectUnSelect ); 
            }
        }
    }
}

//=========================================================================

void world_editor::MakeSelectUnSelectObjectsByLayer (const char * pCheckLayer, xbool SelectUnSelect)
{
    //Now, Select every object of the selected type in every active layer

    ClearSelectedObjectList ();

    xarray <xstring> ListLayers;
    xarray <guid> ObjList;
    g_WorldEditor.GetLayerNames ( ListLayers );

    for (int j = 0; j < ListLayers.GetCount(); j++)
    {
        if( strcmp(ListLayers.GetAt( j ),pCheckLayer) == 0)
        {
            if (g_WorldEditor.GetAllObjectsInLayer( ListLayers.GetAt( j ), ObjList ))
            {
                for (int i = 0; i < ObjList.GetCount(); i++)
                {
                    guid& ObjGuid = ObjList.GetAt( i );
                    object* pObject = g_ObjMgr.GetObjectByGuid( ObjGuid ) ;
                    if( pObject)
                        pObject->SetSelectable(SelectUnSelect);        
                }
            }
        }
    }
}

//=========================================================================

void world_editor::MakeHiddenUnHiddenAllObjects(xbool HiddenUnHidden)
{
    //Now, Select every object of the selected type in every active layer

    ClearSelectedObjectList ();
    xarray <xstring> ListLayers;
    xarray <guid> ObjList;
    g_WorldEditor.GetLayerNames ( ListLayers );

    for (int j = 0; j < ListLayers.GetCount(); j++)
    {
        if (g_WorldEditor.GetAllObjectsInLayer( ListLayers.GetAt( j ), ObjList ))
        {
            for (int i = 0; i < ObjList.GetCount(); i++)
            {
                guid& ObjGuid = ObjList.GetAt( i );
                object* pObject = g_ObjMgr.GetObjectByGuid( ObjGuid );
                if( pObject)
                    pObject->SetHidden(HiddenUnHidden ); 
            }
        }
    }
}

//=========================================================================

void world_editor::MakeHiddenUnHiddenObjectsByFolder( xarray <guid> &ObjList, xbool HiddenUnHidden )
{
    //Now, Select every object of the selected type in every active layer

    ClearSelectedObjectList ();

    for (int i = 0; i < ObjList.GetCount(); i++)
    {
        guid& ObjGuid = ObjList.GetAt( i );
        object* pObject = g_ObjMgr.GetObjectByGuid( ObjGuid ) ;
        if(pObject && pObject->GetType() != object::TYPE_ZONE_PORTAL)
        {            
            pObject->SetHidden( HiddenUnHidden ); 
        }
    }
}

//=========================================================================

void world_editor::MakeHiddenUnHiddenObjectsByLayer(const char * pCheckLayer, xbool HiddenUnHidden)
{
    //Now, Select every object of the selected type in every active layer

    ClearSelectedObjectList ();

    xarray <xstring> ListLayers;
    xarray <guid> ObjList;
    g_WorldEditor.GetLayerNames ( ListLayers );

    for (int j = 0; j < ListLayers.GetCount(); j++)
    {
        if( strcmp(ListLayers.GetAt( j ),pCheckLayer) == 0)
        {
            if (g_WorldEditor.GetAllObjectsInLayer( ListLayers.GetAt( j ), ObjList ))
            {
                for (int i = 0; i < ObjList.GetCount(); i++)
                {
                    guid& ObjGuid = ObjList.GetAt( i );
                    object* pObject = g_ObjMgr.GetObjectByGuid( ObjGuid ) ;
                    if( pObject)
                        pObject->SetHidden(HiddenUnHidden);        
                }
            }
        }
    }

}

//=========================================================================

xbool world_editor::MoveSelectedObjectsToActiveLayer( xarray<editor_item_descript>& lstItems )
{
    return MoveSelectedObjectsToLayer(GetActiveLayer(), GetActiveLayerPath(), lstItems);
}

//=========================================================================

xbool world_editor::MoveSelectedObjectsToLayer( const char* pLayer, const char* pLayerPath, xarray<editor_item_descript>& lstItems )
{
    x_try;
    lstItems.Clear();

    xarray<editor_blueprint_ref> lstBlueprints;
    // Loop through all objects
    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        object *pObject = g_ObjMgr.GetObjectByGuid( m_guidsSelectedObjs.GetAt(i) );
        if( pObject && (pObject->GetAttrBits() & object::ATTR_EDITOR_SELECTED) )
        {
            guid ObjGuid = pObject->GetGuid();

            if (pObject->GetType() == object::TYPE_ZONE_PORTAL)
            {
                //don't move this
            }
            else if (pObject->GetAttrBits() & object::ATTR_EDITOR_BLUE_PRINT)
            {     
                //object was a blueprint, add it to the list
                editor_blueprint_ref BPRef;
                if (GetBlueprintRefContainingObject ( ObjGuid, BPRef ) )
                {
                    BOOL bExists = FALSE;
                    for( s32 k=0; k < lstBlueprints.GetCount(); k++ )
                    {
                        editor_blueprint_ref& BPRefInList = lstBlueprints.GetAt(k);
                        if (BPRefInList.Guid == BPRef.Guid)
                        {
                            //already in list
                            bExists =TRUE;
                            break;
                        }
                    }

                    if (!bExists) //append this bp if it is not already in list
                        lstBlueprints.Append(BPRef);
                }                  
            }
            else
            {
                //remove from layer
                editor_layer& Layer = FindObjectsLayer(ObjGuid,TRUE);
                if (!Layer.IsNull)
                {
                    editor_item_descript Description;
                    Description.Layer = Layer.Name;
                    Description.Guid = ObjGuid;
                    Description.IsInBlueprint = FALSE;
                    Description.LayerPath = FindLayerPathForObject(ObjGuid, Layer);
                    lstItems.Append(Description);

                    //UNDO save old layer state 
                    transaction_object_data* pObjData = NULL;
                    if (m_pCurrentUndoEntry)
                    {
                        pObjData = new transaction_object_data(
                                       transaction_object_data::OBJ_LAYER_CHANGE,
                                       ObjGuid, pObject->GetTypeDesc().GetTypeName() );
                        pObjData->StoreLayer(transaction_data::TRANSACTION_OLD_STATE);
                    }

                    RemoveObjectFromLayer(ObjGuid, Layer.Name);

                    editor_object_ref ObjRef;
                    ObjRef.Guid = ObjGuid;
                    ObjRef.LayerPath = pLayerPath;
                    AddObjectToLayer(ObjRef, pLayer, TRUE);

                    //UNDO save new layer state
                    if (pObjData)
                    {
                        pObjData->StoreLayer(transaction_data::TRANSACTION_NEW_STATE);
                        m_pCurrentUndoEntry->AddCommitStep(pObjData);
                    }
                }
            }
        }
    }

    //update all affected blueprints
    for( s32 j=0; j < lstBlueprints.GetCount(); j++ )
    {       
        editor_blueprint_ref BPRef = lstBlueprints.GetAt(j);

        editor_item_descript Description;
        Description.Guid= BPRef.Guid;
        Description.IsInBlueprint = TRUE;

        editor_layer& Layer = FindBlueprintsLayer( BPRef.Guid );
        if (!Layer.IsNull)
        {
            Description.Layer = Layer.Name;
            BPRef.LayerPath = pLayerPath; //update the layer path hierarchy
            Description.LayerPath = BPRef.LayerPath;
            if (Description.Guid.Guid != 0)
            {
                lstItems.Append(Description);

                //UNDO save old layer state 
                transaction_bpref_data* pRefData = NULL;
                if (m_pCurrentUndoEntry)
                {
                    pRefData = new transaction_bpref_data(transaction_bpref_data::BPREF_EDIT,
                                                          BPRef.Guid);
                    pRefData->StoreReferenceData(transaction_data::TRANSACTION_OLD_STATE);
                }

                RemoveBlueprintFromLayer(BPRef.Guid, Layer.Name, TRUE);
                AddBlueprintToLayer(BPRef, pLayer, TRUE);

                //UNDO save new layer state
                if (pRefData)
                {
                    pRefData->StoreReferenceData(transaction_data::TRANSACTION_NEW_STATE);
                    m_pCurrentUndoEntry->AddCommitStep(pRefData);
                }
            }
        }
    }

    x_catch_display;

    return TRUE;
}

//=========================================================================

void world_editor::DeleteSelectedObjects( xarray<editor_item_descript>& lstItems )
{
    x_try;
    lstItems.Clear();

    // Loop through all objects
    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        object *pObject = g_ObjMgr.GetObjectByGuid( m_guidsSelectedObjs.GetAt(i) );
        if( pObject && pObject->GetAttrBits() & object::ATTR_EDITOR_SELECTED )
        {
            if (pObject->GetTypeDesc().NoAllowEditorCopy())
            {
                //can't delete these this way (ex: TYPE_ZONE_PORTAL)
                x_DebugMsg("You've attempted to delete a protected object.");
            }
            else
            {
                guid ObjGuid = pObject->GetGuid();

                //set the UNDO for this object
                if (m_pCurrentUndoEntry)
                {
                    //delete objects
                    transaction_object_data* pObjData = new transaction_object_data(
                                                        transaction_object_data::OBJ_DELETE,
                                                        ObjGuid, pObject->GetTypeDesc().GetTypeName() );
                    pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);
                    m_pCurrentUndoEntry->AddCommitStep(pObjData);
                }

                //remove from layer
                editor_layer& Layer = FindObjectsLayer(ObjGuid,TRUE);
                if (!Layer.IsNull)
                {
                    editor_item_descript Description;
                    Description.Layer = Layer.Name;
                    Description.IsInBlueprint = (pObject->GetAttrBits() & object::ATTR_EDITOR_BLUE_PRINT);
                    if (Description.IsInBlueprint)
                    {                    
                        editor_blueprint_ref BPDelRef;
                        if (GetBlueprintRefContainingObject(ObjGuid, BPDelRef))
                        {
                            Description.Guid = BPDelRef.Guid;
                            Description.LayerPath = BPDelRef.LayerPath;
                            //set the UNDO for this blueprint
                            if (m_pCurrentUndoEntry)
                            {
                                transaction_bpref_data* pRefData = new transaction_bpref_data(
                                                                    transaction_bpref_data::BPREF_DELETE,
                                                                    Description.Guid);
                                pRefData->StoreReferenceData(transaction_data::TRANSACTION_OLD_STATE);
                                m_pCurrentUndoEntry->AddCommitStep(pRefData);
                            }

                            lstItems.Append(Description);
                            RemoveBlueprintFromLayer(Description.Guid,Layer.Name, TRUE);
                        }
                        else
                        {
                            //must have already been removed
                        }
                    }
                    else
                    {
                        //set the UNDO for this object's layer
                        if (m_pCurrentUndoEntry)
                        {
                            //remove it from layer
                            transaction_object_data* pObjData = new transaction_object_data(
                                           transaction_object_data::OBJ_LAYER_CHANGE,
                                           ObjGuid, pObject->GetTypeDesc().GetTypeName() );
                            pObjData->StoreLayer(transaction_data::TRANSACTION_OLD_STATE);
                            m_pCurrentUndoEntry->AddCommitStep(pObjData);
                        }

                        Description.Guid = ObjGuid;
                        lstItems.Append(Description);
                        RemoveObjectFromLayer(ObjGuid,Layer.Name);
                    }
                }

                //delete selected object
                DestroyObjectEx( ObjGuid, TRUE );
            }
        }
    }

    x_catch_display;

    //clear internal list
    m_guidsSelectedObjs.Clear();
}

//=========================================================================

void world_editor::RotateSelectedObjects( const radian3& r )
{
    matrix4 Transform;
    Transform.Identity();
    Transform.Rotate(r);

    RotateSelectedObjects(Transform);

    // Loop through all objects
    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        guid& ObjectGuid = m_guidsSelectedObjs.GetAt(i);
        SetObjectsLayerAsDirty(ObjectGuid);
    }
}

//=========================================================================

void world_editor::RotateSelectedObjects( const matrix4& Transform )
{
    bbox Bounds = GetSelectedObjectsBounds( );
    vector3 ptCenter = Bounds.GetCenter();
    vector3 ptInvereted = Bounds.GetCenter();
    ptInvereted.Negate();
    radian3 r = Transform.GetRotation();

    xarray<editor_blueprint_ref> lstBlueprints;
    // Loop through all objects
    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        guid& ObjectGuid = m_guidsSelectedObjs.GetAt(i);
        object *pObject = g_ObjMgr.GetObjectByGuid( ObjectGuid );
        if( pObject )
        {
            //UNDO STUFF
            transaction_object_data* pObjData = NULL;
            if (m_pCurrentUndoEntry)
            {
                //UNDO for object change before
                pObjData = new transaction_object_data(
                               transaction_object_data::OBJ_EDIT,
                               ObjectGuid,pObject->GetTypeDesc().GetTypeName() );
                pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);
            }

            matrix4 L2W;

            //compute Matrix
            L2W = pObject->GetL2W();
            //first we need to translate the each object to the origin
            L2W.Translate(ptInvereted);
            //now we transform
            L2W.Rotate(r);
            //now we translate back
            L2W.Translate(ptCenter);

            pObject->OnTransform(L2W);   

            if (m_pCurrentUndoEntry)
            {
                //UNDO for object change after
                pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                m_pCurrentUndoEntry->AddCommitStep(pObjData);
            }

            if (pObject->GetAttrBits() & object::ATTR_EDITOR_BLUE_PRINT)
            {
                //object was a blueprint, set its rotation
                editor_blueprint_ref BPRef;

                if (GetBlueprintRefContainingObject ( ObjectGuid, BPRef ) )
                {
                    BOOL bExists = FALSE;
                    for( s32 k=0; k < lstBlueprints.GetCount(); k++ )
                    {
                        editor_blueprint_ref& BPRefInList = lstBlueprints.GetAt(k);
                        if (BPRefInList.Guid == BPRef.Guid)
                        {
                            //already in list
                            bExists =TRUE;
                            break;
                        }
                    }

                    if (!bExists) //append this bp if it is not already in list
                        lstBlueprints.Append(BPRef);
                }
                else ASSERT(FALSE);
            }
        }
    }

    //update all affected blueprints
    for( s32 j=0; j < lstBlueprints.GetCount(); j++ )
    {       
        editor_blueprint_ref BPRef = lstBlueprints.GetAt(j);

        editor_layer& Layer = FindBlueprintsLayer( BPRef.Guid );
        if (!Layer.IsNull)
        {
            //UNDO for blueprints info (store old state)
            transaction_bpref_data* pRefData = NULL;
            if (m_pCurrentUndoEntry)
            {
                pRefData = new transaction_bpref_data(
                                                    transaction_bpref_data::BPREF_EDIT,
                                                    BPRef.Guid);
                pRefData->StoreReferenceData(transaction_data::TRANSACTION_OLD_STATE);
            }

            RemoveBlueprintFromLayer(BPRef.Guid, Layer.Name, FALSE);
            BPRef.Transform.Rotate(r);
            AddBlueprintToLayer( BPRef, Layer.Name, FALSE );

            if (pRefData)
            {
                //UNDO store new state
                pRefData->StoreReferenceData(transaction_data::TRANSACTION_NEW_STATE);
                //UNDO commit
                m_pCurrentUndoEntry->AddCommitStep(pRefData);
            }
        }
    }
}

//=========================================================================

vector3 world_editor::GetMinPositionForSelected( )
{
    vector3 minPos(0.0,0.0,0.0);
    BOOL bInit = FALSE;

    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        //Get Min object position to snap to
        object *pObject = g_ObjMgr.GetObjectByGuid( m_guidsSelectedObjs.GetAt(i) );
        if( pObject )
        {
            if (!bInit)
            {
                bInit = TRUE;
                minPos = pObject->GetPosition();
            }
            else
            {
                //check for min object
                vector3 objPos = pObject->GetPosition();
                if (objPos.GetY() < minPos.GetY())
                {
                    minPos = objPos;
                }
                else if (objPos.GetY() == minPos.GetY())
                {
                    if (objPos.GetX() < minPos.GetX())
                    {
                        minPos = objPos;
                    }
                    else if (objPos.GetX() == minPos.GetX())
                    {
                        if (objPos.GetZ() < minPos.GetZ())
                        {
                            minPos = objPos;
                        }
                    }
                }
            }
        }
    }
    return minPos;
}

//=========================================================================

void world_editor::MoveSelectedObjects( vector3& pos )
{
    x_try;

    vector3 minPos = GetMinPositionForSelected( );

    //ok, we have a position, lets place it on top of that position, rather than centered
    vector3 delta(pos - minPos);

    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        guid ObjGuid = m_guidsSelectedObjs.GetAt(i);
        object *pObject = g_ObjMgr.GetObjectByGuid( ObjGuid );
        if( pObject )
        {
            pObject->OnMoveRel(delta);
            SetObjectsLayerAsDirty(ObjGuid);
        }
    }
    x_catch_display;
}
    
//=========================================================================

void world_editor::CopySelectedObjects( xarray<guid>& lstObjects, xarray<guid>& lstBPRefs, xbool bKeepCurrentSelection )
{
    x_try;

    xarray<guid> OldSelection = m_guidsSelectedObjs;
    xarray<guid> NewSelection;
    lstObjects.Clear();
    ClearSelectedObjectList();

    xarray<guid> lstAnchors;

    //copy objects
    for( s32 i=0; i < OldSelection.GetCount(); i++ )
    {
        guid ObjGuid = OldSelection.GetAt(i);
        object *pOriginalObject = g_ObjMgr.GetObjectByGuid( ObjGuid );
        if (pOriginalObject)
        {
            if (pOriginalObject->GetAttrBits() & object::ATTR_EDITOR_BLUE_PRINT)
            {
                //this is part of a blueprint don't copy
                if (pOriginalObject->GetType() == object::TYPE_EDITOR_BLUEPRINT_ANCHOR)
                {
                    //hmmm, need to save a reference to this, so we can rebuild later
                    lstAnchors.Append(ObjGuid);
                }
            }
            else
            {    
                //copy the object
                guid CopyGuid = InternalObjectCopy(ObjGuid);
                if (CopyGuid.Guid != 0)
                {
                    object *pObject = g_ObjMgr.GetObjectByGuid( CopyGuid );
                    if( pObject )
                    {
                        //add object to active layer
                        lstObjects.Append(CopyGuid);
                        NewSelection.Append(CopyGuid);
                        AddObjectToActiveLayer(CopyGuid);

                        //now setup UNDO for each object
                        if (m_pCurrentUndoEntry)
                        {
                            transaction_object_data* pObjData = new transaction_object_data(
                                                                transaction_object_data::OBJ_CREATE,
                                                                CopyGuid,pObject->GetTypeDesc().GetTypeName() );
                            pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                            m_pCurrentUndoEntry->AddCommitStep(pObjData);

                            pObjData = new transaction_object_data(
                                           transaction_object_data::OBJ_LAYER_CHANGE,
                                           CopyGuid,pObject->GetTypeDesc().GetTypeName() );
                            pObjData->StoreLayer(transaction_data::TRANSACTION_NEW_STATE);
                            m_pCurrentUndoEntry->AddCommitStep(pObjData);
                        }
                    }
                }
            }
        }
    }

    //must clear to manipulate blueprints correctly
    ClearSelectedObjectList();

    for ( i = 0; i < lstAnchors.GetCount(); i++)
    {
        //now use each anchor to copy the blueprints
        guid& AnchorGuid = lstAnchors.GetAt(i);
        object *pObjectAnchor = g_ObjMgr.GetObjectByGuid( AnchorGuid );
        if (pObjectAnchor)
        {
            editor_blueprint_ref BlueprintReference;
            if (GetBlueprintRefContainingAnchor(AnchorGuid, BlueprintReference))
            {
                //found it, now copy it
                editor_blueprint_ref BPNew;
                //the following functions handle their individual undos 
                AddBlueprint(BlueprintReference.ThemeName, BlueprintReference.RelativePath, BPNew, TRUE, TRUE);
                RotateSelectedObjects( BlueprintReference.Transform );
                MoveBlueprintObjectsToAnchor( pObjectAnchor->GetPosition() );

                lstBPRefs.Append(BPNew.Guid);

                //now add these object to new selection list
                for( s32 j=0; j < m_guidsSelectedObjs.GetCount(); j++ )
                {
                    NewSelection.Append(m_guidsSelectedObjs.GetAt(j)); 
                }
            }
            else
            {
                generic_dialog Dialog;
                Dialog.Execute_OK( "Blueprint not found",
                                   xfs( "Can't locate the blueprint for anchor %08x:%08x.", AnchorGuid.GetHigh(), AnchorGuid.GetLow() ) );
            }
        }
    }

    //build new selection list
    ClearSelectedObjectList();
    if (bKeepCurrentSelection)
    {
        for ( i = 0; i < OldSelection.GetCount(); i++)
        {
            SelectObject(OldSelection.GetAt(i),FALSE);
        }
    }
    else
    {
        for ( i = 0; i < NewSelection.GetCount(); i++)
        {
            SelectObject(NewSelection.GetAt(i),FALSE);
        }
    }

    x_catch_display;
}
    
//=========================================================================

void world_editor::MoveSelectedObjectsToCenterPt( vector3& pos )
{
    x_try;

    //ok, we have a position, lets place it on top of that position, rather than centered
    bbox Bounds = GetSelectedObjectsBounds( );
    vector3 posOld = Bounds.GetCenter();
    //pos.Y += posOld.Y - Bounds.Min.Y;
    //vector3 delta(pos - Bounds.GetCenter());
    vector3 delta(pos - posOld);

    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        guid ObjGuid = m_guidsSelectedObjs.GetAt(i);
        object *pObject = g_ObjMgr.GetObjectByGuid( ObjGuid );
        if( pObject )
        {
            transaction_object_data* pObjData = NULL;
            if (m_pCurrentUndoEntry)
            {
                //UNDO for object change before
                pObjData = new transaction_object_data(
                               transaction_object_data::OBJ_EDIT,
                               ObjGuid,pObject->GetTypeDesc().GetTypeName() );
                pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);
            }

            pObject->OnMoveRel(delta);

            if (m_pCurrentUndoEntry)
            {
                //UNDO for object change after
                pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                m_pCurrentUndoEntry->AddCommitStep(pObjData);
            }

            SetObjectsLayerAsDirty(ObjGuid);
        }
    }

    x_catch_display;
}

//=========================================================================

void world_editor::MoveBlueprintObjectsToAnchor( const vector3& Pos )
{
    x_try;

    bbox Bounds = GetSelectedObjectsBounds( );
    vector3 Anchor = Bounds.GetCenter();

    // Loop through all objects and find the anchor
    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        object *pObject = g_ObjMgr.GetObjectByGuid( m_guidsSelectedObjs.GetAt(i) );
        if( pObject && (pObject->GetType() == object::TYPE_EDITOR_BLUEPRINT_ANCHOR))
        {
            //got the anchor, save as origin
            Anchor = pObject->GetPosition();
            break;
        }
    }

    vector3 delta(Pos - Anchor);

    for( s32 j=0; j < m_guidsSelectedObjs.GetCount(); j++ )
    {       
        guid& ObjectGuid = m_guidsSelectedObjs.GetAt(j);
        object *pObject = g_ObjMgr.GetObjectByGuid( ObjectGuid );
        if( pObject )
        {
            transaction_object_data* pObjData = NULL;
            if (m_pCurrentUndoEntry)
            {
                //UNDO for object change before
                pObjData = new transaction_object_data(
                               transaction_object_data::OBJ_EDIT,
                               ObjectGuid, pObject->GetTypeDesc().GetTypeName() );
                pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);
            }

            pObject->OnMoveRel(delta);

            if (pObjData)
            {
                pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                m_pCurrentUndoEntry->AddCommitStep(pObjData);
            }
        }
    }
    x_catch_display;
}

//=========================================================================

bbox world_editor::GetSelectedObjectsBounds( )
{
    bbox Bounds;
    x_try;
    BOOL bInit = FALSE;
    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        object *pObject = g_ObjMgr.GetObjectByGuid( m_guidsSelectedObjs.GetAt(i) );
        if( pObject )
        {
            if (!bInit)
            {
                Bounds = pObject->GetBBox();
                bInit = TRUE;
            }
            else
            {
                bbox objBounds = pObject->GetBBox();
                Bounds.AddVerts(&(objBounds.Min),1);
                Bounds.AddVerts(&(objBounds.Max),1);
            }
        }
    }

    x_catch_display;
    return Bounds;
}

//=========================================================================
// UNDO REDO
//=========================================================================

void world_editor::ClearUndoList( void )
{
    if (m_pUndoSelectData)
    {
        //delete all data
        DATAPOS pos = m_pUndoSelectData->GetHeadPosition();
        while (pos)
        {
            transaction_data* pData = (transaction_data*)m_pUndoSelectData->GetAt(pos);
            if (pData)
            {
                delete pData;
            }
            m_pUndoSelectData->GetNext(pos);
        }

        //clear ptrs
        m_pUndoSelectData->RemoveAll();

        delete m_pUndoSelectData;
        m_pUndoSelectData = NULL;
    }
}

//=========================================================================

void world_editor::ClearUndoEntry( void )
{
    if (m_pCurrentUndoEntry)
    {
        delete m_pCurrentUndoEntry;
        m_pCurrentUndoEntry = NULL;
    }
}

//=========================================================================

void world_editor::UndoBeginSelObjectsPropsChange( )
{
    ASSERT(m_pUndoSelectData == NULL);

    if (m_pUndoSelectData)
    {
        ClearUndoList( );
    }

    //create new data list
    m_pUndoSelectData = new ptr_list();

    // Loop through all objects
    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        guid& ObjectGuid = m_guidsSelectedObjs.GetAt(i);
        object *pObject = g_ObjMgr.GetObjectByGuid( ObjectGuid );
        if( pObject )
        {
            ASSERT(pObject->GetAttrBits() & object::ATTR_EDITOR_SELECTED);

            //set old state for sel objects
            transaction_object_data* pObjData = new transaction_object_data(
                                               transaction_object_data::OBJ_EDIT,
                                               ObjectGuid, pObject->GetTypeDesc().GetTypeName() );

            pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);

            m_pUndoSelectData->AddTail(pObjData);
        }
    }
}

//=========================================================================

void world_editor::UndoCommitSelObjectsProps( )
{
    ASSERT(m_pUndoSelectData != NULL);

    //set new state for sel objects
    if (m_pUndoSelectData)
    {
        //ensure an entry exists
        if (!m_pCurrentUndoEntry)
        {
            SetCurrentUndoEntry(new transaction_entry(xfs("Updating %d Object Properties",m_pUndoSelectData->GetCount())));
        }

        //delete all data
        DATAPOS pos = m_pUndoSelectData->GetHeadPosition();
        while (pos)
        {
            transaction_object_data* pObjData = (transaction_object_data*)m_pUndoSelectData->GetAt(pos);
            ASSERT(pObjData);
            if (pObjData)
            {
                pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                AddStepToCurrentUndoEntry(pObjData);
            }
            m_pUndoSelectData->GetNext(pos);
        }

        //now delete the list
        m_pUndoSelectData->RemoveAll();
        delete m_pUndoSelectData;
        m_pUndoSelectData = NULL;

        CommitCurrentUndoEntry( );
    }
    else
    {
        ClearUndoEntry( );
    }
}

//=========================================================================

void world_editor::SetCurrentUndoEntry( transaction_entry* pEntry )
{
    ASSERT(pEntry != NULL);
    ASSERT(m_pCurrentUndoEntry == NULL);

    m_pCurrentUndoEntry = pEntry;

    //save initial selection
    transaction_selection_data *pSelection = new transaction_selection_data();
    m_pCurrentUndoEntry->AddCommitStep(pSelection);
}

//=========================================================================

void world_editor::AddStepToCurrentUndoEntry( transaction_data* pData )
{
    ASSERT(pData != NULL);
    ASSERT(m_pCurrentUndoEntry != NULL);

    m_pCurrentUndoEntry->AddCommitStep(pData);
}

//=========================================================================

void world_editor::CommitCurrentUndoEntry( void )
{
    ASSERT(m_pCurrentUndoEntry != NULL);

    if (m_pCurrentUndoEntry)
    {
        //save final selection
        transaction_selection_data *pSelection = new transaction_selection_data();
        m_pCurrentUndoEntry->AddCommitStep(pSelection);
        
        //commit the entry
        transaction_mgr::Transaction()->AppendTransaction(m_pCurrentUndoEntry);   

        //set current entry null
        m_pCurrentUndoEntry = NULL;
    }
}

//=========================================================================

xbool world_editor::InTransaction( void )
{
    return (m_pCurrentUndoEntry != NULL);
}

//=========================================================================
// Object Manipulation
//=========================================================================

xbool world_editor::InternalSelectObject( guid ObjectGuid, xbool bClearSelectionList, xbool bSelectAllSubParts )
{
    m_SelectedZone = 0; //clear any selected zone

    if (bClearSelectionList)
    {
        ClearSelectedObjectList();
    }

    x_try;

    object *pObject = g_ObjMgr.GetObjectByGuid( ObjectGuid );

#if 1 // TODO: Revist this object selection and edit issue

    if( pObject)
    {
        editor_layer& Layer = FindObjectsLayer( ObjectGuid, TRUE );
#ifndef _DEBUG
        //only do this check in release mode
        if ( m_bIgnoreReadOnlyChecksForDebug || Layer.IsEditable )
#endif
        {
            pObject->SetAttrBits(pObject->GetAttrBits() | object::ATTR_EDITOR_SELECTED);
            m_guidsSelectedObjs.Append(ObjectGuid);
            
            if( bSelectAllSubParts )  
            {
                // Tell object it got selected from a "select all" command.
                // eg. the path object selects all of its keys
                pObject->GetTypeDesc().OnEditorSelectAll(*pObject) ;
            }          
        }

        return TRUE;
    }

#else

    if(pObject)
    {
        if(pObject->IsSelectable())
        {
            pObject->SetAttrBits(pObject->GetAttrBits() | object::ATTR_EDITOR_SELECTED);
            m_guidsSelectedObjs.Append(ObjectGuid);

            if( bSelectAllSubParts )  
            {
                // Tell object it got selected from a "select all" command.
                // eg. the path object selects all of its keys
                pObject->GetTypeDesc().OnEditorSelectAll(*pObject) ;
            }          
        }

        return TRUE;
    }

#endif

    x_catch_display;

    return FALSE;
}
//=========================================================================

xbool world_editor::OverRideReadOnly(void)
{
#ifndef _DEBUG
    //only do this check in release mode
    return m_bIgnoreReadOnlyChecksForDebug;
#else
    return FALSE;

#endif
}
//=========================================================================

void world_editor::SelectLastSelected( void )
{
    editor_blueprint_ref BPref;
    if (GetBlueprintRefContainingObject(m_LastSelectedGuidA, BPref))
    {
        //select blueprint
        ClearSelectedObjectList();
        SelectBlueprintObjects( BPref, TRUE );
    }
    else
    {
        //select standard object
        SelectObject( m_LastSelectedGuidA, TRUE );
    }
}

//=========================================================================

guid world_editor::CreateObject( const char* pName, const char* pObjectType, const vector3& Pos, const char* pLayer, const char* pPath )
{
    guid NewGuid = guid(0);
    
    x_try;
    NewGuid = g_ObjMgr.CreateObject( pObjectType );
    if (NewGuid.Guid != 0)
    {
        object *pObject = g_ObjMgr.GetObjectByGuid(NewGuid);
        if (pObject)
        {
            InternalSelectObject(NewGuid, TRUE, TRUE);
            vector3 minPos = GetMinPositionForSelected( );

            //ok, we have a position, lets place it on top of that position, rather than centered
            vector3 delta(Pos - minPos);
            pObject->OnMoveRel(delta);

            //if this is a portal we must use the name given
            if (x_strcmp(pObjectType,zone_portal::GetObjectType().GetTypeName())==0)
            {
                //set the name
                prop_query pqWrite;
                pqWrite.WQueryString( "Base\\Name", pName);
                pObject->OnProperty( pqWrite );
            }

            //UNDO set this object
            SetCurrentUndoEntry(new transaction_entry(xfs("New Object(%s)", (const char*)guid_ToString(NewGuid))));
            transaction_object_data* pObjData = new transaction_object_data(
                                                transaction_object_data::OBJ_CREATE,
                                                NewGuid, pObject->GetTypeDesc().GetTypeName() );
            pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
            AddStepToCurrentUndoEntry(pObjData);

            editor_object_ref ObjRef;
            ObjRef.Guid = NewGuid;
            ObjRef.LayerPath = pPath;
            if (AddObjectToLayer(ObjRef, pLayer, TRUE))
            {
                //layer UNDO
                pObjData = new transaction_object_data(
                               transaction_object_data::OBJ_LAYER_CHANGE,
                               NewGuid, pObject->GetTypeDesc().GetTypeName() );
                pObjData->StoreLayer(transaction_data::TRANSACTION_NEW_STATE);
                AddStepToCurrentUndoEntry(pObjData);
            }
            CommitCurrentUndoEntry();
        }
    }
    x_catch_display;

    return NewGuid;
}

//=========================================================================

xbool world_editor::DeleteObject( guid ObjectGuid )
{
    x_try;

    object* pObject = g_ObjMgr.GetObjectByGuid(ObjectGuid);
    if (pObject)
    {
        //set the UNDO for this object
        if (m_pCurrentUndoEntry)
        {
            //delete objects
            transaction_object_data* pObjData = new transaction_object_data(
                                                transaction_object_data::OBJ_DELETE,
                                                ObjectGuid, pObject->GetTypeDesc().GetTypeName() );
            pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);
            m_pCurrentUndoEntry->AddCommitStep(pObjData);
        }

        //remove from layer
        editor_layer& Layer = FindObjectsLayer(ObjectGuid,FALSE);
        if (!Layer.IsNull)
        {
            //set the UNDO for this object's layer
            if (m_pCurrentUndoEntry)
            {
                //remove it from layer
                transaction_object_data* pObjData = new transaction_object_data(
                               transaction_object_data::OBJ_LAYER_CHANGE,
                               ObjectGuid, pObject->GetTypeDesc().GetTypeName() );
                pObjData->StoreLayer(transaction_data::TRANSACTION_OLD_STATE);
                m_pCurrentUndoEntry->AddCommitStep(pObjData);
            }

            RemoveObjectFromLayer(ObjectGuid,Layer.Name);
        }
    
        DestroyObjectEx( ObjectGuid, TRUE );        

        return TRUE;
    }

    x_catch_display;

    return FALSE;
}

//=========================================================================

guid world_editor::CloneObject( guid ObjectGuid )
{
    guid Guid = InternalObjectCopy(ObjectGuid);
    if (Guid != 0)
    {
        editor_layer& Layer = FindObjectsLayer(ObjectGuid,FALSE);
        if (!Layer.IsNull)
        {
            editor_object_ref ObjRef;
            ObjRef.Guid = ObjectGuid;
            ObjRef.LayerPath = FindLayerPathForObject(ObjectGuid, Layer);
            AddObjectToLayer(ObjRef, Layer.Name, TRUE);
        }
    }
    return Guid;
}

//=========================================================================

xbool world_editor::SelectObject( guid ObjectGuid, xbool bClearSelectionList, xbool bSelectAllSubParts )
{
    m_LastSelectedGuidA = m_LastSelectedGuidB;
    m_LastSelectedGuidB = ObjectGuid;

    return InternalSelectObject(ObjectGuid, bClearSelectionList, bSelectAllSubParts);
}

//=========================================================================
// Rendering
//=========================================================================

void world_editor::RenderObjects( xbool bDoPortalWalk, const view& PortalView, u8 StartZone )
{
    g_ObjMgr.Render( bDoPortalWalk, PortalView, StartZone );
#if defined(dstewart)
    g_ZoneMgr.Render();
#endif
}

//=========================================================================

void world_editor::RenderSpacialDBase()
{
    if( eng_Begin( "Spacial DBase" ) )
    {
        g_SpatialDBase.Render( 0, 2 );
        eng_End();
    }
}

//=========================================================================

void world_editor::RenderSelectedObjectCollision()
{
    if( eng_Begin("Collision Render") )
    {
        for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
        {       
            object *pObject = g_ObjMgr.GetObjectByGuid( m_guidsSelectedObjs.GetAt(i) );
            if( pObject )
            {
                pObject->RenderCollision(FALSE);
            }
        }
        eng_End();
    }
}

//=========================================================================
// Temporary Object handling
//=========================================================================

xbool world_editor::CreateTemporaryObject( const char* pObjectType )
{
    ASSERT(m_guidLstTempObject.GetCount() == 0);
    ClearTemporaryObjects(); //clear one if one already exists

    x_try;
    guid& NewGuid = g_ObjMgr.CreateObject( pObjectType );
    m_guidLstTempObject.Append(NewGuid);

    ClearSelectedObjectList();
//    InternalSelectObject(NewGuid, TRUE); //select the temporary object
 
    MoveTemporaryObjects(vector3(0,0,0));

    object *pObj = g_ObjMgr.GetObjectByGuid(NewGuid);    
    if (pObj)
    {
        pObj->SetAttrBits(pObj->GetAttrBits() | object::ATTR_EDITOR_PLACEMENT_OBJECT);
        pObj->EnableDrawBBox();
    }
    x_catch_display;

    return TRUE;
}

//=========================================================================

xbool world_editor::CreateTemporaryObjectFromSel()
{
    x_try;
    ASSERT(m_guidLstTempObject.GetCount() == 0);
    ClearTemporaryObjects(); //clear one if one already exists

    if (m_guidsSelectedObjs.GetCount() == 1)
    {
        guid& NewGuid = InternalObjectCopy(m_guidsSelectedObjs.GetAt(0));
        if (NewGuid.Guid != 0)
        {
            m_guidLstTempObject.Append(NewGuid);

            ClearSelectedObjectList();
    //      InternalSelectObject(NewGuid, TRUE); //select the temporary object
 
            MoveTemporaryObjects(vector3(0,0,0));

            object *pObj = g_ObjMgr.GetObjectByGuid(NewGuid);
            if (pObj)
            {
                pObj->SetAttrBits(pObj->GetAttrBits() | object::ATTR_EDITOR_PLACEMENT_OBJECT);
                pObj->EnableDrawBBox();
            }
        }
    }
    x_catch_display;

    return TRUE;
}

//=========================================================================

xbool world_editor::CreateTemporaryBlueprintObjects( const char* pTheme, const char* pRelPath )
{
    ASSERT(m_bpRefTemp.Guid == 0);
    ASSERT(m_guidLstTempObject.GetCount() == 0);
    m_guidLstTempObject.Clear();
    ClearSelectedObjectList();

    if (AddBlueprintToSpecificLayer(pTheme, pRelPath, "", "\\", m_bpRefTemp, TRUE, FALSE, guid(0) ) > 0)
    {
        //set all objects
        for (s32 i = 0; i < m_bpRefTemp.ObjectsInBlueprint.GetCount(); i++)
        {   
            guid& ObjGuid = m_bpRefTemp.ObjectsInBlueprint.GetAt(i);
            m_guidLstTempObject.Append(ObjGuid);
            object* pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);
            ASSERT(pObject);
            if (pObject)
            {
                pObject->SetAttrBits(pObject->GetAttrBits() | object::ATTR_EDITOR_PLACEMENT_OBJECT);
            }
        }    

        //set anchor
        m_guidLstTempObject.Append(m_bpRefTemp.Anchor);
        object* pAnchor = g_ObjMgr.GetObjectByGuid(m_bpRefTemp.Anchor);
        ASSERT(pAnchor);
        if (pAnchor)
        {
            pAnchor->SetAttrBits(pAnchor->GetAttrBits() | object::ATTR_EDITOR_PLACEMENT_OBJECT);
        }

        return TRUE;
    }
    else 
    { 
        ASSERT(FALSE); 
    }

    return FALSE;
}

//=========================================================================

void world_editor::MoveTemporaryObjects( vector3& pos )
{
    x_try;
    vector3 minPos = GetMinPositionForTempObjects( );

    //ok, we have a position, lets place it on top of that position, rather than centered
    vector3 delta(pos - minPos);

    for( s32 i=0; i < m_guidLstTempObject.GetCount(); i++ )
    {       
        guid ObjGuid = m_guidLstTempObject.GetAt(i);
        object *pObject = g_ObjMgr.GetObjectByGuid( ObjGuid );
        if( pObject )
        {
            pObject->OnMoveRel(delta);
        }
    }
    x_catch_display;
}

//=========================================================================

vector3 world_editor::GetMinPositionForTempObjects( )
{
    vector3 minPos(0.0,0.0,0.0);
    BOOL bInit = FALSE;

    for( s32 i=0; i < m_guidLstTempObject.GetCount(); i++ )
    {       
        //Get Min object position to snap to
        object *pObject = g_ObjMgr.GetObjectByGuid( m_guidLstTempObject.GetAt(i) );
        if( pObject )
        {
            if (!bInit)
            {
                bInit = TRUE;
                minPos = pObject->GetPosition();
            }
            else
            {
                //check for min object
                vector3 objPos = pObject->GetPosition();
                if (objPos.GetY() < minPos.GetY())
                {
                    minPos = objPos;
                }
                else if (objPos.GetY() == minPos.GetY())
                {
                    if (objPos.GetX() < minPos.GetX())
                    {
                        minPos = objPos;
                    }
                    else if (objPos.GetX() == minPos.GetX())
                    {
                        if (objPos.GetZ() < minPos.GetZ())
                        {
                            minPos = objPos;
                        }
                    }
                }
            }
        }
    }
    return minPos;
}

//=========================================================================

bbox world_editor::GetTemporaryObjectsBounds( )
{
    bbox Bounds;
    x_try;
    BOOL bInit = FALSE;
    for( s32 i=0; i < m_guidLstTempObject.GetCount(); i++ )
    {       
        object *pObject = g_ObjMgr.GetObjectByGuid( m_guidLstTempObject.GetAt(i) );
        if( pObject )
        {
            if (!bInit)
            {
                Bounds = pObject->GetBBox();
                bInit = TRUE;
            }
            else
            {
                bbox objBounds = pObject->GetBBox();
                Bounds.AddVerts(&(objBounds.Min),1);
                Bounds.AddVerts(&(objBounds.Max),1);
            }
        }
    }

    x_catch_display;
    return Bounds;
}

//=========================================================================

void world_editor::RotateTemporaryObjects( const radian3& r )
{
    bbox Bounds = GetTemporaryObjectsBounds( );
    vector3 ptCenter = Bounds.GetCenter();
    vector3 ptInvereted = Bounds.GetCenter();
    ptInvereted.Negate();

    // Loop through all objects
    for( s32 i=0; i < m_guidLstTempObject.GetCount(); i++ )
    {       
        guid& ObjectGuid = m_guidLstTempObject.GetAt(i);
        object *pObject = g_ObjMgr.GetObjectByGuid( ObjectGuid );
        if( pObject )
        {
            matrix4 L2W;

            //compute Matrix
            L2W = pObject->GetL2W();
            //first we need to translate the each object to the origin
            L2W.Translate(ptInvereted);
            //now we transform
            L2W.Rotate(r);
            //now we translate back
            L2W.Translate(ptCenter);

            pObject->OnTransform(L2W);   
        }
    }

    m_bpRefTemp.Transform.Rotate(r);
}

//=========================================================================

void world_editor::ClearTemporaryObjects( )
{
    m_SelectedZone = 0;
    for (s32 i = 0; i < m_guidLstTempObject.GetCount(); i++)
    {   
        x_try;
        DestroyObjectEx(m_guidLstTempObject.GetAt(i), TRUE);
        x_catch_display;
    }    
    m_guidLstTempObject.Clear();
    m_guidLastTempPlaced = guid(0);

    m_bpRefTemp.Guid = guid(0);
    m_bpRefTemp.Anchor = guid(0);
    m_bpRefTemp.ObjectsInBlueprint.Clear();
    m_bpRefTemp.Transform.Identity();
    m_bpRefTemp.ThemeName = "";
    m_bpRefTemp.RelativePath = "";
}

//=========================================================================

guid world_editor::GetGuidOfLastPlacedTemp( void )
{
    return m_guidLastTempPlaced;
}

//=========================================================================

guid world_editor::PlaceObjectsFromTemporary( void )
{
    guid GuidNew(0);
    // GuidNew is Object GUID for single object
    // GuidNew is BP Guid for group
    if (m_guidLstTempObject.GetCount() == 1) //must be a single temp object
    {   
        GuidNew = InternalObjectCopy(m_guidLstTempObject.GetAt(0));
        if (GuidNew.Guid != 0)
        {
            object *pObject = g_ObjMgr.GetObjectByGuid(GuidNew);
            if (pObject)
            {
                //UNDO set this object
                SetCurrentUndoEntry(new transaction_entry(xfs("New Object(%s)", (const char*)guid_ToString(GuidNew))));
                transaction_object_data* pObjData = new transaction_object_data(
                                                    transaction_object_data::OBJ_CREATE,
                                                    GuidNew, pObject->GetTypeDesc().GetTypeName() );
                pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                AddStepToCurrentUndoEntry(pObjData);

                if (AddObjectToActiveLayer(GuidNew))
                {
                    //layer UNDO
                    pObjData = new transaction_object_data(
                                   transaction_object_data::OBJ_LAYER_CHANGE,
                                   GuidNew, pObject->GetTypeDesc().GetTypeName() );
                    pObjData->StoreLayer(transaction_data::TRANSACTION_NEW_STATE);
                    AddStepToCurrentUndoEntry(pObjData);
                }

                CommitCurrentUndoEntry();
            }
            CheckForDuplicateObjects(pObject);
        }
    }
    else if (m_guidLstTempObject.GetCount() > 1)
    {
        //setup generic undo entry for moving of objects
        SetCurrentUndoEntry(new transaction_entry(xfs("Adding Blueprint([%s]%s)", (const char*)m_bpRefTemp.ThemeName, (const char*)m_bpRefTemp.RelativePath)));

        ASSERT(m_bpRefTemp.Anchor != 0);
        object *pObjectAnchor = g_ObjMgr.GetObjectByGuid(m_bpRefTemp.Anchor);
        if (pObjectAnchor) 
        {
            editor_blueprint_ref BPNew;
            //the following functions handle their individual undos 
            AddBlueprint(m_bpRefTemp.ThemeName, m_bpRefTemp.RelativePath, BPNew, TRUE, TRUE);
            RotateSelectedObjects( m_bpRefTemp.Transform );
            MoveBlueprintObjectsToAnchor( pObjectAnchor->GetPosition() );

            GuidNew = BPNew.Guid;

            object *pNewAnchor = g_ObjMgr.GetObjectByGuid(BPNew.Anchor);
            if (pNewAnchor) 
            {
                CheckForDuplicateObjects(pNewAnchor);
            }
        }

        //reset selection
        ClearSelectedObjectList();
//        for (s32 i=0; i < m_guidLstTempObject.GetCount(); i++)
//        {
//            InternalSelectObject(m_guidLstTempObject.GetAt(i),FALSE);
//        }

        //commit undo
        CommitCurrentUndoEntry();
    }

    m_guidLastTempPlaced = GuidNew;
    return GuidNew;
}

//=========================================================================

xbool world_editor::CheckForDuplicateObjects( object* pObject )
{
    xbool bReturn = FALSE;
    if (pObject)
    {
        g_ObjMgr.SelectBBox( object::ATTR_ALL, pObject->GetBBox(), pObject->GetType() );
        slot_id SlotID = g_ObjMgr.StartLoop();

        while( SlotID != SLOT_NULL )
        {
            object* pPossibleDup = g_ObjMgr.GetObjectBySlot( SlotID );
            if ((!(pPossibleDup->GetAttrBits() & object::ATTR_EDITOR_PLACEMENT_OBJECT)) &&
                (pPossibleDup->GetGuid() != pObject->GetGuid()) &&
                (pPossibleDup->GetPosition() == pObject->GetPosition()) &&
                (pPossibleDup->GetBBox().Max == pObject->GetBBox().Max) &&
                (pPossibleDup->GetBBox().Min == pObject->GetBBox().Min))
            {
                bReturn = TRUE;
            }
           
            //get next
            SlotID = g_ObjMgr.GetNextResult( SlotID );
        }

        g_ObjMgr.EndLoop();
    }

    if (bReturn && m_pHandler)
    {
        m_pHandler->ShowWarning(xstring("WARNING: You may have just placed an object at the exact position of a similar object. You may have duplicate objects occupying the same space."));
    }

    return bReturn;
}

//=========================================================================

void world_editor::SetTempObjectExternal( const char* pProperty, const char* pValue )
{
    for (s32 i=0; i<m_guidLstTempObject.GetCount(); i++)
    {
        guid ObjectGuid = m_guidLstTempObject.GetAt(i);

        object *pObject = g_ObjMgr.GetObjectByGuid(ObjectGuid);
        if (pObject) 
        {
            prop_query pq;
            pq.WQueryExternal( pProperty, pValue);
            pObject->OnProperty( pq );
        }
    }
}

//=========================================================================

void world_editor::SetTempObjectInt( const char* pProperty, s32 Data )
{
    for (s32 i=0; i<m_guidLstTempObject.GetCount(); i++)
    {
        guid ObjectGuid = m_guidLstTempObject.GetAt(i);

        object *pObject = g_ObjMgr.GetObjectByGuid(ObjectGuid);
        if (pObject) 
        {
            prop_query pq;
            pq.WQueryInt( pProperty, Data );
            pObject->OnProperty( pq );
        }
    }
}

//=========================================================================

xbool world_editor::IsGlobalObject( guid ObjGuid )
{
    object *pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);
    if (pObject)
    {
        const object_desc&  ObjDesc = pObject->GetTypeDesc();
        return ObjDesc.IsGlobalObject();
    }
    return FALSE;
}

//=========================================================================
// Layer handling
//=========================================================================

void world_editor::GetLayerNames( xarray<xstring>& List )
{
    //make sure layer exists
    if (m_listLayers.GetCount()>0)
    {
        for (s32 i = 0; i < m_listLayers.GetCount(); i++)
		{
            editor_layer& Layer = m_listLayers.GetAt(i);
            List.Append(Layer.Name);
        }
    }
}

//=========================================================================

xbool world_editor::RenameLayer( const char* pOldName, const char* pNewName )
{
    //find the layer
    if (m_listLayers.GetCount()>0)
    {
        for (s32 i = 0; i < m_listLayers.GetCount(); i++)
		{
            editor_layer& Layer = m_listLayers.GetAt(i);
            if (x_stricmp(pOldName, Layer.Name) == 0)
            {
                //m_listLayers.Delete(i);
                Layer.Name = pNewName;
                MarkLayerDirty(pNewName);
                m_listLayers.SetAt(i,Layer);

                if (x_stricmp(m_xstrActiveLayer,pOldName)==0)
                {
                    //oh, just renamed the active layer
                    m_xstrActiveLayer = pNewName;
                }

                //UNDO
                if (m_pCurrentUndoEntry)
                {
                    transaction_layer_data* pData = new transaction_layer_data(transaction_layer_data::LYR_RENAME,pOldName,pNewName);
                    m_pCurrentUndoEntry->AddCommitStep(pData);
                }

                return TRUE;
            }
        }
    }
    return FALSE;
}

//=========================================================================

xbool world_editor::AddLayer( const char* pLayer, xbool bIsDirty )
{
    //make sure layer does not exists!
    if (m_listLayers.GetCount()>0)
    {
        for (s32 i = 0; i < m_listLayers.GetCount(); i++)
		{
            editor_layer& Layer = m_listLayers.GetAt(i);
            if (x_stricmp(pLayer, Layer.Name) == 0)
            {
                return FALSE;
            }
        }
    }

    editor_layer Layer;
    Layer.Name = pLayer;
    m_listLayers.Append(Layer);
    if (bIsDirty)
    {
        MarkLayerDirty(Layer.Name);
    }

    if (m_pCurrentUndoEntry)
    {
        //UNDO
        transaction_layer_data* pData = new transaction_layer_data(transaction_layer_data::LYR_CREATE,"",pLayer);
        m_pCurrentUndoEntry->AddCommitStep(pData);
    }

    return TRUE;
}

//=========================================================================

xbool world_editor::RemoveLayer( const char* pLayer )
{
    ClearSelectedObjectList();

    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& Layer = m_listLayers.GetAt(i);
        if (x_stricmp(pLayer, Layer.Name) == 0)
        {
            //layer found, delete all objects in layer
            for (s32 j = 0; j < Layer.Objects.GetCount(); j++)
		    {
                editor_object_ref& ObjRef = Layer.Objects.GetAt(j);
                object* pObjDel = g_ObjMgr.GetObjectByGuid(ObjRef.Guid);
                if (pObjDel)
                { 
                    //UNDO set this object
                    if (m_pCurrentUndoEntry)
                    {
                        transaction_object_data* pObjData = new transaction_object_data(
                                                            transaction_object_data::OBJ_DELETE,
                                                            ObjRef.Guid, pObjDel->GetTypeDesc().GetTypeName() );
                        pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);
                        m_pCurrentUndoEntry->AddCommitStep(pObjData);

                        pObjData = new transaction_object_data(
                                       transaction_object_data::OBJ_LAYER_CHANGE,
                                       ObjRef.Guid, pObjDel->GetTypeDesc().GetTypeName() );
                        pObjData->StoreLayer(transaction_data::TRANSACTION_OLD_STATE);
                        m_pCurrentUndoEntry->AddCommitStep(pObjData);
                    }
                    DestroyObjectEx( ObjRef.Guid, TRUE );
                }
            }

            //delete all blueprints in layer, with associated objects
            for (s32 k = 0; k < Layer.Blueprints.GetCount(); k++)
		    {
                editor_blueprint_ref& BlueprintRef = Layer.Blueprints.GetAt(k);
                //now delete all objects in blueprint
                for (s32 m = 0; m < BlueprintRef.ObjectsInBlueprint.GetCount(); m++)
	    	    {
                    guid& ObjectGuid = BlueprintRef.ObjectsInBlueprint.GetAt(m);
                    object* pObjDel = g_ObjMgr.GetObjectByGuid(ObjectGuid);

                    if (pObjDel)
                    { 
                        //UNDO set this BluePrint object
                        if (m_pCurrentUndoEntry)
                        {
                            //object change only, bp will take care of layer
                            transaction_object_data* pObjData = new transaction_object_data(
                                                                transaction_object_data::OBJ_DELETE,
                                                                ObjectGuid,pObjDel->GetTypeDesc().GetTypeName() );
                            pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);
                            m_pCurrentUndoEntry->AddCommitStep(pObjData);
                        }
                        DestroyObjectEx( ObjectGuid, TRUE );
                    }
                }

                //delete the anchor
                object* pAnchorDel = g_ObjMgr.GetObjectByGuid(BlueprintRef.Anchor);
                if (pAnchorDel)
                {
                    //UNDO set this BluePrint anchor object
                    if (m_pCurrentUndoEntry)
                    {
                        //anchor change only, bp will take care of layer
                        transaction_object_data* pObjData = new transaction_object_data(
                                                            transaction_object_data::OBJ_DELETE,
                                                            BlueprintRef.Anchor,pAnchorDel->GetTypeDesc().GetTypeName() );
                        pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);
                        m_pCurrentUndoEntry->AddCommitStep(pObjData);

                    }
                    DestroyObjectEx( BlueprintRef.Anchor, TRUE );
                }

                //UNDO set this BluePrint ref
                if (m_pCurrentUndoEntry)
                {
                    transaction_bpref_data* pRefData = new transaction_bpref_data(
                                                        transaction_bpref_data::BPREF_DELETE,
                                                        BlueprintRef.Guid);
                    pRefData->StoreReferenceData(transaction_data::TRANSACTION_OLD_STATE);
                    m_pCurrentUndoEntry->AddCommitStep(pRefData);
                }
            }                

            m_listLayers.Delete(i);
        
            if (x_stricmp(m_xstrActiveLayer,pLayer)==0)
            {
                //oh, just removed the active layer
                m_xstrActiveLayer = GetDefaultLayer( );
                m_xstrActiveLayerPath = "\\";
            }

            //UNDO now remove layer
            if (m_pCurrentUndoEntry)
            {
                transaction_layer_data* pLayerData = new transaction_layer_data(transaction_layer_data::LYR_DELETE,pLayer,"");
                m_pCurrentUndoEntry->AddCommitStep(pLayerData);
            }

            return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================

const char* world_editor::GetDefaultLayer( )
{
    return ".Default";
}

//=========================================================================

const char* world_editor::GetGlobalLayer( )
{
    return ".Globals";
}

//=========================================================================

xbool world_editor::SetActiveLayer( const char* pLayer, const char* pLayerPath )
{
    //make sure layer exists
    if (DoesLayerExist(pLayer))
    {
        //layer exists
        m_xstrActiveLayer = pLayer;
        m_xstrActiveLayerPath = pLayerPath;
        return TRUE;
	}
    return FALSE;
}

//=========================================================================

xbool world_editor::SetCurrentObjectsLayerAsActive( void )
{
    if (m_guidsSelectedObjs.GetCount() > 0)
    {
        guid ObjGuid = m_guidsSelectedObjs.GetAt(0);
        editor_layer &Layer = FindObjectsLayer(ObjGuid, TRUE);
        if (!Layer.IsNull)
        {
            guid BPGuid = GetBlueprintGuidContainingObject(ObjGuid);
            if (BPGuid.Guid != 0)
            {
                //a blueprint
                SetActiveLayer(Layer.Name, FindLayerPathForBlueprint(BPGuid,Layer));
            }
            else
            {
                //an object
                SetActiveLayer(Layer.Name, FindLayerPathForObject(ObjGuid,Layer));
            }
            return TRUE;
        }
    }
    return FALSE;
}

//=========================================================================

xbool world_editor::DoesLayerExist( const char* pLayer )
{
    for (s32 iXaIndex = 0; iXaIndex < m_listLayers.GetCount(); iXaIndex++)
	{
        editor_layer& Layer = m_listLayers.GetAt(iXaIndex);
        if (x_stricmp(pLayer, Layer.Name) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================

xbool world_editor::DoesLayerContainZones( const char* pLayer )
{
    for( s32 i=0; i < m_listZones.GetCount(); i++ )
    {       
        editor_zone_ref &Zone = m_listZones.GetAt(i);
        if ( x_strcmp(Zone.Layer, pLayer) == 0 )
        {
            //contains at least 1 zone
            return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================

const char* world_editor::GetActiveLayer( )
{
    return m_xstrActiveLayer;
}

//=========================================================================

const char* world_editor::GetActiveLayerPath( )
{
    return m_xstrActiveLayerPath;
}

//=========================================================================

editor_layer& world_editor::GetLayerInfo( const char* pLayer )
{
    //make sure layer exists
    for (s32 iXaIndex = 0; iXaIndex < m_listLayers.GetCount(); iXaIndex++)
	{
        editor_layer& Layer = m_listLayers.GetAt(iXaIndex);
        if (x_stricmp(pLayer, Layer.Name) == 0)
        {
            return Layer;
        }
    }
    x_throw(xfs("Error, Couldn't find Layer %s!",pLayer));
    
    return m_NullLayer;
}

//=========================================================================

const char* world_editor::GetLayerContainingBlueprint( guid BlueprintGuid )
{
    for (s32 iXaIndex = 0; iXaIndex < m_listLayers.GetCount(); iXaIndex++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(iXaIndex);
        for (s32 i=0; i < eLayer.Blueprints.GetCount() ; i++)
        {
            editor_blueprint_ref& BPRef = eLayer.Blueprints.GetAt(i);
            if (BPRef.Guid == BlueprintGuid)
            {
                return eLayer.Name;
            }
        }
    }
    return "";
}

//=========================================================================

const char* world_editor::GetLayerContainingObject( guid ObjectGuid )
{
    for (s32 iXaIndex = 0; iXaIndex < m_listLayers.GetCount(); iXaIndex++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(iXaIndex);
        for (s32 i=0; i < eLayer.Objects.GetCount() ; i++)
        {
            editor_object_ref& ObjRef = eLayer.Objects.GetAt(i);
            if ( ObjRef.Guid == ObjectGuid )
            {
                return eLayer.Name;
            }
        }
    }
    return "";
}

//=========================================================================

void world_editor::SelectAllItemsInLayer( const char* pLayer )
{
    ClearSelectedObjectList();

    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& Layer = m_listLayers.GetAt(i);
        if (x_stricmp(pLayer, Layer.Name) == 0)
        {
            //layer found, select all objects in layer
            for (s32 j = 0; j < Layer.Objects.GetCount(); j++)
		    {
                editor_object_ref& ObjRef = Layer.Objects.GetAt(j);
                if (g_ObjMgr.GetObjectByGuid(ObjRef.Guid))
                { 
                    SelectObject( ObjRef.Guid, FALSE, TRUE );
                }
            }

            //select all blueprints in layer
            for (s32 k = 0; k < Layer.Blueprints.GetCount(); k++)
		    {
                editor_blueprint_ref& BlueprintRef = Layer.Blueprints.GetAt(k);
                g_WorldEditor.SelectBlueprintObjects(BlueprintRef,TRUE);
            }                
        }
    }
}

//=========================================================================
// Resource handling
//=========================================================================

xbool world_editor::IsResourceInLayer( const char* pRes, const char* pLayer )
{
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pLayer);
        //layer exists
        for (s32 i=0; i< Layer.Resources.GetCount(); i++)
        {
            xstring &xstrRes = Layer.Resources.GetAt(i);
            if (x_stricmp(xstrRes, pRes) == 0)
            {
                return TRUE;
            }
        }
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return FALSE;
}

//=========================================================================

xbool world_editor::AddResourceToLayer( const char* pRes, const char* pLayer, xbool bIsDirty )
{
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pLayer);
        //layer exists
        xbool bReturn = Layer.AddResource(pRes);
        if (bReturn) 
        {
            if (bIsDirty) 
            {
                MarkLayerDirty(Layer.Name);
            }
        }        
        return bReturn;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return FALSE;
}

//=========================================================================

xbool world_editor::RemoveResourceFromLayer( const char* pRes, const char* pLayer )
{
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pLayer);
        //layer exists
        xbool bReturn = Layer.RemoveResource(pRes);
        if (bReturn) 
        {
            MarkLayerDirty(Layer.Name);
        }        
        return bReturn;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return FALSE;
}

//=========================================================================

xbool world_editor::GetResourcesInLayer( const char* pLayer, xarray<xstring>& List )
{
    x_try;
    {
         editor_layer& Layer = GetLayerInfo(pLayer);
         List.Clear();
         //layer exists
         for (s32 i=0; i < Layer.Resources.GetCount(); i++)
         {
             xstring& xstrRes = Layer.Resources.GetAt(i);
             List.Append(xstrRes);
         }
         return TRUE;        
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return FALSE;
}

//=========================================================================

void world_editor::RefreshResources( void )
{
/*
    if (!g_game_running)
    {
        StoreState(FALSE);
        g_ObjMgr.Clear();
        g_RscMgr.RefreshAll();
        ResetState(FALSE);
    }
*/
    render_inst::UnregisterAll();
    g_RscMgr.RefreshAll();
    render_inst::RegisterAll();

    m_bRefreshRsc = FALSE;
}

//=========================================================================
// Layer with Object handling
//=========================================================================

xbool world_editor::GetObjectsInLayer( const char* pLayer, xarray<guid>& List )
{
    x_try;
    {
         editor_layer& Layer = GetLayerInfo(pLayer);
         List.Clear();
         //layer exists
         for (s32 i=0; i < Layer.Objects.GetCount(); i++)
         {
             editor_object_ref& ObjRef = Layer.Objects.GetAt(i);
             List.Append(ObjRef.Guid);
         }

        if (List.GetCount() > 0)
            return TRUE;
        else
            return FALSE;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return FALSE;
}

//=========================================================================

xbool world_editor::GetObjectsInLayer( const char* pLayer, xarray<editor_object_ref>& List )
{
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pLayer);
        List = Layer.Objects;
        if (List.GetCount() > 0)
            return TRUE;
        else
            return FALSE;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return FALSE;
}

//=========================================================================

xbool world_editor::GetBlueprintsInLayer( const char* pLayer, xarray<editor_blueprint_ref>& List )
{
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pLayer);
        List = Layer.Blueprints;
        if (List.GetCount() > 0)
            return TRUE;
        else
            return FALSE;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return FALSE;
}
//=========================================================================

xbool world_editor::GetAllObjectsInAllBluePrints( xarray<guid> &lstGuids )
{
    xarray<guid> lstGuidsInLayer;

    lstGuids.Clear();

    xarray <xstring> ListLayers;
    g_WorldEditor.GetLayerNames ( ListLayers );

    for (int j = 0; j < ListLayers.GetCount(); j++)
    {
        xarray<editor_blueprint_ref> lstBPRef;
        if (GetBlueprintsInLayer(ListLayers.GetAt(j), lstBPRef ))
        {
            for (s32 k=0; k < lstBPRef.GetCount(); k++)
            {
                editor_blueprint_ref& BPRef = lstBPRef.GetAt(k);
                lstGuids.Append(BPRef.Anchor);
                for (j=0; j < BPRef.ObjectsInBlueprint.GetCount(); j++)
                {
                    guid ObjGuid = BPRef.ObjectsInBlueprint.GetAt(j);
                    lstGuids.Append(ObjGuid);
                }
            }
        }
    }
    return TRUE;
}

//=========================================================================

xbool world_editor::GetAllObjectsInLayer( const char* pLayer, xarray<guid> &lstGuids )
{
    xarray<guid> lstGuidsInLayer;
    editor_layer& Layer = GetLayerInfo(pLayer);

    lstGuids.Clear();
    if(!GetObjectsInLayer( Layer.Name, lstGuidsInLayer ))
        return FALSE;
    //add to export list
    for (s32 j = 0; j < lstGuidsInLayer.GetCount(); j++)
    {
        lstGuids.Append(lstGuidsInLayer.GetAt(j));
    }
    xarray<editor_blueprint_ref> lstBPRef;
    if (GetBlueprintsInLayer( Layer.Name, lstBPRef ))
    {
        for (s32 k=0; k < lstBPRef.GetCount(); k++)
        {
            editor_blueprint_ref& BPRef = lstBPRef.GetAt(k);
            lstGuids.Append(BPRef.Anchor);
            for (j=0; j < BPRef.ObjectsInBlueprint.GetCount(); j++)
            {
                guid ObjGuid = BPRef.ObjectsInBlueprint.GetAt(j);
                lstGuids.Append(ObjGuid);
            }
        }
    }
    return TRUE;
}


//=========================================================================

s32 world_editor::ValidateObjectProperties( xarray<property_error>& Errors, const char* pLogChannel )
{
    s32     i;
    slot_id SlotID;
    s32     nErrors = 0;

    // Clear error list
    Errors.Clear();

    // Loop through all object types
    for( i = object::TYPE_NULL ; i < object::TYPE_END_OF_LIST ; i++ )
    {
        // Loop through all objects of type
        SlotID = g_ObjMgr.GetFirst( (object::type)i );
        while( SlotID != SLOT_NULL )
        {
            // Get object
            object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
            ASSERT( pObject );

            // Clear errors
            xstring ErrorMsg;
            s32     nObjectErrors = 0;

            // Make sure only one object has this guid
            guid    Guid         = pObject->GetGuid();
            object* pOtherObject = g_ObjMgr.GetObjectByGuid( Guid );
            if( pObject != pOtherObject )
            {
                // Add guid error
                nObjectErrors++;
                ErrorMsg += xstring( "Object has same guid as another object in the world:" ) +
                                   + "\n Type: \""  + g_ObjMgr.GetNameFromType( pOtherObject->GetType() ) + "\""
                                   + "\n Name: \""  + pOtherObject->GetName() + "\""
                                   + "\n Guid: " + xfs("%08X:%08X",(u32)(Guid>>32),(u32)Guid )
                                   + "\n Layer: \"" + GetLayerContainingObject( Guid ) + "\""
                                   + "\n\n";
            }
            else
            {
                // Check object for errors           
                nObjectErrors += pObject->OnValidateProperties( ErrorMsg );
            }
                            
            // Report errors?                            
            if( nObjectErrors )
            {
                // Setup error info
                xstring ErrorCount;
                ErrorCount.Format( "%d", nObjectErrors );

                // Add to error list
                property_error& Error = Errors.Append();
                Error.m_ObjectGuid  = pObject->GetGuid();
                Error.m_ErrorMsg   = xstring( "OBJECT PROPERTY ERROR:\n" )
                                            + "\nType: \"" + xstring( g_ObjMgr.GetNameFromType( pObject->GetType() ) ) + "\""
                                            + "\nName: \"" + pObject->GetName() +  "\""
                                            + "\nGuid: " + xstring( xfs("%08X:%08X",(u32)(Guid>>32),(u32)Guid));
                                    
                // Only add layer info if single guid since layers only store guids
                if( pObject == pOtherObject )
                    Error.m_ErrorMsg += xstring( "\nLayer: \"" ) + GetLayerContainingObject( Guid ) + "\"";
                    
                // Append errors
                Error.m_ErrorMsg += xstring( "\nErrors: " ) + ErrorCount + "\n\n" + ErrorMsg + "\n\n";
                nErrors += nObjectErrors;

                // Output to debug window
                x_DebugMsg( Error.m_ErrorMsg );

                // Log the error?
                if( pLogChannel )
                    LOG_ERROR( pLogChannel, Error.m_ErrorMsg );
            }
            
            // Check next object slot
            SlotID = g_ObjMgr.GetNext( SlotID );
        }
    }

    return nErrors;
}

//=========================================================================

xbool world_editor::RemoveObjectFromLayer( guid ObjectGuid , const char* pLayer )
{
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pLayer);
        //layer exists
        xbool bReturn = Layer.RemoveObject(ObjectGuid);
        if (bReturn) 
        {
            MarkLayerDirty(Layer.Name);
        }        
        return bReturn;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return FALSE;
}

//=========================================================================

xbool world_editor::AddObjectToLayer( editor_object_ref ObjRef, const char* pLayer, xbool bIsDirty )
{
    x_try;
    {
        if (IsGlobalObject(ObjRef.Guid))
        {
            //only can add to global layer
            editor_layer& Layer = GetLayerInfo(GetGlobalLayer());
            //layer exists
            xbool bReturn = Layer.AddObject(ObjRef);
            if (bReturn) 
            {
                if (bIsDirty) 
                {
                    MarkLayerDirty(Layer.Name);
                }
            }        
            return bReturn;   
        }
        else
        {
            editor_layer& Layer = GetLayerInfo(pLayer);
            //layer exists
            xbool bReturn = Layer.AddObject(ObjRef);
            if (bReturn) 
            {
                //must determine if this is part of a zone
                xstring xstrZone;
                u8 Zone1 = 0;
                u8 Zone2 = 0;
                if (IsPartOfAPortal(ObjRef.LayerPath, pLayer, Zone1, Zone2))
                {
                    SetObjectsZone(ObjRef.Guid, Zone1, Zone2);
                }
                else if (IsPartOfAZone(ObjRef.LayerPath, pLayer, xstrZone))
                {
                    SetObjectsZone(ObjRef.Guid, GetZoneId(xstrZone));
                }
                else
                {
                    SetObjectsZone(ObjRef.Guid, 0);
                }                
                
                if (bIsDirty) 
                {
                    MarkLayerDirty(Layer.Name);
                }
            }        
            return bReturn;        
        }

    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return FALSE;
}

//=========================================================================

xbool world_editor::RemoveBlueprintFromLayer( guid BlueprintGuid , const char* pLayer, xbool bIsDirty )
{
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pLayer);
        //layer exists
        xbool bReturn = Layer.RemoveBlueprint(BlueprintGuid);
        if (bReturn) 
        {
            if (bIsDirty) 
            {
                MarkLayerDirty(Layer.Name);
            }
        }        
        return bReturn;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return FALSE;
}

//=========================================================================

xbool world_editor::AddBlueprintToLayer( const editor_blueprint_ref& Blueprint, const char* pLayer, xbool bIsDirty )
{
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pLayer);
        //layer exists
        xbool bReturn = Layer.AddBlueprint(Blueprint);
        if (bReturn) 
        {
            //must determine if this is part of a zone
            xstring xstrZone;
            u8 Zone1 = 0;
            u8 Zone2 = 0;
            if (IsPartOfAPortal(Blueprint.LayerPath, pLayer, Zone1, Zone2))
            {
                SetBlueprintObjectsZone(Blueprint.Guid, Zone1, Zone2);
            }
            else if (IsPartOfAZone(Blueprint.LayerPath, pLayer, xstrZone))
            {
                SetBlueprintObjectsZone(Blueprint.Guid, GetZoneId(xstrZone));
            }
            else
            {
                SetBlueprintObjectsZone(Blueprint.Guid, 0);
            }

            if (bIsDirty) 
            {
                MarkLayerDirty(Layer.Name);
            }
        }        
        return bReturn;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return FALSE;
}

//=========================================================================

xbool world_editor::AddObjectToActiveLayer( guid ObjectGuid )
{
    editor_object_ref ObjRef;
    ObjRef.Guid = ObjectGuid;
    ObjRef.LayerPath = GetActiveLayerPath();
    return AddObjectToActiveLayer(ObjRef);
}

//=========================================================================

xbool world_editor::AddObjectToActiveLayer( editor_object_ref ObjRef )
{
    return AddObjectToLayer( ObjRef, m_xstrActiveLayer, TRUE );
}

//=========================================================================

xbool world_editor::AddBlueprintToActiveLayer( const editor_blueprint_ref& Blueprint, xbool bIsDirty )
{
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(m_xstrActiveLayer);
        //layer exists
        xbool bReturn = Layer.AddBlueprint(Blueprint);
        if (bReturn) 
        {
            if (bIsDirty) 
            {
                MarkLayerDirty(Layer.Name);
            }
        }        
        return bReturn;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return FALSE;
}

//=========================================================================

editor_layer& world_editor::FindObjectsLayer( guid ObjectGuid, xbool bIncludeBlueprints )
{
    //make sure layer exists
    if (m_listLayers.GetCount() > 0 )
    {
        for (s32 i = 0; i < m_listLayers.GetCount(); i++)
		{
            editor_layer& eLayer = m_listLayers.GetAt(i);
            //loop through objects
            if ( eLayer.Objects.GetCount() > 0 )
            {
                for (s32 j = 0; j < eLayer.Objects.GetCount(); j++)
		        {
                    editor_object_ref& ObjRef = eLayer.Objects.GetAt(j);
                    if (ObjRef.Guid == ObjectGuid)
                    {
                        return eLayer;
                    }
                }
	        }   
            //loop through blueprints
            if (bIncludeBlueprints && eLayer.Blueprints.GetCount() > 0 )
            {
                for (s32 j = 0; j < eLayer.Blueprints.GetCount(); j++)
		        {
                    editor_blueprint_ref& Blueprint = eLayer.Blueprints.GetAt(j);
                    for (s32 k = 0; k < Blueprint.ObjectsInBlueprint.GetCount(); k++)
		            {
                        guid& ListGuid = Blueprint.ObjectsInBlueprint.GetAt(k);
                        if (ListGuid == ObjectGuid)
                        {
                            return eLayer;
                        }
                    }
                    //check anchor also
                    if (Blueprint.Anchor == ObjectGuid)
                    {
                        return eLayer;
                    }
                }
            }
        }
	}
    return m_NullLayer;
}

//=========================================================================

editor_layer& world_editor::FindBlueprintsLayer( guid BlueprintGuid )
{
    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(i);
        for (s32 j = 0; j < eLayer.Blueprints.GetCount(); j++)
		{
            editor_blueprint_ref& Blueprint = eLayer.Blueprints.GetAt(j);
            if (Blueprint.Guid == BlueprintGuid)
            {
                return eLayer;
            }
        }
    }
    return m_NullLayer;
}

//=========================================================================

const char* world_editor::FindLayerPathForObject( guid ObjectGuid, editor_layer& Layer )
{
    for (s32 j = 0; j < Layer.Objects.GetCount(); j++)
	{
        editor_object_ref& ObjRef = Layer.Objects.GetAt(j);
        if (ObjRef.Guid == ObjectGuid)
        {
            return ObjRef.LayerPath;
        }
    }
    return "\\";
}

//=========================================================================

const char* world_editor::FindLayerPathForBlueprint( guid BlueprintGuid, editor_layer& Layer )
{
    for (s32 j = 0; j < Layer.Blueprints.GetCount(); j++)
	{
        editor_blueprint_ref& BPRef = Layer.Blueprints.GetAt(j);
        if (BPRef.Guid == BlueprintGuid)
        {
            return BPRef.LayerPath;
        }
    }
    return "\\";
}

//=========================================================================

void world_editor::SetObjectLayerPath( guid ObjectGuid, const char* pLayer, const char* pLayerPath )
{
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pLayer);
        //layer exists
        for (s32 i = 0; i < Layer.Objects.GetCount(); i++)
        {
            editor_object_ref& ObjRef = Layer.Objects.GetAt(i);
            if (ObjRef.Guid == ObjectGuid)
            {
                transaction_object_data* pObjData = NULL;
                if (m_pCurrentUndoEntry)
                {
                    pObjData = new transaction_object_data(
                                       transaction_object_data::OBJ_LAYER_CHANGE,
                                       ObjectGuid, g_WorldEditor.GetObjectTypeName(ObjectGuid) );
                    pObjData->StoreLayer(transaction_data::TRANSACTION_OLD_STATE);
                }

                ObjRef.LayerPath = pLayerPath;
                Layer.IsDirty = TRUE;

                if (pObjData)
                {
                    pObjData->StoreLayer(transaction_data::TRANSACTION_NEW_STATE);
                    m_pCurrentUndoEntry->AddCommitStep(pObjData);
                }
                
                return;
            }
        }       
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;
}

//=========================================================================

void world_editor::SetBlueprintLayerPath( guid BPGuid, const char* pLayer, const char* pLayerPath )
{
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pLayer);
        //layer exists
        for (s32 i = 0; i < Layer.Blueprints.GetCount(); i++)
        {
            editor_blueprint_ref& BPRef = Layer.Blueprints.GetAt(i);
            if (BPRef.Guid == BPGuid)
            {
                transaction_bpref_data* pRefData = NULL;
                if (m_pCurrentUndoEntry)
                {
                    pRefData = new transaction_bpref_data(transaction_bpref_data::BPREF_EDIT, BPRef.Guid);
                    pRefData->StoreReferenceData(transaction_data::TRANSACTION_OLD_STATE);
                }

                BPRef.LayerPath = pLayerPath;
                Layer.IsDirty = TRUE;

                if (pRefData)
                {
                    pRefData->StoreReferenceData(transaction_data::TRANSACTION_NEW_STATE);
                    m_pCurrentUndoEntry->AddCommitStep(pRefData);
                }
                
                return;
            }
        }         
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;
}

//=========================================================================

void world_editor::SetObjectsLayerAsDirty( guid ObjectGuid )
{
    //update objects layer (dirty)
    editor_layer &Layer = FindObjectsLayer(ObjectGuid, TRUE);
    if (!Layer.IsNull)
    {
        MarkLayerDirty(Layer.Name);
        return;
    }
}

//=========================================================================

s32 world_editor::GetDirtyLayerCount( void )
{
    s32 nCount = 0;
    for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	{
        editor_layer& eLayer = m_listLayers.GetAt(i);
        if ( eLayer.IsDirty )
        {
            nCount++;
	    }
    }
    return nCount;
}

//=========================================================================

xbool world_editor::IsLayerDirty( const char* pLayer )
{
    //make sure layer exists
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pLayer);
        return Layer.IsDirty;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return FALSE;
}

//=========================================================================

xbool world_editor::IsLayerReadonly( const char* pLayer )
{
    //make sure layer exists
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pLayer);
        return !Layer.IsEditable;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;

    return TRUE;
}

//=========================================================================

void world_editor::MarkLayerReadonly( const char* pLayer, xbool bReadonly )
{
    //make sure layer exists
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pLayer);
        Layer.IsEditable = !bReadonly;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;
}

//=========================================================================

void world_editor::MarkLayerDirty( const char* pLayer )
{
    if( m_DisableDirtyTracking != 0 )
        return;

    //make sure layer exists
    x_try;
    {
        editor_layer& Layer = GetLayerInfo(pLayer);
        Layer.IsDirty = TRUE;
        m_pHandler->SetLayerDirty(Layer.Name);        
        return;
    }
    x_catch_begin;
    //failure
    TRACE("FAILURE to find layer!\n");
    x_catch_end;
}

//=========================================================================

void world_editor::DisableDirtyTracking( void )
{
    m_DisableDirtyTracking++;
}

//=========================================================================

void world_editor::EnableDirtyTracking( void )
{
    m_DisableDirtyTracking--;
}

//=========================================================================

//=========================================================================
// Internal Object handling
//=========================================================================

//=========================================================================

guid world_editor::InternalObjectCopy( guid ObjectGuid )
{
    guid GuidNewObj(0);
 
    x_try;
    object *pObjOriginal = g_ObjMgr.GetObjectByGuid(ObjectGuid);
    if (pObjOriginal)
    {
        //is this a valid registered object
        object::type        ObjType = pObjOriginal->GetType();
        const object_desc&  ObjDesc = pObjOriginal->GetTypeDesc();

        {
            //make sure its not in the NON-COPY list
            if( ObjDesc.NoAllowEditorCopy() == FALSE )
            {
                GuidNewObj = g_ObjMgr.CreateObject( ObjDesc );
                object *pObjNew = g_ObjMgr.GetObjectByGuid(GuidNewObj);

                if (pObjNew)
                {
                    //
                    // This is the right way of copying an object
                    //
                    xarray<prop_container> Containers;
                    u32 OldBits = pObjNew->GetAttrBits();
                    pObjOriginal->OnCopy( Containers );
                    pObjNew->OnPaste( Containers );
                    pObjNew->SetAttrBits(OldBits);
            
                    /*
                    //
                    // Then I assume this is the wrong way
                    //
                    prop_enum              listOriginal;
                    xarray<prop_container> containers;
                    pObjOriginal->OnEnumProp(listOriginal);

		            for (int iXarryIndex = 0; iXarryIndex < listOriginal.GetCount(); iXarryIndex++)
		            {
                        prop_enum::node enData = listOriginal[iXarryIndex];
			            prop_type type = (prop_type)enData.GetType();
			            BOOL bReadOnly = type & PROP_TYPE_READ_ONLY;
			            if (!bReadOnly && (type & PROP_TYPE_BASIC_MASK))
                        {
                            prop_query pq;
                            prop_container pc;
                            pc.InitPropEnum(enData);
                            pq.RQuery(enData.GetName(), pc);
                            pObjOriginal->OnProperty(pq);
                            containers.Append(pc);
                        }
                    }

		            for (int iContainerIndex = 0; iContainerIndex < containers.GetCount(); iContainerIndex++)
		            {
			            prop_container& pc = containers.GetAt(iContainerIndex);
                        prop_query pq;
                        pq.WQuery(pc);
                        pObjNew->OnProperty(pq);
                    }
                    */
                }
            }
        }
    }
    x_catch_display;

    return GuidNewObj;
}   

//=========================================================================
// External Object handling
//=========================================================================

guid world_editor::CreateNewObjectAtPosition( const char* pObjectType, vector3& pos, xbool bSelect )
{
    guid Guid(0);

    x_try;
    Guid = g_ObjMgr.CreateObject( pObjectType );
 
    object *pObj = g_ObjMgr.GetObjectByGuid(Guid);
    if (pObj) 
    {
        pObj->OnMove(pos);
    }

    if (bSelect)
    {
        InternalSelectObject(Guid, TRUE, TRUE); //select the object
    }

    x_catch_display;

    return Guid;
}

//=========================================================================
/*
object::type world_editor::GetObjectType( guid ObjGuid )
{
    object *pObj = g_ObjMgr.GetObjectByGuid(ObjGuid);
    if (pObj) 
    {
        return pObj->GetType();
    }

    return object::TYPE_NULL;
}
*/

//=========================================================================

const char* world_editor::GetObjectTypeName( guid ObjGuid )
{
    object *pObj = g_ObjMgr.GetObjectByGuid(ObjGuid);
    if (pObj) 
    {
        return pObj->GetTypeDesc().GetTypeName();
    }

    return NULL;
}

//=========================================================================
/*
object::type world_editor::GetRegisteredObjectTypeFromName ( const char* pName )
{
    object::type oType = object::TYPE_NULL;
    x_try;
        oType = m_lstObjectTypes.GetValue(pName);
    x_catch_display;
    return oType;
}
*/

//=========================================================================
// External Object handling
//=========================================================================

void world_editor::ComputeLightLayer( const char* pLayer, s32 iType )
{
    x_try;
    
    xtimer LoadTimer;
    LoadTimer.Start();

    xarray<guid> lstGuids;
    if (x_strlen(pLayer)>0)
    {
        //light specific layer
        GetObjectsInLayer( pLayer, lstGuids );
        xarray<editor_blueprint_ref> lstBPRef;
        if (GetBlueprintsInLayer( pLayer, lstBPRef ))
        {
            for (s32 i=0; i < lstBPRef.GetCount(); i++)
            {
                editor_blueprint_ref& BPRef = lstBPRef.GetAt(i);
                for (s32 j=0; j < BPRef.ObjectsInBlueprint.GetCount(); j++)
                {
                    guid ObjGuid = BPRef.ObjectsInBlueprint.GetAt(j);
                    lstGuids.Append(ObjGuid);
                }
            }
        }
    }
    else
    {
        //light all layers
        for (s32 i = 0; i < m_listLayers.GetCount(); i++)
	    {
            editor_layer& eLayer = m_listLayers.GetAt(i);

            xarray<guid> lstGuidsInLayer;
            GetObjectsInLayer( eLayer.Name, lstGuidsInLayer );
            //add to export list
            for (s32 j = 0; j < lstGuidsInLayer.GetCount(); j++)
	        {
                lstGuids.Append(lstGuidsInLayer.GetAt(j));
            }

            xarray<editor_blueprint_ref> lstBPRef;
            if (GetBlueprintsInLayer( eLayer.Name, lstBPRef ))
            {
                for (s32 k=0; k < lstBPRef.GetCount(); k++)
                {
                    editor_blueprint_ref& BPRef = lstBPRef.GetAt(k);
                    for (j=0; j < BPRef.ObjectsInBlueprint.GetCount(); j++)
                    {
                        guid ObjGuid = BPRef.ObjectsInBlueprint.GetAt(j);
                        lstGuids.Append(ObjGuid);
                    }
                }
            }
        }
    }

    switch(iType)
    {
    case 1:
        lighting_LightObjects( PLATFORM_PC, lstGuids, LIGHTING_DIRECTIONAL );
        break;
    case 2:
        lighting_LightObjects( PLATFORM_PC, lstGuids, LIGHTING_DISTANCE );
        break;
    case 3:
        lighting_LightObjects( PLATFORM_PC, lstGuids, LIGHTING_DYNAMIC );
        break;
    case 4:
        lighting_LightObjects( PLATFORM_PC, lstGuids, LIGHTING_WHITE );
        break;
    case 5:
        lighting_LightObjects( PLATFORM_PC, lstGuids, LIGHTING_RAYCAST );
        break;
    case 6:
        lighting_LightObjects( PLATFORM_PC, lstGuids, LIGHTING_ZONE );
        break;

    default:
        ASSERT(FALSE);
        break;
    }

    if (x_strlen(pLayer)>0)
    {
        x_DebugMsg("--TIMECHECK-- WorldEditor::ComputeLightLayer(%s) %g sec.\n",
            pLayer, LoadTimer.TripSec());
    }
    else
    {
        x_DebugMsg("--TIMECHECK-- WorldEditor::ComputeLightLayer(ALL LAYERS) %g sec.\n",
            LoadTimer.TripSec());
    }

    x_catch_display;

    if (m_pHandler) m_pHandler->SetProgress( 0 );
}

//=========================================================================
// Updater functions
//=========================================================================

//this function will get a list of all matching surfaces using same geom
void world_editor::GetListOfMatchingGeoms( guid TemplateGuid, xarray<guid>& lstGuids )
{
    object *pTemplate = g_ObjMgr.GetObjectByGuid( TemplateGuid );
    if( pTemplate )
    {
        object::type ObjType = pTemplate->GetType();
        if ((ObjType == object::TYPE_PLAY_SURFACE) ||
            (ObjType == object::TYPE_PROP_SURFACE) ||
            (ObjType == object::TYPE_ANIM_SURFACE))
        {
            prop_query pq1;
            char cTemplateGeom[MAX_PATH];
            pq1.RQueryExternal( "RenderInst\\File", cTemplateGeom);
            if (pTemplate->OnProperty( pq1 ))
            {
                slot_id SlotID = g_ObjMgr.GetFirst( ObjType );
                while( SlotID != SLOT_NULL )
                {
                    object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
                    if( pObject != NULL )
                    {
                        if (pObject->GetGuid() != TemplateGuid)
                        {
                            //compare RigidGeom Property
                            prop_query pq2;
                            char cGeom[MAX_PATH];
                            pq2.RQueryExternal( "RenderInst\\File", cGeom);
                            if ( pObject->OnProperty( pq2 ) )
                            {
                                if (x_stricmp(cTemplateGeom,cGeom)==0)
                                {
                                    //these match
                                    lstGuids.Append(pObject->GetGuid());
                                }
                            }
                        }
                    }   
                    SlotID = g_ObjMgr.GetNext( SlotID );
                }   
            }
        }
    }
}

//=========================================================================

//this function will get a select of all matching surfaces using same geom
//count of objects selected, if 0 then the no objects match, not even the one you had
//selected
s32 world_editor::SelectMatchingGeomObjects( guid TemplateGuid )
{
    s32 nMatching = 0;

    ClearSelectedObjectList();
    object *pTemplate = g_ObjMgr.GetObjectByGuid( TemplateGuid );
    if( pTemplate )
    {
        prop_query pq1;
        char cTemplateGeom[MAX_PATH];
        pq1.RQueryExternal( "RenderInst\\File", cTemplateGeom);
        if (pTemplate->OnProperty( pq1 ))
        {
            //this object has a renderinst
            for (s32 i = 0; i < m_listLayers.GetCount(); i++)
            {
                editor_layer& eLayer = m_listLayers.GetAt(i);
                if (eLayer.IsEditable)
                {
                    xarray<guid> lstGuidsInLayer;
                    GetObjectsInLayer( eLayer.Name, lstGuidsInLayer );

                    for (s32 j = 0; j < lstGuidsInLayer.GetCount(); j++)
                    {
                        object *pObject = g_ObjMgr.GetObjectByGuid(lstGuidsInLayer.GetAt(j));
                        if( pObject != NULL )
                        {
                            //compare RigidGeom Property
                            prop_query pq2;
                            char cGeom[MAX_PATH];
                            pq2.RQueryExternal( "RenderInst\\File", cGeom);
                            if ( pObject->OnProperty( pq2 ) )
                            {
                                if (x_stricmp(cTemplateGeom,cGeom)==0)
                                {
                                    //these match, select this object
                                    InternalSelectObject(pObject->GetGuid(),FALSE,TRUE);
                                    nMatching++;
                                }
                            }
                        }  
                    }
                }
            }
        }
        else
        {
            nMatching = -1;
        }
    }

    return nMatching;
}

//=========================================================================

void world_editor::UpdateSelectedObjsWithBlueprint( const char* pTheme, const char* pRelPath )
{
    //setup undo
    SetCurrentUndoEntry(new transaction_entry(xfs("Replace Sel Objs with BP(%s)",pRelPath)));

    xarray<guid> lstObjectsToReplace = m_guidsSelectedObjs;
    ClearSelectedObjectList();

    for (s32 i = 0; i < lstObjectsToReplace.GetCount(); i++)
    {
        guid ObjGuid = lstObjectsToReplace.GetAt(i);
        object *pObject = g_ObjMgr.GetObjectByGuid( ObjGuid );
        if( pObject )
        {
            radian3 &r3 = pObject->GetL2W().GetRotation();
            vector3 &v3 = pObject->GetPosition();
            editor_layer& objLayer = FindObjectsLayer(ObjGuid, FALSE);
            xstring strPath = FindLayerPathForObject( ObjGuid, objLayer );

            DeleteObject( ObjGuid );

            //all of the following functions handle there respective undos
            editor_blueprint_ref BPNew;
            AddBlueprintToSpecificLayer(
                pTheme,
                pRelPath,
                objLayer.Name,
                strPath,
                BPNew,
                TRUE,
                TRUE,
                ObjGuid);
            RotateSelectedObjects( r3 );
            MoveBlueprintObjectsToAnchor( v3 );
        }    
    }

    //last part of undo
    CommitCurrentUndoEntry();  
}

//=========================================================================

void world_editor::UpdateListOfGeoms( guid TemplateGuid, xarray<guid>& lstGuids )
{
    SetCurrentUndoEntry(new transaction_entry(xfs("Batch updating %d Geom(s)",lstGuids.GetCount())));

    x_try;
    object *pTemplate = g_ObjMgr.GetObjectByGuid( TemplateGuid );
    if( pTemplate )
    {
        //for each playsurface
        play_surface &TemplateSurface = play_surface::GetSafeType( *pTemplate );
        rigid_inst& TemplateInst = TemplateSurface.GetRigidInst();
        xarray<prop_container> Containers;
        TemplateInst.OnCopy( Containers );

        for (s32 i=0; i<lstGuids.GetCount(); i++)
        {
            object *pObject = g_ObjMgr.GetObjectByGuid( lstGuids.GetAt(i) );
            if( pObject )
            {
                transaction_object_data* pObjData = NULL;
                //UNDO for object change before
                pObjData = new transaction_object_data(
                               transaction_object_data::OBJ_EDIT,
                               pObject->GetGuid(), pObject->GetTypeDesc().GetTypeName());
                pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);

                u32 OldBits = pObject->GetAttrBits();
                pObject->OnPaste( Containers );
                pObject->SetAttrBits(OldBits);

                //mark layer dirty
                SetObjectsLayerAsDirty(pObject->GetGuid());

                //UNDO for object change after
                pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
                m_pCurrentUndoEntry->AddCommitStep(pObjData);
            }
        }
    }
    x_catch_display;

    CommitCurrentUndoEntry();                
}

//=========================================================================

void world_editor::UpdateRigidInstances( xarray<guid>& lstGuids, const char* pNewRsc )
{
    SetCurrentUndoEntry(new transaction_entry(xfs("Batch updating %d Geom(s)",lstGuids.GetCount())));

    x_try;
    for (s32 i=0; i<lstGuids.GetCount(); i++)
    {
        object *pObject = g_ObjMgr.GetObjectByGuid( lstGuids.GetAt(i) );
        if( pObject )
        {

            transaction_object_data* pObjData = NULL;
            //UNDO for object change before
            pObjData = new transaction_object_data(
                           transaction_object_data::OBJ_EDIT,
                           pObject->GetGuid(), pObject->GetTypeDesc().GetTypeName());
            pObjData->StoreProperties(transaction_data::TRANSACTION_OLD_STATE);

            prop_query pq;
            pq.WQueryExternal( "RenderInst\\File", pNewRsc);
            pObject->OnProperty( pq );

            //mark layer dirty
            SetObjectsLayerAsDirty(pObject->GetGuid());

            //UNDO for object change after
            pObjData->StoreProperties(transaction_data::TRANSACTION_NEW_STATE);
            m_pCurrentUndoEntry->AddCommitStep(pObjData);
        }
    }
    x_catch_display;

    CommitCurrentUndoEntry(); 
}

//=========================================================================
// Guid Buffering
//=========================================================================

xbool world_editor::CaptureGuid( void )
{
    //only capture guid if one object is selected
    editor_blueprint_ref BPRef;
    if (m_guidsSelectedObjs.GetCount() == 1)
    {
        m_CapturedGuid = m_guidsSelectedObjs.GetAt(0);
        return TRUE;
    }
    else if (IsOneBlueprintSelected( BPRef ))
    {
        //yes, does it only have one object?
        if (BPRef.ObjectsInBlueprint.GetCount() == 1)
        {
            m_CapturedGuid = BPRef.ObjectsInBlueprint.GetAt(0);
            return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================

guid world_editor::GetCapturedGuid( void )
{
    return m_CapturedGuid;
}

//=========================================================================

guid world_editor::GetHoverSelectionGuid( void )
{
    return m_HoverSelectionGuid;
}

//=========================================================================

xbool world_editor::IsHoverSelectionABlueprint( void )
{
    return m_bHoverSelectionIsBlueprint;
}

//=========================================================================

xbool world_editor::IsInHoverSelectMode( void )
{
    return m_bInHoverSelectMode;
}

//=========================================================================

void world_editor::ClearHoverSelection( void )
{
    m_HoverSelectionGuid = guid(0);
    m_bHoverSelectionIsBlueprint = FALSE;
}

//=========================================================================

void world_editor::DoHoverSelect( const vector3& Start, const vector3& End, xbool bIncludeIcons )
{
    ClearHoverSelection();

    x_try;

    g_CollisionMgr.EditorSelectRay( Start, End, bIncludeIcons );
    if(g_CollisionMgr.m_nCollisions != 0)
    {
        guid ObjGuid = g_CollisionMgr.m_Collisions[0].ObjectHitGuid;

        object *pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);
        if (pObject) 
        {
            if (pObject->GetAttrBits() & object::ATTR_EDITOR_BLUE_PRINT)
            {
                //remove all blueprints items
                editor_blueprint_ref BlueprintReference;
                if (GetBlueprintRefContainingObject(ObjGuid, BlueprintReference))
                {
                    if (BlueprintReference.ObjectsInBlueprint.GetCount() == 1)
                    {
                        object *pBPObject = g_ObjMgr.GetObjectByGuid(BlueprintReference.ObjectsInBlueprint.GetAt(0));
                        //only select dynamic objects
                        if (pBPObject && pBPObject->GetTypeDesc().IsDynamic()) 
                        {
                            m_bHoverSelectionIsBlueprint = TRUE;
                            m_HoverSelectionGuid = pBPObject->GetGuid();
                        }
                    }
                }
            }
            else
            {
                //only select dynamic objects
                if (pObject->GetTypeDesc().IsDynamic())
                {
                    m_HoverSelectionGuid = ObjGuid;
                }
            }
        }
    }
    x_catch_display;
}

//=========================================================================

void world_editor::SetGuidSelectMode ( const char* pProperty, xbool bActivate, xbool bProperty )
{
    ClearHoverSelection();

    if (bActivate)
    {
        m_bInHoverSelectMode = TRUE;
        m_xstrHoverProperty.Format("%s",pProperty);
    }
    else
    {
        m_bInHoverSelectMode = FALSE;
        m_xstrHoverProperty.Clear();
    }
    m_bHoverSelectForProperty = bProperty;
}

//=========================================================================

xbool world_editor::IsGuidSelectMode ( void )
{
    return m_bInHoverSelectMode;
}

//=========================================================================

void world_editor::DoGuidSelect( void )
{
    if (m_bHoverSelectForProperty)
    {
        prop_query pqWrite;
        pqWrite.WQueryGUID(m_xstrHoverProperty, m_HoverSelectionGuid);
        OnProperty(pqWrite);
    }
    else
    {
        //not for properties, for globals instead
        xhandle rHandle;
        if (g_VarMgr.GetGuidHandle(m_xstrHoverProperty, &rHandle))
        {
            g_VarMgr.SetGuid(rHandle, m_HoverSelectionGuid);
            m_pHandler->RefreshGlobals();
        }
    }
}
void world_editor::TreeDoGuidSelect(guid ObjGuid)
{
    ClearHoverSelection();
 
    x_try;

    object *pObject = g_ObjMgr.GetObjectByGuid(ObjGuid);
    if (pObject) 
    {
        if (pObject->GetAttrBits() & object::ATTR_EDITOR_BLUE_PRINT)
        {
            //remove all blueprints items
            editor_blueprint_ref BlueprintReference;
            if (GetBlueprintRefContainingObject(ObjGuid, BlueprintReference))
            {
                if (BlueprintReference.ObjectsInBlueprint.GetCount() == 1)
                {
                    object *pBPObject = g_ObjMgr.GetObjectByGuid(BlueprintReference.ObjectsInBlueprint.GetAt(0));
                    //only select dynamic objects
                    if (pBPObject && pBPObject->GetTypeDesc().IsDynamic()) 
                    {
                        m_bHoverSelectionIsBlueprint = TRUE;
                        m_HoverSelectionGuid = pBPObject->GetGuid();
                        DoGuidSelect();
                        //now clear the hover select stuff
                        ClearHoverSelection();

                    }
                }
            }
        }
        else
        {
            //only select dynamic objects
            if (pObject->GetTypeDesc().IsDynamic())
            {
                m_HoverSelectionGuid = ObjGuid;
                DoGuidSelect();
                //now clear the hover select stuff
                ClearHoverSelection();

            }
        }
        SetGuidSelectMode("",FALSE);
    }
    x_catch_display;

}


//=========================================================================
// Reporting Options
//=========================================================================


struct TextureData {
    u16     uCount;
    xstring xstrName;
};

class CompareGeomsForSort : public x_compare_functor<const RigidGeomData&>
{
public:
    s32 operator()( const RigidGeomData& Geom1, const RigidGeomData& Geom2 )
    {
        if (Geom1.uCount > Geom2.uCount) return( -1 );
        if (Geom1.uCount < Geom2.uCount) return( 1 );   
        else return(  0 );
    }
};

class CompareTexturesForSort : public x_compare_functor<const TextureData&>
{
public:
    s32 operator()( const TextureData& Tex1, const TextureData& Tex2 )
    {
        if (Tex1.uCount > Tex2.uCount) return( -1 );
        if (Tex1.uCount < Tex2.uCount) return(  1 );   
        else return(  0 );
    }
};

//=========================================================================

void world_editor::WriteRigidGeomReport( const char* pFile, xarray<xstring> &lstRigidGeoms )
{
    //convert the list to new array
    xarray<RigidGeomData> RigidGeoms;
    xarray<TextureData> Textures;
    for( s32 i = 0; i < lstRigidGeoms.GetCount(); i++)
    {
        RigidGeomData rgd;
        rgd.uCount = 0;
        rgd.bRegistered = TRUE;
        rgd.xstrName = xstring(lstRigidGeoms.GetAt(i));
        rgd.nVerts = rgd.nPolys = rgd.nMaterials = rgd.nTextures = rgd.nMeshes = rgd.nSubMeshes = -1;
        RigidGeoms.Append(rgd);
    }

    for( i = 0; i < object::TYPE_END_OF_LIST; i++ )
    {
        for( slot_id SlotID = g_ObjMgr.GetFirst( (object::type)i ); 
                     SlotID != SLOT_NULL; 
                     SlotID = g_ObjMgr.GetNext( SlotID ) )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );

            if( pObject == NULL )
                continue;

            if (!(pObject->GetTypeDesc().QuickResourceName()))
                continue;

            if( x_strcmp(pObject->GetTypeDesc().QuickResourceName(),"RigidGeom") != 0 )
                continue;

            //handle texture usage
            rigid_inst* pRigidInst = GetRigidInstForObject(pObject);
            rigid_geom* pGeom = NULL;
            if (pRigidInst)
            {
                pGeom = pRigidInst->GetRigidGeom();
            }

            //so we've got a rigidgeom
            prop_query pqRead;
            char cName[MAX_PATH];
            pqRead.RQueryExternal( pObject->GetTypeDesc().QuickResourcePropertyName(), &cName[0]);
            pObject->OnProperty( pqRead );

            //handle rigidgeom data
            xbool bFound = FALSE;
            for( s32 j = 0; j < RigidGeoms.GetCount(); j++)
            {
                RigidGeomData& rgd = RigidGeoms.GetAt(j);
                if (x_strcmp(rgd.xstrName, cName)==0)
                {
                    rgd.uCount++;

                    if (pGeom)
                    {
                        if (rgd.uCount == 1)
                        {
                            rgd.nVerts     = pGeom->GetNVerts();
                            rgd.nPolys     = pGeom->GetNFaces();
                            rgd.nMaterials = pGeom->m_nMaterials;
                            rgd.nTextures  = pGeom->m_nTextures;
                            rgd.nMeshes    = pGeom->m_nMeshes;
                            rgd.nSubMeshes = pGeom->m_nSubMeshes;
                        }
                    }
                    bFound = TRUE;
                }
            }

            if (!bFound)
            {
                //append new one
                RigidGeomData rgd;
                rgd.uCount = 1;
                rgd.bRegistered = FALSE;
                rgd.xstrName = xstring(cName);
                if (pGeom)
                {
                    rgd.nVerts     = pGeom->GetNVerts();
                    rgd.nPolys     = pGeom->GetNFaces();
                    rgd.nMaterials = pGeom->m_nMaterials;
                    rgd.nTextures  = pGeom->m_nTextures;
                    rgd.nMeshes    = pGeom->m_nMeshes;
                    rgd.nSubMeshes = pGeom->m_nSubMeshes;
                }
                else
                {
                    rgd.nVerts = rgd.nPolys = rgd.nMaterials = rgd.nTextures = rgd.nMeshes = rgd.nSubMeshes = -1;
                }

                RigidGeoms.Append(rgd);
            }

            if (pRigidInst)
            {
                geom* pGeom = pRigidInst->GetGeom();
                if ( pGeom )
                {
                    for ( j = 0; j < pGeom->m_nTextures; j++)
                    {
                        xbool bFoundTex = FALSE;
                        const char* pName = pGeom->GetTextureName( j );
                        for( s32 k = 0; k < Textures.GetCount(); k++)
                        {
                            TextureData& td = Textures.GetAt(k);
                            if (x_strcmp(td.xstrName, pName)==0)
                            {
                                td.uCount++;
                                bFoundTex = TRUE;
                            }
                        }

                        if (!bFoundTex)
                        {
                            TextureData td;
                            td.uCount = 1;
                            td.xstrName = xstring(pName);
                            Textures.Append(td);
                        }
                    }
                }
            }            
        }   
    }  

    //sort data
    x_qsort( &Textures[0], Textures.GetCount(), CompareTexturesForSort() );
    x_qsort( &RigidGeoms[0], RigidGeoms.GetCount(), CompareGeomsForSort() );

    // Check if the report file is available for writing to
    FILE* FH = fopen( pFile, "wb" );
    if( FH )
    {
        fclose( FH );

        text_out Report;
        Report.OpenFile( pFile );

        Report.AddHeader("RigidGeom Usage Data",RigidGeoms.GetCount());

        for ( i = 0; i < RigidGeoms.GetCount(); i++)
        {
            RigidGeomData& rgd = RigidGeoms.GetAt(i);
            Report.AddString    ("Name",        rgd.xstrName);
            Report.AddS32       ("UsageCount",  rgd.uCount);
            Report.AddBool      ("Registered",  rgd.bRegistered);
            Report.AddS32       ("Verts",       rgd.nVerts);
            Report.AddS32       ("Polys",       rgd.nPolys);
            Report.AddS32       ("Materials",   rgd.nMaterials);
            Report.AddS32       ("Textures",    rgd.nTextures);
            Report.AddS32       ("Meshes",      rgd.nMeshes);
            Report.AddS32       ("SubMeshes",   rgd.nSubMeshes);
            Report.AddEndLine();
        }

        Report.AddHeader("Texture Usage Data",Textures.GetCount());


        for ( i = 0; i < Textures.GetCount(); i++)
        {
            TextureData& td = Textures.GetAt(i);
            Report.AddString    ("Name",        td.xstrName);
            Report.AddS32       ("UsageCount",  td.uCount);
            
            //x_fread
            xstring xstrOutputFile;
            xstrOutputFile.Format( "%s\\PS2\\%s", g_Settings.GetReleasePath(), (const char*)td.xstrName );
            X_FILE* pFile = x_fopen(xstrOutputFile,"rb");
            if (pFile)
            {
                Report.AddS32("PS2_FileSize", x_flength(pFile));
                x_fclose(pFile);
            }
            else
            {
                Report.AddS32("PS2_FileSize", -1);
            }

            Report.AddEndLine();
        }

        Report.CloseFile();
    }
    else
    {
        generic_dialog Dialog;
        Dialog.Execute_OK( "Can't open file for write, report will be invalid!", pFile );
    }
}

//=========================================================================

s32 world_editor::GetRigidGeomUseCount( const char* pRsc )
{
    s32 nUsageCount = 0;
    for( s32 i=0; i<object::TYPE_END_OF_LIST; i++ )
    {
        for( slot_id SlotID = g_ObjMgr.GetFirst( (object::type)i ); 
                     SlotID != SLOT_NULL; 
                     SlotID = g_ObjMgr.GetNext( SlotID ) )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );

            if( pObject == NULL )
                continue;

            if (!(pObject->GetTypeDesc().QuickResourceName()))
                continue;

            if( x_strcmp(pObject->GetTypeDesc().QuickResourceName(),"RigidGeom") != 0 )
                continue;

            prop_query pqRead;
            char cName[MAX_PATH];
            pqRead.RQueryExternal( pObject->GetTypeDesc().QuickResourcePropertyName(), &cName[0]);
            pObject->OnProperty( pqRead );

            if (x_stricmp(cName, pRsc)==0)
            {
                nUsageCount++;
            }
        }   
    }

    return nUsageCount;
}

//=========================================================================

xbool  world_editor::HideUnselectedObjects ( void )
{
    //for every rednerable object turn off their render flag 
    //for every selected object turn it back on..

    x_try;

    g_ObjMgr.SelectByAttribute( object::ATTR_RENDERABLE );

    select_slot_iterator SlotIter;

    //for all play surfaces in our query volume, send it to the clipper
    for( SlotIter.Begin();  !SlotIter.AtEnd();  SlotIter.Next() )
    {
        object* pObject = SlotIter.Get();
        pObject->SetAttrBits(pObject->GetAttrBits() & ~object::ATTR_RENDERABLE);
        m_HiddenSet.Append( pObject->GetGuid() );
    }
    
    SlotIter.End();

    // Loop through all selected objects
    for( s32 i=0; i < m_guidsSelectedObjs.GetCount(); i++ )
    {       
        object *pObject = g_ObjMgr.GetObjectByGuid( m_guidsSelectedObjs.GetAt(i) );
         pObject->SetAttrBits(pObject->GetAttrBits() | object::ATTR_RENDERABLE);
    }

    x_catch_display;

    return TRUE;
}

//=========================================================================

xbool  world_editor::UnHideObjects ( void )
{
    x_try;

    // Loop through all selected objects
    for( s32 i=0; i < m_HiddenSet.GetCount(); i++ )
    {       
        object *pObject = g_ObjMgr.GetObjectByGuid( m_HiddenSet.GetAt(i) );
        pObject->SetAttrBits(pObject->GetAttrBits() | object::ATTR_RENDERABLE);
    }

    m_HiddenSet.Clear();
    
    x_catch_display;


    return TRUE;
}

//=========================================================================

void world_editor::ComputeSoundPropagation( void )
{
#ifdef sansari
    for( s32 i = 0; i < g_ZoneMgr.GetZoneCount(); i++ )
    {
        const zone_mgr::zone& Zone = g_ZoneMgr.GetZone( i );
    }
#endif
}

//=============================================================================
//
//  CreateResourceList
//      
//      - Creates a list of resources that need to be packed into the load file
//
//      The current list of file types includes
//              .ANIM
//              .FXD
//              .FXO
//              .NEW
//              .RIGIDCOLOR
//              .RIGIDGEOM
//              .SKINGEOM
//              .XBMP
//              .TEMPLATES
//              .TMPL_DCT
//              .ZONE
//                
//  
//=============================================================================

struct quick_string
{
    enum
    {
        FLAGS_NULL                = 0,
        FLAGS_ALREADY_CHECKED     = (1<<0),
        FLAGS_EXTERNAL_DEPENDENCY = (1<<1),
        FLAGS_DONT_EXPORT         = (1<<2),
    };

    char    String[256];
    u32     Flags;
    static s32 Compare( const void* pItem1, const void* pItem2 )
    {
        return x_stricmp( ((quick_string*)pItem1)->String, ((quick_string*)pItem2)->String );
    }
};

//=========================================================================

xbool DoesRescDescExist( const char* pRescName )
{
    xbool bExist = TRUE;

    x_try;

    rsc_desc& RescDesc = g_RescDescMGR.GetRscDescByString( pRescName );

    x_catch_begin;

        bExist = FALSE;

    x_catch_end;

    return bExist;
}

//=========================================================================
static
void ProcessObject( 
    object*                 pObject, 
    xarray<quick_string>&   ResourceList,
    xarray<quick_string>&   TemplateList,
    s32                     iPlatform)
{
    ASSERT( pObject );

    //
    // Go throw all the properties and find the resources
    //
    prop_enum  EnumList;
    prop_query propQuery;

    // Enum the object properties
    pObject->OnEnumProp( EnumList );

    for( s32 i=0; i<EnumList.GetCount(); i++ )
    {
        prop_enum::node& Node = EnumList[i];

        //
        // Filter any property that we don't care
        //
        if ((Node.GetType() & PROP_TYPE_BASIC_MASK) ==  PROP_TYPE_EXTERNAL )
        {
            // If it is not a resource lets move on
            if( x_stristr( Node.GetEnumType(0), "Resource" ) == NULL )
                continue;
            
            //
            // Read the property
            //
            quick_string QS;

            // Make sure to clear this parameter
            QS.Flags = quick_string::FLAGS_NULL;

            propQuery.RQueryExternal( Node.GetName(), QS.String );
            pObject->OnProperty( propQuery );

            // This property has not been fill in
            if( QS.String[0] == 0 )
                continue;

            //
            // Add the string once and only once into our list
            //
            if( ResourceList.GetCount() > 0)
            {
                void* pLocation = NULL;
                if( x_bsearch ( &QS, &ResourceList[0],  ResourceList.GetCount(),       
                                sizeof(quick_string), quick_string::Compare, pLocation ) == NULL ) 
                {
                    s32 Index = (s32)((quick_string*)pLocation - &ResourceList[0]);

                    ResourceList.Insert(Index) = QS;
                }                
            }
            else
            {
                ResourceList.Append() = QS;
            }

            //
            // Check if we have a rescdesc for this resource
            //
            if( !DoesRescDescExist( QS.String ) )
            {

                const char* pReleasePath = g_Settings.GetReleasePath();
                const char* pPlatformString = g_Settings.GetPlatformStringI( iPlatform );

                xbool bReportError = TRUE;
                _finddata_t     fileinfo;
                long            hFile = _findfirst( xfs( "%s\\%s\\%s",pReleasePath,pPlatformString,QS.String ), &fileinfo );
                if( hFile != -1L )
                {
                    // If the file is read-only then we assume it is from P4
                    if( fileinfo.attrib & _A_RDONLY )
                        bReportError = FALSE;
                }

                if( bReportError )
                {
                    char Str[256];
                    x_sprintf(Str,"<%s> %08X:%08X missing rescdesc for direct dependency\n(%s)\n",
                        pObject->GetTypeDesc().GetTypeName(),
                        (u32)(pObject->GetGuid()>>32),(u32)pObject->GetGuid(),
                        QS.String);

                    LOG_ERROR("Export",xfs("Object (%s) %08X:%08X is missing rescdesc for direct dependency\n(%s)\n",
                        pObject->GetTypeDesc().GetTypeName(),
                        (u32)(pObject->GetGuid()>>32),(u32)pObject->GetGuid(),
                        QS.String));
                }
            }

        }
        else if( (Node.GetType() & PROP_TYPE_BASIC_MASK) ==  PROP_TYPE_FILENAME ) 
        {
            if( x_stristr( Node.GetEnumType(0), "*.bpx" ) )
            {
                //
                // Read the property
                //
                quick_string QS;

                // Make sure to clear this parameter
                QS.Flags = quick_string::FLAGS_NULL;

                propQuery.RQueryFileName( Node.GetName(), QS.String );
                pObject->OnProperty( propQuery );

                // This property has not been fill in
                if( QS.String[0] == 0 )
                    continue;

                //
                // Add the string ones and only ones into our list
                //
                if( TemplateList.GetCount() > 0)
                {
                    void* pLocation = NULL;
                    if( x_bsearch ( &QS, &TemplateList[0],  TemplateList.GetCount(),       
                                    sizeof(quick_string), quick_string::Compare, pLocation ) == NULL ) 
                    {
                        s32 Index = (s32)((quick_string*)pLocation - &TemplateList[0]);

                        TemplateList.Insert(Index) = QS;
                    }                
                    else
                    {
                        // We are done
                        continue;
                    }
                }
                else
                {
                    TemplateList.Append() = QS;
                }

                //
                // Get all the info out of the template
                //
                guid ObjectGuid = g_TemplateMgr.EditorCreateSingleTemplateFromPath( QS.String, vector3(0,0,0), radian3(0,0,0), -1, -1 ); 

                object* pTemplateObject = g_ObjMgr.GetObjectByGuid( ObjectGuid );

                // mreed: if we don't have a template object, we are no longer asserting as if it's an error
                // the reason is that the template manager will not be returning a pointer to a player,
                // since we now just create players through the object manager, not templates.
                if( pTemplateObject )
                {
                    ProcessObject( pTemplateObject, ResourceList, TemplateList, iPlatform );
                    g_ObjMgr.DestroyObjectEx( ObjectGuid, TRUE );
                }
            }
        }
    }
}

//=========================================================================

void world_editor::CreateResourceLoadList( 
    const xarray<guid>& lstGuidsToExport, 
    const xarray<guid>& lstPlaySurfaces, 
    const xarray<guid>& lstDecals,
    const char* fileName,
          s32   iPlatform)
{
    X_FILE*         outFile;
    s32             i;
    const char*     pPlatformString = g_Settings.GetPlatformStringI( iPlatform );
    platform        PlatformType    = g_Settings.GetPlatformTypeI  ( iPlatform );

    outFile = x_fopen(fileName, "wt" ); 
    if(outFile == NULL )
    {
        x_DebugMsg( "Could not open file \"%s\".  Could be write protected\n" );
        return;
    }
    
    //
    // Check all the resources from the object data base
    //
    xarray<quick_string>    ResourceList;
    xarray<quick_string>    TemplateList;
    
    for( i = 0; i < lstGuidsToExport.GetCount(); i++ )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( lstGuidsToExport[i] );
        
        //
        //  Make sure we got a valid object and if not, let them know since
        //  an object in the list is a bug that should be tracked down
        //
        if( pObject == NULL )
        {
            x_DebugMsg( "WARNING:  Invalid object GUID:%X:%X", 
                            lstGuidsToExport[i].GetHigh, lstGuidsToExport[i].GetLow() );
            return;
        }

        ProcessObject( pObject, ResourceList, TemplateList, iPlatform );
    }   

    // Do the play surfaces special
    for( i = 0; i < lstPlaySurfaces.GetCount(); i++ )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( lstPlaySurfaces[i] );
        
        //
        //  Make sure we got a valid object and if not, let them know since
        //  an object in the list is a bug that should be tracked down
        //
        if( pObject == NULL )
        {
            x_DebugMsg( "WARNING:  Invalid object GUID:%X:%X", 
                            lstPlaySurfaces[i].GetHigh, lstPlaySurfaces[i].GetLow() );
            return;
        }

        ProcessObject( pObject, ResourceList, TemplateList, iPlatform );
    }   

    // Do the decals special
    for( i = 0; i < lstDecals.GetCount(); i++ )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( lstDecals[i] );
        
        //
        //  Make sure we got a valid object and if not, let them know since
        //  an object in the list is a bug that should be tracked down
        //
        if( pObject == NULL )
        {
            x_DebugMsg( "WARNING:  Invalid object GUID:%X:%X", 
                            lstDecals[i].GetHigh, lstDecals[i].GetLow() );
            return;
        }

        ProcessObject( pObject, ResourceList, TemplateList, iPlatform );
    }   

    // handle resources required by the zone manager
    for (i = 0; i < m_listZones.GetCount(); i++)
	{
        editor_zone_ref &Zone = m_listZones.GetAt(i);
        if ( Zone.FogMap[0] == '\0' )
            continue;

        quick_string    QS;
        QS.Flags = quick_string::FLAGS_NULL;
        x_strcpy( QS.String, Zone.FogMap );

        //
        // Add the string once and only once into our list
        //
        if( ResourceList.GetCount() > 0)
        {
            void* pLocation = NULL;
            if( x_bsearch ( &QS, &ResourceList[0],  ResourceList.GetCount(),       
                            sizeof(quick_string), quick_string::Compare, pLocation ) == NULL ) 
            {
                s32 Index = (s32)((quick_string*)pLocation - &ResourceList[0]);

                ResourceList.Insert(Index) = QS;
            }                
        }
        else
        {
            ResourceList.Append() = QS;
        }
    }

    //
    // Build the list of secondary dependencies
    //
    xarray<quick_string>    DepList;

    //
    // Finally go through all the resources and get their final dependencies
    //
    for( i=0; i<ResourceList.GetCount(); i++ )
    {
        if( ResourceList[i].Flags & quick_string::FLAGS_ALREADY_CHECKED )
            continue;

        if( ResourceList[i].Flags & quick_string::FLAGS_DONT_EXPORT )
            continue;

        // Make sure to mark this dependency as already check so we know to skip it next time
        ResourceList[i].Flags |= quick_string::FLAGS_ALREADY_CHECKED;

        //
        // Get the resource desc for that type
        //
        x_try;

        rsc_desc& RescDesc = g_RescDescMGR.GetRscDescByString( ResourceList[i].String );

        //
        // Get any external dependency
        //
        xarray<xstring> TempDepList;
        RescDesc.OnGetFinalDependencies( TempDepList, PlatformType, xfs( "%s\\%s",g_Settings.GetReleasePath(),pPlatformString) );

        //
        // Append all the new dependencies
        //
        for( s32 j=0; j<TempDepList.GetCount(); j++ )
        {
            quick_string QS;
            void*        pLocation = NULL;

            x_strcpy(QS.String,(const char*)(TempDepList[j]));

            // Make sure to clear this parameter
            QS.Flags = quick_string::FLAGS_EXTERNAL_DEPENDENCY;


            xbool bFound = TRUE;
            if( DepList.GetCount() > 0 )
            {
                if( x_bsearch( &QS, &DepList[0],  DepList.GetCount(),       
                               sizeof(quick_string), quick_string::Compare, pLocation ) == NULL ) 
                {
                    s32 Index = (s32)((quick_string*)pLocation - &DepList[0]);
                    DepList.Insert(Index) = QS;
                    bFound = FALSE;
                }                
            }
            else
            {
                DepList.Append( QS );
                bFound = FALSE;
            }


            if( bFound == FALSE )
            {
                //
                // Added into the resource if we can
                //
                

                if( DoesRescDescExist( QS.String ) )
                {
                    // Add dependency to final resource list
                    LOG_MESSAGE("Export",xfs("Found resource description\n(%s)\n",QS.String));
                }
                else
                {
                    // We don't have a rescdesc that know's how to build this!
                   //LOG_WARNING("Export",xfs("Missing rescdesc for external dependency\n(%s)\n",QS.String));
                }

                void* pLocation = NULL;
                if( x_bsearch ( &QS, &ResourceList[0],  ResourceList.GetCount(),       
                                sizeof(quick_string), quick_string::Compare, pLocation ) == NULL ) 
                {
                    s32 Index = (s32)((quick_string*)pLocation - &ResourceList[0]);

                    ResourceList.Insert(Index) = QS;
                }                

            }
        }

        //
        // Okay Reset i to zero and try all over again
        //
        i = -1;

        //
        // Handle errors
        //
        x_catch_begin;

            if( ResourceList[i].Flags & quick_string::FLAGS_EXTERNAL_DEPENDENCY )
            {
                // We will assume that the resource manager doesn't need to know about this ones
            }
            else
            {
                // Mark as there is a problem so we dont export
                ResourceList[i].Flags |= quick_string::FLAGS_DONT_EXPORT;
                //x_DebugMsg( 0, xfs("WARNING: %s\n", xExceptionGetErrorString() ) );
                //LOG_WARNING("Export",xfs("%s\n", xExceptionGetErrorString() ) );
            }

            continue;

        x_catch_end;
    }

    //
    // Scan dependencies for resources we don't know how to build
    //
    {
        for( i=0; i<DepList.GetCount(); i++ )
        {
            quick_string DQS = DepList[i];

            if( DoesRescDescExist( DQS.String ) )
            {
                // Add dependency to final resource list
                void* pLocation = NULL;
                if( x_bsearch ( &DQS, &ResourceList[0],  ResourceList.GetCount(),       
                                sizeof(quick_string), quick_string::Compare, pLocation ) == NULL ) 
                {
                    s32 Index = (s32)((quick_string*)pLocation - &ResourceList[0]);

                    ResourceList.Insert(Index) = DQS;
                }                
            }
            else
            {
                // We don't have a rescdesc that know's how to build this!
                //LOG_WARNING("Export",xfs("Missing rescdesc for external dependency\n(%s)\n",DQS.String));
            }
        }
    }

    //
    // Output all the templates for the user
    //
    x_DebugMsg( 0, "TEMPLATES USED\n" );
    for( i=0; i<TemplateList.GetCount(); i++ )
    {
        x_DebugMsg( 0, xfs("Template   : %s\n", TemplateList[i].String) );
    }

    //
    // Save out all the dependencies
    //
    x_DebugMsg( 0, "EXPORT DEPENDENCIES\n" );
    for( i=0; i<ResourceList.GetCount(); i++ )
    {
        if( !(ResourceList[i].Flags & quick_string::FLAGS_ALREADY_CHECKED) )
        {
            continue;
        }

        if( ResourceList[i].Flags & quick_string::FLAGS_DONT_EXPORT )
        {
            x_DebugMsg( 0, xfs("Error   : %s\n", ResourceList[i].String) );
            continue;
        }

        x_fprintf( outFile, "%s\n", ResourceList[i].String );

        if( ResourceList[i].Flags & quick_string::FLAGS_EXTERNAL_DEPENDENCY )
        {
            x_DebugMsg( 0, xfs("External: %s\n", ResourceList[i].String) );
        }
        else
        {
            x_DebugMsg( 0, xfs("Internal: %s\n", ResourceList[i].String) );
        }
    }

    x_fclose( outFile );
}

//=========================================================================

void world_editor::ReverseGuidLookup( void )
{
    m_GuidLookupList.Clear();
    m_ReverseLookupGuid = m_TempPropGuid;
    for( object_desc* pNode = object_desc::GetFirstType(); 
                      pNode; 
                      pNode = object_desc::GetNextType( pNode )  )
    {
        if( pNode->TargetsOtherObjects())
        {
            for( slot_id SlotID = g_ObjMgr.GetFirst( pNode->GetType() ); 
                         SlotID != SLOT_NULL; 
                         SlotID = g_ObjMgr.GetNext( SlotID ) )
            {
                object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
                if( !pObject )
                    continue;

                if (x_strcmp(pNode->GetTypeName(),pObject->GetTypeDesc().GetTypeName()) != 0)
                    continue;

                if ( pObject->GetGuid() == m_ReverseLookupGuid ) //don't check self
                    continue;

                prop_enum List;
                pObject->OnEnumProp( List );
                for( s32 i = 0; i < List.GetCount(); i++)
                {
                    prop_enum::node& enData = List[i];

                    if ((enData.GetType() & PROP_TYPE_BASIC_MASK) == PROP_TYPE_GUID )
                    {
                        //its a guid
                        guid Guid;
                        prop_query pq;
                        pq.RQueryGUID( enData.GetName(), Guid ); 
                        if (pObject->OnProperty( pq ) )
                        {
                            if ( Guid == m_ReverseLookupGuid )
                            {
                                m_GuidLookupList.Append(pObject->GetGuid());
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

//=========================================================================

void world_editor::ReverseGlobalLookup( const char* pGlobal )
{
    m_GuidLookupList.Clear();
    m_ReverseLookupGuid = 0;
    for( object_desc* pNode = object_desc::GetFirstType(); 
                      pNode; 
                      pNode = object_desc::GetNextType( pNode )  )
    {
        if( pNode->TargetsOtherObjects())
        {
            for( slot_id SlotID = g_ObjMgr.GetFirst( pNode->GetType() ); 
                         SlotID != SLOT_NULL; 
                         SlotID = g_ObjMgr.GetNext( SlotID ) )
            {
                object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
                if( !pObject )
                    continue;

                if (x_strcmp(pNode->GetTypeName(),pObject->GetTypeDesc().GetTypeName()) != 0)
                    continue;

                prop_enum List;
                pObject->OnEnumProp( List );
                for( s32 i = 0; i < List.GetCount(); i++)
                {
                    prop_enum::node& enData = List[i];

                    if ((enData.GetType() & PROP_TYPE_BASIC_MASK) == PROP_TYPE_EXTERNAL )
                    {
                        //its an external
                        if (enData.GetEnumCount() >= 2)
                        {
                            if ( (x_strcmp("global_all", xstring(enData.GetEnumType(1)) ) == 0) ||
                                 (x_strcmp("global_guid", xstring(enData.GetEnumType(1)) ) == 0) )
                            {
                                //its a global
                                char       Temp[MAX_PATH];
                                prop_query pq;
                                pq.RQueryExternal( enData.GetName(), Temp ); 
                                if (pObject->OnProperty( pq ) )
                                {
                                    if (x_strcmp(Temp, pGlobal) == 0)
                                    {
                                        m_GuidLookupList.Append(pObject->GetGuid());
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

//=========================================================================

guid world_editor::GetLookupGuid( void )
{
    return m_ReverseLookupGuid;
}

//=========================================================================

void world_editor::ClearGuidLookupList( void )
{
    m_ReverseLookupGuid = 0;
    m_GuidLookupList.Clear();
}

//=========================================================================

xarray<guid>& world_editor::GetGuidLookupList( void )
{
    return m_GuidLookupList;
}

//=========================================================================

xbool world_editor::CanPerformReverseLookup( void )
{
    return (m_TempPropGuid != 0);
}

//=========================================================================
// GLOBALS
//=========================================================================

xbool world_editor::IsGlobalInUse(const char* pGlobal)
{
    for( object_desc* pNode = object_desc::GetFirstType(); 
                      pNode; 
                      pNode = object_desc::GetNextType( pNode )  )
    {
        if( pNode->IsDynamic())
        {
            for( slot_id SlotID = g_ObjMgr.GetFirst( pNode->GetType() ); 
                         SlotID != SLOT_NULL; 
                         SlotID = g_ObjMgr.GetNext( SlotID ) )
            {
                object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
                if( !pObject )
                    continue;

                if (x_strcmp(pNode->GetTypeName(),pObject->GetTypeDesc().GetTypeName()) != 0)
                    continue;

                prop_enum List;
                pObject->OnEnumProp( List );
                for( s32 i = 0; i < List.GetCount(); i++)
                {
                    prop_enum::node& enData = List[i];

                    if ((enData.GetType() & PROP_TYPE_BASIC_MASK) ==  PROP_TYPE_EXTERNAL )
                    {
                        if (enData.GetEnumCount() >= 1)
                        {
                            const char* pType = enData.GetEnumType(0);
                            if (x_strcmp(pType, "global") == 0)
                            {
                                //it is a global
                                char       Temp[MAX_PATH];
                                prop_query pq;
                                pq.RQueryExternal( enData.GetName(), Temp ); 
                                if (pObject->OnProperty( pq ) )
                                {
                                    if (x_strcmp(Temp, pGlobal) == 0)
                                    {
                                        InternalSelectObject(pObject->GetGuid(), TRUE,TRUE);
                                        return TRUE;
                                    }
                                }
                            }
                        }
                    
                    }
                }
            }   
        }  
    }
                      
    return FALSE;
}

//=========================================================================
// Automated Process
//=========================================================================

void world_editor::SetAutomatedBuildParams( const char* pLighting, const char* pExportName, xbool bAutoQuit, xbool bOldBreakVal )
{
    m_bAutoBuilding     = TRUE;
    m_bForceShutdown    = bAutoQuit;
    m_ExportName        = pExportName;
    m_LightingType      = pLighting;
    m_bOldBreakVal      = bOldBreakVal;
}

//=========================================================================

//return TRUE to quit editor
xbool world_editor::UpdateAutomatedBuild( void )
{
    static xbool bCompiled = FALSE;
    static xbool bLit = FALSE;
    static xbool bExported = FALSE;

    if( g_RescDescMGR.IsCompiling() )
        return FALSE;
    
    //waiting for a refresh
    if (ShouldRscRefresh())
        return FALSE;

    if( !bCompiled )
    {
        LOG_MESSAGE("AutoBuild","AUTOBUILD_COMPILE_FINISHED");
        LOG_MESSAGE("AutoBuild","AUTOBUILD_LIGHTING_STARTING");
        bCompiled = TRUE;
    }

    if( !m_LightingType.IsEmpty() )
    {
        if (x_stricmp(m_LightingType,"raycast") == 0)
        {
            g_WorldEditor.ComputeLightLayer("", 5);
        }
        else if (x_stricmp(m_LightingType,"distance") == 0)
        {
            g_WorldEditor.ComputeLightLayer("", 2);
        }
        else if (x_stricmp(m_LightingType,"directional") == 0)
        {
            g_WorldEditor.ComputeLightLayer("", 1);
        }
        else //default to fullbright
        {
            g_WorldEditor.ComputeLightLayer("", 4);
        }

    }

    if( !bLit )
    {
        LOG_MESSAGE("AutoBuild","AUTOBUILD_LIGHTING_FINISHED");
        LOG_MESSAGE("AutoBuild","AUTOBUILD_EXPORT_STARTING");
        bLit = TRUE;
    }

    if( !m_ExportName.IsEmpty() )
    {
        m_ExportName += ".Level";
    	if( g_WorldEditor.ExportToLevel(m_ExportName) )
        {
            LOG_MESSAGE("AutoBuild","AUTOBUILD_EXPORT_FINISHED");
        }
    }  

    if( !bExported )
    {
        bExported = TRUE;
    }

    g_bErrorBreak   = m_bOldBreakVal;
    m_bAutoBuilding = FALSE;
    g_bAutoLoad     = FALSE;


    return TRUE;
    //if( m_bForceShutdown )
        //return TRUE;
    //else
        //return FALSE;
}

//=========================================================================

xbool world_editor::IsPerformingAutomatedBuild( void )
{
    return m_bAutoBuilding;
}

//=========================================================================

rigid_inst* world_editor::GetRigidInstForObject( object* pObject )
{
    rigid_inst* pRigidInst = NULL;

    if (!pObject)
        return pRigidInst;
    
    if( pObject->IsKindOf( play_surface::GetRTTI() ) ||
        pObject->IsKindOf( anim_surface::GetRTTI() ) ||
        pObject->IsKindOf( prop_surface::GetRTTI() ) ||
        pObject->IsKindOf( team_prop::GetRTTI()    ) ||
        pObject->IsKindOf( super_destructible_obj::GetRTTI() ) )
    {
        play_surface* pSurface = &play_surface::GetSafeType( *pObject );
        pRigidInst = &pSurface->GetRigidInst();
    }
    else if(pObject->IsKindOf( pickup::GetRTTI() ) )
    {
        pickup* pSurface = &pickup::GetSafeType( *pObject );
        pRigidInst = &pSurface->GetRigidInst();
    }
    else if(pObject->IsKindOf( door::GetRTTI() ) )
    {
        door* pSurface = &door::GetSafeType( *pObject );
        pRigidInst = &pSurface->GetRigidInst();
    }
    else if ( pObject->IsKindOf( cloth_object::GetRTTI() ) )
    {
        cloth_object* pClothObject = &cloth_object::GetSafeType( *pObject );
        pRigidInst = &pClothObject->GetRigidInst();
    }
    else if( pObject->IsKindOf( new_weapon::GetRTTI() ) )
    {
        new_weapon* pWeapon = &new_weapon::GetSafeType( *pObject );
        pRigidInst = &pWeapon->GetRigidInst();
    }
    else if( pObject->IsKindOf( lore_object::GetRTTI() ) )
    {
        lore_object* pLore = &lore_object::GetSafeType( *pObject );
        pRigidInst = &pLore->GetRigidInst();
    }

    return pRigidInst;
}

//=========================================================================

void world_editor::PrintSelectedRigidInstanceInfo( void )
{
    s32 nVerts = 0;
    s32 nFaces = 0;
    s32 nMaterials = 0;
    s32 nTextures = 0;
    s32 nMeshes = 0;
    s32 nSubMeshes = 0;

    for (s32 i = 0; i < m_guidsSelectedObjs.GetCount(); i++)
    {
        //get stats for objects only
        object* pObject = g_ObjMgr.GetObjectByGuid( m_guidsSelectedObjs.GetAt(i) );
        if (pObject)
        {
            rigid_inst* pRigidInst = GetRigidInstForObject(pObject);
            if (pRigidInst)
            {
                rigid_geom* pGeom = pRigidInst->GetRigidGeom();
                if (pGeom)
                {
                    nVerts      += pGeom->GetNVerts();
                    nFaces      += pGeom->GetNFaces();
                    nMaterials  += pGeom->m_nMaterials;
                    nTextures   += pGeom->m_nTextures;
                    nMeshes     += pGeom->m_nMeshes;
                    nSubMeshes  += pGeom->m_nSubMeshes;
                }
            }
        }
    }

    x_DebugMsg(xfs("Total RIGIDGEOM's selected (%d Verts) (%d Polys) (%d Materials) (%d Textures) (%d Meshes) (%d SubMeshes)\n",
        nVerts, nFaces, nMaterials, nTextures, nMeshes, nSubMeshes));
}

//=========================================================================

xbool world_editor::IsZoneFileEditable( void )
{
    if (m_pHandler && !m_pHandler->IsFileReadOnly(xfs("%s\\Zoning.ProjectInfo", g_Project.GetWorkingPath())))
    {
        return TRUE;
    }
    return FALSE;
}

//=========================================================================

void world_editor::CleanupZones( void )
{
    if (!m_pHandler)
        return;

    for( s32 i=0; i < m_listZones.GetCount(); i++ )
    {       
        editor_zone_ref &Zone = m_listZones.GetAt(i);
        if (!m_pHandler->IsZoneLoaded(Zone.Layer, Zone.Name))
        {
            //zone not loaded, need to delete it
            SetCurrentUndoEntry(new transaction_entry(xfs("AutoCleanup - Remove Zone(%s)", (const char*)Zone.Name)));
            DeleteZone(Zone.Name);
            CommitCurrentUndoEntry();

            //just to be safe restart search at top of list
            i=-1;
        }
    }
}

//=========================================================================

void world_editor::DestroyObjectEx( guid Guid, xbool bRemoveNow )
{
    //handle any special case objects
    object* pObject = NULL;
    if ( SMP_UTIL_IsGuidOfType ( &pObject, Guid , nav_connection2_editor::GetRTTI() ) == TRUE )
    {
        //deleted a navigation connection
        g_ObjMgr.DestroyObjectEx(((nav_connection2_editor*)pObject)->GetAnchor(0)->GetGuid(), bRemoveNow);
        g_ObjMgr.DestroyObjectEx(((nav_connection2_editor*)pObject)->GetAnchor(1)->GetGuid(), bRemoveNow);
    }

    g_ObjMgr.DestroyObjectEx(Guid, bRemoveNow);
}
