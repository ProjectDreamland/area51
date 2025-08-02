//==============================================================================
// Elf_Mdebug.hpp
//==============================================================================

//==============================================================================
// MDB_HDRR
//==============================================================================

struct MDB_HDRR {
    short   magic;          // to verify validity of the table
    short   vstamp;         // version stamp

    long    ilineMax;       // number of line number entries
    long    cbLine;         // number of bytes for line number entries
    long    cbLineOffset;   // offset to start of line number entries

    long    idnMax;         // max index into dense number table
    long    cbDnOffset;     // offset to start dense number table

    long    ipdMax;         // number of procedures
    long    cbPdOffset;     // offset to procedure descriptor table

    long    isymMax;        // number of local symbols
    long    cbSymOffset;    // offset to start of local symbols

    long    ioptMax;        // max index into optimization symbol entries
    long    cbOptOffset;    // offset to optimization symbol entries

    long    iauxMax;        // number of auxiliary symbol entries
    long    cbAuxOffset;    // offset to start of auxiliary symbol entries

    long    issMax;         // max index into local strings
    long    cbSsOffset;     // offset to start of local strings

    long    issExtMax;      // max index into external strings
    long    cbSsExtOffset;  // offset to start of external strings

    long    ifdMax;         // number of file descriptor entries
                            // ifd’s for the file run from 0 to ifdMax-1
                            // Each one is an index to a CFDR record indexed off of PCHDRR pointer
    long    cbFdOffset;     // offset to file descriptor table

    long    crfd;           // number of relative file descriptor entries
    long    cbRfdOffset;    // offset to relative file descriptor table

    long    iextMax;        // max index into external symbols
    long    cbExtOffset;    // offset to start of external symbol entries

                            // If you add machine dependent fields, add them here
};

#define cbHDRR sizeof(HDRR)
#define hdrNil ((pHDRR)0)


//==============================================================================
// MDB_FDR
//==============================================================================

struct MDB_FDR {
    unsigned long   adr;                /* memory address of beginning of file */
                                        /* adr is incorrect if there are no PDR’s for this FDR. */
    long            rss;                /* file name (of source, if known) */

    long            issBase;            /* Offset with local strings of the beginning of strings for this FDR */
    long            cbSs;               /* number of bytes in the ss */

    long            isymBase;           /* beginning of symbols */
    long            csym;               /* count file’s of symbols */

    long            ilineBase;          /* Index into virtual array of line table entries (array is size of text
                                           for this fdr). Array does not exist on disk. Array created by libmld
                                           by expanding compressed line data virtual table has a line number (integer)
                                           per instruction in text. virtual entry is 32-bit integer long cline; Size
                                           of the the virtual array indexed by ilineBase. Count of number of 32 bit
                                           words ilineBase + cline of this FDR are the ilineBase of the next FDR
                                           that has a line table (ie, has text). */

    long            ioptBase;           /* file’s optimization entries */
    long            copt;               /* count of file’s optimization entries */

    unsigned short  ipdFirst;           /* start of procedures for this file */
    unsigned short  cpd;                /* count of procedures for this file */

    long            iauxBase;           /* file’s auxiliary entries */
    long            caux;               /* count of file’s auxiliary entries */

    long            rfdBase;            /* index into the file indirect table */
    long            crfd;               /* count file indirect entries */

    unsigned        lang: 5;            /* language for this file */
                                        /* language numbers assigned "permanently" */
    unsigned        fMerge : 1;         /* whether this file can be merged */
                                        /* fmerge shows as "mergeable" or "unmergeable" for the file via st_printfd() in stdump */
    unsigned        fReadin : 1;        /* true if it was read in (not just created) */
                                        /* Readin shows as "preexisting" if readin set, else
                                           "new" via st_printfd() in stdump */
    unsigned        fBigendian : 1;     /* if set, was compiled on big endian machine */
                                        /* auxes will be in compile hosts sex */
    unsigned        glevel : 2;         /* level this file was compiled with */
    unsigned        signedchar : 1;     /* whether files was compiled with char being signed */
                                        /* the following were added very late in IRIX 5.2 to allow
                                           greater than 64K procedures */
    unsigned        ipdFirstMSBits: 4;  /* upper bits to allow ipdFirst to exceed 64K entries
                                           (These are the most significant bits of what is,
                                           after concatenating the bits, a 20 bit number) */
    unsigned        cpdMSBits: 4;       /* upper bits to allow cpd to exceed 64K entries
                                           (These are the most significant bits of what is,
                                           after concatenating the bits, a 20 bit number) */
    unsigned        pad_reserved : 13;  /* reserved for future use */
    long            cbLineOffset;       /* byte offset from header for this file ln’s */
                                        /* This is offset of the compressed line table
                                           for this fdr (within the physical
                                           compressed line table).
                                           Used by the debugger as a way to start at
                                           the beginning of information for a procedure.
                                           The cbLineOffset of the next FDR cbLineOffset can
                                           be a line table stopping rule.
                                           (thus, cbLineOffset is non-decreasing). */
    long            cbLine;             /* size of lines for this file */
                                        /* Size in bytes of the compressed line table
                                           of this fdr.
                                           cbLIneOffset + cbLine for this fdr
                                           are the cbLineOffset of the next FDR
                                           that has a line table (ie, has text). */
    long            cj_padding;
};

#define cbFDR sizeof(FDR)
#define fdNil ((pFDR)0)
#define ifdNil -1
#define ifdTemp 0
#define ilnNil -1

//==============================================================================
// MDB_SYMR
//==============================================================================

struct MDB_SYMR {
    long        iss;                /* index into String Space of name */
    long        value;              /* value of symbol */
    unsigned    st : 6;             /* symbol type */
    unsigned    sc : 5;             /* storage class - text, data, etc */
    unsigned    pad_reserved : 1;   /* reserved */
    unsigned    index : 20;         /* index into sym/aux table */

    enum type
    {
        ST_NIL          = 0,
        ST_GLOBAL       = 1,
        ST_STATIC       = 2,
        ST_PARAM        = 3,
        ST_LOCAL        = 4,
        ST_LABEL        = 5,
        ST_PROC         = 6,
        ST_BLOCK        = 7,
        ST_END          = 8,
        ST_MEMBER       = 9,
        ST_TYPEDEF      = 10,
        ST_FILE         = 11,
        ST_STATICPROC   = 14,
        ST_CONSTANT     = 15,
        ST_INDIRECT     = 33
    };

    enum storage_class
    {
        SC_NIL          = 0,
        SC_TEXT         = 1,
        SC_DATA         = 2,
        SC_BSS          = 3,
        SC_REGISTER     = 4,
        SC_ABS          = 5,
        SC_UNDEFINED    = 6,
        SC_CDB_LOCAL    = 7,
        SC_BITS         = 8,
        SC_DBX          = 9,
        SC_REG_IMAGE    = 10,
        SC_INFO         = 11,
        SC_USER_STRUCT  = 12,
        SC_SDATA        = 13,
        SC_SBSS         = 14,
        SC_RDATA        = 15,
        SC_VAR          = 16,
        SC_COMMON       = 17,
        SC_SCOMMON      = 18,
        SC_VAR_REGISTER = 19,
        SC_VARIANT      = 20,
        SC_SUNDEFINED   = 21,
        SC_INIT         = 22,
        SC_BASED_VAR    = 23,
        SC_XDATA        = 24,
        SC_PDATA        = 25,
        SC_FINI         = 26,
        SC_NONGP        = 27,
        SC_VIRTUAL      = SC_REGISTER,
        SC_PURE         = SC_VAR_REGISTER,
        SC_CLASS        = SC_PDATA,
        SC_STATIC       = SC_COMMON,
        SC_VIR_STATIC   = SC_SCOMMON,
        SC_MAX          = 32
    };
};

//==============================================================================
