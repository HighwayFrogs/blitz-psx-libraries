/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	actor.h			Skinned model control routines

************************************************************************************/


#ifndef __ACTOR_H__
#define __ACTOR_H__

#define ANIM_QUEUE_LENGTH	8
#define ANIMSHIFT			8

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
	long		animTime;

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

	struct _ACTOR	*next,*prev;
	USHORT			animFrames;				// number of animation frames
	USHORT			*animSegments;			// list of start-end frames (shorts)
	ACTOR_ANIMATION	animation;
	VECTOR			oldPosition;	//
	VECTOR			accumulator;	// for animation movement;
	VECTOR			position;		// real position
   	VECTOR			size;
	ULONG			radius;
	BOUNDINGBOX		bounding;
	PSIDATA			psiData;

} ACTOR;


typedef struct
{
	short animStart;
	short animEnd;
} ANIMATION;

// linked list of actors
typedef struct 
{

	ACTOR	head;
	int	 	numEntries;

}	ACTORLIST;

extern ACTORLIST	actorList;


// functions

void actorInitialise();

void actorAdd(ACTOR *actor);

void actorSub(ACTOR *actor);

void actorFree(ACTOR *actor);

ACTOR *actorCreate(PSIMODEL *psiModel);

void actorInitAnim(ACTOR *actor);

void actorAdjustPosition(ACTOR *actor);

void actorAnimate(ACTOR *actor, int animNum, char loop, char queue, int speed, char skipendframe);

void actorSetAnimationSpeed(ACTOR *actor, int speed);

void actorUpdateAnimations(ACTOR *actor);

void actorDrawAll();

void actorMove(ACTOR *actor);

void actorDraw(ACTOR *actor);

void actorRotate(short angx, short angy, short angz, long movex, long movey, long movez, VECTOR *result);

void actorSetAnimation(ACTOR *actor, ULONG frame);

void actorSetAnimation2(ACTOR *actor, ULONG frame0, ULONG frame1, ULONG blend);

int actorCalcSegments(ACTOR *actor);

void actorSetBoundingRotated(ACTOR *actor,int frame,int rotX,int rotY, int rotZ);

void actorSetBounding(ACTOR *actor,int frame);

UBYTE actorIsVisible(ACTOR *actor);

#define ACTORGETANIMFRAMESTART(actor,anim)	(actor)->animSegments[ (anim)*2]

#define ACTORGETANIMFRAMEEND(actor,anim)		(actor)->animSegments[((anim)*2)+1]


#endif //__ACTOR_H__
