//==============================================================================
//  
//  x_array.hpp
//  
//==============================================================================

#ifndef X_ARRAY_HPP
#define X_ARRAY_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

//==============================================================================
//  TYPES
//==============================================================================

//==============================================================================
//  TEMPLATIZED DYNAMIC ARRAY
//==============================================================================
//  
//  Yet another templatized dynamic array!  This one has one advantage over all 
//  the others, though... it's in the x_files.
//  
//  The class T which is used to instantiate the template must satisfy a few
//  requirements:
//      - Void constructor, "T( void )".
//      - Assignment, "operator = ( const T& )".
//      - Equality test, "operator == ( const T& )", if Find is used.
//  
//  The Count represents how many elements are currently in the xarray.  The 
//  Capacity indicates how many elements can be held without the need for the 
//  xarray to re-allocate its internal array.
//  
//  The xarray class tries to minimize the number of dynamic memory operations
//  performed.  However, it relies on information provided by the SetCapacity 
//  function.
//  
//  An xarray can be prevented from automatically adjusting its capacity by 
//  "locking" it with SetLocked( TRUE ).  The xarray will behave normally as 
//  long as no alterations to the capacity are required.  The xarray can be 
//  unlocked at any time.
//  
//  An xarray can be constructed "around" any pre-existing normal array via the
//  "xarray( T*, s32, s32 )" constructor.  The resulting xarray will consider 
//  itself "static" and permanently locked.  All functions will perform as 
//  expected as long as no capacity changes are required.  Upon destruction, a
//  static xarray will not attempt to delete the pointer to the data array.
//  
//  When the run-time validation macros are enabled, that is when X_ASSERT is
//  defined, all of the functions perform usage validations.  Of particular note
//  is that operator [] has index range checks.
//
//==============================================================================
//  
//  xarray( T*, s32, s32 )  - Construct in "static" mode using given array 
//                            pointer.  Adds the xarray interface to any 
//                            pre-existing normal array.
//  
//  operator T*             - Cast to "T*" function.  Returns the address of the 
//                            internal data array.
//  
//  Append( void )          - Append a void constructed element to the array and
//                            return a reference to this newly added element.
//                            This provides the opportunity to "set up" an 
//                            object "in place" rather than setting it up in an 
//                            external instance and then copying the object into
//                            the xarray.
//  
//  SetCapacity - Attempts to set the Capacity to the given value.  Only valid 
//                if not Locked, not Static, and the given Capacity is not less
//                than the current Count.  This can cause the Capacity to be
//                increased or decreased.
//  
//  Clear       - Sets the Count to zero.  If not Static and not Locked, this 
//                will release the allocation thus setting the Capacity to zero.
//  
//  FreeExtra   - Only valid when not Static and not Locked.  If the Capacity
//                exceeds the Count, then the dynamic allocation is reduced to
//                the minimum necessary size, and Capacity is set to Count.
//  
//==============================================================================

template <class T>
class xarray
{
public:                         
                    xarray          ( void );
                    xarray          ( const xarray<T>& Array   );
                    xarray          ( T* Array, s32 Capacity, s32 Count=0 );
                   ~xarray          ( void );

                    operator T*     ( void ) const; 
        T&          operator []     ( s32 Index ) const;

        T&          GetAt           ( s32 Index ) const;
        void        SetAt           ( s32 Index, const T& Element );
        
        void        Insert          ( s32 Index, const T& Element );
        void        Insert          ( s32 Index, const xarray<T>& Array );
        void        Delete          ( s32 Index, s32 Count=1 );
        
        T&          Append          ( void );
        void        Append          ( const T& Element );
        void        Append          ( const xarray<T>& Array );
        
        s32         GetCount        ( void ) const;
        s32         GetCapacity     ( void ) const;
        void        SetCapacity     ( s32 Capacity );
        
        void        Clear           ( void );
        void        FreeExtra       ( void );
        
        s32         Find            ( const T& Element, s32 StartIndex=0 ) const;

        void        SetLocked       ( xbool Locked );
        xbool       IsLocked        ( void ) const;
        xbool       IsStatic        ( void ) const;
        const T*    GetData         ( void ) const { return m_pData; };

const   xarray<T>&  operator =      ( const xarray<T>& Array   );
const   xarray<T>&  operator +=     ( const xarray<T>& Array   );
const   xarray<T>&  operator +=     ( const T&         Element );
const   xarray<T>   operator +      ( const xarray<T>& Array   ) const; 
                
protected:
        T*          m_pData; 
        s32         m_Count;   
        s32         m_Capacity;
        s32         m_Status;
};

//==============================================================================
//  PRIVATE
//==============================================================================

#include "Implementation/x_array_private.hpp"

//==============================================================================
#endif // X_ARRAY_HPP
//==============================================================================
