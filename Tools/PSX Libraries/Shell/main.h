/******************************************************************************************
	AM2 PS   (c) 1999-2001 ISL

	main.h:			Top level shell
******************************************************************************************/

#ifndef _MAIN_H_
#define _MAIN_H_


#include "psfont.h"

typedef struct _displayPageType {
	DISPENV	dispenv;
	DRAWENV	drawenv;
	ULONG	*ot;
	char	*primBuffer, *primPtr;
} displayPageType;


#define BEGINPRIM(p,t)		(p) = (t *)currentDisplayPage->primPtr;
#define ENDPRIM(p,d,t)		{addPrim(currentDisplayPage->ot+(d), (p)); currentDisplayPage->primPtr += sizeof(t);}
#define SETSEMIPRIM(p,m)	(p)->tpage |= (((m)-1)<<5);


extern ULONG	frame;
extern displayPageType displayPage[2], *currentDisplayPage;
extern psFont *font;


enum {
	SEMITRANS_NONE,
	SEMITRANS_SEMI,
	SEMITRANS_ADD,
	SEMITRANS_SUB,
};


/**************************************************************************
	FUNCTION:	videoInit()
	PURPOSE:	Initialise video mode/GPU
	PARAMETERS:	Order table depth, max number of GT4 primitives needed or 0
	RETURNS:	
**************************************************************************/

void videoInit(int otDepth, int maxPrims);


#endif
