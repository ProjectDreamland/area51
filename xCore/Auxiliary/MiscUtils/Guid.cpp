//==============================================================================
//  
//  guid.cpp
//
//==============================================================================

//==============================================================================
#include "x_types.hpp"
#include "x_debug.hpp"
#include "x_plus.hpp"
#include "x_stdio.hpp"
#include "Auxiliary\MiscUtils\Guid.hpp"

#ifdef X_EDITOR
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#endif // X_EDITOR

//==============================================================================
//
// This is the number of seconds since Jan 01 00:00 1970 to Jan 01 00:00 2001
// and is used as a base from the time() call.
//
//==============================================================================

#define STARTING_DATE_SECONDS (978328800) 

//==============================================================================
//
// These are the number of bits used to store each section of the guid
//
//==============================================================================

#define BITS_USERID     20 // 5 4bit characters
#define BITS_PROCESSID   6 // munged process id 
#define BITS_SECONDS    28 // 2^28 = 268,435,456 = 8.51 years from Jan 01 2001
#define BITS_SEQUENCE   10 // 2^10 = 1024 objects used within 1 second

//==============================================================================

static xbool    s_Inited = FALSE;
static u64      s_Sequence;         // Current sequence number

#ifdef X_EDITOR
static char     s_UserName[16];     // Original user name
static char     s_UserIDName[6];    // Munged user name
static s32      s_UserID;           // Numerically munged user name
static u32      s_FrameSecond;      // Second of last guid creation
static s32      s_FrameSequence;    // Sequence at beginning of second
static s32      s_ProcessID;        // OS process id for application
#endif // X_EDITOR

//==============================================================================
//==============================================================================
//==============================================================================
#ifdef X_EDITOR
//==============================================================================
//==============================================================================
//==============================================================================
static char*    s_LetterList = "bcdfghjklmnprstw"; // 16 letters

static
s32 MakeUserIDFromUserName( const char* pStr )
{
    const char* pN  = pStr;
    s32     nLetters= 0;
    s32     ID[5]={0};

    // Add letters to ID
    while( (*pN) && (nLetters<5) )
    {
        char* pL = s_LetterList;
        while( *pL )
        {
            if( (*pN) == (*pL) ) 
            {
                ID[nLetters] = (s32)(pL - s_LetterList);
                nLetters++;
                break;
            }
            pL++;
        }
        pN++;
    }

    // If less than 5 letters add number of characters
    if( nLetters < 5 )
    {
        s32 N = x_strlen(pStr);
        if( N>16 ) N = 16;
        ID[nLetters] = N;
    }

    // Build value
    s32 UserID = 0;
    for( s32 i=0; i<5; i++ )
        UserID = UserID | (ID[i]<<(4*(4-i)));

    return UserID;
}

//==============================================================================

static
void MakeUserNameFromUserID( s32 UserID, char* pStr )
{
    for( s32 i=0; i<5; i++ )
        pStr[i] = s_LetterList[ (UserID>>(4*(4-i))) & 0xF ];

    pStr[i] = 0;
}

//==============================================================================

static
u32 GetGuidSeconds( void )
{
    u32 Second;
    time((time_t*)&Second);

    // Subtract Jan 1st 2001 from Jan 1st 1970
    Second -= STARTING_DATE_SECONDS;
    return Second;
}

//==============================================================================

static
void TestUserID( void )
{
    char* UserNameList[] = 
    {
        "ahunter",      "gomelchuck",       "mfong",
        "acheng",       "jcossigny",        "mreed",
        "athyssen",     "jslate",           "phaskins",
        "bbickerton",   "jrichardson",      "pfranco",
        "bspears",      "jnagle",           "rbrown",
        "bpavlock",     "jversluis",        "rshelley",
        "bmarques",     "jfalcon",          "rbyrd",
        "bsalinas",     "lgustafson",       "starrant",
        "bwatson",      "mschaefgen",       "shird",
        "clupher",      "mcoven",           "sbroumley",
        "cgalley",      "mmcclelland",      "theimann",
        "clum",         "mtraub",           "tarce",
        "finished",
    };

    for( s32 i=0; i<100; i++ )
    {
        if( x_strcmp(UserNameList[i],"finished")==0 )
            break;

        char STR[6];
        s32 ID0 = MakeUserIDFromUserName(UserNameList[i]);
        MakeUserNameFromUserID( ID0, STR );

        for( s32 j=i+1; j<100; j++ )
        {
            if( x_strcmp(UserNameList[j],"finished")==0 )
                break;

            s32 ID1 = MakeUserIDFromUserName(UserNameList[j]);
            ASSERT( ID0 != ID1 );
        }
    }
}

//==============================================================================

void guid_Init( void )
{
    // Be sure we are initialized only once
    ASSERT( s_Inited == FALSE );
    s_Inited = TRUE;

    // Be sure we have the right number of bits
    ASSERT( (BITS_SECONDS + BITS_PROCESSID + BITS_USERID + BITS_SEQUENCE ) == 64 );

    // Get User Name String
    {
        // Try to use user login name
        if( getenv("USERNAME") )  
        {
            x_strcpy( s_UserName, getenv("USERNAME") );
        }
        else
        // Last resort - random name
        {
            s32 C = clock();
            for( s32 i=0; i<5; i++ )
            {
                s_UserName[i] = s_LetterList[C&0xF];
                C >>= 4;
            }
            s_UserName[5] = 0;
        }
    }

    // Clear statics
    s_Sequence = 0;
    s_FrameSecond = 0;
    s_FrameSequence = 0;
    s_UserID = 0;

    // Confirm all userids are unique
    TestUserID();

    // Build UserID
    s_UserID = MakeUserIDFromUserName( s_UserName );
    MakeUserNameFromUserID( s_UserID, s_UserIDName );

    // Get ID for this process
    s_ProcessID = _getpid();
    s_ProcessID = s_ProcessID ^ (s_ProcessID >> BITS_PROCESSID);
    s_ProcessID = s_ProcessID & ((1<<BITS_PROCESSID)-1);

    /*
    // Display startup info
    x_DebugMsg("***************************\n");
    x_DebugMsg("***** GUID Initialize *****\n");
    x_DebugMsg("***************************\n");
    x_DebugMsg("PID:        %1d\n",s_ProcessID);
    x_DebugMsg("USERNAME:   %s\n",s_UserName);
    x_DebugMsg("USERID:     %s\n",s_UserIDName);
    x_DebugMsg("***************************\n");
    */
}

//==============================================================================

void guid_Kill( void )
{
    ASSERT( s_Inited == TRUE );
    s_Inited = FALSE;
}

//==============================================================================

guid guid_New ( void )
{
    ASSERT( s_Inited == TRUE );

    //
    // Get current second
    //
    u32 Second = GetGuidSeconds();

    //
    // Check if we have run out of sequence
    //
    if( s_Sequence == ((1<<BITS_SEQUENCE)-1) )
    {
        // Wait until second has passed
        while( 1 )
        {
            Second = GetGuidSeconds();
            if( Second > s_FrameSecond )
                break;
        }
    }

    //
    // Check if we are entering a new frame
    //
    if( Second > s_FrameSecond )
    {
        // Reset sequence watch
        s_Sequence = 0;
        s_FrameSecond = Second;
    }

    //
    // Gather data
    //
    u64 GUIDSecond      = Second;
    u64 GUIDUserID      = s_UserID;
    u64 GUIDSequence    = s_Sequence;
    u64 GUIDProcessID   = s_ProcessID;
    u64 GUID            = 0;
   
    GUID = (GUID << BITS_USERID)    | (GUIDUserID    & ((((u64)1)<<BITS_USERID)   -1));
    GUID = (GUID << BITS_PROCESSID) | (GUIDProcessID & ((((u64)1)<<BITS_PROCESSID)-1));
    GUID = (GUID << BITS_SECONDS)   | (GUIDSecond    & ((((u64)1)<<BITS_SECONDS)  -1));
    GUID = (GUID << BITS_SEQUENCE)  | (GUIDSequence  & ((((u64)1)<<BITS_SEQUENCE) -1));

    // Increment sequence
    s_Sequence++;

    guid G;
    G.Guid = GUID;
    return G;
}

//==============================================================================
/*
void guid_GetInfo( guid GUID, guid_info& Info )
{
    // Pull all the data out of the guid
    Info.Sequence = (s32)(GUID & ((((u64)1)<<BITS_SEQUENCE) -1));
    GUID >>= BITS_SEQUENCE;
    Info.Seconds  = (s32)(GUID & ((((u64)1)<<BITS_SECONDS)  -1));
    GUID >>= BITS_SECONDS;
    Info.ProcessID = (s32)(GUID & ((((u64)1)<<BITS_PROCESSID)  -1));
    GUID >>= BITS_PROCESSID;
    Info.UserID    = (s32)(GUID & ((((u64)1)<<BITS_USERID)  -1));
    GUID >>= BITS_USERID;
    ASSERT( GUID == 0 );

    // Build strings
    MakeUserNameFromUserID( Info.UserID, Info.UserName );
    u32 Seconds = Info.Seconds + STARTING_DATE_SECONDS;
    x_strcpy( Info.Date, ctime( (time_t*)&Seconds ) );
}
*/
//==============================================================================
//==============================================================================
//==============================================================================
#endif // X_EDITOR
//==============================================================================
//==============================================================================
//==============================================================================

//==============================================================================
//==============================================================================
//==============================================================================
#ifndef X_EDITOR
//==============================================================================
//==============================================================================
//==============================================================================

void guid_Init( void )
{
    // Be sure we are initialized only once
    if( s_Inited == TRUE )
        return;
    s_Inited = TRUE;

    x_DebugMsg("***************************\n");
    x_DebugMsg("***** GUID Initialize *****\n");
    x_DebugMsg("***************************\n");

    // Start with 1 so we never create a null guid by mistake
    s_Sequence = 1;
}

//==============================================================================

void guid_Kill( void )
{
    ASSERT( s_Inited == TRUE );
    s_Inited = FALSE;
}

//==============================================================================

guid guid_New ( void )
{
    ASSERT( s_Inited == TRUE );

    guid GUID;
    GUID.Guid = s_Sequence;

    s_Sequence++;

    return GUID;
}

//==============================================================================
//==============================================================================
//==============================================================================
#endif // NOT X_EDITOR
//==============================================================================
//==============================================================================
//==============================================================================

// List of closest prime number above 1000*N (1>=N<=256)
static s32 PrimeList[] = 
{
      1009,     2003,     3001,     4001,     5003,     6007,     7001,     8009,
      9001,    10007,    11003,    12007,    13001,    14009,    15013,    16001,
     17011,    18013,    19001,    20011,    21001,    22003,    23003,    24001,
     25013,    26003,    27011,    28001,    29009,    30011,    31013,    32003,
     33013,    34019,    35023,    36007,    37003,    38011,    39019,    40009,
     41011,    42013,    43003,    44017,    45007,    46021,    47017,    48017,
     49003,    50021,    51001,    52009,    53003,    54001,    55001,    56003,
     57037,    58013,    59009,    60013,    61001,    62003,    63029,    64007,
     65003,    66029,    67003,    68023,    69001,    70001,    71011,    72019,
     73009,    74017,    75011,    76001,    77003,    78007,    79031,    80021,
     81001,    82003,    83003,    84011,    85009,    86011,    87011,    88001,
     89003,    90001,    91009,    92003,    93001,    94007,    95003,    96001,
     97001,    98009,    99013,   100003,   101009,   102001,   103001,   104003,
    105019,   106013,   107021,   108007,   109001,   110017,   111029,   112019,
    113011,   114001,   115001,   116009,   117017,   118033,   119027,   120011,
    121001,   122011,   123001,   124001,   125003,   126001,   127031,   128021,
    129001,   130003,   131009,   132001,   133013,   134033,   135007,   136013,
    137029,   138007,   139021,   140009,   141023,   142007,   143053,   144013,
    145007,   146009,   147011,   148013,   149011,   150001,   151007,   152003,
    153001,   154001,   155003,   156007,   157007,   158003,   159013,   160001,
    161009,   162007,   163003,   164011,   165001,   166013,   167009,   168013,
    169003,   170003,   171007,   172001,   173021,   174007,   175003,   176017,
    177007,   178001,   179021,   180001,   181001,   182009,   183023,   184003,
    185021,   186007,   187003,   188011,   189011,   190027,   191021,   192007,
    193003,   194003,   195023,   196003,   197003,   198013,   199021,   200003,
    201007,   202001,   203011,   204007,   205019,   206009,   207013,   208001,
    209021,   210011,   211007,   212029,   213019,   214003,   215051,   216023,
    217001,   218003,   219001,   220009,   221021,   222007,   223007,   224011,
    225023,   226001,   227011,   228013,   229003,   230003,   231001,   232003,
    233021,   234007,   235003,   236017,   237011,   238001,   239017,   240007,
    241013,   242009,   243011,   244003,   245023,   246011,   247001,   248021,
    249017,   250007,   251003,   252001,   253003,   254003,   255007,   256019,
};

//==============================================================================
//==============================================================================
//==============================================================================
// LOOKUP CLASS
//==============================================================================
//==============================================================================
//==============================================================================

guid_lookup::guid_lookup( void )
{
    m_nNodes            = 0;
    m_nNodesAllocated   = 0;
    m_pNode             = NULL;
    m_FirstFreeNode     = -1;
    m_nHashEntries      = 0;
    m_pHashEntry        = NULL;
    m_CanGrow           = TRUE;
}

//==============================================================================

guid_lookup::~guid_lookup ( void )
{
    x_free( m_pNode );
    x_free( m_pHashEntry );

    m_nNodes            = 0;
    m_nNodesAllocated   = 0;
    m_pNode             = NULL;
    m_FirstFreeNode     = -1;
    m_nHashEntries      = 0;
    m_pHashEntry        = NULL;
}
 
//==============================================================================

void guid_lookup::SetCapacity( s32 NGuids, xbool CanGrow )
{
    m_CanGrow = TRUE;
    Resize( NGuids );
    m_CanGrow = CanGrow;
}

//==============================================================================

void guid_lookup::Clear( void )
{
    x_free( m_pNode );
    x_free( m_pHashEntry );

    m_nNodes            = 0;
    m_nNodesAllocated   = 0;
    m_pNode             = NULL;
    m_FirstFreeNode     = -1;
    m_nHashEntries      = 0;
    m_pHashEntry        = NULL;
}

//==============================================================================

void guid_lookup::Resize( s32 aNGuids )
{
    s32 i;

    MEMORY_OWNER( "guid_lookup::Resize()" );

    // Is this table allowed to grow?
    ASSERT( m_CanGrow );

    // Only allow the system to grow
    if( aNGuids <= m_nNodesAllocated )
        return;

    // Decide on which multiple of a thousand to use
    //s32 NNodes = (aNGuids/1000)*1000;
    //if( aNGuids > NNodes ) NNodes += 128;
    s32 NNodes = aNGuids;

    // Decide on prime number of hash table entries
    s32 NHashEntries = ((NNodes/2)*3) / 1000;
    if( NHashEntries > 256 ) NHashEntries = 256;
    if( NHashEntries == 0 ) NHashEntries = 1;
    NHashEntries = PrimeList[NHashEntries-1];

    // Realloc nodes
    if( m_pNode==NULL )
    {
        m_pNode = (node*)x_malloc( NNodes*sizeof(node) );
        ASSERT( m_pNode  );
    }
    else
    {
        m_pNode = (node*)x_realloc( m_pNode, NNodes*sizeof(node) );
        ASSERT( m_pNode  );
    }

    // Clear new nodes and add to free list
    for( i=m_nNodesAllocated; i<NNodes; i++ )
    {
        m_pNode[i].GUID.Guid = 0;
        m_pNode[i].Data = 0;
        m_pNode[i].Next = m_FirstFreeNode;
        m_pNode[i].Prev = -1;
        if( m_FirstFreeNode != -1 )
            m_pNode[ m_FirstFreeNode ].Prev = i;
        m_FirstFreeNode = i;
    }

    // Setup new number of nodes
    m_nNodesAllocated = NNodes;

    // Free current hash table to free up memory manager
    x_free( m_pHashEntry );

    // Set new hash size and allocate new table
    m_nHashEntries = NHashEntries;
    m_pHashEntry = (s32*)x_malloc( m_nHashEntries*sizeof(s32) );

    ASSERT( m_pHashEntry );

    // Clear hash table entries
    for( i=0; i<NHashEntries; i++ )
        m_pHashEntry[i] = -1;

    // Insert current nodes into new hash table
    for( i=0; i<m_nNodesAllocated; i++ )
    {
        // Check if node is in use
        if( m_pNode[i].GUID > 0 )
        {
            // Compute hash entry index
            s32 I = GetHash(m_pNode[i].GUID);

            // Connect to linked list
            m_pNode[i].Next = m_pHashEntry[I];
            m_pNode[i].Prev = -1;
            if( m_pHashEntry[I] != -1 )
                m_pNode[ m_pHashEntry[I] ].Prev = i;
            m_pHashEntry[I] = i;
        }
    }
}

//==============================================================================

s32 guid_lookup::AllocNode( void )
{
    if( m_FirstFreeNode == -1 )
    {
        Resize( m_nNodesAllocated + 128 );
    }

    // Get index of new node
    s32 I = m_FirstFreeNode;
    ASSERT( (I>=0) && (I<m_nNodesAllocated) );

    // Adjust free list
    m_FirstFreeNode = m_pNode[I].Next;
    if( m_FirstFreeNode != -1 )
        m_pNode[ m_FirstFreeNode ].Prev = -1;

    // Init new node
    m_pNode[I].Data = 0;
    m_pNode[I].GUID.Guid = 0;
    m_pNode[I].Next = -1;
    m_pNode[I].Prev = -1;

    m_nNodes++;

    return I;
}

//==============================================================================

void guid_lookup::DeallocNode( s32 Node )
{
    // Decide hash table index
    s32 I = GetHash(m_pNode[Node].GUID);

    // Remove from list
    if( m_pNode[Node].Prev != -1 )
        m_pNode[ m_pNode[Node].Prev ].Next = m_pNode[Node].Next;
    if( m_pNode[Node].Next != -1 )
        m_pNode[ m_pNode[Node].Next ].Prev = m_pNode[Node].Prev;

    // Remove from head
    if( m_pHashEntry[I] == Node )
        m_pHashEntry[I] = m_pNode[Node].Next;

    // Add into free list
    m_pNode[Node].GUID.Guid = 0;
    m_pNode[Node].Data = 0;
    m_pNode[Node].Next = m_FirstFreeNode;
    m_pNode[Node].Prev = -1;
    if( m_FirstFreeNode != -1 )
        m_pNode[ m_FirstFreeNode ].Prev = Node;
    m_FirstFreeNode = Node;

    m_nNodes--;
}

//==============================================================================

void guid_lookup::Add( guid GUID, u32   Data )
{
    s32 I = AllocNode();
    ASSERT( I != -1 );
    m_pNode[I].Data = Data;
    m_pNode[I].GUID = GUID;

    // Compute hash entry index
    s32 HI = GetHash(m_pNode[I].GUID);

    // Connect to linked list
    m_pNode[I].Next = m_pHashEntry[HI];
    m_pNode[I].Prev = -1;
    if( m_pHashEntry[HI] != -1 )
        m_pNode[ m_pHashEntry[HI] ].Prev = I;
    m_pHashEntry[HI] = I;
}

//==============================================================================

void guid_lookup::Add( guid GUID, s32   Data )
{
    Add( GUID, (u32)Data );
}

//==============================================================================

void guid_lookup::Add( guid GUID, void* Data )
{
    Add( GUID, (u32)Data );
}

//==============================================================================

xbool guid_lookup::Find( guid GUID )
{
    s32 I = GetIndex( GUID );
    return (I != -1);
}

//==============================================================================

xbool guid_lookup::Find( guid GUID, u32&       Data )
{
    s32 I = GetIndex( GUID );
    if( I != -1 )
    {
        Data = m_pNode[I].Data;
        return TRUE;
    }

    return FALSE;
}

//==============================================================================

xbool guid_lookup::Find( guid GUID, s32&       Data )
{
    s32 I = GetIndex( GUID );
    if( I != -1 )
    {
        Data = (s32)m_pNode[I].Data;
        return TRUE;
    }

    return FALSE;
}

//==============================================================================

xbool guid_lookup::Find( guid GUID, void*&   Data )
{
    s32 I = GetIndex( GUID );
    if( I != -1 )
    {
        Data = (void*)m_pNode[I].Data;
        return TRUE;
    }

    return FALSE;
}

//==============================================================================

u32& guid_lookup::GetU32( s32 I )
{
    ASSERT( I >= 0 );

    return m_pNode[I].Data;
}

//==============================================================================

s32& guid_lookup::GetS32( s32 I )
{
    ASSERT( I >= 0 );

    return *((s32*)&m_pNode[I].Data);
}

//==============================================================================

void*& guid_lookup::GetP32( s32 I )
{
    ASSERT( I >= 0 );
    return *((void**)&m_pNode[I].Data);
}

//==============================================================================

void guid_lookup::Del( guid GUID )
{
    s32 I = GetIndex( GUID );
    ASSERT( I != -1 );

    DeallocNode( I );
}

//==============================================================================

s32 guid_lookup::GetNBytes( void )
{
    s32 gl = sizeof( guid_lookup );
    s32 n = sizeof( node );
    s32 s = sizeof( s32 );
    return  gl + (n * m_nNodesAllocated) + (s*m_nHashEntries);
}

//==============================================================================

void guid_lookup::Dump( const char* pFileName )
{
    s32 i;
    X_FILE* fp = x_fopen(pFileName,"wt");
    ASSERT(fp);

    x_fprintf(fp,"GUID Lookup Table Dump:\n");
    x_fprintf(fp,"Total Bytes Used:   %1d\n",GetNBytes());
    x_fprintf(fp,"Total GUIDS Stored: %1d\n",m_nNodes);
    x_fprintf(fp,"Total Hash Entries: %1d\n",m_nHashEntries);
    x_fprintf(fp,"\n");

    s32 LargestNGuids=0;
    for( i=0; i<m_nHashEntries; i++ )
    {
        x_fprintf(fp,"%5d] ",i);

        // Compute first node in hash table
        s32 I = m_pHashEntry[i];

        // Check linked list for match
        s32 NGuids = 0;
        while( I != -1 )
        {
            ASSERT( (I>=0) && (I<m_nNodesAllocated) );
            NGuids++;
            I = m_pNode[I].Next;
        }
        x_fprintf(fp,"%3d ",NGuids);
        LargestNGuids = MAX(LargestNGuids,NGuids);

        if( 1 )
        {
            I = m_pHashEntry[i];
            while( I != -1 )
            {
                ASSERT( (I>=0) && (I<m_nNodesAllocated) );
                x_fprintf(fp,"%08X%08X:%08X   ",
                    (u32)((m_pNode[I].GUID>>32)&0xFFFFFFFF),
                    (u32)((m_pNode[I].GUID>>0)&0xFFFFFFFF),
                    (u32)(m_pNode[I].Data));
                I = m_pNode[I].Next;
            }
        }
        x_fprintf(fp,"\n");
    }

    x_fprintf(fp,"\nLargest Number of Guids In Hash Entry: %1d\n",LargestNGuids);

    x_fclose(fp);
}

//==============================================================================

void guid_lookup::SanityCheck( void )
{
    ASSERT((m_CanGrow==TRUE) || (m_CanGrow==FALSE));
    ASSERT((m_pHashEntry==NULL) || ((u32)m_pHashEntry>16384));
}

//==============================================================================

xstring guid_ToString( guid GUID )
{
	xstring Result;
	Result.Format("%08X:%08X",
                    (u32)((GUID>>32)&0xFFFFFFFF),
                    (u32)((GUID>>0)&0xFFFFFFFF));
	return Result;
}

//==============================================================================

guid guid_FromString( const char* pGUID )
{
    guid GUID;
    GUID.Guid=0;

    while( *pGUID )
    {
        char c = *pGUID;
        pGUID++;

        if( (c == ':') || x_isspace(c) )
            continue;

        u32 v = 0;
        c = x_toupper(c);
        if( c>'9' ) v = (c-'A')+10;
        else        v = (c-'0');

        GUID.Guid <<= 4;
        GUID.Guid |= (v&0xF);
    }

    return GUID;
}

//==============================================================================

//==============================================================================
void guid_StoreSequence( bitstream& BitStream )
{
    ASSERT( s_Inited == TRUE );
    BitStream.WriteU64( s_Sequence );    
}

//==============================================================================

void guid_RestoreSequence( bitstream& BitStream )
{
    ASSERT( s_Inited == TRUE );
    BitStream.ReadU64( s_Sequence );    
}

//==============================================================================

void guid_DumpSequence( bitstream& BitStream )
{
    u64 Sequence;
    BitStream.ReadU64( Sequence );
    x_DebugMsg( 0, "Guid sequence = %d\n", Sequence );
}

//==============================================================================

u64 guid_GetSequence( void )
{
    return( s_Sequence );
}

