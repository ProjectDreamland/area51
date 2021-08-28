/*
 *	Xing VBR tagging for LAME.
 *
 *	Copyright (c) 1999 A.L. Faber
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* $Id: VbrTag.c,v 1.58 2002/10/08 19:30:00 bouvigne Exp $ */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "machine.h"
#if defined(__riscos__) && defined(FPA10)
#include	"ymath.h"
#else 
#include	<math.h>
#endif


#include "bitstream.h"
#include "lame.h"
#include "VbrTag.h"
#include "version.h"

#include	<assert.h>
#include 	<stdlib.h>
#include 	<string.h>

#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif

#ifdef __sun__
/* woraround for SunOS 4.x, it has SEEK_* defined here */
#include <unistd.h>
#endif


#ifdef _DEBUG
/*  #define DEBUG_VBRTAG */
#endif

/*
//    4 bytes for Header Tag
//    4 bytes for Header Flags
//  100 bytes for entry (NUMTOCENTRIES)
//    4 bytes for FRAME SIZE
//    4 bytes for STREAM_SIZE
//    4 bytes for VBR SCALE. a VBR quality indicator: 0=best 100=worst
//   20 bytes for LAME tag.  for example, "LAME3.12 (beta 6)"
// ___________
//  140 bytes
*/
#define VBRHEADERSIZE (NUMTOCENTRIES+4+4+4+4+4)

#define LAMEHEADERSIZE (VBRHEADERSIZE + 9 + 1 + 1 + 8 + 1 + 1 + 3 + 1 + 1 + 2 + 4 + 2 + 2)

/* the size of the Xing header (MPEG1 and MPEG2) in kbps */
#define XING_BITRATE1 128
#define XING_BITRATE2  64
#define XING_BITRATE25 32



const static char	VBRTag[]={"Xing"};
const static char	VBRTag2[]={"Info"};




/* Lookup table for fast CRC computation
 * See 'CRC_update_lookup'
 * Uses the polynomial x^16+x^15+x^2+1 */

unsigned int crc16_lookup[256] =
{
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40, 
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40, 
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41, 
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441, 
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41, 
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41, 
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640, 
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240, 
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441, 
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840, 
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640, 
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041, 
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241, 
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40, 
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};





/***********************************************************************
 *  Robert Hegemann 2001-01-17
 ***********************************************************************/

static void addVbr(VBR_seek_info_t * v, int bitrate)
{
    int i;

    v->sum += bitrate;
    v->seen ++;
    
    if (v->seen < v->want) {
        return;
    }

    if (v->pos < v->size) {
        v->bag[v->pos] = v->sum;
        v->pos ++;
        v->seen = 0;
    }
    if (v->pos == v->size) {
        for (i = 1; i < v->size; i += 2) {
            v->bag[i/2] = v->bag[i]; 
        }
        v->want *= 2;
        v->pos  /= 2;
    }
}

static void Xing_seek_table(VBR_seek_info_t * v, unsigned char *t)
{
    int i, index;
    int seek_point;
    
    if (v->pos <= 0)
        return;
        
    for (i = 1; i < NUMTOCENTRIES; ++i) {
        float j = i/(float)NUMTOCENTRIES, act, sum;
        index = (int)(floor(j * v->pos));
        if (index > v->pos-1)
            index = v->pos-1;
        act = v->bag[index];
        sum = v->sum;
        seek_point = (int)(256. * act / sum);
        if (seek_point > 255)
            seek_point = 255;
        t[i] = seek_point;
    }
}

#if 0
void print_seeking(unsigned char *t)
{
    int i;
    
    printf("seeking table ");
    for (i = 0; i < NUMTOCENTRIES; ++i) {
        printf(" %d ", t[i]);
    }
    printf("\n");
}
#endif



/****************************************************************************
 * AddVbrFrame: Add VBR entry, used to fill the VBR the TOC entries
 * Paramters:
 *	nStreamPos: how many bytes did we write to the bitstream so far
 *				(in Bytes NOT Bits)
 ****************************************************************************
*/
void AddVbrFrame(lame_global_flags *gfp)
{
    lame_internal_flags *gfc = gfp->internal_flags;

    int kbps = bitrate_table[gfp->version][gfc->bitrate_index];
    
    if (gfc->VBR_seek_table.bag == NULL) {
        gfc->VBR_seek_table.sum  = 0;
        gfc->VBR_seek_table.seen = 0;
        gfc->VBR_seek_table.want = 1;
        gfc->VBR_seek_table.pos  = 0;
        gfc->VBR_seek_table.bag  = malloc (400*sizeof(int));
        if (gfc->VBR_seek_table.bag != NULL) {
            gfc->VBR_seek_table.size = 400;
        }
        else {
            gfc->VBR_seek_table.size = 0;
            ERRORF (gfc,"Error: can't allocate VbrFrames buffer\n");
            return;
        }   
    }
    addVbr(&gfc->VBR_seek_table, kbps);
    gfp->nVbrNumFrames++;
}


/*-------------------------------------------------------------*/
static int ExtractI4(unsigned char *buf)
{
	int x;
	/* big endian extract */
	x = buf[0];
	x <<= 8;
	x |= buf[1];
	x <<= 8;
	x |= buf[2];
	x <<= 8;
	x |= buf[3];
	return x;
}

static void CreateI4(unsigned char *buf, int nValue)
{
        /* big endian create */
	buf[0]=(nValue>>24)&0xff;
	buf[1]=(nValue>>16)&0xff;
	buf[2]=(nValue>> 8)&0xff;
	buf[3]=(nValue    )&0xff;
}



static void CreateI2(unsigned char *buf, int nValue)
{
        /* big endian create */
	buf[0]=(nValue>> 8)&0xff;
	buf[1]=(nValue    )&0xff;
}


/*-------------------------------------------------------------*/
/* Same as GetVbrTag below, but only checks for the Xing tag.
   requires buf to contain only 40 bytes */
/*-------------------------------------------------------------*/
int CheckVbrTag(unsigned char *buf)
{
	int			h_id, h_mode, h_sr_index;

	/* get selected MPEG header data */
	h_id       = (buf[1] >> 3) & 1;
	h_sr_index = (buf[2] >> 2) & 3;
	h_mode     = (buf[3] >> 6) & 3;

	/*  determine offset of header */
	if( h_id )
	{
                /* mpeg1 */
		if( h_mode != 3 )	buf+=(32+4);
		else				buf+=(17+4);
	}
	else
	{
                /* mpeg2 */
		if( h_mode != 3 ) buf+=(17+4);
		else              buf+=(9+4);
	}

	if( buf[0] != VBRTag[0] && buf[0] != VBRTag2[0] ) return 0;    /* fail */
	if( buf[1] != VBRTag[1] && buf[1] != VBRTag2[1]) return 0;    /* header not found*/
	if( buf[2] != VBRTag[2] && buf[2] != VBRTag2[2]) return 0;
	if( buf[3] != VBRTag[3] && buf[3] != VBRTag2[3]) return 0;
	return 1;
}

int GetVbrTag(VBRTAGDATA *pTagData,  unsigned char *buf)
{
	int			i, head_flags;
	int			h_bitrate,h_id, h_mode, h_sr_index;
        int enc_delay,enc_padding; 

	/* get Vbr header data */
	pTagData->flags = 0;

	/* get selected MPEG header data */
	h_id       = (buf[1] >> 3) & 1;
	h_sr_index = (buf[2] >> 2) & 3;
	h_mode     = (buf[3] >> 6) & 3;
        h_bitrate  = ((buf[2]>>4)&0xf);
	h_bitrate = bitrate_table[h_id][h_bitrate];

        /* check for FFE syncword */
        if ((buf[1]>>4)==0xE) 
            pTagData->samprate = samplerate_table[2][h_sr_index];
        else
            pTagData->samprate = samplerate_table[h_id][h_sr_index];
        //	if( h_id == 0 )
        //		pTagData->samprate >>= 1;



	/*  determine offset of header */
	if( h_id )
	{
                /* mpeg1 */
		if( h_mode != 3 )	buf+=(32+4);
		else				buf+=(17+4);
	}
	else
	{
                /* mpeg2 */
		if( h_mode != 3 ) buf+=(17+4);
		else              buf+=(9+4);
	}

	if( buf[0] != VBRTag[0] && buf[0] != VBRTag2[0] ) return 0;    /* fail */
	if( buf[1] != VBRTag[1] && buf[1] != VBRTag2[1]) return 0;    /* header not found*/
	if( buf[2] != VBRTag[2] && buf[2] != VBRTag2[2]) return 0;
	if( buf[3] != VBRTag[3] && buf[3] != VBRTag2[3]) return 0;

	buf+=4;

	pTagData->h_id = h_id;

	head_flags = pTagData->flags = ExtractI4(buf); buf+=4;      /* get flags */

	if( head_flags & FRAMES_FLAG )
	{
		pTagData->frames   = ExtractI4(buf); buf+=4;
	}

	if( head_flags & BYTES_FLAG )
	{
		pTagData->bytes = ExtractI4(buf); buf+=4;
	}

	if( head_flags & TOC_FLAG )
	{
		if( pTagData->toc != NULL )
		{
			for(i=0;i<NUMTOCENTRIES;i++)
				pTagData->toc[i] = buf[i];
		}
		buf+=NUMTOCENTRIES;
	}

	pTagData->vbr_scale = -1;

	if( head_flags & VBR_SCALE_FLAG )
	{
		pTagData->vbr_scale = ExtractI4(buf); buf+=4;
	}

	pTagData->headersize = 
	  ((h_id+1)*72000*h_bitrate) / pTagData->samprate;

        buf+=21;
        enc_delay = buf[0] << 4;
        enc_delay += buf[1] >> 4;
        enc_padding= (buf[1] & 0x0F)<<8;
        enc_padding += buf[2];
        // check for reasonable values (this may be an old Xing header,
        // not a INFO tag)
        if (enc_delay<0 || enc_delay > 3000) enc_delay=-1;
        if (enc_padding<0 || enc_padding > 3000) enc_padding=-1;

        pTagData->enc_delay=enc_delay;
        pTagData->enc_padding=enc_padding;

#ifdef DEBUG_VBRTAG
	fprintf(stderr,"\n\n********************* VBR TAG INFO *****************\n");
	fprintf(stderr,"tag         :%s\n",VBRTag);
	fprintf(stderr,"head_flags  :%d\n",head_flags);
	fprintf(stderr,"bytes       :%d\n",pTagData->bytes);
	fprintf(stderr,"frames      :%d\n",pTagData->frames);
	fprintf(stderr,"VBR Scale   :%d\n",pTagData->vbr_scale);
        fprintf(stderr,"enc_delay  = %i \n",enc_delay);
        fprintf(stderr,"enc_padding= %i \n",enc_padding);
	fprintf(stderr,"toc:\n");
	if( pTagData->toc != NULL )
	{
		for(i=0;i<NUMTOCENTRIES;i++)
		{
			if( (i%10) == 0 ) fprintf(stderr,"\n");
			fprintf(stderr," %3d", (int)(pTagData->toc[i]));
		}
	}
	fprintf(stderr,"\n***************** END OF VBR TAG INFO ***************\n");
#endif
	return 1;       /* success */
}


/****************************************************************************
 * InitVbrTag: Initializes the header, and write empty frame to stream
 * Paramters:
 *				fpStream: pointer to output file stream
 *				nMode	: Channel Mode: 0=STEREO 1=JS 2=DS 3=MONO
 ****************************************************************************
*/
int InitVbrTag(lame_global_flags *gfp)
{
	int nMode,SampIndex;
	lame_internal_flags *gfc = gfp->internal_flags;
#define MAXFRAMESIZE 2880 // or 0xB40, the max freeformat 640 32kHz framesize
	//	uint8_t pbtStreamBuffer[MAXFRAMESIZE];
	nMode = gfp->mode;
	SampIndex = gfc->samplerate_index;


	/* Clear Frame position array variables */
	//gfp->pVbrFrames=NULL;
	gfp->nVbrNumFrames=0;
	gfp->nVbrFrameBufferSize=0;


	/* Clear stream buffer */
	//	memset(pbtStreamBuffer,0x00,sizeof(pbtStreamBuffer));



	/*
	// Xing VBR pretends to be a 48kbs layer III frame.  (at 44.1kHz).
        // (at 48kHz they use 56kbs since 48kbs frame not big enough for
        // table of contents)
	// let's always embed Xing header inside a 64kbs layer III frame.
	// this gives us enough room for a LAME version string too.
	// size determined by sampling frequency (MPEG1)
	// 32kHz:    216 bytes@48kbs    288bytes@ 64kbs
	// 44.1kHz:  156 bytes          208bytes@64kbs     (+1 if padding = 1)
	// 48kHz:    144 bytes          192
	//
	// MPEG 2 values are the same since the framesize and samplerate
        // are each reduced by a factor of 2.
	*/
	{
	int i,bitrate,tot;
	if (1==gfp->version) {
	  bitrate = XING_BITRATE1;
	} else {
	  if (gfp->out_samplerate < 16000 )
	    bitrate = XING_BITRATE25;
	  else
	    bitrate = XING_BITRATE2;
	}

	if (gfp->VBR==vbr_off)
			bitrate = gfp->brate;
	
	gfp->TotalFrameSize= 
  	  ((gfp->version+1)*72000*bitrate) / gfp->out_samplerate;

	tot = (gfc->sideinfo_len+LAMEHEADERSIZE);

	if (gfp->TotalFrameSize < tot || 
            gfp->TotalFrameSize > MAXFRAMESIZE ) {
            // disable tag, it wont fit
            gfp->bWriteVbrTag = 0;
            return 0;
        }

	for (i=0; i<gfp->TotalFrameSize; ++i)
	  add_dummy_byte(gfp,0);
	}

	/* Success */
	return 0;
}



/* fast CRC-16 computation - uses table crc16_lookup 8*/
int CRC_update_lookup(int value, int crc)
{
	int tmp;
	tmp=crc^value;
	crc=(crc>>8)^crc16_lookup[tmp & 0xff];
	return crc;
}

void UpdateMusicCRC(uint16_t *crc,unsigned char *buffer, int size){
    int i;
    for (i=0; i<size; ++i) 
        *crc = CRC_update_lookup(buffer[i],*crc);
}




void ReportLameTagProgress(lame_global_flags *gfp,int nStart)
{
	if (!gfp->bWriteVbrTag)
		return;

	if (nStart)
		MSGF( gfp->internal_flags, "Writing LAME Tag...");
	else
		MSGF( gfp->internal_flags, "done\n");

}


/****************************************************************************
 * Jonathan Dee 2001/08/31
 *
 * PutLameVBR: Write LAME info: mini version + info on various switches used
 * Paramters:
 *				pbtStreamBuffer	: pointer to output buffer  
 *				id3v2size		: size of id3v2 tag in bytes
 *				crc				: computation of crc-16 of Lame Tag so far (starting at frame sync)
 *				
 ****************************************************************************
*/
int PutLameVBR(lame_global_flags *gfp, FILE *fpStream, uint8_t *pbtStreamBuffer, uint32_t id3v2size,  uint16_t crc)
{
    lame_internal_flags *gfc = gfp->internal_flags;
//	FLOAT fVersion = LAME_MAJOR_VERSION + 0.01 * LAME_MINOR_VERSION;

	int nBytesWritten = 0;
	int nFilesize	  = 0;		//size of fpStream. Will be equal to size after process finishes.
	int i;

    int enc_delay=lame_get_encoder_delay(gfp);       // encoder delay
    int enc_padding=lame_get_encoder_padding(gfp);   // encoder padding 

	//recall:	gfp->VBR_q is for example set by the switch -V 
	//			gfp->quality by -q, -h, -f, etc
	
	int nQuality		= (100 - 10 * gfp->VBR_q - gfp->quality);
	

	const char *szVersion	= get_lame_very_short_version();
	uint8_t nVBR;
	uint8_t nRevision = 0x00;
	uint8_t nRevMethod;
	uint8_t vbr_type_translator[] = {1,5,3,2,4,0,3};		//numbering different in vbr_mode vs. Lame tag

	uint8_t nLowpass		= ( ((gfp->lowpassfreq / 100.0)+.5) > 255 ? 255 : (gfp->lowpassfreq / 100.0)+.5 );

	ieee754_float32_t fPeakSignalAmplitude	= 0;				//TODO...
	uint16_t nRadioReplayGain		= 0;				//TODO...
	uint16_t nAudioPhileReplayGain  = 0;				//TODO...




	uint8_t nNoiseShaping			= gfp->internal_flags->noise_shaping;
	uint8_t nStereoMode				= 0;
	int		bNonOptimal				= 0;
	uint8_t nSourceFreq				= 0;
	uint8_t nMisc					= 0;
	uint16_t nPresetValue			= 0;
	uint32_t nMusicLength			= 0;
	int		bId3v1Present			= ((gfp->internal_flags->tag_spec.flags & CHANGED_FLAG)
		&& !(gfp->internal_flags->tag_spec.flags & V2_ONLY_FLAG));
	uint16_t nMusicCRC				= 0;

	//psy model type: Gpsycho or NsPsytune
	unsigned char    bExpNPsyTune	= gfp->exp_nspsytune & 1;
	unsigned char	 bSafeJoint		= (gfp->exp_nspsytune & 2)!=0;

	unsigned char	 bNoGapMore		= 0;
	unsigned char	 bNoGapPrevious	= 0;

	int		 nNoGapCount	= gfp->internal_flags->nogap_total;
	int		 nNoGapCurr		= gfp->internal_flags->nogap_current;


	uint8_t  nAthType		= gfp->ATHtype;	//4 bits.
	
	uint8_t  nFlags			= 0;

	// if ABR, {store bitrate <=255} else { store "-b"}
	int nABRBitrate	= (gfp->VBR==vbr_abr)?gfp->VBR_mean_bitrate_kbps:gfp->brate;

	//revision and vbr method
	if (gfp->VBR>=0 && gfp->VBR < sizeof(vbr_type_translator))
		nVBR = vbr_type_translator[gfp->VBR];
	else
		nVBR = 0x00;		//unknown.

	nRevMethod = 0x10 * nRevision + nVBR; 
	
	//nogap
	if (nNoGapCount != -1)
	{
		if (nNoGapCurr > 0)
			bNoGapPrevious = 1;

		if (nNoGapCurr < nNoGapCount-1)
			bNoGapMore = 1;
	}

	//flags

	nFlags	= nAthType	+ (bExpNPsyTune		<< 4)
						+ (bSafeJoint		<< 5)
						+ (bNoGapMore		<< 6)
						+ (bNoGapPrevious	<< 7);


	if (nQuality < 0)
		nQuality = 0;

	/*stereo mode field... a bit ugly.*/

	switch(gfp->mode)
	{
	case MONO:
		nStereoMode = 0;
		break;
	case STEREO:
		nStereoMode = 1;
		break;
	case DUAL_CHANNEL:
		nStereoMode = 2;
		break;
	case JOINT_STEREO:
		if (gfp->force_ms)
			nStereoMode = 4;
		else
			nStereoMode = 3;
		break;
	case NOT_SET:
	    /* FALLTHROUGH */
	default:
		nStereoMode = 7;
		break;
	}

	if (gfp->mode_automs)
		nStereoMode = 5;

	/*Intensity stereo : nStereoMode = 6. IS is not implemented */

	if (gfp->in_samplerate <= 32000)
		nSourceFreq = 0x00;
	else if (gfp->in_samplerate ==48000)
		nSourceFreq = 0x02;
	else if (gfp->in_samplerate > 48000)
		nSourceFreq = 0x03;
	else
		nSourceFreq = 0x01;  //default is 44100Hz.


	//Check if the user overrided the default LAME behaviour with some nasty options

	if (gfp->short_blocks == short_block_forced 			||
		gfp->short_blocks == short_block_dispensed 		||
		((gfp->lowpassfreq == -1) && (gfp->highpassfreq == -1))	|| // "-k"
		(gfp->scale_left != gfp->scale_right)			||
		gfp->disable_reservoir		||
		gfp->noATH			||
		gfp->ATHonly			||
		(nAthType == 0)    ||
		gfp->in_samplerate <= 32000)
			bNonOptimal = 1;
	
	nMisc =		nNoiseShaping
			+	(nStereoMode << 2)
			+	(bNonOptimal << 5)
			+	(nSourceFreq << 6);


	
	//get filesize
	fseek(fpStream, 0, SEEK_END);
	nFilesize = ftell(fpStream);

    nPresetValue = gfp->preset;
	
	nMusicLength = nFilesize - id3v2size;		//omit current frame
	if (bId3v1Present)
		nMusicLength-=128;                     //id3v1 present.
        nMusicCRC = gfc->nMusicCRC;


	/*Write all this information into the stream*/
	CreateI4(&pbtStreamBuffer[nBytesWritten], nQuality);
	nBytesWritten+=4;

	strncpy(&pbtStreamBuffer[nBytesWritten], szVersion, 9);
	nBytesWritten+=9;

	pbtStreamBuffer[nBytesWritten] = nRevMethod ;
	nBytesWritten++;

	pbtStreamBuffer[nBytesWritten] = nLowpass;
	nBytesWritten++;

	memmove(&pbtStreamBuffer[nBytesWritten], &fPeakSignalAmplitude, 4);
	nBytesWritten+=4;

	CreateI2(&pbtStreamBuffer[nBytesWritten],nRadioReplayGain);
	nBytesWritten+=2;

	CreateI2(&pbtStreamBuffer[nBytesWritten],nAudioPhileReplayGain);
	nBytesWritten+=2;

	pbtStreamBuffer[nBytesWritten] = nFlags;
	nBytesWritten++;

	if (nABRBitrate >= 255)
		pbtStreamBuffer[nBytesWritten] = 0xFF;
	else
		pbtStreamBuffer[nBytesWritten] = nABRBitrate;
	nBytesWritten++;

        pbtStreamBuffer[nBytesWritten   ] = enc_delay >> 4; // works for win32, does it for unix?
        pbtStreamBuffer[nBytesWritten +1] = (enc_delay << 4) + (enc_padding >> 8);
        pbtStreamBuffer[nBytesWritten +2] = enc_padding;

	nBytesWritten+=3;

	pbtStreamBuffer[nBytesWritten] = nMisc;
	nBytesWritten++;


	memset(pbtStreamBuffer+nBytesWritten,0, 1);		//unused in rev1
	nBytesWritten+=1;

	CreateI2(&pbtStreamBuffer[nBytesWritten], nPresetValue);
	nBytesWritten+=2;

	CreateI4(&pbtStreamBuffer[nBytesWritten], nMusicLength);
	nBytesWritten+=4;

	CreateI2(&pbtStreamBuffer[nBytesWritten], nMusicCRC);
	nBytesWritten+=2;

	/*Calculate tag CRC.... must be done here, since it includes
	 *previous information*/
	
	for (i = 0;i<nBytesWritten;i++)
		crc = CRC_update_lookup(pbtStreamBuffer[i], crc);
	
	CreateI2(&pbtStreamBuffer[nBytesWritten], crc);
	nBytesWritten+=2;

	return nBytesWritten;
}

/***********************************************************************
 * 
 * PutVbrTag: Write final VBR tag to the file
 * Paramters:
 *				lpszFileName: filename of MP3 bit stream
 *				nVbrScale	: encoder quality indicator (0..100)
 ****************************************************************************
*/
int PutVbrTag(lame_global_flags *gfp,FILE *fpStream,int nVbrScale)
{
    lame_internal_flags * gfc = gfp->internal_flags;

	long lFileSize;
	int nStreamIndex;
	char abyte,bbyte;
	uint8_t		btToc[NUMTOCENTRIES];
	uint8_t pbtStreamBuffer[MAXFRAMESIZE];
	
	int i;
	uint16_t crc = 0x00;
	
    unsigned char id3v2Header[10];
    size_t id3v2TagSize;

    if (gfc->VBR_seek_table.pos <= 0)
	return -1;


	/* Clear stream buffer */
	memset(pbtStreamBuffer,0x00,sizeof(pbtStreamBuffer));

	/* Seek to end of file*/
	fseek(fpStream,0,SEEK_END);

	/* Get file size */
	lFileSize=ftell(fpStream);

	/* Abort if file has zero length. Yes, it can happen :) */
	if (lFileSize==0)
		return -1;

        /*
         * The VBR tag may NOT be located at the beginning of the stream.
         * If an ID3 version 2 tag was added, then it must be skipped to write
         * the VBR tag data.
         */

        /* seek to the beginning of the stream */
	fseek(fpStream,0,SEEK_SET);
        /* read 10 bytes in case there's an ID3 version 2 header here */
        fread(id3v2Header,1,sizeof id3v2Header,fpStream);
        /* does the stream begin with the ID3 version 2 file identifier? */
        if (!strncmp((char *)id3v2Header,"ID3",3)) {
          /* the tag size (minus the 10-byte header) is encoded into four
           * bytes where the most significant bit is clear in each byte */
          id3v2TagSize=(((id3v2Header[6] & 0x7f)<<21)
            | ((id3v2Header[7] & 0x7f)<<14)
            | ((id3v2Header[8] & 0x7f)<<7)
            | (id3v2Header[9] & 0x7f))
            + sizeof id3v2Header;
        } else {
          /* no ID3 version 2 tag in this stream */
          id3v2TagSize=0;
        }

	/* Seek to first real frame */
	fseek(fpStream,id3v2TagSize+gfp->TotalFrameSize,SEEK_SET);

	/* Read the header (first valid frame) */
	fread(pbtStreamBuffer,4,1,fpStream);

	/* the default VBR header. 48 kbps layer III, no padding, no crc */
	/* but sampling freq, mode andy copyright/copy protection taken */
	/* from first valid frame */
	pbtStreamBuffer[0]=(uint8_t) 0xff;
	abyte = (pbtStreamBuffer[1] & (char) 0xf1);
	{	
		int bitrate;
		if (1==gfp->version) {
		  bitrate = XING_BITRATE1;
		} else {
		  if (gfp->out_samplerate < 16000 )
			bitrate = XING_BITRATE25;
		  else
			bitrate = XING_BITRATE2;
		}
		
		if (gfp->VBR==vbr_off)
			bitrate = gfp->brate;

		bbyte = 16*BitrateIndex(bitrate,gfp->version,gfp->out_samplerate);
	}

	/* Use as much of the info from the real frames in the
	 * Xing header:  samplerate, channels, crc, etc...
	 */ 
	if (gfp->version==1) {
	  /* MPEG1 */
	  pbtStreamBuffer[1]=abyte | (char) 0x0a;     /* was 0x0b; */
	  abyte = pbtStreamBuffer[2] & (char) 0x0d;   /* AF keep also private bit */
	  pbtStreamBuffer[2]=(char) bbyte | abyte;     /* 64kbs MPEG1 frame */
	}else{
	  /* MPEG2 */
	  pbtStreamBuffer[1]=abyte | (char) 0x02;     /* was 0x03; */
	  abyte = pbtStreamBuffer[2] & (char) 0x0d;   /* AF keep also private bit */
	  pbtStreamBuffer[2]=(char) bbyte | abyte;     /* 64kbs MPEG2 frame */
	}

	/* Clear all TOC entries */
	memset(btToc,0,sizeof(btToc));

        Xing_seek_table (&gfc->VBR_seek_table, btToc);
        /* print_seeking (btToc); */

	/* Start writing the tag after the zero frame */
	nStreamIndex=gfc->sideinfo_len;
	/* note! Xing header specifies that Xing data goes in the
	 * ancillary data with NO ERROR PROTECTION.  If error protecton
	 * in enabled, the Xing data still starts at the same offset,
	 * and now it is in sideinfo data block, and thus will not
	 * decode correctly by non-Xing tag aware players */
	if (gfp->error_protection) nStreamIndex -= 2;

	/* Put Vbr tag */
	if (gfp->VBR == vbr_off)
	{
		pbtStreamBuffer[nStreamIndex++]=VBRTag2[0];
		pbtStreamBuffer[nStreamIndex++]=VBRTag2[1];
		pbtStreamBuffer[nStreamIndex++]=VBRTag2[2];
		pbtStreamBuffer[nStreamIndex++]=VBRTag2[3];

	}
	else
	{
		pbtStreamBuffer[nStreamIndex++]=VBRTag[0];
		pbtStreamBuffer[nStreamIndex++]=VBRTag[1];
		pbtStreamBuffer[nStreamIndex++]=VBRTag[2];
		pbtStreamBuffer[nStreamIndex++]=VBRTag[3];
	}	

	/* Put header flags */
	CreateI4(&pbtStreamBuffer[nStreamIndex],FRAMES_FLAG+BYTES_FLAG+TOC_FLAG+VBR_SCALE_FLAG);
	nStreamIndex+=4;

	/* Put Total Number of frames */
	CreateI4(&pbtStreamBuffer[nStreamIndex],gfp->nVbrNumFrames);
	nStreamIndex+=4;

	/* Put Total file size */
	CreateI4(&pbtStreamBuffer[nStreamIndex],(int)lFileSize);
	nStreamIndex+=4;

	/* Put TOC */
	memcpy(&pbtStreamBuffer[nStreamIndex],btToc,sizeof(btToc));
	nStreamIndex+=sizeof(btToc);


	if (gfp->error_protection) {
	  /* (jo) error_protection: add crc16 information to header */
	  CRC_writeheader(gfc, pbtStreamBuffer);
	}



	//work out CRC so far: initially crc = 0
	for (i = 0;i< nStreamIndex ;i++)
		crc = CRC_update_lookup(pbtStreamBuffer[i], crc);

	/*Put LAME VBR info*/
	nStreamIndex+=PutLameVBR(gfp, fpStream, pbtStreamBuffer + nStreamIndex, id3v2TagSize,crc);

#ifdef DEBUG_VBRTAG
	{
	  VBRTAGDATA TestHeader;
	  GetVbrTag(&TestHeader,pbtStreamBuffer);
	}
#endif

	/*Seek to the beginning of the stream */
	fseek(fpStream,id3v2TagSize,SEEK_SET);

        /* Put it all to disk again */
	if (fwrite(pbtStreamBuffer,(unsigned int)gfp->TotalFrameSize,1,fpStream)!=1)
	{
		return -1;
	}
	/* Save to delete the frame buffer */
	//free(gfp->pVbrFrames);  see HACKING for instructions on how
	//gfp->pVbrFrames=NULL;   memory in 'gfp' is allocated/free'd

	return 0;       /* success */
}

