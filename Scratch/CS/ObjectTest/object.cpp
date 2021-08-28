//==============================================================================
//
//  Object.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================
#include "header.hpp"

#ifdef EDITOR
#include <crtdbg.h>
#endif
#include "Entropy.hpp"
#include "Obj_Mgr.hpp"
#include "object.hpp"

//==============================================================================
//  DEFINES
//==============================================================================
#define     OBJECT_VERSION          1001

//==============================================================================
//  OBJECT FUNCTIONS
//==============================================================================

object::object( void )
{
    // All initialization is done in OnInit
    m_pCustomName = NULL;
}

//==============================================================================

object::~object( void )
{
    //  if we are actually destructing the object then all that needs to be 
    //  done is deallocate any pointers that were allocated
    if ( m_pCustomName )
        delete [] m_pCustomName;
}

//==============================================================================
//
//  
//
//==============================================================================

void object::OnAdvanceLogic( f32 DeltaTime )
{
   
}

//==============================================================================

void object::Move( const vector3& NewPos )
{
    ASSERT( x_isvalid(NewPos.X) );
    ASSERT( x_isvalid(NewPos.Y) );
    ASSERT( x_isvalid(NewPos.Z) );

    ASSERT( x_abs(NewPos.X) <= 1000000.0f );
    ASSERT( x_abs(NewPos.Y) <= 1000000.0f );
    ASSERT( x_abs(NewPos.Z) <= 1000000.0f );


    //  Bounding box is already here, just need to move it 
    m_WorldBBox.Translate( NewPos - m_WorldPos );

    // Compute new pos and bbox
    m_WorldPos = NewPos;

}

//==============================================================================

void object::MoveRel( const vector3& DeltaPos )
{
    Move( m_WorldPos + DeltaPos );
}

//==============================================================================

u32 object::GetAttrBits( void ) const
{
    return( m_AttrBits );
}

//==============================================================================
void object::SetAttrBits( u32 NewBits ) 
{
    m_AttrBits = NewBits;
}


//==============================================================================

guid object::GetGuid( void ) const
{
    return( m_Guid );
}

//
//  Decided to remove the GetType and GetTypeName functions and make them pure
//  virtual to make sure people don't forget to overload them.  Easy to put 
//  back if we change our minds...
//

//==============================================================================
//  GetType will be overloaded by each Type that inherits from it
//object::type object::GetType( void ) const
//{
//    return( TYPE_NULL );
//}

//==============================================================================
//  
//const char* object::GetTypeName( void ) const
//{
//    return( "NULL" );
//}

//==============================================================================

const bbox& object::GetBBox( void ) const
{
    return( m_WorldBBox );
}

//==============================================================================

const vector3& object::GetPosition( void ) const
{
    return( m_WorldPos );
}

//==============================================================================

const radian3& object::GetRotation( void ) const
{
    return ( m_Orient );
}

//==============================================================================

void object::SetRotation( const radian3& Orient )
{
    m_Orient = Orient;
}


//==============================================================================

void object::OnInit( void ) 
{


    m_AttrBits = 0;

    m_WorldPos ( 0, 0, 0 );
    m_Orient   ( 0, 0, 0 );

    m_WorldBBox( vector3(-1,-1,-1 ), vector3(1,1,1) );

    m_Guid.Guid = 0;

    //  If the name is something other than null that means this object is
    //  being reused and we need to free it and NULL it out
    if ( m_pCustomName )
    {
        delete [] m_pCustomName;
        m_pCustomName = NULL;
    }


}

//==============================================================================
//
//  Any thing that needs to happen before the destructor should go here.
//
//==============================================================================
void object::OnKill( void )
{  
    m_Guid.Guid = 0;
    if ( m_pCustomName )
        delete [] m_pCustomName;
}

//==============================================================================

void object::DebugRender( void )
{
    draw_Marker( m_WorldPos, XCOLOR_RED );
}

//==============================================================================

void object::Render( void )
{
    draw_Marker( m_WorldPos, XCOLOR_RED );
}

//==============================================================================

void object::SetObjectName(const char *Name)
{
    if(m_pCustomName)
        delete [] m_pCustomName;
 
    s32 StringSize = x_strlen(Name);
    m_pCustomName = new char[StringSize+1];
	x_strncpy(m_pCustomName, Name, StringSize );
}

//==============================================================================

const char* object::GetObjectName(void) const
{
	return m_pCustomName;
}


//==============================================================================
void object::RenderCollision( void )
{

}



//==============================================================================

/******************************************************************************
---------------------    Code Graveyard    ------------------------------------
*******************************************************************************


//=============================================================================

void object::RemoveFromSpatialDBase()
{
    obj_cell_link* pLink = g_ObjMgr.GetCellLinks();

    // Remove ourselves
    for( s32 i=0; i<8; i++ )
    {
        // Get index to this link
        s32 LI = m_Slot*8+i;

        // Check if unused
        if( pLink[LI].CellID == SPATIAL_CELL_NULL )
            break;
        // Get access to cell
        spatial_cell* pCell = (spatial_cell*)g_SpatialDBase.GetCell( pLink[LI].CellID );

        // Remove from linked list
        if( pCell->FirstObjectLink[m_SpatialChannel] == LI )
            pCell->FirstObjectLink[m_SpatialChannel] = pLink[LI].Next;
        if( pLink[LI].Next != -1 )
            pLink[ pLink[LI].Next ].Prev = pLink[LI].Prev;
        if( pLink[LI].Prev != -1 )
            pLink[ pLink[LI].Prev ].Next = pLink[LI].Next;

        // Check if list is empty
        if( pCell->FirstObjectLink[m_SpatialChannel] == -1 )
        {
            pCell->OccFlags &= (~(1<<m_SpatialChannel));
            g_SpatialDBase.UpdateCell( pCell );
        }

        pLink[LI].CellID = SPATIAL_CELL_NULL;
        pLink[LI].Next = -1;
        pLink[LI].Prev = -1;
    }
}


//==============================================================================


void object::AddToSpatialDBase()
{
    // Update spatial database
    s32 i;
    s32 X,Y,Z;
    s32 MinX,MinY,MinZ;
    s32 MaxX,MaxY,MaxZ;
    s32 Level;

    // Get cell regions affected
    Level = g_SpatialDBase.GetBBoxLevel( m_WorldBBox );
    g_SpatialDBase.GetCellRegion( m_WorldBBox, Level, MinX, MinY, MinZ, MaxX, MaxY, MaxZ );

    obj_cell_link* pLink = g_ObjMgr.GetCellLinks();

    for( i=0; i<8; i++ )
    {
        s32 LI = m_Slot*8+i;
        ASSERT( pLink[LI].CellID == SPATIAL_CELL_NULL );
        ASSERT( pLink[LI].Next == -1 );
        ASSERT( pLink[LI].Prev == -1 );
    }

    // Add ourselves
    i=0;
    for( X=MinX; X<=MaxX; X++ )
    for( Y=MinY; Y<=MaxY; Y++ )
    for( Z=MinZ; Z<=MaxZ; Z++ )
    {
        // Get index to link
        s32 LI = m_Slot*8+i;
        ASSERT( i<8 );

        // Get access to cell
        pLink[LI].CellID = g_SpatialDBase.GetCellIndex( X, Y, Z, Level, TRUE );
        //x_DebugMsg("Calling AddToSpatialDBase with CellID '%d'\n", pLink[LI].CellID);
#ifndef TARGET_GCN
        //if(pLink[LI].CellID == 0)
        //    x_DebugMsg("CellID is 0\n");
#endif
        spatial_cell* pCell = (spatial_cell*)g_SpatialDBase.GetCell( pLink[LI].CellID );

        // Add link to list
        pLink[LI].Prev = -1;
        ASSERT(LI != pCell->FirstObjectLink[m_SpatialChannel]);
        pLink[LI].Next = pCell->FirstObjectLink[m_SpatialChannel];        
        if( pCell->FirstObjectLink[m_SpatialChannel] != -1 )
            pLink[ pCell->FirstObjectLink[m_SpatialChannel] ].Prev = LI;
        pCell->FirstObjectLink[m_SpatialChannel] = LI;

        // Turn on occupied flag
        pCell->OccFlags |= (1<<m_SpatialChannel);

        // Move to next link
        i++;
    }
}




//==============================================================================

void object::OnExport( text_out& TOut )
{
    (void)TOut;
}

//==============================================================================

void object::OnImport( text_in& TIn )
{
    (void)TIn;
}





void object::ImportSurfaceFlags ( text_in& TIn )
{
    TIn.ReadHeader();
    ASSERTS( x_stricmp(TIn.GetHeaderName(),"SurfaceFlags") == 0,
        "export.txt file out of date");

    s32 i; 
    s32 Count = TIn.GetHeaderCount();
    m_nTriFlags = Count;
    s32 Flags;
    if (m_pTriFlag)
        delete [] m_pTriFlag;

    m_pTriFlag = new byte[Count];
    x_memset(m_pTriFlag, 0, Count);

    for (i = 0; i < Count; i++)
    {
        TIn.ReadFields();
        TIn.GetS32("Flags", Flags);
        ASSERT(Flags <= U8_MAX);
        m_pTriFlag[i] = (u8)Flags;
    }
}



//==============================================================================
//==============================================================================
//==============================================================================
#ifdef EDITOR
//==============================================================================
//==============================================================================
//==============================================================================
xbool object::HasGeom ( void )
{
    return FALSE;
}

//==============================================================================

xarray<ed_property_desc> object::EnumerateProperties( void )
{
	xarray<ed_property_desc> Result;
	//Make sure LocalBBox stays at the top so it gets set first on load
	EnumerateBboxProperties(Result, TEXT("LocalBBox"), EPD_READONLY);
    EnumerateBboxProperties(Result, TEXT("WorldBBox"), EPD_READONLY | EPD_TRANSIENT);
    EnumerateVector3Properties(Result, TEXT("Position"), (m_Frozen ? EPD_READONLY : EPD_SIMPLE));
    EnumerateRadian3Properties(Result, TEXT("Rotation"), EPD_VIEW_DEGREES);

	ed_property_desc epd;
	epd.m_Name = TEXT("Name");
	epd.m_Type = XW_TYPE_XW_PROP_xwstring;
	epd.m_Flags = EPD_INVISIBLE;
	Result.Append(epd);
	
    epd.m_Name = TEXT("System");
	epd.m_Type = XW_TYPE_XW_PROP_separator;
	epd.m_Flags = EPD_COMPOUND | EPD_READONLY | EPD_TRANSIENT;
	Result.Append(epd);

    epd.m_Name = TEXT("System\\Version");
    epd.m_Type = XW_TYPE_XW_PROP_u32;
    epd.m_Flags = EPD_READONLY;
    Result.Append(epd);

    epd.m_Name = TEXT("System\\Layer");
    epd.m_Type = XW_TYPE_XW_PROP_layer;
    epd.m_Flags = EPD_TRANSIENT;
    Result.Append(epd);
    epd.m_Name = TEXT("System\\LinkedObject");
    epd.m_Type = XW_TYPE_XW_PROP_object;
    epd.m_Flags = EPD_SIMPLE;
    Result.Append(epd);

    epd.m_Name = TEXT("System\\Test Set");
    epd.m_Type = XW_TYPE_XW_PROP_selset;
    epd.m_Flags = EPD_SIMPLE;
    Result.Append(epd);

    epd.m_Name = TEXT("System\\Do Not Export");
    epd.m_Type = XW_TYPE_XW_PROP_xbool;
    epd.m_Flags = EPD_TRANSIENT;
    Result.Append(epd);

    epd.m_Name = TEXT("Does Not Collide");
    epd.m_Type = XW_TYPE_XW_PROP_xbool;
    epd.m_Flags = EPD_SIMPLE;
    Result.Append(epd);

	return Result;
}

//==============================================================================

void object::OnRotate(const radian3& NewValue)
{
    // need to compute new local/world bbox size after rotation
/*
    matrix4 RotateMat;
    RotateMat.Identity();
        
    RotateMat.Rotate(NewValue);

    RemoveFromSpatialDBase();

    m_LocalBBox = m_InitialBBox;
    m_LocalBBox.Transform(RotateMat);
    m_WorldBBox = GetLocalBBox();
    m_WorldBBox.Translate( m_WorldPos );

    AddToSpatialDBase();
}

//==============================================================================

xbool object::SetProperty( const xwstring& PropName, xw_object *pNewValue )
{
	vector3 Position = GetPosition();
	bbox LocalBBox = GetLocalBBox();
	if(SetBboxProperties(TEXT("LocalBBox"), PropName, pNewValue, LocalBBox))
	{        
		m_LocalBBox = LocalBBox;
        Move(m_WorldPos);
        return TRUE;
	}
    if(m_Frozen && PropName.StartsWith("Position"))
        return FALSE;
	if(SetVector3Properties(TEXT("Position"), PropName, pNewValue, Position))
	{
		if(Position.X < -100000) Position.X = -100000; if(Position.X > 100000) Position.X = 100000;
		if(Position.Y < -100000) Position.Y = -100000; if(Position.Y > 100000) Position.Y = 100000;
		if(Position.Z < -100000) Position.Z = -100000; if(Position.Z > 100000) Position.Z = 100000;

		Move(Position);
		return TRUE;
	}
    if (m_Frozen && PropName.StartsWith("Rotation"))
        return FALSE;

    if(SetRadian3Properties(TEXT("Rotation"), PropName, pNewValue, m_Orient))
	{
        OnRotate(m_Orient);
        
		return TRUE;
	}

	if(PropName == TEXT("Name"))
	{
		xwstring S = ((ed_value_holder<xwstring> *)pNewValue)->GetValue();
		SetName(S);
		return TRUE;
	}
    if(PropName == TEXT("System\\Version"))
    {
        s32 v = ((ed_value_holder<u32> *)pNewValue)->GetValue();
        SetVersion(v);
    }
    if(PropName == TEXT("System\\Layer"))
    {
        xwstring S = ((ed_value_holder<xwstring> *)pNewValue)->GetValue();
		SetLayer(S);
		return TRUE;
	}
    if(PropName == TEXT("System\\LinkedObject"))
    {
        guid G = ((ed_value_holder<guid> *)pNewValue)->GetValue();
        SetLinkedGuid(G);
    }
    if(PropName == TEXT("System\\Test Set"))
    {
        ed_selection_set *pSet = ((ed_value_holder<ed_selection_set *> *)pNewValue)->GetValue();
        m_pSet = pSet;
    }
    if (PropName == TEXT("System\\Do Not Export"))
    {
        m_bNoExport = ((ed_value_holder<xbool> *)pNewValue)->GetValue();
        return TRUE;
    }
    if (PropName == TEXT("Does Not Collide"))
    {
        m_bDoNotCollide = ((ed_value_holder<xbool> *)pNewValue)->GetValue();
        return TRUE;
    }
	return FALSE;
}

//==============================================================================

const xw_object* object::GetProperty( const xwstring& PropName )
{
	xw_object *Result;
	vector3 Position = GetPosition();
	bbox LocalBBox = GetLocalBBox();
    bbox WorldBBox = GetBBox();
    radian3 Rot     = GetRotation();
	if(Result = GetBboxProperties(TEXT("LocalBBox"), PropName, LocalBBox))
		return Result;
	if(Result = GetBboxProperties(TEXT("WorldBBox"), PropName, WorldBBox))
		return Result;
	if(Result = GetVector3Properties(TEXT("Position"), PropName, Position))
		return Result;
    if(Result = GetRadian3Properties(TEXT("Rotation"), PropName, Rot))
        return Result;

	if(PropName == TEXT("Name"))
	{
        //ASSERT(_CrtCheckMemory());
        xwstring S(GetStringName());
        //ASSERT(_CrtCheckMemory());
		return new ed_value_holder<xwstring>(S);
        //ASSERT(_CrtCheckMemory());
	}
    if(PropName == TEXT("System\\Layer"))
    {
		return new ed_value_holder<xwstring>(GetLayer());
	}
    if(PropName == TEXT("System\\Version"))
    {
        return new ed_value_holder<u32>(GetVersion());
    }
    if(PropName == TEXT("System\\LinkedObject"))
    {
        return new ed_value_holder<guid>(GetLinkedGuid());
    }
    if(PropName == TEXT("System\\Test Set"))
    {
        return new ed_value_holder<ed_selection_set*>(m_pSet);
    }
    if(PropName == TEXT("System\\Do Not Export"))
    {
        return new ed_value_holder<xbool>(m_bNoExport);
    }
    if(PropName == TEXT("Does Not Collide"))
    {
        return new ed_value_holder<xbool>(m_bDoNotCollide);
    }
	return NULL;
}

//==============================================================================

void object::SetUnselectable(xbool Flag, xbool AdjustClasses, xbool AdjustLayers)
{
    ASSERT(Flag == TRUE || Flag == FALSE);
    m_Unselectable = Flag;
    if(AdjustClasses)
    {
        if(Flag)
        {
            //Check to see if all are unselectable
            guid Guid;
			g_ObjMgr.StartTypeLoop( this->GetType() );
			while( Guid = g_ObjMgr.GetNextInType() )
			{
				object* pObject = (object*)g_ObjMgr.LockObject( Guid );
				ASSERT(pObject);
				
                if(!pObject->GetUnselectable())
                {
                    g_ObjMgr.SetUnselectable(this->GetType(), -1, FALSE);
                    g_ObjMgr.UnlockObject( Guid );
                    g_ObjMgr.EndTypeLoop();
                    goto AdjustLayersLabel;
                }
				g_ObjMgr.UnlockObject( Guid );
			}
			g_ObjMgr.EndTypeLoop();   
            g_ObjMgr.SetUnselectable(this->GetType(), TRUE, FALSE);
        }
        else
        {
            //Check to see if all are selectable
            guid Guid;
			g_ObjMgr.StartTypeLoop( this->GetType() );
			while( Guid = g_ObjMgr.GetNextInType() )
			{
				object* pObject = (object*)g_ObjMgr.LockObject( Guid );
				ASSERT(pObject);
				
                if(pObject->GetUnselectable())
                {
                    g_ObjMgr.SetUnselectable(this->GetType(), -1, FALSE);
                    g_ObjMgr.UnlockObject( Guid );
                    g_ObjMgr.EndTypeLoop();
                    goto AdjustLayersLabel;
                }
				g_ObjMgr.UnlockObject( Guid );
			}
			g_ObjMgr.EndTypeLoop();   
            g_ObjMgr.SetUnselectable(this->GetType(), FALSE, FALSE);
        }
    }
AdjustLayersLabel:
    if(AdjustLayers)
    {
        if(this->GetLayer().GetLength() > 0)
        {
            if(Flag)
            {
                //Check to see if all are unselectable
                guid Guid;
			    g_ObjMgr.StartLayerLoop( this->GetLayer() );
			    while( Guid = g_ObjMgr.GetNextInLayer() )
			    {
				    object* pObject = (object*)g_ObjMgr.LockObject( Guid );
				    ASSERT(pObject);
				    
                    if(!pObject->GetUnselectable())
                    {
                        g_ObjMgr.SetUnselectable(this->GetLayer(), -1, FALSE);
                        g_ObjMgr.UnlockObject( Guid );
                        g_ObjMgr.EndLayerLoop();
                        return;
                    }
				    g_ObjMgr.UnlockObject( Guid );
			    }
			    g_ObjMgr.EndLayerLoop();   
                g_ObjMgr.SetUnselectable(this->GetLayer(), TRUE, FALSE);
            }
            else
            {
                //Check to see if all are selectable
                guid Guid;
			    g_ObjMgr.StartLayerLoop( this->GetLayer() );
			    while( Guid = g_ObjMgr.GetNextInLayer() )
			    {
				    object* pObject = (object*)g_ObjMgr.LockObject( Guid );
				    ASSERT(pObject);
				    
                    if(pObject->GetUnselectable())
                    {
                        g_ObjMgr.SetUnselectable(this->GetLayer(), -1, FALSE);
                        g_ObjMgr.UnlockObject( Guid );
                        g_ObjMgr.EndLayerLoop();
                        return;
                    }
				    g_ObjMgr.UnlockObject( Guid );
			    }
			    g_ObjMgr.EndLayerLoop();   
                g_ObjMgr.SetUnselectable(this->GetLayer(), FALSE, FALSE);
            }
        }
    }
}

//==============================================================================

xbool object::GetUnselectable(void)
{
    return m_Unselectable;
}



//==============================================================================

void object::SetHidden(xbool Flag, xbool AdjustClasses, xbool AdjustLayers)
{
    m_Hidden = Flag;
    ASSERT(Flag == TRUE || Flag == FALSE);
    if(AdjustClasses)
    {
        if(Flag)
        {
            //Check to see if all are hidden
            guid Guid;
			g_ObjMgr.StartTypeLoop( this->GetType() );
			while( Guid = g_ObjMgr.GetNextInType() )
			{
				object* pObject = (object*)g_ObjMgr.LockObject( Guid );
				ASSERT(pObject);
				
                if(!pObject->GetHidden())
                {
                    g_ObjMgr.SetHidden(this->GetType(), -1, FALSE);
                    g_ObjMgr.UnlockObject( Guid );
                    g_ObjMgr.EndTypeLoop();
                    goto AdjustLayersLabel;
                }
				g_ObjMgr.UnlockObject( Guid );
			}
			g_ObjMgr.EndTypeLoop();   
            g_ObjMgr.SetHidden(this->GetType(), TRUE, FALSE);
        }
        else
        {
            //Check to see if all are not hidden
            guid Guid;
			g_ObjMgr.StartTypeLoop( this->GetType() );
			while( Guid = g_ObjMgr.GetNextInType() )
			{
				object* pObject = (object*)g_ObjMgr.LockObject( Guid );
				ASSERT(pObject);
				
                if(pObject->GetHidden())
                {
                    g_ObjMgr.SetHidden(this->GetType(), -1, FALSE);
                    g_ObjMgr.UnlockObject( Guid );
                    g_ObjMgr.EndTypeLoop();
                    goto AdjustLayersLabel;
                }
				g_ObjMgr.UnlockObject( Guid );
			}
			g_ObjMgr.EndTypeLoop();   
            g_ObjMgr.SetHidden(this->GetType(), FALSE, FALSE);
        }
    }
AdjustLayersLabel:
    if(AdjustLayers)
    {
        if(this->GetLayer().GetLength() > 0)
        {
            if(Flag)
            {
                //Check to see if all are hidden
                guid Guid;
			    g_ObjMgr.StartLayerLoop( this->GetLayer() );
			    while( Guid = g_ObjMgr.GetNextInLayer() )
			    {
				    object* pObject = (object*)g_ObjMgr.LockObject( Guid );
				    ASSERT(pObject);
				    
                    if(!pObject->GetHidden())
                    {
                        g_ObjMgr.SetHidden(this->GetLayer(), -1, FALSE);
                        g_ObjMgr.UnlockObject( Guid );
                        g_ObjMgr.EndLayerLoop();
                        return;
                    }
				    g_ObjMgr.UnlockObject( Guid );
			    }
			    g_ObjMgr.EndLayerLoop();   
                g_ObjMgr.SetHidden(this->GetLayer(), TRUE, FALSE);
            }
            else
            {
                //Check to see if all are not hidden
                guid Guid;
			    g_ObjMgr.StartLayerLoop( this->GetLayer() );
			    while( Guid = g_ObjMgr.GetNextInLayer() )
			    {
				    object* pObject = (object*)g_ObjMgr.LockObject( Guid );
				    ASSERT(pObject);
				    
                    if(pObject->GetHidden())
                    {
                        g_ObjMgr.SetHidden(this->GetLayer(), -1, FALSE);
                        g_ObjMgr.UnlockObject( Guid );
                        g_ObjMgr.EndLayerLoop();
                        return;
                    }
				    g_ObjMgr.UnlockObject( Guid );
			    }
			    g_ObjMgr.EndLayerLoop();   
                g_ObjMgr.SetHidden(this->GetLayer(), FALSE, FALSE);
            }
        }
    }
}

//==============================================================================

xbool object::GetHidden(void)
{
    return m_Hidden;
}

//==============================================================================

void object::SetLayer( const xwstring& NewLayer )
{
    m_Layer = NewLayer;
}

//==============================================================================

xwstring object::GetLayer( void )
{
    return m_Layer;
}

//==============================================================================

void object::SetFrozen(xbool Frozen)
{
    m_Frozen = Frozen;
}

//==============================================================================

xbool object::GetFrozen( void )
{
    return m_Frozen;
}

//==============================================================================

xbool object::IsTransient( void )
{
    return m_Transient;
}

//==============================================================================

void object::SetTransient ( xbool Transient )
{
    m_Transient = Transient;
}

//==============================================================================

s32 object::SavePrivateData ( ed_db& Db )
{
    s32 i;
    if (m_pVertexColor)
    {
        for (i = 0; i < m_nVertices; i++)
            Db.write(0, m_pVertexColor[i]);
     
        return i;
    }
    return 0;
}

//==============================================================================

void object::LoadPrivateData ( ed_db& Db, s32 Count )
{
    // Need access to number of verts.
    (void)Db;
    (void)Count;
}

//==============================================================================

void object::DiscardPrivateData ( ed_db& Db )
{
    s32 DbgCtr = 0;
    // Dump out all the old vert data & tri data if this is an old version.
    ed_db_prop Garbage = Db.peek();
    while (  (Garbage.m_PropType == ED_DB_XCOLOR)
          || (Garbage.m_PropType == ED_DB_U8))
    {
        if (Garbage.m_PropId == ED_PID_EOF
            || Garbage.m_PropId == ED_PID_BEGIN)
        {
            x_DebugMsg("Discarded %d DbProps\n", DbgCtr);
            return;
        }
        else if (Garbage.m_PropId == ED_PID_END)
        {
            s32 Count = *((s32*)Garbage.m_pData);
            if (!Count)
            {
                x_DebugMsg("Discarded %d DbProps\n", DbgCtr);
                return;
            }
        }
        Db.read();
        Garbage = Db.peek();
        DbgCtr++;
    }
    x_DebugMsg("Discarded %d DbProps\n", DbgCtr);
}

//==============================================================================

xbool object::HasPrivateData ( void )
{
    return (m_nVertices != 0);
}

//==============================================================================

s32 object::GetPrivateDataCount ( void )
{
    return VertCount();
}

//==============================================================================

void object::CompileLighting ( xbool AllLights, xbool AllObjects,f32 ScaleVal  )
{
    (void)AllLights;
    (void)AllObjects;
    (void)ScaleVal;
}
//==============================================================================

s32 object::VertCount  ( void )
{
    return 0;
}

//==============================================================================

s32 object::GetTriCount ( void )
{
    return 0;
}

//==============================================================================

xbool object::HasErrors( void )
{
    return FALSE;
}

//==============================================================================

void object::SetVersion( s32 Version )
{
    m_Version = Version;
}

//==============================================================================

s32 object::GetVersion( void )
{
    return m_Version;
}

//==============================================================================

void object::EnableCollisionRender ( void )
{
}

//==============================================================================

void object::DisableCollisionRender ( void )
{
}

//==============================================================================

xbool object::SupportsSubSelect( void )
{
    return FALSE;
}

//==============================================================================

void object::SetSubSelectMode    ( xbool SubSelectMode )
{
}

//==============================================================================

s32  object::SubSelectRay        ( vector3 Begin, vector3 End )
{
    return 0;
}

//==============================================================================

void object::SetSubSelection     ( s32 Id )
{
}
//==============================================================================

xbool object::LoadTriFlags( ed_db& Db, s32 Count )  // defined in RigidInstance 
{                                               // & PlaySurface
    (void)Db;
    return FALSE;
}

//==============================================================================

s32 object::SaveTriFlags( ed_db& Db ) // defined in RigidInstance 
{                                               // & PlaySurface
    (void)Db;
    return FALSE;
}

//==============================================================================

void object::SetActiveTri ( const xarray<s32>& Tris )
{
    (void) Tris;
}

//==============================================================================

void object::SetActiveTri ( s32 TriId )
{
    (void) TriId;
}

//==============================================================================

void object::ClearActiveTri ( s32 TriId )
{
    (void) TriId;
}

//==============================================================================

xbool object::GetSubSelectMode ( void )
{
    return m_SubSelectMode;
}

//==============================================================================

const vector3& object::GetHandlePosition   ( void )
{
    return GetPosition();
}

//==============================================================================

void object::OnDeselect( void )
{
    if(SupportsSubSelect())
    {
        SetSubSelectMode(FALSE);
    }
    return;
}

//==============================================================================

xbool object::SupportsTagSurfaces ( void )
{
    return FALSE;
}

//==============================================================================

void object::SetTagMode          ( xbool EnableTagging )
{
    m_SurfaceTagMode = EnableTagging;
}

//==============================================================================
#endif //EDITOR








  */