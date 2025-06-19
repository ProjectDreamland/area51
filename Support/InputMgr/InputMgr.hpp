#ifndef INPUT_MGR_HPP
#define INPUT_MGR_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Auxiliary\MiscUtils\Property.hpp"
#include "Entropy.hpp"

//=========================================================================
// DEFINES
//=========================================================================
// TO_ADD_CONTEXT - Search for this for where to put add context code.
#define INGAME_CONTEXT      (1<<1)
#define INVENTORY_CONTEXT   (1<<2)
#define PAUSE_CONTEXT       (1<<3)
#define FRONTEND_CONTEXT    (1<<4)
#define LADDER_CONTEXT      (1<<5)

#define MAX_CONTEXT         5

#ifdef TARGET_PC
    #define INPUT_PLATFORM_PS2  0
    #define INPUT_PLATFORM_XBOX 1
    #define INPUT_PLATFORM_PC   2
    #define MAX_INPUT_PLATFORMS 3
#else
    #define INPUT_PLATFORM_PS2  0
    #define INPUT_PLATFORM_XBOX 0
    #define INPUT_PLATFORM_PC   0
    #define MAX_INPUT_PLATFORMS 1
#endif

//=========================================================================
// CLASSES 
//=========================================================================

//=========================================================================

class input_pad : public prop_interface
{
public:     
    
    struct logical
    {
#ifdef TARGET_PC
        char            ActionName[48];         // Action Name
#endif
        f32             IsValue;                // Tells what value the current input gadget has
        f32             WasValue;               // Tells what value the gadget in a debounce state
        f32             MapsIsValue;
        f32             MapsWasValue;
        f32             TimePressed;            // How long this gadget has been pressed (used for tap and hold)
    };

protected:
    struct map
    {
#ifdef TARGET_PC
        u32             MapContext;             // Which maps context does this mapping belongs to
#endif
        u32             bButton : 1;            // Use to indicate wether is a stick or a button
        u32             bIsTap : 1;             // Is the map for a button tap?
        u32             bIsHold : 1;            // Is the map for a button hold?
        input_gadget    GadgetID;               // Use to map to the physical input gadget
        f32             Scale;                  // Use for sensibility and revert axis
        s32             iLogicalMapping;        // This tells which logical slot does this value goes into
    };

public:                        
                        input_pad       ( void );                        
    virtual void        OnEnumProp      ( prop_enum&  List );
    virtual xbool       OnProperty      ( prop_query& I    );
    logical*            GetLogical      ( void )            { return m_Logical; }
    map*                GetMap          ( s32 iPlatform )   { return m_Map[iPlatform]; }
    s32                 GetNMaps        ( s32 iPlatform )   { return m_nMaps[iPlatform]; }
    void                SetAllLogical   ( logical*  Logical );
    void                SetAllMap       ( s32       iPlatform,
                                          map*      Map,
                                          s32       nMaps );

    logical&            GetLogical      ( s32 I ) { ASSERT( I>=0 && I<MAX_LOGICAL); return m_Logical[I]; }

    void                SetLogical      ( s32 ID, const char* pName );
    void                SetLogical      ( s32 iPlatform, input_gadget GadgetID );
    void                AddMapping      ( s32 iPlatform, s32 ID, input_gadget GadgetID, xbool IsButton, f32 Scale = 1 );
    void                GetMapping      ( s32 iPlatform, s32 ID, input_gadget& GadgetID, xbool& IsButton, f32& Scale );
    void                DelMapping        ( s32 iPlatform, s32 iMapping );
    
    void                EnableContext   ( u32 Context );
    void                DisableContext  ( u32 Context );

    void                SetControllerID ( s32 ControllerID ){ m_ControllerID = ControllerID; }
    s32                 GetControllerID ( void ) const      { return m_ControllerID; }
    
    input_gadget        GetGadgetIDFromName ( s32 iPlatform, const char* pGadgetName );
    const char*         GetNameFromGadgetID ( s32 iPlatform, input_gadget GadgetID );
    const char*         GetGadgetIDNames    ( s32 iPlatform );

    void                ClearAllLogical ( void );

protected:

    enum
    {
        MAX_LOGICAL  = 52,
        MAX_MAPPINGS = 64,
    };

protected:

    void                ClearMapping    ( void );

    virtual void        OnUpdate        ( s32 iPlatform, f32 DeltaTime );
    virtual void        OnInitialize    ( void );

#ifndef X_RETAIL
    virtual void        OnDebugRender   ( void );
#endif // X_RETAIL

protected:

    logical             m_Logical[MAX_LOGICAL];                     // Logical data
    map                 m_Map[MAX_INPUT_PLATFORMS][MAX_MAPPINGS];   // Logical To Physical Mapping
    s32                 m_nMaps[MAX_INPUT_PLATFORMS];               // Number of physical map input gagets
    input_pad*          m_pNext;                                    // List of register input controlers
    u32                 m_ActiveContext;                            // All the contexts that are currently Active.
    s32                 m_ControllerID;                             // Which controller are reading the input from.
    char*               m_pName;    
    
protected:

    friend class input_mgr;
};

//=========================================================================

class input_mgr
{
public:
    
                        input_mgr       ( void );
    xbool               Update          ( f32 DeltaTime );
    void                RegisterPad     ( input_pad& Pad );
    s32                 WasPausePressed ( void );

protected:

    static input_pad* s_pHead;    

};

//=========================================================================
// GLOBALS 
//=========================================================================
extern input_mgr g_InputMgr;

//=========================================================================
// END
//=========================================================================
#endif
