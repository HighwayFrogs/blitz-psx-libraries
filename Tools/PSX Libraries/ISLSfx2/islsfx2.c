/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islsfx2.c:		Sound fx handling 2.0

************************************************************************************/

#include <stdio.h>
#include <stddef.h>
#include <libspu.h>
#include "..\islmem\islmem.h"
#include "..\islfile\islfile.h"
#include "..\islutil\islutil.h"
#include "islsfx2.h"


//#define DEBUGGING


#define SFX2_MAXSAMPLES 100
#define SFX2_MAXBANKS	16


typedef struct _Sfx2Data {
	SfxBankType		*sfx2Banks[SFX2_MAXBANKS];		// sample bank pointers
	char			sfx2Mallocs[SPU_MALLOC_RECSIZ * (SFX2_MAXSAMPLES+1)];	// spu ram malloc space
	int				sfx2SampleVolume;				// global sample volume (0 - 255)
	int				sfx2Reverb;						// reverb active
	SfxSampleType	*sfx2Active[24];				// last played sample on this channel
	int				sfx2AllocatedMemory;			// amount of memory allocated
} Sfx2Data;

static Sfx2Data sfx2Data;



/**************************************************************************
	FUNCTION:	sfxInitialise()
	PURPOSE:	Initialise the sound library
	PARAMETERS:	reverb setting
	RETURNS:	none
**************************************************************************/

void sfxInitialise(int reverbMode)
{
	SpuReverbAttr	sceReverb;	// SCE reverb struct
	SpuEnv			sceEnv;		// SCE environment struct
	int				loop;

#ifdef DEBUGGING
	printf("sfxInitialise: Initialising sfx2\n");
#endif
	
	for(loop = 0; loop < SFX2_MAXBANKS; loop ++)
	{
		sfx2Data.sfx2Banks[loop] = NULL;
	}
	
	SpuInit ();									// initialise spu
	SpuSetCommonMasterVolume(0x3fff, 0x3fff);	// set master volume to max
	SpuSetCommonCDMix(SPU_ON);					// turn cd audio on
	SpuSetTransferCallback(NULL);				// no transfer callback
	// Again, the SCE libs are broken, so transferring data by DMA is out
	//SpuSetTransferMode(SPU_TRANSFER_BY_DMA);	// transfer data by dma
	SpuSetTransferMode(SPU_TRANSFER_BY_IO);

	SpuInitMalloc(SFX2_MAXSAMPLES, sfx2Data.sfx2Mallocs);// init spu malloc
	
	if(reverbMode)								// does client want to use reverb features?
	{
#ifdef DEBUGGING
	printf("sfxInitialise: allocating reverb space\n");
#endif
		sceReverb.mask = SPU_REV_MODE | SPU_REV_FEEDBACK | SPU_REV_DELAYTIME;
		sceReverb.mode = reverbMode;
		sceReverb.depth.left = 0x7fff;
		sceReverb.depth.right = 0x7fff;
		sceReverb.delay = 64;
		sceReverb.feedback = 64;

		SpuSetReverbModeParam(&sceReverb);		// set reverb mode using reverb struct

		sceReverb.mask = 0;
		SpuSetReverbDepth(&sceReverb);

		SpuSetReverbVoice(SPU_ON, SPU_ALLCH);	// make sure reverb is on for all voices
		
		if(SpuSetReverb(SPU_ON))
		{
#ifdef DEBUGGING
	printf("sfxInitialise: reverb initialised\n");
#endif
			SpuReserveReverbWorkArea(SPU_ON);
			sfx2Data.sfx2Reverb = reverbMode;
			switch(reverbMode)
			{
			case SFX_REVERB_MODE_ROOM:
				sfx2Data.sfx2AllocatedMemory = 9920 + 0x1010;
				break;
			case SFX_REVERB_MODE_SMALL_STUDIO:
				sfx2Data.sfx2AllocatedMemory = 8000 + 0x1010;
				break;
			case SFX_REVERB_MODE_MEDIUM_STUDIO:
				sfx2Data.sfx2AllocatedMemory = 18496 + 0x1010;
				break;
			case SFX_REVERB_MODE_LARGE_STUDIO:
				sfx2Data.sfx2AllocatedMemory = 28640 + 0x1010;
				break;
			case SFX_REVERB_MODE_HALL:
				sfx2Data.sfx2AllocatedMemory = 44512 + 0x1010;
				break;
			case SFX_REVERB_MODE_SPACE:
				sfx2Data.sfx2AllocatedMemory = 63168 + 0x1010;
				break;
			case SFX_REVERB_MODE_ECHO:
			case SFX_REVERB_MODE_DELAY:
				sfx2Data.sfx2AllocatedMemory = 98368 + 0x1010;
				break;
			case SFX_REVERB_MODE_PIPE:
				sfx2Data.sfx2AllocatedMemory = 15360 + 0x1010;
				break;
			}
		}
		else
		{
			sfx2Data.sfx2AllocatedMemory = 0x1010;
		}
	}

	sfx2Data.sfx2SampleVolume = 255;

	sceEnv.mask = 0;							// turn voice queueing on
	sceEnv.queueing = SPU_ON;

	SpuSetEnv(&sceEnv);
}


/**************************************************************************
	FUNCTION:	sfxDestroy()
	PURPOSE:	Shutdown the sound library
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxDestroy()
{
	int	loop;
	SpuReverbAttr	sceReverb;					// SCE reverb struct

#ifdef DEBUGGING
	printf("sfxDestroy: Shutting down sfx2\n");
#endif
	
	SpuSetKey(SPU_OFF, 0xFFFFFFFF);				// turn off all voices

	sceReverb.mask = SPU_REV_MODE;				// turn off reverb
	sceReverb.mode = SPU_REV_MODE_OFF;
	SpuSetReverbModeParam(&sceReverb);
	SpuReserveReverbWorkArea(SPU_OFF);

	// now go and SpuFree all the space that got SpuMalloc'd
	for(loop = 0; loop < SFX2_MAXBANKS; loop ++)
	{
		if(sfx2Data.sfx2Banks[loop])
		{
			sfxUnloadSampleBank(sfx2Data.sfx2Banks[loop]);
		}
	}
}


/**************************************************************************
	FUNCTION:	sfxFixupSampleBankHeader()
	PURPOSE:	fix up sample bank header
	PARAMETERS:	pointer to sample bank header, spu base address
	RETURNS:	pointer to sample bank header
**************************************************************************/

SfxBankType *sfxFixupSampleBankHeader(SfxBankType *bank, unsigned long spuAddr)
{
	int	loop;

	if(!bank)
		return NULL;

	for(loop = 0; loop < SFX2_MAXBANKS; loop ++)
	{
		if(sfx2Data.sfx2Banks[loop] == NULL)
		{
			sfx2Data.sfx2Banks[loop] = bank;
			break;
		}
	}

	(unsigned long)bank->sample = (unsigned long)bank + sizeof(SfxBankType);
	for(loop = 0; loop < bank->numSamples; loop ++)
	{
		bank->sample[loop].inSPURam = 1;
		bank->sample[loop].spuOffset = (unsigned long)(bank->sample[loop].sampleData) + spuAddr;
	}

	bank->baseAddr = spuAddr;

	return bank;
}


/**************************************************************************
	FUNCTION:	sfxLoadSampleBankBody()
	PURPOSE:	Load sample bank body into spu ram
	PARAMETERS:	filename of sample bank (minus extension)
	RETURNS:	spu base address
**************************************************************************/

unsigned long sfxLoadSampleBankBody(char *fileName)
{
	unsigned long	spuAddr;
	unsigned long	spuWrittenSize;
	unsigned char	*sampleData;
	char			bodyFileName[20];
	int				fileSize;

	sprintf(bodyFileName, "%s.SBB", fileName);
	sampleData = fileLoad(bodyFileName, &fileSize);

	if(sampleData)
	{
		spuAddr = SpuMalloc(fileSize);
		if(spuAddr)
		{
			SpuSetTransferStartAddr(spuAddr);
			spuWrittenSize = SpuWrite(sampleData, fileSize);
			SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
			FREE(sampleData);
			sfx2Data.sfx2AllocatedMemory += fileSize;
		}
		return spuAddr;
	}
	else
	{
		return 0;
	}
}


/**************************************************************************
	FUNCTION:	sfxLoadSampleBank()
	PURPOSE:	Load a sample bank into ram
	PARAMETERS:	filename of sample bank (minus extension)
	RETURNS:	pointer to sample bank
**************************************************************************/

SfxBankType *sfxLoadSampleBank(char *fileName)
{
	SfxBankType		*newBank;
	char			headerFileName[20];
	char			bodyFileName[20];
	unsigned char	*sampleData;
	int				loop;

#ifdef DEBUGGING
	printf("sfxLoadSampleBank: Loading bank %s\n", fileName);
#endif
	
	// sort out filenames
	sprintf(headerFileName, "%s.SBH", fileName);
	sprintf(bodyFileName, "%s.SBB", fileName);

	// load bank header
	newBank = (SfxBankType *)fileLoad(headerFileName, 0);

	// put bank header into internal list
	if(newBank)
	{
		for(loop = 0; loop < SFX2_MAXBANKS; loop ++)
		{
			if(sfx2Data.sfx2Banks[loop] == NULL)
			{
				sfx2Data.sfx2Banks[loop] = newBank;
				break;
			}
		}
	}
	else
	{
		return 0;
	}

	// load bank body
	sampleData = fileLoad(bodyFileName, 0);

	if(sampleData)
	{
		newBank->sampleData = sampleData;
		(unsigned long)newBank->sample = (unsigned long)newBank + sizeof(SfxBankType);

		// fix up sample data pointers
		for(loop = 0; loop < newBank->numSamples; loop ++)
		{
#ifdef DEBUGGING
	printf("sfxLoadSampleBank: sample %d/%d @ 0x%x\n", loop, newBank->numSamples, (unsigned long)newBank->sample[loop].sampleData);
#endif
			(unsigned long)newBank->sample[loop].sampleData += (unsigned long)sampleData;
		}
	}

	return newBank;	
}


/**************************************************************************
	FUNCTION:	sfxFindSampleInBank()
	PURPOSE:	Find a sample in a bank
	PARAMETERS:	sample name
	RETURNS:	pointer to sample, or NULL if not found
**************************************************************************/

SfxSampleType *sfxFindSampleInBank(char *sampleName, SfxBankType *bank)
{
	unsigned long	CRC;
	int				loop;

	CRC = utilStr2CRC(sampleName);

	for(loop = 0; loop < bank->numSamples; loop ++)
	{
#ifdef DEBUGGING
	printf("sfxFindSampleInBank: looking for CRC 0x%x, found 0x%x\n", CRC, bank->sample[loop].nameCRC);
#endif
		if(bank->sample[loop].nameCRC == CRC)
			return &bank->sample[loop];
	}

	return NULL;
}


/**************************************************************************
	FUNCTION:	sfxFindSampleInAllBanks()
	PURPOSE:	Find a sample in all loaded banks
	PARAMETERS:	sample name
	RETURNS:	pointer to sample, or NULL if not found
**************************************************************************/

SfxSampleType *sfxFindSampleInAllBanks(char *sampleName)
{
	int				loop;
	SfxSampleType	*sample;

	for(loop = 0; loop < SFX2_MAXBANKS; loop ++)
	{
		if(sfx2Data.sfx2Banks[loop])
		{
			if((sample = sfxFindSampleInBank(sampleName, sfx2Data.sfx2Banks[loop])))
			{
				return sample;
			}
		}
	}

	return NULL;
}


/**************************************************************************
	FUNCTION:	sfxDownloadSample
	PURPOSE:	Download sample to SPU ram
	PARAMETERS:	pointer to sample
	RETURNS:	pointer to sample, or NULL if failed
**************************************************************************/

SfxSampleType *sfxDownloadSample(SfxSampleType *sample)
{
	unsigned long	spuAddr;
	unsigned long	spuWrittenSize;

	// if sample is already in spu ram, don't load it again
	if(sample->inSPURam)
	{
#ifdef DEBUGGING
	printf("sfxDownloadSample: sample already loaded @ 0x%x\n", sample->spuOffset);
#endif
		return sample;
	}

	spuAddr = SpuMalloc(sample->sampleSize);
#ifdef DEBUGGING
	printf("sfxDownloadSample: SpuMalloc returned 0x%x\n", spuAddr);
#endif
	if(spuAddr)
	{
		SpuSetTransferStartAddr(spuAddr);
		spuWrittenSize = SpuWrite(sample->sampleData, sample->sampleSize);
		// It appears that the SCE libs are broken, as this call will fail the second time around the library is used
		//SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
		if(spuWrittenSize == sample->sampleSize)
		{
#ifdef DEBUGGING
	printf("sfxDownloadSample: downloaded sample to 0x%x\n", spuAddr);
#endif
			sample->inSPURam = 1;
			sample->spuOffset = spuAddr;
			sfx2Data.sfx2AllocatedMemory += sample->sampleSize;
			return sample;
		}

		SpuFree(spuAddr);
	}

	return NULL;
}


/**************************************************************************
	FUNCTION:	sfxDownloadSampleBank
	PURPOSE:	Download entire sample bank to SPU ram
	PARAMETERS:	pointer to sample bank
	RETURNS:	pointer to sample bank, or NULL if failed
**************************************************************************/

SfxBankType *sfxDownloadSampleBank(SfxBankType *bank)
{
	int	loop;

	if(bank)
	{
		for(loop = 0; loop < bank->numSamples; loop ++)
		{
			if(sfxDownloadSample(&bank->sample[loop]) == NULL)
			{
				return NULL;
			}
		}

		return bank;
	}

	return NULL;
}


/**************************************************************************
	FUNCTION:	sfxDestroySampleBank
	PURPOSE:	Destroy ram copy of sample data
	PARAMETERS:	pointer to sample bank
	RETURNS:	pointer to samplebank, or NULL if failed
**************************************************************************/

SfxBankType *sfxDestroySampleBank(SfxBankType *bank)
{
	if(bank)
	{
		if(bank->sampleData)
		{
			FREE(bank->sampleData);
			bank->sampleData = NULL;
			return bank;
		}
	}

	return NULL;
}


/**************************************************************************
	FUNCTION:	sfxUnloadSample
	PURPOSE:	Unload sample from SPU ram
	PARAMETERS:	pointer to sample
	RETURNS:	pointer to sample, or NULL if failed
**************************************************************************/

SfxSampleType *sfxUnloadSample(SfxSampleType *sample)
{
	// make sure sample is in spu ram
	if(!sample->inSPURam)
		return NULL;

	// free the spu ram
	SpuFree(sample->spuOffset);
	sfx2Data.sfx2AllocatedMemory -= sample->sampleSize;
	// clear flags
	sample->inSPURam = 0;
	sample->spuOffset = 0;

	return sample;
}


/**************************************************************************
	FUNCTION:	sfxUnloadSampleBank
	PURPOSE:	Unload sample bank from SPU ram
	PARAMETERS:	pointer to sample bank
	RETURNS:	pointer to sample bank, or NULL if failed
**************************************************************************/

SfxBankType *sfxUnloadSampleBank(SfxBankType *bank)
{
	int loop;
	
	// make sure bank is not null
	if(bank)
	{
		// unload all samples in bank
		for(loop = 0; loop < bank->numSamples; loop ++)
		{
			sfxUnloadSample(&bank->sample[loop]);
		}

		return bank;
	}

	return NULL;
}


/**************************************************************************
	FUNCTION:	sfxOn()
	PURPOSE:	Turn sound output on
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxOn()
{
	SpuSetMute(SPU_OFF);						// mute off = sound on
}


/**************************************************************************
	FUNCTION:	sfxOff()
	PURPOSE:	Turn sound output off
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxOff()
{
	SpuSetMute(SPU_ON);							// mute on = sound off
}


/**************************************************************************
	FUNCTION:	sfxStartSound()
	PURPOSE:	Start sound DMA processing
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxStartSound()
{
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
	SpuQuit();									// stop spu dma
}


/**************************************************************************
	FUNCTION:	sfxSetGlobalVolume()
	PURPOSE:	Set global sample volume (0 - 255)
	PARAMETERS:	volume
	RETURNS:	none
**************************************************************************/

void sfxSetGlobalVolume(int vol)
{
	sfx2Data.sfx2SampleVolume = vol;
}


/**************************************************************************
	FUNCTION:	sfxSetReverb()
	PURPOSE:	Set reverb level
	PARAMETERS:	delay (0 - 128), depth (0 - 255).
	RETURNS:	1 if successful, 0 if failure
	NOTE:		delay is only effective on SFX_REVERB_MODE_ECHO & SFX_REVERB_MODE_DELAY
**************************************************************************/

int sfxSetReverb(int delay, int depth)
{
	SpuReverbAttr	sceReverb;	// SCE reverb struct

	// if less than max reverb set at initialisation
	if(sfx2Data.sfx2Reverb)
	{
		sceReverb.mask = SPU_REV_MODE | SPU_REV_FEEDBACK | SPU_REV_DELAYTIME;
		sceReverb.mode = sfx2Data.sfx2Reverb;
		sceReverb.delay = delay;
		sceReverb.feedback = delay;

		SpuSetReverbModeParam(&sceReverb);		// set reverb mode using reverb struct

		sceReverb.mask = 0;
		sceReverb.depth.left = depth * (0x7fff / 255);
		sceReverb.depth.right = sceReverb.depth.left;
		
		SpuSetReverbDepth(&sceReverb);

		return 1;
	}
	
	return 0;
}


/**************************************************************************
	FUNCTION:	sfxPlaySample()
	PURPOSE:	Plays the requested sample
	PARAMETERS:	pointer to sample, left & right volume (0 - 255), pitch (Hz, 0 for default)
	RETURNS:	channel number, or -1 if failure
**************************************************************************/

int sfxPlaySample(SfxSampleType *sample, int volL, int volR, int pitch)
{
	int voiceLoop, tmpVolL, tmpVolR, tmpPitch;

	// make sure sample is in spu ram
	if(!sample->inSPURam)
		return -1;

	// try to find a free channel
	for(voiceLoop = 0; voiceLoop < 24; voiceLoop ++)
	{
		if(SpuGetKeyStatus(1 << voiceLoop) != SPU_ON)
		{
			// calculate left and right volumes
			if(volL)
			{
				tmpVolL = (((volL * sfx2Data.sfx2SampleVolume) / 256) * 0x3fff) / 256;
			}
			else
			{
				tmpVolL = 0;
			}

			if(volR)
			{
				tmpVolR = (((volR * sfx2Data.sfx2SampleVolume) / 256) * 0x3fff) / 256;
			}
			else
			{
				tmpVolR = 0;
			}

			// calculate pitch
			if(pitch)
			{
				tmpPitch = (pitch * 4096) / 44100;
			}
			else
			{
				tmpPitch = sample->sampleRate;
			}

			SpuSetVoiceVolume(voiceLoop, tmpVolL, tmpVolR);		// channel, left vol, right vol,
			SpuSetVoicePitch(voiceLoop,	tmpPitch);				// channel, pitch
			SpuSetVoiceStartAddr(voiceLoop, sample->spuOffset);	// channel, spu offset
			SpuSetKey(SPU_ON, 1 << voiceLoop);					// on/off, 1 << channel
			sfx2Data.sfx2Active[voiceLoop] = sample;			// store sample pointer
#ifdef DEBUGGING
	printf("sfxPlaySample: playing sample on channel %d, vol 0x%x, pitch 0x%x\n", voiceLoop, tmpVolL, tmpPitch);
#endif

			return voiceLoop;
		}
	}

	// ran out of channels
	return -1;
}


/**************************************************************************
	FUNCTION:	sfxStopSample()
	PURPOSE:	Stop the requested sample
	PARAMETERS:	pointer to sample, channel number (0 - 23) or -1 for all
	RETURNS:	1 if ok, 0 if failure
**************************************************************************/

int sfxStopSample(SfxSampleType *sample, int channel)
{
	int	loop, ok;

	if((channel >= 0) && (channel <= 23))
	{
		if(sfx2Data.sfx2Active[channel] == sample)
		{
			sfxStopChannel(channel);
			return 1;
		}
	}
	else
	{
		if(channel == -1)
		{
			ok = 0;
			for(loop = 0; loop < 24; loop ++)
			{
				if(sfx2Data.sfx2Active[loop] == sample)
				{
					sfxStopChannel(loop);
					ok = 1;
				}
			}

			return ok;
		}
	}

	return 0;
}


/**************************************************************************
	FUNCTION:	sfxStopChannel
	PURPOSE:	Stop a channel from playing
	PARAMETERS:	channel number (0 - 23), or -1 for all
	RETURNS:	none
**************************************************************************/

void sfxStopChannel(int channel)
{
	int	loop;

	if(channel == -1)
	{
		for(loop = 0; loop < 24; loop ++)
		{
			SpuSetKey(SPU_OFF, 1 << loop);
			SpuSetVoiceVolume(loop, 0, 0);	// mute this channel, cuz setting key off doesn't always work
		}
	}
	else
	{
		SpuSetKey(SPU_OFF, 1 << channel);
		SpuSetVoiceVolume(channel, 0, 0);	// mute this channel, cuz setting key off doesn't always work
	}
}


/**************************************************************************
	FUNCTION:	sfxGetSampleStatus
	PURPOSE:	get the status of a sample
	PARAMETERS:	pointer to sample
	RETURNS:	Bitfield of channels currently playing sample
**************************************************************************/

unsigned long sfxGetSampleStatus(SfxSampleType *sample)
{
	int				loop;
	unsigned long	playing;

	playing = 0;

	// loop through stored sample pointers
	for(loop = 0; loop < 24; loop ++)
	{
		// if pointer matches
		if(sfx2Data.sfx2Active[loop] == sample)
		{
			// if channel is playing
			if(SpuGetKeyStatus(1 << loop) == SPU_ON)
			{
				// must be this sample playing then
				playing |= (1 << loop);
			}
		}
	}

	// return bitfield
	return playing;
}


/**************************************************************************
	FUNCTION:	sfxGetChannelStatus
	PURPOSE:	get the status of a channel
	PARAMETERS:	channel number (0 - 23)
	RETURNS:	Currently playing sample, or NULL if no sample playing
**************************************************************************/

SfxSampleType *sfxGetChannelStatus(int channel)
{
	// make sure channel is valid
	if((channel >= 0) && (channel <= 23))
	{
		// if channel is playing
		if(SpuGetKeyStatus(1 << channel) == SPU_ON)
		{
			// return sample pointer
			return sfx2Data.sfx2Active[channel];
		}
	}

	// channel not valid, or isn't playing anything
	return NULL;
}


/**************************************************************************
	FUNCTION:	sfxSetChannelReverb
	PURPOSE:	Turn reverb on/off for a channel
	PARAMETERS:	channel number (0 - 23), status (1 = on, 0 = off)
	RETURNS:	1 if OK, 0 if failure
**************************************************************************/

int sfxSetChannelReverb(int channel, int status)
{
	// make sure channel is valid
	if((channel >= 0) && (channel <= 23))
	{
		if(status)
		{
			// turn on reverb for this channel
			SpuSetReverbVoice(SPU_ON, (1 << channel));
		}
		else
		{
			// turn off reverb for this channel
			SpuSetReverbVoice(SPU_OFF, (1 << channel));
		}

		// okey dokey
		return 1;
	}

	// oops, channel invalid
	return 0;
}


/**************************************************************************
	FUNCTION:	sfxGetChannelReverb
	PURPOSE:	Get reverb status for a channel
	PARAMETERS:	channel number (0 - 23)
	RETURNS:	1 if on, 0 if off, -1 if failure
**************************************************************************/

int sfxGetChannelReverb(int channel)
{
	// make sure channel is valid
	if((channel >= 0) && (channel <= 23))
	{
		// get all voices reverb status
		if(SpuGetReverbVoice() & (1 << channel))
			return 1;
		else
			return 0;
	}

	return -1;
}


/**************************************************************************
	FUNCTION:	sfxGetFreeSoundMemory
	PURPOSE:	Print the amount of free SPU ram
	PARAMETERS:	
	RETURNS:	free ram in bytes
**************************************************************************/

int sfxGetFreeSoundMemory()
{
	int	reverbAmount = 0;

	switch(sfx2Data.sfx2Reverb)
	{
	case SFX_REVERB_MODE_OFF:
		reverbAmount = 0;
		break;
	case SFX_REVERB_MODE_ROOM:
		reverbAmount = 9920;
		break;
	case SFX_REVERB_MODE_SMALL_STUDIO:
		reverbAmount = 8000;
		break;
	case SFX_REVERB_MODE_MEDIUM_STUDIO:
		reverbAmount = 18496;
		break;
	case SFX_REVERB_MODE_LARGE_STUDIO:
		reverbAmount = 28640;
		break;
	case SFX_REVERB_MODE_HALL:
		reverbAmount = 44512;
		break;
	case SFX_REVERB_MODE_SPACE:
		reverbAmount = 63168;
		break;
	case SFX_REVERB_MODE_ECHO:
	case SFX_REVERB_MODE_DELAY:
		reverbAmount = 98368;
		break;
	case SFX_REVERB_MODE_PIPE:
		reverbAmount = 15360;
		break;
	}
	
	printf("sfxGetFreeSoundMemory:\n----------------------\nTotal: %d bytes used (%d bytes available)\n", sfx2Data.sfx2AllocatedMemory, (512*1024)-sfx2Data.sfx2AllocatedMemory);
	printf("Samples: %d bytes used\n", sfx2Data.sfx2AllocatedMemory-reverbAmount-0x1010);
	printf("Reverb: %d bytes used\n", reverbAmount);
	return (512*1024)-sfx2Data.sfx2AllocatedMemory;
}
