/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islcard.c:		memory card functions

************************************************************************************/


#include <stddef.h>
#include <stdio.h>
#include <libmcrd.h>
#include <libpad.h>
#include <libetc.h>
#include "islcard.h"
#include "..\islutil\islutil.h"
#include "..\islmem\islmem.h"

static struct DIRENTRY		memCardDir[15];
static long					memCardDirNum, memCardDirSize;


static int cardCheckCard(int *numBlocks)
{
	long	cmds, result, sync, cardFound = 0, loop;

#ifdef _DEBUG
	printf("Reading memory card...\n");
#endif
	while(!cardFound)
	{
		VSync(0);
		sync = MemCardSync(1, &cmds, &result);
		switch(sync)
		{
		case -1:
			MemCardExist(0);
			break;
		case 0:
			break;
		case 1:
			switch(cmds)
			{
			case McFuncExist:
				switch(result)
				{
				case McErrNone:		// No change, card is OK
#ifdef _DEBUG
					printf("No change, but I'm going to check anyway like the Sony manual says!\n");
#endif
					MemCardAccept(0);
//					cardFound = 1;
					break;
				case McErrNewCard:	// Different card, check further
#ifdef _DEBUG
					printf("Different card, check further\n");
#endif
					MemCardAccept(0);
					break;
				default:
#ifdef _DEBUG
					printf("No card\n");
#endif
					return result;
				}
				break;
			case McFuncAccept:
				switch(result)
				{
				case McErrNewCard:		// Card is new
#ifdef _DEBUG
					printf("...Card is new\n");
#endif
				case McErrNone:			// Card is fine
#ifdef _DEBUG
					printf("...Card is fine\n");
#endif
					memCardDirSize = 0;
					if (MemCardGetDirentry(0, "*", memCardDir, &memCardDirNum, 0, 15)==0)
					{
						for(loop=0; loop<memCardDirNum; loop++)
							memCardDirSize += memCardDir[loop].size/8192+((memCardDir[loop].size%8192)?(1):(0));
						cardFound = 1;
					}
					break;
				case McErrNotFormat:	// Card is unformatted
#ifdef _DEBUG
					printf("...Card is unformatted\n");
#endif
					return result;
				default:
#ifdef _DEBUG
					printf("...Problem card (%d)\n",result);
#endif
					return result;
				}
				break;
			}
			break;
		}

	}
#ifdef _DEBUG
	printf("Card ready: %d files, %d blocks used\n", memCardDirNum, memCardDirSize);
	for(loop=0; loop<memCardDirNum; loop++)
		printf("  %s [%d]\n", memCardDir[loop].name, memCardDir[loop].size);
#endif

	*numBlocks = memCardDirSize;
	return 0;
}


/**************************************************************************
	FUNCTION:	saveRead()
	PURPOSE:	Read game save from memory card
	PARAMETERS:	save name, pointer to data, size of data
	RETURNS:	0 if loaded OK, +ve Sony errors, -1 not found, -2 corrupt, -3 not found and full card
**************************************************************************/

int cardRead(char *memCardName, void *gameSaveData, int gameSaveDataSize)
{
	int		result, loop, crc, errorCheck, dummy;
	char	*buffer, *data;
	long	cmds, res;

	PadStopCom();

	loop=0;

	do
	{
		result = cardCheckCard(&dummy);
		errorCheck = cardCheckCard(&dummy);
		loop++;
	}
	while((result!=errorCheck) && loop<4);

	if (result)
	{
#ifdef _DEBUG
		printf("Load cancelled\n");
#endif
		PadStartCom();
		return result;
	}
	for(loop=0; loop<memCardDirNum; loop++)
	{
		if (strcmp(memCardName, memCardDir[loop].name)==0)
		{
			buffer = MALLOC(8192);
			MemCardReadFile(0, memCardName, (unsigned long *)buffer, 0, 8192);
			MemCardSync(0, &cmds, &res);
			if (res==0)
			{
#ifdef _DEBUG
				printf("Game loaded\n");
#endif
			}
			else
			{
#ifdef _DEBUG
				printf("Error loading game (%d)\n", res);
#endif
				PadStartCom();
				return res;
			}
			data = buffer;
			crc = utilBlockCRC(buffer, gameSaveDataSize);
			data += gameSaveDataSize;
			if (crc==*((int *)data))
			{
				if(gameSaveData)
					memcpy(gameSaveData, buffer, gameSaveDataSize);
			}
			else
			{
#ifdef _DEBUG
				printf("CRC error in game save - trying backup\n");
#endif
				data += 4;
				crc = utilBlockCRC(data, gameSaveDataSize);
				data += gameSaveDataSize;
				if (crc==*((int *)data))
				{
					if(gameSaveData)
						memcpy(gameSaveData, data, gameSaveDataSize);
				}
				else
				{
#ifdef _DEBUG
					printf("CRC error in backup game save - give up\n");
#endif
					FREE(buffer);
					PadStartCom();
					return CARDREAD_CORRUPT;
				}
			}
			FREE(buffer);
			PadStartCom();
			return CARDREAD_OK;
		}
	}
	PadStartCom();

	if(dummy == 15)
		return CARDREAD_NOTFOUNDANDFULL;

	return CARDREAD_NOTFOUND;
}

/**************************************************************************
	FUNCTION:	cardWrite()
	PURPOSE:	Write game save to memory card
	PARAMETERS:	save name, pointer to data, size of data
	RETURNS:	0 if saved OK, +ve Sony errors, -1 no room
**************************************************************************/

int cardWrite(char *memCardName, void *gameSaveData, int gameSaveDataSize)
{
	int		result, crc, errorCheck, dummy;
	char	*buffer, *data;
	long	cmds, res;
	int		loop;
	
	PadStopCom();

	loop=0;

	do
	{
		result = cardCheckCard(&dummy);
		errorCheck = cardCheckCard(&dummy);
		loop++;
	}
	while((result!=errorCheck) && loop<4);

	if (result)
	{
#ifdef _DEBUG
		printf("Save cancelled\n");
#endif
		PadStartCom();
		return result;
	}
/*	if (memCardDirSize>14)
	{
		printf("Save cancelled\n");
		PadStartCom();
		return -1;
	}*/
	buffer = MALLOC(8192);
	data = buffer;
	memcpy(data, gameSaveData, gameSaveDataSize);
	crc = utilBlockCRC(data, gameSaveDataSize);
	data += gameSaveDataSize;
	*((int *)data) = crc;
	data += 4;
	memcpy(data, gameSaveData, gameSaveDataSize);
	crc = utilBlockCRC(data, gameSaveDataSize);
	data += gameSaveDataSize;
	*((int *)data) = crc;
	data += 4;
	result = MemCardCreateFile(0, memCardName, 1);
	VSync(2);
	if ((result!=0) && (result!=McErrAlreadyExist))
	{
#ifdef _DEBUG
		printf("Error creating game (%d)\n", result);
#endif
		FREE(buffer);
		PadStartCom();
		return result;
	}
	MemCardWriteFile(0, memCardName, (unsigned long *)buffer, 0, 8192);
	VSync(2);
	MemCardSync(0, &cmds, &res);
	FREE(buffer);
	if (res==0)
	{
#ifdef _DEBUG
		printf("Game saved\n");
#endif
	}
	else
	{
#ifdef _DEBUG
		printf("Error saving game (%d)\n", res);
#endif
		PadStartCom();
		return result;
	}

	PadStartCom();
	return CARDWRITE_OK;
}


/**************************************************************************
	FUNCTION:	cardFormat()
	PURPOSE:	Format memory card (BE CAREFUL!)
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

int cardFormat()
{
	int result;

	PadStopCom();
	VSync(2);
	result = MemCardFormat(0);
	VSync(2);
	PadStartCom();
	
	return result;
}
