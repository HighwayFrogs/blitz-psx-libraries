/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islxa.h:		XA audio streaming functions

************************************************************************************/

#include <stdio.h>
#include <stddef.h>
#include <libcd.h>
#include <libspu.h>
#include <string.h>
#include <libetc.h>
#include "..\islmem\islmem.h"
#include "islxa.h"


#define XAROOT	"\\"

typedef struct _XADataType {
	CdlCB		oldCallback;
	char		buffer[2340];
	int			currChannel;
	int			activeChannel;
	int			currSector;
	XAFileType	*currXA;
	XAFileType	*prevXA;
	int			prevChannel;
	int			prevSector;
} XADataType;


XADataType	XAData;

// are we enabled or not
static int	XAenable;


/**************************************************************************
	FUNCTION:	XAsetStatus()
	PURPOSE:	Enable/disable xa playing
	PARAMETERS:	0 - disable / 1 - enable
	RETURNS:	
**************************************************************************/

void XAsetStatus(int enable)
{
	if(enable)
		XAenable = 1;
	else
		XAenable = 0;
}


/**************************************************************************
	FUNCTION:	XAgetStatus()
	PURPOSE:	Get current status of XA playing
	PARAMETERS:	
	RETURNS:	0 - disabled / 1 - enabled
**************************************************************************/

int XAgetStatus()
{
	return XAenable;
}


/**************************************************************************
	FUNCTION:	XAcallback()
	PURPOSE:	Callback for XA audio stream
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

static void XAcallback(int intr, u_char *result)
{
	unsigned long	*cAddress = (unsigned long *)XAData.buffer;
	int		ID;

	if (intr == CdlDataReady)
	{
		CdGetSector(cAddress, 8);
		ID = *(unsigned short *)(cAddress+3);
		// video sector channel number format = 1CCCCC0000000001
		XAData.currChannel = *((unsigned short *)(cAddress+3)+1);
		XAData.currChannel = (XAData.currChannel & 31744)>>10;

		XAData.currSector++;
		if((ID == 352) && (XAData.currChannel==XAData.activeChannel))	// Check for end sector marker
		{
	        CdControlF(CdlPause,0);
			if (XAData.currXA->loop)
			{
				XAplayChannel(XAData.currXA, XAData.activeChannel, XAData.currXA->loop, XAData.currXA->vol);
			}
			else
			{
				XAData.currXA->status = 0;
				XAData.currXA = NULL;
				XAstop();
			}
//	        SsSetSerialVol(SS_SERIAL_A,0,0);
		}
	}
	else
		printf("UnHandled Callback Occured\n");	
}


/**************************************************************************
	FUNCTION:	XAgetFileInfo()
	PURPOSE:	Get position information for XA file
	PARAMETERS:	Filename
	RETURNS:	
**************************************************************************/

XAFileType *XAgetFileInfo(char *fileName)
{
	XAFileType	*xaf;
	char		fName[40];

	sprintf(fName, "XA/%s", fileName);
	xaf = (XAFileType *)MALLOC(sizeof(XAFileType));
	strcpy(fName, XAROOT);
	strcat(fName, fileName);
	strcat(fName, ";1");
	if (!XAenable)
	{
//		printf("XA track '%s' disabled\n", fName);
		return xaf;
	}
	if (CdSearchFile(&xaf->fileInfo, fName)==0)
	{
		printf("XA file '%s' not found\n", fName);
		return NULL;
	}
	xaf->startPos = CdPosToInt(&xaf->fileInfo.pos);
	xaf->endPos = xaf->startPos + (xaf->fileInfo.size/2048)-1;
	xaf->status = 0;
	printf("XA track '%s' %d->%d\n", fName, xaf->startPos,xaf->endPos);
	return xaf;
}


/**************************************************************************
	FUNCTION:	XAstart()
	PURPOSE:	Get CD ready to play XA audio stream
	PARAMETERS:	CD speed 0=single 1=double
	RETURNS:	
**************************************************************************/

void XAstart(int speed)
{
	u_char param[4];

	if (!XAenable)
		return;
	if (speed)
		param[0] = CdlModeSpeed|CdlModeRT|CdlModeSF|CdlModeSize1;
	else
		param[0] = CdlModeRT|CdlModeSF|CdlModeSize1;
	CdControlB(CdlSetmode, param, 0);
	VSync(3);	// TRC regulation (could be shit)
	CdControlF(CdlPause,0);

	XAData.oldCallback = CdReadyCallback((CdlCB)XAcallback);
	XAData.currXA = NULL;
}


/**************************************************************************
	FUNCTION:	XAstop()
	PURPOSE:	CD back to data - finished playing XA audio streams
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void XAstop()
{
	u_char param[4];

	if (!XAenable)
		return;
	SpuSetCommonCDVolume(0,0);
	CdControlF(CdlPause,0);
	CdReadyCallback((void *)XAData.oldCallback);
	param[0] = CdlModeSpeed;
	CdControlB(CdlSetmode, param, 0);
}


/**************************************************************************
	FUNCTION:	XApause()
	PURPOSE:	Stop the XA audio stream
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void XApause()
{
	if (!XAenable)
		return;
	SpuSetCommonCDVolume(0,0);
	CdControlF(CdlPause,0);
}


/**************************************************************************
	FUNCTION:	XAplayChannelOffset()
	PURPOSE:	Start playing channel from given XA audio stream
	PARAMETERS:	XA file, offset (sectors), channel number, loop
	RETURNS:	
**************************************************************************/

void XAplayChannelOffset(XAFileType *xaF, int offset, int channel, int loop, int vol)
{
	CdlLOC		loc;
	CdlFILTER	theFilter;
	int			sector;

	if (!XAenable)
	{
		printf("XAplayChannel: disabled\n");
		xaF->status = 0;
		return;
	}
	if (xaF==NULL)
	{
		printf("XAplayChannel: stream not found\n");
		return;
	}
	theFilter.file = 1;
	theFilter.chan = channel;
	XAData.activeChannel = channel;
	CdControlF(CdlSetfilter, (u_char *)&theFilter);
	sector = xaF->startPos+offset;
	CdIntToPos(sector, &loc);
	CdControlF(CdlReadS, (u_char *)&loc);
	XAData.currXA = xaF;
	XAData.currSector = sector;
	xaF->status = 1;
	xaF->loop = loop;
	xaF->vol = vol;
	printf("XAplayChannel: %d->%d\n", sector,xaF->endPos);
	SpuSetCommonCDVolume((vol*0x7fff)/100,(vol*0x7fff)/100);
}


/**************************************************************************
	FUNCTION:	XAcheckPlay()
	PURPOSE:	Test if XA has begun playback
	PARAMETERS:	
	RETURNS:	0 = Not playing yet, 1 = Is playing now
**************************************************************************/

int XAcheckPlay()
{
	unsigned char	result[8];

	if (!XAenable)
		return 1;
	CdControlB(CdlNop, 0, result);
	return ((result[0] & CdlStatRead) && (!(result[0] & CdlStatSeek)));
}


/**************************************************************************
	FUNCTION:	XAstorePrevious()
	PURPOSE:	Store current XA playback track/channel
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void XAstorePrevious()
{
	XAData.prevXA = XAData.currXA;
	XAData.prevChannel = XAData.activeChannel;
	XAData.prevSector = XAData.currSector;
}


/**************************************************************************
	FUNCTION:	XArestartPrevious()
	PURPOSE:	Restart previously stored XA playback track/channel
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void XArestartPrevious()
{
	CdlLOC		loc;
	CdlFILTER	theFilter;

	if (!XAenable)
	{
		printf("XArestartPrevious: disabled\n");
		XAData.prevXA->status = 0;
		return;
	}
	if (XAData.prevXA==NULL)
	{
		printf("XArestartPrevious: stream not found\n");
		return;
	}
	theFilter.file = 1;
	theFilter.chan = XAData.prevChannel;
	XAData.activeChannel = XAData.prevChannel;
	CdControlF(CdlSetfilter, (u_char *)&theFilter);
	CdIntToPos(XAData.prevSector, &loc);
	CdControlF(CdlReadS, (u_char *)&loc);
	XAData.currXA = XAData.prevXA;
	XAData.currSector = XAData.prevSector;
	XAData.prevXA->status = 1;
	printf("XArestartPrevious: %d [%d->%d]\n", XAData.prevSector,XAData.prevXA->startPos,XAData.prevXA->endPos);
	SpuSetCommonCDVolume((XAData.currXA->vol*0x7fff)/100,(XAData.currXA->vol*0x7fff)/100);
}

