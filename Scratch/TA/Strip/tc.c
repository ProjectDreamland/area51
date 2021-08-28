/*
 * $Header: /home/grantham/cvsroot/projects/modules/tc/tc.c,v 1.27 2000/10/10 16:09:23 grantham Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "tc.h"

/* SNIPPET "table.c" Inducted Wed Nov 22 09:36:55 2000 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(MEM_CHART)
#define chartedSetLabel(a)
#endif

#define LEVEL1COUNT 16384
#define LEVEL2COUNT 1024
#define LEVEL3COUNT 256

typedef struct TableLevel3
{
    int EntryCount;
    void *Table[LEVEL3COUNT];
    char IsSet[LEVEL3COUNT];
} TableLevel3;

typedef struct TableLevel2
{
    int EntryCount;
    TableLevel3 *Table[LEVEL2COUNT];
} TableLevel2;

typedef struct TableRoot
{
    size_t EntryCount;
    size_t TotalEntryCount;
    size_t TotalAllocatedBytes;
    int EmptyEntryCount;
    TableLevel2 *Table[LEVEL1COUNT];
} TableRoot;

typedef struct TableIterator {
    int i1, i2, i3;
    unsigned i;
    TableLevel3 *CurLevel3;
    int CheckLevel1, CheckLevel2;
} TableIterator;

TableRoot *tableNew(void)
{
    TableRoot *tr;

    chartedSetLabel("table root");
    tr = (TableRoot *)calloc(sizeof(TableRoot), 1);
    return tr;
}

TableIterator *tableNewIterator(void)
{
    TableIterator *ti;

    chartedSetLabel("table iterator");
    ti = (TableIterator *)malloc(sizeof(TableIterator));
    return ti;
}

int tableRetrieve(unsigned a, TableRoot *table, void **ref)
{
    int i1 = a / (LEVEL2COUNT * LEVEL3COUNT);
    int i2 = (a / LEVEL3COUNT) % LEVEL2COUNT;
    int i3 = a % LEVEL3COUNT;

    if(table->Table[i1] == NULL)
        return 0;
    if(table->Table[i1]->Table[i2] == NULL)
        return 0;
    if(!table->Table[i1]->Table[i2]->IsSet[i3])
        return 0;
    if(ref != NULL) 
        *ref = table->Table[i1]->Table[i2]->Table[i3];
    return 1;
}

int tableInsert(unsigned a, TableRoot *table, void *ref)
{
    int i1 = a / (LEVEL2COUNT * LEVEL3COUNT);
    int i2 = (a / LEVEL3COUNT) % LEVEL2COUNT;
    int i3 = a % LEVEL3COUNT;

    if(table->Table[i1] == NULL) {
	chartedSetLabel("table level 2");
        table->Table[i1] = (TableLevel2 *)calloc(1, sizeof(TableLevel2));
	table->TotalAllocatedBytes += sizeof(TableLevel2);
        if(table->Table[i1] == NULL)
            return 0;
    }
    if(table->Table[i1]->Table[i2] == NULL) {
	chartedSetLabel("table level 3");
        table->Table[i1]->Table[i2] =
	    (TableLevel3 *)calloc(1, sizeof(TableLevel3));
	table->TotalAllocatedBytes += sizeof(TableLevel3);
        if(table->Table[i1]->Table[i2] == NULL)
            return 0;
	table->Table[i1]->EntryCount++;
	table->TotalEntryCount += LEVEL3COUNT;
	table->EmptyEntryCount += LEVEL3COUNT;
    }
    if(!table->Table[i1]->Table[i2]->IsSet[i3]) {
	table->Table[i1]->Table[i2]->EntryCount++;
	table->EmptyEntryCount --;
	table->Table[i1]->Table[i2]->IsSet[i3] = 1;
    }
    table->Table[i1]->Table[i2]->Table[i3] = ref;
    return 1;
}

int tableRemove(unsigned a, TableRoot *table, void **wasref)
{
    int i1 = a / (LEVEL2COUNT * LEVEL3COUNT);
    int i2 = (a / LEVEL3COUNT) % LEVEL2COUNT;
    int i3 = a % LEVEL3COUNT;

    if(table->Table[i1] == NULL)
	return 0;
    if(table->Table[i1]->Table[i2] == NULL)
	return 0;
    if(!table->Table[i1]->Table[i2]->IsSet[i3])
        return 0;
    if(wasref != NULL)
        *wasref = table->Table[i1]->Table[i2]->Table[i3];
    table->Table[i1]->Table[i2]->IsSet[i3] = 0;
    table->EmptyEntryCount ++;
    if(--table->Table[i1]->Table[i2]->EntryCount == 0) {
	table->EmptyEntryCount -= LEVEL3COUNT;
	table->TotalEntryCount -= LEVEL3COUNT;
	free(table->Table[i1]->Table[i2]);
	table->TotalAllocatedBytes -= sizeof(TableLevel3);
	table->Table[i1]->Table[i2] = NULL;
	if(--table->Table[i1]->EntryCount == 0) {
	    table->TotalAllocatedBytes -= sizeof(TableLevel2);
	    free(table->Table[i1]);
	    table->Table[i1] = NULL;
	}
    }
    return 1;
}

void tableResetIterator(TableIterator *ti)
{
    ti->i1 = 0;
    ti->i2 = 0;
    ti->i3 = 0;
    ti->i = 0;
    ti->CheckLevel1 = 1;
    ti->CheckLevel2 = 1;
}

int tableIterate(TableRoot *table, TableIterator *ti, unsigned *i, void **ref)
{
    int done;

    done = 0;
    while(ti->i1 < LEVEL1COUNT) {
        if(ti->CheckLevel1 && table->Table[ti->i1] == NULL) {
	    ti->i += LEVEL2COUNT * LEVEL3COUNT;
	    ti->i1++;
	    continue;
	} else
	    ti->CheckLevel1 = 0;
        if(ti->CheckLevel2 && table->Table[ti->i1]->Table[ti->i2] == NULL) {
	    ti->i += LEVEL3COUNT;
	    if(++ti->i2 >= LEVEL2COUNT) {
		ti->i2 = 0;
		ti->i1++;
		ti->CheckLevel1 = 1;
	    }
	    continue;
	} else 
	    ti->CheckLevel2 = 0;
	if(ti->i3 == 0)
	    ti->CurLevel3 = table->Table[ti->i1]->Table[ti->i2];
	if(ti->CurLevel3->IsSet[ti->i3]) {
	    if(ref != NULL)
		*ref = ti->CurLevel3->Table[ti->i3];
	    if(i != NULL)
		*i = ti->i;
	    done = 1;
	}
	ti->i++;
	if(++ti->i3 >= LEVEL3COUNT) {
	    ti->i3 = 0;
	    ti->CheckLevel2 = 1;
	    if(++ti->i2 >= LEVEL2COUNT) {
		ti->i2 = 0;
		ti->i1++;
		ti->CheckLevel1 = 1;
	    }
	}
	if(done)
	    return 1;
    }
    return 0;
}

void tableDelete(TableRoot *table, void (*datumDelete)(void *datum))
{
    int i1, i2, i3;

    for(i1 = 0; i1 < LEVEL1COUNT; i1++) {
        if(table->Table[i1] != NULL) {
	    for(i2 = 0; i2 < LEVEL2COUNT; i2++) {
		if(table->Table[i1]->Table[i2] != NULL) {
		    for(i3 = 0; i3 < LEVEL3COUNT; i3++) {
			if(table->Table[i1]->Table[i2]->IsSet[i3])
			    datumDelete(table->Table[i1]->Table[i2]->Table[i3]);
		    }
		    free(table->Table[i1]->Table[i2]);
		}
	    }
	    free(table->Table[i1]);
	    table->Table[i1] = NULL;
	}
    }
    table->TotalEntryCount = 0;
    table->EmptyEntryCount = 0;
    table->TotalAllocatedBytes = 0;
}

void tableGetStats(TableRoot *table, size_t *totalBytes, size_t *emptyCount,
    size_t *totalCount)
{
    if(emptyCount != NULL)
        *emptyCount = table->EmptyEntryCount;
    if(totalCount != NULL)
        *totalCount = table->TotalEntryCount;
    if(totalBytes != NULL)
        *totalBytes = table->TotalAllocatedBytes;
}

size_t tableGetIteratorSize(void)
{
    return sizeof(TableIterator);
}

/* "table.c" ENDSNIPPET */

#if !defined(MEM_CHART)
#define chartedSetLabel(a)
#endif

#if defined(DEBUG)
#define ACTC_DEBUG(a) a
#else
#define ACTC_DEBUG(a)
#endif

#if defined(INFO)
#define ACTC_INFO(a) a
#else
#define ACTC_INFO(a)
#endif


#define ACTC_CHECK(a) \
    { \
        int theErrorNow; \
	theErrorNow = (a); \
	if(theErrorNow < 0) \
	    return theErrorNow; \
    }

typedef struct {
    struct ACTCVertex *FinalVert;
} ACTCTriangle;

typedef struct {
    struct ACTCVertex *V2;
    int Count;
    int TriangleCount;
    ACTCTriangle *Triangles;
} ACTCEdge;

typedef struct ACTCVertex {
    unsigned V;
    int Count;
    struct ACTCVertex **PointsToMe;
    struct ACTCVertex *Next;
    int EdgeCount;
    ACTCEdge *Edges;	
} ACTCVertex;

/* private tokens */
#define ACTC_NO_MATCHING_VERT		-0x3000
#define ACTC_FWD_ORDER			0
#define ACTC_REV_ORDER			1

#define MAX_STATIC_VERTS		10000000 /* buh? */

struct _ACTCData {

    /* vertex and edge database */
    int VertexCount;
    TableRoot *Vertices;
    TableIterator *VertexIterator;
    int CurMaxVertValence;
    int CurMinVertValence;
    ACTCVertex **VertexBins;

    /* alternate vertex array if range small enough */
    ACTCVertex *StaticVerts;
    int UsingStaticVerts;
    unsigned VertRange;

    /* During consolidation */
    int CurWindOrder;
    int PrimType;
    ACTCVertex *V1;
    ACTCVertex *V2;
    int VerticesSoFar;

    /* Error and state handling */
    int IsInputting;
    int IsOutputting;
    int Error;

    /* actcParam-settable parameters */
    unsigned MinInputVert;
    unsigned MaxInputVert;
    int MaxVertShare;
    int MaxEdgeShare;
    int MinFanVerts;
    int MaxPrimVerts;
    int HonorWinding;
};

#if defined(DEBUG) || defined(INFO)

static void dumpTriangles(ACTCEdge *e, FILE *fp)
{
    int i;
    int c;
    char v[12];

    c = fprintf(fp, "      %d triangles: ");
    for(i = 0; i < e->TriangleCount; i++) {
	if(c + 1 + sprintf(v, "%u", e->Triangles[i].FinalVert) > 78) {
	    fputs("\n", fp);
	    c = fprintf(fp, "        ");
	}
	c += fprintf(fp, " %s", v);
    }
    fputs("\n", fp);
}

static void dumpEdges(ACTCVertex *vert, FILE *fp)
{
    int i;
    int c;
    char v[26]; /* two signed ints plus x plus NUL */

    for(i = 0; i < vert->EdgeCount; i++) {
	fprintf(fp, "    %u->%u (%d times)\n", vert->V, vert->Edges[i].V2->V,
	    vert->Edges[i].Count);
	dumpTriangles(&vert->Edges[i], fp);
    }
    fputs("\n", fp);
}

static void dumpVertices(ACTCData *tc, FILE *fp)
{
    int i;
    ACTCVertex *v;

    if(!tc->UsingStaticVerts)
        tableResetIterator(tc->VertexIterator);

    fprintf(fp, "%d vertices in valences list\n", tc->VertexCount);
    if(tc->UsingStaticVerts) {
        for(i = 0; i < tc->VertRange; i++) {
	    v = &tc->StaticVerts[i];
	    if(v->Count > 0) {
		fprintf(fp, "  vertex %u, valence %d, %d edges\n", v->V,
		    v->Count, v->EdgeCount);
		dumpEdges(v, fp);
	    }
	}
    } else {
	for(i = 0; i < tc->VertexCount; i++) {
	    if(tableIterate(tc->Vertices, tc->VertexIterator, NULL,
		(void **)&v) == 0) {
		fprintf(fp, "ACTC::dumpVertices : fewer vertices in the table "
		    "than we expected!\n");
		fprintf(stderr, "ACTC::dumpVertices : fewer vertices in the table "
		    "than we expected!\n");
	    }
	    if(v == NULL) {
		fprintf(fp, "ACTC::dumpVertices : did not expect to get a NULL"
		    "Vertex from the table iterator!\n");
		fprintf(stderr, "ACTC::dumpVertices : did not expect to get a NULL"
		    "Vertex from the table iterator!\n");
	    }
	    fprintf(fp, "  vertex %u, valence %d, %d edges\n", v->V, v->Count,
		v->EdgeCount);
	    dumpEdges(v, fp);
	}
    }
}

static void dumpVertexBins(ACTCData *tc, FILE *fp)
{
    ACTCVertex *cur;
    int i;
    int c;
    char v[26]; /* two signed ints plus x plus NUL */

    fprintf(fp, "vertex bins:\n");
    if(tc->VertexBins == NULL) {
        fprintf(fp, "        empty.\n");
	return;
    }
    for(i = 1; i <= tc->CurMaxVertValence; i++) {
        cur = tc->VertexBins[i];
	c = fprintf(fp, "        bin %d -> ", i);
	while(cur != NULL) {
	    if(c + 1 + sprintf(v, "%ux%d", cur->V, cur->Count) > 78) {
		fputs("\n", fp);
		c = fprintf(fp, "          ");
	    }
	    c += fprintf(fp, " %s", v);
	    cur = cur->Next;
	}
	fputs("\n", fp);
    }
}

void actcDumpState(ACTCData *tc, FILE *fp)
{
    dumpVertices(tc, fp);
    dumpVertexBins(tc, fp);
}

#endif /* DEBUG || INFO */

#if defined(DEBUG)

static int abortWithOptionalDump(ACTCData *tc)
{
    ACTC_INFO(actcDumpState(tc, stderr));
    abort();
}

#endif /* defined(DEBUG) */

int actcGetError(ACTCData *tc)
{
    int error = tc->Error;
    tc->Error = ACTC_NO_ERROR;
    return error;
}

static void *reallocAndAppend(void **ptr, unsigned *itemCount, size_t itemBytes,
    void *append)
{
    void *t;

    t = realloc(*ptr, itemBytes * (*itemCount + 1));
    if(t == NULL)
	return NULL;
    *ptr = t;

    memcpy((unsigned char *)*ptr + *itemCount * itemBytes, append, itemBytes);
    (*itemCount) += 1;

    return *ptr;
}

/*
 * Call only during input; changes vertices' valences and does not
 * fix the bins that are ordered by vertex valence.  (It's usually cheaper
 * to traverse the vertex list once after all are added, since that's
 * linear in the NUMBER OF UNIQUE VERTEX INDICES, which is almost always
 * going to be less than the number of vertices.)
 */
static int incVertexValence(ACTCData *tc, unsigned v, ACTCVertex **found)
{
    ACTCVertex *vertex;

    if(tc->UsingStaticVerts) {
        vertex = &tc->StaticVerts[v];
	vertex->Count++;
	if(vertex->Count == 1) {
	    vertex->V = v;
	    tc->VertexCount++;
	}
    } else {
	if(tableRetrieve(v, tc->Vertices, (void **)&vertex) == 1) {
	    if(vertex->V != v) {
		ACTC_DEBUG(
		    fprintf(stderr, "ACTC::incVertexValence : Got vertex %d when "
			"looking for vertex %d?!?\n", vertex->V, v);
		    abortWithOptionalDump(tc);
		)
		return tc->Error = ACTC_DATABASE_CORRUPT;
	    }
	    vertex->Count++;
	} else {
	    chartedSetLabel("new Vertex");
	    vertex = (ACTCVertex *)malloc(sizeof(ACTCVertex));
	    vertex->V = v;
	    vertex->Count = 1;
	    vertex->Edges = NULL;
	    vertex->EdgeCount = 0;
	    if(tableInsert(v, tc->Vertices, vertex) == 0) {
		ACTC_DEBUG(fprintf(stderr, "ACTC::incVertexValence : Failed "
		    "to insert vertex into table\n");)
		return tc->Error = ACTC_ALLOC_FAILED;
	    }
	    tc->VertexCount++;
	}
    }
    if(vertex->Count > tc->CurMaxVertValence)
	tc->CurMaxVertValence = vertex->Count;

    *found = vertex;

    return ACTC_NO_ERROR;
}

static int decVertexValence(ACTCData *tc, ACTCVertex **vptr)
{
    ACTCVertex *v = *vptr;

    v->Count--;
    if(v->Count < 0) {
	ACTC_DEBUG(
	    fprintf(stderr, "ACTC::decVertexValence : Valence went "
		"negative?!?\n");
	    abortWithOptionalDump(tc);
	)
	return tc->Error = ACTC_DATABASE_CORRUPT;
    }
    
    if(v->PointsToMe != NULL) {
	*v->PointsToMe = v->Next;
	if(v->Next != NULL)
	    v->Next->PointsToMe = v->PointsToMe;
	v->Next = NULL;
    }

    if(v->Count == 0) {
	tc->VertexCount--;
	if(v->Edges != NULL)
	    free(v->Edges);
        if(!tc->UsingStaticVerts) {
	    tableRemove(v->V, tc->Vertices, NULL);
	    free(v);
	}
	*vptr = NULL;
    } else {
	if(tc->VertexBins != NULL) {
	    v->Next = tc->VertexBins[v->Count];
	    v->PointsToMe = &tc->VertexBins[v->Count];
	    if(v->Next != NULL)
		v->Next->PointsToMe = &v->Next;
	    tc->VertexBins[v->Count] = v;
	    if(v->Count < tc->CurMinVertValence)
		tc->CurMinVertValence = v->Count;
	}
    }

    return ACTC_NO_ERROR;
}

static int findNextFanVertex(ACTCData *tc, ACTCVertex **vert)
{
    if(tc->CurMaxVertValence < tc->MinFanVerts) {
	return ACTC_NO_MATCHING_VERT;
    }
    while(tc->VertexBins[tc->CurMaxVertValence] == NULL) {
	tc->CurMaxVertValence--;
	if(tc->CurMaxVertValence < tc->CurMinVertValence) {
	    if(tc->VertexCount > 0) {
		ACTC_DEBUG(fprintf(stderr, "tc::findNextFanVertex : no more "
		    "vertices in bins but VertexCount > 0\n");)
		return tc->Error = ACTC_DATABASE_CORRUPT;
	    }
	    return ACTC_NO_MATCHING_VERT;
	}
    }
    *vert = tc->VertexBins[tc->CurMaxVertValence];
    return ACTC_NO_ERROR;
}

static int findNextStripVertex(ACTCData *tc, ACTCVertex **vert)
{
    while(tc->VertexBins[tc->CurMinVertValence] == NULL) {
	tc->CurMinVertValence++;
	if(tc->CurMinVertValence > tc->CurMaxVertValence) {
	    if(tc->VertexCount > 0) {
		ACTC_DEBUG(fprintf(stderr, "tc::findNextStripVertex : no more "
		    "vertices in bins but VertexCount > 0\n");)
		return tc->Error = ACTC_DATABASE_CORRUPT;
	    }
	    return ACTC_NO_MATCHING_VERT;
	}
    }
    *vert = tc->VertexBins[tc->CurMinVertValence];
    return ACTC_NO_ERROR;
}

int actcGetIsDuringInput(ACTCData *tc) {
    return tc->IsInputting;
}

int actcBeginInput(ACTCData *tc)
{
    if(tc->IsOutputting) {
	ACTC_DEBUG(fprintf(stderr, "actcBeginInput : called within "
	    "BeginOutput/EndOutput\n");)
	return tc->Error = ACTC_DURING_INPUT;
    }

    if(tc->IsInputting) {
	ACTC_DEBUG(fprintf(stderr, "actcBeginInput : called within "
	    "BeginInput/EndInput\n");)
	return tc->Error = ACTC_DURING_INPUT;
    }

    tc->IsInputting = 1;
    tc->CurMaxVertValence = 0;

    if(tc->MaxInputVert < MAX_STATIC_VERTS - 1) {
	size_t byteCount;
	tc->UsingStaticVerts = 1;
	tc->VertRange = tc->MaxInputVert + 1;
	byteCount = sizeof(ACTCVertex) * tc->VertRange;
	chartedSetLabel("static verts");
	tc->StaticVerts = (ACTCVertex *)calloc(sizeof(ACTCVertex), tc->VertRange);
	if(tc->StaticVerts == NULL) {
	    ACTC_INFO(printf("Couldn't allocate static %d vert block of %u "
		"bytes\n", tc->VertRange, byteCount);)
	    tc->UsingStaticVerts = 0;
	}
    } else
	tc->UsingStaticVerts = 0;

    return ACTC_NO_ERROR;
}

int actcEndInput(ACTCData *tc)
{
    if(tc->IsOutputting) {
	ACTC_DEBUG(fprintf(stderr, "actcEndInput : called within "
	    "BeginOutput/EndOutput\n");)
	return tc->Error = ACTC_DURING_OUTPUT;
    }

    if(!tc->IsInputting) {
	ACTC_DEBUG(fprintf(stderr, "actcEndInput : called outside "
	    "BeginInput/EndInput\n");)
	return tc->Error = ACTC_IDLE;
    }

    tc->IsInputting = 0;

    return ACTC_NO_ERROR;
}

int actcGetIsDuringOutput(ACTCData *tc) {
    return tc->IsOutputting;
}

int actcBeginOutput(ACTCData *tc)
{
    ACTCVertex *v;
    int i;

    if(tc->IsInputting) {
	ACTC_DEBUG(fprintf(stderr, "actcBeginOutput : called within "
	    "BeginInput/EndInput\n");)
	return tc->Error = ACTC_DURING_INPUT;
    }

    if(tc->IsOutputting) {
	ACTC_DEBUG(fprintf(stderr, "actcBeginOutput : called within "
	    "BeginOutput/EndOutput\n");)
	return tc->Error = ACTC_DURING_OUTPUT;
    }

    tc->IsOutputting = 1;

    tc->CurMinVertValence = INT_MAX;
    chartedSetLabel("vertex bins");
    tc->VertexBins = (ACTCVertex **)calloc(sizeof(ACTCVertex *), tc->CurMaxVertValence + 1);
    if(tc->VertexBins == NULL) {
	ACTC_DEBUG(fprintf(stderr, "actcBeginOutput : couldn't allocate %d bytes "
	    "for Vertex Bins\n",
	    sizeof(ACTCVertex *) * tc->CurMaxVertValence);)
	return tc->Error = ACTC_ALLOC_FAILED;
    }

    if(tc->UsingStaticVerts) {
        double edgeTotal = 0;
	for(i = 0; i < (int)tc->VertRange; i++) {
	    v = &tc->StaticVerts[i];
	    if(v->Count > 0) {
		v->Next = tc->VertexBins[v->Count];
		v->PointsToMe = &tc->VertexBins[v->Count];
		tc->VertexBins[v->Count] = v;
		if(v->Next != NULL)
		    v->Next->PointsToMe = &v->Next;
		if(v->Count < tc->CurMinVertValence)
		    tc->CurMinVertValence = v->Count;
		edgeTotal += v->EdgeCount;
	    }
	}
    } else {
	tableResetIterator(tc->VertexIterator);
	for(i = 0; i < tc->VertexCount; i++) {
	    if(tableIterate(tc->Vertices, tc->VertexIterator, NULL, (void **)&v)
		== 0) {
		ACTC_DEBUG(fprintf(stderr, "actcBeginOutput : fewer vertices in "
		    "the table than we expected!\n");)
		return tc->Error = ACTC_DATABASE_CORRUPT;
	    }
	    v->Next = tc->VertexBins[v->Count];
	    v->PointsToMe = &tc->VertexBins[v->Count];
	    tc->VertexBins[v->Count] = v;
	    if(v->Next != NULL)
		v->Next->PointsToMe = &v->Next;
	    if(v->Count < tc->CurMinVertValence)
		tc->CurMinVertValence = v->Count;
	}
    }

    return ACTC_NO_ERROR;
}

int actcEndOutput(ACTCData *tc)
{
    if(tc->IsInputting) {
	ACTC_DEBUG(fprintf(stderr, "actcEndOutput : called within "
	    "BeginInput/EndInput\n");)
	return tc->Error = ACTC_DURING_INPUT;
    }

    if(!tc->IsOutputting) {
	ACTC_DEBUG(fprintf(stderr, "actcEndOutput : called outside "
	    "BeginOutput/EndOutput\n");)
	return tc->Error = ACTC_IDLE;
    }

    tc->IsOutputting = 0;

    if(tc->UsingStaticVerts) {
	free(tc->StaticVerts);
	tc->StaticVerts = NULL;
	tc->UsingStaticVerts = 0;
    }

    free(tc->VertexBins);
    tc->VertexBins = NULL;

    return ACTC_NO_ERROR;
}

ACTCData *actcNew(void)
{
    ACTCData *tc;
#if defined(DEBUG) || defined(INFO)
    static int didPrintVersion = 0;

    if(!didPrintVersion) {
	int verMinor, verMajor;
	didPrintVersion = 1;

        actcGetParami(tc, ACTC_MAJOR_VERSION, &verMajor);
        actcGetParami(tc, ACTC_MINOR_VERSION, &verMinor);
	fprintf(stderr, "TC Version %d.%d\n", verMajor, verMinor);
    }
#endif /* defined(DEBUG) || defined(INFO) */

    chartedSetLabel("the tc struct");
    tc = (ACTCData *)calloc(sizeof(*tc), 1);

    if(tc == NULL) {
	ACTC_DEBUG(fprintf(stderr, "actcNew : couldn't allocate %d bytes "
	    "for new ACTCData\n", sizeof(*tc));)
	return NULL;
    }

    tc->Vertices = tableNew();
    tc->VertexIterator = tableNewIterator();

    tc->MinFanVerts = INT_MAX;
    tc->MaxPrimVerts = INT_MAX;
    tc->MaxInputVert = INT_MAX;
    tc->MaxEdgeShare = INT_MAX;
    tc->MaxVertShare = INT_MAX;
    tc->HonorWinding = 1;
    /* seed = 0 handled by calloc */
    /* XXX grantham 20000615 - seed ignored for now */

    return tc;
}

static size_t allocatedForTriangles(ACTCEdge *e)
{
    return sizeof(ACTCTriangle) * e->TriangleCount;
}

static size_t allocatedForEdges(ACTCVertex *vert)
{
    int i;
    size_t size;

    size = sizeof(ACTCEdge) * vert->EdgeCount;
    for(i = 0; i < vert->EdgeCount; i++) {
	size += allocatedForTriangles(&vert->Edges[i]);
    }
    return size;
}

static size_t allocatedForVertices(ACTCData *tc)
{
    int i;
    int size = 0;
    ACTCVertex *v;

    if(!tc->UsingStaticVerts)
        tableResetIterator(tc->VertexIterator);

    if(tc->UsingStaticVerts) {
        size = tc->VertRange * sizeof(ACTCVertex);
        for(i = 0; i < (int)tc->VertRange; i++) {
	    v = &tc->StaticVerts[i];
	    if(v->Count > 0)
	        size += allocatedForEdges(v);
	}
    } else {
	for(i = 0; i < tc->VertexCount; i++) {
	    tableIterate(tc->Vertices, tc->VertexIterator, NULL, (void **)&v);
	    size += allocatedForEdges(v);
	}
    }
    return size;
}

int actcGetMemoryAllocation(ACTCData *tc, size_t *bytesAllocated)
{
    size_t tableBytes;

    tableGetStats(tc->Vertices, NULL, NULL, &tableBytes);
    *bytesAllocated = sizeof(ACTCData);
    *bytesAllocated += tableBytes;
    *bytesAllocated += allocatedForVertices(tc); /* recurses */

    return ACTC_NO_ERROR;
}

static void freeVertex(void *p)
{
    ACTCVertex *v = (ACTCVertex *)p;
    int i;

    for(i = 0; i < v->EdgeCount; i++)
        free(v->Edges[i].Triangles);
    free(v->Edges);
    free(v);
}

int actcMakeEmpty(ACTCData *tc)
{
    tc->VertexCount = 0;
    if(!tc->UsingStaticVerts)
        tableDelete(tc->Vertices, freeVertex);
    if(tc->VertexBins != NULL) {
        free(tc->VertexBins);
	tc->VertexBins = NULL;
    }
    tc->IsOutputting = 0;
    tc->IsInputting = 0;
    return ACTC_NO_ERROR;
}

void actcDelete(ACTCData *tc)
{
    actcMakeEmpty(tc);
    free(tc->VertexIterator);
    free(tc->Vertices);
    free(tc);
}

int actcParami(ACTCData *tc, int param, int value)
{
    if(tc->IsInputting) {
	ACTC_DEBUG(fprintf(stderr, "actcParami : within BeginInput/"
	    "EndInput\n");)
	return tc->Error = ACTC_DURING_INPUT;
    }
    if(tc->IsOutputting) {
	ACTC_DEBUG(fprintf(stderr, "actcParami : within BeginOutput/"
	    "EndOutput\n");)
	return tc->Error = ACTC_DURING_OUTPUT;
    }
    switch(param) {
	case ACTC_OUT_MIN_FAN_VERTS:
	    tc->MinFanVerts = value;
	    break;

	case ACTC_IN_MAX_VERT:
	    if(value < (int)tc->MinInputVert) {
		ACTC_DEBUG(fprintf(stderr, "actcParami : tried to set "
		    "MAX_INPUT_VERT to %d, less than MIN_INPUT_VERT (%d)\n",
		    value, tc->MinInputVert);)
		return tc->Error = ACTC_INVALID_VALUE;
	    }
	    tc->MaxInputVert = value;
	    break;

	case ACTC_IN_MIN_VERT:
	    if(value > (int)tc->MaxInputVert) {
		ACTC_DEBUG(fprintf(stderr, "actcParami : tried to set "
		    "MIN_INPUT_VERT to %d, greater than MAX_INPUT_VERT (%d)\n",
		    value, tc->MaxInputVert);)
		return tc->Error = ACTC_INVALID_VALUE;
	    }
	    tc->MinInputVert = value;
	    break;

	case ACTC_IN_MAX_EDGE_SHARING:
	    tc->MaxEdgeShare = value;
	    break;

	case ACTC_IN_MAX_VERT_SHARING:
	    tc->MaxVertShare = value;
	    break;

	case ACTC_OUT_HONOR_WINDING:
	    tc->HonorWinding = value;
	    break;

	case ACTC_OUT_MAX_PRIM_VERTS:
	    if(value < 3) {
		ACTC_DEBUG(fprintf(stderr, "actcParami : tried to set "
		    "MAX_PRIM_VERTS to %d (needed to be 3 or more)\n", value);)
		return tc->Error = ACTC_INVALID_VALUE;
	    }
	    tc->MaxPrimVerts = value;
	    break;

    }
    return ACTC_NO_ERROR;
}

int actcParamu(ACTCData *tc, int param, unsigned value)
{
    /*
     * XXX - yes, this is questionable, but I consulted industry
     * experts and we agreed that most common behavior is to copy the
     * bits directly, which is what I want.
     */
    return actcParami(tc, param, (int)value);
}

int actcGetParami(ACTCData *tc, int param, int *value)
{
    switch(param) {
	case ACTC_MAJOR_VERSION:
	    *value = 1;
	    break;

	case ACTC_MINOR_VERSION:
	    *value = 1;
	    break;

	case ACTC_IN_MAX_VERT:
	    *value = tc->MaxInputVert;
	    break;

	case ACTC_IN_MIN_VERT:
	    *value = tc->MinInputVert;
	    break;

	case ACTC_IN_MAX_EDGE_SHARING:
	    *value = tc->MaxEdgeShare;
	    break;

	case ACTC_IN_MAX_VERT_SHARING:
	    *value = tc->MaxVertShare;
	    break;

	case ACTC_OUT_MIN_FAN_VERTS:
	    *value = tc->MinFanVerts;
	    break;

	case ACTC_OUT_HONOR_WINDING:
	    *value = tc->HonorWinding;
	    break;

	case ACTC_OUT_MAX_PRIM_VERTS:
	    *value = tc->MaxPrimVerts;
	    break;

	default:
	    *value = 0;
	    return tc->Error = ACTC_INVALID_VALUE;
	    /* break; */
    }
    return ACTC_NO_ERROR;
}

int actcGetParamu(ACTCData *tc, int param, unsigned *value)
{
    return actcGetParami(tc, param, (int *)value);
}

static int mapEdgeTriangle(ACTCData *tc, ACTCEdge *edge, ACTCVertex *v3)
{
    ACTCTriangle tmp;
    void *r;

    tmp.FinalVert = v3;
    chartedSetLabel("triangle list");
    r = reallocAndAppend((void **)&edge->Triangles, &edge->TriangleCount,
        sizeof(tmp), &tmp);
    if(r == NULL) {
	ACTC_DEBUG(fprintf(stderr, "ACTC::mapEdgeTriangle : Couldn't allocate "
	    "%d bytes for triangles\n", sizeof(tmp) *
	    (edge->TriangleCount + 1));)
	return tc->Error = ACTC_ALLOC_FAILED;
    }

    return ACTC_NO_ERROR;
}

static int unmapEdgeTriangle(ACTCData *tc, ACTCEdge *edge, ACTCVertex *v3)
{
    int i;

    for(i = 0; i < edge->TriangleCount; i++)
	if(edge->Triangles[i].FinalVert == v3)
	    break;

    if(i == edge->TriangleCount) {
	ACTC_DEBUG(
	    fprintf(stderr, "ACTC::unmapEdgeTriangle : Couldn't find third vertex"
	        " from edge in order to delete it?!?\n");
	    abortWithOptionalDump(tc);
	)
	return tc->Error = ACTC_DATABASE_CORRUPT;
    }

    edge->Triangles[i] = edge->Triangles[edge->TriangleCount - 1];
    edge->TriangleCount --;

    return ACTC_NO_ERROR;
}

static int mapVertexEdge(ACTCData *tc, ACTCVertex *v1, ACTCVertex *v2, ACTCEdge **edge)
{
    unsigned i;
    ACTCEdge tmp;
    void *r;

    for(i = 0; (int)i < v1->EdgeCount; i++)
        if(v1->Edges[i].V2 == v2) {
	    v1->Edges[i].Count++;
	    break;
	}

    if((int)i == v1->EdgeCount) {

	tmp.V2 = v2;
	tmp.Count = 1;
	tmp.Triangles = NULL;
	tmp.TriangleCount = 0;

	chartedSetLabel("vert-to-edge mapping");
	r = reallocAndAppend((void **)&v1->Edges, &v1->EdgeCount,
	    sizeof(tmp), &tmp);
	if(r == NULL) {
	    ACTC_DEBUG(fprintf(stderr, "ACTC::mapVertexEdge : Couldn't reallocate "
	        "to %d bytes for vertex's edge list\n", sizeof(tmp) *
		v1->EdgeCount);)
	    return tc->Error = ACTC_ALLOC_FAILED;
	}
    }
    *edge = &v1->Edges[i];

    return ACTC_NO_ERROR;
}

static int unmapVertexEdge(ACTCData *tc, ACTCVertex *v1, ACTCVertex *v2)
{
    int i;

    for(i = 0; i < v1->EdgeCount; i++)
	if(v1->Edges[i].V2 == v2)
	    break;

    if(i == v1->EdgeCount) {
	ACTC_DEBUG(
	    fprintf(stderr, "ACTC::unmapVertexEdge : Couldn't find edge %d,%d"
	        " from vertex in order to unmap it?!?\n", v1->V, v2->V);
	    abortWithOptionalDump(tc);
	)
	return tc->Error = ACTC_DATABASE_CORRUPT;
    }

    v1->Edges[i].Count --;
    if(v1->Edges[i].Count == 0) {
        if(v1->Edges[i].Triangles != NULL)
	    free(v1->Edges[i].Triangles);
	v1->Edges[i] = v1->Edges[v1->EdgeCount - 1];
	v1->EdgeCount --;
    }

    return ACTC_NO_ERROR;
}

int actcAddTriangle(ACTCData *tc, unsigned v1, unsigned v2, unsigned v3)
{
    ACTCVertex *vertexRec1;
    ACTCVertex *vertexRec2;
    ACTCVertex *vertexRec3;

    ACTCEdge *edge12;
    ACTCEdge *edge23;
    ACTCEdge *edge31;

    if(tc->IsOutputting) {
	ACTC_DEBUG(fprintf(stderr, "actcAddTriangle : inside "
	    "BeginOutput/EndOutput\n");)
	return tc->Error = ACTC_IDLE;
    }
    if(!tc->IsInputting) {
	ACTC_DEBUG(fprintf(stderr, "actcAddTriangle : outside "
	    "BeginInput/EndInput\n");)
	return tc->Error = ACTC_DURING_INPUT;
    }

    if(incVertexValence(tc, v1, &vertexRec1) != ACTC_NO_ERROR) goto returnError1;
    if(incVertexValence(tc, v2, &vertexRec2) != ACTC_NO_ERROR) goto free1;
    if(incVertexValence(tc, v3, &vertexRec3) != ACTC_NO_ERROR) goto free2;

    if(mapVertexEdge(tc, vertexRec1, vertexRec2, &edge12) != ACTC_NO_ERROR)
        goto free3;
    if(mapVertexEdge(tc, vertexRec2, vertexRec3, &edge23) != ACTC_NO_ERROR)
        goto free4;
    if(mapVertexEdge(tc, vertexRec3, vertexRec1, &edge31) != ACTC_NO_ERROR)
        goto free5;

    if(mapEdgeTriangle(tc, edge12, vertexRec3) != ACTC_NO_ERROR) goto free6;
    if(mapEdgeTriangle(tc, edge23, vertexRec1) != ACTC_NO_ERROR) goto free7;
    if(mapEdgeTriangle(tc, edge31, vertexRec2) != ACTC_NO_ERROR) goto free8;

    return ACTC_NO_ERROR;

    /*
     * XXX Unfortunately, while backing out during the following
     * statements, we might encounter errors in the database which
     * will not get returned properly to the caller; I take heart in
     * the fact that if such an error occurs, TC is just a moment from
     * core dumping anyway. XXX grantham 20000615
     */

free8:
    unmapEdgeTriangle(tc, edge23, vertexRec1);
free7:
    unmapEdgeTriangle(tc, edge12, vertexRec3);
free6:
    unmapVertexEdge(tc, vertexRec3, vertexRec1);
free5:
    unmapVertexEdge(tc, vertexRec2, vertexRec3);
free4:
    unmapVertexEdge(tc, vertexRec1, vertexRec2);
free3:
    decVertexValence(tc, &vertexRec3);
free2:
    decVertexValence(tc, &vertexRec2);
free1:
    decVertexValence(tc, &vertexRec1);
returnError1:
    return tc->Error;
}

int actcStartNextPrim(ACTCData *tc, unsigned *v1Return, unsigned *v2Return)
{
    ACTCVertex *v1 = NULL;
    ACTCVertex *v2 = NULL;
    int findResult;

    if(tc->IsInputting) {
	ACTC_DEBUG(fprintf(stderr, "actcStartNextPrim : within "
	    "BeginInput/EndInput\n");)
	return tc->Error = ACTC_DURING_INPUT;
    }
    if(!tc->IsOutputting) {
	ACTC_DEBUG(fprintf(stderr, "actcStartNextPrim : outside "
	    "BeginOutput/EndOutput\n");)
	return tc->Error = ACTC_IDLE;
    }

    findResult = findNextFanVertex(tc, &v1);
    if(findResult == ACTC_NO_ERROR)
	tc->PrimType = ACTC_PRIM_FAN;
    else if(findResult != ACTC_NO_MATCHING_VERT) {
	ACTC_DEBUG(fprintf(stderr, "actcStartNextPrim : internal "
	    "error finding next appropriate vertex\n");)
	return tc->Error = findResult;
    } else {
	findResult = findNextStripVertex(tc, &v1);
	if(findResult != ACTC_NO_ERROR && findResult != ACTC_NO_MATCHING_VERT) {
	    ACTC_DEBUG(fprintf(stderr, "actcStartNextPrim : internal "
		"error finding next appropriate vertex\n");)
	    return tc->Error = findResult;
	}
	tc->PrimType = ACTC_PRIM_STRIP;
    }

    if(findResult == ACTC_NO_MATCHING_VERT) {
	*v1Return = -1;
	*v2Return = -1;
	return tc->Error = ACTC_DATABASE_EMPTY;
    }

    v2 = v1->Edges[0].V2;

    tc->CurWindOrder = ACTC_FWD_ORDER;
    tc->VerticesSoFar = 2;

    tc->V1 = v1;
    tc->V2 = v2;

    *v1Return = v1->V;
    *v2Return = v2->V;

    ACTC_INFO(printf("starting with edge %u, %u\n", tc->V1->V, tc->V2->V);)

    return tc->PrimType;
}

static int findEdge(ACTCVertex *v1, ACTCVertex *v2, ACTCEdge **edge)
{
    int i;

    for(i = 0; i < v1->EdgeCount; i++)
        if(v1->Edges[i].V2 == v2) {
	    *edge = &v1->Edges[i];
	    return 1;
	}
    return 0;
}

int unmapEdgeTriangleByVerts(ACTCData *tc, ACTCVertex *v1, ACTCVertex *v2,
    ACTCVertex *v3)
{
    ACTCEdge *e;

    ACTC_CHECK(findEdge(v1, v2, &e));
    unmapEdgeTriangle(tc, e, v3);
    return ACTC_NO_ERROR;
}

int actcGetNextVert(ACTCData *tc, unsigned *vertReturn)
{
    ACTCEdge *edge;
    int wasEdgeFound = 0;
    ACTCVertex *thirdVertex;
    int wasFoundReversed;

    if(tc->IsInputting) {
	ACTC_DEBUG(fprintf(stderr, "actcGetNextVert : within BeginInput/"
	    "EndInput\n");)
	return tc->Error = ACTC_DURING_INPUT;
    }
    if(!tc->IsOutputting) {
	ACTC_DEBUG(fprintf(stderr, "actcGetNextVert : outside BeginOutput/"
	    "EndOutput\n");)
	return tc->Error = ACTC_IDLE;
    }
    if(tc->PrimType == -1) {
	ACTC_DEBUG(fprintf(stderr, "actcGetNextVert : Asked for next vertex "
	    "without a primitive (got last\n    vertex already?)\n");)
	return tc->Error = ACTC_INVALID_VALUE;
    }

    if(tc->VerticesSoFar >= tc->MaxPrimVerts) {
	tc->PrimType = -1;
	return tc->Error = ACTC_PRIM_COMPLETE;
    }

    if(tc->V1 == NULL || tc->V2 == NULL) {
	tc->PrimType = -1;
	return tc->Error = ACTC_PRIM_COMPLETE;
    }

    ACTC_INFO(printf("looking for edge %u, %u\n", tc->V1->V, tc->V2->V);)

    wasFoundReversed = 0;

    if(findEdge(tc->V1, tc->V2, &edge) != 0) {
	wasEdgeFound = 1;
    } else if(!tc->HonorWinding) {
	wasFoundReversed = 1;
	if(findEdge(tc->V2, tc->V1, &edge) != 0) {
	    wasEdgeFound = 1;
	}
    }

    if(!wasEdgeFound) {
	tc->PrimType = -1;
	return tc->Error = ACTC_PRIM_COMPLETE;
    }

    thirdVertex = edge->Triangles[edge->TriangleCount - 1].FinalVert;

    ACTC_INFO(printf("third vertex = %u\n", thirdVertex->V);)
    *vertReturn = thirdVertex->V;

    if(wasFoundReversed) {
	ACTC_CHECK(unmapEdgeTriangle(tc, edge, thirdVertex));
	ACTC_CHECK(unmapEdgeTriangleByVerts(tc, tc->V1, thirdVertex, tc->V2));
	ACTC_CHECK(unmapEdgeTriangleByVerts(tc, thirdVertex, tc->V2, tc->V1));
	ACTC_CHECK(unmapVertexEdge(tc, tc->V2, tc->V1));
	ACTC_CHECK(unmapVertexEdge(tc, tc->V1, thirdVertex));
	ACTC_CHECK(unmapVertexEdge(tc, thirdVertex, tc->V2));
    } else {
	ACTC_CHECK(unmapEdgeTriangle(tc, edge, thirdVertex));
	ACTC_CHECK(unmapEdgeTriangleByVerts(tc, tc->V2, thirdVertex, tc->V1));
	ACTC_CHECK(unmapEdgeTriangleByVerts(tc, thirdVertex, tc->V1, tc->V2));
	ACTC_CHECK(unmapVertexEdge(tc, tc->V1, tc->V2));
	ACTC_CHECK(unmapVertexEdge(tc, tc->V2, thirdVertex));
	ACTC_CHECK(unmapVertexEdge(tc, thirdVertex, tc->V1));
    }
    ACTC_CHECK(decVertexValence(tc, &tc->V1));
    ACTC_CHECK(decVertexValence(tc, &tc->V2));
    ACTC_CHECK(decVertexValence(tc, &thirdVertex));

    if(tc->PrimType == ACTC_PRIM_FAN) {
        tc->V2 = thirdVertex;
    } else /* PRIM_STRIP */ {
	if(tc->CurWindOrder == ACTC_FWD_ORDER)
	    tc->V1 = thirdVertex;
	else
	    tc->V2 = thirdVertex;
	tc->CurWindOrder = !tc->CurWindOrder;
    }

    tc->VerticesSoFar++;
    return ACTC_NO_ERROR;
}

int actcTrianglesToPrimitives(ACTCData *tc, int triangleCount,
    unsigned (*triangles)[3], int primTypes[], int primLengths[], unsigned vertices[],
    int maxBatchSize)
{
    int r;
    int curTriangle;
    int curPrimitive;
    unsigned curVertex;
    int prim;
    unsigned v1, v2, v3;
    int lastPrim;
    int passesWithoutPrims;
    int trisSoFar;

    if(tc->IsInputting) {
	ACTC_DEBUG(fprintf(stderr, "actcTrianglesToPrimitives : within BeginInput/"
	    "EndInput\n");)
	return tc->Error = ACTC_DURING_INPUT;
    }
    if(tc->IsOutputting) {
	ACTC_DEBUG(fprintf(stderr, "actcTrianglesToPrimitives : within"
	    "BeginOutput/EndOutput\n");)
	return tc->Error = ACTC_DURING_OUTPUT;
    }
    curTriangle = 0;
    curPrimitive = 0;
    curVertex = 0;
    passesWithoutPrims = 0;

    actcMakeEmpty(tc);

    ACTC_CHECK(actcBeginInput(tc));
    trisSoFar = 0;
    while(curTriangle < triangleCount) {
	r = actcAddTriangle(tc, triangles[curTriangle][0],
	    triangles[curTriangle][1], triangles[curTriangle][2]);
	trisSoFar++;
	curTriangle++;
	if((trisSoFar >= maxBatchSize) ||
	    (r == ACTC_ALLOC_FAILED && curTriangle != triangleCount) ||
	    (r == ACTC_NO_ERROR && curTriangle == triangleCount)) {

	    /* drain what we got */
	    trisSoFar = 0;
	    ACTC_CHECK(actcEndInput(tc));
	    ACTC_CHECK(actcBeginOutput(tc));
	    lastPrim = curPrimitive;
	    while((prim = actcStartNextPrim(tc, &v1, &v2)) != ACTC_DATABASE_EMPTY) {
		ACTC_CHECK(prim);
		primTypes[curPrimitive] = prim;
		primLengths[curPrimitive] = 2;
		vertices[curVertex++] = v1;
		vertices[curVertex++] = v2;
		while((r = actcGetNextVert(tc, &v3)) != ACTC_PRIM_COMPLETE) {
		    ACTC_CHECK(r);
		    vertices[curVertex++] = v3;
		    primLengths[curPrimitive]++;
		}
		curPrimitive++;
	    }
	    ACTC_CHECK(actcEndOutput(tc));
	    if(r == ACTC_ALLOC_FAILED && curPrimitive == lastPrim) {
	        if(passesWithoutPrims == 0) {
		    /* not enough memory to add a triangle and */
		    /* nothing in the database, better free everything */
		    /* and try again */
		    actcMakeEmpty(tc);
		} else {
		    /* cleaned up and STILL couldn't get a triangle in; */
		    /* give up */
		    return tc->Error = ACTC_ALLOC_FAILED;
		}
		passesWithoutPrims++;
	    }
	    ACTC_CHECK(actcBeginInput(tc));
	} else
	    ACTC_CHECK(r);
	if(r == ACTC_ALLOC_FAILED)
	    curTriangle--;
    }
    ACTC_CHECK(actcEndInput(tc));

    actcMakeEmpty(tc);

    return curPrimitive;
}

/* vi:tabstop=8
 */
