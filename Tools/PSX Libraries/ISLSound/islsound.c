/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islsound.c:		Sound fx handling

************************************************************************************/

#include <stdio.h>
#include <stddef.h>
#include <libspu.h>
#include <libgte.h>
#include "..\islmem\islmem.h"
#include "..\islfile\islfile.h"
#include "islsound.h"


#define MAX_SAMPLES			100
#define SAMPLELIST_LENGTH	64

enum {
	PLAY_SAMPLE,
	SET_PITCH,
	SET_VOLUME,
	STOP_SAMPLE,
};



/**************************************************************************
	PRIVATE STRUCTURES
**************************************************************************/

typedef struct _sfxSampleType
{
	unsigned long	size;
	unsigned long	pitches;
	unsigned long	spuOffset;
	long			leftVol;
	long			rightVol;
} sfxSampleType;


typedef struct _sfxBankType
{
	unsigned long	numSamples;
	sfxSampleType	*samples;
	char			rec[SPU_MALLOC_RECSIZ * (4+1)];
	unsigned long	startAddr;
	int				volume;
} sfxBankType;

typedef struct _sfxListType
{
	short			sample;
	short			leftVolume;
	short			rightVolume;
	short			pitch;
	char			type;
	unsigned char	*returnAddress;
} sfxListType;


static sfxBankType		sampleBank;
static long				lastSampleCall[MAX_SAMPLES];
static unsigned long	sfxFrameNum;
static sfxListType		sfxList[SAMPLELIST_LENGTH];
static int				listHead, listTail;



/**************************************************************************
	PRIVATE FUNCTIONS
**************************************************************************/


/**************************************************************************
	FUNCTION:	sfxCalc3DPosition()
	PURPOSE:	calculate volume of sound at position in world
	PARAMETERS:	position, left and right volumes to return
	RETURNS:	none
**************************************************************************/

static int sfxCalc3DPosition(VECTOR *pos, VECTOR *camPos, VECTOR *camRef, int *leftVol, int *rightVol)
{
	/*
	int cameraAngle, soundAngle, offsetAngle;
	int	distanceX, distanceZ, distance;
	int tmpLeftVol, tmpRightVol;

	// get the sound's distance from the camera
	distanceX = pos->vx - camPos->vx;
	distanceZ = pos->vz - camPos->vz;
	distance = calcSqrt(distanceX * distanceX + distanceZ * distanceZ) >> 16;

	// if sound is too far to be heard, don't bother with positioning
	if(distance > 5000)
	{
//		printf("\nsound @ %d dist, too far away!",distance);
		return 0;
	}
	
	// find the angle of the sound with respect to the camera's position
	cameraAngle = calcAngle(camRef->vx-camPos->vx, camRef->vz-camPos->vz);
	soundAngle = calcAngle(pos->vx-camPos->vx, pos->vz-camPos->vz);
	
	offsetAngle = findShortestAngle(soundAngle & 4095, cameraAngle & 4095);

	// calculate some temporary volumes

	if(abs(offsetAngle) > 1024)					// sound is behind us
	{
		if(offsetAngle > 0)
		{
			offsetAngle = 2048 - offsetAngle;
		}
		else
		{
			offsetAngle = -2048 - offsetAngle;
		}
	}
	
	// standard stereo panning
	tmpRightVol = (offsetAngle + 1024) * 8;
	tmpLeftVol = 16384 - tmpRightVol;

	if(distance)
	{
		*leftVol = ((-tmpLeftVol * distance) / 5000) + tmpLeftVol;
		*rightVol = ((-tmpRightVol * distance) / 5000) + tmpRightVol;
	}
	
	return 1;
	*/
	return 0;
}


/**************************************************************************
	FUNCTION:	sfxPlaySound()
	PURPOSE:	Plays the requested sample
	PARAMETERS:	sample number
	RETURNS:	none
**************************************************************************/

static unsigned long sfxPlayVoice(int sfxNum)
{
	unsigned long voiceLoop;

	if (lastSampleCall[sfxNum]+2 > sfxFrameNum)
		return -2;

	lastSampleCall[sfxNum] = sfxFrameNum;

	for(voiceLoop = 0; voiceLoop < 23; voiceLoop ++)
	{
		if(SpuGetKeyStatus(1 << voiceLoop) != SPU_ON)
		{
			SpuSetVoiceVolume(voiceLoop,(sampleBank.samples[sfxNum].leftVol*sampleBank.volume)/100,(sampleBank.samples[sfxNum].rightVol*sampleBank.volume)/100);	// channel, left vol, right vol,
			SpuSetVoicePitch(voiceLoop,	sampleBank.samples[sfxNum].pitches);		// channel, pitch
			SpuSetVoiceStartAddr(voiceLoop, sampleBank.samples[sfxNum].spuOffset);	// channel, spu offset
			SpuSetKey(SPU_ON, 1 << voiceLoop);										// on/off, 1 << channel
			return voiceLoop;
		}
	}
#ifdef _DEBUG
	printf("\nSFX Overflow");
#endif
	return -1;
}


/**************************************************************************
	EXPORTED FUNCTIONS
**************************************************************************/


/**************************************************************************
	FUNCTION:	sfxSetVolume()
	PURPOSE:	sets volume of a channel
	PARAMETERS:	channel number, volume
	RETURNS:	none
**************************************************************************/

void sfxSetVolume(int sampleNum, int vol)
{
	if (sampleNum < 0 || sampleNum > 99) return;
	SpuSetVoiceVolume(sampleNum,(vol*sampleBank.volume)/100,(vol*sampleBank.volume)/100);	// channel, left vol, right vol,
}


/**************************************************************************
	FUNCTION:	sfxStopVoice()
	PURPOSE:	stops a channel
	PARAMETERS:	channel number
	RETURNS:	none
**************************************************************************/

void sfxStopVoice(int sfxNum)
{
	if (sfxNum < 0 || sfxNum > 99) return;
	SpuSetKey(SPU_OFF, 1 << sfxNum);
}


/**************************************************************************
	FUNCTION:	sfxQueueStopSample()
	PURPOSE:	queues a stop request
	PARAMETERS:	sample number
	RETURNS:	none
**************************************************************************/

void sfxQueueStopSample(int sfxNum)
{
	if (((listHead+1)%SAMPLELIST_LENGTH)==listTail) return;

	sfxList[listHead].type = STOP_SAMPLE;

	sfxList[listHead].sample = sfxNum;
	sfxList[listHead].returnAddress = 0;

	listHead = (listHead+1)%SAMPLELIST_LENGTH;
}


/**************************************************************************
	FUNCTION:	sfxPlaySound()
	PURPOSE:	plays a sound
	PARAMETERS:	sample number, volume
	RETURNS:	channel number
**************************************************************************/

unsigned long sfxPlaySound(int sfxNum, int vol)
{
	sampleBank.samples[sfxNum].leftVol = vol;
	sampleBank.samples[sfxNum].rightVol = vol;
	return (sfxPlayVoice(sfxNum));
}


/**************************************************************************
	FUNCTION:	sfxQueueSound2()
	PURPOSE:	queues a sound request
	PARAMETERS:	sample number, volume, pitch, variable to set with channel number
	RETURNS:	none
**************************************************************************/

void sfxQueueSound2(int sampleNum, int vol, int pitch,unsigned char *address)
{
	if (((listHead+1)%SAMPLELIST_LENGTH)==listTail) return;

	sfxList[listHead].type = PLAY_SAMPLE;

	sfxList[listHead].leftVolume = vol;
	sfxList[listHead].rightVolume = vol;
	sfxList[listHead].pitch = pitch;
	sfxList[listHead].sample = sampleNum;
	sfxList[listHead].returnAddress = address;
	listHead = (listHead+1)%SAMPLELIST_LENGTH;

}


/**************************************************************************
	FUNCTION:	sfxQueueSound()
	PURPOSE:	queues a sound request
	PARAMETERS:	sample number, volume, pitch
	RETURNS:	none
**************************************************************************/

void sfxQueueSound(int sampleNum, int vol, int pitch)
{
	if (((listHead+1)%SAMPLELIST_LENGTH)==listTail) return;

	sfxList[listHead].type = PLAY_SAMPLE;

	sfxList[listHead].leftVolume = vol;
	sfxList[listHead].rightVolume = vol;
	sfxList[listHead].pitch = pitch;
	sfxList[listHead].sample = sampleNum;
	sfxList[listHead].returnAddress = 0;
	listHead = (listHead+1)%SAMPLELIST_LENGTH;
}


/**************************************************************************
	FUNCTION:	sfxQueueSetPitch()
	PURPOSE:	queues a volume set request
	PARAMETERS:	sample number, volume
	RETURNS:	none
**************************************************************************/

void sfxQueueSetPitch(int sampleNum, int pitch)
{
	if (((listHead+1)%SAMPLELIST_LENGTH)==listTail) return;

	sfxList[listHead].type = SET_PITCH;

	sfxList[listHead].pitch = pitch;
	sfxList[listHead].sample = sampleNum;
	sfxList[listHead].returnAddress = 0;

	listHead = (listHead+1)%SAMPLELIST_LENGTH;
}


/**************************************************************************
	FUNCTION:	sfxQueueSetVolume()
	PURPOSE:	queues a volume set request
	PARAMETERS:	sample number, volume
	RETURNS:	none
**************************************************************************/

void sfxQueueSetVolume(int sampleNum, int vol)
{
	if (((listHead+1)%SAMPLELIST_LENGTH)==listTail) return;

	sfxList[listHead].type = SET_VOLUME;

	sfxList[listHead].leftVolume = vol;
	sfxList[listHead].sample = sampleNum;
	sfxList[listHead].returnAddress = 0;

	listHead = (listHead+1)%SAMPLELIST_LENGTH;
}


/**************************************************************************
	FUNCTION:	sfxPlaySound3D()
	PURPOSE:	Plays the requested sound in 3D space
	PARAMETERS:	sample number, position in space, camera reference points
	RETURNS:	none
**************************************************************************/

void sfxPlaySound3D(int sampleNum, VECTOR *position, VECTOR *camPos, VECTOR *camRef)
{
	int leftVol, rightVol;

	if(sfxCalc3DPosition(position, camPos, camRef, &leftVol, &rightVol))
	{
		sampleBank.samples[sampleNum].leftVol = leftVol;
		sampleBank.samples[sampleNum].rightVol = rightVol;

		sfxPlayVoice(sampleNum);
	}
}


/**************************************************************************
	FUNCTION:	sfxQueueSound3D()
	PURPOSE:	queues the requested sound in 3D space
	PARAMETERS:	sample number, position in space
	RETURNS:	none
**************************************************************************/

void sfxQueueSound3D(int sampleNum, VECTOR *position, VECTOR *camPos, VECTOR *camRef, int pitch)
{
	int leftVol, rightVol;

	if (((listHead+1)%SAMPLELIST_LENGTH)==listTail) return;

	if(sfxCalc3DPosition(position, camPos, camRef, &leftVol, &rightVol))
	{
		sfxList[listHead].leftVolume = leftVol;
		sfxList[listHead].rightVolume = rightVol;
		sfxList[listHead].pitch = pitch;
		sfxList[listHead].sample = sampleNum;
		listHead = (listHead+1)%SAMPLELIST_LENGTH;
	}
}


/**************************************************************************
	FUNCTION:	sfxInitialise()
	PURPOSE:	Initialise sound processor & routines
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxInitialise()
{
	unsigned long loop;
#ifdef _DEBUG
	printf("\nInitialising sound system...");
#endif
	SpuInit ();									// initialise spu
	SpuSetCommonMasterVolume(0x3fff,0x3fff);	// set master volume to max
	SpuSetCommonCDMix(SPU_ON);					// turn cd audio off
	SpuSetTransferCallback(NULL);
	SpuInitMalloc(4,sampleBank.rec);
	sfxFrameNum = 0;
	for (loop=0; loop<MAX_SAMPLES; loop++)
		lastSampleCall[loop] = -10;
	listHead=listHead=0;
	listHead=listTail=0;
	sampleBank.samples=0;
	sampleBank.volume = 100;
}


/**************************************************************************
	FUNCTION:	sfxStartSound()
	PURPOSE:	Start sound DMA processing
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxStartSound()
{
#ifdef _DEBUG
	printf("\nSound system started...");
#endif
	SpuStart();									// start spu dma
}


/**************************************************************************
	FUNCTION:	sfxStopSound()
	PURPOSE:	Stop sound DMA processing
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxStopSound()
{
#ifdef _DEBUG
	printf("\nSound system stopped...");
#endif
	SpuQuit();									// stop spu dma
}


/**************************************************************************
	FUNCTION:	sfxLoadBank()
	PURPOSE:	loads the requested sample bank
	PARAMETERS:	filename of sample bank
	RETURNS:	none
**************************************************************************/

void sfxLoadBank(char *filename)
{
	unsigned char	*vagData;
	unsigned char	*vagPtr;
	unsigned char	*vabData;
	char			vagFile[40];
	char			vabFile[40];
	int				fileLength;
	unsigned short	numVags;
	unsigned short	numProgs;
	unsigned short	*vagInfo;
	unsigned long	vagLoop;
	unsigned long	spuAddr;
	unsigned long	spuWrittenSize;

	if(sampleBank.samples)
		sfxDestroy();

#ifdef _DEBUG
	printf("\nSFX: Loading sound bank %s\n",filename);
#endif
	sprintf(vabFile,"%s.VH",filename);
	sprintf(vagFile,"%s.VB",filename);

	vabData = fileLoad(vabFile, NULL);

	if(vabData == NULL)
		return;

	numVags = ((unsigned short *)vabData)[11];
	numProgs = ((unsigned short *)vabData)[9];
	
	if(numVags == 0)
		return;

	sampleBank.numSamples = numVags;

	sampleBank.samples = MALLOC(sizeof(sfxSampleType) * sampleBank.numSamples);

	if(sampleBank.samples == NULL) return;

	vagData = fileLoad(vagFile,&fileLength);

	if(vagData == NULL)
	{
		FREE(vabData);
		return;
	}
	
	vagPtr = vagData;
	vagInfo = (unsigned short *) (vabData + 2082 + (512 * numProgs));

	
	
#ifdef _DEBUG
	printf("\nSFX: Found %d programs",numProgs);
	printf("\nSFX: %d samples found",numVags);
#endif
	

	spuAddr = SpuMalloc(fileLength);
	sampleBank.startAddr = spuAddr;

#ifdef _DEBUG
	if(spuAddr < 0x100f)
		printf("\nSample memory allocation failed");
	else
		printf("\nSample bank allocated at %d",spuAddr);
#endif

	SpuSetTransferStartAddr(spuAddr);
	spuWrittenSize = SpuWrite(vagData,fileLength);

#ifdef _DEBUG
	if(spuWrittenSize != fileLength)
		printf("\nSFX: Could only upload %d bytes to SPU",spuWrittenSize);
#endif
	
	for(vagLoop = 0; vagLoop < numVags; vagLoop ++)
	{
		sampleBank.samples[vagLoop].spuOffset = spuAddr;
		sampleBank.samples[vagLoop].pitches = 0x0800;
		sampleBank.samples[vagLoop].size = (*vagInfo << 3);
		sampleBank.samples[vagLoop].leftVol = 0x3fff;
		sampleBank.samples[vagLoop].rightVol = 0x3fff;
		spuAddr += (*vagInfo << 3);
#ifdef _DEBUG
		printf("\nSFX: Sample %d, offset %d, size %d",vagLoop,sampleBank.samples[vagLoop].spuOffset - 0x1010,(*vagInfo << 3));
#endif
		vagInfo ++;
	}

	SpuIsTransferCompleted(SPU_TRANSFER_WAIT);

#ifdef _DEBUG
	printf("\nSFX: Sample upload complete\n");
#endif

	FREE(vagPtr);
	FREE(vabData);

	sfxOn();
	listHead=0;
	listTail=0;
}


/**************************************************************************
	FUNCTION:	sfxDestroy()
	PURPOSE:	shuts down sound and frees memory
	PARAMETERS:	filename of sample bank
	RETURNS:	none
**************************************************************************/

void sfxDestroy()
{
	SpuReverbAttr r_attr;

	SpuSetKey(SPU_OFF,0xFFFFFFFF);
	if(sampleBank.samples)
	{
		FREE(sampleBank.samples);	
		sampleBank.samples=0;
	}
	SpuFree(sampleBank.startAddr);

	r_attr.mask = (SPU_REV_MODE);
	r_attr.mode = SPU_REV_MODE_OFF;
	SpuSetReverbModeParam (&r_attr);

	listHead=0;
	listTail=0;

	SpuSetReverb(SPU_OFF);
}


/**************************************************************************
	FUNCTION:	sfxDisplayDebug()
	PURPOSE:	display status of sound chip
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxDisplayDebug()
{
#ifdef _DEBUG
	char status[25];
	int i;
	SpuGetAllKeysStatus(status);
	for (i=0; i<24; i++)
	{
		switch (status[i])
		{
			case SPU_OFF:			status[i]='-'; break;
			case SPU_ON:			status[i]='X'; break;
			case SPU_OFF_ENV_ON:	status[i]='<'; break;
			case SPU_ON_ENV_OFF:	status[i]='>'; break;
		}
	}
	status[24]=0;
	printf("\n%s",status);
#endif
}


/**************************************************************************
	FUNCTION:	sfxSetPitch()
	PURPOSE:	set pitch of channel
	PARAMETERS:	channel number, pitch
	RETURNS:	none
**************************************************************************/

void sfxSetPitch(int sampleNum, int pitch)
{
	if(sampleNum<0) return;
	if(sampleNum>23) return;

	SpuSetVoicePitch(sampleNum,pitch);
}


/**************************************************************************
	FUNCTION:	sfxFrame()
	PURPOSE:	queueing handler
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxFrame()
{
	int sfxNum;

	sfxFrameNum ++;

	if(listHead!=listTail)
	{
		switch(sfxList[listTail].type)
		{
		case PLAY_SAMPLE:
			sampleBank.samples[sfxList[listTail].sample].leftVol = sfxList[listTail].leftVolume;
			sampleBank.samples[sfxList[listTail].sample].rightVol = sfxList[listTail].rightVolume;
			sfxNum = sfxPlayVoice(sfxList[listTail].sample);

			if(sfxNum!=-1)
			{
//				printf("\nSOUND: Playing sound %d at %d",sfxList[listTail].sample,sfxList[listTail].pitch);
				sfxSetPitch(sfxNum,sfxList[listTail].pitch);
				if (sfxList[listTail].returnAddress)
					*(sfxList[listTail].returnAddress) = sfxNum;
				listTail = (listTail+1)%SAMPLELIST_LENGTH;
			}
//			else
//				printf("\nSOUND: Failed to play sound");

//			printf("\nSOUND: Returned %d",sfxNum);

			break;
		case SET_PITCH:
//			printf("\nSOUND: Setting Pitch to %d",sfxList[listTail].pitch);
			sfxSetPitch(sfxList[listTail].sample,sfxList[listTail].pitch);
			listTail = (listTail+1)%SAMPLELIST_LENGTH;
			break;
		case SET_VOLUME:
//			printf("\nSOUND: Setting Volume to %d",sfxList[listTail].leftVolume);
			sfxSetVolume(sfxList[listTail].sample,sfxList[listTail].leftVolume);
			listTail = (listTail+1)%SAMPLELIST_LENGTH;
			break;
		case STOP_SAMPLE:
//			printf("\nSOUND: Setting Volume to %d",sfxList[listTail].leftVolume);
			sfxStopVoice(sfxList[listTail].sample);
			listTail = (listTail+1)%SAMPLELIST_LENGTH;
			break;
		}
	}
}


/**************************************************************************
	FUNCTION:	sfxOn()
	PURPOSE:	Turn sound output on
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxOn()
{
	//mute off = sound effects on. see?
	SpuSetMute(SPU_OFF);
}


/**************************************************************************
	FUNCTION:	sfxOff()
	PURPOSE:	Turn sound output off
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxOff()
{
	//mute on = sound effects off. get it now?
	SpuSetMute(SPU_ON);
}


/**************************************************************************
	FUNCTION:	sfxSetGlobalVolume()
	PURPOSE:	Set global volume
	PARAMETERS:	volume
	RETURNS:	none
**************************************************************************/

void sfxSetGlobalVolume(int vol)
{
	sampleBank.volume = vol;
}
