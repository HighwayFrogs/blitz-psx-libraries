/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islxa.h:		XA audio streaming functions

************************************************************************************/


#ifndef __ISLXA_H__
#define	__ISLXA_H__


typedef struct _XAFileType {
	CdlFILE			fileInfo;
	int				startPos;
	int				endPos;
	unsigned char	status;
	unsigned char	loop;
	int				vol;
} XAFileType;


/**************************************************************************
	FUNCTION:	XAsetStatus()
	PURPOSE:	Enable/disable xa playing
	PARAMETERS:	0 - disable / 1 - enable
	RETURNS:	
**************************************************************************/

void XAsetStatus(int enable);


/**************************************************************************
	FUNCTION:	XAgetStatus()
	PURPOSE:	Get current status of XA playing
	PARAMETERS:	
	RETURNS:	0 - disabled / 1 - enabled
**************************************************************************/

int XAgetStatus();


/**************************************************************************
	FUNCTION:	XAgetFileInfo()
	PURPOSE:	Get position information for XA file
	PARAMETERS:	Filename
	RETURNS:	
**************************************************************************/

XAFileType *XAgetFileInfo(char *fileName);


/**************************************************************************
	FUNCTION:	XAstart()
	PURPOSE:	Get CD ready to play XA audio stream
	PARAMETERS:	Speed 0 = single, 1 = double
	RETURNS:	
**************************************************************************/

void XAstart(int speed);


/**************************************************************************
	FUNCTION:	XAstop()
	PURPOSE:	CD back to data - finished playing XA audio streams
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void XAstop();


/**************************************************************************
	FUNCTION:	XApause()
	PURPOSE:	Stop the XA audio stream
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void XApause();


/**************************************************************************
	FUNCTION:	XAplayChannel()
	PURPOSE:	Start playing channel from given XA audio stream
	PARAMETERS:	XA file, channel number, loop, vol
	RETURNS:	
**************************************************************************/

#define XAplayChannel(xaF, channel, loop, vol) XAplayChannelOffset(xaF, 0, channel, loop, vol);


/**************************************************************************
	FUNCTION:	XAplayChannelOffset()
	PURPOSE:	Start playing channel from given XA audio stream
	PARAMETERS:	XA file, offset (sectors), channel number, loop, vol
	RETURNS:	
**************************************************************************/

void XAplayChannelOffset(XAFileType *xaF, int offset, int channel, int loop, int vol);


/**************************************************************************
	FUNCTION:	XAcheckPlay()
	PURPOSE:	Test if XA has begun playback
	PARAMETERS:	
	RETURNS:	0 = Not playing yet, 1 = Is playing now
**************************************************************************/

int XAcheckPlay();


/**************************************************************************
	FUNCTION:	XAstorePrevious()
	PURPOSE:	Store current XA playback track/channel
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void XAstorePrevious();


/**************************************************************************
	FUNCTION:	XArestartPrevious()
	PURPOSE:	Restart previously stored XA playback track/channel
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

void XArestartPrevious();


#endif	//__ISLXA_H__
