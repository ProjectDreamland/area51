/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/*
 * I/O Processor Library 
 *
 * Copyright (C) 1998-2002 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 *
 * libsd.h
 */

#ifndef _LIBSD_H_
#define _LIBSD_H_

#include <sys/types.h>
#include <sdmacro.h>

extern int     sceSdInit( int flag );
extern void    sceSdSetParam( u_short entry, u_short value );
extern u_short sceSdGetParam( u_short entry );
extern void    sceSdSetSwitch( u_short entry, u_int value );
extern u_int   sceSdGetSwitch( u_short entry );
extern void    sceSdSetAddr( u_short entry, u_int value );
extern u_int   sceSdGetAddr( u_short entry );
extern void    sceSdSetCoreAttr( u_short entry, u_short value );
extern u_short sceSdGetCoreAttr( u_short entry );
extern u_short sceSdNote2Pitch ( u_short center_note, u_short center_fine,
			 u_short note, short fine);
extern u_short sceSdPitch2Note ( u_short center_note, u_short center_fine, u_short pitch);
extern int     sceSdProcBatch( sceSdBatch* batch, u_int returns[], u_int num  );
extern int     sceSdProcBatchEx( sceSdBatch* batch, u_int returns[], u_int num, u_int voice  );
extern int     sceSdVoiceTrans( short channel, u_short mode, u_char *m_addr, u_int s_addr, u_int size );
extern int     sceSdBlockTrans( short channel, u_short mode, u_char *m_addr, u_int size, ... );
extern int     sceSdStopTrans (int channel);
extern int     sceSdVoiceTransStatus (short channel, short flag);
extern u_int   sceSdBlockTransStatus (short channel, short flag);
extern int     sceSdSetEffectAttr ( int core, sceSdEffectAttr *attr );
extern void    sceSdGetEffectAttr ( int core, sceSdEffectAttr *attr );
extern int     sceSdClearEffectWorkArea ( int core, int channel, int effect_mode );
extern int     sceSdCleanEffectWorkArea ( int core, int channel, int effect_mode );

extern int sceSdSetEffectMode (int core, sceSdEffectAttr *param);
extern int sceSdSetEffectModeParams (int core, sceSdEffectAttr *attr);

extern sceSdSpu2IntrHandler sceSdSetSpu2IntrHandler (sceSdSpu2IntrHandler func, void *arg);
extern sceSdTransIntrHandler sceSdSetTransIntrHandler (int channel, sceSdTransIntrHandler func, void *arg);
extern void *sceSdGetTransIntrHandlerArgument (int);
extern void *sceSdGetSpu2IntrHandlerArgument (void);

#endif /* _LIBSD_H_ */
