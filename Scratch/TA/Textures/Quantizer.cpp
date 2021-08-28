//==============================================================================
//
//  Quantizer.cpp
//
//  A vert fast, fleixible, full precision, good quality, color quantizer!
//
//==============================================================================


//==============================================================================
// INCLUDES
//==============================================================================

#include "Quantizer.h"


//==============================================================================
// CLASS color_node
//==============================================================================


// Constructor
color_node::color_node()
{
    m_R = m_G = m_B = m_A = 0 ;
    m_PixelCount = 0 ;
    
    m_pParent          = NULL ;
    m_ppParentChildPtr = NULL ;
    
    for (s32 i = 0 ; i < 16 ; i++)
        m_pChild[i] = NULL ;
    
    m_pNext        = NULL ;
    m_pPrev        = NULL ;
    
    m_ClutIndex   = -1 ;
}

//==============================================================================

// Destructor
color_node::~color_node()
{
    // Check all children
    for (s32 i = 0 ; i < 16 ; i++)
    {
        // Recurse on child?
        if (m_pChild[i])
            delete m_pChild[i] ;
    }
}

//==============================================================================

// Sort by lowest pixel count to highest pixel count
s32 color_node_ptr_compare_fn( const void* pItem1, const void* pItem2 )
{
	color_node *NodeA = *(color_node **)pItem1 ;
	color_node *NodeB = *(color_node **)pItem2 ;

    // Sort by lowest pixel count to biggest pixel count
    if (NodeA->m_PixelCount > NodeB->m_PixelCount)
        return 1 ;
    else
    if (NodeA->m_PixelCount < NodeB->m_PixelCount)
        return -1 ;
    else
        return 0 ;
}

//==============================================================================
// CLASS color_tree
//==============================================================================

// Constructor
color_tree::color_tree()
{
    m_Size      = 1 ;
    m_pRootNode = new color_node() ;
    m_pPalette  = NULL ;
    m_Colors    = 0 ;
}

//==============================================================================

// Destructor
color_tree::~color_tree()
{
    // Cleanup nodes
    if (m_pRootNode)
        delete m_pRootNode ;

    // Cleanup palette
    if (m_pPalette)
        delete m_pPalette ;
}

//==============================================================================

// Adds color to tree
void color_tree::AddColor( const xcolor& C )
{
    s32         Level   = 7 ;
    color_node* Node    = m_pRootNode ;

    // Search through nodes or create them if they don't exist
    while(Level >= 0)
    {
        // Which cube?
        s32 Index =    (((C.R >> Level) & 1) << 0)
                    |  (((C.G >> Level) & 1) << 1)
                    |  (((C.B >> Level) & 1) << 2)
                    |  (((C.A >> Level) & 1) << 3) ;

        // Already there?
        if (Node->m_pChild[Index])
            Node = Node->m_pChild[Index] ;
        else
        {
            // Create new node
            color_node* Child = new color_node ;
            ASSERT(Child) ;
       
            // Setup parent references
            Child->m_pParent          = Node ;
            Child->m_ppParentChildPtr = &Node->m_pChild[Index] ;
        
            // Setup parent references
            Node->m_pChild[Index] = Child ;

            // Search next
            Node    = Child ;
        }

        // Next level
        Level-- ;
    }

    // We are now at the final node, so update the stats!
    ASSERT(Node) ;

    // Add to level0 list?
    if (Node->m_PixelCount == 0)
    {
        m_LevelList[0].Append(Node) ;
        m_Size++ ;
    }

    // Update node stats
    Node->m_R += C.R ;
    Node->m_G += C.G ;
    Node->m_B += C.B ;
    Node->m_A += C.A ;
    Node->m_PixelCount++ ;
}

//==============================================================================

// Sorts a level list from lowest (head of list) to highest (tail of list) pixel count
void color_tree::SortLevelList(s32 Level)
{
    // Make sure level is valid
    ASSERT(Level >= 0) ;
    ASSERT(Level < 8) ;

    // Create array of pointers
    s32          Size  = m_LevelList[Level].GetSize() ;
    color_node** Array = new color_node*[Size] ;
    ASSERT(Array) ;
    
    // Fill array with list
    color_node* Node  = m_LevelList[Level].GetHead() ;
    s32         Index = 0 ;
    while(Node)
    {
        Array[Index++] = Node ;
        Node = Node->m_pNext ;
    }

    // Sort the array by lowest to highest pixel count
    if (Size)
    {
        x_qsort(Array,			            // Address of first item in array.
			  Size,			                // Number of items in array.
              sizeof(color_node**),	        // Size of one item.
			  color_node_ptr_compare_fn) ;  // Compare function.
    }

    // Now put the array back into the list...
    m_LevelList[Level].Clear() ;
    for (Index = 0 ; Index < Size ; Index++)
        m_LevelList[Level].Append(Array[Index]) ;

    // Finally, delete the array
    delete [] Array ;
}

//==============================================================================

// Reduces tree to required colors, and sets up clut
void color_tree::Quantize( s32 Colors )
{
    // Always reduce nodes from the lowest level first (least significant)
    s32         Level    = 0 ;
    color_node* BestNode = NULL ;
    //s32         End      = 0 ;

    // Reduce colors?
    while(m_Size > Colors)
    {
        // Get least used color node at current level
        BestNode = m_LevelList[Level].GetHead() ;
        while(BestNode == NULL)
        {
            // Check next level and sort it from lowest to highest pixel count
            Level++ ;
            //SortLevelList(Level) ;

            // Pick lowest used color node
            //End ^= 1 ;
            //if (End)
                BestNode = m_LevelList[Level].GetHead() ;
            //else
                //BestNode = LevelList[Level].GetTail() ;
        }

        // Make sure node is valid
        ASSERT(BestNode) ;
        ASSERT(Level >= 0) ;
        ASSERT(Level < 7) ;

        // Collapse node(s) into parent
        color_node* Parent = BestNode->m_pParent ;
        ASSERT(Parent) ;

        ASSERT(BestNode->m_pParent == Parent) ;
        ASSERT(BestNode->m_PixelCount) ;

        // Add parent to lists?
        if (Parent->m_PixelCount == 0)
        {
            m_LevelList[Level+1].Append(Parent) ;
            m_Size++ ;
        }

        // Collapse all the children?
        for (s32 i = 0 ; i < 16 ; i++)
        {
            BestNode = Parent->m_pChild[i] ;
            if (BestNode)
            {
                // Update parent stats
                Parent->m_R += BestNode->m_R ;
                Parent->m_G += BestNode->m_G ;
                Parent->m_B += BestNode->m_B ;
                Parent->m_A += BestNode->m_A ;
                Parent->m_PixelCount += BestNode->m_PixelCount ;
                *BestNode->m_ppParentChildPtr = NULL ;

                // Delete from level list
                m_LevelList[Level].Delete(BestNode) ;
                m_Size-- ;

                // Finally, delete the node!
                delete BestNode ;

                // Remove reference from parent
                Parent->m_pChild[i] = NULL ;
            }
        }
    }

    // Make sure there isn't a palette already!
    ASSERT(m_pPalette == NULL) ;
    ASSERT(m_Colors == 0) ;

    // Create the palette and set to white
    m_Colors   = Colors ;
    m_pPalette = new xcolor[Colors] ;
    for (s32 i = 0 ; i < Colors ; i++)
        m_pPalette[i] = xcolor(255,255,255,255);

    // Now create the palette
    s32 Index = 0 ;
    for (Level = 0 ; Level < 8 ; Level++)
    {
        color_node* Node = m_LevelList[Level].GetHead() ;
        while(Node)
        {
            // Set index
            Node->m_ClutIndex = Index ;
        
            // Calculate average color for clut
            ASSERT(Node->m_PixelCount) ;

            // Set the palette entry
            m_pPalette[Index].R = (u8)(Node->m_R / Node->m_PixelCount) ;
            m_pPalette[Index].G = (u8)(Node->m_G / Node->m_PixelCount) ;
            m_pPalette[Index].B = (u8)(Node->m_B / Node->m_PixelCount) ;
            m_pPalette[Index].A = (u8)(Node->m_A / Node->m_PixelCount) ;

            // Next color
            Node = Node->m_pNext ;
            Index++ ;
        }
    }

    // Okay?
    ASSERT(Index <= Colors) ;
}

//==============================================================================

// Returns clut entry color
xcolor color_tree::GetClutColor( s32 Index )
{
    ASSERT(Index >= 0) ;
    ASSERT(Index < m_Colors) ;
    return m_pPalette[Index] ;
}

//==============================================================================

// Returns clut index of color to use
// ("Quantize" must be called before using this function)
s32 color_tree::GetClutIndex ( const xcolor& C )
{
    // Make sure quantize was called
    ASSERT(m_pPalette) ;
    ASSERT(m_Colors) ;

    // Start at top level
    s32         Level=7 ;
    color_node* SearchNode = m_pRootNode ;

    // Loop through all levels finding the best matching color
    while(Level >=0)
    {
        // Which cube?
        s32 Index =    (((C.R >> Level) & 1) << 0)
                    |  (((C.G >> Level) & 1) << 1)
                    |  (((C.B >> Level) & 1) << 2)
                    |  (((C.A >> Level) & 1) << 3) ;

        // Child in tree?
        if (SearchNode->m_pChild[Index])
            SearchNode = SearchNode->m_pChild[Index] ;
        else
            break ; // Skip out of while loop

        Level-- ;
    }

    // Make sure color was found...
    ASSERT(SearchNode) ;
    ASSERT(SearchNode->m_PixelCount) ;
    ASSERT(SearchNode->m_ClutIndex != -1) ;

    // Return final index
    return SearchNode->m_ClutIndex ;
}

//======================================================================
