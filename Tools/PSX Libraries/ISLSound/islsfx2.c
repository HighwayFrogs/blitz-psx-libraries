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


#define SFX2_MAXSAMPLES 100
#define SFX2_MAXBANKS	16


typedef struct _Sfx2Data {
	SfxBankType	*sfx2Banks[SFX2_MAXBANKS];
	char		sfx2Mallocs[SPU_MALLOC_RECSIZ * (SFX2_MAXSAMPLES+1)];
	int			sfx2SampleVolume;
} Sfx2Data;

static Sfx2Data sfx2Data;




// can't change the interface too much, or everyone will be really miffed.
// so here's the current interface for reference

//void sfxInitialise()
//void sfxDestroy()

//void sfxLoadBank(char *filename)

//void sfxStartSound()
//void sfxStopSound()
//void sfxOn()
//void sfxOff()

//void sfxFrame()

//void sfxPlaySound(int sampleNum, int vol, int pitch)
//void sfxPlaySound2(int sampleNum, int vol, int pitch,unsigned char *address)
//void sfxPlaySound3D(int sampleNum, VECTOR *position, VECTOR *camPos, VECTOR *camRef, int pitch)
//void sfxStopChannel(int sfxNum)

//void sfxSetPitch(int sampleNum, int pitch)
//void sfxSetVolume(int sampleNum, int vol)
//void sfxSetVoicePitch(int sampleNum, int pitch)
//void sfxSetGlobalVolume(int vol)

//void sfxDisplayDebug()


/**************************************************************************
	FUNCTION:	sfxInitialise()
	PURPOSE:	Initialise the sound library
	PARAMETERS:	maximum reverb setting
	RETURNS:	none
**************************************************************************/

void sfxInitialise(int maxReverb)
{
	SpuReverbAttr	sceReverb;	// SCE reverb struct
	SpuEnv			sceEnv;		// SCE environment struct
	int				loop;

	
	for(loop = 0; loop < SFX2_MAXBANKS; loop ++)
	{
		sfx2Data.sfx2Banks[loop] = NULL;
	}
	
	SpuInit ();									// initialise spu
	SpuSetCommonMasterVolume(0x3fff,0x3fff);	// set master volume to max
	SpuSetCommonCDMix(SPU_ON);					// turn cd audio on
	SpuSetTransferCallback(NULL);				// no transfer callback
	SpuSetTransferMode(SPU_TRANSFER_BY_DMA);	// transfer data by dma
	SpuInitMalloc(SFX2_MAXSAMPLES, sfx2Data.sfx2Mallocs);// init spu malloc
	
	if(maxReverb)								// does client want to use reverb features?
	{
		sceReverb.mask = SPU_REV_MODE | SPU_REV_FEEDBACK | SPU_REV_DELAYTIME;
		sceReverb.mode = SPU_REV_MODE_HALL;
		sceReverb.delay = maxReverb;
		sceReverb.feedback = maxReverb;

		SpuSetReverbModeParam(&sceReverb);		// set reverb mode using reverb struct
		SpuSetReverbVoice(SPU_BIT, 0xFFFFFFFF);	// make sure reverb is on for all voices
		
		if (SpuSetReverb(SPU_ON) != SPU_ON)		// switch reverb on
			utilPrintf("Unable to activate reverb\n");
		SpuReserveReverbWorkArea(SPU_ON);		// reserve memory for reverb
	}

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
	
	SpuSetKey(SPU_OFF,0xFFFFFFFF);				// turn off all voices

	sceReverb.mask = SPU_REV_MODE;				// turn off reverb
	sceReverb.mode = SPU_REV_MODE_OFF;
	SpuSetReverbModeParam(&sceReverb);

	// now go and SpuFree all the space that got SpuMalloc'd
	for(loop = 0; loop < SFX2_MAXBANKS; loop ++)
	{
		if(sfx2Data.sfx2Banks[loop])
		{
			sfxUnloadBank(sfx2Data.sfx2Banks[loop]);
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
		utilPrintf("sfxLoadSampleBankBody: SpuMalloc returned 0x%x\n", spuAddr);
		SpuSetTransferStartAddr(spuAddr);
		spuWrittenSize = SpuWrite(sampleData, fileSize);
		utilPrintf("sfxLoadSampleBankBody: Uploaded %d / %d bytes to SPU\n", spuWrittenSize, fileSize);
		SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
		FREE(sampleData);
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

	if(sample->inSPURam)
		return sample;

	spuAddr = SpuMalloc(sample->sampleSize);
	if(spuAddr)
	{
		SpuSetTransferStartAddr(spuAddr);
		spuWrittenSize = SpuWrite(sample->sampleData, sample->sampleSize);
		SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
		if(spuWrittenSize == sample->sampleSize)
		{
			sample->inSPURam = 1;
			sample->spuOffset = spuAddr;
			return sample;
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
	if(!sample->inSPURam)
		return NULL;

	SpuFree(sample->spuOffset);

	return sample;
}


/**************************************************************************
	FUNCTION:	sfxUnloadBank
	PURPOSE:	Unload sample bank from SPU ram
	PARAMETERS:	pointer to sample bank
	RETURNS:	pointer to sample bank, or NULL if failed
**************************************************************************/

SfxBankType *sfxUnloadBank(SfxBankType *bank)
{
	int loop;
	
	if(bank)
	{
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
	PURPOSE:	Set global sample volume
	PARAMETERS:	volume
	RETURNS:	none
**************************************************************************/

void sfxSetGlobalVolume(int vol)
{
	sfx2Data.sfx2SampleVolume = vol;
}
