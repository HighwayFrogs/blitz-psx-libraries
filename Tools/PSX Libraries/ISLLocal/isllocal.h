/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	isllocal.c:		Localalised text handling

************************************************************************************/

#ifndef __ISLLOCAL_H_
#define __ISLLOCAL_H_


// use this macro to access the string you want
#define TEXTSTR(n) (gameTextStr[n])


extern void	**gameTextStr;

/**************************************************************************
	FUNCTION:	gameTextInitFromFile
	PURPOSE:	Read in the text from a gametext file
	PARAMETERS:	Filename, num strings, number of languages, current language
	RETURNS:	
**************************************************************************/

void gameTextInit(char *fName, int numStrings, int numLang, int currLang);


/**************************************************************************
	FUNCTION:	gameTextDestroy
	PURPOSE:	Free up text
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void gameTextDestroy();


#endif
