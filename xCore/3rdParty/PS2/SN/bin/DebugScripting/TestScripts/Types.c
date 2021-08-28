//****************************************************************************
//
// Copyright SN Systems Ltd 2003
//
// Description: An example of the use of callbacks.
//                                                                          
//****************************************************************************
//
// A customer's custom type displays that I have nicked
// because they are a nice example of the use of callbacks.
//
// Unfortunately everyone's base classes are probably different so
// it's unlikely anyone can use this as-is but it does show what sort
// of things can be done.
//
#include <stdio.h>
#include <string.h>
#include <SNscript.h>

typedef	union {int i; float f;}		itof;
typedef struct {itof x, y, z, w;}	vector;

static char	vector_out[256];

char *Subfield(char* pexpr, char *subfield)
{
	static char		exp[256];
	if (pexpr == exp)
		strcat(exp, subfield);
	else
		sprintf(exp, "(%s)%s", pexpr, subfield);
	return exp;
}

void GetVector(vector *v, int unitnum, char* pexpr)
{
	char		*pexperr;
   	snval_t		result;

	pexperr = SNEvaluate(&result, unitnum, Subfield(pexpr, ".v"));
	*v		= *(vector*)&result.val;
}

char* VectorHandler(int unitnum, char* pexpr)
{
	vector		v;
	GetVector(&v, unitnum, pexpr);
	sprintf(vector_out, "(%+9g, %+9g, %+9g, %+9g)", v.x.f, v.y.f, v.z.f, v.w.f);
	return vector_out;
}
char* VectorXHandler(int unitnum, char* pexpr)
{
	vector		v;
	GetVector(&v, unitnum, pexpr);
	sprintf(vector_out, "%g", v.x.f);
	return vector_out;
}
char* VectorYHandler(int unitnum, char* pexpr)
{
	vector		v;
	GetVector(&v, unitnum, pexpr);
	sprintf(vector_out, "%g", v.y.f);
	return vector_out;
}
char* VectorZHandler(int unitnum, char* pexpr)
{
	vector		v;
	GetVector(&v, unitnum, pexpr);
	sprintf(vector_out, "%g", v.z.f);
	return vector_out;
}
char* VectorWHandler(int unitnum, char* pexpr)
{
	vector		v;
	GetVector(&v, unitnum, pexpr);
	sprintf(vector_out, "%g", v.w.f);
	return vector_out;
}
char* VectorXYHandler(int unitnum, char* pexpr)
{
	vector		v;
	GetVector(&v, unitnum, pexpr);
	sprintf(vector_out, "(%+9g, %+9g)", v.x.f, v.y.f);
	return vector_out;
}
char* VectorXZHandler(int unitnum, char* pexpr)
{
	vector		v;
	GetVector(&v, unitnum, pexpr);
	sprintf(vector_out, "(%+9g, %+9g)", v.x.f, v.z.f);
	return vector_out;
}
char* VectorXYZHandler(int unitnum, char* pexpr)
{
	vector		v;
	GetVector(&v, unitnum, pexpr);
	sprintf(vector_out, "(%+9g, %+9g, %+9g)", v.x.f, v.y.f, v.z.f);
	return vector_out;
}

char* CuboidHandler(int unitnum, char* pexpr)
{
	vector		vmin, vmax;

	GetVector(&vmin, unitnum, Subfield(pexpr, ".min"));
	GetVector(&vmax, unitnum, Subfield(pexpr, ".max"));
	
	sprintf(vector_out, "(%+6.3g, %+6.3g, %+6.3g)-(%+6.3g, %+6.3g, %+6.3g)", vmin.x.f, vmin.y.f, vmin.z.f, vmax.x.f, vmax.y.f, vmax.z.f);

	return vector_out;
}

char* COLLTYPE(int i)
{
	static char	*COLLTYPES[]= {"COLL_END", "COLL_BOX", "COLL_PLANES", "COLL_SOUP", "COLL_SPHERE", "Unknown collision type"};
	return COLLTYPES[i > 4 ? 5 : i];
}
char* COLLISIONHandler(int unitnum, char* pexpr)
{
   	snval_t		result;
	char		*pexperr	= SNEvaluate(&result, unitnum, Subfield(pexpr, ".type"));
	return COLLTYPE(result.val.u16);
}
char* PCOLLISIONHandler(int unitnum, char* pexpr)
{
   	snval_t		result;
	char		*pexperr	= SNEvaluate(&result, unitnum, Subfield(pexpr, "->type"));
	return COLLTYPE(result.val.u16);
}

char*	MyIntH(int unitnum, char* pexpr)
{
	return pexpr;
}

int main(int argc, char** argv)
{
	SNSetTypeDisplay("int",					":MyIntH"				);
	SNSetTypeDisplay("class VectorX",		":VectorXHandler"		);
	SNSetTypeDisplay("class VectorY",		":VectorYHandler"		);
	SNSetTypeDisplay("class VectorZ",		":VectorZHandler"		);
	SNSetTypeDisplay("class VectorW",		":VectorWHandler"		);
	SNSetTypeDisplay("class VectorXY",		":VectorXYHandler"		);
	SNSetTypeDisplay("class VectorXZ",		":VectorXZHandler"		);
	SNSetTypeDisplay("class VectorXYZ",		":VectorXYZHandler"		);
	SNSetTypeDisplay("class Vector",		":VectorHandler"		);
	SNSetTypeDisplay("class VectorDir",		":VectorXYZHandler"		);
	SNSetTypeDisplay("class VectorPos",		":VectorXYZHandler"		);
	SNSetTypeDisplay("class Quaternion",	":VectorHandler"		);
	SNSetTypeDisplay("class Cuboid",		":CuboidHandler"		);
	SNSetTypeDisplay("class COLLISION",		":COLLISIONHandler"		);
	SNSetTypeDisplay("class COLLISION*",	":PCOLLISIONHandler"	);
	SNSetTypeDisplay("class COLOUR",		"(%i{@.r}, %i{@.g}, %i{@.b}, %i{@.a})");

	return 0;
}
