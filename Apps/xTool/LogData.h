// LogData.h
/////////////////////////////////////////////////////////////////////////////

#if !defined(LOGDATA_H)
#define LOGDATA_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "implementation/x_tool_private.hpp"
#include "x_array.hpp"
#include "SelectionSet.h"

/////////////////////////////////////////////////////////////////////////////

class CxToolDoc;

/////////////////////////////////////////////////////////////////////////////

class log_packet
{
protected:
    CxToolDoc*  m_pDocument;
    char*       m_pStart;
    char*       m_pData;
    s32         m_Size;
    s32         m_nBytes;
    xbool       m_BigEndian;

public:
    log_packet  ( CxToolDoc* pDoc, char* pData, s32 nBytes, xbool BigEndian );
    ~log_packet  ( void );

    BOOL        IsEmpty     ( void )    { return m_nBytes == 0; }
    void        Align       ( void );
    CxToolDoc*  GetDocument ( void )    { return m_pDocument; }

    friend log_packet& operator >> ( log_packet& Packet, f32&   v );
    friend log_packet& operator >> ( log_packet& Packet, s32&   v );
    friend log_packet& operator >> ( log_packet& Packet, u32&   v );
    friend log_packet& operator >> ( log_packet& Packet, s8&    v );
    friend log_packet& operator >> ( log_packet& Packet, u8&    v );
    friend log_packet& operator >> ( log_packet& Packet, xtick& v );
    friend log_packet& operator >> ( log_packet& Packet, const char*& pString );
};

/////////////////////////////////////////////////////////////////////////////

class log_entry
{
public:
    enum type
    {
        type_entry,
        type_message,
        type_memory
    };

    enum field
    {
        field_Entry,
        field_Time,
        field_Thread,
        field_Channel,
        field_Message,
        field_Line,
        field_File
    };

    enum flag
    {
        flag_selected       = (1<<0),           // log entry is selected
        flag_marked         = (1<<1),           // log entry has been marked by the user
        flag_system_error   = (1<<2),           // log entry has a system error
        flag_memory_active  = (1<<3),           // memory log entry is still an active allocation
    };

protected:
    double      m_Ticks;
    CxToolDoc*  m_pDoc;
    s32         m_Type;
    u32         m_Sequence;
    u32         m_ThreadID;
    s32         m_ChannelID;
    u32         m_Flags;

public:
                                    log_entry       ( void );
    virtual                        ~log_entry       ( void );

    xtool::log_type                 GetType         ( void ) const { return (xtool::log_type)m_Type; }
    double                          GetTicks        ( void ) const { return m_Ticks; }
    u32                             GetThreadID     ( void ) const { return m_ThreadID; }
    u32                             GetSequence     ( void ) const { return m_Sequence; }
    void                            SetFlags        ( u32 Flags, u32 Mask ) { m_Flags = (m_Flags & ~Mask) | (Flags & Mask); }
    u32                             GetFlags        ( u32 Mask = ~0 ) { return m_Flags & Mask; }
    virtual xtool::log_severity     GetSeverity     ( void ) const;
    virtual const char*             GetChannel      ( void ) const;
    virtual int                     GetChannelID    ( void ) const;
    virtual const char*             GetMessage      ( void ) const;
    virtual const char*             GetFile         ( void ) const;
    virtual int                     GetLine         ( void ) const;

    virtual void                    Serialize       ( CArchive& ar );
};

/////////////////////////////////////////////////////////////////////////////

class log_message : public log_entry
{
protected:
    s32             m_Severity;
    CString         m_Message;
    CString         m_File;
    s32             m_Line;

public:
                                    log_message     ( void );
    virtual                        ~log_message     ( void );

    void                            Init            ( CxToolDoc*            pDoc,
                                                      u32                   Sequence,
                                                      double                Ticks,
                                                      u32                   ThreadID,
                                                      s32                   ChannelID,
                                                      xtool::log_severity   Severity,
                                                      const char*           pMessage,
                                                      const char*           pFile,
                                                      s32                   Line );

    virtual xtool::log_severity     GetSeverity     ( void ) const;
    virtual const char*             GetChannel      ( void ) const;
    virtual const char*             GetMessage      ( void ) const;
    virtual const char*             GetFile         ( void ) const;
    virtual int                     GetLine         ( void ) const;

    virtual void                    Serialize       ( CArchive& ar );
};

/////////////////////////////////////////////////////////////////////////////

class log_memory : public log_entry
{
protected:
    s32             m_Operation;    // See xtool::log_mem_operation enumeration
    u32             m_Address;
    u32             m_OldAddress;
    u32             m_Size;
    u32             m_CurrentBytes;
    CString         m_File;
    s32             m_Line;
    s32             m_Mark;
    s32             m_CallStackIndex;

public:
                                    log_memory      ( void );
    virtual                        ~log_memory      ( void );

    void                            Init            ( CxToolDoc*        pDoc,
                                                      u32               Sequence,
                                                      double            Ticks,
                                                      u32               ThreadID,
                                                      s32               ChannelID,
                                                      s32               Operation,
                                                      u32               Address,
                                                      u32               OldAddress,
                                                      u32               Size,
                                                      const char*       pFile,
                                                      s32               Line,
                                                      s32               Mark,
                                                      s32               CallStackIndex );

    virtual xtool::log_severity     GetSeverity         ( void ) const;
    virtual const char*             GetChannel          ( void ) const;
    virtual const char*             GetMessage          ( void ) const;
    virtual const char*             GetFile             ( void ) const;
    virtual int                     GetLine             ( void ) const;
    s32                             GetOperation        ( void ) const { return m_Operation; }
    u32                             GetAddress          ( void ) const { return m_Address; }
    u32                             GetOldAddress       ( void ) const { return m_OldAddress; }
    void                            SetSize             ( u32 Size ) { m_Size = Size; }
    u32                             GetSize             ( void ) const { return m_Size; }
    s32                             GetMark             ( void ) const { return m_Mark; }
    s32                             GetCallStackIndex   ( void ) const  { return m_CallStackIndex; }

    void                            SetCurrentBytes     ( u32 CurrentBytes ) { m_CurrentBytes = CurrentBytes; }
    u32                             GetCurrentBytes     ( void ) const { return m_CurrentBytes; }

    virtual void                    Serialize           ( CArchive& ar );
};

/////////////////////////////////////////////////////////////////////////////

class log_channel
{
public:
    enum field
    {
        field_Name,
        field_Thread,
    };

    enum flag
    {
        flag_selected       = (1<<0),           // channel entry is selected
        flag_marked         = (1<<1),           // channel entry has been marked by the user
        flag_checked        = (1<<2)            // channel entry is enabled
    };

public:
    CxToolDoc*  m_pDoc;
    CString     m_Name;
    u32         m_ThreadID;
    u32         m_Flags;

public:
                            log_channel     ( void );
                           ~log_channel     ( void );

    void                    Init            ( CxToolDoc* pDoc, const char* pName, u32 ThreadID );
    const char*             GetName         ( void ) const;
    u32                     GetThreadID     ( void ) const;
    void                    SetFlags        ( u32 Flags, u32 Mask ) { m_Flags = (m_Flags & ~Mask) | (Flags & Mask); }
    u32                     GetFlags        ( u32 Mask = ~0 ) { return m_Flags & Mask; }

    virtual void            Serialize       ( CArchive& ar );
};

/////////////////////////////////////////////////////////////////////////////

class channel_array
{
    friend CxToolDoc;

protected:
    xarray<log_channel*>    m_Array;

public:
                    channel_array   ( void );
                   ~channel_array   ( void );

    int             GetSize         ( void );
    int             Add             ( log_channel* pChannel );
    log_channel**   GetData         ( void );
    int             Find            ( log_channel* pChannel );
    log_channel*    operator[]      ( int Index );

    void            GetSelectionSet ( CSelectionSet& SelectionSet );

    virtual void    Serialize       ( CArchive& ar, CxToolDoc* pDoc );
};

/////////////////////////////////////////////////////////////////////////////

class log_array
{
    friend CxToolDoc;

protected:
    xarray<log_entry*>  m_Array;

public:
                    log_array       ( void );
                   ~log_array       ( void );

    void            Clear           ( void );
    int             GetSize         ( void );
    int             Add             ( log_entry* pLog );
    log_entry**     GetData         ( void );
    int             Find            ( log_entry* pLog );
    log_entry*      operator[]      ( int Index );

    void            GetSelectionSet ( CSelectionSet& SelectionSet );

    virtual void    Serialize       ( CArchive& ar, CxToolDoc* pDoc );
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(LOGDATA_H)
