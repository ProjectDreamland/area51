//==============================================================================
//
// Quantizer.h
//
//==============================================================================

#ifndef __QUANTIZER_H__
#define __QUANTIZER_H__

//==============================================================================
// INCLUDES
//==============================================================================

#include "x_files\x_files.hpp"
#include "List.h"
#define offsetof(s,m)   (size_t)&(((s *)0)->m)

//==============================================================================
// CLASSES
//==============================================================================

// Color node - stores info about tree color entry
class color_node
{
public:
    // Data
    s32             m_R, m_G, m_B, m_A ;    // Color totals
    s32             m_PixelCount ;          // Total pixels using this color
                                        
    color_node*     m_pParent ;             // Parent node
    color_node**    m_ppParentChildPtr ;    // Parents child ptr that points to this node

    color_node*     m_pChild[16] ;          // List of children

    color_node*     m_pNext ;               // Next in level list
    color_node*     m_pPrev ;               // Previous level list
                                       
    s32             m_ClutIndex ;           // Index for final palette

    // Constructor
    color_node() ;

    // Destructor
    ~color_node() ;
} ;

//==============================================================================

// Color tree
class color_tree
{
private:
    // Data
    color_node* m_pRootNode ;      // Root node

    // Level lists (1 level per bit of color component)
    list<color_node, offsetof(color_node, m_pNext), offsetof(color_node, m_pPrev)>  m_LevelList[8] ;

    s32         m_Size ;            // Size of tree

    xcolor*     m_pPalette ;        // Palette of colors
    s32         m_Colors ;          // # of colors in palette

public:
    // Constructor
    color_tree() ;

    // Destructor
    ~color_tree() ;

private:
    // Sorts a level list from lowest (head of list) to highest (tail of list) pixel count
    void SortLevelList(s32 Level) ;

public:
    // Adds color to tree
    void AddColor( const xcolor& C ) ;
   
    // Reduces tree to required colors, and sets up clut
    void Quantize( s32 Colors ) ;

    // Returns clut entry color
    xcolor GetClutColor( s32 Index ) ;

    // Returns clut index of color to use
    // ("Quantize" must be called before using this function)
    s32 GetClutIndex ( const xcolor& C ) ;
} ;

//==============================================================================



#endif  //#ifndef __QUANTIZER_H__
