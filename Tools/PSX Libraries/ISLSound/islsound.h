/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islsound.h:		Sound fx handling

************************************************************************************/

#ifndef __ISLSOUND_H__
#define __ISLSOUND_H__


/**************************************************************************
	FUNCTION:	sfxInitialise()
	PURPOSE:	Initialise sound processor & routines
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxInitialise();


/**************************************************************************
	FUNCTION:	sfxDestroy()
	PURPOSE:	shuts down sound and frees memory
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxDestroy();


/**************************************************************************
	FUNCTION:	sfxStartSound()
	PURPOSE:	Start sound DMA processing
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxStartSound();


/**************************************************************************
	FUNCTION:	sfxStopSound()
	PURPOSE:	Stop sound DMA processing
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxStopSound();


/**************************************************************************
	FUNCTION:	sfxOn()
	PURPOSE:	Turn sound output on
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxOn();


/**************************************************************************
	FUNCTION:	sfxOff()
	PURPOSE:	Turn sound output off
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxOff();


/**************************************************************************
	FUNCTION:	sfxLoadBank()
	PURPOSE:	loads the requested sample bank
	PARAMETERS:	filename of sample bank
	RETURNS:	none
**************************************************************************/

void sfxLoadBank(char *filename);


/**************************************************************************
	FUNCTION:	sfxPlaySound()
	PURPOSE:	plays a sound
	PARAMETERS:	sample number, volume
	RETURNS:	channel number
**************************************************************************/

unsigned long sfxPlaySound(int sfxNum, int vol);


/**************************************************************************
	FUNCTION:	sfxStopVoice()
	PURPOSE:	stops a channel
	PARAMETERS:	channel number
	RETURNS:	none
**************************************************************************/

void sfxStopVoice(int channelNum);


/**************************************************************************
	FUNCTION:	sfxSetVolume()
	PURPOSE:	sets volume of a channel
	PARAMETERS:	channel number, volume
	RETURNS:	none
**************************************************************************/

void sfxSetVolume(int channelNum, int vol);


/**************************************************************************
	FUNCTION:	sfxSetPitch()
	PURPOSE:	set pitch of channel
	PARAMETERS:	channel number, pitch
	RETURNS:	none
**************************************************************************/

void sfxSetPitch(int channelNum, int pitch);


/**************************************************************************
	FUNCTION:	sfxSetGlobalVolume()
	PURPOSE:	Set global volume
	PARAMETERS:	volume
	RETURNS:	none
**************************************************************************/

void sfxSetGlobalVolume(int vol);


/**************************************************************************
	FUNCTION:	sfxFrame()
	PURPOSE:	queuing handler
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxFrame();


/**************************************************************************
	FUNCTION:	sfxQueueSound()
	PURPOSE:	queues a sound request
	PARAMETERS:	sample number, volume, pitch
	RETURNS:	none
**************************************************************************/

void sfxQueueSound(int sampleNum, int vol, int pitch);


/**************************************************************************
	FUNCTION:	sfxQueueSound2()
	PURPOSE:	queues a sound request
	PARAMETERS:	sample number, volume, pitch, variable to set with channel number
	RETURNS:	none
**************************************************************************/

void sfxQueueSound2(int sampleNum, int vol, int pitch,unsigned char *address);


/**************************************************************************
	FUNCTION:	sfxQueueStopSample()
	PURPOSE:	queues a stop request
	PARAMETERS:	channel number
	RETURNS:	none
**************************************************************************/

void sfxQueueStopSample(int channelNum);


/**************************************************************************
	FUNCTION:	sfxQueueSetVolume()
	PURPOSE:	queues a volume set request
	PARAMETERS:	sample number, volume
	RETURNS:	none
**************************************************************************/

void sfxQueueSetVolume(int sampleNum, int vol);


/**************************************************************************
	FUNCTION:	sfxQueueSetPitch()
	PURPOSE:	queues a volume set request
	PARAMETERS:	sample number, volume
	RETURNS:	none
**************************************************************************/

void sfxQueueSetPitch(int sampleNum, int pitch);


/**************************************************************************
	FUNCTION:	sfxDisplayDebug()
	PURPOSE:	display status of sound chip
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxDisplayDebug();


#endif // __ISLSOUND_H__
