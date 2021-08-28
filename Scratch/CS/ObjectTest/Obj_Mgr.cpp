//==============================================================================
//
//  ObjectMgr.cpp
//
//==============================================================================


//==============================================================================
//  INCLUDES
//==============================================================================
#include "header.hpp"

#include "Obj_Mgr.hpp"
#include "x_stdio.hpp"

#include "player.hpp"


#ifdef EDITOR
#include "xw_manager.hpp"
#include "c:\projects\meridian\apps\editor\editor.hpp"
#endif

//==============================================================================
//  Global declarations
//==============================================================================


obj_mgr g_ObjMgr;   //  obj_mgr is a singleton class


//==============================================================================
//  OBJECT MANAGER FUNCTIONS
//==============================================================================


obj_mgr::obj_mgr( void )
{
    //  To ensure it's singleton nature is not violated
    static bool AlreadyCreated = false;
    ASSERT( !AlreadyCreated);
    AlreadyCreated = true;
}

//==============================================================================

obj_mgr::~obj_mgr( void )
{
    // Assert that there are no objects left or not needed because it's a singleton
}


//==============================================================================
//
// Destroy all objects
//
//==============================================================================

void obj_mgr::Kill( void )
{

    s32     count;
    slot_ID SlotID;
    
    for( count = 0; count < object::TYPE_END_OF_LIST; count++ )
    {
        if( m_ObjectType[count].InstanceCount )
        {

            SlotID = GetFirst( (object::type)count );
            while( SlotID != INVALID_OBJECT )
            {
                ASSERT( GetObjectBySlot( SlotID ) );
                DestroyObject( GetObjectBySlot( SlotID )->GetGuid() );
                SlotID = GetNext( count );
            } 
        }
    }
}

//==============================================================================

void obj_mgr::Init( void )
{
    s32 i;

    // Setup guid lookup capacity to number of objects
    m_GuidLookup.SetCapacity( MAX_OBJECTS, FALSE );
    SanityCheck();

    m_FirstSearchResult = MAX_OBJECTS;

    // Clear type lists for all info ptrs
    for( i=0; i<object::TYPE_END_OF_LIST; i++ )
    {
        m_ObjectType[i].FirstType = -1;
        m_ObjectType[i].InstanceCount = 0;
        m_ObjectType[i].LogicTime.Reset();
    }

    // Hook all Slots into free list
    m_FirstFreeSlot = -1;

    for( i=0; i<MAX_OBJECTS; i++ )
    {
        m_ObjectSlot[i].pObject = NULL;
        m_ObjectSlot[i].Next = m_FirstFreeSlot;
        m_ObjectSlot[i].Prev = -1;
        m_ObjectSlot[i].NextVis = -1;

        for( s32 j=0; j<8; j++ )
        {
            m_ObjectLink[i*8+j].CellID = SPATIAL_CELL_NULL;
            m_ObjectLink[i*8+j].Next = -1;
            m_ObjectLink[i*8+j].Prev = -1;
        }

        if( m_FirstFreeSlot != -1 )
            m_ObjectSlot[ m_FirstFreeSlot ].Prev = i;

        m_FirstFreeSlot = i;
    }

}


//==============================================================================
guid obj_mgr::CreateObject( char *pObjectTypeName )
{
	for(u32 count = 0; count < object::TYPE_END_OF_LIST; count++)
	{
//		obj_type_info* pInfo = m_pObjTypeInfo[count];

		if(m_ObjectType[count].pTypeName && x_strcmp(m_ObjectType[count].pTypeName, pObjectTypeName) == 0)
		{
            return CreateObject((object::type)count);
		}
	}

    ASSERT(FALSE);
	return guid();      // FIXME:  Should never get here but need a better way to handle the error
}
        
//==============================================================================

guid obj_mgr::CreateObject( object::type Type)
{
    //ASSERT( (Type>=0) && (Type<object::TYPE_END_OF_LIST) );
    SanityCheck();

    object* pObject;

    ASSERT( IN_RANGE( 0, Type, object::TYPE_END_OF_LIST-1 ) );

    // Increment number of instances
    m_ObjectType[Type].InstanceCount++;

    // Find slot for object
    ASSERT( m_FirstFreeSlot != -1 );

    s32 Slot = m_FirstFreeSlot;

    ASSERT(m_FirstFreeSlot != m_ObjectSlot[Slot].Next);

    m_FirstFreeSlot = m_ObjectSlot[Slot].Next;
    m_ObjectSlot[Slot].Next = -1;
    m_ObjectSlot[Slot].Prev = -1;
    if( m_FirstFreeSlot != -1 )
        m_ObjectSlot[m_FirstFreeSlot].Prev = -1;

    // Add slot to type list
    if( m_ObjectType[Type].FirstType != -1 )
        m_ObjectSlot[ m_ObjectType[Type].FirstType ].Prev = Slot;
    m_ObjectSlot[ Slot ].Next = m_ObjectType[Type].FirstType;
    m_ObjectSlot[ Slot ].Prev = -1;
    m_ObjectType[ Type ].FirstType = Slot;





    //
    //  Only new should be called by this switch statement, onInit gets called
    //  later
    //
    //  FIXME:  need a better system for creation.  Could switch to use the
    //          system that is used by Meridian and register a creation fucntion
    //        
    switch (Type)
    {
    case object::TYPE_LIGHT:
        // allocate it
//        pObject = new Light;
        break;

    case object::TYPE_PLAYER:
        pObject = new player;
        break;

    case object::TYPE_TRIGGER:
        // allocate it
//        pObject = new Trigger;
        break;

    case object::TYPE_NULL:
    case object::TYPE_END_OF_LIST:
    default:
        ASSERT(FALSE);          //  Can't allocate objects of this type

    }

    ASSERT(pObject);

    // Compute a guid for object
    guid Guid = guid_New();

    m_GuidLookup.Add( Guid, Slot );
    m_ObjectSlot[Slot].pObject = pObject;

    // Clear links
    for( s32 i=0; i<8; i++ )
    {
        m_ObjectLink[Slot*8+i].CellID = SPATIAL_CELL_NULL;
        m_ObjectLink[Slot*8+i].Next = -1;
        m_ObjectLink[Slot*8+i].Prev = -1;
    }


    // Init base values in object
    pObject->m_Guid          = Guid;
	pObject->OnInit();

    return( Guid );
}

//==============================================================================

s32 obj_mgr::GetSlotFromGuid( guid Guid )
{
    s32 Slot;

    if( m_GuidLookup.Find( Guid, Slot ) )
        return Slot;

    return -1;
}

//==============================================================================

void obj_mgr::DestroyObject( guid Guid )
{

    slot_ID Slot = GetSlotFromGuid( Guid );
    ASSERT( Slot != -1 );

    m_ObjectType[m_ObjectSlot[Slot].pObject->GetType()].InstanceCount--;

    // Check type loop cursors
    RemoveFromSpatialDBase( Slot );

	// Tell object to remove
	m_ObjectSlot[Slot].pObject->OnKill();

    // Remove from type list
    if( m_ObjectType[ m_ObjectSlot[Slot].pObject->GetType() ].FirstType == Slot )
        m_ObjectType[ m_ObjectSlot[Slot].pObject->GetType() ].FirstType = m_ObjectSlot[Slot].Next;

    if( m_ObjectSlot[Slot].Prev != -1 )
        m_ObjectSlot[ m_ObjectSlot[Slot].Prev ].Next = m_ObjectSlot[Slot].Next;

    if( m_ObjectSlot[Slot].Next != -1 )
        m_ObjectSlot[ m_ObjectSlot[Slot].Next ].Prev = m_ObjectSlot[Slot].Prev;

    // Put slot in free list
    m_ObjectSlot[ Slot ].Next = m_FirstFreeSlot;
    m_ObjectSlot[ Slot ].Prev = -1;

    if( m_FirstFreeSlot != -1 )
        m_ObjectSlot[ m_FirstFreeSlot ].Prev = Slot;

    m_FirstFreeSlot = Slot;

    // Free memory
    delete m_ObjectSlot[Slot].pObject;

    // Remove GUID from lookup
    m_GuidLookup.Del( m_ObjectSlot[Slot].pObject->GetGuid() );

    // Clear memory
    m_ObjectSlot[Slot].pObject = NULL;

    // Clear links
    for( s32 i=0; i<8; i++ )
    {
        m_ObjectLink[Slot*8+i].CellID = SPATIAL_CELL_NULL;
        m_ObjectLink[Slot*8+i].Next = -1;
        m_ObjectLink[Slot*8+i].Prev = -1;
    }

}

//==============================================================================
//
//  Clears all objects from the list and destroys them.
//
//==============================================================================
void obj_mgr::Clear( void )
{
    s32 i;

    for( i = 0; i < MAX_OBJECTS; i++ )
    {
        if( m_ObjectSlot[i].pObject )
            DestroyObject( m_ObjectSlot[i].pObject->GetGuid() );
        m_ObjectSlot[i].pObject = NULL;
    }

}

//==============================================================================

void obj_mgr::AdvanceAllLogic( f32 DeltaTime )
{
    xtimer  TotalLogicTime;
    xtimer  TInner;
    TInner.Reset();
    TotalLogicTime.Start();
    //guid    Guid;
    s32     i;

//    g_RenderMgr.Stats.RigidInstAnimPlayerMS = 0;

    for( i = 0; i < object::TYPE_END_OF_LIST; i++ )
    {
        TInner.Reset();

        m_ObjectType[i].LogicTime.Reset();
        m_ObjectType[i].LogicTime.Start();

        if( m_ObjectType[i].InstanceCount )// && m_pObjTypeInfo[i]->bRunLogic)
        {
            slot_ID SlotID =  GetFirst( ( object::type)i );

            while( GetObjectBySlot( SlotID ) )
            {
                ASSERT(GetObjectBySlot( SlotID ) );

                TInner.Start();
                
                m_ObjectSlot[i].pObject->OnAdvanceLogic( DeltaTime );                

                TInner.Stop();

                // Check if object wants to be destroyed
                if( GetObjectBySlot(SlotID)->GetAttrBits() & object::ATTR_DESTROY )
                    DestroyObject( GetObjectBySlot(SlotID)->GetGuid() );

                SlotID = GetNext(SlotID);
            }
        }

        m_ObjectType[i].LogicTime.Stop();
    }
//    g_RenderMgr.Stats.LogicMS = TotalLogicTime.ReadMs();
}


//==============================================================================

slot_ID obj_mgr::GetFirst( object::type Type )
{
    return m_ObjectType[Type].FirstType;

}


//==============================================================================

slot_ID obj_mgr::GetNext( slot_ID SlotID )
{
    ASSERT(GetObjectBySlot(SlotID) );
    return m_ObjectSlot[SlotID].Next;    
}

const object* obj_mgr::GetObjectBySlot( slot_ID SlotID)
{
    return m_ObjectSlot[SlotID].pObject;
}

const object* obj_mgr::GetObjectByGuid( guid Guid)
{
    u32 Slot;
    m_GuidLookup.Find( Guid, Slot );
    return m_ObjectSlot[Slot].pObject;

}

//==============================================================================

void obj_mgr::SanityCheck( void )
{
    // Make sure the number of objects of each type in the data base is the
    // same as the number of objects in the type cell.

    // Make sure free list and used list do not overlap
#ifdef ahunter
m_GuidLookup.SanityCheck();
#endif

}
//==============================================================================

s32 obj_mgr::GetNumInstances( object::type Type )
{
    //ASSERT( (Type>=0) && (Type<TYPE_END_OF_LIST) );
    return m_ObjectType[Type].InstanceCount;
}

//==============================================================================
void obj_mgr::ResetSearchResult( void )
{
    m_FirstSearchResult = INVALID_OBJECT;

}

//==============================================================================
void obj_mgr::SetNextSearchResult( slot_ID SlotID )
{
    m_ObjectSlot[ SlotID ].NextSearch = m_FirstSearchResult;
    m_FirstSearchResult = SlotID;

}

slot_ID obj_mgr::GetFirstResult(void)
{
    return m_FirstSearchResult;

}


//==============================================================================
//==============================================================================
//==============================================================================
// QUERIES
//==============================================================================
//==============================================================================
//==============================================================================



//==============================================================================
// CheckCollisions
//==============================================================================
struct select_func_info
{
    u32     AttrFlags;
    bbox    BBox;
};

xbool SelectFunc( spatial_cell* pCell, void* pPrivateData )
{
    select_func_info* pInfo = (select_func_info*)pPrivateData;

    g_ObjMgr.ResetSearchResult();

    // Loop through them
    u32 Channel = 0;
    for (Channel = 0; Channel < NUM_SPATIAL_CHANNELS; Channel++)
    {
	    s32 LI = pCell->FirstObjectLink[ Channel ];
        s32 C2=0;
        while( LI != -1 )
        {
            ASSERT( (C2++)<1000 );

            // Get object ptr
//            object* pObject = (object*)g_ObjMgr.LockObjectFromSlot( LI/8 );
            object* pObject = (object*)g_ObjMgr.GetObjectBySlot( LI/8 );
            ASSERT(pObject);

            if( pObject->GetAttrBits() & pInfo->AttrFlags )
            {
                if( pInfo->BBox.Intersect( pObject->GetBBox() ) )
                {
//                    selset_Add( pInfo->Set, pObject->GetGuid() );
                      g_ObjMgr.SetNextSearchResult( LI/8 );
                }
            }

//            g_ObjMgr.UnlockObject( pObject->GetGuid() );

            // Move to next link for this cell
            LI = g_ObjMgr.GetCellLinks()[LI].Next;
	    }
    }

    return TRUE;
}

//==============================================================================
//  FIXME:  SrcSet is not currently used.  Objects should e selected from 
//          the resulting list from this object.  All objects should be used
//          if -1 is used for the srcset since that is the number that will
//          be returned by the GetFirstSearchResult call
//==============================================================================

slot_ID obj_mgr::SelectBBox( slot_ID SrcSet, u32 Attribute, const bbox& BBox , object::type Type )
{

    select_func_info Info;
    Info.AttrFlags = Attribute;
    Info.BBox      = BBox;
    g_SpatialDBase.TraverseCells( BBox, SelectFunc, &Info );

    return g_ObjMgr.GetFirstResult();
}

//==============================================================================
//  FIXME:  This function is bad.  Gotta be the worst way to handle this
//    Kill the xarrays, put a real sort in there.  This is currently ass
//
//  FIXME:  SrcSet is not currently used.  Objects should e selected from 
//          the resulting list from this object.  All objects should be used
//          if -1 is used for the srcset since that is the number that will
//          be returned by the GetFirstSearchResult call
//==============================================================================

slot_ID obj_mgr::SelectRay( slot_ID SrcSet, u32 Attribute, const vector3& Start, const vector3& End, xbool Unsorted, object::type Type )
{

    xarray<f32> distances;
    xarray<slot_ID> guids;

    // Loop through all objects
    for( s32 i=0; i<MAX_OBJECTS; i++ )
    {   
    
        if( GetObjectBySlot( i ) )
        {
            if( GetObjectBySlot( i )->GetAttrBits() & Attribute )
            {
                f32 T;
                bbox BBox = GetObjectBySlot( i )->GetBBox();

                if( BBox.Intersect( T, Start, End ) && T > 0 )
                {
                    guids.Append(i );
                    distances.Append(T);
                }
            }
        }
    }
    if(!Unsorted)
    {
        for(s32 i = 0; i < guids.GetCount(); i++)
            for(s32 j = i + 1; j < guids.GetCount(); j++)
            {
                if(distances.GetAt(i) > distances.GetAt(j))
                {
                    f32 tmp = distances.GetAt(j);
                    distances.GetAt(j) = distances.GetAt(i);
                    distances.GetAt(i) = tmp;
                    slot_ID tmpg = guids.GetAt(j);
                    guids.GetAt(j) = guids.GetAt(i);
                    guids.GetAt(i) = tmpg;
                }
            }
    }
    
    ResetSearchResult();
    for(s32 count = guids.GetCount()-1; count >= 0; count--)
    {
        SetNextSearchResult( guids.GetAt( count ) );
    }

    return GetFirstResult();
}


//==============================================================================
//  FIXME:  SrcSet is not currently used.  Objects should e selected from 
//          the resulting list from this object.  All objects should be used
//          if -1 is used for the srcset since that is the number that will
//          be returned by the GetFirstSearchResult call
//==============================================================================
slot_ID obj_mgr::SelectVolume( slot_ID SrcSet, u32 Attribute, const plane* pPlane, s32 NPlanes, xbool Fast, object::type Type  )
{
    s32 i,j,k;

    ASSERT( NPlanes <= 16 );

    // SLOW VERSION
    ASSERT( SrcSet != INVALID_OBJECT );


    ResetSearchResult();
    // Loop through all objects
    for( i=0; i<MAX_OBJECTS; i++ )
    {
    
        if( GetObjectBySlot(i) )
        {
            const object* pObj = GetObjectBySlot(i);
            ASSERT( pObj );

            if( pObj->GetAttrBits() & Attribute )
            {
                bbox BBox = pObj->GetBBox();
                u32 Spanning = 0;

                // Determine for each plane whether it is in or out
                for( j=0; j<NPlanes; j++ )
                {
                    vector3 BestPt;
                    vector3 WorstPt;

                    if( pPlane[j].Normal.X > 0 ) { BestPt.X = BBox.Max.X; WorstPt.X = BBox.Min.X; }
                    else                         { BestPt.X = BBox.Min.X; WorstPt.X = BBox.Max.X; }
                    if( pPlane[j].Normal.Y > 0 ) { BestPt.Y = BBox.Max.Y; WorstPt.Y = BBox.Min.Y; }
                    else                         { BestPt.Y = BBox.Min.Y; WorstPt.Y = BBox.Max.Y; }
                    if( pPlane[j].Normal.Z > 0 ) { BestPt.Z = BBox.Max.Z; WorstPt.Z = BBox.Min.Z; }
                    else                         { BestPt.Z = BBox.Min.Z; WorstPt.Z = BBox.Max.Z; }

                    f32 BestD = pPlane[j].Distance( BestPt );
                    f32 WorstD = pPlane[j].Distance( WorstPt );

                    if( BestD < 0.0f ) 
                    {
                        // Trivial rejection so terminate early
                        break;
                    }

                    if( WorstD < 0.0f )
                    {
                        // Spanning plane so store for clipping
                        Spanning |= (1<<j);
                    }
                }

                // Did pass trivial rejection?
                if( j==NPlanes )
                {
                    // If fast or bbox is completely inside all planes just add
                    if( Fast || (Spanning==0) )
                    {
//                        selset_Add( Set, m_ObjectSlot[i].Guid );
                          SetNextSearchResult(i);
                    }
                    else
                    {
                        s16 Index[] = {3,1,5,7, 7,5,4,6, 3,2,0,1, 6,4,0,2, 2,3,7,6, 1,0,4,5};
                        vector3 P[8];

                        P[0].X = BBox.Min.X;    P[0].Y = BBox.Min.Y;    P[0].Z = BBox.Min.Z;
                        P[1].X = BBox.Min.X;    P[1].Y = BBox.Min.Y;    P[1].Z = BBox.Max.Z;
                        P[2].X = BBox.Min.X;    P[2].Y = BBox.Max.Y;    P[2].Z = BBox.Min.Z;
                        P[3].X = BBox.Min.X;    P[3].Y = BBox.Max.Y;    P[3].Z = BBox.Max.Z;
                        P[4].X = BBox.Max.X;    P[4].Y = BBox.Min.Y;    P[4].Z = BBox.Min.Z;
                        P[5].X = BBox.Max.X;    P[5].Y = BBox.Min.Y;    P[5].Z = BBox.Max.Z;
                        P[6].X = BBox.Max.X;    P[6].Y = BBox.Max.Y;    P[6].Z = BBox.Min.Z;
                        P[7].X = BBox.Max.X;    P[7].Y = BBox.Max.Y;    P[7].Z = BBox.Max.Z;

                        // Loop through the six quads of the bbox
                        for( k=0; k<6; k++ )
                        {
                            vector3 Vert[64];
                            s32     SrcOffset=0;
                            s32     DstOffset=32;
                            s32     NSrcVerts=4;
                            s32     NDstVerts;
                            Vert[0] = P[Index[k*4+0]];
                            Vert[1] = P[Index[k*4+1]];
                            Vert[2] = P[Index[k*4+2]];
                            Vert[3] = P[Index[k*4+3]];

                            // Loop through the planes and clip ngon to each plane
                            for( j=0; j<NPlanes; j++ )
                            if( Spanning & (1<<j) )
                            {
                                if( !pPlane[j].ClipNGon( Vert+DstOffset, NDstVerts, Vert+SrcOffset, NSrcVerts ) )
                                    break;

                                // Flip buffers
                                SrcOffset = DstOffset;
                                DstOffset = (DstOffset + 32)%64;
                                NSrcVerts = NDstVerts;
                            }
                    
                            // Did we survive all planes?
                            if( j==NPlanes )
                            {
                                SetNextSearchResult(i);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    // Check if set is empty

    return GetFirstResult();
}


//==============================================================================

void obj_mgr::GetAllTypes( xarray<xwstring> &Names, xarray<object::type> &Types)
{
	for(u32 count = 0; count < object::TYPE_END_OF_LIST; count++)
	{
		if( m_ObjectType[count].pTypeName )
		{
			Names.Append( m_ObjectType[count].pTypeName );
			Types.Append( (object::type)count );
		}
	}
	return;
}

//==============================================================================

object::type obj_mgr::GetTypeFromName( const char* pName )
{
    for( s32 i=0; i<object::TYPE_END_OF_LIST; i++ )
    {
        if( m_ObjectType[i].pTypeName )
        {
            if( x_stricmp(pName, m_ObjectType[i].pTypeName ) == 0 )
                return (object::type)i;
        }
    }

    return object::TYPE_NULL;
}

//==============================================================================

const char* obj_mgr::GetNameFromType( object::type Type )
{
    ASSERT( (Type>=object::TYPE_NULL) && (Type<object::TYPE_END_OF_LIST) );
    if( m_ObjectType[Type].pTypeName )
        return m_ObjectType[Type].pTypeName;

    return NULL;
}

//==============================================================================
//==============================================================================
//==============================================================================
// VISIBILITY
//==============================================================================
//==============================================================================
//==============================================================================

void obj_mgr::SolveVisibleObjects( const view& View, xbool DoCulling )
{

	xtimer Time;

    Time.Start();

//    m_VisSequence++;

    // Reset the Vis List
    for ( s32 count = 0; count < MAX_OBJECTS; count++)
        m_ObjectSlot[count].NextVis = -1;
    
    g_SpatialDBase.BuildVisList( &View, DoCulling );

    spatial_cell_id CI = g_SpatialDBase.GetFirstVisCell();

    // Clear vis lists
    for( s32 i=0; i<object::TYPE_END_OF_LIST; i++ )
    {
        m_ObjectType[i].FirstVis = -1;
        m_ObjectType[i].nVis = 0;
    }

	
    s32 C1=0;
    // Loop through all visible nodes that contain objects
    while( CI != SPATIAL_CELL_NULL )
    {
        ASSERT( (C1++)<5000 );
            
        spatial_cell* pCell = g_SpatialDBase.GetCell( CI );

        // Check if there are any objects in cell
        if( pCell->OccFlags )
        {
            s32 Channel;
            for( Channel=0; Channel<NUM_SPATIAL_CHANNELS; Channel++ )
            {
	            s32 LI = pCell->FirstObjectLink[ Channel ];
                s32 C2=0;
                while( LI != -1 )
                {
                    ASSERT( (C2++)<5000 );

	                    // Get object index
                    s32 I = LI / 8;
                
                    if(m_ObjectSlot[I].pObject->GetType() != -1)
                    {
                        xbool Hidden = FALSE;

                        // If in editor check if it is hidden
                        #ifdef EDITOR
                        Hidden = m_ObjectSlot[I].pObject->GetHidden();
                        #endif

                        // Check if object is already in list
                        if( !Hidden )
                        {
                            //  Make sure it hasn't been added to this list so we don't try and render twice.
                            if( m_ObjectSlot[count].NextVis == -1 )
                            {
//                                m_ObjectSlot[I].VisSequence = m_VisSequence;
                                m_ObjectSlot[I].NextVis = m_ObjectType[ m_ObjectSlot[I].pObject->GetType() ].FirstVis ;
                                m_ObjectType[ m_ObjectSlot[I].pObject->GetType() ].FirstVis = I ;
                                m_ObjectType[ m_ObjectSlot[I].pObject->GetType() ].nVis++ ;
                            }
                        }
                    }
                    // Move to next link for this cell
                    ASSERT(LI != m_ObjectLink[LI].Next);
                    LI = m_ObjectLink[LI].Next;

	            }
            }
	    }

	
        CI = pCell->VisNext;
    }

	
    Time.Stop();


}

//==============================================================================

 
slot_ID obj_mgr::GetFirstVis( object::type Type )
{
    return m_ObjectType[Type].FirstVis;


}


//==============================================================================

slot_ID obj_mgr::GetNextVis( slot_ID SlotID )
{
    ASSERT(m_ObjectSlot[ SlotID ].pObject );

    return m_ObjectSlot[ SlotID ].NextVis;

}



//==============================================================================
void obj_mgr::RemoveFromSpatialDBase(guid Guid)
{
    RemoveFromSpatialDBase(GetSlotFromGuid(Guid) );

}


void obj_mgr::AddToSpatialDBase(guid Guid)
{
    AddToSpatialDBase(GetSlotFromGuid(Guid) );

}


//==============================================================================
//
//  Removes the object in the slot from the Spatial database
//
//==============================================================================

void obj_mgr::RemoveFromSpatialDBase(slot_ID SlotID)
{
    obj_cell_link* pLink = GetCellLinks();

    // Remove ourselves
    for( s32 i=0; i<8; i++ )
    {
        // Get index to this link
        s32 LI = SlotID*8+i;

        // Check if unused
        if( pLink[LI].CellID == SPATIAL_CELL_NULL )
            break;

        // Get access to cell
        spatial_cell* pCell = (spatial_cell*)g_SpatialDBase.GetCell( pLink[LI].CellID );

        // Remove from linked list
        if( pCell->FirstObjectLink[0] == LI )
            pCell->FirstObjectLink[0] = pLink[LI].Next;

        if( pLink[LI].Next != -1 )
            pLink[ pLink[LI].Next ].Prev = pLink[LI].Prev;

        if( pLink[LI].Prev != -1 )
            pLink[ pLink[LI].Prev ].Next = pLink[LI].Next;

        // Check if list is empty
        if( pCell->FirstObjectLink[0] == -1 )
        {
            pCell->OccFlags &= (~(1<<0));
            g_SpatialDBase.UpdateCell( pCell );
        }

        pLink[LI].CellID = SPATIAL_CELL_NULL;
        pLink[LI].Next = -1;
        pLink[LI].Prev = -1;
    }
}


//==============================================================================
//
//  Removes the object in the slot from the Spatial database
//
//==============================================================================

void obj_mgr::AddToSpatialDBase(slot_ID SlotID)
{
    // Update spatial database
    s32 i;
    s32 X,Y,Z;
    s32 MinX,MinY,MinZ;
    s32 MaxX,MaxY,MaxZ;
    s32 Level;

    // Get cell regions affected
    Level = g_SpatialDBase.GetBBoxLevel( GetObjectBySlot(SlotID)->GetBBox() );
    g_SpatialDBase.GetCellRegion( GetObjectBySlot(SlotID)->GetBBox() , Level, MinX, MinY, MinZ, MaxX, MaxY, MaxZ );

    obj_cell_link* pLink = g_ObjMgr.GetCellLinks();

    for( i=0; i<8; i++ )
    {
        s32 LI = SlotID*8+i;
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
        s32 LI = SlotID*8+i;
        ASSERT( i<8 );

        // Get access to cell
        pLink[LI].CellID = g_SpatialDBase.GetCellIndex( X, Y, Z, Level, TRUE );

        spatial_cell* pCell = (spatial_cell*)g_SpatialDBase.GetCell( pLink[LI].CellID );

        // Add link to list
        pLink[LI].Prev = -1;
        ASSERT(LI != pCell->FirstObjectLink[0]);
        pLink[LI].Next = pCell->FirstObjectLink[0];        
        if( pCell->FirstObjectLink[0] != -1 )
            pLink[ pCell->FirstObjectLink[0] ].Prev = LI;
        pCell->FirstObjectLink[0] = LI;

        // Turn on occupied flag
        pCell->OccFlags |= (1<<0);

        // Move to next link
        i++;
    }
}












/*





#ifdef EDITOR
//==============================================================================

void obj_mgr::AdjustLayer(xwstring LayerName)
{       
    if(LayerName.GetLength() == 0)
        return;
    xbool AllHidden = TRUE;
    xbool AllNotHidden = TRUE;
    xbool AllUnselectable = TRUE;
    xbool AllNotUnselectable = TRUE;

    guid Guid;
	g_ObjMgr.StartLayerLoop( LayerName );
	while( Guid = g_ObjMgr.GetNextInLayer() )
	{
		object* pObject = (object*)g_ObjMgr.LockObject( Guid );
		ASSERT(pObject);
		
        if(pObject->GetHidden())
        {
            AllNotHidden = FALSE;
        }
        else
        {
            AllHidden = FALSE;
        }
        if(pObject->GetUnselectable())
        {
            AllNotUnselectable = FALSE;
        }
        else
        {
            AllUnselectable = FALSE;
        }
		g_ObjMgr.UnlockObject( Guid );
	}
	g_ObjMgr.EndLayerLoop();   
    if(AllHidden)
    {
        g_ObjMgr.SetHidden(LayerName, TRUE, FALSE);
    }
    else if(AllNotHidden)
    {
        g_ObjMgr.SetHidden(LayerName, FALSE, FALSE);
    }
    else
    {
        g_ObjMgr.SetHidden(LayerName, -1, FALSE);
    }
    if(AllUnselectable)
    {
        g_ObjMgr.SetUnselectable(LayerName, TRUE, FALSE);
    }
    else if(AllNotUnselectable)
    {
        g_ObjMgr.SetUnselectable(LayerName, FALSE, FALSE);
    }
    else
    {
        g_ObjMgr.SetUnselectable(LayerName, -1, FALSE);
    }
}

void obj_mgr::SetUnselectable( object::type ObjType, xbool Flag, xbool AdjustObjects )
{
    m_Unselectable[ObjType] = Flag;
    if(!AdjustObjects) return;
    //Set all objects of this type to unselectable
    guid Guid;
	g_ObjMgr.StartTypeLoop( ObjType );
    s32 C1=0;
	xarray<xwstring> LayerNames;
	while( Guid = g_ObjMgr.GetNextInType() )
	{
        ASSERT( (C1++)<5000 );

		object* pObject = (object*)g_ObjMgr.LockObject( Guid );
		ASSERT(pObject);
		
        pObject->SetUnselectable(Flag, FALSE, FALSE);  //Don't cascade back up here
	    xwstring LayerName = pObject->GetLayer();
        if(LayerNames.Find(LayerName) == -1)
            LayerNames.Append(LayerName);
        g_ObjMgr.UnlockObject( Guid );
	}
	g_ObjMgr.EndTypeLoop();    
    for(s32 i = 0; i < LayerNames.GetCount(); i++)
    {
        AdjustLayer(LayerNames.GetAt(i));
    }  
}

s32 obj_mgr::GetUnselectable( object::type ObjType )
{
    return m_Unselectable[ObjType];
}

void obj_mgr::SetHidden( object::type ObjType, xbool Flag, xbool AdjustObjects )
{
    m_Hidden[ObjType] = Flag;
    if(!AdjustObjects) return;
    //Set all objects of this type to hidden
    guid Guid;
	g_ObjMgr.StartTypeLoop( ObjType );
    s32 C1=0;
    xarray<xwstring> LayerNames;
	while( Guid = g_ObjMgr.GetNextInType() )
	{
        ASSERT( (C1++)<5000 );
		object* pObject = (object*)g_ObjMgr.LockObject( Guid );
		ASSERT(pObject);
		
        pObject->SetHidden(Flag, FALSE, FALSE);  //Don't cascade back up here
        xwstring LayerName = pObject->GetLayer();
        if(LayerNames.Find(LayerName) == -1)
            LayerNames.Append(LayerName);
	    g_ObjMgr.UnlockObject( Guid );
	}
	g_ObjMgr.EndTypeLoop();   
    for(s32 i = 0; i < LayerNames.GetCount(); i++)
    {
        AdjustLayer(LayerNames.GetAt(i));
    }
}

s32 obj_mgr::GetHidden( object::type ObjType )
{
    return m_Hidden[ObjType];
}

void obj_mgr::SetUnselectable( const xwstring& LayerFileName, xbool Flag, xbool AdjustObjects )
{
    ASSERT(LayerFileName.GetLength() > 0);
    m_LayerUnselectable.GetAt(m_LayerFileNames.Find(LayerFileName)) = Flag;
    if(!AdjustObjects) return;
    //Set all objects of this type to unselectable
    guid Guid;
	g_ObjMgr.StartLayerLoop( LayerFileName );
    s32 C1=0;
	while( Guid = g_ObjMgr.GetNextInLayer() )
	{
        ASSERT( (C1++)<5000 );
		object* pObject = (object*)g_ObjMgr.LockObject( Guid );
		ASSERT(pObject);
		
        pObject->SetUnselectable(Flag, TRUE, FALSE);  //Don't cascade back up here
	    g_ObjMgr.UnlockObject( Guid );
	}
	g_ObjMgr.EndLayerLoop();   
}

s32 obj_mgr::GetUnselectable( const xwstring& LayerFileName )
{
    return m_LayerUnselectable.GetAt(m_LayerFileNames.Find(LayerFileName));
}

void obj_mgr::SetHidden( const xwstring& LayerFileName, xbool Flag, xbool AdjustObjects )
{
    m_LayerHidden.GetAt(m_LayerFileNames.Find(LayerFileName)) = Flag;
    if(!AdjustObjects) return;
    //Set all objects of this type to hidden
    guid Guid;
	g_ObjMgr.StartLayerLoop( LayerFileName );
    s32 C1=0;
	while( Guid = g_ObjMgr.GetNextInLayer() )
	{
        ASSERT( (C1++)<5000 );
		object* pObject = (object*)g_ObjMgr.LockObject( Guid );
		ASSERT(pObject);
		
        pObject->SetHidden(Flag, TRUE, FALSE);  //Don't cascade back up here
	    g_ObjMgr.UnlockObject( Guid );
	}
	g_ObjMgr.EndLayerLoop();   
}

s32 obj_mgr::GetHidden( const xwstring& LayerFileName )
{
    return m_LayerHidden.GetAt(m_LayerFileNames.Find(LayerFileName));
}

void obj_mgr::SetAllUnselectable( s32 Flag )
{
    for(u32 count = 0; count < object::TYPE_END_OF_LIST; count++)
	{
		obj_type_info* pInfo = m_pObjTypeInfo[count];
		if(pInfo)
		{
            SetUnselectable((object::type) count, Flag);
        }
    }
}
 
s32 obj_mgr::GetAllUnselectable  ( void )
{
    s32 Result = -1;
    for(u32 count = 0; count < object::TYPE_END_OF_LIST; count++)
	{
		obj_type_info* pInfo = m_pObjTypeInfo[count];
		if(pInfo)
		{
            s32 Val = GetUnselectable((object::type) count);
            if(Val == -1) return -1;
            if(Result == -1)
                Result = Val;
            else
                if(Result != Val)
                    return -1;
        }
    }
    return Result;
}

void obj_mgr::SetAllHidden( s32 Flag )
{
    for(u32 count = 0; count < object::TYPE_END_OF_LIST; count++)
	{
		obj_type_info* pInfo = m_pObjTypeInfo[count];
		if(pInfo)
		{
            SetHidden((object::type) count, Flag);
        }
    }
}

s32 obj_mgr::GetAllHidden( void )
{
    s32 Result = -1;
    for(u32 count = 0; count < object::TYPE_END_OF_LIST; count++)
	{
		obj_type_info* pInfo = m_pObjTypeInfo[count];
		if(pInfo)
		{
            s32 Val = GetHidden((object::type) count);
            if(Val == -1) return -1;
            if(Result == -1)
                Result = Val;
            else
                if(Result != Val)
                    return -1;
        }
    }
    return Result;
}

void obj_mgr::AddLayer( const xwstring& LayerFileName, const xwstring& LayerTitle )
{
    if(m_LayerFileNames.Find(LayerFileName) < 0)
    {
        m_LayerFileNames.Append(LayerFileName);
        m_LayerTitles.Append(LayerTitle);
        m_LayerHidden.Append(FALSE);
        m_LayerUnselectable.Append(FALSE);
    }
}

void obj_mgr::RemoveLayer( const xwstring& LayerFileName )
{
    s32 Index = m_LayerFileNames.Find(LayerFileName);
    m_LayerFileNames.Delete(Index);
    m_LayerTitles.Delete(Index);
    m_LayerHidden.Delete(Index);
    m_LayerUnselectable.Delete(Index);
}

void obj_mgr::ClearLayers( void )
{
    m_LayerFileNames.Clear();
    m_LayerTitles.Clear();
    m_LayerFileNames.Append(TEXT("Base Layer"));
    m_LayerTitles.Append(TEXT("Base Layer"));
    m_LayerHidden.Clear();
    m_LayerUnselectable.Clear();
    m_LayerHidden.Append(FALSE);
    m_LayerUnselectable.Append(FALSE);
}

void obj_mgr::GetAllLayers( xarray<xwstring> &LayerFileNames, xarray<xwstring> &LayerTitles)
{
    for(s32 count = 0; count < m_LayerFileNames.GetCount(); count++)
    {
        LayerFileNames.Append(m_LayerFileNames.GetAt(count));
        LayerTitles.Append(m_LayerTitles.GetAt(count));
    }
    return;
}

void obj_mgr::StartLayerLoop( const xwstring& LayerFileName )
{
    ASSERT(!m_LayerFull);
    m_LayerFull = TRUE;
    xarray<xwstring> TypeNames;
	xarray<object::type> Types;
	g_ObjMgr.GetAllTypes(TypeNames, Types);
    s32 count;
	for(count = 0; count < TypeNames.GetCount(); count++)
	{
	    g_ObjMgr.StartTypeLoop(Types.GetAt(count));
		guid Guid;
        s32 C1=0;
		while(Guid = g_ObjMgr.GetNextInType())
		{	
            ASSERT( (C1++)<5000 );
			object *pObject = g_ObjMgr.LockObject(Guid);
            xwstring ObjLayerName = pObject->GetLayer();
            if(ObjLayerName == LayerFileName)
                m_LayerGuids.Append(Guid);
			g_ObjMgr.UnlockObject(Guid);
			
		}
		g_ObjMgr.EndTypeLoop();
	}
    m_LayerCursor = 0;
    return;
}

guid obj_mgr::GetNextInLayer( void )
{
    if(!m_LayerFull || (m_LayerCursor >= m_LayerGuids.GetCount()))
        return 0;
    return m_LayerGuids.GetAt(m_LayerCursor++);
}

void obj_mgr::EndLayerLoop( void )
{
    m_LayerFull = FALSE;
    m_LayerGuids.Clear();
}
    
/*void obj_mgr::RenameLayer( xwstring OldLayerName, xwstring NewLayerName )
{
    RemoveLayer(OldLayerName);
    AddLayer(NewLayerName);
    g_ObjMgr.StartLayerLoop(OldLayerName);
    guid Guid;
    while(Guid = g_ObjMgr.GetNextInLayer())
    {
        object *pObject = g_ObjMgr.LockObject(Guid);
        xwstring ObjLayerName = pObject->GetLayer();
        if(ObjLayerName == OldLayerName)
            pObject->SetLayer(NewLayerName);
        g_ObjMgr.UnlockObject(Guid);		
    }
    g_ObjMgr.EndLayerLoop();
}*/

/*
void obj_mgr::SetLayerTitle( const xwstring& LayerFileName, const xwstring& LayerTitle )
{
    s32 Index = m_LayerFileNames.Find(LayerFileName);
    if(Index >= 0)
    {
        m_LayerTitles.GetAt(Index) = LayerTitle;
    }
    
}

xwstring obj_mgr::GetLayerTitle( const xwstring& LayerFileName )
{
    s32 Index = m_LayerFileNames.Find(LayerFileName);
    if(Index >= 0)
    {
        return m_LayerTitles.GetAt(Index);
    }
    return TEXT("");
}
        
#endif //EDITOR

*/










/******************************************************************************

  **    Code Graveyard  **



//==============================================================================
//  STORAGE
//==============================================================================

//------------------------------------------------------------------------------

obj_type_info*   obj_mgr::m_pObjTypeInfo[ object::TYPE_END_OF_LIST ] = { 0 };

//==============================================================================
//  OBJECT TYPE INFO FUNCTIONS
//==============================================================================

obj_type_info::obj_type_info( object::type   aType, 
                              char*          apTypeName, 
                              obj_create_fn* apCreateFn,
                              u32            aDefaultAttrBits,
                              xbool          abDoRunLogic)
{
    Type            = aType;
    pTypeName       = apTypeName;
    pCreateFn       = apCreateFn;
    DefaultAttrBits = aDefaultAttrBits;
    bRunLogic       = abDoRunLogic;
    obj_mgr::m_pObjTypeInfo[ aType ] = this;
}



//==============================================================================
void obj_mgr::PrintStats(void)
{
    x_DebugMsg("\n OBJECT MANAGER STATS                                          \n");
    x_DebugMsg("===============================================================\n");
    x_DebugMsg("%25s     %9d\n", "maxLocksPerObject", Stats.maxLocksPerObject);
    x_DebugMsg("%25s     %9d\n", "maxNumberLockedObjects", Stats.maxNumberLockedObjects);
    x_DebugMsg("%25s     %9d\n", "numLockedObjects", Stats.numLockedObjects);
    x_DebugMsg("%25s     %9d\n", "numPhonyUnlocks", Stats.numPhonyUnlocks);
    x_DebugMsg("%25s     %9d\n", "numTotalLocks", Stats.numTotalLocks);
    
    x_DebugMsg("\n%25s%14s\n", "STATS PER TYPE",  "LOCKED");
    x_DebugMsg("          ====================================================\n");
    for(s32 i = 0; i < object::TYPE_END_OF_LIST;i++)
    {
        if(m_pObjTypeInfo[i])
            x_DebugMsg("%25s%14d\n", m_pObjTypeInfo[i]->pTypeName, Stats.numLockedObjectsPerType[i]);                
    }

}




//==============================================================================

object* obj_mgr::LockObject( guid ObjectGuid )
{
    // Determine slot and return null if doesn't exist
    s32 Slot = GetSlotFromGuid( ObjectGuid );
    if( Slot == -1 )
        return NULL;

#ifdef OBJ_MGR_COLLECT_STATS
    Stats.numTotalLocks++;
    if(m_ObjectSlot[Slot].NLocks == 0)
    {
        Stats.numLockedObjects++;
        Stats.numLockedObjectsPerType[m_ObjectSlot[Slot].pObject->GetType()]++;
    }
    if(Stats.numLockedObjects > Stats.maxNumberLockedObjects)
    {
        Stats.maxNumberLockedObjects = Stats.numLockedObjects;
        //ASSERT(Stats.maxNumberLockedObjects < 30);
    }
#endif
    // Increment lock and return pointer
    m_ObjectSlot[Slot].NLocks++;
#ifdef OBJ_MGR_COLLECT_STATS
    if(m_ObjectSlot[Slot].NLocks > Stats.maxLocksPerObject)
        Stats.maxLocksPerObject = m_ObjectSlot[Slot].NLocks;
#endif

    return m_ObjectSlot[Slot].pObject;
}

//==============================================================================

object* obj_mgr::LockObjectFromSlot( s32 Slot )
{
    // Determine slot and return null if doesn't exist
    //s32 Slot = GetSlotFromGuid( ObjectGuid );
    if( Slot == -1 )
        return NULL;

#ifdef OBJ_MGR_COLLECT_STATS   
    Stats.numTotalLocks++;
    if(m_ObjectSlot[Slot].NLocks == 0)
    {
        Stats.numLockedObjects++;            
        Stats.numLockedObjectsPerType[m_ObjectSlot[Slot].pObject->GetType()]++;
    }
    if(Stats.numLockedObjects > Stats.maxNumberLockedObjects)
        Stats.maxNumberLockedObjects = Stats.numLockedObjects;
#endif
    // Increment lock and return pointer
    m_ObjectSlot[Slot].NLocks++;
#ifdef OBJ_MGR_COLLECT_STATS
    if(m_ObjectSlot[Slot].NLocks > Stats.maxLocksPerObject)
    {
        Stats.maxLocksPerObject = m_ObjectSlot[Slot].NLocks;    
        //ASSERT(Stats.maxNumberLockedObjects < 30);
    }
#endif
    return m_ObjectSlot[Slot].pObject;
}

//==============================================================================

void obj_mgr::UnlockObject( guid ObjectGuid )
{
    // Determine slot and return null if doesn't exist
    s32 Slot = GetSlotFromGuid( ObjectGuid );
    if( Slot != -1 )
    {
        
#ifdef OBJ_MGR_COLLECT_STATS                  
        Stats.numTotalLocks--;
#endif
        // Decrement lock and return
        m_ObjectSlot[Slot].NLocks--;
#ifdef OBJ_MGR_COLLECT_STATS                  
        if(m_ObjectSlot[Slot].NLocks == 0)
        {
            ASSERT(Stats.numLockedObjects > 0);
            Stats.numLockedObjects--;                
            Stats.numLockedObjectsPerType[m_ObjectSlot[Slot].pObject->GetType()]--;
        }
#endif
///        ASSERT( m_ObjectSlot[Slot].NLocks >= 0 );
    }
#ifdef OBJ_MGR_COLLECT_STATS                  
    else
    {
        Stats.numPhonyUnlocks++;
    }
#endif
}

//==============================================================================
void obj_mgr::AssertNoLockedObjects(void)
{
#ifdef OBJ_MGR_COLLECT_STATS
#ifdef CHECK_FOR_LOCKS
#ifdef EDITOR
    if(!g_xwManager.IsPaused())
#endif
    ASSERT(Stats.numLockedObjects == 0);
#endif
#endif
}

//==============================================================================



//==============================================================================

guid obj_mgr::GetNextInType( void )
{
    guid Guid;

    if( m_TypeCursor[m_TypeCursorID] == -1 )
        return 0;

    Guid = m_ObjectSlot[ m_TypeCursor[m_TypeCursorID] ].Guid;
    m_TypeCursor[m_TypeCursorID] = m_ObjectSlot[ m_TypeCursor[m_TypeCursorID] ].Next;

    return Guid;
}

//==============================================================================

object* obj_mgr::GetRawNextInType( void )
{
    if( m_TypeCursor[m_TypeCursorID] == -1 )
        return NULL;

    object* pObject = m_ObjectSlot[ m_TypeCursor[m_TypeCursorID] ].pObject;
    m_TypeCursor[m_TypeCursorID] = m_ObjectSlot[ m_TypeCursor[m_TypeCursorID] ].Next;

    return pObject;
}

//==============================================================================

void obj_mgr::EndTypeLoop( void )
{
    ASSERT( m_TypeCursorID >= 0 );
    m_TypeCursor[m_TypeCursorID] = -1;
    m_TypeCursorID--;
}

//==============================================================================

void obj_mgr::RemoveFromTypeLoops( s32 Slot )
{
    for( s32 i=0; i<=m_TypeCursorID; i++ )
    {
        // Are we deleting the object at the current cursor?
        if( m_TypeCursor[i] == Slot )
        {
            // Move cursor to next object
            m_TypeCursor[i] = m_ObjectSlot[ Slot ].Next;
        }
    }
}


//==============================================================================

void obj_mgr::DisplayStats( void )
{
    s32 L=2;

    x_printfxy( 0, L++, "* OBJECT MANAGER *");
    for( s32 T=0; T<object::TYPE_END_OF_LIST; T++ )
    {
        if( m_pObjTypeInfo[T] )
        {
            x_printfxy( 0, L++, "%3d %4.1f %s", 
                m_ObjectType[T].InstanceCount,
                m_ObjectType[T].LogicTime.ReadMs(),
                m_pObjTypeInfo[T]->pTypeName );
        }
    }
}



//==============================================================================

xbool obj_mgr::ApplyPain( guid Guid, const vector3& Pos, f32 Radius, f32 Pain, f32 Force, u32 Attributes, s32 Type )
{
    s32 nChecked=0;
    s32 nHit=0;

    bbox PainBBox( Pos, Radius );

    // Loop through all objects
    for( s32 i=0; i<MAX_OBJECTS; i++ )
    if( m_ObjectSlot[i].Guid && (m_ObjectSlot[i].Guid!=Guid) )
    {
        nChecked++;
        object* pObj = LockObject( m_ObjectSlot[i].Guid );
        ASSERT( pObj );
        if( pObj->GetAttrBits() & Attributes )
        {
            if( PainBBox.Intersect( pObj->GetBBox() ) )
            {
                nHit++;
                pObj->OnApplyPain( Guid, Pos, Radius, Pain, Force, Type );
            }
        }
        UnlockObject( m_ObjectSlot[i].Guid );
    }

    return (nHit > 0);
}


void obj_mgr::StartVisLoop( object::type Type )
{
    ASSERT( m_InVisLoop == FALSE );
    ASSERT( (Type>=0) && (Type<object::TYPE_END_OF_LIST) );

    m_InVisLoop = TRUE;
    m_VisCursor = m_ObjectType[Type].FirstVis;
}

//==============================================================================

guid obj_mgr::GetNextInVis( void )
{
    ASSERT( m_InVisLoop == TRUE );

    guid Guid;

    if( m_VisCursor == -1 )
        return 0;

    Guid = m_ObjectSlot[ m_VisCursor ].Guid;
    m_VisCursor = m_ObjectSlot[ m_VisCursor ].NextVis;

    return Guid;
}

//==============================================================================

void obj_mgr::EndVisLoop( void )
{
    ASSERT( m_InVisLoop == TRUE );
    m_InVisLoop = FALSE;
    m_VisCursor = -1;
}

//==============================================================================



*/