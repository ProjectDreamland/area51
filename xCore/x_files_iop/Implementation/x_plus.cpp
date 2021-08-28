//==============================================================================
//
//  x_plus.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_PLUS_HPP
#include "..\x_plus.hpp"
#endif

#ifndef X_DEBUG_HPP
#include "..\x_debug.hpp"
#endif

#ifndef X_FILES_PRIVATE_HPP
#include "x_files_private.hpp"
#endif

//==============================================================================
//  FUNCTIONS
//==============================================================================

//==============================================================================
//  Quick Sort
//==============================================================================
//  
//  This implementation of the quick sort algorithm is, on average, 25% faster 
//  than most "system" versions.  The improvement ranges from a low of 10% to a 
//  high of 50%.
//  
//  Two functions are used to accomplish the sort: x_qsort and PseudoQuickSort.  
//  
//  The PseudoQuickSort function (abbreviated PQS) will only "mostly" sort a 
//  given array of items.  It is recursive, but it is guaranteed that the 
//  function will not recurse more than log base 2 of n times.
//  
//  The x_qsort function will first perform some set up.  It then optionally
//  invokes the PQS function.  Finally, it will perform an optimized insertion 
//  sort to finish off the sort process.
//  
//==============================================================================

// Two thresholds are used to optimize the algorithm.  These values are 
// currently tuned for items having an average size of 48 bytes.

#define RECURSION_THRESH  4     //  Items needed to invoke PQS.
#define PARTITION_THRESH  6     //  Items needed to seek partition key.

// Anounce the PQS function.

static void PseudoQuickSort( byte*        pBase, 
                             byte*        pMax, 
                             s32          ItemSize, 
                             s32          RecursionThresh, 
                             s32          PartitionThresh, 
                             compare_fn*  pCompare );

//------------------------------------------------------------------------------

void x_qsort( const void*  apBase,     
              s32          NItems,    
              s32          ItemSize,  
              compare_fn*  pCompare )  
{
    byte  c;
    byte* i;
    byte* j;
    byte* lo;
    byte* hi;
    byte* min;
    byte* pMax;
    byte* pBase = (byte*)apBase;
    s32   RecursionThresh;          // Recursion threshold in bytes
    s32   PartitionThresh;          // Partition threshold in bytes

    ASSERT( pBase    );
    ASSERT( pCompare );
    ASSERT( NItems   > 0 );
    ASSERT( ItemSize > 0 );

    // Easy out?
    if( NItems <= 1 )
        return;

    // Set up some useful values.
    RecursionThresh = ItemSize * RECURSION_THRESH;
    PartitionThresh = ItemSize * PARTITION_THRESH;
    pMax            = pBase + (NItems * ItemSize);

    //
    // Set the 'hi' value.
    // Also, if there are enough values, call the PseudoQuickSort function.
    //
    if( NItems >= RECURSION_THRESH )
    {
        PseudoQuickSort( pBase, 
                         pMax, 
                         ItemSize, 
                         RecursionThresh, 
                         PartitionThresh, 
                         pCompare );
        hi = pBase + RecursionThresh;
    }
    else
    {
        hi = pMax;
    }

    //
    // Find the smallest element in the first "MIN(RECURSION_THRESH,NItems)"
    // items.  At this point, the smallest element in the entire list is 
    // guaranteed to be present in this sublist.
    //
    for( j = lo = pBase; (lo += ItemSize) < hi;  )
    {
        if( pCompare( j, lo ) > 0 )
            j = lo;
    }

    // 
    // Now put the smallest item in the first position to prime the next 
    // loop.
    //
    if( j != pBase )
    {
        for( i = pBase, hi = pBase + ItemSize; i < hi;  )
        {
            c    = *j;
            *j++ = *i;
            *i++ = c;
        }
    }

    //
    // Smallest item is in place.  Now we run the following hyper-fast
    // insertion sort.  For each remaining element, min, from [1] to [n-1],
    // set hi to the index of the element AFTER which this one goes.  Then,
    // do the standard insertion sort shift on a byte at a time basis.
    //
    for( min = pBase; (hi = min += ItemSize) < pMax;  )
    {
        while( pCompare( hi -= ItemSize, min ) > 0 )
        {
            // No body in this loop.
        }        

        if( (hi += ItemSize) != min )
        {
            for( lo = min + ItemSize; --lo >= min;  )
            {
                c = *lo;
                for( i = j = lo; (j -= ItemSize) >= hi; i = j )
                {
                    *i = *j;
                }
                *i = c;
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//  What the hell is a PseudoQuickSort?  Simple!  Its "sorta quick sort".
//
//  Find a partition element and put it in the first position of the list.  The 
//  partition value is the median value of the first, middle, and last items.  
//  (Using the median value of these three items rather than just using the 
//  first item is a big win.)
//
//  Then, the usual partitioning and swapping, followed by swapping the 
//  partition element into place.
//
//  Then, of the two portions of the list which remain on either side of the 
//  partition, sort the smaller portion recursively, and sort the larger 
//  portion via another iteration of the same code.
//
//  Do not bother "quick sorting" lists (or sub lists) which have fewer than
//  RECURSION_THRESH items.  All final sorting is handled with an insertion sort
//  which is executed by the caller (x_qsort).  This is another huge win.  This 
//  means that this function does not actually completely sort the input list.  
//  It mostly sorts it.  That is, each item will be within RECURSION_THRESH 
//  positions of its correct position.  Thus, an insert sort will be able to 
//  finish off the sort process without a serious performance hit.
//
//  All data swaps are done in-line, which trades some code space for better
//  performance.  There are only three swap points, anyway.
//
//------------------------------------------------------------------------------

static 
void PseudoQuickSort( byte*        pBase,          
                      byte*        pMax,           
                      s32          ItemSize,        
                      s32          RecursionThresh,
                      s32          PartitionThresh,
                      compare_fn*  pCompare )
{
    byte* i;
    byte* j;
    byte* jj;
    byte* mid;
    byte* tmp;
    byte  c;
    s32   ii;
    s32   lo;
    s32   hi;

    lo = (s32)(pMax - pBase);   // Total data to sort in bytes.

    // Deep breath now...
    do
    {
        //
        // At this point, lo is the number of bytes in the items in the current
        // partition.  We want to find the median value item of the first, 
        // middle, and last items.  This median item will become the middle 
        // item.  Set j to the "greater of first and middle".  If last is larger
        // than j, then j is the median.  Otherwise, compare the last item to 
        // "the lesser of the first and middle" and take the larger.  The code 
        // is biased to prefer the middle over the first in the event of a tie.
        //

        mid = i = pBase + ItemSize * ((u32)(lo / ItemSize) >> 1);

        if( lo >= PartitionThresh )
        {
            j = (pCompare( (jj = pBase), i ) > 0)  ?  jj : i;

            if( pCompare( j, (tmp = pMax - ItemSize) ) > 0 )
            {
                // Use lesser of first and middle.  (First loser.)
                j = (j == jj ? i : jj);
                if( pCompare( j, tmp ) < 0 )
                    j = tmp;
            }

            if( j != i )
            {
                // Swap!
                ii = ItemSize;
                do
                {
                    c    = *i;
                    *i++ = *j;
                    *j++ = c;
                } while( --ii );
            }
        }
                
        //
        // Semi-standard quicksort partitioning / swapping...
        //

        for( i = pBase, j = pMax - ItemSize;  ;  )
        {
            while( (i < mid) && (pCompare( i, mid ) <= 0) )
            {
                i += ItemSize;
            }

            while( j > mid )
            {
                if( pCompare( mid, j ) <= 0 )
                {
                    j -= ItemSize;
                    continue;
                }

                tmp = i + ItemSize;     // Value of i after swap.

                if( i == mid )
                {
                    // j <-> mid, new mid is j.
                    mid = jj = j;       
                }
                else
                {
                    // i <-> j
                    jj = j;
                    j -= ItemSize;
                }

                goto SWAP;
            }

            if( i == mid )
            {
                break;
            }
            else
            {
                jj  = mid;
                tmp = mid = i;
                j  -= ItemSize;
            }
SWAP:
            ii = ItemSize;
            do
            {
                c     = *i;
                *i++  = *jj;
                *jj++ = c;
            } while( --ii );

            i = tmp;
        }

        //
        // Consider the sizes of the two partitions.  Process the smaller
        // partition first via recursion.  Then process the larger partition by
        // iterating through the above code again.  (Update variables as needed
        // to make it work.)
        //
        // NOTE:  Do not bother sorting a given partition, either recursively or
        // by another iteration, if the size of the partition is less than 
        // RECURSION_THRESH items.
        //

        j = mid;
        i = mid + ItemSize;

        if( (lo = (j-pBase)) <= (hi = (pMax-i)) )
        {
            if( lo >= RecursionThresh )
                PseudoQuickSort( pBase, j, 
                                 ItemSize,        
                                 RecursionThresh,
                                 PartitionThresh,
                                 pCompare );
            pBase = i;
            lo    = hi;
        }
        else
        {
            if( hi >= RecursionThresh )
                PseudoQuickSort( i, pMax,
                                 ItemSize,        
                                 RecursionThresh,
                                 PartitionThresh,
                                 pCompare );
            pMax = j;
        }

    } while( lo >= RecursionThresh );
}

//==============================================================================

void* x_bsearch( const void*  pKey,
                 const void*  pBase,       
                 s32          NItems,      
                 s32          ItemSize,    
                 compare_fn*  pCompare )
{
    void* pLocation;
    return( x_bsearch( pKey, pBase, NItems, ItemSize, pCompare, pLocation ) );
}

//==============================================================================

void* x_bsearch( const void*  pKey,
                 const void*  pBase,       
                 s32          NItems,      
                 s32          ItemSize,    
                 compare_fn*  pCompare,
                 void*&       pLocation )
{
    byte* pLo;
    byte* pHi;
    byte* pMid;
    s32   Half1;
    s32   Half2;
    s32   Result;
    
    ASSERT( pKey     );
    ASSERT( pBase    );
    ASSERT( pCompare );
    ASSERT( NItems   > 0 );
    ASSERT( ItemSize > 0 );

    pLocation = NULL;

    pLo = (byte*)pBase;
    pHi = pLo + ((NItems-1) * ItemSize);

    while( pLo <= pHi )
    {
        //
        // If there are at least two items in the list, then we want to do the
        // binary search stuff.
        //
        // But if there is only one item, then just do a single compare and be
        // done with it.  The "normal" code path (for 2 or more items) would 
        // work, but it is far less efficient.
        //
        // A "no items in the list" case is impossible based on the nature of
        // the surrounding while loop.
        //

        if( NItems >= 2 )
        {
            Half1  = NItems / 2;
            Half2  = NItems - Half1 - 1;
            pMid   = pLo + (Half1 * ItemSize);

            Result = pCompare( pKey, pMid );

            if( Result < 0 )
            {
                // The mid item is too "high".  
                // Keep searching.  New range is [pLo, pMid-1].
                pHi    = pMid - ItemSize;
                NItems = Half1;
            }

            else if( Result > 0 )
            {
                // The mid item is too "low".  
                // Keep searching.  New range is [pMid+1, pHi].
                pLo    = pMid + ItemSize;
                NItems = Half2;
            }

            else // if( Result == 0 )
            {
                //
                // In a version which is not prepared for duplicates, this case
                // would simply "return( pMid )".
                //
                // Instead, we will continue to search since there may be items
                // just prior to pMid which are the same.  We want to return the
                // first match in the list, not the first match we find.
                //
                // Check the pMid-1 item.  If it doesn't match, then we are 
                // assuredly done.  If it does match, then we will act as if we 
                // had a "<0" return from before.
                //
                // Note that it is impossible for pLo and pMid to be the same at
                // this point.  (Or, more to the point, it is impossible for 
                // pMid-1 to go out of the valid range.)
                //

                Result = pCompare( pKey, pMid-ItemSize );

                ASSERT( Result >= 0 );

                if( Result > 0 )
                {
                    // No match in prior item.  So, pMid is it!
                    return( pMid );
                }
                else
                {
                    // Both pMid and pMid-1 match the desired item!
                    // Keep searching.  New range is [pLo, pMid-1].
                    pHi    = pMid - ItemSize;
                    NItems = Half1;
                }
            }
        }
        else
        {
            // If we are here, then NItems is 1.
            ASSERT( NItems == 1 );

            Result = pCompare( pKey, pLo );
            if( Result == 0 )
            {
                // We got it!
                return( pLo );
            }

            //
            // If we are here, then the desired item is not in the list.  The
            // only thing remaining is to decide where it would go if it were
            // to be added.  The results from the most recent compare tell us
            // all we need to know.
            //

            if( Result > 0 )
            {
                pLocation = pLo + ItemSize;
            }
            else // if( Result < 0 )
            {
                pLocation = pLo;
            }

            // That's it.  The pLocation has been set.  Return NULL to indicate
            // the search failed.
            return( NULL );
        }
    }

    // Shouldn't be possible to get here.
    ASSERT( FALSE );
    return( NULL );
}

//==============================================================================
//  Pseudo random number generation functions.
//==============================================================================

//==============================================================================
//  
//  We have a single global instance of class random to be used for the standard
//  function calls (x_rand and such).  This instance is not thread aware, but it
//  should be thread safe.  Even a worst case thread interaction should not
//  cause it to fail.
//  
//==============================================================================

static random s_Random( 2 );

//==============================================================================
//  Global "standard" pseudo random number generation functions.
//==============================================================================

void x_srand( s32 Seed )
{
    s_Random.srand( Seed );
}

//==============================================================================

s32 x_rand( void )
{
    return( s_Random.rand() );
}

//==============================================================================

s32 x_irand( s32 Min, s32 Max )
{
    return( s_Random.irand( Min, Max ) );
}

//==============================================================================

f32 x_frand( f32 Min, f32 Max )
{
    return( s_Random.frand( Min, Max ) );
}

//==============================================================================
//  Functions for class random.
//==============================================================================

random::random( void )
{
    m_Seed = 2;
}

//==============================================================================

random::random( s32 Seed )
{
    m_Seed = Seed;
}                

//==============================================================================

void random::srand( s32 Seed )
{
    m_Seed = Seed;
}

//==============================================================================

s32 random::rand( void )
{
    m_Seed = m_Seed * 214013 + 2531011;
    return( (s32)((m_Seed >> 16) & X_RAND_MAX) );
}

//==============================================================================

s32 random::irand( s32 Min, s32 Max )
{
    ASSERT( Max >= Min );
    return( (rand() % (Max-Min+1)) + Min );
}

//==============================================================================

f32 random::frand( f32 Min, f32 Max )
{
    ASSERT( Max >= Min );
    return( (((f32)rand() / (f32)X_RAND_MAX) * (Max-Min)) + Min );
}

//==============================================================================

//==============================================================================
//  Standard NULL terminated string manipulation functions.
//==============================================================================

s32 x_strlen( const char* pStr )
{
    const char *pEnd = pStr;

    ASSERT( pStr );

    while( *pEnd++ )
        ; // empty body

    return( (s32)(pEnd - pStr - 1) );
}

//==============================================================================

char* x_strcpy( char* pDest, const char* pSrc )
{
    char* p = pDest;

    ASSERT( pDest );
    ASSERT( pSrc  );

    while( (*p++ = *pSrc++) )
        ; // empty body

    return( pDest );
}

//==============================================================================

char* x_strdup( const char* pStr )
{
	ASSERT( pStr );

	char* pStrResult = (char *)x_malloc( sizeof(char) * x_strlen(pStr) );
	char* p = pStrResult;
	ASSERT( pStrResult );

	while( (*p++ = *pStr++) )
		;

	return( pStrResult );
}

//==============================================================================

char* x_strncpy( char* pDest, const char* pSrc, s32 Count )
{
    char* pStart = pDest;

    ASSERT( pDest );
    ASSERT( pSrc  );
    ASSERT( Count >= 0 );

    while( Count && (*pDest++ = *pSrc++) )
        Count--;

    if( Count )
        while( --Count )
            *pDest++ = '\0';

    return( pStart );
}

//==============================================================================

char* x_strcat( char* pFront, const char* pBack )
{
    char* p = pFront;

    ASSERT( pFront );
    ASSERT( pBack  );

    while( *p ) 
        p++;

    while( (*p++ = *pBack++) )
        ; // empty body

    return( pFront );
}

//==============================================================================

char* x_strncat( char* pFront, const char* pBack, s32 Count )
{
    char *pStart = pFront;

    ASSERT( pFront );
    ASSERT( pBack  );
    ASSERT( Count >= 0 );

    while( *pFront++ )
        ; // empty body

    pFront--;

    while( Count-- )
        if( !(*pFront++ = *pBack++) )
            return( pStart );

    *pFront = '\0';

    return( pStart );
}

//==============================================================================

s32 x_strcmp( const char* pStr1, const char* pStr2 )
{
    s32 Result = 0;

    ASSERT( pStr1 );
    ASSERT( pStr2 );

    while( !(Result = ((s32)*pStr1) - ((s32)*pStr2)) && *pStr1 )
        ++pStr1, ++pStr2;

    return( Result );
}                    

//==============================================================================

s32 x_strncmp( const char* pStr1, const char* pStr2, s32 Count )
{
    ASSERT( pStr1 );
    ASSERT( pStr2 );
    ASSERT( Count >= 0 );

    if( !Count )  
        return( 0 );

    while( --Count && *pStr1 && (*pStr1 == *pStr2) )
    {
        pStr1++;
        pStr2++;
    }

    return( ((s32)*pStr1) - ((s32)*pStr2) );
}

//==============================================================================
// Assumption: Letters A-Z and a-z are contiguous in the character set.
// This is true for ASCII and UniCode, but not for EBCDIC!

s32 x_stricmp( const char* pStr1,  const char* pStr2 )
{
    s32 C1, C2;

    ASSERT( pStr1 );
    ASSERT( pStr2 );

    do
    {
        C2 = (s32)(*(pStr1++));
        if( (C2 >= 'A') && (C2 <= 'Z') )
            C2 -= ('A' - 'a');

        C1 = (s32)(*(pStr2++));
        if( (C1 >= 'A') && (C1 <= 'Z') )
            C1 -= ('A' - 'a');

    } while( C1 && (C1 == C2) );

    return( C2 - C1 );
}

//==============================================================================

char* x_strstr( const char* pMainStr, const char* pSubStr )
{
    char* pM = (char*)pMainStr;
    char* pS1; 
    char* pS2;

    if( !*pSubStr )
        return( (char*)pMainStr );

    while( *pM )
    {
        pS1 = pM;
        pS2 = (char*)pSubStr;

        while( *pS1 && *pS2 && !(*pS1 - *pS2) )
        {
            pS1++;
            pS2++;
        }

        if( !*pS2 )
            return( pM );

        pM++;
    }

    return( NULL );
}

//==============================================================================

char* x_strchr( const char* pStr, s32 C )
{
    while( *pStr && (*pStr != (char)C) )
        pStr++;

    if( *pStr == (char)C )
        return( (char*)pStr );
    return( NULL );
}

//==============================================================================

char* x_strrchr( const char* pStr, s32 C )
{
    char* pStart = (char*)pStr;

    // Find end of string.
    while( *pStr++ )
        ; // empty body

    // Search from back towards front.
    while( (--pStr != pStart) && (*pStr != (char)C) )
        ; // empty body

    // Did we find it?
    if( *pStr == (char)C )
        return( (char*)pStr );
    else
        return( NULL );
}

//==============================================================================
// Assumption: Letters A-Z and a-z are contiguous in the character set.
// This is true for ASCII and UniCode, but not for EBCDIC!

char* x_strtoupper( char* pStr )
{
    char* p = pStr;

    ASSERT( pStr );

    while( *p != '\0' )
    {
        if( (*p >= 'a') && (*p <= 'z') )
            *p += ('A' - 'a');
        p++;
    }

    return( pStr );
}

//==============================================================================
// Assumption: Letters A-Z and a-z are contiguous in the character set.
// This is true for ASCII and UniCode, but not for EBCDIC!

char* x_strtolower( char* pStr )
{
    char* p = pStr;

    ASSERT( pStr );

    while( *p != '\0' )
    {
        if( (*p >= 'A') && (*p <= 'Z') )
            *p += ('a' - 'A');
        p++;
    }

    return( pStr );
}

//==============================================================================
//  Standard NULL terminated WIDE string manipulation functions.
//==============================================================================

s32 x_wstrlen( const xwchar* pStr )
{
    const xwchar* pEnd = pStr;

    ASSERT( pStr );

    while( *pEnd++ )
        ; // empty body

    return( (s32)(pEnd - pStr - 1) );
}

//==============================================================================

xwchar* x_wstrcpy( xwchar* pDest, const xwchar* pSrc )
{
    xwchar* p = pDest;

    ASSERT( pDest );
    ASSERT( pSrc  );

    while( (*p++ = *pSrc++) )
        ; // empty body

    return( pDest );
}

//==============================================================================

xwchar* x_strdup( const xwchar* pWideStr )
{
	ASSERT( pWideStr );

	xwchar* pWideStrResult = (xwchar *)x_malloc( sizeof(xwchar) * x_wstrlen(pWideStr) );
	ASSERT( pWideStrResult );

	while( (*pWideStrResult++ = *pWideStr++) )
		;

	return( pWideStrResult );
}

//==============================================================================

xwchar* x_wstrncpy( xwchar* pDest, const xwchar* pSrc, s32 Count )
{
    xwchar* pStart = pDest;

    ASSERT( pDest );
    ASSERT( pSrc  );
    ASSERT( Count >= 0 );

    while( Count && (*pDest++ = *pSrc++) )
        Count--;

    if( Count )
        while( --Count )
            *pDest++ = NULL;

    return( pStart );
}

//==============================================================================

xwchar* x_wstrcat( xwchar* pFront, const xwchar* pBack )
{
    xwchar* p = pFront;

    ASSERT( pFront );
    ASSERT( pBack  );

    while( *p ) 
        p++;

    while( (*p++ = *pBack++) )
        ; // empty body

    return( pFront );
}

//==============================================================================

xwchar* x_wstrncat( xwchar* pFront, const xwchar* pBack, s32 Count )
{
    xwchar* pStart = pFront;

    ASSERT( pFront );
    ASSERT( pBack  );
    ASSERT( Count >= 0 );

    while( *pFront++ )
        ; // empty body

    pFront--;

    while( Count-- )
        if( !(*pFront++ = *pBack++) )
            return( pStart );

    *pFront = NULL;

    return( pStart );
}

//==============================================================================

s32 x_wstrcmp( const xwchar* pStr1, const xwchar* pStr2 )
{
    s32 Result = 0;

    ASSERT( pStr1 );
    ASSERT( pStr2 );

    while( !(Result = ((s32)*pStr1) - ((s32)*pStr2)) && *pStr1 )
        ++pStr1, ++pStr2;

    return( Result );
}                    

//==============================================================================

s32 x_wstrncmp( const xwchar* pStr1, const xwchar* pStr2, s32 Count )
{
    ASSERT( pStr1 );
    ASSERT( pStr2 );
    ASSERT( Count >= 0 );

    if( !Count )  
        return( 0 );

    while( --Count && *pStr1 && (*pStr1 == *pStr2) )
    {
        pStr1++;
        pStr2++;
    }

    return( ((s32)*pStr1) - ((s32)*pStr2) );
}

//==============================================================================

xwchar* x_wstrstr( const xwchar* pMainStr, const xwchar* pSubStr )
{
    xwchar* pM = (xwchar*)pMainStr;
    xwchar* pS1; 
    xwchar* pS2;

    if( !*pSubStr )
        return( (xwchar*)pMainStr );

    while( *pM )
    {
        pS1 = pM;
        pS2 = (xwchar*)pSubStr;

        while( *pS1 && *pS2 && !(*pS1 - *pS2) )
        {
            pS1++;
            pS2++;
        }

        if( !*pS2 )
            return( pM );

        pM++;
    }

    return( NULL );
}

//==============================================================================

xwchar* x_wstrchr( const xwchar* pStr, s32 C )
{
    while( *pStr && (*pStr != (xwchar)C) )
        pStr++;

    if( *pStr == (xwchar)C )
        return( (xwchar*)pStr );
    return( NULL );
}

//==============================================================================

xwchar* x_wstrrchr( const xwchar* pStr, s32 C )
{
    xwchar* pStart = (xwchar*)pStr;

    // Find end of string.
    while( *pStr++ )
        ; // empty body

    // Search from back towards front.
    while( (--pStr != pStart) && (*pStr != (xwchar)C) )
        ; // empty body

    // Did we find it?
    if( *pStr == (xwchar)C )
        return( (xwchar*)pStr );
    else
        return( NULL );
}

//==============================================================================
//  Standard block memory functions.
//==============================================================================

void* x_memcpy( void* pDest, const void* pSrc, s32 Count )
{
    ASSERT( pDest );
    ASSERT( pSrc );
    ASSERT( Count >= 0 );

    ASSERTS( ((pDest < pSrc ) && ( (((byte*)pDest) + Count ) <= pSrc  )) ||
             ((pSrc  < pDest) && ( (((byte*)pSrc ) + Count ) <= pDest )), 
             "x_memcpy() does not support overlapping memory regions." );

    return( x_memmove( pDest, pSrc, Count ) );
}

//==============================================================================
// This version of memmove will attempt to move as much memory as possible
// as 32-bit data.

void* x_memmove( void* pDest, const void* pSrc, s32 Count )
{
	byte* pFrom;
	byte* pTo;
	s32   t;

    ASSERT( pDest );
    ASSERT( pSrc  );
    ASSERT( Count >= 0 );

    // See if we have anything to do.
	if( (Count == 0) || (pSrc == pDest) )
        return( pDest );

    pFrom = (byte*)pSrc;
    pTo   = (byte*)pDest;

	if( pTo < pFrom )
    {
        //
        // Copy forward.
        //

		t = (s32)pFrom;

		if( (t | (s32)pTo) & 3 )
        {
            // Try to align operands.  This cannot be done unless the low 
            // bits match.

			if( ((t ^ (s32)pTo) & 3) || (Count < 4) )
            {
                t = Count;
            }
			else
            {
                t = 4 - ( t & 3 );
            }

			Count -= t;

            do
            {
                *pTo++ = *pFrom++;
            } while( --t );
		}

        // Try to copy 32 bits at a time.

		t = Count >> 2;

        if( t )
        {
            do
            {
                *(u32*)pTo = *(u32*)pFrom;
                pFrom += 4;
                pTo   += 4;
            } while( --t );
        }

        // Copy any left over bytes.

		t = Count & 3;

        if( t )
        {
            do
            {
                *pTo++ = *pFrom++;
            } while( --t );
        }
	}
    else
    {
		//
		// Copy backwards.
		//

		pFrom += Count;
		pTo   += Count;

		t = (s32)pFrom;

		if( (t | (s32)pTo) & 3 )
        {
			if( ((t ^ (s32)pTo) & 3) || (Count <= 4) )
            {
				t = Count;
            }
			else
            {
				t &= 3;
            }

			Count -= t;

            do
            {
                *--pTo = *--pFrom;
            } while( --t );
		}

        // Try to copy 32 bits at a time.

		t = Count >> 2;

        if( t )
        {
            do
            {
                pFrom -= 4;
                pTo   -= 4;
                *(u32*)pTo = *(u32*)pFrom;

            } while( --t );
        }

        // Copy any left over bytes.

		t = Count & 3;

        if( t )
        {
            do
            {
                *--pTo = *--pFrom;
            } while( --t );
        }
	}

	return( pDest );
}

//==============================================================================

void* x_memset( void* pBuf, s32 C, s32 Count )
{
    byte* p;
    byte* pEnd;
    u32*  p4;
    u32   C4;

    ASSERT( pBuf );
    ASSERT( Count >= 0 );

    p    = (byte*)pBuf;
    pEnd = p + Count;
    C4   = ( ((u32)C) << 24 ) |
           ( ((u32)C) << 16 ) |
           ( ((u32)C) <<  8 ) |
           ( ((u32)C) <<  0 );

    // Write starting bytes.
    while( (p < pEnd) && (((u32)p) & 3) )
        *p++ = (byte)C;

    // Write 4 bytes at a time.
    p4 = (u32*)p;
    while( (p4+1) < (u32*)pEnd )
    {
        *p4++ = C4;
    }
    p = (byte*)p4;

    // Write ending bytes.
    while( p < pEnd )
        *p++ = (byte)C;

    return( pBuf );
}

//==============================================================================

void* x_memchr( void* pBuf, s32 C, s32 Count )
{
    byte* p;
    byte  c;

    ASSERT( pBuf );
    ASSERT( Count >= 0 );

    p = (byte*)pBuf;
    c = (byte)C;

    while( Count && (*p != c) )
    {
        p++;
        Count--;
    }

    return( Count ? (void*)p : NULL );
}

//==============================================================================

s32 x_memcmp( const void* pBuf1, const void* pBuf2, s32 Count )
{
    byte* p1;
    byte* p2;
    byte* pEnd;

    ASSERT( pBuf1 );
    ASSERT( pBuf2 );
    ASSERT( Count >= 0 );

    p1   = (byte*)pBuf1;
    p2   = (byte*)pBuf2;
    pEnd = (byte*)pBuf1 + Count;

    //
    // Compare aligned data 32 bits at a time.
    //
    if( ( Count >= 4 ) && !((u32)p1 & 3) && !((u32)p2 & 3) )
    {
        pEnd -= 4;
        while( p1 <= pEnd )
        {
            if( *((u32*)p1) != *((u32*)p2) )
            {
                break;
            }

            p1 += 4;
            p2 += 4;
        }
        pEnd += 4;
    }

    //
    // Compare remaining data one byte at a time.
    //
    while( p1 < pEnd )
    {
        if( *p1 != *p2 )
        {
            if( *p1 < *p2 )  return( -1 );
            else             return(  1 );
        }

        p1++;
        p2++;
    }

    return( 0 );
}

//==============================================================================

u32 x_chksum( const void* pBuf, s32 Count )
{
    u32   Result = 0xAAAAAAAA;
    byte* p;
    byte* pEnd;
    
    ASSERT( pBuf );
    ASSERT( Count >= 0 );
    
    p    = (byte*)pBuf;
    pEnd = p + Count;

    while( p != pEnd )
    {
        u32 HiBit = Result & 0x80000000;

        Result  ^= *p;
        Result <<=  1;
        Result  |= (HiBit>>31);

        p++;
    }

    return( Result );
}

//==============================================================================
//  Standard ASCII conversion functions.
//==============================================================================

//==============================================================================
// Assumption: Letters A-Z and a-z are contiguous in the character set.
// This is true for ASCII and UniCode, but not for EBCDIC!

s32 x_toupper( s32 C )
{
    if( (C >= 'a') && (C <= 'z') )
        C += ('A' - 'a');
    return( C );
}

//==============================================================================
// Assumption: Letters A-Z and a-z are contiguous in the character set.
// This is true for ASCII and UniCode, but not for EBCDIC!

s32 x_tolower( s32 C )
{
    if( (C >= 'A') && (C <= 'Z') )
        C += ('a' - 'A');
    return( C );
}

//==============================================================================

s32 x_atoi( const char* pStr )
{
    char C;         // Current character.
    char Sign;      // If '-', then negative, otherwise positive.
    s32  Total;     // Current total.

    ASSERT( pStr );

    // Skip whitespace.
    for( ; *pStr == ' '; ++pStr )
        ; // empty body

     // Save sign indication.
    C = *pStr++;
    Sign = C;

    // Skip sign.
    if( (C == '-') || (C == '+') )
    {
        C = *pStr++;
    }

    Total = 0;

    while( (C >= '0') && (C <= '9') )
    {
        // Accumulate digit.
        Total = (10 * Total) + (C - '0');

        // Get next char.
        C = *pStr++;
    }

    // Negate the total if negative.
    if( Sign == '-' ) 
        Total = -Total;

    return( Total );
}

//==============================================================================
// This implementation will handle scientific notation such as "1.23e+7".

f32 x_atof( const char* pStr )
{
    s32   ISign   = 1;    // Integer portion sign.
    s32   IValue  = 0;    // Integer portion value.
    s32   DValue  = 0;    // Decimal portion numerator.
    s32   DDenom  = 1;    // Decimal portion denominator.
    xbool Integer = TRUE; // Is it an integer?

    ASSERT( pStr );

    // Skip whitespace.
    for( ; *pStr == ' '; ++pStr )
        ; // empty body

    // Get the sign.
    if( *pStr == '-' )  { pStr++; ISign = -1; }
    if( *pStr == '+' )  { pStr++;             }

    // Handle integer portion.  Accumulate integer digits.
    while( (*pStr >= '0') && (*pStr <= '9') )
    {
        IValue = (IValue<<3) + (IValue<<1) + (*pStr-'0');
        pStr++;
    }

    // Handle decimal portion.
    if( *pStr == '.' )
    {
        // Its not an integer any more!
        Integer = FALSE;

        // Skip the decimal point.
        pStr++;

        // Accumulate decimal digits.
        while( (*pStr >= '0') && (*pStr <= '9') )
        {
            DValue = (DValue<<3) + (DValue<<1) + (*pStr-'0');
            DDenom = (DDenom<<3) + (DDenom<<1);
            pStr++;
        }
    }

    // Handle scientific notation.
    if( (*pStr == 'e') || (*pStr == 'E') || (*pStr == 'd') || (*pStr == 'D') )
    {
        s32  EValue = 0;    // Exponent portion value.
        f32  ESign  = 1;    // Exponent portion sign.
        f32  e;             // Exponent order.

        // Skip the 'e' (or 'd').
        pStr++;

        // Get the sign for the exponent.
        if( *pStr == '-' )  { pStr++; ESign = -1; }
        if( *pStr == '+' )  { pStr++;             }

        // Accumulate exponent digits.
        while( (*pStr >= '0') && ( *pStr <= '9' ) )
        {
            EValue = (10 * EValue) + (*pStr - '0');
            pStr++;
        }

        // Compute exponent.  We make a shortcut for small exponents.
        if( EValue < 30 )
        {
            e = 1.0f;
            while( EValue )
            {
                e *= 10.0f;
                EValue --;
            }
        }
        else
        {
            e = x_pow( 10.0f, EValue );
        }

        // Return final number for exponent case.
        {
            f32 Temp = ISign * (IValue + ((f32)DValue / (f32)DDenom));

            if( ESign < 0 )  return( Temp / e );
            else             return( Temp * e );
        }
    }

    // No exponent
    if( Integer ) return( (f32)(ISign * IValue) );
    else          return( ISign * (IValue + ((f32)DValue/(f32)DDenom)) );
}

//==============================================================================
//  Path manipulation functions.
//==============================================================================

//==============================================================================
// Split path behavior based on Microsoft's implementation.

void x_splitpath( const char* pPath, char* pDrive,
                                     char* pDir,
                                     char* pFName,
                                     char* pExt )
{
    char* p;
    char* pLastSlash = NULL;
    char* pLastDot   = NULL;
    s32   Len;

    //
    // Extract drive letter and ':', if any.
    //

    if( (x_strlen(pPath) >= X_MAX_DRIVE-2) &&
        (pPath[X_MAX_DRIVE-2] == ':') )
    {
        if( pDrive != NULL )
        {
            x_strncpy( pDrive, pPath, X_MAX_DRIVE-1 );
            pDrive[X_MAX_DRIVE-1] = '\0';
        }
        pPath += X_MAX_DRIVE-1;
    }
    else
    if( pDrive )
    {
        // No drive.
        pDrive[0] = '\0';
    }

    //
    // Extract path string, if any.  pPath now points to first character of the
    // path, if present, or the filename or even the extension.  Look ahead for
    // the last path separator ('\' or '/').  If none is found, there is no 
    // path.  Also note the location of the last '.', if any, to assist in 
    // isolating the extension later.
    //

    for( p = (char*)pPath; *p; ++p )
    {
        if( (*p == '/') || (*p == '\\') )
            pLastSlash = p+1;
        else
        if( *p == '.' )
            pLastDot = p;
    }

    if( pLastSlash != NULL )
    {
        // There is a path.  Copy up to last slash or X_MAX_DIR chars.
        if( pDir != NULL )
        {
            Len = MIN( (pLastSlash - pPath), X_MAX_DIR-1 );
            x_strncpy( pDir, pPath, Len );
            pDir[Len] = '\0';
        }
        pPath = pLastSlash;
    }
    else
    if ( pDir != NULL )
    {
        // No path.
        pDir[0] = '\0';
    }

    //
    // Extract file name and extension, if any.  pPath now points to the first
    // character of the file name, if any, or to the extension, if any.  
    // pLastDot points to the beginning of the extension, if any.
    //

    if( (pLastDot != NULL) && (pLastDot >= pPath) )
    {
        // Found the marker for the extension.  Copy the file name through to 
        // the '.' marker.
        
        if( pFName != NULL )
        {
            Len = MIN( (pLastDot - pPath), X_MAX_FNAME-1 );
            x_strncpy( pFName, pPath, Len );
            pFName[Len] = '\0';
        }

        // Now get the extension.  Note that p still points to the NULL which 
        // terminates the path.

        if( pExt != NULL )
        {
            Len = MIN( (p - pLastDot), X_MAX_EXT-1 );
            x_strncpy( pExt, pLastDot, Len );
            pExt[Len] = '\0';
        }
    }
    else
    {
        if( pFName != NULL )
        {
            Len = MIN( (p - pPath), X_MAX_FNAME );
            x_strncpy( pFName, pPath, Len );
            pFName[Len] = '\0';
        }
        if ( pExt != NULL )
        {
            pExt[0] = '\0';
        }
    }
}

//==============================================================================
// Make path behavior based on Microsoft's implementation.

void x_makepath( char* pPath, const char* pDrive,
                              const char* pDir,
                              const char* pFName,
                              const char* pExt )
{
    const char* p;

    // Copy the drive specification, if given.

    if( pDrive && *pDrive )
    {
        *pPath++ = *pDrive;
        *pPath++ = ':';
    }

    // Copy the directory specification, if given.

    if( pDir && *pDir )
    {
        p = pDir;
        do
        {
            *pPath++ = *p++;
        } while( *p );

        p--;
        if( (*p != '/') && (*p != '\\') )
        {
            *pPath++ = '\\';
        }
    }

    // Copy the file name, if given.

    if( pFName )
    {
        p = pFName;
        while( *p )
        {
            *pPath++ = *p++;
        }
    }

    // Copy the extension, if given.  Add the '.' marker if not given.

    if( pExt )
    {
        p = pExt;
        if( *p && (*p != '.') )
        {
            *pPath++ = '.';
        }
        while( *p )
        {
            *pPath++ = *p++;
        }
    }

    // Terminate the string.
    *pPath = '\0';
}

//==============================================================================
