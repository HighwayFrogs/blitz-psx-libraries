/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islpsi.h			Skinned model routines

************************************************************************************/


#ifndef __ISLPSI_H__
#define __ISLPSI_H__


// lighting types
enum
{
	NOLIGHTING,
	DIRECTIONAL,
	DIRECTIONONLY,
	COLOURIZE,
	AMBIENT,
};

// flags for PSIDATA
#define ACTOR_DYNAMICSORT	1
#define ACTOR_HOLDMOTION	2
#define ACTOR_BONED			4
#define ACTOR_MOTIONBONE	8


// keyframe data structures
typedef struct
{
	SHORT x,y,z,w;
} SHORTQVECTOR;

typedef struct
{
	SHORT x,y,z;
} SHORTVECTOR;

typedef struct
{
	SHORTQVECTOR vect;
	SHORT time;
} SQKEYFRAME;

typedef struct
{
	SHORTVECTOR vect;
	SHORT time;
} SVKEYFRAME;


// PSI mesh data structure
typedef struct _PSIMESH
{

	VERT *vertop;  	       	// vertex top address
	u_long  vern;          	// the number of vertices
	VERT *nortop;			// normal top address
	u_long  norn;           // the number of normals
	u_long  scale;          // the scale factor of TMD format

	UBYTE			name[16];
	struct _PSIMESH	*child;
	struct _PSIMESH	*next;

	USHORT			numScaleKeys;
	USHORT			numMoveKeys;
	USHORT			numRotateKeys;
	USHORT			pad1;

	SVKEYFRAME		*scalekeys;
	SVKEYFRAME		*movekeys;
	SQKEYFRAME		*rotatekeys;
	USHORT			sortlistsize[8];
	USHORT			*sortlistptr[8];
	SVECTOR			center;
} PSIMESH;


// PSI object data structure
typedef struct _PSIOBJECT
{

	MATRIX  	matrixscale;		//not needed
	MATRIX  	matrix;

	PSIMESH		*meshdata;		//sort list size and pointers included in TMD

	struct _PSIOBJECT	*child;
	struct _PSIOBJECT	*next;

	
 	SVECTOR		rotate;			// calculated angle of object vx,vy,vz
   	VECTOR		scale;			// calculated scale


	int	depth;

} PSIOBJECT;

typedef struct _PSIDATA
{
	unsigned long	flags;
	unsigned long	numObjects;
	unsigned long	noofVerts;
	unsigned long	noofPrims;
	unsigned long	primitiveList;
	unsigned long	*objectTable;
	PSIOBJECT		*object;
	char			*modelName;
} PSIDATA;


// PSI model structure from Jobe
typedef struct
{

	char	id[4];			// "PSI",0

	long	version;		// version number

	long   	flags;			// 

	char	name[32];		// model name

	long	noofmeshes;		// number of objects in this model

	long	noofVerts;		// number of vertices in this model

	long	noofPrims;		// primitive list size
	long	primOffset;		// primitive list offset from start of file

	USHORT 	animStart;		// first frame number
	USHORT 	animEnd;		// last frame number

	long	animSegments;	// noof anim segments
	long	animSegmentList;// offest (from file start) to frame list


	long	noofTextures;	// number of textures
	long	textureOffset;	// offset to texture names

	long	firstMesh;		// offset to mesh data

	long	radius;			// max radius this model ever reaches

	char	pad[192-88];

} PSIMODEL;


// RGB colour struct
typedef struct {
	u_char r, g, b;
} RGB;


// model control struct
typedef struct
{
	USHORT 		depthoverride;	//0=sort normally, otherwise model will be at this depth;
	UBYTE  		specialmode;	//OFF addhue
	UBYTE		onmap;			//YES or NO;
	RGB			col;			//override RGB of model
	UBYTE		sprites;		//ON or OFF attached sprites on model
	UBYTE		alpha;			//0 or 1 (NO or YES)	//	was UBYTE	lighting;	//OFF ON (not used)
	UBYTE		semitrans;		//0 or 2 for ON!
	USHORT		PrimLeft;
	int			*SortPtr;
	ULONG		*PrimTop;
	VERT		*VertTop;
	VERT		*NormTop;
	LONG		polysdrawn;
	LONG		polysclipped;
	LONG		preclipped;
	USHORT		lastdepth;
	USHORT		nearclip;		//default 100
	long		farclip;
	GsRVIEW2	*whichcamera;			
	USHORT		sorttable;
	USHORT		*SortOffs;
	UBYTE		lighting;		// lighting mode
	int			depthShift;		// 0== use default shift !0=shift value

} PSIMODELCTRL;

extern PSIMODELCTRL	PSImodelctrl;

extern VECTOR *PSIactorScale;

extern VECTOR *PSIrootScale;


// function prototypes

void psiInitialise();

void psiDestroy();

void psiSetLight(int lightNum, int r, int g, int b, int x, int y, int z);

void psiDisplay(PSIMODEL* psiModel);

PSIOBJECT *psiObjectScan(PSIOBJECT *obj, char *name);

PSIMODEL *psiCheck(char *psiName);

PSIMODEL *psiLoad(char *psiName);

void psiDrawLine(ULONG depth);

void psiDrawBox(SHORT x,SHORT y,SHORT w,SHORT h,UBYTE r,UBYTE g,UBYTE b,UBYTE semi,SHORT pri);

void psiUpdateAnimation();

void psiDebug();

void psiSetMoveKeyFrames(PSIOBJECT *world, ULONG frame);

void psiSetScaleKeyFrames(PSIOBJECT *world, ULONG frame);

void psiSetRotateKeyFrames(PSIOBJECT *world, ULONG frame);

void psiSetMoveKeyFrames2(PSIOBJECT *world, ULONG frame0, ULONG frame1, ULONG blend);

void psiSetScaleKeyFrames2(PSIOBJECT *world, ULONG frame0, ULONG frame1, ULONG blend);

void psiSetRotateKeyFrames2(PSIOBJECT *world, ULONG frame0, ULONG frame1, ULONG blend);

void psiInitSortList(int range);

void psiCalcWorldMatrix(PSIOBJECT *world);

void psiDrawSegments(PSIDATA *psiData);

void psiCalcLocalMatrix(PSIOBJECT *world,MATRIX *parentM,MATRIX *parentMS);

void psiRegisterDrawFunction(void (*drawHandler)(int));

#endif //__ISLPSI_H__
