/************************************************************************************
	PSX CORE (c) 1999 ISL

	psi.c:		Playstation Model (i) Handler

************************************************************************************/

#include <stddef.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <inline_c.h>
#include <gtemac.h>
#include <stdio.h>
#include "..\shell\shell.h"
#include "islpsi.h"
#include "psi.h"
#include "quatern.h"
#include "..\islmem\islmem.h"
#include "..\islfile\islfile.h"
#include "..\isltex\isltex.h"
#include "..\islutil\islutil.h"


#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

// an assembler version of transformVertexList()
void transformVertexListA(VERT *vertPtr, long numVerts, long *transformedVerts, long *transformedDepths);

void ShortquaternionGetMatrix(SHORTQUAT *squat, MATRIX *dmatrix);

void ShortquaternionSlerpMatrix(SHORTQUAT *src1, SHORTQUAT *sp2, ULONG t,MATRIX *dmatrix);


#define Bound(a,x,b) min(b,max(a,x))
/***************************************************************************************************/

//#define SHOWNORM 1
/***************************************************************************************************/

MATRIX	cameraAndGlobalscale;
VECTOR *PSIactorScale = 0;
VECTOR *PSIrootScale = 0;

GsF_LIGHT	flatLight[3];
MATRIX		lightMat;
SVECTOR		lightRotVec;

SHORTQUAT squat={ 0,0,0,1 };

PSIMODEL *currentPSI;
char *PSIname=0;

//#define transformedNormals  ((long*) getScratchAddr(0))
long *transformedVertices = 0;
long *transformedDepths = 0;
VERT *transformedNormals = 0;
int  tfTotal = 0;
#ifdef SHOWNORM
long *transformedScreenN = 0;
#endif

long	clipflag  __attribute__ ((section("cachedata"))) = 0;

RGBCD ambience={ 128,128,128,0};


int	biggestVertexModel=0;
int	biggestPrimModel=0;

int biggestSortDepth=0;
int smallestSortDepth=0;


#define MAXMODELS 50

PSIMODEL *psiModelList[MAXMODELS];
char psiModelListNames[MAXMODELS][32];
long psiModelListCRC[MAXMODELS];
int	psiModelListLen = 0;

long *pilLibraryList[8];
int pilLibraryLen = 0;

#define gte_SetLDDQA( r0 ) __asm__ volatile (		\
	"ctc2	%0, $27"					\
	:							\
	: "r"( r0 ) )

#define gte_SetLDDQB( r0 ) __asm__ volatile (		\
	"ctc2	%0, $28"					\
	:							\
	: "r"( r0 ) )

TMD_P_FT4I *testpol;
POLY_FT4 *testsi;

#define ADD2POINTER(a,b) ( (void *) ( (int)(a) + (int)(b))  )

#define DEFAULTDEPTHSHIFT 1

ACTORLIST	actorList;

typedef struct {
	char tname[32];
} PSITEXTURES;

PSITEXTURES	*currentTextureList;

PSIMODELCTRL	PSImodelctrl;


void MATHS_Rotate(short angx, short angy, short angz, long movex, long movey, long movez, VECTOR *result)
{																
	SVECTOR  MATHS_ang;											
	VECTOR   MATHS_move;										
	MATRIX   MATHS_rotMat;										
																
	MATHS_ang.vx = (angx);										
	MATHS_ang.vy = (angy);										
	MATHS_ang.vz = (angz);										
																
	MATHS_move.vx = (movex);								
	MATHS_move.vy = (movey);									
	MATHS_move.vz = (movez);									
																
	MATHS_rotMat = GsIDMATRIX;									
	RotMatrixYXZ_gte(&MATHS_ang, &MATHS_rotMat);				
	ApplyMatrixLV(&MATHS_rotMat, &MATHS_move, (result));		

}

/***************************************************************************************************/

void PSIResetModelctrl()
{
	PSImodelctrl.depthoverride = 0;
	PSImodelctrl.specialmode = OFF;
	PSImodelctrl.onmap = 0;
	PSImodelctrl.col.r = 128;
	PSImodelctrl.col.g = 128;
	PSImodelctrl.col.b = 128;
	PSImodelctrl.sprites = YES;
	PSImodelctrl.nearclip = 100;
	PSImodelctrl.farclip = 16384;

}

/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void DrawNormals()
{
#ifdef SHOWNORM

	int i;
	ACTOR *actor;
	VERT *verts;
	VERT *norms;
	VERT *convs;
	VERT newnorm;
	PSIOBJECT *world;
	int loop,obs,j,v,rf,tot;
	SVECTOR *newvects;
	DVECTOR *tfvs,*tfscxy;
	DVECTOR *tfsn = (DVECTOR*)transformedScreenN;

	tfvs = (DVECTOR*)transformedVertices;


	actor = actorList.head.next;
	world = actor->object;
	obs = actor->numObjects;
	i = actor->noofVerts;
	newvects = (SVECTOR*)MALLOC( i*sizeof(SVECTOR));

	verts = actor->object->meshdata->vertop;

	polyLine.r0 = 240;
	polyLine.g0 = 240;

	tot = 0;

	for (loop = 0; loop < obs; loop++)
	{
		world = (PSIOBJECT*)actor->objectTable[loop];

	 	gte_SetRotMatrix(&world->matrixscale);			 
	   	gte_SetTransMatrix(&world->matrixscale);		


		verts = world->meshdata->vertop;
		norms = world->meshdata->nortop;
		v = world->meshdata->norn;
		convs = (VERT*)newvects;

		rf = 128;					 

		for (j=0; j<v; j++)
		{

			convs->vx = (norms->vx/rf) + verts->vx;
			convs->vy = (norms->vy/rf) + verts->vy;
			convs->vz = (norms->vz/rf) + verts->vz;
			verts++;
			norms++;
			convs++;
		}

		transformVertexListB(newvects, world->meshdata->norn, tfsn );
		
		tfsn += v;
		tot += v;

	}
	
	tfsn = (DVECTOR*)transformedScreenN;
	tfvs = (DVECTOR*)transformedVertices;
	
	
	for (i = 0; i<tot; i++)
	{
		polyLine.r0 = 240;
		polyLine.g0 = 240;
		polyLine.b0 = 240;

		polyLine.x0 = tfvs->vx;
		polyLine.y0 = tfvs->vy;
		polyLine.x1 = tfsn->vx;
		polyLine.y1 = tfsn->vy;

		PSIDrawLine(0);
		tfvs++;
		tfsn++;
	}
	FREE(newvects);
#endif
}
/**************************************************************************/
/**************************************************************************/

int maxDepthRange=0;

ULONG *sortedIndex=0;//[DEPTHINIT];
int sortCount=0;

long maxDepth=0;
long minDepth=0;
int depthRange=0;

/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void PSIInitSortList(int range)
{
	if ( range<= maxDepthRange )
		return;

	maxDepthRange = range;

	if (sortedIndex)
		FREE(sortedIndex);
	
	sortedIndex = (ULONG*)MALLOC( range*sizeof(ULONG));
}

/**************************************************************************
	FUNCTION:	CalcMaxMin	
	PURPOSE:	run thru transformed depth list, find max and min values
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
void actorCalcMaxMin(ACTOR *actor)
{
	long *tfd = transformedDepths;
	int i;
	

	for (i=0; i<maxDepthRange; i++) sortedIndex[i]=0;
	sortCount=0;
	maxDepth = minDepth = *tfd;
	
	i = tfTotal;
	while (i)
	{
		if (*tfd < minDepth)
			minDepth = *tfd;
		if (*tfd > maxDepth)
			maxDepth = *tfd;
		tfd++;
		i--;
	}
	depthRange= (maxDepth-minDepth)+1;
	if (depthRange>maxDepthRange)
	{
		printf("PROGRAM HALTED. \nDEPTH RANGE EXCEEDED !\nINCREASE MAXDEPTHRANGE TO %d (=%d)\n\n",depthRange,maxDepthRange);
		printf("Object Mesh Name = >%s<\n",actor->modelName);
		for (;;);
	}
	if (depthRange > biggestSortDepth)
		biggestSortDepth = depthRange;

}
/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
void PSISetLight(int lightNum, int r, int g, int b, int x, int y, int z)
{

	flatLight[lightNum].vx = x;
	flatLight[lightNum].vy = y;
	flatLight[lightNum].vz = z;
 	flatLight[lightNum].r  = r;
 	flatLight[lightNum].g  = g;
 	flatLight[lightNum].b  = b;
	GsSetFlatLight( lightNum, &flatLight[lightNum]);


   	lightMat=GsIDMATRIX;
	RotMatrixYXZ(&lightRotVec,&lightMat);
	GsSetLightMatrix2(&lightMat);

}
/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

GsCOORDINATE2 lighting;

void PSIInitLights()
{

   	lightMat=GsIDMATRIX;
	lightRotVec.vx = 0;
	lightRotVec.vy = 0;
	lightRotVec.vz = 0;
	RotMatrixYXZ(&lightRotVec,&lightMat);

	PSISetLight(0, 255,255,255, 256,0,0 );	// white light, left->right

//	PSISetLight(0, 0,0,0, 0,0,0 );
	//PSISetLight(1, 0,0,0, 0,0,0 );
	//PSISetLight(2, 0,0,0, 0,0,0 );
	
 	GsSetAmbient(1024,1024,1024);
	GsSetLightMode(0);

}



/**************************************************************************
	Function 	: InitPSI()
	Purpose 	: initialises the linked list of actors and local vars
	Parameters 	: none
	Returns 	: none
	Info 		:
**************************************************************************/
void PSIInitialise()
{

	PSIResetModelctrl();

	PSIInitSortList(256);

	PSIInitLights();

	actorList.numEntries = 0;
	actorList.head.next = actorList.head.prev = &actorList.head;

	transformedVertices = 0;
	transformedDepths = 0;
	transformedNormals = 0;
	biggestVertexModel = 0;
	biggestPrimModel = 0;

	psiModelListLen = 0;
	pilLibraryLen = 0;


}
/**************************************************************************
	Function 	: actorAdd()
	Purpose 	: adds an actor to the linked list
	Parameters 	: pointer to actor
	Returns 	: none
	Info 		:
**************************************************************************/
void actorAdd(ACTOR *actor)
{
	ACTOR *ptr;

	ptr = actorList.head.next;

	if(actor->next == NULL)
	{
		actorList.numEntries++;
		actor->next = ptr;
		actor->prev = ptr->prev;
		ptr->prev->next = actor;
		ptr->prev = actor;		
	}
}

/**************************************************************************
	Function 	: SubActor()
	Purpose 	: removes an actor from the linked list
	Parameters 	: pointer to actor
	Returns 	: none
	Info 		:
**************************************************************************/
void actorSub(ACTOR *actor)
{

	if(actor->next != NULL)
	{
		actor->prev->next = actor->next;
		actor->next->prev = actor->prev;
		actorList.numEntries--;
		actor->next = NULL;
	}

}
/**************************************************************************
	FUNCTION:	FreeActor
	PURPOSE:	free all memory used by actor structure
	PARAMETERS:	ACTOR *
	RETURNS:	
**************************************************************************/
void actorFree(ACTOR *actor)
{
	FREE(actor->objectTable);
	actorSub(actor);
	FREE(actor);
}


/**************************************************************************
	Function 	: PSIDestroy()
	Purpose 	: free all memory used by PSI
	Parameters 	: 
	Returns 	: 
	Info 		:
**************************************************************************/
void PSIDestroy()
{

	while (actorList.numEntries)
		actorFree(actorList.head.next);
	
	FREE(transformedNormals);
	FREE(transformedDepths);
	FREE(transformedVertices);


	while (psiModelListLen)
	{
	 	psiModelListLen--;
	 	FREE(psiModelList[psiModelListLen]);
	}

	while (pilLibraryLen)
	{
		pilLibraryLen--;
	 	FREE(pilLibraryList[pilLibraryLen]);
	}

	transformedVertices = 0;
	transformedDepths = 0;
	transformedNormals = 0;
	biggestVertexModel = 0;
	biggestPrimModel = 0;
	
	if (sortedIndex)
		FREE(sortedIndex);
	sortedIndex = NULL;
	maxDepthRange = 0;

	
}

/**************************************************************************
	FUNCTION:	PSIObjectScan
	PURPOSE:	Scan object heirarchy for name
	PARAMETERS:	ASSUMES *name IS UPPER CASE !!
	RETURNS:	
**************************************************************************/

PSIOBJECT *PSIObjectScan(PSIOBJECT *obj, char *name)
{


	PSIOBJECT *result;
	
	while (obj)
	{
//		utilPrintf("<%s>\n",obj->meshdata->name);
		
		utilUpperStr(obj->meshdata->name);

		if ( strcmp( obj->meshdata->name, name )==0 ) return obj;
		
		if ( obj->child ){
			 result = PSIObjectScan(obj->child,name);
			 if (result) return result;
		}
		obj = obj->next;

	}	
	return NULL;
}
/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	ASSUMES *name IS UPPER CASE !!
	RETURNS:	
**************************************************************************/

PSIOBJECT *PSIFindObject(ACTOR *actor, char *name)
{


	utilPrintf("looking for %s\n",name);
	return PSIObjectScan(actor->object,name);
}

/**************************************************************************
	FUNCTION:	SetPSIObject
	PURPOSE:	set up next & child pointers in PSIOBJECT
	PARAMETERS:	
	RETURNS:	
**************************************************************************/


PSIOBJECT *psiobNext;
ULONG 	*segTable;


void PSISetTree(PSIOBJECT *obj,PSIMESH *mesh)
{

		*segTable = (ULONG)obj;
		segTable++;

		obj->meshdata = mesh;

		if (mesh->child)
		{
//			utilPrintf("chil->$%x\n",mesh->child);
			obj->child	  = psiobNext;
	   		PSISetTree(psiobNext++,mesh->child);
		}
		if (mesh->next)
		{
//			utilPrintf("next->$%x\n",mesh->next);
			obj->next  = psiobNext;
	   		PSISetTree(psiobNext++,mesh->next);
		}
}


/**************************************************************************
	FUNCTION:	CreateActor
	PURPOSE:	create an actor structure from PSIModel data
	PARAMETERS:	PSIModel*
	RETURNS:	ACTOR*
**************************************************************************/
ACTOR *actorCreate(PSIMODEL *psiModel)
{

	ULONG len,parts,i;
	ULONG *amem;
	ACTOR *actor;
	PSIOBJECT *psiobRoot;
	PSIMESH *meshRoot;
	char actorName[32];

	if (psiModel==NULL) return 0;

	parts = psiModel->noofmeshes;			// number of objects in this model

	len = parts * sizeof(PSIOBJECT);
	len += sizeof (ACTOR);

	utilPrintf("New actor: %d bytes\n",len);
	
	sprintf(actorName,"ACTOR:%s",(char*)psiModel+4);
	actor = (ACTOR*)MALLOC( len);
	
	// flush memory
	amem = (ULONG*)actor;
	for (i=0; i<(len/4); i++) *amem++ = 0;
//	memoryShow();

	actor->flags = 0;
	if (psiModel->flags & 1) actor->flags = ACTOR_BONED;



	actor->modelName = &psiModel->name[0];

	actor->numObjects = parts;

	// list of pointers to model objects/meshes
	actor->objectTable = (ULONG*)MALLOC( parts*sizeof(int));

	// set primitive pointer
	actor->primitiveList = psiModel->primOffset;

	// set prim counter
	actor->noofPrims = psiModel->noofPrims;

	actor->noofVerts = psiModel->noofVerts;


	meshRoot = (PSIMESH*)psiModel->firstMesh;

	//	set anim frame pointers
	actor->animation.numAnimations = psiModel->animSegments;
	actor->animSegments = (USHORT*)psiModel->animSegmentList;	// offest to frame list

	// set up actor structure

	psiobRoot = (PSIOBJECT*)((char*)actor + sizeof(ACTOR));		// object structs follow actor struct
	psiobNext = psiobRoot+1;

	actor->object = psiobRoot;									// pointer to 1st object

	// fill in next & child pointers
	segTable = actor->objectTable;
	PSISetTree(psiobRoot,meshRoot);

	actorAdd(actor);											// add actor to actorList

	actorInitAnim(actor);										// initialise animation structure

	actor->animFrames = psiModel->animEnd;						// total noof frames of animation

	actor->radius = psiModel->radius;
	
	PSIInitSortList( (actor->radius*2)+8 );	 					// make enough room to z sort polys
	actor->size.vx = actor->size.vy = actor->size.vz = 4096;

	if (PSIObjectScan(actor->object,"MOTION"))
	{
		utilPrintf("motion bone found\n");
		actor->flags |= ACTOR_MOTIONBONE;
	}
	else utilPrintf("motion bone NOT found\n");

	actor->flags |= ACTOR_DYNAMICSORT;							// by default

	PSISetBoundingRotated(actor,0,0,0,0);						// fill in bounding box info

	return actor;

}

/**************************************************************************
	FUNCTION:	PageTexture
	PURPOSE:	get tpage and clut for this prim's texture
	PARAMETERS:	index number+1
	RETURNS:	pointer to texture TextureType
**************************************************************************/
TextureType* PageTexture(USHORT ix)
{
	TextureType *sprt;

	if (!ix) return 0;

	ix--;

	sprt = textureFindCRCInAllBanks(utilStr2CRC((char*)&currentTextureList[ix]) );
/*	if (strcmp((char*)&currentTextureList[ix], "ACMN8A" )==0)
	{
		utilPrintf("    texture->%s<- $%x\n",(char*)&currentTextureList[ix],sprt );
		iansprt=sprt;
	}
*/
	
	return sprt;

}

/**************************************************************************
	FUNCTION:	FixupPrims
	PURPOSE:	run thru the primitive list, loading textures & pointers
	PARAMETERS:	PSIMODEL address
	RETURNS:	nowt
**************************************************************************/
int TG4count=0;
int TG3count=0;
int TF4count=0;
int TF3count=0;
int F4count=0;
int F3count=0;

void PSIFixupPrims(PSIMODEL *psiModel)
{
	TextureType *sprt;
	int p,a;
	char *primitive;
	UBYTE s;

	//int ix; //temp

		p = psiModel->noofPrims;
		primitive = (char*)psiModel->primOffset;

		while (p)
		{
			
			s = ((TMD_P_GT4I*)primitive)->cd;	// code
			switch (s &(0xff-2)){

				case GPU_COM_G3:
				case GPU_COM_F3:	a = sizeof(TMD_P_FG3I );

							 		((TMD_P_FG3I*)primitive)->in=(sizeof(TMD_P_FG3I)/4)-1;	//ilen
		 							((TMD_P_FG3I*)primitive)->out=0xc; 						//olen   
									if (((TMD_P_FG3I*)primitive)->dummy & psiTRANSPAR ) ((TMD_P_FG3I*)primitive)->cd |=2;
									((TMD_P_FG3I*)primitive)->dummy &= psiDOUBLESIDED; 
									F3count++;							  
									break;

				case GPU_COM_G4:
				case GPU_COM_F4:	a = sizeof(TMD_P_FG4I );
							 		((TMD_P_FG4I*)primitive)->in=(sizeof(TMD_P_FG4I)/4)-1;	//ilen
		 							((TMD_P_FG4I*)primitive)->out=0x8; 						//olen   
									if (((TMD_P_FG4I*)primitive)->dummy & psiTRANSPAR ) ((TMD_P_FG4I*)primitive)->cd |=2;
									((TMD_P_FG4I*)primitive)->dummy &= psiDOUBLESIDED; 
									F4count++;
									break;

				case GPU_COM_TF3:	a = sizeof(TMD_P_FT3I );
									sprt = PageTexture( ((TMD_P_FT3I*)primitive)->clut ); // clut = texture index number
									if (((TMD_P_FT3I*)primitive)->tpage) ((TMD_P_FT3I*)primitive)->cd |= 2;
									((TMD_P_FT3I*)primitive)->tpage |= sprt->tpage;
									((TMD_P_FT3I*)primitive)->clut = sprt->clut;

							 		((TMD_P_FT3I*)primitive)->in=(sizeof(TMD_P_FT3I)/4)-1;	//ilen
		 							((TMD_P_FT3I*)primitive)->out=0x7; 						//olen   

									((TMD_P_FT3I*)primitive)->tu0 += sprt->x;
									((TMD_P_FT3I*)primitive)->tu1 += sprt->x;
									((TMD_P_FT3I*)primitive)->tu2 += sprt->x;

									((TMD_P_FT3I*)primitive)->tv0 += sprt->y;
									((TMD_P_FT3I*)primitive)->tv1 += sprt->y;
									((TMD_P_FT3I*)primitive)->tv2 += sprt->y;

									TF3count++;
									((TMD_P_FG4I*)primitive)->dummy &= psiDOUBLESIDED; 
									break;

				case GPU_COM_TF4SPR:
				case GPU_COM_TF4:	a = sizeof(TMD_P_FT4I );
									sprt = PageTexture( ((TMD_P_FT4I*)primitive)->clut ); // clut= texture index number

									if (((TMD_P_FT4I*)primitive)->tpage) ((TMD_P_FT4I*)primitive)->cd |= 2;
									((TMD_P_FT4I*)primitive)->tpage |= sprt->tpage;
									((TMD_P_FT4I*)primitive)->clut = sprt->clut;

							 		((TMD_P_FT4I*)primitive)->in=(sizeof(TMD_P_FT4I)/4)-1;	//ilen
		 							((TMD_P_FT4I*)primitive)->out=0x9; 						//olen   

									((TMD_P_FT4I*)primitive)->tu0 += sprt->x;
									((TMD_P_FT4I*)primitive)->tu1 += sprt->x;
									((TMD_P_FT4I*)primitive)->tu2 += sprt->x;
									((TMD_P_FT4I*)primitive)->tu3 += sprt->x;

									((TMD_P_FT4I*)primitive)->tv0 += sprt->y;
									((TMD_P_FT4I*)primitive)->tv1 += sprt->y;
									((TMD_P_FT4I*)primitive)->tv2 += sprt->y;
									((TMD_P_FT4I*)primitive)->tv3 += sprt->y;

									TF4count++;
									((TMD_P_FG4I*)primitive)->dummy &= psiDOUBLESIDED; 
									break;

				case GPU_COM_TG3:	a = sizeof(TMD_P_GT3I );
									sprt = PageTexture( ((TMD_P_GT3I*)primitive)->clut ); // clut= texture index number
									if (((TMD_P_GT3I*)primitive)->tpage) ((TMD_P_GT3I*)primitive)->cd |= 2;
									((TMD_P_GT3I*)primitive)->tpage |= sprt->tpage;
									((TMD_P_GT3I*)primitive)->clut = sprt->clut;

							 		((TMD_P_GT3I*)primitive)->in=(sizeof(TMD_P_GT3I)/4)-1;	//ilen
		 							((TMD_P_GT3I*)primitive)->out=0x9; 						//olen   

									((TMD_P_GT3I*)primitive)->tu0 += sprt->x;
									((TMD_P_GT3I*)primitive)->tu1 += sprt->x;
									((TMD_P_GT3I*)primitive)->tu2 += sprt->x;

									((TMD_P_GT3I*)primitive)->tv0 += sprt->y;
									((TMD_P_GT3I*)primitive)->tv1 += sprt->y;
									((TMD_P_GT3I*)primitive)->tv2 += sprt->y;

									TG3count++;
									((TMD_P_FG4I*)primitive)->dummy &= psiDOUBLESIDED; 
									break;

				case GPU_COM_TG4:	a = sizeof(TMD_P_GT4I );
									sprt = PageTexture( ((TMD_P_GT4I*)primitive)->clut ); // clut= texture index number

									if (((TMD_P_GT4I*)primitive)->tpage) ((TMD_P_GT4I*)primitive)->cd |= 2;
									((TMD_P_GT4I*)primitive)->tpage |= sprt->tpage;
									((TMD_P_GT4I*)primitive)->clut = sprt->clut;

							 		((TMD_P_GT4I*)primitive)->in=(sizeof(TMD_P_GT4I)/4)-1;	//ilen
		 							((TMD_P_GT4I*)primitive)->out=0xc; 						//olen   

									((TMD_P_GT4I*)primitive)->tu0 += sprt->x;
									((TMD_P_GT4I*)primitive)->tu1 += sprt->x;
									((TMD_P_GT4I*)primitive)->tu2 += sprt->x;
									((TMD_P_GT4I*)primitive)->tu3 += sprt->x;

									((TMD_P_GT4I*)primitive)->tv0 += sprt->y;
									((TMD_P_GT4I*)primitive)->tv1 += sprt->y;
									((TMD_P_GT4I*)primitive)->tv2 += sprt->y;
									((TMD_P_GT4I*)primitive)->tv3 += sprt->y;
									
									TG4count++;
									((TMD_P_FG4I*)primitive)->dummy &= psiDOUBLESIDED; 


									break;


				default:	a = 0;
							utilPrintf("FATAL: don't know that kind of prim...($%x)\n",s);
							for (;;);
							break;

			}
			primitive += a;
			p--;
		}
}

/**************************************************************************
	FUNCTION:	FixupMesh
	PURPOSE:	turn loaded PSImodel offsets into physical addresses
	PARAMETERS:	mesh address, start of model address
	RETURNS:	nowt
**************************************************************************/
void PSIFixupMesh(PSIMESH *mesh)
{
	int p,i;


	// main pointers										
	(int)mesh->vertop = (int)mesh + (int)mesh->vertop;
	(int)mesh->nortop = (int)mesh + (int)mesh->nortop;

	(int)mesh->scalekeys = (int)mesh + (int)mesh->scalekeys;
	(int)mesh->movekeys = (int)mesh + (int)mesh->movekeys;
	(int)mesh->rotatekeys = (int)mesh + (int)mesh->rotatekeys;


	// sortlist pointers
	p = (int)mesh;
	for (i=0; i<8; i++)	(int)mesh->sortlistptr[i] = p + (int)mesh->sortlistptr[i];

	// child and next pointers
	if (mesh->child)
	{
		(int)mesh->child = (int)mesh + (int)mesh->child;
		PSIFixupMesh(mesh->child);
	}
	if (mesh->next)
	{
		(int)mesh->next = (int)mesh + (int)mesh->next;
		PSIFixupMesh(mesh->next);
	}
}


/**************************************************************************
	FUNCTION:	DisplayPSI()
	PURPOSE:	display PSIMODEL stats
	PARAMETERS:	PSIMODEL*
	RETURNS:	nowt
**************************************************************************/
void PSIDisplay(PSIMODEL* psiModel)
{

	utilPrintf("Loading model '%s'\n",psiModel->name);

	utilPrintf("PSI Verion no. %d\n",psiModel->version);

	utilPrintf("%d meshes\n",psiModel->noofmeshes);

	utilPrintf("%d vertices\n",psiModel->noofVerts);

	utilPrintf("%d polys\n",psiModel->noofPrims);

	utilPrintf("%d textures.\n",psiModel->noofTextures);
}

/**************************************************************************
	FUNCTION:	ContructPSIName()
	PURPOSE:	build file name into *.PSI
	PARAMETERS:	filename
	RETURNS:	
**************************************************************************/

char *PSIConstructName(char *psiName)
{
	int i;
	
	for ( i=0; i<8; i++) if ( (psiName[i]==0) || (psiName[i]==32) || (psiName[i]=='.') )break;

	psiName[i] = '.';
	psiName[i+1] = 'P';
	psiName[i+2] = 'S';
	psiName[i+3] = 'I';
	psiName[i+4] = 0;

	utilUpperStr(psiName);
	return &psiName[0];
	
}
/**************************************************************************
	FUNCTION:	ContructPSIName()
	PURPOSE:	build file name into *.PSI
	PARAMETERS:	filename
	RETURNS:	
**************************************************************************/

long PSICRCName(char *psiName)
{
	int i;
	char str[16];

	
	utilUpperStr(psiName);
	for ( i=0; i<8; i++)
	{
		str[i] = psiName[i];
		if ( (psiName[i]==0) || (psiName[i]==32) || (psiName[i]=='.') )break;
	}
	str[i] = 0;
	return utilStr2CRC(&str[0]);
}


/**************************************************************************
	FUNCTION:	CheckPSI()
	PURPOSE:	see if a psi file is already loaded
	PARAMETERS:	filename.psi
	RETURNS:	pointer to PSIMODEL / null
**************************************************************************/

PSIMODEL *PSICheck(char *psiName)
{
	int i,crc;

	crc = PSICRCName(psiName);

	for ( i=0; i<psiModelListLen; i++)
	{
		if ( psiModelListCRC[i] == crc ) return psiModelList[i];
	}
	return NULL;
}


/**************************************************************************
	FUNCTION:	PSIFixup()
	PURPOSE:	initialise a psi file, fix up ponters and load textures
	PARAMETERS:	pointer to PSIMODEL
	RETURNS:	pointer to PSIMODEL
**************************************************************************/
PSIMODEL *PSIFixup(char *addr)
{
	PSIMESH *mesh;
	int /*lastfilelength,*/i;
	PSIMODEL *psiModel;

	psiModel = (PSIMODEL*)addr;

	currentPSI = psiModel;

	// do check for "PSI",0
	if(strcmp((char*)psiModel,"PSI")!=0){ utilPrintf("%s is not a valid PSI file.\n",addr); return 0; }

	// primitive offset pointer
	psiModel->primOffset += (int)addr;
	
	// list of texture names
	psiModel->textureOffset += (int)addr;
	
	// mesh offset
	psiModel->firstMesh += (int)addr;
	
	//	offset to anim frame list
	psiModel->animSegmentList += (int)addr;

	// malloc transformation vertex,normal and depth buffers to biggest
	// model vert count loaded so far
	i = psiModel->noofVerts;
	if ( i>biggestVertexModel )
	{
		if (transformedVertices!=0)
		{
			 FREE(transformedVertices);
			 FREE(transformedDepths);
			 FREE(transformedNormals);
#ifdef SHOWNORM
			FREE(transformedScreenN);
#endif
		}
		transformedVertices = (long*)MALLOC( i*sizeof(SVECTOR));
		transformedDepths = (long*)MALLOC( i*sizeof(SVECTOR));
		transformedNormals = (VERT*)MALLOC( i*sizeof(VERT));
		biggestVertexModel = i;

#ifdef SHOWNORM
		transformedScreenN = (long*)MALLOC( i*sizeof(SVECTOR));
#endif
	}

//	DisplayPSI(psiModel);

	// number of textures
	(char*)currentTextureList = psiModel->textureOffset;

//	numoftextures = psiModel->noofTextures;
//	for (i=0; i<numoftextures; i++) utilPrintf("%d: %s\n",i,&textureList[i]);

	
	
	// PSI MESH LIST

	mesh = (PSIMESH*)psiModel->firstMesh;

	PSIFixupMesh(mesh);

	PSIFixupPrims(psiModel);

//	utilPrintf("Load PSI done\n");

	// add to loaded model list (for checking re-loads)
	psiModelList[psiModelListLen] = psiModel;
//	strncpy( psiModelListNames[psiModelListLen], PSIname, 16);
	psiModelListCRC[psiModelListLen] = PSICRCName( PSIname );
	psiModelListLen++;

	return psiModel;
}

/**************************************************************************
	FUNCTION:	LoadPSI()
	PURPOSE:	load a psi file, fix up ponters and load textures
	PARAMETERS:	filename.psi
	RETURNS:	pointer to PSIMODEL
**************************************************************************/


PSIMODEL *PSILoad(char *psiName)
{
	char *addr;
	//PSIMESH *mesh;
	int lastfilelength;//,i;
	PSIMODEL *psiModel;


	if (sizeof(PSIMODEL) != 192) utilPrintf("\nWARNING: PSIMODEL structure length invalid (%d != 192)\n\n",sizeof(PSIMODEL));
	

	psiModel = PSICheck(psiName);		// already loaded ?
	if (psiModel!=NULL)
	{
		utilPrintf("Already loaded file %s\n",psiName);
		PSIDisplay(psiModel);
		return psiModel;

	}

	PSIname = psiName;

	if (psiModelListLen>=MAXMODELS) { utilPrintf("jees ! how many models do you expect me to load ?\n"); return 0; }


	psiName = PSIConstructName(psiName);
	addr = (void *)fileLoad(psiName, &lastfilelength);
	
	return PSIFixup(addr);
}

/**************************************************************************
	FUNCTION:	LoadPIL()
	PURPOSE:	load psi library and ALL models in it, fix up ponters and load textures
	PARAMETERS:	filename.pil
	RETURNS:	pointer to PSIMODEL
**************************************************************************/


void *PSILoadPIL(char *pilName)
{
	char *addr;
	int psiM;
	int *table,*crcs;
	int lastfilelength,i;

	addr = (void *)fileLoad(pilName, &lastfilelength);
	if(!addr)
		return NULL;

	i = (int)*addr;
	(char*)table = addr+4;

	utilPrintf("%d models in this library.\n",i);

	while (i)
	{

		i--;
		psiM = *(table++);
//		utilPrintf("#%d $%x (%s)\n",i,psiM,(psiM+(int)addr));
		psiM += (int)addr;
		PSIname = (char*)(psiM);
		(char*)crcs = (char*)(psiM+16);
//		utilPrintf("CRC=%x\n",*crcs);
		PSIFixup( (char*)(psiM+20) );
	}
	
	pilLibraryList[pilLibraryLen] = (long*)addr;
	pilLibraryLen++;
	utilPrintf("Libray Loaded\n");

	return NULL;
	
}

/**************************************************************************
	FUNCTION:	DrawLine()
	PURPOSE:	Add line to packet draw list
	PARAMETERS:	Depth value
	RETURNS:	
**************************************************************************/

LINE_F2	polyLine;

void PSIDrawLine(ULONG depth)
{
	LINE_F2	*si;
	
	BEGINPRIM(si,LINE_F2);
  	*(ULONG*)&si->r0 = *(ULONG*)&polyLine.r0;
  	*(ULONG*)&si->x0 = *(ULONG*)&polyLine.x0;
  	*(ULONG*)&si->x1 = *(ULONG*)&polyLine.x1;
	si->code = 0x40|PSImodelctrl.semitrans;
  	ENDPRIM(si, depth & 1023, LINE_F2);
}
/**************************************************************************
	FUNCTION:	DrawBox()
	PURPOSE:	Add box to packet draw list
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
void PSIDrawBox(SHORT x,SHORT y,SHORT w,SHORT h,UBYTE r,UBYTE g,UBYTE b,UBYTE semi,SHORT pri)
{

	POLY_F4 *si;
	
	BEGINPRIM(si, POLY_F4);
	
	si->r0=r;
	si->g0=g;
	si->b0=b;
	if (semi==0)si->code=GPU_COM_F4;
	else		si->code=GPU_COM_F4+2;
	si->x0=si->x2=x;
	si->x1=si->x3=x+w;
	si->y0=si->y1=y;
	si->y2=si->y3=y+h;
	ENDPRIM(si, pri & 1023, POLY_F4);
}

/*
void
Get_Screenxy(VERT *v0,LONG *xy){
 	gte_ldv0(v0);		//load vector
 	gte_rtps();        	//RotTransPers1
	gte_stsxy(xy);	 	//store
}
*/

/**************************************************************************
	FUNCTION:	PSISortPrims
	PURPOSE:	create link list table for model's polys
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
void PSISortPrimitives()
{
	register long *tfd = transformedDepths;
	register long *tfv = transformedVertices;
	register TMD_P_GT4I	*opcd;
	PSIMODELCTRL	*modctrl = &PSImodelctrl;
	int				*sl;
	//long			clipflag;


	long deep;
	int prims,primsleft;

	(TMD_P_GT3I*)sl = &sortedIndex[0];

	prims = (int)modctrl->PrimTop;
	primsleft = modctrl->PrimLeft;
	opcd = (TMD_P_GT4I*)( prims  );

	while(primsleft)
	{
		
		switch (opcd->cd & (0xff-2))
		{
/*-----------------------------------------------------------------------------------------------------------------*/
#define op ((TMD_P_FT3I*)opcd)
			case GPU_COM_TF3:
						
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				deep = (tfd[op->v2]-minDepth);

				gte_nclip_b();	// takes 8 cycles
				

				gte_stopz(&clipflag);
				
					
				if (( clipflag<0) || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					sortCount++;
				}
				op++;
				break;

#undef op			
/*-----------------------------------------------------------------------------------------------------------------*/
#define op ((TMD_P_FT4I*)opcd)
			case GPU_COM_TF4:
			
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				deep = (tfd[op->v2]-minDepth);

				gte_nclip_b();	// takes 8 cycles
				

				gte_stopz(&clipflag);
				
					
				if (( clipflag<0) || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					sortCount++;
				}
				op++;
				break;
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define op ((TMD_P_GT3I*)opcd)
			case GPU_COM_TG3:
	
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				deep = (tfd[op->v2]-minDepth);

				gte_nclip_b();	// takes 8 cycles
				

				gte_stopz(&clipflag);
				
					
				if (( clipflag<0) || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					sortCount++;
				}
				
				op++;
				break;
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define op opcd
			case GPU_COM_TG4:

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				deep = (tfd[op->v2]-minDepth);

				gte_nclip_b();	// takes 8 cycles
		
				gte_stopz(&clipflag);
					
				if (( clipflag<0) || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					sortCount++;
				}

				op++;
				break;
#undef op

/*-----------------------------------------------------------------------------------------------------------------*/
/*
#define op ((TMD_P_FT4I*)opcd)
			case GPU_COM_TF4SPR :
			
				deep = (tfd[op->v2]-minDepth);

				op->next = sl[deep];
				sl[deep] = (int)op;
				sortCount++;

				op++;
				break;
#undef op
*/
/*-----------------------------------------------------------------------------------------------------------------*/
#define op ((TMD_P_FG4I*)opcd)
			case GPU_COM_G4:
			case GPU_COM_F4:
   		
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				deep = (tfd[op->v2]-minDepth);

				gte_nclip_b();	// takes 8 cycles
				

				gte_stopz(&clipflag);
				
					
				if (( clipflag<0) || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					sortCount++;
				}

				op++;
				break;
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define op ((TMD_P_FG3I*)opcd)
			case GPU_COM_G3:
			case GPU_COM_F3:

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				deep = (tfd[op->v2]-minDepth);

				gte_nclip_b();	// takes 8 cycles
				

				gte_stopz(&clipflag);
				
					
				if (( clipflag<0) || (op->dummy))
				{
					op->next = sl[deep];
					sl[deep] = (int)op;
					sortCount++;
				}

				op++;
				break;
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
			default:
				break;
		}
		primsleft--;
	}
}


/**************************************************************************
	FUNCTION:	PSIDrawSortedPrims
	PURPOSE:	draw dynamic sorted polys
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void PSIDrawSortedPrimitives(int depth)
{
	register PACKET*		packet;
	register long *tfv = transformedVertices;
	register TMD_P_GT4I	*opcd;

	PSIMODELCTRL	*modctrl = &PSImodelctrl;
	VERT	*vp = modctrl->VertTop;
	int primsleft,lightmode;//,colr;
	VERT 	*tfn = transformedNormals;
//	SVECTOR 	*tfn = (SVECTOR*)transformedNormals;

	ULONG *sorts = sortedIndex;
								  
	primsleft = sortCount;
	if (!primsleft)
		return;

	lightmode = modctrl->lighting;
	opcd = 0;
	while(primsleft)
	{
		if (opcd==0)
		{
			(int)opcd = *(sorts++);
			continue;
		}

		primsleft--;

		switch (opcd->cd & (0xff-2))
		{
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_FT3*)packet)
#define op ((TMD_P_FT3I*)opcd)

			case GPU_COM_TF3:
			
				BEGINPRIM(si, POLY_FT3);

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
			
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);

				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				gte_stsxy3_ft3(si);
			
				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb(&op->r0);
						gte_ldv0(&tfn[op->v0]);
						gte_nccs();			
						gte_strgb(&si->r0);
						break;
					case DIRECTIONONLY:
						gte_ldv0(&tfn[op->v0]);
						gte_ncs();
						gte_strgb(&si->r0);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *) (&si->r0) = *(u_long *) (&op->r0);		// 9 cycles here
		 		}

				setPolyFT3(si);
				//si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_FT3);

				op = op->next;
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_FT4*)packet)
#define op ((TMD_P_FT4I*)opcd)
				
				case GPU_COM_TF4:

				BEGINPRIM(si, POLY_FT4);
   			
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords

		
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				gte_stsxy3_ft4(si);

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);

				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb(&op->r0);
						gte_ldv0(&tfn[op->v0]);
						gte_nccs();			
						gte_strgb(&si->r0);
						break;
					case DIRECTIONONLY:
						gte_ldv0(&tfn[op->v0]);
						gte_ncs();			
						gte_strgb(&si->r0);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *) (&si->r0) = *(u_long *) (&op->r0);		// 9 cycles here
				}
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *)  (&si->u3) = *(u_long *) (&op->tu3);

				setPolyFT4(si);
				//si->code = op->cd | modctrl->semitrans;
 				ENDPRIM(si, depth, POLY_FT4);
				op = op->next;
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_GT3*)packet)
#define op ((TMD_P_GT3I*)opcd)

			case GPU_COM_TG3:

				BEGINPRIM(si, POLY_GT3);

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				gte_stsxy3_gt3(si);

				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);

				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();			
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				}
				setPolyGT3(si);
				//si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_GT3);
				op = op->next;
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_GT4*)packet)
#define op opcd

			case GPU_COM_TG4:
				BEGINPRIM(si, POLY_GT4);

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);
					
				gte_stsxy3_gt4(si);
				
						
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *)  (&si->u3) = *(u_long *) (&op->tu3);

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);
		
				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						gte_ldrgb(&op->r3);
						gte_ldv0(&tfn[op->v3]);
						gte_nccs();			// NormalColorCol
						gte_strgb(&si->r3);
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();			
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						gte_ldv0(&tfn[op->v3]);
						gte_ncs();			// NormalColorCol
						gte_strgb(&si->r3);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r3) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
						*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);
				}
		
				setPolyGT4(si);
				//si->code = op->cd | modctrl->semitrans;
 				ENDPRIM(si, depth, POLY_GT4);
				(int)op = op->next;
				break;

#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_FT4*)packet)
#define op ((TMD_P_FT4I*)opcd)
/*
			case GPU_COM_TF4SPR :
				{
				SHORT		spritez;
				int		width, height;

				si = (POLY_FT4*)packet;
				op = (TMD_P_FT4I*)(opcd);

				BEGINPRIM(si, POLY_FT4);

				testpol = op;
				testsi = si;

	// scaling-and-transform-in-one-go code from the Action Man people...
				width = op->v1;
				gte_SetLDDQB(0);			// clear offset control reg (C2_DQB)
				gte_ldv0(&vp[op->v0]);		// Load centre point
				gte_SetLDDQA(width);		// shove sprite width into control reg (C2_DQA)
				gte_rtps();					// do the rtps
				gte_stsxy(&si->x0);			// get screen x and y
				gte_stsz(&spritez);		// get order table z
	// end of scaling-and-transform


				if (spritez < 20) break;

				gte_stopz(&width);		// get scaled width of sprite
				width >>= 17;

			

 				*(u_long *) & si->r0 = *(u_long *) & op->r0;			// Texture coords / colors
				*(u_long *) & si->u0 = *(u_long *) & op->tu0;
				*(u_long *) & si->u1 = *(u_long *) & op->tu1;
				*(u_long *) & si->u2 = *(u_long *) & op->tu2;
				*(u_long *) & si->u3 = *(u_long *) & op->tu3;

				si->x1 = si->x3=si->x0+width;
				si->x0 = si->x2=si->x0-width;
			
 		 		height = width>>1;//(LONG)(width*(3))/spritez;
			
				si->y2 = si->y3=si->y0+height;
				si->y0 = si->y1=si->y0-height;

				si->code = GPU_COM_TF4 | modctrl->semitrans;
		
 				ENDPRIM(si, depth, POLY_FT4);
				op = op->next;
			}
			break;
*/
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_G4*)packet)
#define op ((TMD_P_FG4I*)opcd)
			case GPU_COM_G4:
			case GPU_COM_F4:
				
				BEGINPRIM(si, POLY_G4);
   			
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);
 
				
				gte_stsxy3_g4(si);


				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						
						gte_ldrgb(&op->r3);
						gte_ldv0(&tfn[op->v3]);
						gte_nccs();			// NormalColorCol
						gte_strgb(&si->r3);
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();			
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						gte_ldv0(&tfn[op->v3]);
						gte_ncs();			// NormalColorCol
						gte_strgb(&si->r3);
						break;

					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r3) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
						*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);
				}


				setPolyG4(si);
				//si->code = op->cd | modctrl->semitrans;
 				ENDPRIM(si, depth, POLY_G4);
				op = op->next;
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_G3*)packet)
#define op ((TMD_P_FG3I*)opcd)
			
			case GPU_COM_G3:
			case GPU_COM_F3:
			
				BEGINPRIM(si, POLY_G3);
   			
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

 
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
				
				gte_stsxy3_g3(si);


				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();			
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				}


				setPolyG3(si);
				//si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth, POLY_G3);
				op = op->next;
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
		default:
			break;
		}
	}
}

/**************************************************************************
	FUNCTION:	PSIDrawPrimitives
	PURPOSE:	add list of polys into ot
	PARAMETERS:	ot (at depth)
	RETURNS:	
**************************************************************************/

void PSIDrawPrimitives(int depth)
{
	register PACKET*		packet;
	register long *tfv = transformedVertices;
	register TMD_P_GT4I	*opcd;
	//long	 clipflag;
	PSIMODELCTRL	*modctrl = &PSImodelctrl;
	VERT	*vp = modctrl->VertTop;
	int prims,primsleft,lightmode;
	USHORT *sorts;
	VERT 	*tfn = transformedNormals;


	prims = (int)modctrl->PrimTop;
	sorts = modctrl->SortOffs;
	primsleft = modctrl->PrimLeft;
	lightmode = modctrl->lighting;

	while(primsleft)
	{
		
		opcd = (TMD_P_GT4I*)( prims + (*sorts++<<3) );
		switch (opcd->cd & (0xff-2))
		{
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_FT3*)packet)
#define op ((TMD_P_FT3I*)opcd)
			case GPU_COM_TF3:

				BEGINPRIM(si, POLY_FT3);

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
			
				gte_nclip_b();	// takes 8 cycles


				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);

	 			gte_stopz(&clipflag);
			
				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) )
					break;									// Back face culling

				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);
				gte_stsxy3_ft3(si);
			
				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb(&op->r0);
						gte_ldv0(&tfn[op->v0]);
						gte_nccs();			
						gte_strgb(&si->r0);
						break;
					case DIRECTIONONLY:
						gte_ldv0(&tfn[op->v0]);
						gte_ncs();			
						gte_strgb(&si->r0);
						break;

					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *) (&si->r0) = *(u_long *) (&op->r0);		// 9 cycles here
		 		}
				si->code = op->cd | modctrl->semitrans;

 				ENDPRIM(si, depth & 1023, POLY_FT3);
				modctrl->polysdrawn++;
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_FT4*)packet)
#define op ((TMD_P_FT4I*)opcd)
			case GPU_COM_TF4:

				BEGINPRIM(si, POLY_FT4);
   			
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
			
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords

				gte_nclip_b();	// takes 8 cycles
		
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				gte_stopz(&clipflag);

				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) )
					break;	// Back face culling
				
				gte_stsxy3_ft4(si);

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);

				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb(&op->r0);
						gte_ldv0(&tfn[op->v0]);
						gte_nccs();			
						gte_strgb(&si->r0);
						break;
					case DIRECTIONONLY:
						gte_ldv0(&tfn[op->v0]);
						gte_ncs();			
						gte_strgb(&si->r0);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *) (&si->r0) = *(u_long *) (&op->r0);		// 9 cycles here
				}
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *)  (&si->u3) = *(u_long *) (&op->tu3);

				si->code = op->cd | modctrl->semitrans;
				
				modctrl->polysdrawn++;
 			
 				ENDPRIM(si, depth & 1023, POLY_FT4);
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_GT3*)packet)
#define op ((TMD_P_GT3I*)opcd)
			case GPU_COM_TG3:
			
				BEGINPRIM(si, POLY_GT3);

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords

				gte_nclip_b();	// takes 8 cycles
				
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
					
				gte_stopz(&clipflag);
				
				if (!(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) )
					break;								// Back face culling
				

				gte_stsxy3_gt3(si);
			

				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);

				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				}
				si->code = op->cd | modctrl->semitrans;
			
				modctrl->polysdrawn++;

				ENDPRIM(si, depth & 1023, POLY_GT3);
				break;
 		
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_GT4*)packet)
#define op opcd
			case GPU_COM_TG4:
		
				BEGINPRIM(si, POLY_GT4);

				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices
				
				*(u_long *)  (&si->u0) = *(u_long *) (&op->tu0);		// Texture coords
			
				gte_nclip_b();	// takes 8 cycles
		
				*(u_long *)  (&si->u1) = *(u_long *) (&op->tu1);
					
				gte_stopz(&clipflag);
				
				if (clipflag >= 0) break;								// Back face culling

				gte_stsxy3_gt4(si);
				
						
				*(u_long *)  (&si->u2) = *(u_long *) (&op->tu2);
				*(u_long *)  (&si->u3) = *(u_long *) (&op->tu3);


				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);
		
		
				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						
						gte_ldrgb(&op->r3);
						gte_ldv0(&tfn[op->v3]);
						gte_nccs();			// NormalColorCol
						gte_strgb(&si->r3);
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						gte_ldv0(&tfn[op->v3]);
						gte_ncs();			
						gte_strgb(&si->r3);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r3) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
						*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);
				}
		
				modctrl->polysdrawn++;
			
				si->code = op->cd | modctrl->semitrans;
				ENDPRIM(si, depth & 1023, POLY_GT4);
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_FT4*)packet)
#define op ((TMD_P_FT4I*)opcd)
			case GPU_COM_TF4SPR :
		   	{
				SHORT		spritez;
				int		width, height;

				si = (POLY_FT4*)packet;
				op = (TMD_P_FT4I*)(opcd);

				BEGINPRIM(si, POLY_FT4);

				testpol = op;
				testsi = si;

	// scaling-and-transform-in-one-go code from the Action Man people...
				width = op->v1;
				gte_SetLDDQB(0);			// clear offset control reg (C2_DQB)
				gte_ldv0(&vp[op->v0]);		// Load centre point
				gte_SetLDDQA(width);		// shove sprite width into control reg (C2_DQA)
				gte_rtps();					// do the rtps
				gte_stsxy(&si->x0);			// get screen x and y
				gte_stsz(&spritez);		// get order table z
	// end of scaling-and-transform


	// tbd - make this ditch according to on-screen SIZE
	// limit to "max poly depth", and we can ditch the "MAXDEPTH" "and" below...


				if (spritez < 20)
					break;

				gte_stopz(&width);		// get scaled width of sprite
				width >>= 17;

			

 				*(u_long *) & si->r0 = *(u_long *) & op->r0;			// Texture coords / colors
				*(u_long *) & si->u0 = *(u_long *) & op->tu0;
				*(u_long *) & si->u1 = *(u_long *) & op->tu1;
				*(u_long *) & si->u2 = *(u_long *) & op->tu2;
				*(u_long *) & si->u3 = *(u_long *) & op->tu3;

				si->x1 = si->x3=si->x0+width;
				si->x0 = si->x2=si->x0-width;
			
 		 		height = width>>1;//(LONG)(width*(3))/spritez;
			
				si->y2 = si->y3=si->y0+height;
				si->y0 = si->y1=si->y0-height;

				si->code = GPU_COM_TF4 | modctrl->semitrans;

				ENDPRIM(si, depth & 1023, POLY_FT4);
				break;
			}	
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_G4*)packet)
#define op ((TMD_P_FG4I*)opcd)
			case GPU_COM_G4:
			case GPU_COM_F4:
				
				BEGINPRIM(si, POLY_G4);
   			
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

				*(u_long *)  (&si->x3) = *(u_long *) (&tfv[op->v3]);
 
 				gte_nclip_b();	// takes 8 cycles
		

				gte_stopz(&clipflag);

	//			if (clipflag >= 0) break; 								// Back face culling
				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) ) break;									// Back face culling
				
				gte_stsxy3_g4(si);


				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						
						gte_ldrgb(&op->r3);
						gte_ldv0(&tfn[op->v3]);
						gte_nccs();			// NormalColorCol
						gte_strgb(&si->r3);
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						gte_ldv0(&tfn[op->v3]);
						gte_ncs();			
						gte_strgb(&si->r3);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r3) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
						*(u_long *)  (&si->r3) = *(u_long *) (&op->r3);
				}


				si->code = op->cd | modctrl->semitrans;

				modctrl->polysdrawn++;

				ENDPRIM(si, depth & 1023, POLY_G4);
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
#define si ((POLY_G3*)packet)
#define op ((TMD_P_FG3I*)opcd)
			
			case GPU_COM_G3:
			case GPU_COM_F3:

				BEGINPRIM(si, POLY_G3);
   			
				gte_ldsxy3(tfv[op->v0], tfv[op->v1], tfv[op->v2]);		// Load 1st three vertices

 
				*(u_long *)  (&si->r0) = *(u_long *) (&op->r0);
 				gte_nclip_b();	// takes 8 cycles
		

				gte_stopz(&clipflag);

	//			if (clipflag >= 0) break; 								// Back face culling
				if ( !(op->dummy & psiDOUBLESIDED) && (clipflag >= 0) ) break;									// Back face culling
				
				gte_stsxy3_g3(si);


				switch (lightmode)
				{
					case DIRECTIONAL:
						gte_ldrgb3(&op->r0, &op->r1, &op->r2);
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_ncct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						break;
					case DIRECTIONONLY:
						gte_ldv3(&tfn[op->v0], &tfn[op->v1], &tfn[op->v2]);
						gte_nct();
						gte_strgb3(&si->r0, &si->r1, &si->r2);
						break;
					case COLOURIZE:
						*(u_long *)  (&si->r0) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r1) = *(u_long *) (&modctrl->col);
						*(u_long *)  (&si->r2) = *(u_long *) (&modctrl->col);
						break;
					default:
						*(u_long *)  (&si->r1) = *(u_long *) (&op->r1);
						*(u_long *)  (&si->r2) = *(u_long *) (&op->r2);
				}


				modctrl->polysdrawn++;
			
				si->code = op->cd | modctrl->semitrans;

				ENDPRIM(si, depth & 1023, POLY_G3);
				break;
#undef si
#undef op
/*-----------------------------------------------------------------------------------------------------------------*/
			default:
				break;
		}
		primsleft--;
	}
}



/**************************************************************************
	FUNCTION:	PSIDrawSegments
	PURPOSE:	draw all object segments of an actor
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void PSIDrawSegments(ACTOR *actor)
{
	register long	*tfv = transformedVertices;
	register long	*tfd = transformedDepths;
	register PSIOBJECT		*world;
	register SVECTOR *np;
	PSIMODELCTRL		*modctrl = &PSImodelctrl;
	LONG			depth;
	LONG			s = modctrl->sorttable;
	int loop,obs,i,j;
	VERT	*tfn;

#ifdef SHOWNORM
	register long	*tfns = transformedScreenN;
#endif

	
	world = actor->object;
	obs = actor->numObjects;

	// transform all the vertices (by heirarchy)
	// tfv == one long list of transformed vertices

	tfn = transformedNormals;
	tfTotal = 0;

	for (loop = 0; loop < obs; loop++)
	{
		world = (PSIOBJECT*)actor->objectTable[loop];

	 	gte_SetRotMatrix(&world->matrixscale);			 
	   	gte_SetTransMatrix(&world->matrixscale);		

		gte_ldv0(&world->meshdata->center);

		modctrl->NormTop = world->meshdata->nortop;

		gte_rtps_b();

		gte_stsz(&world->depth);

		if (modctrl->lighting)
		{
			np = (SVECTOR*)world->meshdata->nortop;
			j = world->meshdata->norn;
			
			for (i=0; i<j; i++)
			{
				gte_ldv0(&np[i]);
				gte_rtv0();
				gte_stsv(&tfn[0]);
				tfn++;
			}
		}
		transformVertexListA(world->meshdata->vertop, world->meshdata->vern, tfv, tfd );

		tfv+=world->meshdata->vern;
		tfd+=world->meshdata->vern;
		tfTotal+=world->meshdata->vern;


	}

//	tfv = (DVECTOR*)transformedVertices;
//	DotToDot(&tfv[21],&tfv[22]);

	// draw all the primitives (by heirarchy)
	// tfv = transformedNormals;

	modctrl->PrimTop = (ULONG*)actor->primitiveList;

//	i = PSIIsVisible(actor);
//	utilPrintf("viz=%d\n",i);
//	if (i==0) return;

	j = modctrl->depthShift;
	if (j==0)
	{
		j = DEFAULTDEPTHSHIFT;
	}

	if (actor->flags & ACTOR_DYNAMICSORT)
	{
	 	actorCalcMaxMin(actor);
		world = (PSIOBJECT*)actor->objectTable[0];

		if (modctrl->depthoverride)
		{
			depth = modctrl->depthoverride;
		}
		else
		{
			depth = world->depth;
		}
		
		modctrl->PrimLeft = actor->noofPrims;
		modctrl->VertTop = world->meshdata->vertop;
		PSISortPrimitives();
		modctrl->polysdrawn = sortCount;
		PSIDrawSortedPrimitives(depth & 1023);
	}
	else
	{
		for (loop = 0; loop < obs; loop++)
		{
			world = (PSIOBJECT*)actor->objectTable[loop];
	
			if (modctrl->depthoverride)
			{
				depth = modctrl->depthoverride;
			}
			else
			{
				depth = world->depth;
			}
	
			modctrl->VertTop = world->meshdata->vertop;
		 	modctrl->SortOffs = world->meshdata->sortlistptr[s];
			modctrl->PrimLeft = world->meshdata->sortlistsize[s];
			PSIDrawPrimitives(depth);
		}
	}
	

#ifdef SHOWNORM
	DrawNormals();
#endif
}


void PSISetRotateKeyFrames(PSIOBJECT *world, ULONG frame,int root)
{		  
	MATRIX		rotmat1;
	SQKEYFRAME	*tmprotatekeys,*tmprotatekeyslast;
	long		keystep;
	long		t;
	PSIMESH		*mesh;
	
	while(world)
	{
		mesh = world->meshdata;
		tmprotatekeys = mesh->rotatekeys;
		
		if ((!frame) || (mesh->numRotateKeys<=1))
		{
			ShortquaternionGetMatrix((SHORTQUAT *)&tmprotatekeys->vect, &world->matrix);
		}
		else
		{
			////////////////////////////////////////////
			// bin search
			////////////////////////////////////////////
			keystep = mesh->numRotateKeys / 2;

			tmprotatekeys = &mesh->rotatekeys[keystep];
			
			if ( keystep>1 )
			{
				keystep /= 2;
			}
			
			while (frame != tmprotatekeys->time)
			{

				if (frame > tmprotatekeys->time)
				{
					if ( frame <= tmprotatekeys[1].time )
					{
						tmprotatekeys++;
						break;
					}
					else
					{
						tmprotatekeys += keystep;
					}
				}
				else
				{
					tmprotatekeys -= keystep;
				}

				if ( keystep>1 )
				{
					keystep /= 2;
				}
			}
			////////////////////////////////////////////
			
			if (tmprotatekeys->time == frame) // it's on the keyframe 
			{
				ShortquaternionGetMatrix((SHORTQUAT *)&tmprotatekeys->vect, &world->matrix);
			}
			else // work out the differences 
			{
				tmprotatekeyslast = tmprotatekeys - 1;

				if((tmprotatekeys->vect.x == tmprotatekeyslast->vect.x) && (tmprotatekeys->vect.y == tmprotatekeyslast->vect.y) && (tmprotatekeys->vect.z == tmprotatekeyslast->vect.z) && (tmprotatekeys->vect.w == tmprotatekeyslast->vect.w))
				{
					ShortquaternionGetMatrix((SHORTQUAT *)&tmprotatekeys->vect, &world->matrix);
				}
				else
				{
//					t =  ((frame - tmprotatekeyslast->time) << 12)/(tmprotatekeys->time - tmprotatekeyslast->time);
					t =  tmprotatekeyslast->time;
					t =  ((frame - t) << 12)/(tmprotatekeys->time - t);
					ShortquaternionSlerpMatrix((SHORTQUAT *)&tmprotatekeyslast->vect,
									(SHORTQUAT *)&tmprotatekeys->vect,
									t,
									&world->matrix);
				}
			}
		}
	   
		RotMatrixYXZ_gte(&world->rotate,&rotmat1);
		gte_MulMatrix0(&rotmat1,&world->matrix,&world->matrix);

		
		if(world->child)
		{
			root++;
			PSISetRotateKeyFrames(world->child,frame,root);
			root--;
		}
		world = world->next;
	}
}

void PSISetScaleKeyFrames(PSIOBJECT *world, ULONG frame)
{
	SVKEYFRAME	*tmpscalekeys,*tmpscalekeyslast;
	USHORT		oldframe=frame;
	LONG		t;
	LONG keystep;
	PSIMESH		*mesh;


	while(world)
	{
		mesh = world->meshdata;
		tmpscalekeys = mesh->scalekeys;

		if ((!frame) || (mesh->numScaleKeys<=1))
		{
			world->scale.vx = (tmpscalekeys->vect.x)<<2;
			world->scale.vy = (tmpscalekeys->vect.y)<<2;
			world->scale.vz = (tmpscalekeys->vect.z)<<2;
		}
		else
		{
			////////////////////////////////////////////
			// bin search
			////////////////////////////////////////////
			keystep = mesh->numScaleKeys / 2;

			tmpscalekeys = &mesh->scalekeys[keystep];
			
			if ( keystep>1 )
			{
				keystep /= 2;
			}
			
			while (frame != tmpscalekeys->time)
			{

				if (frame > tmpscalekeys->time)
				{
					if ( frame <= tmpscalekeys[1].time )
					{
						tmpscalekeys++;
						break;
					}
					else
					{
						tmpscalekeys += keystep;
					}
				}
				else
				{
					tmpscalekeys -= keystep;
				}

				if ( keystep>1 )
				{
					keystep /= 2;
				}
			}
			////////////////////////////////////////////

			if(tmpscalekeys->time == frame)	// it's on the keyframe 
			{
				world->scale.vx = (tmpscalekeys->vect.x)<<2;
				world->scale.vy = (tmpscalekeys->vect.y)<<2;
				world->scale.vz = (tmpscalekeys->vect.z)<<2;
			}
			else // work out the differences 
			{
				tmpscalekeyslast = tmpscalekeys - 1;

				if((tmpscalekeys->vect.x == tmpscalekeyslast->vect.x) && (tmpscalekeys->vect.y == tmpscalekeyslast->vect.y) && (tmpscalekeys->vect.z == tmpscalekeyslast->vect.z))
				{
					world->scale.vx = (tmpscalekeys->vect.x)<<2;
					world->scale.vy = (tmpscalekeys->vect.y)<<2;
					world->scale.vz = (tmpscalekeys->vect.z)<<2;
				}
				else
				{
					VECTOR temp1;
					VECTOR temp2;
					temp1.vx =((int)(tmpscalekeyslast->vect.x))<<2;
					temp1.vy =((int)(tmpscalekeyslast->vect.y))<<2;
					temp1.vz =((int)(tmpscalekeyslast->vect.z))<<2;
					temp2.vx =((int)(tmpscalekeys->vect.x))<<2;
					temp2.vy =((int)(tmpscalekeys->vect.y))<<2;
					temp2.vz =((int)(tmpscalekeys->vect.z))<<2;

					t = tmpscalekeyslast->time;
					t = ((frame - t) << 12) / (tmpscalekeys->time - t);

					gte_lddp(t);							// load interpolant
					gte_ldlvl(&temp1);						// load source
					gte_ldfc(&temp2);						// load dest
					gte_intpl();							// interpolate (8 cycles)
					gte_stlvnl(&world->scale);				// store interpolated vector

				}

			}
		}	

		world->scale.vx = (( world->scale.vx * PSIactorScale->vx ) >> 12);
		world->scale.vy = (( world->scale.vy * PSIactorScale->vy ) >> 12);
		world->scale.vz = (( world->scale.vz * PSIactorScale->vz ) >> 12);

		if(world->child)
		{
			PSISetScaleKeyFrames(world->child,oldframe);
		}

		world = world->next;
	}
}

void PSISetMoveKeyFrames(PSIOBJECT *world, ULONG frame)
{
	
	register SVKEYFRAME	*workingkeys,*tmpmovekeys;
	LONG		t;
	VECTOR		source, dest;
	long		keystep;
	PSIMESH 	*mesh;


	while(world)
	{

		mesh = world->meshdata;
		tmpmovekeys = mesh->movekeys;//+frame;

		if ((!frame) || (mesh->numMoveKeys<=1) )
		{
			world->matrix.t[0] = (tmpmovekeys->vect.x);
			world->matrix.t[1] = -(tmpmovekeys->vect.y);
			world->matrix.t[2] = (tmpmovekeys->vect.z);
		}
		else
		{
			////////////////////////////////////////////
			// bin search
			////////////////////////////////////////////
			keystep = mesh->numMoveKeys / 2;

			tmpmovekeys = &mesh->movekeys[keystep];
			
			if ( keystep>1 )
			{
				keystep /= 2;
			}
			
			while (frame != tmpmovekeys->time)
			{
				if (frame > tmpmovekeys->time)
				{
					if ( frame <= tmpmovekeys[1].time )
					{
						tmpmovekeys++;
						break;
					}
					else
					{
						tmpmovekeys += keystep;
					}
				}
				else
				{
					tmpmovekeys -= keystep;
				}

				if ( keystep>1 )
				{
					keystep /= 2;
				}
			}
			////////////////////////////////////////////

			if(tmpmovekeys->time == frame)	// it's on the keyframe 
			{
				world->matrix.t[0] = (tmpmovekeys->vect.x);
				world->matrix.t[1] = -(tmpmovekeys->vect.y);
				world->matrix.t[2] = (tmpmovekeys->vect.z);
			}
			else // work out the differences 
			{
				workingkeys = tmpmovekeys - 1;

				if((tmpmovekeys->vect.x == workingkeys->vect.x) && (tmpmovekeys->vect.y == workingkeys->vect.y) && (tmpmovekeys->vect.z == workingkeys->vect.z))
				{
					world->matrix.t[0] = (tmpmovekeys->vect.x);
					world->matrix.t[1] = -(tmpmovekeys->vect.y);
					world->matrix.t[2] = (tmpmovekeys->vect.z);
				}
				else
				{
//					t = ((frame - workingkeys->time) << 12) / (tmpmovekeys->time - workingkeys->time);
					t = workingkeys->time;
					t = ((frame - t) << 12) / (tmpmovekeys->time - t);
					
					source.vx = workingkeys->vect.x;
					source.vy = -workingkeys->vect.y;
					source.vz = workingkeys->vect.z;

					dest.vx = tmpmovekeys->vect.x;
					dest.vy = -tmpmovekeys->vect.y;
					dest.vz = tmpmovekeys->vect.z;

					gte_lddp(t);							// load interpolant
					gte_ldlvl(&source);						// load source
					gte_ldfc(&dest);						// load dest
					gte_intpl();							// interpolate (8 cycles)
					gte_stlvnl(&world->matrix.t);			// store interpolated vector
				}
			}
		}	
		if(world->child)
		{
			PSISetMoveKeyFrames(world->child,frame);
		}
		
		world = world->next;
	}
}


void PSISetRotateKeyFrames2(PSIOBJECT *world, ULONG frame0, ULONG frame1, int b)
{		  
	MATRIX		rotmat1;
	SQKEYFRAME	*tmprotatekeys,*tmprotatekeyslast;
	long		keystep;
	long		t;
	PSIMESH		*mesh;
	ULONG		frame[2];
	SHORTQUAT	quat[2];
	int			loop;
	VECTOR		result;

	frame[0] = frame0;
	frame[1] = frame1;
	
	while(world)
	{
		mesh = world->meshdata;

		for(loop = 0; loop < 2; loop ++)
		{
			tmprotatekeys = mesh->rotatekeys;
			
			if ((!frame[loop]) || (mesh->numRotateKeys<=1))
			{
				// get quaternion rather than a matrix
				quat[loop].x = tmprotatekeys->vect.x;
				quat[loop].y = tmprotatekeys->vect.y;
				quat[loop].z = tmprotatekeys->vect.z;
				quat[loop].w = tmprotatekeys->vect.w;
				//ShortquaternionGetMatrix((SHORTQUAT *)&tmprotatekeys->vect, &world->matrix);
			}
			else
			{
				////////////////////////////////////////////
				// bin search
				////////////////////////////////////////////
				keystep = mesh->numRotateKeys / 2;

				tmprotatekeys = &mesh->rotatekeys[keystep];
				
				if ( keystep>1 )
				{
					keystep /= 2;
				}
				
				while (frame[loop] != tmprotatekeys->time)
				{

					if (frame[loop] > tmprotatekeys->time)
					{
						if ( frame[loop] <= tmprotatekeys[1].time )
						{
							tmprotatekeys++;
							break;
						}
						else
						{
							tmprotatekeys += keystep;
						}
					}
					else
					{
						tmprotatekeys -= keystep;
					}

					if ( keystep>1 )
					{
						keystep /= 2;
					}
				}
				////////////////////////////////////////////
				
				if (tmprotatekeys->time == frame[loop]) // it's on the keyframe 
				{
					// get quaternion rather than a matrix
					quat[loop].x = tmprotatekeys->vect.x;
					quat[loop].y = tmprotatekeys->vect.y;
					quat[loop].z = tmprotatekeys->vect.z;
					quat[loop].w = tmprotatekeys->vect.w;
					//ShortquaternionGetMatrix((SHORTQUAT *)&tmprotatekeys->vect, &world->matrix);
				}
				else // work out the differences 
				{
					tmprotatekeyslast = tmprotatekeys - 1;

					if((tmprotatekeys->vect.x == tmprotatekeyslast->vect.x) && (tmprotatekeys->vect.y == tmprotatekeyslast->vect.y) && (tmprotatekeys->vect.z == tmprotatekeyslast->vect.z) && (tmprotatekeys->vect.w == tmprotatekeyslast->vect.w))
					{
						// get quaternion rather than a matrix
						quat[loop].x = tmprotatekeys->vect.x;
						quat[loop].y = tmprotatekeys->vect.y;
						quat[loop].z = tmprotatekeys->vect.z;
						quat[loop].w = tmprotatekeys->vect.w;
						//ShortquaternionGetMatrix((SHORTQUAT *)&tmprotatekeys->vect, &world->matrix);
					}
					else
					{
						// calculate blended quaternion rather than a matrix

						t =  ((frame[loop] - tmprotatekeyslast->time) << 12)/(tmprotatekeys->time - tmprotatekeyslast->time);
						t =  tmprotatekeyslast->time;
						t =  ((frame[loop] - t) << 12)/(tmprotatekeys->time - t);

						

						gte_ld_intpol_sv1(&tmprotatekeyslast->vect);	// load x, y, z
						gte_ld_intpol_sv0(&tmprotatekeys->vect);		// load x, y, z
						gte_lddp(t);			// load interpolant
						gte_intpl();			// interpolate (8 cycles)
					
						quat[loop].w = (((4096 - t) * tmprotatekeyslast->vect.w) + (t * tmprotatekeys->vect.w)) / 4096;
					
						gte_stlvnl(&result);	// put interpolated x y & z into result

						quat[loop].x = result.vx;
						quat[loop].y = result.vy;
						quat[loop].z = result.vz;

						/*
						ShortquaternionSlerpMatrix((SHORTQUAT *)&tmprotatekeyslast->vect,
										(SHORTQUAT *)&tmprotatekeys->vect,
										t,
										&world->matrix);
						*/

					}
				}
			}
		}

		// get matrix from blended quaternions
		//ShortquaternionSlerpMatrix(&quat[0],&quat[1],b,&world->matrix);
		{
			LONG	s, xs,ys,zs, wx,wy,wz, xx,xy,xz, yy,yz,zz/*, cosom, tempcalc*/;
			VECTOR	source, sqrin, sqrout;

			/*
			// calc cosine (dot product)
			cosom = (quat[0].x * quat[1].x + quat[0].y * quat[1].y + quat[0].z * quat[1].z
			+ quat[0].w * quat[1].w);

			// adjust signs (if necessary)
			
			if (cosom < 0)
			{
				//cosom = -cosom;
				quat[1].x = -quat[1].x;
				quat[1].y = -quat[1].y;
				quat[1].z = -quat[1].z;
				quat[1].w = -quat[1].w;
			}
			*/
			   
			// "from" and "to" quaternions are very close 
			//  ... so we can do a linear interpolation
			
			gte_ld_intpol_sv1(&quat[0]);
			gte_ld_intpol_sv0(&quat[1]);

			gte_lddp(b);			// load interpolant
			gte_intpl();			// interpolate (8 cycles)
			
			// interpolate w manually while we wait for the GTE
			source.vx = ((4096 - b) * quat[0].w);
			source.vx += (b * quat[1].w);
			source.vx /= 4096;

			gte_stlvnl(&sqrin);		// put interpolated x y & z into sqrin

			gte_ldlvl(&sqrin);		// load interpolated x y & z
			gte_sqr0();				// square (5 cycles)
			
			// square w while we wait for the GTE
			source.vy = source.vx * source.vx;

			gte_stlvnl(&sqrout);	// put into sqrout

			s = sqrout.vx + sqrout.vy + sqrout.vz + source.vy;

			s = (s >> 12);
				
			s = (s > 1) ? (33554432 / s) : (33554432);
			
			xs = (sqrin.vx * s) >> 12;
			ys = (sqrin.vy * s) >> 12;
			zs = (sqrin.vz * s) >> 12;

			wx = (source.vx * xs) >> 12;
			wy = (source.vx * ys) >> 12;
			wz = (source.vx * zs) >> 12;

			xx = (sqrin.vx * xs) >> 12;
			xy = (sqrin.vx * ys) >> 12;
			xz = (sqrin.vx * zs) >> 12;

			yy = (sqrin.vy * ys) >> 12;
			yz = (sqrin.vy * zs) >> 12;
			zz = (sqrin.vz * zs) >> 12;

			world->matrix.m[0][0] = 4096 - (yy + zz);
			world->matrix.m[0][1] = xy + wz;
			world->matrix.m[0][2] = xz - wy;

			world->matrix.m[1][0] = xy - wz;
			world->matrix.m[1][1] = 4096 - (xx + zz);
			world->matrix.m[1][2] = yz + wx;

			world->matrix.m[2][0] = xz + wy;
			world->matrix.m[2][1] = yz - wx;
			world->matrix.m[2][2] = 4096 - (xx + yy);
		}

		RotMatrixYXZ_gte(&world->rotate,&rotmat1);
		gte_MulMatrix0(&rotmat1,&world->matrix,&world->matrix);
		
		if(world->child)
		{
			PSISetRotateKeyFrames2(world->child,frame0, frame1, b);
		}
		world = world->next;
	}
}

void PSISetScaleKeyFrames2(PSIOBJECT *world, ULONG frame0, ULONG frame1, int b)
{
	SVKEYFRAME	*tmpscalekeys,*tmpscalekeyslast;
	LONG		t;
	LONG		keystep;
	PSIMESH		*mesh;
	SVECTOR		scale[3];
	ULONG		frame[2];
	int			loop;

	frame[0] = frame0;
	frame[1] = frame1;


	while(world)
	{
		mesh = world->meshdata;

		for(loop = 0; loop < 2; loop ++)
		{
			tmpscalekeys = mesh->scalekeys;

			if ((!frame[loop]) || (mesh->numScaleKeys<=1))
			{
				scale[loop].vx = (tmpscalekeys->vect.x)<<2;
				scale[loop].vy = (tmpscalekeys->vect.y)<<2;
				scale[loop].vz = (tmpscalekeys->vect.z)<<2;
				/*
				world->scale.vx = (tmpscalekeys->vect.x)<<2;
				world->scale.vy = (tmpscalekeys->vect.y)<<2;
				world->scale.vz = (tmpscalekeys->vect.z)<<2;
				*/
			}
			else
			{
				////////////////////////////////////////////
				// bin search
				////////////////////////////////////////////
				keystep = mesh->numScaleKeys / 2;

				tmpscalekeys = &mesh->scalekeys[keystep];
				
				if ( keystep>1 )
				{
					keystep /= 2;
				}
				
				while (frame[loop] != tmpscalekeys->time)
				{

					if (frame[loop] > tmpscalekeys->time)
					{
						if ( frame[loop] <= tmpscalekeys[1].time )
						{
							tmpscalekeys++;
							break;
						}
						else
						{
							tmpscalekeys += keystep;
						}
					}
					else
					{
						tmpscalekeys -= keystep;
					}

					if ( keystep>1 )
					{
						keystep /= 2;
					}
				}
				////////////////////////////////////////////

				if(tmpscalekeys->time == frame[loop])	// it's on the keyframe 
				{
					scale[loop].vx = (tmpscalekeys->vect.x)<<2;
					scale[loop].vy = (tmpscalekeys->vect.y)<<2;
					scale[loop].vz = (tmpscalekeys->vect.z)<<2;
					/*
					world->scale.vx = (tmpscalekeys->vect.x)<<2;
					world->scale.vy = (tmpscalekeys->vect.y)<<2;
					world->scale.vz = (tmpscalekeys->vect.z)<<2;
					*/
				}
				else // work out the differences 
				{
					tmpscalekeyslast = tmpscalekeys - 1;

					if((tmpscalekeys->vect.x == tmpscalekeyslast->vect.x) && (tmpscalekeys->vect.y == tmpscalekeyslast->vect.y) && (tmpscalekeys->vect.z == tmpscalekeyslast->vect.z))
					{
						scale[loop].vx = (tmpscalekeys->vect.x)<<2;
						scale[loop].vy = (tmpscalekeys->vect.y)<<2;
						scale[loop].vz = (tmpscalekeys->vect.z)<<2;
						/*
						world->scale.vx = (tmpscalekeys->vect.x)<<2;
						world->scale.vy = (tmpscalekeys->vect.y)<<2;
						world->scale.vz = (tmpscalekeys->vect.z)<<2;
						*/

					}
					else
					{
						VECTOR temp1;
						VECTOR temp2;
						VECTOR dest;
						
						temp1.vx =((int)(tmpscalekeyslast->vect.x))<<2;
						temp1.vy =((int)(tmpscalekeyslast->vect.y))<<2;
						temp1.vz =((int)(tmpscalekeyslast->vect.z))<<2;
						temp2.vx =((int)(tmpscalekeys->vect.x))<<2;
						temp2.vy =((int)(tmpscalekeys->vect.y))<<2;
						temp2.vz =((int)(tmpscalekeys->vect.z))<<2;
						
						t = tmpscalekeyslast->time;
						t = ((frame[loop] - t) << 12) / (tmpscalekeys->time - t);

						gte_lddp(t);							// load interpolant
						gte_ldlvl(&temp1);						// load source
						gte_ldfc(&temp2);						// load dest
						gte_intpl();							// interpolate (8 cycles)
						gte_stlvnl(&dest);				// store interpolated vector
						scale[loop].vx = dest.vx;
						scale[loop].vy = dest.vy;
						scale[loop].vz = dest.vz;

					}

				}
			}
		}

		// interpolate between scale values
		gte_ld_intpol_sv1(&scale[0]);
		gte_ld_intpol_sv0(&scale[1]);
		gte_lddp(b);
		gte_intpl();
		gte_stlvnl(&world->scale);

		world->scale.vx = (( world->scale.vx * PSIactorScale->vx ) >> 12);
		world->scale.vy = (( world->scale.vy * PSIactorScale->vy ) >> 12);
		world->scale.vz = (( world->scale.vz * PSIactorScale->vz ) >> 12);

		if(world->child)
		{
			PSISetScaleKeyFrames2(world->child,frame0, frame1, b);
		}

		world = world->next;
	}
}

void PSISetMoveKeyFrames2(PSIOBJECT *world, ULONG frame0, ULONG frame1, int b)
{
	
	register SVKEYFRAME	*workingkeys,*tmpmovekeys;
	LONG		t;
	VECTOR		dest;
	long		keystep;
	PSIMESH 	*mesh;
	SVECTOR		move[3];
	int			loop;
	ULONG		frame[2];


	frame[0] = frame0;
	frame[1] = frame1;


	while(world)
	{

		mesh = world->meshdata;

		for(loop = 0; loop < 2; loop ++)
		{
			tmpmovekeys = mesh->movekeys;//+frame;

			if ((!frame[loop]) || (mesh->numMoveKeys<=1) )
			{
				move[loop].vx = (tmpmovekeys->vect.x);
				move[loop].vy = -(tmpmovekeys->vect.y);
				move[loop].vz = (tmpmovekeys->vect.z);

				/*
				world->matrix.t[0] = (tmpmovekeys->vect.x);
				world->matrix.t[1] = -(tmpmovekeys->vect.y);
				world->matrix.t[2] = (tmpmovekeys->vect.z);
				*/
			}
			else
			{
				////////////////////////////////////////////
				// bin search
				////////////////////////////////////////////
				keystep = mesh->numMoveKeys / 2;

				tmpmovekeys = &mesh->movekeys[keystep];
				
				if ( keystep>1 )
				{
					keystep /= 2;
				}
				
				while (frame[loop] != tmpmovekeys->time)
				{
					if (frame[loop] > tmpmovekeys->time)
					{
						if ( frame[loop] <= tmpmovekeys[1].time )
						{
							tmpmovekeys++;
							break;
						}
						else
						{
							tmpmovekeys += keystep;
						}
					}
					else
					{
						tmpmovekeys -= keystep;
					}

					if ( keystep>1 )
					{
						keystep /= 2;
					}
				}
				////////////////////////////////////////////

				if(tmpmovekeys->time == frame[loop])	// it's on the keyframe 
				{
					move[loop].vx = (tmpmovekeys->vect.x);
					move[loop].vy = -(tmpmovekeys->vect.y);
					move[loop].vz = (tmpmovekeys->vect.z);
					/*
					world->matrix.t[0] = (tmpmovekeys->vect.x);
					world->matrix.t[1] = -(tmpmovekeys->vect.y);
					world->matrix.t[2] = (tmpmovekeys->vect.z);
					*/
				}
				else // work out the differences 
				{
					workingkeys = tmpmovekeys - 1;

					if((tmpmovekeys->vect.x == workingkeys->vect.x) && (tmpmovekeys->vect.y == workingkeys->vect.y) && (tmpmovekeys->vect.z == workingkeys->vect.z))
					{
						move[loop].vx = (tmpmovekeys->vect.x);
						move[loop].vy = -(tmpmovekeys->vect.y);
						move[loop].vz = (tmpmovekeys->vect.z);
						/*
						world->matrix.t[0] = (tmpmovekeys->vect.x);
						world->matrix.t[1] = -(tmpmovekeys->vect.y);
						world->matrix.t[2] = (tmpmovekeys->vect.z);
						*/
					}
					else
					{
	//					t = ((frame - workingkeys->time) << 12) / (tmpmovekeys->time - workingkeys->time);
						t = workingkeys->time;
						t = ((frame[loop] - t) << 12) / (tmpmovekeys->time - t);
						
						gte_ld_intpol_sv1(&workingkeys->vect);
						gte_ld_intpol_sv0(&tmpmovekeys->vect);

						gte_lddp(t);							// load interpolant
						//gte_ldlvl(&source);					// load source
						//gte_ldfc(&dest);						// load dest
						gte_intpl();							// interpolate (8 cycles)
						gte_stlvnl(&dest);						// store interpolated vector

						move[loop].vx = dest.vx;
						move[loop].vy = -dest.vy;
						move[loop].vz = dest.vz;
					}
				}
			}
		}

		// interpolate between frame0 and frame1
		
		gte_lddp(b);							// load interpolant
		gte_ld_intpol_sv1(&move[0]);
		gte_ld_intpol_sv0(&move[1]);
		gte_intpl();							// interpolate (8 cycles)
		gte_stlvnl(&world->matrix.t);			// store interpolated vector

		if(world->child)
		{
			PSISetMoveKeyFrames2(world->child, frame0, frame1, b);
		}
		
		world = world->next;
	}
}


/**************************************************************************
	FUNCTION:	SetAnimation()
	PURPOSE:	Set keyframe information for current object frame
	PARAMETERS:	Model structure, frame number
	RETURNS:	
**************************************************************************/


void PSISetAnimation(ACTOR *actor, ULONG frame)
{
	PSIOBJECT *world;
	ACTOR_ANIMATION *actorAnim = &actor->animation;
	animation *anim;
	long temp0,temp1,temp2;
	VECTOR result;

	world = actor->object;
	
	PSIactorScale = &actor->size;

	PSISetMoveKeyFrames(world, frame);

	PSISetScaleKeyFrames(world, frame);

	PSISetRotateKeyFrames(world, frame,0);
	
	
	actor->accumulator.vx = world->matrix.t[0];
	actor->accumulator.vy = world->matrix.t[1];
	actor->accumulator.vz = world->matrix.t[2];

	world->matrix.t[0] -= actor->oldPosition.vx;
	world->matrix.t[1] -= actor->oldPosition.vy;
	world->matrix.t[2] -= actor->oldPosition.vz;

//	utilPrintf("add %d (%d)\n",world->matrix.t[1],actor->oldPosition.vy);

	actor->oldPosition = actor->accumulator;

	temp0 = world->matrix.t[0];
	temp1 = world->matrix.t[1];
	temp2 = world->matrix.t[2];
	
	MATHS_Rotate(world->rotate.vx,world->rotate.vy,world->rotate.vz,temp0,temp1,temp2,&result);

	world->matrix.t[0] = result.vx;
	world->matrix.t[1] = result.vy;
	world->matrix.t[2] = result.vz;
	

	//	utilPrintf("y=%d\n",world->matrix.t[1]);

	if (actorAnim->exclusive)  
	{
		anim = (animation*) (actor->animSegments + (actorAnim->currentAnimation*2));
		if(actorAnim->frame >= anim->animEnd)
		{
			actorAnim->frame = anim->animStart;
			actorAnim->animTime = anim->animStart<<ANIMSHIFT;
			actorAdjustPosition(actor);
		}
		else 
		{
			if(actorAnim->frame < anim->animStart)
			{
				actorAnim->frame = anim->animEnd;
				actorAnim->animTime = anim->animEnd <<ANIMSHIFT;
				actorAdjustPosition(actor);
			}
		}
	}		
		// Store Root Bone movement
		// Use Basic Position - IE Set root bone to zero


}

void PSISetAnimation2(ACTOR *actor, ULONG frame0, ULONG frame1, ULONG blend)
{
	PSIOBJECT *world;
	ACTOR_ANIMATION *actorAnim = &actor->animation;
	animation *anim;
	long temp0,temp1,temp2;
	VECTOR result;

	world = actor->object;
	
	PSIactorScale = &actor->size;

	PSISetMoveKeyFrames2(world, frame0, frame1, blend);

	PSISetScaleKeyFrames2(world, frame0, frame1, blend);

	PSISetRotateKeyFrames2(world, frame0, frame1, blend);
	
	/*
	actor->accumulator.vx = world->matrix.t[0];
	actor->accumulator.vy = world->matrix.t[1];
	actor->accumulator.vz = world->matrix.t[2];

	world->matrix.t[0] -= actor->oldPosition.vx;
	world->matrix.t[1] -= actor->oldPosition.vy;
	world->matrix.t[2] -= actor->oldPosition.vz;

//	utilPrintf("add %d (%d)\n",world->matrix.t[1],actor->oldPosition.vy);

	actor->oldPosition = actor->accumulator;

	temp0 = world->matrix.t[0];
	temp1 = world->matrix.t[1];
	temp2 = world->matrix.t[2];
	
	MATHS_Rotate(world->rotate.vx,world->rotate.vy,world->rotate.vz,temp0,temp1,temp2,&result);

	world->matrix.t[0] = result.vx;
	world->matrix.t[1] = result.vy;
	world->matrix.t[2] = result.vz;
	*/

	//	utilPrintf("y=%d\n",world->matrix.t[1]);

	if (actorAnim->exclusive)  
	{
		anim = (animation*) (actor->animSegments + (actorAnim->currentAnimation*2));
		if(actorAnim->frame >= anim->animEnd)
		{
			actorAnim->frame = anim->animStart;
			actorAnim->animTime = anim->animStart<<ANIMSHIFT;
			actorAdjustPosition(actor);
		}
		else 
		{
			if(actorAnim->frame < anim->animStart)
			{
				actorAnim->frame = anim->animEnd;
				actorAnim->animTime = anim->animEnd <<ANIMSHIFT;
				actorAdjustPosition(actor);
			}
		}
	}		
		// Store Root Bone movement
		// Use Basic Position - IE Set root bone to zero


}

/**************************************************************************
	FUNCTION:	updateAnimation()
	PURPOSE:	Set keyframe information for current object frame
	PARAMETERS:	Model structure, frame number
	RETURNS:	
**************************************************************************/

void PSIUpdateAnimation()
{
	int loop;
	ACTOR *actor;
	
	loop = actorList.numEntries;
	actor = actorList.head.next;
	while (loop)
	{

		actorUpdateAnimations(actor);
		actor = actor->next;
		loop--;

	}	
}


/**************************************************************************
	FUNCTION:
	PURPOSE:	calc matrices relative to camera
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void PSICalcChildMatrix(PSIOBJECT *world,PSIOBJECT *parent)
{

	while(world)
	{
	   	world->matrixscale = world->matrix;
	   	ScaleMatrix(&world->matrixscale,&world->scale);

  		gte_MulMatrix0(&parent->matrix, &world->matrix, &world->matrix);
  		gte_MulMatrix0(&parent->matrix, &world->matrixscale, &world->matrixscale);

  		gte_SetRotMatrix(&parent->matrixscale);
  		gte_SetTransMatrix(&parent->matrixscale);
  		gte_ldlvl(&world->matrix.t);
  		gte_rtirtr();
  		gte_stlvl(&world->matrixscale.t);
  
  		if(world->child)
		{
			PSICalcChildMatrix(world->child,world);
		}

		world = world->next;
	}
}

/**************************************************************************
	FUNCTION:
	PURPOSE:	calc matrices relative to camera
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void PSICalcWorldMatrix(PSIOBJECT *world)
{
	cameraAndGlobalscale = GsWSMATRIX;
	ScaleMatrix(&cameraAndGlobalscale, PSIrootScale);

//	while(world)
//	{
	world->matrixscale = world->matrix;
	ScaleMatrix(&world->matrixscale,&world->scale);

	gte_MulMatrix0(&cameraAndGlobalscale, &world->matrix, &world->matrix);
//	gte_MulMatrix0(&cameraAndGlobalscale, &world->matrixscale, &world->matrixscale);
	gte_MulMatrix0(&GsWSMATRIX, &world->matrixscale, &world->matrixscale);
	gte_SetRotMatrix(&GsWSMATRIX);
	gte_SetTransMatrix(&GsWSMATRIX);
	gte_ldlvl(&world->matrixscale.t);
	gte_rtirtr();
	gte_stlvl(&world->matrixscale.t);

	if(world->child)
	{
		PSICalcChildMatrix(world->child,world);
	}

//		world = world->next;
//	}
}

/**************************************************************************
	FUNCTION:
	PURPOSE:	calc matrices relative to model origin
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void PSICalcLocalMatrix(PSIOBJECT *world,MATRIX *parentM,MATRIX *parentMS)
{

	while(world)
	{

		if (parentM==0)
		{
			world->matrix.t[0] = 0;
			world->matrix.t[1] = 0;
			world->matrix.t[2] = 0;
		}
	   	
	   	world->matrixscale = world->matrix;
	   	ScaleMatrix(&world->matrixscale,&world->scale);

		if (parentM!=0)
	   	{
			gte_MulMatrix0(parentM, &world->matrix, &world->matrix);
			gte_MulMatrix0(parentM, &world->matrixscale, &world->matrixscale);

			gte_SetRotMatrix(parentMS);
			gte_SetTransMatrix(parentMS);
			gte_ldlvl(&world->matrix.t);
			gte_rtirtr();
			gte_stlvl(&world->matrixscale.t);
		}
		else
		{
			gte_MulMatrix0(PSIrootScale, &world->matrix, &world->matrix);
		}

		if(world->child)
		{
			PSICalcLocalMatrix(world->child,&world->matrix,&world->matrixscale);
		}

		world = world->next;
	}
}

/**************************************************************************
	FUNCTION:
	PURPOSE:	find min and max xyz from model 'pose'
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

int PSICalcSegments(ACTOR *actor)
{
	register PSIOBJECT	*world;
	int loop,obs,i,j,q,x,y,z,s;
	long flg;
	VECTOR	tfv;
	short *verts;
	SVECTOR v;
	BOUNDINGBOX *corners;

	q = 0;
	obs = actor->numObjects;
	corners = &actor->bounding;

	corners->minX = corners->maxX = 0;
	corners->minY = corners->maxY = 0;
	corners->minZ = corners->maxZ = 0;

	for (loop = 0; loop < obs; loop++)
	{
		world = (PSIOBJECT*)actor->objectTable[loop];

	 	gte_SetRotMatrix(&world->matrixscale);
	   	gte_SetTransMatrix(&world->matrixscale);

		verts=(short*)world->meshdata->vertop;
		j = world->meshdata->vern;

		for (i=0; i < j; i++)
		{

			v.vx = *(verts++);
			v.vy = *(verts++);
			v.vz = *(verts++);

			RotTrans(&v,&tfv,&flg);
			
			verts++;

			x = tfv.vx;
			y = tfv.vy;
			z = tfv.vz;


			if ( x < corners->minX ) corners->minX = x;
			if ( x > corners->maxX ) corners->maxX = x;
			if ( y < corners->minY ) corners->minY = y;
			if ( y > corners->maxY ) corners->maxY = y;
			if ( z < corners->minZ ) corners->minZ = z;
			if ( z > corners->maxZ ) corners->maxZ = z;
							 

			s = abs(x*x) + abs(y*y) + abs(z*z);

			if (s>q)
			{
				q = s;
			}
		}
	}

  	j = utilSqrt(q) >> 16;
//	utilPrintf("rad=%d (%d)\n",j,actor->radius);

	return j;	
}

/**************************************************************************
	FUNCTION:	PSISetBoundingRotated
	PURPOSE:	set bounding box values for animation frame & rotations
	PARAMETERS:	
	RETURNS:	sets values in actor structure's bounding box
**************************************************************************/
void PSISetBoundingRotated(ACTOR *actor,int frame,int rotX,int rotY, int rotZ)
{
	int x,y,z;

	// save rotations
	
	x = actor->object->rotate.vx;	
	y = actor->object->rotate.vy;	
	z = actor->object->rotate.vz;	

	actor->object->rotate.vx = rotX;		
	actor->object->rotate.vy = rotY;
	actor->object->rotate.vz = rotZ;

	// set the model 'pose'
	PSISetAnimation(actor, frame);

	PSIrootScale = &actor->object->scale;
	PSICalcLocalMatrix(actor->object,0,0);

	PSICalcSegments(actor);

	// restore rotations
		
	actor->object->rotate.vx = x;		
	actor->object->rotate.vy = y;
	actor->object->rotate.vz = z;
}

/**************************************************************************
	FUNCTION:	PSISetBounding
	PURPOSE:	as PSISetBoundingRotated but forces 0 rotation
	PARAMETERS:	
	RETURNS:	sets values in actor structure's bounding box
**************************************************************************/
void PSISetBounding(ACTOR *actor,int frame)
{
		PSISetBoundingRotated(actor,frame,0,0,0);
}

/**************************************************************************
	FUNCTION:
	PURPOSE:	using actor's preset radius, see if it's screen vivible
	PARAMETERS:	
	RETURNS:	true / false
**************************************************************************/

UBYTE PSIIsVisible(ACTOR *actor)
{
	//MATRIX *localM;
	SVECTOR v;
	long zed,diff,lx,rx,ty,by;
	DVECTOR	scrxy1;
	DVECTOR	scrxy2;

	gte_SetRotMatrix(&GsWSMATRIX);					// load viewpoint matrices
	gte_SetTransMatrix(&GsWSMATRIX);

	v.vx = actor->position.vx;						// use actor's world position
	v.vy = actor->position.vy;
	v.vz = actor->position.vz;

	gte_ldv0(&v);									// RotTrans world position
	gte_rt();
	gte_stsz(&zed);
	if ( zed <= 0)
	{
		return FALSE;					// return if 'behind' camera
	}
  

	if ((zed-actor->radius)>PSImodelctrl.farclip)
	{
		return FALSE;								// To far away
	}

//	localM = &actor->object->matrix;				// local transformation matrix.t includes world position
//	v.vx = localM->t[0];
//	v.vy = localM->t[1];
//	v.vz = localM->t[2];

	gte_ldv0(&v);									// transform 'center' x,y
	gte_rtps();
	gte_stsxy(&scrxy1);

	v.vx += actor->radius;							// add model radius
	gte_ldv0(&v);									// transform radial x,y
	gte_rtps();
	gte_stsxy(&scrxy2);
	
	diff = abs(scrxy1.vx- scrxy2.vx);				// screen distance to outer edge of radius

	lx = (scrxy1.vx - diff);
	rx = (scrxy1.vx + diff);
	diff /= 2;
	ty = (scrxy1.vy - diff);
	by = (scrxy1.vy + diff);

	if ( 	(lx < 320) &&
			(rx > -320)   &&
			(ty < 120) &&
			(by > -120 ))
	{
		return TRUE;
	}

	return FALSE;


}

/**************************************************************************
	FUNCTION:	
	PURPOSE:	move actor at actor->position
	PARAMETERS:	ACTOR *
	RETURNS:	
**************************************************************************/

void PSIMoveActor(ACTOR *actor)
{
	int i,v,rotcalc,rotate;
	PSIOBJECT *world;
	register PSIMODELCTRL	*modctrl = &PSImodelctrl;

   	world = actor->object;

	if ( !(actor->flags & ACTOR_DYNAMICSORT) )
	{
		v = 0;
		if (-PSImodelctrl.whichcamera->vpy < actor->position.vy)
		{
			v=4;
		}

		rotcalc=utilCalcAngle(	PSImodelctrl.whichcamera->vpx - actor->position.vx,
							PSImodelctrl.whichcamera->vpz - actor->position.vz)&0xffff;
		rotate=world->rotate.vy+2048; 

		i = utilCalcAngle(	PSImodelctrl.whichcamera->vpx - actor->position.vx,
						PSImodelctrl.whichcamera->vpz - actor->position.vz)&0xffff;
		i=((((rotcalc+rotate)&4095)/1024))&3;
		i += v;
		modctrl->sorttable=i;
	}

	PSISetAnimation(actor, actor->animation.frame);


	if (actor->flags&ACTOR_MOTIONBONE)
	{
		if (actor->flags&ACTOR_HOLDMOTION)
		{
			world->matrix.t[0] = actor->position.vx;
			world->matrix.t[1] = actor->position.vy;
			world->matrix.t[2] = actor->position.vz;
		}
		else
		{
			world->matrix.t[0] += actor->position.vx;
			world->matrix.t[1] += actor->position.vy;
			world->matrix.t[2] += actor->position.vz;
			actor->position.vx = world->matrix.t[0];
			actor->position.vy = world->matrix.t[1];
			actor->position.vz = world->matrix.t[2];
		}
	}
}
/**************************************************************************
	FUNCTION:	
	PURPOSE:	draw actor at actor->position
	PARAMETERS:	ACTOR *
	RETURNS:	
**************************************************************************/

void PSIDrawActor(ACTOR *actor)
{
	//int i,v,rotcalc,rotate;
	PSIOBJECT *world;
	//register PSIMODELCTRL	*modctrl = &PSImodelctrl;

   	world = actor->object;

	if(actor->flags&ACTOR_MOTIONBONE)
	{
		world->matrix.t[0] = actor->position.vx;
		world->matrix.t[1] = actor->position.vy;
		world->matrix.t[2] = actor->position.vz;
	}
	else
	{
		world->matrix.t[0] += actor->position.vx;
		world->matrix.t[1] += actor->position.vy;
		world->matrix.t[2] += actor->position.vz;
	}

	PSIrootScale = &actor->object->scale;
	
	PSICalcWorldMatrix(actor->object);
//	PSICalcMatrix(actor->object);

	PSIDrawSegments(actor);
}

/**************************************************************************
	FUNCTION:	
	PURPOSE:	draw all actors in actorList
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
void PSIDraw()
{

	int loop;
	PSIOBJECT *world;
	ACTOR *tempActor;

 	GsSetAmbient(ambience.r<<4,ambience.g<<4,ambience.b<<4);

	loop = actorList.numEntries;
	tempActor = actorList.head.next;
	
	while (loop)
	{
		world = tempActor->object;
		if (loop==1)
		{
			world->rotate.vy = squat.y;
		}

		PSIMoveActor(tempActor);

		PSIDrawActor(tempActor);

		tempActor = tempActor->next;
		loop--;
	}
}



void PSIDebug()
{

}

/**************************************************************************/
// ANIMATION FUNCTIONS
/**************************************************************************/




/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
void actorInitAnim(ACTOR *actor)
{
	actor->animation.currentAnimation = -1;

	actor->animation.queueAnimation[0] = -1;
	actor->animation.queueAnimation[1] = -1;
	actor->animation.queueAnimation[2] = -1;
	actor->animation.queueAnimation[3] = -1;
	actor->animation.queueAnimation[4] = -1;
	actor->animation.loopAnimation = YES;//NO;
	actor->animation.numberQueued = 0;
	actor->animation.animTime = 0;
	actor->animation.reachedEndOfAnimation = FALSE;
}

/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
void actorFlushQueue(ACTOR *actor)
{
	int i;

	actor->animation.numberQueued = 0;

	for(i = 0;i < ANIM_QUEUE_LENGTH;i++)
	{
		actor->animation.queueAnimation[i] = -1;
		actor->animation.queueAnimationSpeed[i] = 1;
	}
}
/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
void actorAnimate(ACTOR *actor, int animNum, char loop, char queue, int speed, char skipendframe)
{
	ACTOR_ANIMATION *actorAnim = &actor->animation;
	animation *anim;
	int	actualSpeed;

	
	if(actorAnim->numAnimations == 0)
	{
		return;
	}

	if( animNum >= actorAnim->numAnimations )
	{
		utilPrintf("anim index number out of range.\n");
		return;
	}

	
	if( (actorAnim->currentAnimation == animNum) &&
		(queue == NO) && 
		(actorAnim->reachedEndOfAnimation == FALSE) &&
		( speed == actorAnim->animationSpeed) )
	{
		actorAnim->loopAnimation = loop;
		actorFlushQueue(actor);
		return;
	}

//	if(animNum > actorAnim->numAnimations) animNum = 0;

	if(speed == 0)
	{
		actualSpeed = 256;
	}
	else
	{
		actualSpeed = speed;
	}

	actorAnim->exclusive = skipendframe;
	
	if(queue == NO)
	{
		actorAdjustPosition(actor);
		actorAnim->currentAnimation = animNum;
		actorAnim->loopAnimation = loop;
		actorAnim->animationSpeed = actualSpeed;
		actorAnim->reachedEndOfAnimation = FALSE;
		anim = (animation*)(actor->animSegments + (actorAnim->currentAnimation*2));
		actorAnim->animTime = anim->animStart << ANIMSHIFT;
		actorAnim->frame = anim->animStart;
		actorFlushQueue(actor);
		if(actualSpeed < 0)
		{
			actorAnim->animTime = anim->animEnd << ANIMSHIFT;
		}
	}
	else
	{
		if(actorAnim->currentAnimation == -1)
		{
			actorAnim->currentAnimation = animNum;
			actorAnim->loopAnimation = loop;
			actorAnim->animationSpeed = actualSpeed;
		}
		else
		{
			if(actorAnim->numberQueued == 5)
			{
				return;
			}
			actorAnim->queueAnimation[actorAnim->numberQueued] = animNum;
			actorAnim->queueLoopAnimation[actorAnim->numberQueued] = loop;
			actorAnim->queueAnimationSpeed[actorAnim->numberQueued] = actualSpeed;
			actorAnim->numberQueued++;
		}
	}

}

/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
void actorSetAnimationSpeed(ACTOR *actor, int speed)
{
	actor->animation.animationSpeed = speed;
}

/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
void actorAdjustPosition(ACTOR *actor)
{
//	utilPrintf("y = %d\n",actor->oldPosition.vy);

	actor->oldPosition.vx = 0;//actor->position.vx;// actor->accumulator.vx;
	actor->oldPosition.vy = 0;//actor->position.vy;//actor->accumulator.vy;
	actor->oldPosition.vz = 0;//actor->position.vz;//actor->accumulator.vz;
	

	actor->accumulator.vx = 0;
	actor->accumulator.vy = 0;
	actor->accumulator.vz = 0;

//	actor->position = actor->oldPosition;

//	actor->position.vx = actor->position.vy = actor->position.vz = 0;

}
/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
void actorUpdateAnimations(ACTOR *actor)
{
	ACTOR_ANIMATION *actorAnim = &actor->animation;
	animation *anim;
	int i;

	//reject all objects which have no animation
	if(actorAnim->numAnimations == 0)
		return;

	if(actorAnim->currentAnimation < 0)
		return;

	anim = (animation*) (actor->animSegments + (actorAnim->currentAnimation*2));
	
	actorAnim->reachedEndOfAnimation = FALSE;
	actorAnim->animTime += actorAnim->animationSpeed;

	actorAnim->frame = actorAnim->animTime >> ANIMSHIFT;
	
	if( ((actorAnim->frame > anim->animEnd) || (actorAnim->frame < anim->animStart)) &&
	 (actorAnim->loopAnimation == NO))
	{
		actorAnim->frame = Bound(actorAnim->frame,anim->animStart,anim->animEnd);
		actorAnim->animTime = actorAnim->frame << ANIMSHIFT;
		actorAnim->reachedEndOfAnimation = actorAnim->currentAnimation+1;

		//queue stuff - do this later
		if(actorAnim->queueAnimation[0] != -1)
		{
//			SetAnimCallBack(temp, actorAnim->anims[actorAnim->queueAnimation[0]].animationSet);

			actorAnim->currentAnimation = actorAnim->queueAnimation[0];//actorAnim;
			actorAnim->loopAnimation = actorAnim->queueLoopAnimation[0];//loop;
			actorAnim->animationSpeed = actorAnim->queueAnimationSpeed[0];//speed;
			if(actorAnim->animationSpeed < 0)
			{
				actorAnim->animTime = actor->animSegments[actorAnim->currentAnimation*2 +1] <<ANIMSHIFT;
			}
			else
			{
				actorAnim->animTime = actor->animSegments[actorAnim->currentAnimation*2] <<ANIMSHIFT;
			}

			for(i = 0;i < ANIM_QUEUE_LENGTH - 1;i++)
			{
				actorAnim->queueAnimation[i] = actorAnim->queueAnimation[i + 1];
				actorAnim->queueLoopAnimation[i] = actorAnim->queueLoopAnimation[i + 1];
				actorAnim->queueAnimationSpeed[i] = actorAnim->queueAnimationSpeed[i + 1];
			}
			actorAnim->queueAnimation[ANIM_QUEUE_LENGTH - 1] = -1;
			actorAnim->queueLoopAnimation[ANIM_QUEUE_LENGTH - 1] = -1;
			actorAnim->queueAnimationSpeed[ANIM_QUEUE_LENGTH - 1] = -1;
			if(actorAnim->numberQueued >0) 
			{
				actorAnim->numberQueued--;
			}
		}
	}
	else
	{
		if(actorAnim->frame > anim->animEnd)
		{
			actorAnim->frame = anim->animStart;
			actorAnim->animTime = anim->animStart<<ANIMSHIFT;
			actorAdjustPosition(actor);
		}
		else 
		{
			if(actorAnim->frame < anim->animStart)
			{
				actorAnim->frame = anim->animEnd;
				actorAnim->animTime = anim->animEnd <<ANIMSHIFT;
				actorAdjustPosition(actor);
			}
		}
	}
}

/**************************************************************************
	FUNCTION:	
	PURPOSE:	
	PARAMETERS:	
	RETURNS:	
**************************************************************************/
void actorDrawRadiusBox(ACTOR *actor)
{

	BOUNDINGBOX *corners;
	SVECTOR v;
	//long lx,rx,ty,by,ison;
	DVECTOR scrxy1,scrxy2;

//	PSISetBoundingRotated(actor,actor->animation.frame,actor->object->rotate.vx,actor->object->rotate.vy,actor->object->rotate.vz);
//	PSISetBounding(actor,0);

	gte_SetRotMatrix(&GsWSMATRIX);
	gte_SetTransMatrix(&GsWSMATRIX);

	corners = &actor->bounding;
//////////////
	corners->maxX = actor->radius;
	corners->maxY = actor->radius;
	corners->maxZ = actor->radius;

	corners->minX = -actor->radius;
	corners->minY = -actor->radius;
	corners->minZ = -actor->radius;

////////////


	polyLine.r0 = 240;
	polyLine.g0 = 0;
	polyLine.b0 = 240;

	v.vx = corners->minX+actor->position.vx;
	v.vy = corners->minY+actor->position.vy;
	v.vz = corners->minZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

	v.vx = corners->maxX+actor->position.vx;
	v.vy = corners->maxY+actor->position.vy;
	v.vz = corners->minZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);


	v.vx = corners->maxX+actor->position.vx;
	v.vy = corners->minY+actor->position.vy;
	v.vz = corners->maxZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

	v.vx = corners->minX+actor->position.vx;
	v.vy = corners->maxY+actor->position.vy;
	v.vz = corners->maxZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);


}

/*
void DrawBoundingBox(ACTOR *actor)
{

	BOUNDINGBOX *corners;
	SVECTOR v;
	long lx,rx,ty,by,ison;
	DVECTOR scrxy1,scrxy2;

	PSISetBoundingRotated(actor,actor->animation.frame,actor->object->rotate.vx,actor->object->rotate.vy,actor->object->rotate.vz);
//	PSISetBounding(actor,0);

	gte_SetRotMatrix(&GsWSMATRIX);
	gte_SetTransMatrix(&GsWSMATRIX);

	corners = &actor->bounding;


	polyLine.r0 = 240;
	polyLine.g0 = 240;
	polyLine.b0 = 0;

	v.vx = corners->minX+actor->position.vx;
	v.vy = corners->minY+actor->position.vy;
	v.vz = corners->minZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

	v.vx = corners->maxX+actor->position.vx;
	v.vy = corners->maxY+actor->position.vy;
	v.vz = corners->minZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);


	v.vx = corners->maxX+actor->position.vx;
	v.vy = corners->minY+actor->position.vy;
	v.vz = corners->maxZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

	v.vx = corners->minX+actor->position.vx;
	v.vy = corners->maxY+actor->position.vy;
	v.vz = corners->maxZ+actor->position.vz;
	gte_ldv0(&v);
	gte_rtps();
	gte_stsxy(&scrxy1);
	polyLine.x0 = scrxy1.vx;
	polyLine.y0 = scrxy1.vy;

		v.vx = corners->maxX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->maxY+actor->position.vy;
		v.vz = corners->minZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);
	
		v.vx = corners->minX+actor->position.vx;
		v.vy = corners->minY+actor->position.vy;
		v.vz = corners->maxZ+actor->position.vz;
		gte_ldv0(&v);
		gte_rtps();
		gte_stsxy(&scrxy2);
		polyLine.x1 = scrxy2.vx;
		polyLine.y1 = scrxy2.vy;
		PSIDrawLine(0);

		DrawRadiusBox(actor);
}

*/

