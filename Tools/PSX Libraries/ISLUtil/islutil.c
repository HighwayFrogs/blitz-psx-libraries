/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islutil.c:		CRC functions

************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdarg.h>
#include <libsn.h>


#define POLYNOMIAL			0x04c11db7L			// polynomial for crc algorithm


//#define FLA_MAGIC			0x464c4132			// Magic number 'FLA2' MOTOROLA
#define FLA_MAGIC			0x32414c46			// Magic number 'FLA2' INTEL
#define	HISTORY_SIZE		4096				// flatpacker stuff
#define MASK_HISTORY		(HISTORY_SIZE-1)
#define MASK_UPPER			(0xF0)
#define MASK_LOWER			(0x0F)
#define SHIFT_UPPER			16
#define LSR_UPPER			4
#define MAX_COMP_LEN		17


static unsigned char	*outputBufPtr;			// flatpacker temp storage
static int				outputBufLen;
static unsigned char	LZhistory[HISTORY_SIZE];
static unsigned short	LZhistoryOff;

static unsigned long	CRCtable[256];			// crc table

static char				buffer[256];			// for faster printf

static int				randomSeed;				// random number nonsense

extern unsigned long 	sqrtable[];				// square root lookup table

long					quadrants[] = { 0, 1024, 3072, 2048 };	// angle calculation
long					quads[] = { 0, 1, 1, 0 };					
extern short			acostable[];			// arccos table


/**************************************************************************
	FUNCTION:	utilInitCRC()
	PURPOSE:	Initialise internal table for CRC calculations
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void utilInitCRC()
{
	register int i, j;
	register unsigned long CRCaccum;

	for (i=0; i<256; i++)
		{
		CRCaccum = ((unsigned long)i<<24);
		for (j=0; j<8; j++)
			{
			if (CRCaccum & 0x80000000L)
				CRCaccum = (CRCaccum<<1)^POLYNOMIAL;
			else
				CRCaccum = (CRCaccum<<1);
			}
		CRCtable[i] = CRCaccum;
		}
}


/**************************************************************************
	FUNCTION:	utilStr2CRC()
	PURPOSE:	Calculate 32-bit CRC for given string
	PARAMETERS:	Ptr to string
	RETURNS:	32-bit CRC
**************************************************************************/

unsigned long utilStr2CRC(char *ptr)
{
	register int i, j;
	int size = strlen(ptr);
	unsigned long CRCaccum = 0;

	for (j=0; j<size; j++)
		{
		i = ((int)(CRCaccum>>24)^(*ptr++))&0xff;
		CRCaccum = (CRCaccum<<8)^CRCtable[i];
		}
	return CRCaccum;
}


/**************************************************************************
	FUNCTION:	utilBlockCRC()
	PURPOSE:	Calculate 32-bit CRC for given memory block
	PARAMETERS:	Ptr to block
	RETURNS:	32-bit CRC
**************************************************************************/

unsigned long utilBlockCRC(char *ptr, int length)
{
	register int	i, j;
	unsigned long	CRCaccum = 0;

	for (j=0; j<length; j++)
	{
		i = ((int)(CRCaccum>>24)^(*ptr++))&0xff;
		CRCaccum = (CRCaccum<<8)^CRCtable[i];
	}
	return CRCaccum;
}


/**************************************************************************
	FUNCTION:	DecompressOutputByte()
	PURPOSE:	decompresses a byte of data
	PARAMETERS:	the byte of data
	RETURNS:
**************************************************************************/

static inline void DecompressOutputByte(unsigned char data)
{
	*outputBufPtr++ = data;
	outputBufLen++;
	LZhistory[LZhistoryOff] = data;
	LZhistoryOff = (LZhistoryOff+1) & MASK_HISTORY;
}


/**************************************************************************
	FUNCTION:	utilDecompressBuffer()
	PURPOSE:	Decompress data from input buffer into output buffer
	PARAMETERS:	Input buffer start, Output buffer start
	RETURNS:	Length of uncompressed data
**************************************************************************/

int utilDecompressBuffer(unsigned char *inBuf, unsigned char *outBuf)
{
	unsigned short	tag, count, offset, loop;

	outputBufPtr = outBuf;								// Initialise output
	outputBufLen = 0;
	
	LZhistoryOff = 0;									// Clear history
	memset(LZhistory, 0, HISTORY_SIZE);

	while(1)
	{
		tag = *inBuf++;
		for(loop=0; loop!=8; loop++)
		{
			if (tag & 0x80)
			{
				if ((count=*inBuf++) == 0)
				{
					return outputBufLen;				// Finished now
				}
				else
				{										// Copy from history
					offset = HISTORY_SIZE-(((MASK_UPPER & count)*SHIFT_UPPER)+(*inBuf++));
					count &= MASK_LOWER;
					count += 2;
					while (count!=0)
					{
						DecompressOutputByte(LZhistory[(LZhistoryOff+offset) & MASK_HISTORY]);
						count--;
					}
				}
			}
			else
			{
				DecompressOutputByte(*inBuf++);			// Copy data byte
			}
			tag += tag;
		}
	}
	return outputBufLen;
}

/**************************************************************************
	FUNCTION:	utilPrintf()
	PURPOSE:	Faster printf
	PARAMETERS:	see printf()
	RETURNS:	Length of string
**************************************************************************/

int utilPrintf(char* fmt, ...)
{
    va_list	arglist;
    long	r1, r2, r3, r4, r5, r6;
    int		len;

	va_start(arglist, fmt);
    r1 = va_arg(arglist, long);
    r2 = va_arg(arglist, long);
    r3 = va_arg(arglist, long);
    r4 = va_arg(arglist, long);
    r5 = va_arg(arglist, long);
    r6 = va_arg(arglist, long);
	va_end(arglist);
    len = sprintf(buffer, fmt, r1, r2, r3, r4, r5, r6);
#if _DEBUG
	buffer[len] = 13;
	buffer[len+1] = 10;
    PCwrite(-1, buffer, len+1);
#else
	printf(buffer);
#endif
    return len;
}

/**************************************************************************
	FUNCTION:	utilUpperStr()
	PURPOSE:	Convert string to upper case
	PARAMETERS:	String ptr
	RETURNS:	
**************************************************************************/

void utilUpperStr(char *str)
{
	while(*str)
	{
		if ((*str>='a')&&(*str<='z'))
			*str -= 'a'-'A';
		str++;
	}
}

/**************************************************************************
	FUNCTION:	utilSeedRandomInt()
	PURPOSE:	Set random integer seed value
	PARAMETERS:	Seed value
	RETURNS:	
**************************************************************************/

void utilSeedRandomInt(int seed)
{
	randomSeed = seed;
}


/**************************************************************************
	FUNCTION:	utilRandomInt()
	PURPOSE:	Generate pseudo random integer
	PARAMETERS:	Maximum value
	RETURNS:	Random value (0-limit)
**************************************************************************/

int utilRandomInt(int limit)
{
	int	retVal;

	if (limit<0)
		limit=0;

	randomSeed = randomSeed ^ (randomSeed<<1);
	if ((randomSeed & 0x80000000)==0)
	{
		randomSeed++;
	}
	else
	{
		if (randomSeed == 0x80000000)
		{
			randomSeed = 0;
		}
	}

	retVal = randomSeed % (limit+1);

	return ((retVal < 0) ? (-retVal) : (retVal));
}


/**************************************************************************
	FUNCTION:	utilSqrt()
	PURPOSE:	Fast square-root function
	PARAMETERS:	number to square-root
	RETURNS:	Square-root of number (16.16 fixed)
**************************************************************************/

unsigned long utilSqrt(unsigned long num)
{
	long	j;

	j = 0;
	
	while(num > 1024)
	{
		num >>= 2;
		j++;
	}

	return sqrtable[num] << j;
}

/**************************************************************************
	FUNCTION:	utilCalcAngle()
	PURPOSE:	Calc angle of a triangle
	PARAMETERS:	adjacent and opposite lengths
	RETURNS:	angle (0-4095)
**************************************************************************/

long utilCalcAngle(long adj,long opp)
{
	long hyp,xsgn,ysgn,sine,quadrant;

	while ((adj>32767)||(opp>32767)||(adj<-32767)||(opp<-32767))
	{
		adj /= 2;
		opp /= 2;
	}

	xsgn = (adj < 0);									// +/-
	ysgn = (opp < 0);									// +/-

	quadrant = (xsgn * 2) + ysgn;							//
													  
	hyp = utilSqrt(adj * adj + opp * opp);	//
	ysgn=((hyp & 0xffff) > 0x7fff);

	sine = abs(opp) << 10;
	xsgn = ((hyp >> 16) + ysgn) & 0xffff;

	if(xsgn)
		sine = abs(sine / xsgn);

	sine = acostable[sine];
	
	if(quads[quadrant])
		sine = 1024 - sine;

	sine = ((sine + quadrants[quadrant]) + 4096) & 4095;
	
	sine |= (hyp & 0xffff0000);
	return sine;
}
