/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islcard.h:		memory card functions

************************************************************************************/

#ifndef __ISLCARD_H__
#define __ISLCARD_H__

// error codes

#define CARDREAD_OK					0
#define CARDREAD_NOCARD				1
#define CARDREAD_BADCARD			2
#define CARDREAD_NOTFORMATTED		4
#define CARDREAD_NOTFOUND			-1
#define CARDREAD_CORRUPT			-2
#define CARDREAD_NOTFOUNDANDFULL	-3

#define CARDWRITE_OK				0
#define CARDWRITE_NOCARD			1
#define CARDWRITE_BADCARD			2
#define CARDWRITE_NOTFORMATTED		4
#define CARDWRITE_FULL				7


/**************************************************************************
	FUNCTION:	cardRead()
	PURPOSE:	Read game save from memory card
	PARAMETERS:	save name, pointer to data, size of data
	RETURNS:	see list above (+ve Sony errors, -ve loading errors)
**************************************************************************/

int cardRead(char *memCardName, void *saveData, int saveDataSize);


/**************************************************************************
	FUNCTION:	cardWrite()
	PURPOSE:	Write game save to memory card
	PARAMETERS:	save name, pointer to data, size of data
	RETURNS:	see list above (+ve Sony errors, -ve saving errors)
**************************************************************************/

int cardWrite(char *memCardName, void *saveData, int saveDataSize);


/**************************************************************************
	FUNCTION:	cardFormat()
	PURPOSE:	Format memory card (BE CAREFUL!)
	PARAMETERS:	
	RETURNS:	0 if ok, +ve for an error (Sony errors)
**************************************************************************/

int cardFormat();


#endif //__ISLCARD_H__
