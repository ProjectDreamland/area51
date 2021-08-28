/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 $Id: sq.h,v 1.4 2003/09/12 05:23:39 tokiwa Exp $
 */
/*
 * Copyright (C) 2003 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * format/sound/sq.h - .sq file format
 *
 *	See "sformat.pdf" or converted HTML file for more details.
 */

#ifndef _SCE_FORMAT_SOUND_SQ_H
#define _SCE_FORMAT_SOUND_SQ_H

/*
 * typedef char			S8;	  8bit signed
 * typedef unsigned char	U8;	  8bit unsigned
 * typedef short		S16;	 16bit signed
 * typedef unsigned short	U16;	 16bit unsigned
 * typedef int			S32;	 32bit signed
 * typedef unsigned int		U32;	 32bit unsigned
 * typedef long			S64;	 64bit signed
 * typedef unsigned long	U64;	 64bit unsigned
 *
 *  File offset (the number of bytes from the top of file)
 * typedef U32 FOFST;
 * #define NO_FOFST ((FOFST)-1)
 *  Chunk offset (the number of bytes from the top of chunk)
 * typedef U32 COFST;
 * #define NO_COFST ((COFST)-1)
 *  Offset in the chunk (the number of bytes from the top of chunk)
 * typedef U32 SOFST;
 * #define NO_SOFST ((SOFST)-1)
 *  Position offset (the number of bytes from the current point in the chunk)
 * typedef U32 POFST;
 * #define NO_POFST ((POFST)-1)
 *  Chunk index (index number to the structure in the chunk)
 * typedef U16 CINDEX;
 * #define NO_CINDEX ((CINDEX)-1)
 */

/*
 * Used in the structure's member below:
 *	sceSeqMidiCompBlock.compOption
 */
#define SCESQ_COMP_KEYON2POLY	0x0001

/*
 * Used in the structure's member below:
 *	sceSeqMidiDataBlock.sequenceDataOffset
 */
#define SCESQ_NOCOMPBLOCK	6

/****************************************************************
 * Song Chunk
 ****************************************************************/
typedef struct {
    unsigned int Creator;	/* Creator Code: 'SCEI'	*/
    unsigned int Type;		/* Chunk Type:   'Song'	*/
    unsigned int chunkSize;	/* Chunk Size		*/

    unsigned int maxSongNumber;		/* max. number of Song number	*/
    unsigned int songOffsetAddr[0];	/* offset to Song table	(COFST)	*/
} sceSeqSongChunk;

/****************************************************************
 * Midi Chunk
 ****************************************************************/
typedef struct	{
    unsigned short compOption;		/* compression mode		*/
    unsigned short compTableSize;	/* compression table size	*/
    unsigned char  compTable[0];	/* compression table		*/
} sceSeqMidiCompBlock;

typedef struct	{
    unsigned int   sequenceDataOffset;	/* offset to sequence data	(POFST)	*/
    unsigned short Division;		/* resolution of a quarter note		*/
    sceSeqMidiCompBlock	compBlock[0];	/* compression data block:
					   [NOTICE: sceSeqMidiDataBlock may have
					    no compression data block] */
} sceSeqMidiDataBlock;

typedef struct	{
    unsigned int Creator;	/* Creator Code: `SCEI'	*/
    unsigned int Type;		/* Chunk Type:   `Midi'	*/
    unsigned int chunkSize;	/* Chunk Size		*/

    unsigned int maxMidiNumber;		/* max. number of Midi sequence number	*/
    unsigned int midiOffsetAddr[0];	/* offset to Midi table		(COFST)	*/
} sceSeqMidiChunk;

/****************************************************************
 * SE Sequence Chunk
 ****************************************************************/
/*
 * Contents of SE Sequence Chunk:
 *   sceSeqSeSequenceChunk
 *   sceSeqSeSequenceBlock
 *   sceSeqSeSequenceDataBlock
 */

typedef struct {
    unsigned int   Creator;	/* Creator Code: `SCEI'	*/
    unsigned int   Type;	/* Chunk Type:   `Sesq'	*/
    unsigned int   chunkSize;	/* Chunk Size		*/

    unsigned int   maxSeSequenceSetNumber;	/* max. no. of SE sequence set no.			*/
    unsigned int   tableOffset;			/* offset to SE sequence set offset table	(POFST)	*/
    unsigned char  seSequenceMasterVolume;	/* master volume					*/
    char           seSequenceMasterPanpot;	/* master panpot (0-64-127=L-C-R; negative says reverse phase) */
    unsigned short seSequenceMasterTimeScale;	/* master time scale (x1=1000, slowest=1 - fastest=65535) */
    unsigned int   dmy0;			/* reserved						*/
    unsigned int   dmy1;			/* reserved						*/
    unsigned int   seSequenceSetOffsetAddr[0];	/* offset list to SE sequence set		(COFST)	*/
} sceSeqSeSequenceChunk;

typedef struct {
    unsigned int   maxSeSequenceNumber;		/* max. no. of SE sequence no. in SE sequence set	*/
    unsigned int   tableOffset;			/* offset to SE sequence offset table		(POFST)	*/
    unsigned char  seSequenceSetVolume;		/* volume						*/
    char           seSequenceSetPanpot;		/* panpot (0-64-127=L-C-R; negative says reverse phase) */
    unsigned short seSequenceSetTimeScale;	/* time scale (x1=1000; slowest=1 - fastest=65535	*/
    unsigned int   dmy;				/* reserved						*/
    unsigned int   seSequenceOffsetAddr[0];	/* offset list to SE sequence			(COFST) */
} sceSeqSeSequenceBlock;

typedef struct {
    unsigned int   seSequenceDataOffset;	/* offset to SE sequence Data			(POFST)	*/
    unsigned char  seSequenceVolume;		/* volume						*/
    char           seSequencePanpot;		/* panpot (0-64-127=L-C-R; negative says reverse phase) */
    unsigned short seSequenceTimeScale;		/* time scale (x1=1000; slowest=1 - fastest=65535	*/
    unsigned int   seSequenceDataSize;		/* data size						*/
    unsigned int   dmy;				/* reserved 						*/
    /* SE Sequence starts from here: SE sequence Command, Delta Time, ... */
} sceSeqSeSequenceDataBlock;

/****************************************************************
 * Header Chunk
 ****************************************************************/
typedef struct	{
    unsigned int   Creator;	/* Creator Code: `SCEI'	*/
    unsigned int   Type;	/* Chunk Type:   `Sequ'	*/
    unsigned int   chunkSize;	/* Chunk Size		*/

    unsigned int   fileSize;		/* file size					*/
    unsigned int   songChunkAddr;	/* offset address: Song Chunk		(FOFST)	*/
    unsigned int   midiChunkAddr;	/* offset address: Midi Chunk		(FOFST)	*/
    unsigned int   seSequenceChunkAddr;	/* offset address: SE Sequence Chunk	(FOFST)	*/
    unsigned int   seSongChunkAddr;	/* offset address: SE Song Chunk	(FOFST)	*/
} sceSeqHeaderChunk;
		
/****************************************************************
 * Version Chunk
 ****************************************************************/
typedef struct {
    unsigned int   Creator;	/* Creator Code: `SCEI'	*/
    unsigned int   Type;	/* Chunk Type:   `Vers'	*/
    unsigned int   chunkSize;	/* Chunk Size		*/

    unsigned short reserved;		/* reserved		*/
    unsigned char  versionMajor;	/* major version	*/
    unsigned char  versionMinor;	/* minor version	*/
} sceSeqVersionChunk;

#endif /* _SCE_FORMAT_SOUND_SQ_H */
