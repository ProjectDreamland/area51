
#include "x_files.hpp"
#include "x_time.hpp"
#include "e_ScratchMem.hpp"

//==============================================================================

static const s32 kMaxSortItems = 16384;

struct sort_data
{
    u32             SortKey;
    u32             GeomPtr;
    u32             MatrixPtr;
    u32             ColorPtr;
    u32             Type;
    u32             Material;
    u32             Flags;
    u32             SubMesh;
    u32             UVOffsetX;
    u32             UVOffsetY;
    u32             Lighting;
};

//==============================================================================

void ClearSortData( sort_data& Data )
{
    Data.SortKey     = 0;
    Data.GeomPtr     = 0;
    Data.MatrixPtr   = 0;
    Data.ColorPtr    = 0;
    Data.Type        = 0;
    Data.Material    = 0;
    Data.Flags       = 0;
    Data.SubMesh     = 0;
    Data.UVOffsetX   = 0;
    Data.UVOffsetY   = 0;
    Data.Lighting    = 0;
}

//==============================================================================

void AccessSortData( sort_data& Data )
{
    Data.GeomPtr     = 0;
    Data.MatrixPtr   = 0;
    Data.ColorPtr    = 0;
    Data.Type        = 0;
    Data.Material    = 0;
    Data.Flags       = 0;
    Data.SubMesh     = 0;
    Data.UVOffsetX   = 0;
    Data.UVOffsetY   = 0;
    Data.Lighting    = 0;
}

//==============================================================================

typedef void init_data_fn   ( void );
typedef void insert_data_fn ( u32* pKeys, s32 NKeys );
typedef void sort_data_fn   ( void );
typedef void walk_data_fn   ( void );
                            
//==============================================================================
// Simple qsort
//==============================================================================

namespace qsort
{
    static xarray<sort_data> s_List;

    //==============================================================================

    void init( void )
    {
        s_List.SetCapacity(kMaxSortItems);
    }

    //==============================================================================

    void insert( u32* pKeys, s32 NKeys )
    {
        s_List.Delete( 0, s_List.GetCount() );

        for ( s32 i = 0; i < NKeys; i++ )
        {
            sort_data& Data = s_List.Append();
            ClearSortData(Data);
            Data.SortKey = pKeys[i];
        }
    }

    //==============================================================================

    s32 compare( const void* pA, const void* pB )
    {
        sort_data* pSortA = (sort_data*)pA;
        sort_data* pSortB = (sort_data*)pB;
        if ( pSortA->SortKey < pSortB->SortKey )
            return -1;
        if ( pSortA->SortKey > pSortB->SortKey )
            return 1;
        return 0;
    };

    //==============================================================================

    void sort( void )
    {
        x_qsort( s_List.GetPtr(), s_List.GetCount(), sizeof(sort_data), compare );
    }

    //==============================================================================

    void walk( void )
    {
        s32 NAccess = 0;
        for ( s32 i = 0; i < s_List.GetCount(); i++ )
        {
            sort_data& Data = s_List[i];
            AccessSortData(Data);
            NAccess++;
        }

        ASSERT( NAccess == s_List.GetCount() );
    }
};

//==============================================================================
// qsort an array of pointers
//==============================================================================

namespace qsortptr
{
    struct sort_struct
    {
        u32        Key;
        sort_data* pData;
    };

    static xarray<sort_data>   s_List;
    static xarray<sort_struct> s_SortData;

    //==============================================================================

    void init( void )
    {
        s_List.SetCapacity(kMaxSortItems);
        s_SortData.SetCapacity(kMaxSortItems);
    }

    //==============================================================================

    void insert( u32* pKeys, s32 NKeys )
    {
        s_List.Delete( 0, s_List.GetCount() );
        s_SortData.Delete( 0, s_SortData.GetCount() );

        for ( s32 i = 0; i < NKeys; i++ )
        {
            sort_data& Data = s_List.Append();
            ClearSortData(Data);
            Data.SortKey = pKeys[i];

            sort_struct& SortStruct = s_SortData.Append();
            SortStruct.Key   = Data.SortKey;
            SortStruct.pData = &Data;
        }
    }

    //==============================================================================

    s32 compare( const void* pA, const void* pB )
    {
        sort_struct* pSortA = (sort_struct*)pA;
        sort_struct* pSortB = (sort_struct*)pB;
        if ( pSortA->Key < pSortB->Key )
            return -1;
        if ( pSortA->Key > pSortB->Key )
            return 1;
        return 0;
    };

    //==============================================================================

    void sort( void )
    {
        x_qsort( s_SortData.GetPtr(), s_SortData.GetCount(), sizeof(sort_struct), compare );
    }

    //==============================================================================

    void walk( void )
    {
        s32 NAccess = 0;
        for ( s32 i = 0; i < s_SortData.GetCount(); i++ )
        {
            sort_struct& SortData = s_SortData[i];

            sort_data& Data = *SortData.pData;
            AccessSortData(Data);
            NAccess++;
        }

        ASSERT( NAccess == s_List.GetCount() );
    }
};

//==============================================================================
// radix sort ptr
//==============================================================================

namespace bucktptr
{
    struct sort_struct
    {
        u32        Key;
        sort_data* pData;
    };

    static xarray<sort_data>   s_List;
    static xarray<sort_struct> s_SortData;

    //==============================================================================

    void init( void )
    {
        s_List.SetCapacity(kMaxSortItems);
        s_SortData.SetCapacity(kMaxSortItems);
    }

    //==============================================================================

    void insert( u32* pKeys, s32 NKeys )
    {
        s_List.Delete( 0, s_List.GetCount() );
        s_SortData.Delete( 0, s_SortData.GetCount() );

        for ( s32 i = 0; i < NKeys; i++ )
        {
            sort_data& Data = s_List.Append();
            ClearSortData(Data);
            Data.SortKey = pKeys[i];

            sort_struct& SortStruct = s_SortData.Append();
            SortStruct.Key   = Data.SortKey;
            SortStruct.pData = &Data;
        }
    }

    //==============================================================================

    void sort( void )
    {
        static const s32 kBuckets    = 32;
        static const s32 kSortBits   = 5;
        static const s32 kSortPasses = 6;   // 5-bits per pass...30-bit key
        s32 BucketSize = s_SortData.GetCount();

        smem_StackPushMarker();
        sort_struct* pBuckets = (sort_struct*)smem_StackAlloc(kBuckets*BucketSize*sizeof(sort_struct));
        s32          BucketCounts[kBuckets];
        
        u32 Mask  = 0x1f;
        u32 Shift = 0;
        for ( s32 iPass = 0; iPass < kSortPasses; iPass++ )
        {
            // empty the buckets
            for ( s32 iCount = 0; iCount < kBuckets; iCount++ )
            {
                BucketCounts[iCount] = 0;
            }
    
            // move the sort data into the buckets
            for ( s32 iSortData = 0; iSortData < s_SortData.GetCount(); iSortData++ )
            {
                sort_struct& Data = s_SortData[iSortData];
                s32 Bucket = (Data.Key & Mask)>>Shift;
                ASSERT( (Bucket >= 0) && (Bucket < kBuckets) );
                pBuckets[BucketSize*Bucket + BucketCounts[Bucket]++] = Data;
                ASSERT( BucketCounts[Bucket] <= BucketSize );
            }

            // empty the buckets
            s_SortData.Delete(0, s_SortData.GetCount());
            for ( s32 iBucket = 0; iBucket < kBuckets; iBucket++ )
            {
                for ( s32 iData = 0; iData < BucketCounts[iBucket]; iData++ )
                {
                    s_SortData.Append( pBuckets[BucketSize*iBucket + iData] );
                }
            }
            ASSERT( s_SortData.GetCount() == s_List.GetCount() );

            // move to the next set of bits
            Mask <<= kSortBits;
            Shift += kSortBits;
        }

        smem_StackPopToMarker();
    }

    //==============================================================================

    void walk( void )
    {
        s32 NAccess = 0;
        for ( s32 i = 0; i < s_SortData.GetCount(); i++ )
        {
            sort_struct& SortData = s_SortData[i];

            sort_data& Data = *SortData.pData;
            AccessSortData(Data);
            NAccess++;
        }

        ASSERT( NAccess == s_List.GetCount() );
    }
};


//==============================================================================
// binary tree
//==============================================================================

namespace bintree
{
    struct tree_data
    {
        sort_data Sort;
        s16       Left;
        s16       Right;
    };

    static xarray<tree_data> s_Tree;

    //==============================================================================

    void init( void )
    {
        s_Tree.SetCapacity(kMaxSortItems);
    }

    //==============================================================================

    void AddToBinaryTree( tree_data& Data, s32 Pos )
    {
        // nothing to do if it's the root
        if ( Pos == 0 )
            return;

        // walk the tree and add it
        s32   CurrPos  = 0;
        xbool Inserted = FALSE;
        while ( !Inserted )
        {
            tree_data& CompareNode = s_Tree[CurrPos];
            if ( Data.Sort.SortKey < CompareNode.Sort.SortKey )
            {
                if ( CompareNode.Left == -1 )
                {
                    CompareNode.Left = (s16)Pos;
                    Inserted         = TRUE;
                }
                else
                {
                    CurrPos = CompareNode.Left;
                }
            }
            else
            {
                if ( CompareNode.Right == -1 )
                {
                    CompareNode.Right = (s16)Pos;
                    Inserted          = TRUE;
                }
                else
                {
                    CurrPos = CompareNode.Right;
                }
            }
        }
    }

    //==============================================================================

    void insert( u32* pKeys, s32 NKeys )
    {
        s_Tree.Delete(0, s_Tree.GetCount());

        for ( s32 i = 0; i < NKeys; i++ )
        {
            tree_data& Data = s_Tree.Append();
            ClearSortData( Data.Sort );
            Data.Sort.SortKey = pKeys[i];
            Data.Left         = -1;
            Data.Right        = -1;
            AddToBinaryTree(Data, i);
        }
    }

    //==============================================================================

    void sort( void )
    {
    }

    //==============================================================================

    void walk( void )
    {
        smem_StackPushMarker();
        s16* Stack = (s16*)smem_StackAlloc(sizeof(s16)*s_Tree.GetCount());
        s16 StackDepth = -1;
        s32 NAccess    = 0;

        s32 CurrPos = 0;
        while ( CurrPos != -1 )
        {
            // handle the left node first
            tree_data& Data = s_Tree[CurrPos];
            if ( Data.Left != -1 )
            {
                Stack[++StackDepth] = CurrPos;
                CurrPos = Data.Left;
                continue;
            }
        
            // handle this node
            AccessSortData( Data.Sort );
            NAccess++;

            // handle the right node
            if ( Data.Right != -1 )
            {
                CurrPos = Data.Right;
            }
            else if ( StackDepth != -1 )
            {
                // no right node? Okay, hit the parent
                CurrPos = Stack[StackDepth--];

                // also, don't let the parent hit the left child again, we're not
                // walking over this tree again, so just set that side to -1
                ASSERT( CurrPos >= 0 );
                s_Tree[CurrPos].Left = -1;
            }
            else
            {
                CurrPos = -1;
            }
        }

        smem_StackPopToMarker();

        ASSERT( NAccess == s_Tree.GetCount() );
    }
};

//==============================================================================
// binary tree using next indices for equal sort keys
//==============================================================================

namespace bintree2
{
    struct tree_data
    {
        sort_data Sort;
        s16       Left;
        s16       Right;
        s16       Next;
    };

    static xarray<tree_data> s_Tree;

    //==============================================================================

    void init( void )
    {
        s_Tree.SetCapacity(kMaxSortItems);
    }

    //==============================================================================

    void AddToBinaryTree( tree_data& Data, s32 Pos )
    {
        // nothing to do if it's the root
        if ( Pos == 0 )
            return;

        // walk the tree and add it
        s32   CurrPos  = 0;
        xbool Inserted = FALSE;
        while ( !Inserted )
        {
            tree_data& CompareNode = s_Tree[CurrPos];
            if ( Data.Sort.SortKey == CompareNode.Sort.SortKey )
            {
                Data.Next        = CompareNode.Next;
                CompareNode.Next = Pos;
                Inserted         = TRUE;
            }
            else
            if ( Data.Sort.SortKey < CompareNode.Sort.SortKey )
            {
                if ( CompareNode.Left == -1 )
                {
                    CompareNode.Left = (s16)Pos;
                    Inserted         = TRUE;
                }
                else
                {
                    CurrPos = CompareNode.Left;
                }
            }
            else
            {
                if ( CompareNode.Right == -1 )
                {
                    CompareNode.Right = (s16)Pos;
                    Inserted          = TRUE;
                }
                else
                {
                    CurrPos = CompareNode.Right;
                }
            }
        }
    }

    //==============================================================================

    void insert( u32* pKeys, s32 NKeys )
    {
        s_Tree.Delete(0, s_Tree.GetCount());

        for ( s32 i = 0; i < NKeys; i++ )
        {
            tree_data& Data = s_Tree.Append();
            ClearSortData( Data.Sort );
            Data.Sort.SortKey = pKeys[i];
            Data.Left         = -1;
            Data.Right        = -1;
            Data.Next         = -1;
            AddToBinaryTree(Data, i);
        }
    }

    //==============================================================================

    void sort( void )
    {
    }

    //==============================================================================

    void walk( void )
    {
        smem_StackPushMarker();
        s16* Stack = (s16*)smem_StackAlloc(sizeof(s16)*s_Tree.GetCount());
        s16 StackDepth = -1;
        s32 NAccess    = 0;

        s32 CurrPos = 0;
        while ( CurrPos != -1 )
        {
            // handle the left node first
            tree_data& Data = s_Tree[CurrPos];
            if ( Data.Left != -1 )
            {
                Stack[++StackDepth] = CurrPos;
                CurrPos = Data.Left;
                continue;
            }
        
            // handle this node
            AccessSortData( Data.Sort );
            NAccess++;

            // handle the next node
            if ( Data.Next != -1 )
            {
                tree_data& Next = s_Tree[Data.Next];
                Next.Left       = Data.Left;
                Next.Right      = Data.Right;
                CurrPos         = Data.Next;
            }
            else
            if ( Data.Right != -1 )
            {
                // handle the right node
                CurrPos = Data.Right;
            }
            else if ( StackDepth != -1 )
            {
                // no right node? Okay, hit the parent
                CurrPos = Stack[StackDepth--];

                // also, don't let the parent hit the left child again, we're not
                // walking over this tree again, so just set that side to -1
                ASSERT( CurrPos >= 0 );
                s_Tree[CurrPos].Left = -1;
            }
            else
            {
                CurrPos = -1;
            }
        }

        smem_StackPopToMarker();

        ASSERT( NAccess == s_Tree.GetCount() );
    }
};

//==============================================================================
// red-black tree
//==============================================================================

namespace rbtree
{
    struct tree_data
    {
        enum { RED_NODE = 0, BLACK_NODE };
        
        sort_data Sort;
        s16       Left;
        s16       Right;
        s16       Parent;
        s16       RedBlack;
    };

    static xarray<tree_data> s_Tree;
    static s32               s_Root;

    //==============================================================================

    void init( void )
    {
        s_Tree.SetCapacity(kMaxSortItems);
    }

    //==============================================================================

    void InsertNode( tree_data& Data, s32 Pos )
    {
        // insert the node like you would a normal binary tree
        xbool Inserted = FALSE;
        if ( Pos == 0 )
        {
            // first node added? Then its the root
            s_Root   = 0;
            Inserted = TRUE;
        }
        s32 CurrNode = s_Root;
        while ( !Inserted )
        {
            tree_data& TreeNode = s_Tree[CurrNode];
            if ( Data.Sort.SortKey < TreeNode.Sort.SortKey )
            {
                if ( TreeNode.Left == -1 )
                {
                    TreeNode.Left = Pos;
                    Data.Parent   = CurrNode;
                    Inserted      = TRUE;
                }
                else
                {
                    CurrNode = TreeNode.Left;
                }
            }
            else
            {
                if ( TreeNode.Right == -1 )
                {
                    TreeNode.Right = Pos;
                    Data.Parent    = CurrNode;
                    Inserted       = TRUE;
                }
                else
                {
                    CurrNode = TreeNode.Right;
                }
            }
        }
    }

    //==============================================================================

    void RotateTreeLeft( s32 X )
    {
        tree_data& XInst = s_Tree[X];
        s32        Y     = XInst.Right;
        ASSERT( Y != -1 );
        tree_data& YInst = s_Tree[Y];
    
        // move y's child over
        XInst.Right = YInst.Left;
        if ( YInst.Left != -1 )
        {
            tree_data& Left = s_Tree[(s32)YInst.Left];
            Left.Parent = X;
        }

        // move y up to x's position
        YInst.Parent = XInst.Parent;
        if ( XInst.Parent == -1 )
        {
            s_Root = Y;
        }
        else
        {
            tree_data& Parent = s_Tree[(s32)XInst.Parent];
            if ( X == Parent.Left )
                Parent.Left  = Y;
            else
                Parent.Right = Y;
        }

        // move x down
        YInst.Left   = X;
        XInst.Parent = Y;
    }

    //==============================================================================

    void RotateTreeRight( s32 Y )
    {
        tree_data& YInst = s_Tree[Y];
        s32        X     = YInst.Left;
        ASSERT( X != -1 );
        tree_data& XInst = s_Tree[X];

        // move x's child over
        YInst.Left = XInst.Right;
        if ( XInst.Right != -1 )
        {
            tree_data& Right = s_Tree[(s32)XInst.Right];
            Right.Parent = Y;
        }

        // move x up to y's position
        XInst.Parent = YInst.Parent;
        if ( YInst.Parent == -1 )
        {
            s_Root = X;
        }
        else
        {
            tree_data& Parent = s_Tree[(s32)YInst.Parent];
            if ( Y == Parent.Left )
                Parent.Left  = X;
            else
                Parent.Right = X;
        }

        // move y down
        XInst.Right  = Y;
        YInst.Parent = X;
    }

    //==============================================================================

    void AddToRedBlackTree( tree_data& Data, s32 Pos )
    {
        // insert it like you would a normal binary tree
        InsertNode( Data, Pos );

        // now restore the red-black property
        Data.RedBlack  = tree_data::RED_NODE;
        s32 X          = Pos;
        tree_data* pX  = &s_Tree[X];
        while ( 1 )
        {
            // bail if we're at the root
            if ( X == s_Root )
            {
                ASSERT( pX->Parent == -1 );
                break;
            }

            // get a handy pointer to the parent...we'll be needing that
            ASSERT( pX->Parent != -1 );
            s32 iParent              = pX->Parent;
            tree_data* pParent = &s_Tree[iParent];

            // if the parent's color is black, then the tree is maintained, we can quit
            if ( pParent->RedBlack == tree_data::BLACK_NODE )
            {
                break;
            }

            // get a handy pointer to the grandparent
            ASSERT( pParent->Parent != -1 );
            s32 iGrandParent        = pParent->Parent;
            tree_data* pGrandParent = &s_Tree[iGrandParent];

            // we definitely need to do a fixup to maintain the red-black properties
            if ( iParent == pGrandParent->Left )
            {
                s32 iUncle = pGrandParent->Right;
                if ( (iUncle == -1) || (s_Tree[iUncle].RedBlack == tree_data::BLACK_NODE) )
                {
                    if ( X == pParent->Right )
                    {
                        // case 2: x's uncle is black, x is the right child of its parent
                        X  = iParent;
                        pX = pParent;
                        RotateTreeLeft(X);
                        // after rotation we drop through to case 3
                        ASSERT( pX->Parent != -1 );
                        iParent = pX->Parent;
                        pParent = &s_Tree[iParent];
                        ASSERT( pParent->Parent != -1 );
                        iGrandParent = pParent->Parent;
                        pGrandParent = &s_Tree[iGrandParent];
                    }

                    // case 3: x's uncle is black, x is the left child of its parent
                    pParent->RedBlack      = tree_data::BLACK_NODE;
                    pGrandParent->RedBlack = tree_data::RED_NODE;
                    RotateTreeRight( iGrandParent );
                }
                else
                {
                    ASSERT( iUncle != -1 );
                    tree_data& Uncle = s_Tree[iUncle];
                    ASSERT( Uncle.RedBlack == tree_data::RED_NODE );

                    // case 1: x's uncle is red
                    pParent->RedBlack      = tree_data::BLACK_NODE;
                    Uncle.RedBlack         = tree_data::BLACK_NODE;
                    pGrandParent->RedBlack = tree_data::RED_NODE;
                    X  = iGrandParent;
                    pX = pGrandParent;
                }
            }
            else
            {
                s32 iUncle = pGrandParent->Left;
                if ( (iUncle == -1) || (s_Tree[iUncle].RedBlack == tree_data::BLACK_NODE) )
                {
                    if ( X == pParent->Left )
                    {
                        // case 2: x's uncle is black, x is the left child of its parent
                        X  = iParent;
                        pX = pParent;
                        RotateTreeRight(X);
                        // after rotation we drop through to case 3
                        ASSERT( pX->Parent != -1 );
                        iParent = pX->Parent;
                        pParent = &s_Tree[iParent];
                        ASSERT( pParent->Parent != -1 );
                        iGrandParent = pParent->Parent;
                        pGrandParent = &s_Tree[iGrandParent];
                    }

                    // case 3: x's uncle is black, x is the right child of its parent
                    pParent->RedBlack      = tree_data::BLACK_NODE;
                    pGrandParent->RedBlack = tree_data::RED_NODE;
                    RotateTreeLeft( iGrandParent );
                }
                else
                {
                    ASSERT( iUncle != -1 );
                    tree_data& Uncle = s_Tree[iUncle];
                    ASSERT( Uncle.RedBlack == tree_data::RED_NODE );

                    // case 1: x's uncle is red
                    pParent->RedBlack      = tree_data::BLACK_NODE;
                    Uncle.RedBlack         = tree_data::BLACK_NODE;
                    pGrandParent->RedBlack = tree_data::RED_NODE;
                    X  = iGrandParent;
                    pX = pGrandParent;
                }
            }
        }

        // set the root back to black
        s_Tree[s_Root].RedBlack = tree_data::BLACK_NODE;
    }

    //==============================================================================

    void insert( u32* pKeys, s32 NKeys )
    {
        s_Root = -1;
        s_Tree.Delete(0, s_Tree.GetCount());

        for ( s32 i = 0; i < NKeys; i++ )
        {
            tree_data& Data = s_Tree.Append();
            ClearSortData( Data.Sort );
            Data.Sort.SortKey = pKeys[i];
            Data.Left         = -1;
            Data.Right        = -1;
            Data.Parent       = -1;
            AddToRedBlackTree(Data, i);
        }
    }

    //==============================================================================

    void sort( void )
    {
    }

    //==============================================================================

    void walk( void )
    {
        smem_StackPushMarker();
        s16* Stack = (s16*)smem_StackAlloc(sizeof(s16)*s_Tree.GetCount());
        s16 StackDepth = -1;
        s32 NAccess    = 0;

        s32 CurrPos = s_Root;
        while ( CurrPos != -1 )
        {
            // handle the left node first
            tree_data& Data = s_Tree[CurrPos];
            if ( Data.Left != -1 )
            {
                Stack[++StackDepth] = CurrPos;
                CurrPos = Data.Left;
                continue;
            }
        
            // handle this node
            AccessSortData( Data.Sort );
            NAccess++;

            // handle the right node
            if ( Data.Right != -1 )
            {
                CurrPos = Data.Right;
            }
            else if ( StackDepth != -1 )
            {
                // no right node? Okay, hit the parent
                CurrPos = Stack[StackDepth--];

                // also, don't let the parent hit the left child again, we're not
                // walking over this tree again, so just set that side to -1
                ASSERT( CurrPos >= 0 );
                s_Tree[CurrPos].Left = -1;
            }
            else
            {
                CurrPos = -1;
            }
        }

        smem_StackPopToMarker();

        ASSERT( NAccess == s_Tree.GetCount() );
    }
};

//==============================================================================
// red-black tree with the idea of a "next" node for equal sort keys
//==============================================================================

namespace rbtree2
{
    struct tree_data
    {
        enum { RED_NODE = 0, BLACK_NODE };
        
        sort_data Sort;
        s16       Left;
        s16       Right;
        s16       Parent;
        s16       Next;
        s16       RedBlack;
    };

    static xarray<tree_data> s_Tree;
    static s32               s_Root;

    //==============================================================================

    void init( void )
    {
        s_Tree.SetCapacity(kMaxSortItems);
    }

    //==============================================================================

    xbool InsertNode( tree_data& Data, s32 Pos )
    {
        // insert the node like you would a normal binary tree
        xbool Inserted = FALSE;
        if ( Pos == 0 )
        {
            // first node added? Then its the root
            s_Root   = 0;
            Inserted = TRUE;
        }
        s32 CurrNode = s_Root;
        while ( !Inserted )
        {
            tree_data& TreeNode = s_Tree[CurrNode];
            if ( Data.Sort.SortKey == TreeNode.Sort.SortKey )
            {
                Data.Next     = TreeNode.Next;
                TreeNode.Next = Pos;
                return FALSE;
            }
            else
            if ( Data.Sort.SortKey < TreeNode.Sort.SortKey )
            {
                if ( TreeNode.Left == -1 )
                {
                    TreeNode.Left = Pos;
                    Data.Parent   = CurrNode;
                    Inserted      = TRUE;
                }
                else
                {
                    CurrNode = TreeNode.Left;
                }
            }
            else
            {
                if ( TreeNode.Right == -1 )
                {
                    TreeNode.Right = Pos;
                    Data.Parent    = CurrNode;
                    Inserted       = TRUE;
                }
                else
                {
                    CurrNode = TreeNode.Right;
                }
            }
        }

        return TRUE;
    }

    //==============================================================================

    void RotateTreeLeft( s32 X )
    {
        tree_data& XInst = s_Tree[X];
        s32        Y     = XInst.Right;
        ASSERT( Y != -1 );
        tree_data& YInst = s_Tree[Y];
    
        // move y's child over
        XInst.Right = YInst.Left;
        if ( YInst.Left != -1 )
        {
            tree_data& Left = s_Tree[(s32)YInst.Left];
            Left.Parent = X;
        }

        // move y up to x's position
        YInst.Parent = XInst.Parent;
        if ( XInst.Parent == -1 )
        {
            s_Root = Y;
        }
        else
        {
            tree_data& Parent = s_Tree[(s32)XInst.Parent];
            if ( X == Parent.Left )
                Parent.Left  = Y;
            else
                Parent.Right = Y;
        }

        // move x down
        YInst.Left   = X;
        XInst.Parent = Y;
    }

    //==============================================================================

    void RotateTreeRight( s32 Y )
    {
        tree_data& YInst = s_Tree[Y];
        s32        X     = YInst.Left;
        ASSERT( X != -1 );
        tree_data& XInst = s_Tree[X];

        // move x's child over
        YInst.Left = XInst.Right;
        if ( XInst.Right != -1 )
        {
            tree_data& Right = s_Tree[(s32)XInst.Right];
            Right.Parent = Y;
        }

        // move x up to y's position
        XInst.Parent = YInst.Parent;
        if ( YInst.Parent == -1 )
        {
            s_Root = X;
        }
        else
        {
            tree_data& Parent = s_Tree[(s32)YInst.Parent];
            if ( Y == Parent.Left )
                Parent.Left  = X;
            else
                Parent.Right = X;
        }

        // move y down
        XInst.Right  = Y;
        YInst.Parent = X;
    }

    //==============================================================================

    void AddToRedBlackTree( tree_data& Data, s32 Pos )
    {
        // insert it like you would a normal binary tree
        if ( !InsertNode( Data, Pos ) )
        {
            return;
        }

        // now restore the red-black property
        Data.RedBlack  = tree_data::RED_NODE;
        s32 X          = Pos;
        tree_data* pX  = &s_Tree[X];
        while ( 1 )
        {
            // bail if we're at the root
            if ( X == s_Root )
            {
                ASSERT( pX->Parent == -1 );
                break;
            }

            // get a handy pointer to the parent...we'll be needing that
            ASSERT( pX->Parent != -1 );
            s32 iParent              = pX->Parent;
            tree_data* pParent = &s_Tree[iParent];

            // if the parent's color is black, then the tree is maintained, we can quit
            if ( pParent->RedBlack == tree_data::BLACK_NODE )
            {
                break;
            }

            // get a handy pointer to the grandparent
            ASSERT( pParent->Parent != -1 );
            s32 iGrandParent        = pParent->Parent;
            tree_data* pGrandParent = &s_Tree[iGrandParent];

            // we definitely need to do a fixup to maintain the red-black properties
            if ( iParent == pGrandParent->Left )
            {
                s32 iUncle = pGrandParent->Right;
                if ( (iUncle == -1) || (s_Tree[iUncle].RedBlack == tree_data::BLACK_NODE) )
                {
                    if ( X == pParent->Right )
                    {
                        // case 2: x's uncle is black, x is the right child of its parent
                        X  = iParent;
                        pX = pParent;
                        RotateTreeLeft(X);
                        // after rotation we drop through to case 3
                        ASSERT( pX->Parent != -1 );
                        iParent = pX->Parent;
                        pParent = &s_Tree[iParent];
                        ASSERT( pParent->Parent != -1 );
                        iGrandParent = pParent->Parent;
                        pGrandParent = &s_Tree[iGrandParent];
                    }

                    // case 3: x's uncle is black, x is the left child of its parent
                    pParent->RedBlack      = tree_data::BLACK_NODE;
                    pGrandParent->RedBlack = tree_data::RED_NODE;
                    RotateTreeRight( iGrandParent );
                }
                else
                {
                    ASSERT( iUncle != -1 );
                    tree_data& Uncle = s_Tree[iUncle];
                    ASSERT( Uncle.RedBlack == tree_data::RED_NODE );

                    // case 1: x's uncle is red
                    pParent->RedBlack      = tree_data::BLACK_NODE;
                    Uncle.RedBlack         = tree_data::BLACK_NODE;
                    pGrandParent->RedBlack = tree_data::RED_NODE;
                    X  = iGrandParent;
                    pX = pGrandParent;
                }
            }
            else
            {
                s32 iUncle = pGrandParent->Left;
                if ( (iUncle == -1) || (s_Tree[iUncle].RedBlack == tree_data::BLACK_NODE) )
                {
                    if ( X == pParent->Left )
                    {
                        // case 2: x's uncle is black, x is the left child of its parent
                        X  = iParent;
                        pX = pParent;
                        RotateTreeRight(X);
                        // after rotation we drop through to case 3
                        ASSERT( pX->Parent != -1 );
                        iParent = pX->Parent;
                        pParent = &s_Tree[iParent];
                        ASSERT( pParent->Parent != -1 );
                        iGrandParent = pParent->Parent;
                        pGrandParent = &s_Tree[iGrandParent];
                    }

                    // case 3: x's uncle is black, x is the right child of its parent
                    pParent->RedBlack      = tree_data::BLACK_NODE;
                    pGrandParent->RedBlack = tree_data::RED_NODE;
                    RotateTreeLeft( iGrandParent );
                }
                else
                {
                    ASSERT( iUncle != -1 );
                    tree_data& Uncle = s_Tree[iUncle];
                    ASSERT( Uncle.RedBlack == tree_data::RED_NODE );

                    // case 1: x's uncle is red
                    pParent->RedBlack      = tree_data::BLACK_NODE;
                    Uncle.RedBlack         = tree_data::BLACK_NODE;
                    pGrandParent->RedBlack = tree_data::RED_NODE;
                    X  = iGrandParent;
                    pX = pGrandParent;
                }
            }
        }

        // set the root back to black
        s_Tree[s_Root].RedBlack = tree_data::BLACK_NODE;
    }

    //==============================================================================

    void insert( u32* pKeys, s32 NKeys )
    {
        s_Root = -1;
        s_Tree.Delete(0, s_Tree.GetCount());

        for ( s32 i = 0; i < NKeys; i++ )
        {
            tree_data& Data = s_Tree.Append();
            ClearSortData( Data.Sort );
            Data.Sort.SortKey = pKeys[i];
            Data.Left         = -1;
            Data.Right        = -1;
            Data.Parent       = -1;
            Data.Next         = -1;
            AddToRedBlackTree(Data, i);
        }
    }

    //==============================================================================

    void sort( void )
    {
    }

    //==============================================================================

    void walk( void )
    {
        smem_StackPushMarker();
        s16* Stack = (s16*)smem_StackAlloc(sizeof(s16)*s_Tree.GetCount());
        s16 StackDepth = -1;
        s32 NAccess = 0;

        s32 CurrPos = s_Root;
        while ( CurrPos != -1 )
        {
            // handle the left node first
            tree_data& Data = s_Tree[CurrPos];
            if ( Data.Left != -1 )
            {
                Stack[++StackDepth] = CurrPos;
                CurrPos = Data.Left;
                continue;
            }
        
            // handle this node
            AccessSortData( Data.Sort );
            NAccess++;

            // handle the next node
            if ( Data.Next != -1 )
            {
                tree_data& Next = s_Tree[Data.Next];
                Next.Left       = Data.Left;
                Next.Right      = Data.Right;
                Next.Parent     = Data.Parent;
                CurrPos         = Data.Next;
            }
            else
            if ( Data.Right != -1 )
            {
                // handle the right node
                CurrPos = Data.Right;
            }
            else if ( StackDepth != -1 )
            {
                // no right node? Okay, hit the parent
                CurrPos = Stack[StackDepth--];

                // also, don't let the parent hit the left child again, we're not
                // walking over this tree again, so just set that side to -1
                ASSERT( CurrPos >= 0 );
                s_Tree[CurrPos].Left = -1;
            }
            else
            {
                CurrPos = -1;
            }
        }

        smem_StackPopToMarker();

        ASSERT( NAccess == s_Tree.GetCount() );
    }
};

//==============================================================================
// Hash table hybrid
//==============================================================================

namespace hashbrid
{
    static const s32 kTableSize = 769;  // 1543;
    
    struct hash_entry
    {
        sort_data   Data;
        s32         Next;
        s32         Brother;
    };

    static s32                s_HashTable[kTableSize];
    static xarray<hash_entry> s_List;
    static s32                s_LoMark;
    static s32                s_HiMark;
    static s32                s_NKeysSafetyCheck;

    //==============================================================================

    u32 HashValue( u32 Key )
    {
        return Key % kTableSize;
    }

    //==============================================================================

    void init( void )
    {
        s_List.SetCapacity(kMaxSortItems);
        s_List.SetCount(kMaxSortItems);
    }

    //==============================================================================

    void insert( u32* pKeys, s32 NKeys )
    {
        // clear the hash table
        for ( s32 iHash = 0; iHash < kTableSize; iHash++ )
        {
            s_HashTable[iHash] = -1;
        }
        s_LoMark = 0;
        s_HiMark = s_List.GetCount() - 1;
        s_NKeysSafetyCheck = NKeys;

        // add them
        for ( s32 i = 0; i < NKeys; i++ )
        {
            s32 HashIndex = HashValue( pKeys[i] );

            // if the hash entry is empty, add it to the bottom of the list
            if ( s_HashTable[HashIndex] == -1 )
            {
                s_HashTable[HashIndex] = s_LoMark;
                hash_entry& Data  = s_List[s_LoMark++];
                ClearSortData(Data.Data);
                Data.Data.SortKey = pKeys[i];
                Data.Next         = -1;
                Data.Brother      = -1;
            }
            else
            {
                // walk the linked list to see if it's a brother of someone
                s32 CurrIndex   = s_HashTable[HashIndex];
                xbool IsBrother = FALSE;
                while ( CurrIndex != -1 )
                {
                    hash_entry& Current = s_List[CurrIndex];
                    if ( Current.Data.SortKey == pKeys[i] )
                    {
                        // it's a brother, add it to the top of the list
                        hash_entry& Data = s_List[s_HiMark];
                        ClearSortData(Data.Data);
                        Data.Data.SortKey = pKeys[i];
                        Data.Next         = Current.Next;
                        Data.Brother      = Current.Brother;
                        Current.Brother   = s_HiMark;
                        s_HiMark--;
                        IsBrother = TRUE;
                        break;
                    }

                    CurrIndex = Current.Next;
                }
                
                if ( !IsBrother )
                {
                    // add it to the linked list
                    hash_entry& Data       = s_List[s_LoMark];
                    ClearSortData(Data.Data);
                    Data.Data.SortKey      = pKeys[i];
                    Data.Next              = s_HashTable[HashIndex];
                    Data.Brother           = -1;
                    s_HashTable[HashIndex] = s_LoMark;
                    s_LoMark++;

                }
            }

            ASSERT( s_LoMark <= s_HiMark );
        }
    }

    //==============================================================================

    s32 compare( const void* pA, const void* pB )
    {
        hash_entry* pSortA = (hash_entry*)pA;
        hash_entry* pSortB = (hash_entry*)pB;
        if ( pSortA->Data.SortKey < pSortB->Data.SortKey )
            return -1;
        if ( pSortA->Data.SortKey > pSortB->Data.SortKey )
            return 1;
        return 0;
    };

    //==============================================================================

    void sort( void )
    {
        x_qsort( s_List.GetPtr(), s_LoMark, sizeof(hash_entry), compare );
    }

    //==============================================================================

    void walk( void )
    {
        s32 NAccess = 0;
        //u32 PrevKey = 0;
        for ( s32 i = 0; i < s_LoMark; i++ )
        {
            s32 CurrNode = i;
            while ( CurrNode != -1 )
            {
                hash_entry& Data = s_List[CurrNode];
                AccessSortData(Data.Data);
                NAccess++;
                CurrNode = Data.Brother;
                //ASSERT( Data.Data.SortKey >= PrevKey );
                //PrevKey  = Data.Data.SortKey;
            }
        }

        ASSERT( NAccess == s_NKeysSafetyCheck );
    }
};

//==============================================================================
// Hash table using pointers
//==============================================================================

namespace hashptr
{
    static const s32 kTableSize = 769;  // 1543;
    
    struct hash_entry
    {
        sort_data   Data;
        s32         Next;
        s32         Brother;
    };

    struct sort_entry
    {
        u32         SortKey;
        s32         iData;
    };

    static s32                s_HashTable[kTableSize];
    static xarray<hash_entry> s_List;
    static xarray<sort_entry> s_Sort;
    static s32                s_LoMark;
    static s32                s_HiMark;
    static s32                s_NKeysSafetyCheck;

    //==============================================================================

    u32 HashValue( u32 Key )
    {
        return Key % kTableSize;
    }

    //==============================================================================

    void init( void )
    {
        s_List.SetCapacity(kMaxSortItems);
        s_List.SetCount(kMaxSortItems);
        s_Sort.SetCapacity(kMaxSortItems);
        s_Sort.SetCount(kMaxSortItems);
    }

    //==============================================================================

    void insert( u32* pKeys, s32 NKeys )
    {
        // clear the hash table
        for ( s32 iHash = 0; iHash < kTableSize; iHash++ )
        {
            s_HashTable[iHash] = -1;
        }
        s_LoMark = 0;
        s_HiMark = s_List.GetCount() - 1;
        s_NKeysSafetyCheck = NKeys;

        // add them
        for ( s32 i = 0; i < NKeys; i++ )
        {
            s32 HashIndex = HashValue( pKeys[i] );

            // if the hash entry is empty, add it to the bottom of the list
            if ( s_HashTable[HashIndex] == -1 )
            {
                s_HashTable[HashIndex] = s_LoMark;
                hash_entry& Data  = s_List[s_LoMark];
                sort_entry& Sort  = s_Sort[s_LoMark];
                ClearSortData(Data.Data);
                Data.Data.SortKey = pKeys[i];
                Data.Next         = -1;
                Data.Brother      = -1;
                Sort.iData        = s_LoMark;
                Sort.SortKey      = pKeys[i];
                s_LoMark++;
            }
            else
            {
                // walk the linked list to see if it's a brother of someone
                s32 CurrIndex   = s_HashTable[HashIndex];
                xbool IsBrother = FALSE;
                while ( CurrIndex != -1 )
                {
                    hash_entry& Current = s_List[CurrIndex];
                    if ( Current.Data.SortKey == pKeys[i] )
                    {
                        // it's a brother, add it to the top of the list
                        hash_entry& Data = s_List[s_HiMark];
                        ClearSortData(Data.Data);
                        Data.Data.SortKey = pKeys[i];
                        Data.Next         = Current.Next;
                        Data.Brother      = Current.Brother;
                        Current.Brother   = s_HiMark;
                        s_HiMark--;
                        IsBrother = TRUE;
                        break;
                    }

                    CurrIndex = Current.Next;
                }
                
                if ( !IsBrother )
                {
                    // add it to the linked list
                    hash_entry& Data       = s_List[s_LoMark];
                    sort_entry& Sort       = s_Sort[s_LoMark];
                    ClearSortData(Data.Data);
                    Data.Data.SortKey      = pKeys[i];
                    Data.Next              = s_HashTable[HashIndex];
                    Data.Brother           = -1;
                    s_HashTable[HashIndex] = s_LoMark;
                    Sort.iData             = s_LoMark;
                    Sort.SortKey           = pKeys[i];
                    s_LoMark++;

                }
            }

            ASSERT( s_LoMark <= s_HiMark );
        }
    }

    //==============================================================================

    s32 compare( const void* pA, const void* pB )
    {
        sort_entry* pSortA = (sort_entry*)pA;
        sort_entry* pSortB = (sort_entry*)pB;
        if ( pSortA->SortKey < pSortB->SortKey )
            return -1;
        if ( pSortA->SortKey > pSortB->SortKey )
            return 1;
        return 0;
    };

    //==============================================================================

    void sort( void )
    {
        x_qsort( s_Sort.GetPtr(), s_LoMark, sizeof(sort_entry), compare );
        //x_qsort( s_List.GetPtr(), s_LoMark, sizeof(hash_entry), compare );
    }

    //==============================================================================

    void walk( void )
    {
        s32 NAccess = 0;
        //u32 PrevKey = 0;
        for ( s32 i = 0; i < s_LoMark; i++ )
        {
            s32 CurrNode = s_Sort[i].iData;
            ASSERT( (CurrNode>=0) && (CurrNode < s_LoMark) );
            //s32 CurrNode = i;
            while ( CurrNode != -1 )
            {
                hash_entry& Data = s_List[CurrNode];
                AccessSortData(Data.Data);
                NAccess++;
                CurrNode = Data.Brother;
                //ASSERT( Data.Data.SortKey >= PrevKey );
                //PrevKey  = Data.Data.SortKey;
            }
        }

        ASSERT( NAccess == s_NKeysSafetyCheck );
    }
};

//==============================================================================
// Testing functions
//==============================================================================

static const s32 kNumTests = 9;

struct sort_test
{
    const char*     Name;
    init_data_fn*   InitFn;
    insert_data_fn* InsertFn;
    sort_data_fn*   SortFn;
    walk_data_fn*   WalkFn;
    xtick           InsertTime;
    xtick           SortTime;
    xtick           WalkTime;
};

static s32              s_NKeys = 0;
static u32*             s_pKeys = NULL;

static sort_test    s_SortTests[kNumTests] =
{
    { "qsort",      qsort::init,    qsort::insert,      qsort::sort,    qsort::walk,    0,  0,  0 },
    { "qsortptr",   qsortptr::init, qsortptr::insert,   qsortptr::sort, qsortptr::walk, 0,  0,  0 },
    { "bucktptr",   bucktptr::init, bucktptr::insert,   bucktptr::sort, bucktptr::walk, 0,  0,  0 },
    { "bintree",    bintree::init,  bintree::insert,    bintree::sort,  bintree::walk,  0,  0,  0 },
    { "bintree2",   bintree2::init, bintree2::insert,   bintree2::sort, bintree2::walk, 0,  0,  0 },
    { "rbtree",     rbtree::init,   rbtree::insert,     rbtree::sort,   rbtree::walk,   0,  0,  0 },
    { "rbtree2",    rbtree2::init,  rbtree2::insert,    rbtree2::sort,  rbtree2::walk,  0,  0,  0 },
    { "hashbrid",   hashbrid::init, hashbrid::insert,   hashbrid::sort, hashbrid::walk, 0,  0,  0 },
    { "hashptr",    hashptr::init,  hashptr::insert,    hashptr::sort,  hashptr::walk,  0,  0,  0 },
};


void TestSortRoutines( void )
{

    for ( s32 i = 0; i < kNumTests; i++ )
    {
        // insert the data
        {
            xtimer InsertTime;
            InsertTime.Start();
            s_SortTests[i].InsertFn( s_pKeys, s_NKeys );
            s_SortTests[i].InsertTime = InsertTime.Stop();
        }

        // sort the data
        {
            xtimer SortTime;
            SortTime.Start();
            s_SortTests[i].SortFn();
            s_SortTests[i].SortTime = SortTime.Stop();
        }

        // walk the data
        {
            xtimer WalkTime;
            WalkTime.Start();
            s_SortTests[i].WalkFn();
            s_SortTests[i].WalkTime = WalkTime.Stop();
        }

        // invalidate the cache to make the next function fair (just access 16k elsewhere)
        byte* pDataArea = (byte*)x_malloc(32*1024);
        for ( s32 y = 0; y < 32*1024; y++ )
        {
            byte temp              = pDataArea[y];
            pDataArea[y]           = pDataArea[32*1024-y-1];
            pDataArea[32*1024-y-1] = temp;
        }
        x_free(pDataArea);
    }
}

//==============================================================================

void PrintSortResults( void )
{
    x_printfxy( 0, 0, "%8s: %6s %6s %6s %6s",
                "routine", "Insert", "Sort", "Walk", "Total" );
    x_printfxy( 0, 1, "=====================================" );
    s32 curry = 2;
    for ( s32 i = 0; i < kNumTests; i++ )
    {
        x_printfxy( 0, curry++,
                    "%8s: %6.2f %6.2f %6.2f %6.2f",
                    s_SortTests[i].Name,
                    x_TicksToMs(s_SortTests[i].InsertTime),
                    x_TicksToMs(s_SortTests[i].SortTime),
                    x_TicksToMs(s_SortTests[i].WalkTime),
                    x_TicksToMs(s_SortTests[i].InsertTime+s_SortTests[i].SortTime+s_SortTests[i].WalkTime) );
    }
}

//==============================================================================

void InitSortRoutines( void )
{
    s32 i;

/*
    s_NKeys = 1800;
    s_pKeys = new u32[s_NKeys];
    for ( i = 0; i < s_NKeys; i++ )
    {
        s_pKeys[i] = ((u32)x_rand()) & 0x3fffffff;
    }
    */

    X_FILE* fh = x_fopen("C:\\SortKeys.bin", "rb");
    x_fread( &s_NKeys, sizeof(s32), 1, fh );
    s_pKeys = new u32[s_NKeys];
    x_fread( s_pKeys, sizeof(u32), s_NKeys, fh );
    
    for ( i = 0; i < kNumTests; i++ )
    {
        s_SortTests[i].InitFn();
    }
}