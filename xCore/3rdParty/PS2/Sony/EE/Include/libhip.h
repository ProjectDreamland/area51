/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
*/
/*
 *                      Emotion Engine Library
 *
 *      Copyright (C) 2000 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                         libhip - libhip.h
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *                      Sep,19,2000     kaneko
 */
/*	$Id: libhip.h,v 1.30 2003/10/06 11:02:19 masae Exp $	*/

#ifndef __HIP_H__
#define __HIP_H__
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __HiG_H__
#include <libhig.h>
#endif

/*********************************************************************************
 *				CONSTANTS
 *********************************************************************************/

/* Micro Code ID for micro plug*/
typedef enum {
    SCE_HIP_MICRO_ATTR_NONE	= 0,
    SCE_HIP_MICRO_ATTR_FGE	= (1 << 0),
    SCE_HIP_MICRO_ATTR_ANTI	= (1 << 1)
} sceHiPlugMicroAttr_t;

/*********************************************************************************
 *				STRUCTURES
 *********************************************************************************/

typedef struct {
    u_int	*micro;
    u_int	attr;
} sceHiPlugMicroTbl_t;

typedef struct {
    sceHiPlugMicroTbl_t	*tbl;
    u_int		tblnum;
} sceHiPlugMicroInitArg_t;

typedef struct {
    int		micro;	/* The microcode which you want to activate */
    float	anticutoff;	/* anti-alias parameter */
    float	fogbegin;     	/* fog begin */
    float	fogend;		/* fog end */
} sceHiPlugMicroPreCalcArg_t;

typedef struct {
    int			resident;	/* for resident */
    sceHiGsMemTbl	*tbl;		/* for tbp & cbp */
}sceHiPlugTex2dInitArg_t;

typedef struct {
    int			resident;	/* for resident */
    sceHiGsMemTbl	*tbl;		/* for tbp & cbp */
}sceHiPlugTim2InitArg_t;

typedef struct {
    u_int	setframe;		/* force frame count */
    int		setframe_enable;	/* enable setframe value */
    u_int	currentframe;		/* return current frame cocunt */
} sceHiPlugAnimePreCalcArg_t;

typedef struct {
    sceVu0FMATRIX *root;
} sceHiPlugHrchyPreCalcArg_t;

/*		ClutBump		*/
typedef struct {
    sceVu0FVECTOR               light_dir;   
    sceVu0FVECTOR		shading;
} sceHiPlugClutBumpPreArg_t;

/*		ShadowMap		*/
typedef struct {
    int			width,height;
    u_int		*box;		/* shadowobj bounding box */
}sceHiPlugShadowMapInitArg_t;

/*		LightMap		*/
typedef struct {
    int			width,height;
    int			fov;
}sceHiPlugLightMapInitArg_t;

/*		FishEye			*/
typedef struct {
    u_int zdepth;
    float rmin;
    float rmax;
} sceHiPlugFishEyeInitArg_t;

typedef struct {
    sceVu0FVECTOR               *camera_pos;
    sceVu0FVECTOR               *camera_zdir;
    sceVu0FVECTOR               *camera_up;
    float tex_size;
} sceHiPlugFishEyePreArg_t;

/*		Reflect			*/
typedef struct {
    sceVu0FVECTOR               *camera_pos;
    sceVu0FVECTOR               *camera_zdir;
    sceVu0FVECTOR               *camera_up;
    float                       zoom;
    float                       z_shift;
} sceHiPlugReflectPreArg_t;

/*		Refract			*/
typedef struct {
    sceVu0FVECTOR               *camera_pos;
    sceVu0FVECTOR               *camera_zdir;
    sceVu0FVECTOR               *camera_up;
    float                       refract_index;
    float                       zoom;
    float                       z_shift;
} sceHiPlugRefractPreArg_t;

/*********************************************************************************
 *				FUNCTIONS
 *********************************************************************************/

/*	plugin functions	*/
extern sceHiErr sceHiPlugMicro(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugTex2D(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugTim2 (sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugShape(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugHrchy(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugAnime(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugShare(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugClutBump(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugShadowMap(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugShadowBox(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugLightMap(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugFishEye(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugReflect(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugRefract(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugSkin(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugClip(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugCamera(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugMatrix(sceHiPlug *plug, int process);
extern sceHiErr sceHiPlugManime(sceHiPlug *plug, int process);

/*	plugin services		*/ 
extern size_t sceHiPlugTex2DSize(sceHiPlug *plug);
extern int	sceHiPlugTim2Num(sceHiPlug *plug);
extern char	*sceHiPlugTim2GetName(sceHiPlug *plug, int idx);
extern sceHiErr sceHiPlugTim2SetData(sceHiPlug *plug, int idx, u_int *fdata);
extern sceHiErr sceHiPlugTim2GetNPictures(sceHiPlug *plug, int n, int *num);
extern sceHiErr sceHiPlugTim2SetPicture(sceHiPlug *plug, int n, int num);
extern sceHiErr sceHiPlugShapeInvisible(sceHiPlug *plug, int matidx, int flag);
extern sceHiErr sceHiPlugShapeMasterChainSetting(sceHiPlug *, int flag);
enum { /* the bellow use as flag in sceHiPlugShapeMasterChainSetting() args */
    SCE_HIP_SHAPE_MASTER_CHAIN_IN_STATIC_O = (1 << 0)
};

/*********************************************************************************
 *				ACCESSOR
 *********************************************************************************/
typedef union {
    struct {
	int	id;		/* geometry id */
	size_t	size;		/* geometry word size */
	u_int	prim;		/* prim register bits */
	int	num;		/* num of prim */
    }geo;
    struct {
	int	id;		/* material id */
	int   	num;		/* num of geometry */
	int	tex_id;		/* texture id */
	int	tex_num;	/* num of texture */
    }mat;
    struct {
	int	id;		/* shape id */
	size_t	size;		/* shape word size */
	int	reserve;	/* reserve */
	int	num;		/* num of material */
    }dat;
    struct {
	int	reserve[3];	/* reserve */
	int	num;		/* num of shape */
    }top;
}sceHiPlugShapeHead_t;

typedef struct {
    int			reserve[2];	/* reserve */
    int			flags;		/* visible/invisible flag */
    int			shape;		/* shape id */
    sceVu0FMATRIX	local;		/* local world matrix */
    sceVu0FMATRIX	light;		/* light rotation matrix */
}sceHiPlugShapeMatrix_t;

extern sceHiPlugShapeHead_t *sceHiPlugShapeGetHead(sceHiPlug *p, sceHiType t);
extern sceHiPlugShapeHead_t *sceHiPlugShapeGetDataHead(sceHiPlugShapeHead_t *h, int idx);
extern sceHiPlugShapeHead_t *sceHiPlugShapeGetMaterialHead(sceHiPlugShapeHead_t *h, int idx);
extern sceHiPlugShapeHead_t *sceHiPlugShapeGetGeometryHead(sceHiPlugShapeHead_t *h, int idx);
extern sceHiGsGiftag *sceHiPlugShapeGetMaterialGiftag(sceHiPlugShapeHead_t *h);
extern sceVu0FMATRIX *sceHiPlugShapeGetMaterialAttrib(sceHiPlugShapeHead_t *h);
extern sceVu0FVECTOR *sceHiPlugShapeGetGeometryVertex(sceHiPlugShapeHead_t *h, int idx);
extern sceVu0FVECTOR *sceHiPlugShapeGetGeometryNormal(sceHiPlugShapeHead_t *h, int idx);
extern sceVu0FVECTOR *sceHiPlugShapeGetGeometryST(sceHiPlugShapeHead_t *h, int idx);
extern sceVu0FVECTOR *sceHiPlugShapeGetGeometryColor(sceHiPlugShapeHead_t *h, int idx);
extern sceHiPlugShapeMatrix_t *sceHiPlugShapeGetMatrix(sceHiPlugShapeHead_t *h, int idx);

typedef struct {
    int			reserve[2];	/* reserve */
    u_int		rorder;		/* rotation order */
    int			num;		/* num of data */
}sceHiPlugHrchyHead_t;

typedef struct {
    sceVu0FVECTOR	trans;		/* trans xyz */
    sceVu0FVECTOR	rot;		/* rot xyz */
    sceVu0FVECTOR	scale;		/* scale xyz */
    int			shape;		/* shape id */
    int			parent;		/* parent id */
    int			child;		/* unused */
    int			sibling;	/* unused */
}sceHiPlugHrchyData_t;

extern sceHiPlugHrchyHead_t *sceHiPlugHrchyGetHead(sceHiPlug *p, sceHiType t);
extern sceHiPlugHrchyData_t *sceHiPlugHrchyGetData(sceHiPlugHrchyHead_t *h, int idx);
extern sceVu0FVECTOR *sceHiPlugHrchyGetPivot(sceHiPlugHrchyHead_t *h, int idx);

typedef struct {
    int		reserve[3];		/* reserve */
    int		num;			/* num of data */
}sceHiPlugTex2DHead_t;

typedef struct {
    sceGsTex0	tex0;			/* Gs tex0 register */
    union {
	struct {
	    int reserve;
	    int miplv;
	} mipmap;
	u_long	addr;			/* Gs tex0 address */
    } info;
    size_t	texelsize;		/* texel word size */
    size_t	clutsize;		/* clut word size */
    u_short	texelwidth;		/* texel width */
    u_short	texelheight;		/* texel height */
    u_short	clutwidth;		/* clut width */
    u_short	clutheight;		/* clut height */
}sceHiPlugTex2DData_t;

extern sceHiPlugTex2DHead_t *sceHiPlugTex2DGetHead(sceHiPlug *p, sceHiType t);
extern sceHiPlugTex2DData_t *sceHiPlugTex2DGetData(sceHiPlugTex2DHead_t *h, int idx);
extern u_int *sceHiPlugTex2DGetTexel(sceHiPlugTex2DData_t *d);
extern u_int *sceHiPlugTex2DGetClut(sceHiPlugTex2DData_t *d);

extern sceHiGsGiftag *sceHiPlugTex2DGetEnv(sceHiPlugTex2DHead_t *h, int idx);

typedef struct {
    sceVu0FVECTOR	dir[4];			/* 3 light direction, 1 reserve */
    sceVu0FVECTOR	pos[4];			/* 3 light position, 1 reserve */
    sceVu0FVECTOR	col[4];			/* 3 light color, 1 ambient */
}sceHiPlugMicroLight_t;

typedef struct {
    sceVu0FMATRIX	wscreen;		/* world screen matrix */
    union {
	sceVu0FMATRIX	wview;			/* world view matrix */
	sceVu0FMATRIX	texproj;		/* texture projection matrix */
    }mtx;
    sceVu0FMATRIX	material;		/* overwrite from shape */
    float		camx,camy,camz;		/* camera position */
    float		aa1;			/* aa1 cut off angle */
    float		fogA,fogB;		/* fog parameters */
    u_int		prmode;			/* prmode bit field */
    int			reserve;		/* reserve */
    union {
	sceVu0FVECTOR	clip;			/* clip parameters */
	struct {
	    float	texsize;		/* spherical texture size */
	    float	ZA,ZB;			/* depth parameters */
	    int		reserve;		/* reserve */
	}fisheye;
    }clp;
    float		shift;			/* emboss bump shift */
    float		refidx;			/* refract index */
    float		zoom;			/* reflect/refract zoom */
    float		zshift;			/* reflect/refract zshift */
    sceHiPlugMicroLight_t	light[3];	/* 3*3 lights */
} sceHiPlugMicroData_t;

extern sceHiPlugMicroData_t *sceHiPlugMicroGetData(sceHiPlug *p);

typedef struct {
    sceVu0FVECTOR	min;		/* bbox min size */
    sceVu0FVECTOR	max;		/* bbox max size */
    sceVu0FVECTOR	box[8];		/* dist bbox pos */
}sceHiPlugShadowBoxData_t;

extern sceHiPlugShadowBoxData_t *sceHiPlugShadowBoxGetData(sceHiPlug *p);

typedef struct {
    int		reserve[3];		/* reserve */
    int		num;			/* num of data */
}sceHiPlugClutBumpHead_t;

typedef struct {
    int		shape;			/* shape id */
    int		tex2d;			/* tex2d id */
    int		normal;			/* normal id */
    int		reserve;		/* reserve */
}sceHiPlugClutBumpData_t;

extern sceHiPlugClutBumpHead_t *sceHiPlugClutBumpGetHead(sceHiPlug *p, sceHiType t);
extern sceHiPlugClutBumpData_t *sceHiPlugClutBumpGetData(sceHiPlugClutBumpHead_t *h, int idx);
extern sceVu0FVECTOR *sceHiPlugClutBumpGetNormal(sceHiPlugClutBumpHead_t *h, int idx);

typedef struct {
    int		reserve[3];		/* reserve */
    int		num;			/* num of data */
}sceHiPlugTim2Head_t;

typedef struct {
    int		id;			/* tim2 id */
    int		*ptr;			/* file looad ptr */
    size_t	size;			/* file size */
    size_t	length;			/* name length fix 16 */
    char	fname[16];		/* file name fix 16 */
}sceHiPlugTim2Data_t;

extern sceHiPlugTim2Head_t *sceHiPlugTim2GetHead(sceHiPlug *p, sceHiType t);
extern sceHiPlugTim2Data_t *sceHiPlugTim2GetData(sceHiPlugTim2Head_t *h, int idx);

typedef union {
    struct {
	u_int	type;			/* interp|fcurve type */
	int	index;			/* index number (but unused) */
	size_t	size;			/* word size */
	int	num;			/* num of frame|value */
    }key;
    struct {
	int	reserve[3];		/* reserve */
	int	num;			/* num of data|key */
    }top;
}sceHiPlugAnimeHead_t;

typedef struct {
    int		hrchy;			/* hrchy id */
    int		numframe;		/* num of frame */
    int		keyframe;		/* keyframe id */
    int		keyvalue;		/* keyvalue id */
}sceHiPlugAnimeData_t;

extern sceHiPlugAnimeHead_t *sceHiPlugAnimeGetHead(sceHiPlug *p, sceHiType t);
extern sceHiPlugAnimeData_t *sceHiPlugAnimeGetData(sceHiPlugAnimeHead_t *h, int idx);
extern sceHiPlugAnimeHead_t *sceHiPlugAnimeGetKeyHead(sceHiPlugAnimeHead_t *h, int idx);
extern u_int *sceHiPlugAnimeGetFrame(sceHiPlugAnimeHead_t *h, int idx);
extern sceVu0FVECTOR *sceHiPlugAnimeGetValue(sceHiPlugAnimeHead_t *h, int idx);

typedef struct {
    int		reserve[2];	/* reserve */
    int		shape;		/* shape id */
    int		num;		/* num of data */
}sceHiPlugShareHead_t;

typedef union {
    struct {
	int	offset;		/* src offset */
	int	geomid;		/* geometry id */
	int	reserve;	/* reserve */
	int	num;		/* num of data */
    }shr;
    struct {
	int	voffset;	/* dst vertex offset */
	int	vlength;	/* dst vertex length */
	int	noffset;	/* dst normal offset */
	int	nlength;	/* dst normal length */
    }dat;
}sceHiPlugShareData_t;

extern sceHiPlugShareHead_t *sceHiPlugShareGetHead(sceHiPlug *p, sceHiType t);
extern sceHiPlugShareData_t *sceHiPlugShareGetData(sceHiPlugShareHead_t *h, int idx);
extern sceHiPlugShareData_t *sceHiPlugShareGetShare(sceHiPlugShareHead_t *h, int idx);
extern int *sceHiPlugShareGetIndex(sceHiPlugShareHead_t *h, int idx);
extern sceVu0FVECTOR *sceHiPlugShareGetSrc(sceHiPlugShareHead_t *h, int idx);
extern sceVu0FVECTOR *sceHiPlugShareGetDst(sceHiPlugShareHead_t *h, int idx);

typedef struct {
    int			reserve[3];
    int			num;
}sceHiPlugClipHead_t;

typedef struct {
    sceVu0FVECTOR	min;
    sceVu0FVECTOR	max;
}sceHiPlugClipData_t;

extern sceHiPlugClipHead_t *sceHiPlugClipGetHead(sceHiPlug *p, sceHiType t);
extern sceHiPlugClipData_t *sceHiPlugClipGetData(sceHiPlugClipHead_t *h, int idx);

typedef struct {
    int			reserve[2];	/* reserve */
    int			func;		/* function option */
    int			num;		/* num of data */
}sceHiPlugSkinHead_t;

typedef struct {
    sceVu0FVECTOR	vertex;
    sceVu0FVECTOR	normal;
    sceVu0FVECTOR	weight;
    int		       	matrix[4];
    int			id[4];
}sceHiPlugSkinData_t;

extern sceHiPlugSkinHead_t *sceHiPlugSkinGetHead(sceHiPlug *p, sceHiType t);
extern sceHiPlugSkinData_t *sceHiPlugSkinGetData(sceHiPlugSkinHead_t *h, int idx);
extern sceVu0FMATRIX *sceHiPlugSkinGetLB(sceHiPlugSkinHead_t *h, int idx);
extern sceVu0FMATRIX *sceHiPlugSkinGetLW(sceHiPlugSkinHead_t *h, int idx);
extern int *sceHiPlugSkinGetBW(sceHiPlugSkinHead_t *h, int idx);


typedef struct {
    int			reserve[4];
    sceVu0FVECTOR	screen;
    sceVu0FVECTOR	window;
    sceVu0FVECTOR	depth;
    sceVu0FVECTOR	position;
    sceVu0FVECTOR	rotation;
    sceVu0FVECTOR	interest;
    sceVu0FVECTOR	upvector;
}sceHiPlugCameraData_t;

extern sceHiPlugCameraData_t *sceHiPlugCameraGetData(sceHiPlug *p);

typedef struct {
    int			reserve[3];
    int			parent;
    sceVu0FMATRIX	matrix;
}sceHiPlugHrchyMatrix_t;

extern sceHiPlugHrchyMatrix_t *sceHiPlugHrchyGetMatrix(sceHiPlugHrchyHead_t *h, int idx);

/* ERX */
extern void *sceHiPlugGetErxEntries(void);

#include <hipdef.h>

#ifdef __cplusplus
}
#endif
#endif /* !__HIP_H__ */
