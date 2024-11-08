#ifndef __CONTROLLER_H
#define __CONTROLLER_H

#include "x_files.hpp"
#include "x_bytestream.hpp"
#include "export.hpp"
#include "igfmgr.hpp"

namespace fx_core
{

//============================================================================
// Misc
#define BAD_TIME    0xDEADBEEF

//============================================================================
// controller names
#define LINEAR_KEY  "Linear Key"
#define SMOOTH_KEY  "Smooth Key"


//============================================================================
class   controller;
typedef controller* (*ctrl_factory_fn)( void );

struct ctrl_reg
{
    const char*         m_pTypeName;
    ctrl_factory_fn     m_pFactoryFn;   


                        ctrl_reg( const char*       pTypeName,
                                  ctrl_factory_fn   FactoryFn );
};

//============================================================================
// Make a controller using the factory
controller* MakeController( const char* pType );

//============================================================================
// Generic controller class (should be used only as a base class)
class controller
{
protected:
                                                                    // (This will be used by the factory)
    u32                 m_SerialNum;                                // Unique serial number

public:
    u32                 m_ExpIdx;                                   // used during export

    enum out_of_range
    {
        CLAMP,
        LOOP,
        PINGPONG
    };

protected:
    out_of_range        m_InType, m_OutType;                        // how to deal with out-of-range
    f32                 m_Scalar;                                   // optional scalar, defaults to 1.0f

public:

    virtual             ~controller() {};
                        controller();
    
            u32         GetSerNum   ( void )              { return m_SerialNum; }

            void        SetScalar   ( f32 Scalar )        { m_Scalar = Scalar;  }

    virtual const char* GetType     ( void )              const = 0;
    virtual xbool       GetValue    ( f32 T, f32* pVals ) const = 0;// return the value at time T
    virtual s32         GetNumFloats( void )              const = 0;// number of floats we're controlling

    virtual void        SaveData    ( igfmgr& fp )        const = 0;// controller can save it's own data
    virtual void        LoadData    ( igfmgr& fp )              = 0;// controller can load it's own data

    virtual void        ExportData  ( export::fx_controllerhdr& ContHdr,
                                      xstring&                  Type,
                                      xbytestream&              Stream ) = 0;
    
    virtual controller* CopyOf      ( void )              const = 0;// Helper for constructing copies

    out_of_range        GetInType   ( void ) const          { return m_InType;  }
    out_of_range        GetOutType  ( void ) const          { return m_OutType; }
    void                SetInType   ( out_of_range Type )   { m_InType  = Type; }
    void                SetOutType  ( out_of_range Type )   { m_OutType = Type; }

    static s32          OutOfRangeType_FromString         ( const char* pString );
    static const char*  OutOfRangeType_ToString           ( s32 OutOfRangeType );
};

//============================================================================
// A key
class key
{
    s32                 m_Time;                                     // The occurrence time (T)                                    
    f32*                m_pData;                                    // data for a key
    s32                 m_nVals;                                    // number of float values in this key
    xbool               m_IsValid;                                  // has this key been initialized

public:
                        key         ( s32 nVals )                   { m_pData = new f32[nVals]; m_nVals = nVals; m_Time = BAD_TIME; }
                        ~key        ( )                             { delete[] m_pData; }
                      
    key*                CopyOf      ( void              );          // duplicate a key
                                                                   
    void                SetKey      ( s32 T, f32* pVals )           { m_Time = T; x_memmove( m_pData, pVals, m_nVals * sizeof(f32) ); }
    void                SetKeyTime  ( s32 T             )           { m_Time = T; }
    void                SetKeyData  ( s32* pVals        )           { x_memmove( m_pData, pVals, m_nVals * sizeof(f32) ); }                                                                    
    s32                 GetKeyTime  ( void )                        { return m_Time;    }  //const;
    f32*                GetKeyValue ( void )                        { return m_pData;   }  //const;

    void                SaveData    ( igfmgr& Igf       );
    void                LoadData    ( igfmgr& Igf       );
                                        
};

//============================================================================
// generic keyed-type controller (meant to be used as a base class)
class ctrl_key : public controller
{
protected:
    xarray<key*>        m_Keys;                                     // the key container
    s32                 m_nFloats;                                  // the number of values in each key

    void                SortByT             ( void );               // private function to sort by time

public:
    virtual             ~ctrl_key();

    const char*         GetType             ( void )              const { return NULL; }
    void                SetNumFloats        ( s32 nFloats )             { m_nFloats = nFloats; }
    void                SetValue            ( s32 T, key* pKey );   // descendants can validate data
    virtual xbool       GetValue            ( f32 T, f32* pVals ) const { return FALSE; }// return the value at time T                                            
    virtual s32         GetNumFloats        ( void )              const ; // number of floats we're controlling 
    virtual void        SaveData            ( igfmgr& Igf )       const ;// controller can save it's own data
    virtual void        LoadData            ( igfmgr& Igf )             ;// controller can load it's own data

    virtual void        ExportData          ( export::fx_controllerhdr& ContHdr,
                                              xstring&                  Type,
                                              xbytestream&              Stream ) ;

    controller*         CopyOf              ( void )              const ; // Helper for constructing copies
    key*                GetKeyByIndex       ( s32 Idx )   const;
    s32                 GetKeyCount         ( void )                { return m_Keys.GetCount(); }
    s32                 GetOptimalKeyCount  ( s32* pChannels );
    void                GetBookends         ( key** FirstKey, key** LastKey ) const;
    s32                 GetMinT             ( void ) const;
    s32                 GetMaxT             ( void ) const;
    xbool               DeleteKeyByIndex    ( s32 Idx );
    xbool               DeleteKey           ( s32 T   );
};

//============================================================================
// linear keyframe controller
class ctrl_linear : public ctrl_key
{

protected:
    
    xbool               m_IsSmooth;

public:

                        ctrl_linear();

    const char*         GetType     ( void )              const { return LINEAR_KEY; }
    xbool               GetValue    ( f32 T, f32* pVals ) const;           // return the value at time T
    void                GetTangent  ( s32 KeyIndex, f32* pVals ) const;    // return the tangency values for a key
    s32                 GetNumFloats( void )              const;           // number of floats we're controlling                                                                           
    void                SaveData    ( igfmgr& Igf )       const;           // controller can save it's own data
    void                LoadData    ( igfmgr& Igf )            ;           // controller can load it's own data
    controller*         CopyOf      ( void )              const;           // Helper for constructing copies

    xbool               IsSmooth    ( void ) const          { return m_IsSmooth; }
    void                SetSmooth   ( xbool IsSmooth )      { m_IsSmooth = IsSmooth; }

    virtual void        ExportData  ( export::fx_controllerhdr& ContHdr,
                                      xstring&                  Type,
                                      xbytestream&              Stream ) ;
};

//============================================================================
// smooth keyframe controller
class ctrl_smooth : public ctrl_key
{
public:
                        const char*         GetType     ( void )              const { return SMOOTH_KEY; }
    xbool               GetValue    ( f32 T, f32* pVals ) const;           // return the value at time T
    s32                 GetNumFloats( void )              const;           // number of floats we're controlling                                                                           
    void                SaveData    ( igfmgr& Igf )       const;           // controller can save it's own data
    void                LoadData    ( igfmgr& Igf )            ;           // controller can load it's own data
    controller*         CopyOf      ( void )              const;           // Helper for constructing copies

    virtual void        ExportData  ( export::fx_controllerhdr& ContHdr,
                                      xstring&                  Type,
                                      xbytestream&              Stream ) ;

};


/*

//============================================================================
// Some specialized derived controllers

//============================================================================
// Vector 3
class controller_v3 : public controller
{

public:
    controller_v3( );
    controller_v3( const controller_v3& Controller );
    // Helper for constructing copies
    virtual controller*    CopyOf   ( void ) const;

    controller_v3( const char* ParentLabel );
    
    // save/load my own data
    virtual bool    SaveData        ( text_out& Out, bool WriteHdr );
    virtual bool    LoadData        ( text_in& In );
    virtual void    ExportData      ( export::fx_controllerhdr& ContHdr, 
                                      xstring& Type,
                                      xbytestream& Stream );
};
  
//============================================================================
// Transform
class controller_xfrm : public controller
{

public:

    struct format
    {
        vector3     m_Scale;
        radian3     m_Rotation;
        vector3     m_Translation;
    };

    controller_xfrm( );
    controller_xfrm( const controller_xfrm& Controller );
    // Helper for constructing copies
    virtual controller*    CopyOf   ( void ) const;

    controller_xfrm( const char* ParentLabel );

    // save/load my own data
    virtual bool    SaveData        ( text_out& Out, bool WriteHdr );
    virtual bool    LoadData        ( text_in& In );
    virtual void    ExportData      ( export::fx_controllerhdr& ContHdr, 
                                      xstring& Type,
                                      xbytestream& Stream );

};

//============================================================================
// Color
class controller_color : public controller
{

public:

    struct format
    {
        f32 R, G, B, A;
    };

    controller_color( );
    controller_color( const controller_color& Controller );
    // Helper for constructing copies
    virtual controller*    CopyOf   ( void ) const;

    controller_color( const char* ParentLabel );

    // save/load my own data
    virtual bool    SaveData        ( text_out& Out, bool WriteHdr );
    virtual bool    LoadData        ( text_in& In );
    virtual void    ExportData      ( export::fx_controllerhdr& ContHdr, 
                                      xstring& Type,
                                      xbytestream& Stream );

};


//============================================================================
// Function for reading in and instantiating controllers
controller* Controller__LoadController( text_in& TextIn );


//============================================================================
// Function for reading in and instantiating controllers
extern CMapWordToPtr   g_HandleMap;

*/

} // namespace fx_core

#endif
