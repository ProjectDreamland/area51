
#ifndef TEXT_OUT_HPP
#define TEXT_OUT_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "x_stdio.hpp"
#include "x_math.hpp"
#include "x_color.hpp"

//=========================================================================
// text_out
//=========================================================================
class text_out
{
public:

                 text_out  ( void );
                ~text_out  ( void );

           void  OpenFile   ( const char* pFileName );
           void  CloseFile  ( void );

           void  AddHeader  ( const char* pHeaderName, s32 Count = -1 );

           void  AddField       ( const char* pName, ... );
           void  AddVector3     ( const char* pName, const vector3& V );
           void  AddColor       ( const char* pName, xcolor C );
           void  AddF32         ( const char* pName, f32 F );
           void  AddS32         ( const char* pName, s32 I );
           void  AddString      ( const char* pName, const char* pStr );
           void  AddBBox        ( const char* pName, const bbox& BBox );
           void  AddRadian3     ( const char* pName, const radian3& Orient );
           void  AddQuaternion  ( const char* pName, const quaternion& Q );
           void  AddBool        ( const char* pName, xbool Bool );
           void  AddGuid        ( const char* pName, guid  Guid );

           void  AddEndLine ( void );

    inline X_FILE* GetFp    ( void ) { return m_Fp; }

//=========================================================================
// END PUBLIC INTERFACE
//=========================================================================
protected:

    struct field
    {
        char  Name[256];        // Field description 
        s32   iType;            // where the types starts with in the fieldDesc
        s32   nTypes;           // How many types in the field        
        s32   HasNegative[64];  // Whether it countains negative digits
        s32   TotalSpace[64];   // How much space is requiere for each field
        s32   TotalSpaceBlock;  // Total space for this block
    };

    struct type_entry
    {
        s32     iOffset;
        s32     iType;
        s16     iField;
        s16     iBackOffset;
        s16     Length;
        s16     bDigit;
    };

protected:

    X_FILE*         m_Fp;               // Pointer to the file

    char*           m_pBlock;           // Pointer to the memory allocated
    s32             m_BlockSize;        // How much memory is allocated 
    s32             m_iBlock;           // Current offset from the block

    s32             m_nFields;          // Number of active fields
    field           m_Field[256];       // Fields that we know about

    s32             m_nTypesPerLine;    // number of types that are for each line
                                        // ( m_nTypesPerLine * m_LineCount ) == nTypeEntries
    s32             m_iTypeEntry;       // Curent entry
    type_entry*     m_pTypeEntry;       // memory allocated for each of the types

    s32             m_LineCount;        // How many lines does the user says it is going to be
    s32             m_iLine;            // Curent line that we are doing
    s32             m_iFiled;           // Curent filed

    char            m_BlockName[256];   // Block name
};

//=========================================================================
// end
//=========================================================================
#endif
