/*==========================================================================;
 *
 *  Copyright (C) 2000 - 2001 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       xgraphics.h
 *  Content:    Xbox graphics helper utilities
 *
 ****************************************************************************/

#ifndef _XGRAPHICS_H_
#define _XGRAPHICS_H_

/*****************************************************************************
 * 
 * Swizzler
 *
 * Purpose: To allow simple manipulations of a swizzled texture, without the 
 * hassle or overhead of unswizzling the whole thing in order to tweak a few 
 * points on the texture. This works with both 2D and 3D textures.
 * 
 * Notes: 
 *   Most of the time when messing with a texture, you will be incrementing
 *   by a constant value in each dimension.  Those deltas can be converted
 *   to an intermediate value via the SwizzleXXX(num) methods which can be
 *   used to quickly increment a dimension.
 *
 *   The type SWIZNUM is used to represent numbers returned by the SwizzleXXX()
 *   methods, also known as "intermediate values" in this documentation.
 * 
 *   Code in comments may be uncommented in order to provide some sort of 
 *   parameter sanity. It assures that any number passed to num will only 
 *   alter the dimension specified by dim.
 * 
 * Elements:
 *   
 *   m_u = texture map (converted) u coordinate
 *   m_v = texture map (converted) v coordinate
 *   m_w = texture map (converted) w coordinate
 * 
 *   m_MaskU = internal mask for u coordinate
 *   m_MaskV = internal mask for v coordinate
 *   m_MaskW = internal mask for w coordinate
 *
 *   m_Width = width of the texture this instance of the class has been initialized for
 *   m_Height = height of the texture this instance of the class has been initialized for
 *   m_Depth = depth of the texture this instance of the class has been initialized for
 * 
 * Methods:
 *   SWIZNUM SwizzleU(DWORD num) -- converts num to an intermediate value that
 *     can be used to modify the u coordinate
 *   SWIZNUM SwizzleV(DWORD num) -- converts num to an intermediate value that
 *     can be used to modify the v coordinate
 *   SWIZNUM SwizzleW(DWORD num) -- converts num to an intermediate value that
 *     can be used to modify the w coordinate
 *
 *   DWORD UnswizzleU(SWIZNUM index) -- takes an index to the swizzled texture, 
 *     and extracts & returns the u coordinate
 *   DWORD UnswizzleV(SWIZNUM index) -- takes an index to the swizzled texture, 
 *     and extracts & returns the v coordinate
 *   DWORD UnswizzleW(SWIZNUM index) -- takes an index to the swizzled texture, 
 *     and extracts & returns the w coordinate
 *
 *   SWIZNUM SetU(SWIZNUM num) -- sets the U coordinate to num, where num is an intermediate 
 *     value returned by Convert; returns num.
 *   SWIZNUM SetV(SWIZNUM num) -- sets the V coordinate to num, where num is an intermediate 
 *     value returned by Convert; returns num.
 *   SWIZNUM SetW(SWIZNUM num) -- sets the W coordinate to num, where num is an intermediate 
 *     value returned by Convert; returns num.
 *
 *   SWIZNUM AddU(SWIZNUM num) -- adds num to the U coordinate, where num is an intermediate value 
 *     returned by Convert; returns the new (swizzled) U coordinate
 *   SWIZNUM AddV(SWIZNUM num) -- adds num to the V coordinate, where num is an intermediate value 
 *     returned by Convert; returns the new (swizzled) V coordinate
 *   SWIZNUM AddW(SWIZNUM num) -- adds num to the W coordinate, where num is an intermediate value 
 *     returned by Convert; returns the new (swizzled) W coordinate
 *
 *   SWIZNUM SubU(SWIZNUM num) -- subtracts num from the U coordinate, where num is an intermediate value 
 *     returned by Convert; returns the new (swizzled) U coordinate
 *   SWIZNUM SubV(SWIZNUM num) -- subtracts num from the V coordinate, where num is an intermediate value 
 *     returned by Convert; returns the new (swizzled) V coordinate
 *   SWIZNUM SubW(SWIZNUM num) -- subtracts num from the W coordinate, where num is an intermediate value 
 *     returned by Convert; returns the new (swizzled) W coordinate
 *
 *   SWIZNUM IncU() -- increments the U coordinate by 1, returns the new (swizzled) U coordinate.
 *   SWIZNUM IncV() -- increments the V coordinate by 1, returns the new (swizzled) V coordinate.
 *   SWIZNUM IncW() -- increments the W coordinate by 1, returns the new (swizzled) W coordinate.
 *
 *   SWIZNUM DecU() -- decrements the U coordinate by 1, returns the new (swizzled) U coordinate.
 *   SWIZNUM DecV() -- decrements the V coordinate by 1, returns the new (swizzled) V coordinate.
 *   SWIZNUM DecW() -- decrements the W coordinate by 1, returns the new (swizzled) W coordinate.
 *
 *   SWIZNUM Get2() -- returns the index to the swizzled volume texture, based on 
 *     the U, and V coordinates, as modified by the previous methods.
 *
 *   SWIZNUM Get3() -- returns the index to the swizzled volume texture, based on 
 *     the U, V, and W coordinates, as modified by the previous methods.
 *
 * Performance:
 *   The algorithm used in most methods of this class require only Subtraction and a binary And
 *   operation to complete the operation. In the AddXXX methods, a negation, a subtraction, and two
 *   binary And operations are required. For this reason, the SubXXX methods are actually faster than
 *   AddXXX. Inc and Dec are roughly the same speed however.
 *
 ****************************************************************************/

#ifdef __cplusplus

typedef DWORD SWIZNUM;

class Swizzler 
{
public:

    // Dimensions of the texture
    DWORD m_Width;
    DWORD m_Height;
    DWORD m_Depth; 

    // Internal mask for each coordinate
    DWORD m_MaskU;
    DWORD m_MaskV;
    DWORD m_MaskW; 

    // Swizzled texture coordinates
    DWORD m_u;
    DWORD m_v;
    DWORD m_w;     

    Swizzler(): m_Width(0), m_Height(0), m_Depth(0),
        m_MaskU(0), m_MaskV(0), m_MaskW(0),
        m_u(0), m_v(0), m_w(0)
        { }

    // Initializes the swizzler
    Swizzler(
        DWORD width, 
        DWORD height, 
        DWORD depth
        )
    { 
        Init(width, height, depth);
    }

    void Init(
        DWORD width,
        DWORD height,
        DWORD depth
        )
    {
        m_Width = width; 
        m_Height = height; 
        m_Depth = depth;
        m_MaskU = 0;
        m_MaskV = 0;
        m_MaskW = 0;
        m_u = 0;
        m_v = 0;
        m_w = 0;

        DWORD i = 1;
        DWORD j = 1;
        DWORD k;

        do 
        {
            k = 0;
            if (i < width)   
            { 
                m_MaskU |= j;   
                k = (j<<=1);  
            }

            if (i < height)  
            { 
                m_MaskV |= j;   
                k = (j<<=1);  
            }

            if (i < depth)   
            {
                 m_MaskW |= j;   
                 k = (j<<=1);  
            }

            i <<= 1;
        } 
        while (k);
    }

    // Swizzles a texture coordinate
    SWIZNUM SwizzleU( 
        DWORD num 
        )
    {
        SWIZNUM r = 0;

        for (DWORD i = 1; i <= m_MaskU; i <<= 1) 
        {
            if (m_MaskU & i) 
            {
                r |= (num & i);
            }
            else
            {
                num <<= 1;
            }
        }

        return r;
    }

    SWIZNUM SwizzleV( 
        DWORD num 
        )
    {
        SWIZNUM r = 0;

        for (DWORD i = 1; i <= m_MaskV; i <<= 1) 
        {
            if (m_MaskV & i)
            {
                r |= (num & i);
            }
            else
            {
                num <<= 1;
            }
        }

        return r;
    }

    SWIZNUM SwizzleW( 
        DWORD num 
        )
    {
        SWIZNUM r = 0;

        for (DWORD i = 1; i <= m_MaskW; i <<= 1) 
        {
            if (m_MaskW & i)
            {
                r |= (num & i);
            }
            else
            {
                num <<= 1;
            }
        }

        return r;
    }

    SWIZNUM Swizzle(
        DWORD u, 
        DWORD v, 
        DWORD w
        )
    {
        return SwizzleU(u) | SwizzleV(v) | SwizzleW(w);
    }
    
    // Unswizzles a texture coordinate
    DWORD UnswizzleU( 
        SWIZNUM num
        )
    {
        DWORD r = 0;

        for (DWORD i = 1, j = 1; i; i <<= 1) 
        {
            if (m_MaskU & i)  
            {   
                r |= (num & j);   
                j <<= 1; 
            } 
            else               
            {   
                num >>= 1; 
            }
        }

        return r;
    }

    DWORD UnswizzleV( 
        SWIZNUM num 
        )
    {
        DWORD r = 0;

        for (DWORD i = 1, j = 1; i; i <<= 1) 
        {
            if (m_MaskV & i)  
            {   
                r |= (num & j);   
                j <<= 1; 
            } 
            else
            {   
                num >>= 1; 
            }
        }

        return r;
    }

    DWORD UnswizzleW( 
        SWIZNUM num 
        )
    {
        DWORD r = 0;

        for (DWORD i = 1, j = 1; i; i <<= 1) 
        {
            if (m_MaskW & i)  
            {   
                r |= (num & j);   
                j <<= 1; 
            } 
            else               
            {   
                num >>= 1; 
            }
        }

        return r;
    }

    // Sets a texture coordinate
    __forceinline SWIZNUM SetU(SWIZNUM num) { return m_u = num /* & m_MaskU */; }
    __forceinline SWIZNUM SetV(SWIZNUM num) { return m_v = num /* & m_MaskV */; }
    __forceinline SWIZNUM SetW(SWIZNUM num) { return m_w = num /* & m_MaskW */; }
    
    // Adds a value to a texture coordinate
    __forceinline SWIZNUM AddU(SWIZNUM num) { return m_u = ( m_u - ( (0-num) & m_MaskU ) ) & m_MaskU; }
    __forceinline SWIZNUM AddV(SWIZNUM num) { return m_v = ( m_v - ( (0-num) & m_MaskV ) ) & m_MaskV; }
    __forceinline SWIZNUM AddW(SWIZNUM num) { return m_w = ( m_w - ( (0-num) & m_MaskW ) ) & m_MaskW; }

    // Subtracts a value from a texture coordinate
    __forceinline SWIZNUM SubU(SWIZNUM num) { return m_u = ( m_u - num /* & m_MaskU */ ) & m_MaskU; }
    __forceinline SWIZNUM SubV(SWIZNUM num) { return m_v = ( m_v - num /* & m_MaskV */ ) & m_MaskV; }
    __forceinline SWIZNUM SubW(SWIZNUM num) { return m_w = ( m_w - num /* & m_MaskW */ ) & m_MaskW; }

    // Increments a texture coordinate
    __forceinline SWIZNUM IncU()              { return m_u = ( m_u - m_MaskU ) & m_MaskU; }
    __forceinline SWIZNUM IncV()              { return m_v = ( m_v - m_MaskV ) & m_MaskV; }
    __forceinline SWIZNUM IncW()              { return m_w = ( m_w - m_MaskW ) & m_MaskW; }

    // Decrements a texture coordinate
    __forceinline SWIZNUM DecU()              { return m_u = ( m_u - 1 ) & m_MaskU; }
    __forceinline SWIZNUM DecV()              { return m_v = ( m_v - 1 ) & m_MaskV; }
    __forceinline SWIZNUM DecW()              { return m_w = ( m_w - 1 ) & m_MaskW; }

    // Gets the current swizzled address for a 2D or 3D texture
    __forceinline SWIZNUM Get2D()          { return m_u | m_v; }
    __forceinline SWIZNUM Get3D()          { return m_u | m_v | m_w; }
};

#endif __cplusplus

/*
 * Swizzle methods.  These are implemented based on the above class
 * for the moment but will be further optimized in the future.
 */

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _XGPOINT3D
{
    DWORD u;
    DWORD v;
    DWORD w;
} XGPOINT3D;

// Returns whether a texture format is swizzled or not.
BOOL WINAPI XGIsSwizzledFormat(
    D3DFORMAT Format
    );

// Returns the byte per texel of a format.
DWORD WINAPI XGBytesPerPixelFromFormat(
    D3DFORMAT Format
    );

// Swizzle a subrectangle from a buffer into a larger texture.  The 
// destination rectangle must be completely contained within the destination 
// texture (no clipping).
//
// If pRect is NULL, pPoint is NULL and Pitch == 0, this routine will
// assume that the source buffer is exactly the same size as the destination
// texture and will swizzle the whole thing.  This routine will run
// considerably faster in that case.
//
VOID WINAPI XGSwizzleRect(
    LPCVOID  pSource,      // The buffer that contains the source rectangle
    DWORD    Pitch,        // The pitch of the buffer that contains the source
    LPCRECT  pRect,        // The rectangle within the buffer to copy.
    LPVOID   pDest,        // The destination texture.
    DWORD    Width,        // The width of the entire destination texture.
    DWORD    Height,       // The height of the entire destination texture.
    CONST LPPOINT pPoint,  // Where to put the rectangle in the texture.
    DWORD    BytesPerPixel
    );

// Unswizzle a subrectangle from a texture into a buffer.
//
// If pRect is NULL, pPoint is NULL and Pitch == 0, this routine will
// assume that the source texture is exactly the same size as the destination
// buffer and will unswizzle the whole thing.  This routine will run
// considerably faster in that case.
//
VOID WINAPI XGUnswizzleRect(
    LPCVOID  pSource,      // The source texture.
    DWORD    Width,        // The width of the entire source texture.
    DWORD    Height,       // The height of the entire source texture.
    LPCRECT  pRect,        // The rectangle within the texture to copy.
    LPVOID   pDest,        // The destination buffer
    DWORD    Pitch,        // The pitch of the destination buffer
    CONST LPPOINT pPoint,  // Where to copy the rectangle to
    DWORD    BytesPerPixel
    );

// Swizzle a box from a buffer into a larger texture.  The destination box 
// must be completely contained within the destination texture (no clipping).
//
VOID WINAPI XGSwizzleBox(
    LPCVOID     pSource,      // The buffer that contains the source rectangle
    DWORD       RowPitch,     // Byte offset from the left edge of one row to
                                // the left edge of the next row
    DWORD       SlicePitch,   // Byte offset from the top-left of one slice to
                                // the top-left of the next deepest slice
    CONST D3DBOX * pBox,      // The box within the buffer to copy.
    LPVOID      pDest,        // The destination texture.
    DWORD       Width,        // The width of the entire destination texture.
    DWORD       Height,       // The height of the entire destination texture.
    DWORD       Depth,        // The depth of the entire destination texture.
    CONST XGPOINT3D * pPoint, // Where to put the rectangle in the texture.
    DWORD       BytesPerPixel
    );

// Unswizzle a box from a texture into a buffer.
//
void WINAPI XGUnswizzleBox(
    LPCVOID     pSource,      // The source texture.
    DWORD       Width,        // The width of the entire source texture.
    DWORD       Height,       // The height of the entire source texture.
    DWORD       Depth,        // The depth of the entire destination texture.
    D3DBOX *    pBox,         // The rectangle within the texture to copy.
    LPVOID      pDest,        // The destination buffer
    DWORD       RowPitch,     // Byte offset from the left edge of one row to
                                // the left edge of the next row
    DWORD       SlicePitch,   // Byte offset from the top-left of one slice to
                                // the top-left of the next deepest slice
    XGPOINT3D * pPoint,       // Where to copy the rectangle to
    DWORD       BytesPerPixel
    );

#ifdef __cplusplus
}
#endif

/****************************************************************************
 *
 * Push buffer compiler.
 *
 ****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * The push-buffer compiler allows the title developer to pre-process
 *   some common commands into a push buffer to save the driver the work of
 *   having to process the commands into the format that the hardware
 *   understands at runtime.  This should be a big win for calls to the
 *   indexed draw primitives, which normally have to use the CPU to copy
 *   the indices every time.
 */

HRESULT WINAPI XGCompileDrawIndexedVertices(
    void *pBuffer,
    DWORD *pSize, // In: total size of buffer, Out: size of resulting push-buffer
    D3DPRIMITIVETYPE PrimitiveType,
    UINT VertexCount,
    CONST WORD *pIndexData
    );

#ifdef __cplusplus
}
#endif

/****************************************************************************
 *
 * XGBuffer:
 *
 * An object that is used to return arbitrary length data.
 *
 ****************************************************************************/

typedef struct XGBuffer XGBuffer;
typedef XGBuffer *LPXGBUFFER, *PXGBUFFER;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

HRESULT WINAPI XGBufferCreate(DWORD numBytes, LPXGBUFFER* ppBuffer);

/* XGBuffer */

ULONG   WINAPI XGBuffer_AddRef(XGBuffer *pThis);
ULONG   WINAPI XGBuffer_Release(XGBuffer *pThis);
LPVOID  WINAPI XGBuffer_GetBufferPointer(XGBuffer *pThis);
DWORD   WINAPI XGBuffer_GetBufferSize(XGBuffer *pThis);

#ifdef __cplusplus
}
#endif //__cplusplus

struct XGBuffer
{
    DWORD  refCount;            // The ref count.
    LPVOID pData;               // The data
    DWORD  size;                // The size of the buffer
#ifdef __cplusplus
    // IUnknown
    ULONG WINAPI AddRef() { return XGBuffer_AddRef(this); }
    ULONG WINAPI Release(){ return XGBuffer_Release(this); }

    // IXGBuffer methods
    LPVOID WINAPI GetBufferPointer() { return XGBuffer_GetBufferPointer(this); }
    DWORD  WINAPI GetBufferSize() { return XGBuffer_GetBufferSize(this); }
#endif // __cplusplus
};

/****************************************************************************
 *
 * Vertex and Pixel Shader Assembler.
 *
 ****************************************************************************/

/* Typedef of Resolver callback function used to process #include files
 *
 * This function is and called by the assembler to read files when the
 * #include statement is encountered.
 *
 * Parameters:
 *
 *    pResolverUserData
 *        This is arbitray data passed in to the AssembleShader
 *        function. Typically used to store context information
 *        for the resolver function.
 *    isSystemInclude
 *        A boolean value that is TRUE if the #include statement
 *        uses angle brackets, and FALSE if it uses double-quotes.
 *        The look-up rules are slightly different for
 *        the two types of files. Ordinary include files are first
 *        searched for in the same directory as the file that containst
 *        the #include statement. If the file is not found there,
 *        then additional directories are searched. System include
 *        files just search the additional directories. It's up to
 *        the function implementer to follow this rule.
 *    sourceFilePath
 *        The path of the file that contains the #include statement.
 *        Useful to implement searching for the #include file.
 *    includeFileName
 *        The name of the file to include. This has had the double-quotes
 *        and/or angle-brackets removed.
 *    resolvedFilePath
 *        Return the full path name of the file here.
 *    resolvedFilePathSize
 *        The size of the resolvedFilePath buffer.
 *    ppResolvedFile
 *        Used to return a pointer to an XGBuffer containing the text
 *        of the resolved file.
 *
 * return value:
 *
 *   Return SUCCESS if the file was found and read successfully.
 *   Return FAILURE if the file could not be found, or could not be read.
 */

typedef HRESULT (*SASM_ResolverCallback)(LPVOID pResolverUserData,
        BOOL isSystemInclude, LPCSTR sourceFilePath,
        LPCSTR includeFileName,
        LPSTR resolvedFilePath, DWORD resolvedFilePathSize,
        LPXGBUFFER* ppResolvedFile);


//-------------------------------------------------------------------------
// SASM flags:
// --------------
//
// SASM_DEBUG
//   Add debugging information to the token stream. Only effective if
//   used in combination with the SASM_OUTPUTTOKENS flag.
//
// SASM_SKIPVALIDATION
//   Don't validate the correctness of the shader.
//
// SASM_DONOTOPTIMIZE
//   Don't attempt to optimize the microcode.
//
// SASM_OUTPUTTOKENS
//   Output DX8 tokens instead of microcode.
//
// Only choose at most one of the SASM_INPUT_XXX_SHADER_TOKENS flags:
//
// SASM_INPUT_PIXELSHADER_TOKENS
//   Input is DX8 pixel shader tokens instead of ascii assembler source code.
//
// SASM_INPUT_VERTEXSHADER_TOKENS
//   Input is DX8 vertex shader tokens instead of ascii assembler source code.
//
// SASM_INPUT_READWRITE_VERTEXSHADER_TOKENS
//   Input is DX8 read/write vertex shader tokens instead of ascii assembler source code.
//
// SASM_INPUT_VERTEXSTATESHADER_TOKENS
//   Input is DX8 vertex state shader tokens instead of ascii assembler source code.
//
// SASM_INPUT_SCREENSPACE_VERTEXSHADER_TOKENS
//   Input tokens are for a #pragma screenspace vertex shader. Only valid if combined
//   with SASM_INPUT_VERTEXSHADER_TOKENS or SASM_INPUT_READWRITE_VERTEXSHADER_TOKENS.
//
// SASM_INPUT_NONXBOX_TOKENS
//   Input tokens are for an ordinary, non-Xbox shader. Only valid if combined
//   with SASM_INPUT_VERTEXSHADER_TOKENS or SASM_INPUT_PIXELSHADER_TOKENS.
//
// SASM_INPUT_MICROCODE
//   Input is microcode. Useful for optimizing hand-built code.
//
// SASM_INPUT_SCREENSPACE_MICROCODE
//   Input is screenspace microcode. Useful for optimizing hand-built code.
//
// SASM_PREPROCESSONLY
//   Run the preprocessor, and copy output of the preprocessor to the output.
//
// SASM_DISABLE_GLOBAL_OPTIMIZATIONS
//   Disable global optimizations in the vertex shader.
//
// SASM_VERIFY_OPTIMIZATIONS
//   Verify that the optimized shader produces the same result as the
//   original shader. Use this if you suspect that your vertex shader is
//   being optimized incorrectly. (There can be a substantial speed and
//   memory penalty for using this flag.)
// 
// SASM_USE_V1_OPTIMIZER
//   Use old (non-graph-based) version of the vertex shader optimizer.
// 
// SASM_USE_V2_OPTIMIZER
//   Use new (graph-based) version of the vertex shader optimizer.
//
// SASM_PACKMATRIX_ROWMAJOR
// SASM_PACKMATRIX_COLUMNMAJOR
//
//-------------------------------------------------------------------------

#define SASM_DEBUG                                  (1 << 0)
#define SASM_SKIPVALIDATION                         (1 << 1)
#define SASM_DONOTOPTIMIZE                          (1 << 2)
#define SASM_OUTPUTTOKENS                           (1 << 3)
#define SASM_INPUT_PIXELSHADER_TOKENS               (1 << 4)
#define SASM_INPUT_VERTEXSHADER_TOKENS              (1 << 5)
#define SASM_INPUT_READWRITE_VERTEXSHADER_TOKENS    (1 << 6)
#define SASM_INPUT_VERTEXSTATESHADER_TOKENS         (1 << 7)
#define SASM_INPUT_SCREENSPACE_VERTEXSHADER_TOKENS  (1 << 8)
#define SASM_INPUT_NONXBOX_TOKENS                   (1 << 9)
#define SASM_INPUT_MICROCODE                        (1 << 10)
#define SASM_INPUT_SCREENSPACE_MICROCODE            (1 << 11)
#define SASM_PREPROCESSONLY                         (1 << 12)
#define SASM_SKIPPREPROCESSOR                       (1 << 13)
#define SASM_DISABLE_GLOBAL_OPTIMIZATIONS           (1 << 14)
#define SASM_VERIFY_OPTIMIZATIONS                   (1 << 15)
#define SASM_USE_V1_OPTIMIZER                       (1 << 16)
#define SASM_USE_V2_OPTIMIZER                       (1 << 17)
#define SASM_PACKMATRIX_ROWMAJOR                    (1 << 18)
#define SASM_PACKMATRIX_COLUMNMAJOR                 (1 << 19)

//-------------------------------------------------------------------------
// SASMT values:
// --------------
//
// SASMT_PIXELSHADER
//   A pixel shader.
//
// SASMT_VERTEXSHADER
//   An ordinary vertex shader.
//
// SASMT_READWRITE_VERTEXSHADER
//   A vertex shader that can write to the constant registers.
//
// SASMT_VERTEXSTATESHADER
//   A vertex state shader.
//
//
// SASMT_SCREENSPACE
//   For vertex shaders and read/write vertex shaders, indicates that the
//   shader outputs screen space coordinates rather than clip space coordinates.
//
//
// SASMT_VERTEXSHADER thru SASMT_VERTEXSTATESHADER are
// guaranteed to have the same values as D3DMT_VERTEXSHADER
// thru D3DMT_VERTEXSTATESHADER.
//
//-------------------------------------------------------------------------

#define SASMT_PIXELSHADER               0
#define SASMT_VERTEXSHADER              1
#define SASMT_READWRITE_VERTEXSHADER    2
#define SASMT_VERTEXSTATESHADER         3
#define SASMT_INVALIDSHADER             0xff
#define SASMT_SCREENSPACE               0x100
#define SASMT_FRAGMENT                  0x200
#define SASMT_SHADERTYPEMASK            0xff

#define SASMT_SHADERTYPE(X) ((X) & SASMT_SHADERTYPEMASK)
#define SASMT_ISSCREENSPACE(X) (((X) & SASMT_SCREENSPACE) != 0)
#define SASMT_ISFRAGMENT(X) (((X) & SASMT_FRAGMENT) != 0)


//-------------------------------------------------------------------------
// XGAssembleShader:
// ------------------------
// Assembles an ASCII description of a vertex or pixel shader into 
// binary form.
//
// Parameters:
//
//  pSourceFileName
//      Source file name - used in error messages
//  pSrcData
//      A pointer to the source data
//  SrcDataLen
//      The source data length
//  Flags
//      SASM_xxx flags
//  pConstants
//      If constants are declared in the shader, they are written here. Pass NULL if
//      you don't care.
//  pCompiledShader
//      The shader microcode is written here. Pass NULL if you don't care.
//  pErrorLog
//      Errors are written here. Pass NULL if you don't care.
//  pListing
//      A human-readable listing is written here. Pass NULL if you don't want it.
//  pResolver
//      Used by the preprocessor. Can be NULL if you don't use #include in your source file.
//  pResolverUserData
//      Passed unmodified to the pResolver function.
//  pShaderType
//      Returns the type of shader that was assembled. Pass NULL if you don't care.
//
// Return value:
//    Returns S_OK if no errors.
//    Returns a failure code if an error occured. For problems with the
//    assembly program syntax, human-readable errors and warnings are
//    written to the pErrorLog.
//-------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

HRESULT WINAPI
XGAssembleShader(
    LPCSTR pSourceFileName,
    LPCVOID pSrcData,
    UINT SrcDataLen,
    DWORD Flags,
    LPXGBUFFER* pConstants,
    LPXGBUFFER* pCompiledShader,
    LPXGBUFFER* pErrorLog,
    LPXGBUFFER* pListing,
    SASM_ResolverCallback pResolver,
    LPVOID pResolverUserData,
    LPDWORD pShaderType
    );

#define AssembleShader XGAssembleShader

//-------------------------------------------------------------------------
// XGCompileShader:
// ------------------------
// Compiles HLSL shader into either assembly or binary form
//
// Parameters:
//
//  pSourceFileName
//      Source file name - used in error messages
//  pSrcData
//      A pointer to the source data
//  SrcDataLen
//      The source data length
//  Flags
//      SASM_xxx flags
//  pEntryName
//      Entry point name to compile (i.e. main)
//  pTargetName
//      Name of target assembly to compile to (i.e. vs.1.1, xvs.1.1)
//          vs.1.1 - DX8 vs.1.1 target
//         xvs.1.1 - vs.1.1 and dph, negative constants, oB* registers
//        xvss.1.1 - xvs.1.1 with pragma screenspace (all constants available)
//      Note: Pixel shader compilation (ps.1.1 and xps.1.1) is not currently supported.
//  pConstants
//      If constants are declared in the shader, they are written here. Pass NULL if
//      you don't care.
//  pCompiledShader
//      The shader microcode is written here. Pass NULL if you don't care.
//  pErrorLog
//      Errors are written here. Pass NULL if you don't care.
//  pAsmListing
//      A human-readable listing of DX8 asm is written here. Pass NULL if you don't want it.
//  pMachineListing
//      A human-readable listing of machine code is written here. Pass NULL if you don't want it.
//  pResolver
//      Used by the preprocessor. Can be NULL if you don't use #include in your source file.
//  pResolverUserData
//      Passed unmodified to the pResolver function.
//  pShaderType
//      Returns the type of shader that was assembled. Pass NULL if you don't care.
//
// Return value:
//    Returns S_OK if no errors.
//    Returns a failure code if an error occured. For problems with the
//    HLSL syntax, human-readable errors and warnings are
//    written to the pErrorLog.
//-------------------------------------------------------------------------

HRESULT WINAPI
XGCompileShader(
    LPCSTR pSourceFileName,
    LPCVOID pSrcData,
    UINT SrcDataLen,
    DWORD Flags,
    LPCSTR pEntryName,
    LPCSTR pTargetName, 
    LPXGBUFFER* pConstants,
    LPXGBUFFER* pCompiledShader,
    LPXGBUFFER* pErrorLog,
    LPXGBUFFER* pAsmListing,
    LPXGBUFFER* pMachineListing,
    SASM_ResolverCallback pResolver,
    LPVOID pResolverUserData,
    LPDWORD pShaderType
    );

#define CompileShader XGCompileShader

//XGSpliceVertexShaders:
//  Splice together shaders in the ppShaderArray, return it in *pNewShader.
//  If pcbNewShaderBufferSize is provided and is too small, it will be changed to the minimum allowable buffer size, and will return S_FALSE. 
//      pNewShader can be NULL in this case. If pcbNewShaderBufferSize is NULL or points to a non-zero size, pNewShader must not be NULL.
//  The return value will be S_OK or S_FALSE. If optimizing in low-mem conditions, it can run out of memory, and will return an error code.
//  If bad params are passed, it will assert.
HRESULT WINAPI XGSpliceVertexShaders (
    /*             OUT  */  DWORD*   pNewShader,              //pointer to buffer to fill with output
    /* OPTIONAL IN OUT  */  DWORD*   pcbNewShaderBufferSize, //How many bytes long the shader buffer is
    /* OPTIONAL    OUT  */  DWORD*   pNewInstructionCount,   //how many instrucitons are in the newly-spliced shader
    /*    IN      */  CONST DWORD* CONST*  ppShaderArray,          //arrray of pointers to shaders to splice together
    /*          IN      */  DWORD    NumShaders,             //num of shaders in ppShaderArray
    /*          IN      */  BOOL     bOptimizeResults        //TRUE to optimize, FALSE to not optimize
);




// Examines vertex shader microcode, and determines the type of vertex shader.
//
// Parameters:
//
//  pMicrocode
//      A pointer to the vertex shader microcode.
//
// Return value:
//   Returns
//      Returns the type of shader that is pointed to by pMicrocode. Due to
//      implementation restrictions, the SASMT_SCREENSPACE bit will not be
//      set, even if the microcode is for a screen space vertex shader.
//
//      If the microcode is invalid, the result is SASMT_INVALIDSHADER.

DWORD WINAPI XGSUCode_GetVertexShaderType(
    LPCVOID pMicrocode
    );

// Examines shader microcode, and determines the length
// in instructions.
//
// Parameters:
//
//  pMicrocode
//      A pointer to the vertex shader microcode.
//
// Return value:
//      Returns the length of the vertex shader in microinstructions.
//      If the microcode is not a valid vertex shader, the result is undefined.

DWORD WINAPI XGSUCode_GetVertexShaderLength(
    LPCVOID pMicrocode
    );

// Compares two vertex shaders to see if they produce equivalent results.
//
// Parameters:
//
//  pMicrocodeA
//      A pointer to a vertex shader microcode program.
//  pMicrocodeB
//      A pointer to a vertex shader microcode program.
//  ppErrorLog
//      Differences between the two shaders results are written here. Pass NULL if you don't care.
//
// Return value:
//      Returns S_OK if the two shaders produce equivalent results.
//      Returns a failure code if the two shaders do not produce equivalent results, or
//      if there was an internal error.
//
//      If either microcode program is not valid vertex shader, the result is undefined.

HRESULT WINAPI XGSUCode_CompareVertexShaders(
    LPCVOID pMicrocodeA,
    LPCVOID pMicrocodeB,
    LPXGBUFFER* ppErrorLog
    );


/*****************************************************************************
 * 
 * XGWriteSurfaceToFile
 *
 * Purpose: 
 *   Allows the contents of a surface to be written to a 24-bit .bmp file.
 *   The following surface formats are supported:
 *     D3DFMT_LIN_A8R8G8B8
 *     D3DFMT_LIN_X8R8G8B8
 *     D3DFMT_LIN_R5G6B5
 *     D3DFMT_LIN_R5G6B5
 *     D3DFMT_LIN_X1R5G5B5
 *
 *   These are the formats that are possible for a frontbuffer.  Swizzled
 *   formats are not currently supported.
 *
 *   Requires creation of a temporary buffer 3*heigh*width bytes big.  If 
 *   this allocation fails, the file is written more slowly using a much 
 *   smaller buffer.
 *
 *   pSurf holds a pointer to a surface.
 *   cPath is a character string that contains the drive, path and filename
 *   for the output file.  If the file exists, it will be overwritten.  The
 *   filename should have a .bmp extension.
 *
 ****************************************************************************/

HRESULT WINAPI XGWriteSurfaceToFile(
    IDirect3DSurface8 *pSurf,
    const char *cPath
    );


/*****************************************************************************
 * 
 * XPR structures and constants
 *
 * Purpose: 
 *   The XPR file format allows multiple graphics resources to be pre-defined
 *   and bundled together into one file.  These resources can be copied into
 *   memory and then immediately used in-place as D3D objects such as textures
 *   and vertex buffers.  The structure below defines the XPR header and the 
 *   unique identifier for this file type.
 *
 ****************************************************************************/
typedef struct {
    DWORD dwMagic;
    DWORD dwTotalSize;
    DWORD dwHeaderSize;
} XPR_HEADER;

#define XPR_MAGIC_VALUE 0x30525058


/*****************************************************************************
 * 
 * XGWriteSurfaceOrTextureToXPR
 *
 * Purpose: 
 *   This utility functions allows a single surface or texture to be saved
 *   to a packed resource file (.xpr).  All surface and texture formats
 *   supported by Direct3D are supported by this function.
 *
 *   pResource holds a pointer to a surface or texture.
 *   cPath is a character string that contains the drive, path and filename
 *   for the output file.  If the file exists, it will be overwritten.  The
 *   filename should have a .xpr or .xbx extension.
 *   if bWriteSurfaceAsTexture is TRUE and pResource is a surface, the 
 *   resource is converted to a texture before writing it to the xpr file.
 *
 *   Note that on Silver XDK boxes, the texture or surface must reside in 
 *   AGP memory.
 *
 ****************************************************************************/
HRESULT WINAPI XGWriteSurfaceOrTextureToXPR(
    IDirect3DResource8 *pResource, 
    const char *cPath,
    BOOL bWriteSurfaceAsTexture
    );


/*****************************************************************************
 * 
 * XGCompressRect
 *
 * Purpose: 
 *   This function will compress a rectangle into one of the Xbox supported 
 *   compressed texture formats: DXT1, DXT2, DXT3, DXT4, or DXT5.  The format
 *   should be specified in the format argument.  The compressed texture data 
 *   is written out to pDestBuf, which can point to a D3DCreated texture or a 
 *   contiguous memory allocation to be used with the Register() API
 *
 * Parameters:
 * DestFormat 
 *      Should be one of the Xbox DXT format enums
 * dwDestPitch
 *      Should be the pitch of the destination, in terms of a row
 *      of 4x4 blocks.  So a DXT1 texture with a width of 16 pixels has a
 *      pitch of 4 blocks * 8 bytes per block = 32 bytes
 * pSrcData 
 *      Should point to linear (not swizzled) texture data
 * SrcFormat
 *      Should be one of the Xbox linear ARGB or XRGB texture formats
 * dwSrcPitch
 *      Should be the pitch of the source texture, in bytes
 * fAlphaRef 
 *      The cutoff between transparent and opaque for DXT1
 * dwFlags:
 *  XGCOMPRESS_PREMULTIPLY specifies whether or not to premultiply 
 *      by alpha (DXT2/4)
 *  XGCOMPRESS_NEEDALPHA0(1) specify that interpolated alpha should ensure
 *      that 0(1) is one of the resultant values
 *  XGCOMPRESS_PROTECTNONZERO specifies that non-zero alpha values should
 *      not be quantized to zero.
 ****************************************************************************/
#define XGCOMPRESS_PREMULTIPLY      0x1
#define XGCOMPRESS_NEEDALPHA0       0x2
#define XGCOMPRESS_NEEDALPHA1       0x4
#define XGCOMPRESS_PROTECTNONZERO   0x8

HRESULT WINAPI XGCompressRect(
    LPVOID pDestBuf,
    D3DFORMAT DestFormat,
    DWORD dwDestPitch, 
    DWORD dwWidth,
    DWORD dwHeight,
    LPVOID pSrcData,
    D3DFORMAT SrcFormat,
    DWORD dwSrcPitch,
    FLOAT fAlphaRef,
    DWORD dwFlags
    );


/*****************************************************************************
 * 
 * XGSetSurfaceHeader
 *
 ****************************************************************************/

DWORD WINAPI XGSetSurfaceHeader(
    UINT Width,
    UINT Height,
    D3DFORMAT Format,
    IDirect3DSurface8* pSurface,
    UINT Data,                  // Offset to the data held by this resource
    UINT Pitch                  // Surface pitch
    );


/*****************************************************************************
 * 
 * XGSetTextureHeader
 *
 ****************************************************************************/

DWORD WINAPI XGSetTextureHeader(
    UINT Width,
    UINT Height,
    UINT Levels,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    IDirect3DTexture8* pTexture,
    UINT Data,                  // Offset to the data held by this resource
    UINT Pitch                  // Texture pitch
    );


/*****************************************************************************
 * 
 * XGSetCubeTextureHeader
 *
 ****************************************************************************/

DWORD WINAPI XGSetCubeTextureHeader(
    UINT EdgeLength,
    UINT Levels,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    IDirect3DCubeTexture8* pCubeTexture,
    UINT Data,                  // Offset to the data held by this resource
    UINT Pitch                  // CubeTexture pitch
    );


/*****************************************************************************
 * 
 * XGSetVolumeTextureHeader
 *
 ****************************************************************************/

DWORD WINAPI XGSetVolumeTextureHeader(
    UINT Width,
    UINT Height,
    UINT Depth,
    UINT Levels,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    IDirect3DVolumeTexture8* pVolumeTexture,
    UINT Data,                  // Offset to the data held by this resource
    UINT Pitch                  // VolumeTexture pitch
    );

/*****************************************************************************
 * 
 * XGSetVertexBufferHeader
 *
 ****************************************************************************/

VOID WINAPI XGSetVertexBufferHeader(
    UINT Length,
    DWORD Usage,
    DWORD FVF,
    D3DPOOL Pool,
    IDirect3DVertexBuffer8 *ppVertexBuffer,
    UINT Data
    );

/*****************************************************************************
 * 
 * XGSetIndexBufferHeader
 *
 ****************************************************************************/

VOID WINAPI XGSetIndexBufferHeader(
    UINT Length,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    IDirect3DIndexBuffer8 *pIndexBuffer,
    UINT Data
    );

#ifdef _XBOX

/*****************************************************************************
 * 
 * XGSetPaletteHeader
 *
 ****************************************************************************/

VOID WINAPI XGSetPaletteHeader(
    D3DPALETTESIZE Size, 
    IDirect3DPalette8 *pPalette,
    UINT Data
    );

/*****************************************************************************
 * 
 * XGSetPushBufferHeader
 *
 ****************************************************************************/

VOID WINAPI XGSetPushBufferHeader(
    UINT Size,
    BOOL RunUsingCpuCopy,
    IDirect3DPushBuffer8 *pPushBuffer,
    UINT Data
    );

/*****************************************************************************
 * 
 * XGSetFixupHeader
 *
 ****************************************************************************/

VOID WINAPI XGSetFixupHeader(
    UINT Size,
    IDirect3DFixup8 *pFixup,
    UINT Data
    );

#endif // _XBOX

#ifdef __cplusplus
}
#endif //__cplusplus

#endif /* _XGRAPHICS_H_ */

