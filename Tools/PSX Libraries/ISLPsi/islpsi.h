#ifndef __ISLPSI_H__
#define __ISLPSI_H__


enum
{
	NOLIGHTING,
	DIRECTIONAL,
	DIRECTIONONLY,
	COLOURIZE,
	AMBIENT,
};


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


// PSI model structure
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


#define ANIM_QUEUE_LENGTH 8

// Actor animation data structure
typedef struct
{
	short		numAnimations;
	short		currentAnimation;

	UBYTE		reachedEndOfAnimation;
	UBYTE		loopAnimation;
	short 		numberQueued;

	long		animationSpeed;

	short		queueAnimation[ANIM_QUEUE_LENGTH];
	UBYTE		queueLoopAnimation[ANIM_QUEUE_LENGTH];
	long		queueAnimationSpeed[ANIM_QUEUE_LENGTH];
//	animation	*anims;
	long		animTime;//, animTimeDelta;

	short		frame;
	UBYTE 		exclusive;
	UBYTE 		spare;

} ACTOR_ANIMATION;


// bounding box data structure
typedef struct
{

	long	minX,maxX;
	long	minY,maxY;
	long	minZ,maxZ;

} BOUNDINGBOX;


// Actor data structure
typedef struct _ACTOR
{

	struct _ACTOR 	*next,*prev;

	unsigned long flags;	

	unsigned long numObjects;

	unsigned long noofVerts;

	unsigned long noofPrims;
	unsigned long primitiveList;

	USHORT		animFrames;				// number of animation frames
	USHORT		*animSegments;			// list of start-end frames (shorts)

	ACTOR_ANIMATION	animation;
	ULONG		*objectTable;

	VECTOR		oldPosition;	//
	VECTOR		accumulator;	// for animation movement;
	VECTOR		position;		// real position

   	VECTOR		size;

	PSIOBJECT	*object;

	ULONG		radius;
	BOUNDINGBOX	bounding;

	char		*modelName;

} ACTOR;

// RGB colour struct
typedef struct {
	u_char r, g, b;
} RGB;

// model control struct
typedef struct
{
	USHORT 	depthoverride; //0=sort normally, otherwise model will be at this depth;
	UBYTE  	specialmode;	//OFF addhue
	UBYTE	onmap;		//YES or NO;
			
	RGB		col;		//override RGB of model
			

	UBYTE		sprites;	//ON or OFF attached sprites on model
	UBYTE		alpha;		//0 or 1 (NO or YES)	//	was UBYTE	lighting;	//OFF ON (not used)
	UBYTE		semitrans;  //0 or 2 for ON!
	USHORT		PrimLeft;
	int			*SortPtr;
	ULONG		*PrimTop;
	VERT		*VertTop;
	VERT		*NormTop;
	LONG		polysdrawn;
	LONG		polysclipped;
	LONG		preclipped;
	USHORT		lastdepth;
	USHORT		nearclip; //default 100
	long		farclip;

	GsRVIEW2	*whichcamera;			

	USHORT		sorttable;
	USHORT		*SortOffs;

	UBYTE		lighting;

	int			depthShift;	// 0== use default shift !0=shift value

} PSIMODELCTRL;

extern PSIMODELCTRL	PSImodelctrl;


// function prototypes

void PSIInitialise();
void PSIDestroy();

void PSISetLight(int lightNum, int r, int g, int b, int x, int y, int z);

void actorAdd(ACTOR *actor);
void actorSub(ACTOR *actor);
void actorFree(ACTOR *actor);

ACTOR *actorCreate(PSIMODEL *psiModel);

PSIOBJECT *PSIFindObject(ACTOR *actor, char *name);
												  
void PSIDisplay(PSIMODEL* psiModel);

PSIMODEL *PSICheck(char *psiName);

PSIMODEL *PSILoad(char *psiName);

void PSIDrawLine(ULONG depth);

void PSIDrawBox(SHORT x,SHORT y,SHORT w,SHORT h,UBYTE r,UBYTE g,UBYTE b,UBYTE semi,SHORT pri);

void PSISetAnimation(ACTOR *actor, ULONG frame);

void PSISetAnimation2(ACTOR *actor, ULONG frame0, ULONG frame1, ULONG blend);

void PSIUpdateAnimation();

void PSISetBoundingRotated(ACTOR *actor,int frame,int rotX,int rotY, int rotZ);

void PSISetBounding(ACTOR *actor,int frame);

UBYTE PSIIsVisible(ACTOR *actor);

void PSIMoveActor(ACTOR *actor);

void PSIDrawActor(ACTOR *actor);

void PSIDraw();

void PSIDebug();

void actorInitAnim(ACTOR *actor);

void actorAdjustPosition(ACTOR *actor);

void actorAnimate(ACTOR *actor, int animNum, char loop, char queue, int speed, char skipendframe);

void actorSetAnimationSpeed(ACTOR *actor, int speed);

void actorUpdateAnimations(ACTOR *actor);


void PSISetMoveKeyFrames(PSIOBJECT *world, ULONG frame);


#define PSIGetAnimFrameStart(actor,anim)	(actor)->animSegments[ (anim)*2]
#define PSIGetAnimFrameEnd(actor,anim)		(actor)->animSegments[((anim)*2)+1]

#endif //__ISLPSI_H__