/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islutil.h:		Utility functions

************************************************************************************/

#ifndef __ISLUTIL_H__
#define __ISLUTIL_H__


/**************************************************************************
	FUNCTION:	utilInitCRC()
	PURPOSE:	Initialise internal table for CRC calculations
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void utilInitCRC();


/**************************************************************************
	FUNCTION:	utilStr2CRC()
	PURPOSE:	Calculate 32-bit CRC for given string
	PARAMETERS:	Ptr to string
	RETURNS:	32-bit CRC
**************************************************************************/

unsigned long utilStr2CRC(char *ptr);


/**************************************************************************
	FUNCTION:	utilBlockCRC()
	PURPOSE:	Calculate 32-bit CRC for given memory block
	PARAMETERS:	Ptr to block
	RETURNS:	32-bit CRC
**************************************************************************/

unsigned long utilBlockCRC(char *ptr, int length);


/**************************************************************************
	FUNCTION:	utilDecompressBuffer()
	PURPOSE:	Decompress data from input buffer into output buffer
	PARAMETERS:	Input buffer start, Output buffer start
	RETURNS:	Length of uncompressed data
**************************************************************************/

int utilDecompressBuffer(unsigned char *inBuf, unsigned char *outBuf);


/**************************************************************************
	FUNCTION:	utilPrintf()
	PURPOSE:	Faster printf
	PARAMETERS:	See printf()
	RETURNS:	Length of string
**************************************************************************/

int utilPrintf(char* fmt, ...);


/**************************************************************************
	FUNCTION:	utilUpperStr()
	PURPOSE:	Convert string to upper case
	PARAMETERS:	String ptr
	RETURNS:	
**************************************************************************/

void utilUpperStr(char *str);


/**************************************************************************
	FUNCTION:	utilInstallException()
	PURPOSE:	Install debugding exception handler
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void utilInstallException();


/**************************************************************************
	FUNCTION:	utilSeedRandomInt()
	PURPOSE:	Set random integer seed value
	PARAMETERS:	Seed value
	RETURNS:	
**************************************************************************/

void utilSeedRandomInt(int seed);


/**************************************************************************
	FUNCTION:	utilRandomInt()
	PURPOSE:	Generate pseudo random integer
	PARAMETERS:	Maximum value
	RETURNS:	Random value (0-limit)
**************************************************************************/

int utilRandomInt(int limit);


/**************************************************************************
	FUNCTION:	utilSqrt()
	PURPOSE:	Fast square-root function
	PARAMETERS:	number to square-root
	RETURNS:	Square-root of number (16.16 fixed)
**************************************************************************/

unsigned long utilSqrt(unsigned long num);


/**************************************************************************
	FUNCTION:	utilCalcAngle()
	PURPOSE:	Calc angle of a triangle
	PARAMETERS:	adjacent and opposite lengths
	RETURNS:	angle (0-4095)
**************************************************************************/

long utilCalcAngle(long adj,long opp);


#endif