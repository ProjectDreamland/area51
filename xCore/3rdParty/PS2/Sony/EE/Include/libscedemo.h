/*
 * SCE Demo Disc SDK Version 2.0
 *
 * For support please contact:
 * SCEE: demo_support@scee.net
 * SCEA: 
 */

#ifndef _LIBSCEDEMO_H_
#define _LIBSCEDEMO_H_

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

/* LANGUAGE */
#define SCE_DEMO_LANGUAGE_ENGLISH    0
#define SCE_DEMO_LANGUAGE_FRENCH     1
#define SCE_DEMO_LANGUAGE_GERMAN     2
#define SCE_DEMO_LANGUAGE_SPANISH    3
#define SCE_DEMO_LANGUAGE_ITALIAN    4
#define SCE_DEMO_LANGUAGE_DUTCH      5
#define SCE_DEMO_LANGUAGE_PORTUGUESE 6
#define SCE_DEMO_LANGUAGE_JAPANESE   7
#define SCE_DEMO_LANGUAGE_DANISH     8
#define SCE_DEMO_LANGUAGE_NORWEGIAN  9
#define SCE_DEMO_LANGUAGE_FINNISH    10
#define SCE_DEMO_LANGUAGE_SWEDISH    11
#define SCE_DEMO_LANGUAGE_KOREAN     12

/* ASPECT RATIO */
#define SCE_DEMO_ASPECT_4_3          0
#define SCE_DEMO_ASPECT_16_9         1

/* PLAYABLE MODE */
#define SCE_DEMO_PLAYMODE_PLAYABLE   0
#define SCE_DEMO_PLAYMODE_ATTRACT    1
#define SCE_DEMO_PLAYMODE_ONLINE_PLAYABLE     2
#define SCE_DEMO_PLAYMODE_ONLINE_ATTRACT     3

/* MEDIA TYPE - DEPRECATED */
#define SCE_DEMO_MEDIATYPE_CD        0
#define SCE_DEMO_MEDIATYPE_DVD       1

/* SCEDEMOEND REASON */
typedef enum {
    SCE_DEMO_ENDREASON_ATTRACT_INTERRUPTED,
    SCE_DEMO_ENDREASON_ATTRACT_COMPLETE,
    SCE_DEMO_ENDREASON_PLAYABLE_INACTIVITY_TIMEOUT,
    SCE_DEMO_ENDREASON_PLAYABLE_GAMEPLAY_TIMEOUT,
    SCE_DEMO_ENDREASON_PLAYABLE_COMPLETE,
    SCE_DEMO_ENDREASON_PLAYABLE_QUIT,
    SCE_DEMO_ENDREASON_NETCONFIG_REQUEST,
    SCE_DEMO_ENDREASON_NETCONFIG_COMPLETE,
} sceDemoEndReason;

/* PROTOTYPES */
void sceDemoStart(int            argc,
		  char           **argv,
		  unsigned short *language,
		  unsigned short *aspect,
		  unsigned short *playmode,
		  unsigned short *to_inactive,
		  unsigned short *to_gameplay,
		  unsigned char *passphrase,
		  unsigned char *authdata,
		  unsigned int *buffer
		  );

void sceDemoEnd(sceDemoEndReason why);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif /* _LIBSCEDEMO_H_ */
