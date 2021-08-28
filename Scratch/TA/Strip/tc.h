/*
 * $Header: /home/grantham/cvsroot/projects/modules/tc/tc.h,v 1.14 2000/10/03 07:19:20 grantham Exp $
 */

#if !defined(_ACTC_H_)
#define _ACTC_H_

#include <sys/types.h>

typedef struct _ACTCData ACTCData;

/*
    Abbreviated:
    	vertex		vert
	primitive	prim
	maximum		max
	minimum		min
	parameter	param
 */

#define ACTC_NO_ERROR			0
#define ACTC_ALLOC_FAILED		-0x2000
#define ACTC_DURING_INPUT		-0x2001
#define ACTC_DURING_OUTPUT		-0x2002
#define ACTC_IDLE			-0x2003
#define ACTC_INVALID_VALUE		-0x2004
#define ACTC_DATABASE_EMPTY		-0x2005
#define ACTC_DATABASE_CORRUPT		-0x2006
#define ACTC_PRIM_COMPLETE		-0x2007

#define ACTC_OUT_MIN_FAN_VERTS		0x1000
#define ACTC_OUT_HONOR_WINDING		0x1001
#define ACTC_OUT_MAX_PRIM_VERTS		0x1004
#define ACTC_IN_MIN_VERT		0x1005
#define ACTC_IN_MAX_VERT		0x1006
#define ACTC_IN_MAX_VERT_SHARING	0x1007
#define ACTC_IN_MAX_EDGE_SHARING	0x1008
#define ACTC_MINOR_VERSION		0x1009
#define ACTC_MAJOR_VERSION		0x1010

#define ACTC_PRIM_FAN			0x2000
#define ACTC_PRIM_STRIP			0x2001

#define ACTC_TRUE			1
#define ACTC_FALSE			0

#ifdef __cplusplus
extern "C" {
#endif

ACTCData *actcNew(void);
int actcParami(ACTCData *tc, int param, int value);
int actcGetParami(ACTCData *tc, int param, int *value);
int actcParamu(ACTCData *tc, int param, unsigned value);
int actcGetParamu(ACTCData *tc, int param, unsigned *value);
int actcGetError(ACTCData *tc);
int actcMakeEmpty(ACTCData *tc);
void actcDelete(ACTCData *tc);
void actcDumpState(ACTCData *tc, FILE *fp);

int actcBeginInput(ACTCData *tc);
int actcGetIsDuringInput(ACTCData *tc);
int actcAddTriangle(ACTCData *tc, unsigned v1, unsigned v2, unsigned v3);
int actcEndInput(ACTCData *tc);

int actcBeginOutput(ACTCData *tc);
int actcGetIsDuringOutput(ACTCData *tc);
int actcStartNextPrim(ACTCData *tc, unsigned *v1Return, unsigned *v2Return);
int actcGetNextVert(ACTCData *tc, unsigned *vReturn);
int actcEndOutput(ACTCData *tc);

int actcGetMemoryAllocation(ACTCData *tc, size_t *bytesAllocated);

int actcTrianglesToPrimitives(ACTCData *tc, int triangleCount,
    unsigned (*triangles)[3], int primTypes[], int primLengths[], unsigned vertices[],
    int maxBatchSize);

#ifdef __cplusplus
}
#endif

#endif /* _ACTC_H_ */

/* vi:tabstop=8
 */
