/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islsfx2.h:		Sound fx handling 2.0

************************************************************************************/

#ifndef __ISLSFX2_H__
#define __ISLSFX2_H__

typedef struct _SfxSampleType {
	unsigned char	inSPURam;		// is this sample in SPU ram?
	unsigned char	pad;
	unsigned short	sampleRate;		// default sample rate
	unsigned long	spuOffset;		// offset in SPU ram
	unsigned long	nameCRC;		// CRC of sample name
	unsigned long	sampleSize;		// length of sample data
	unsigned char	*sampleData;	// pointer to sample data in RAM
} SfxSampleType;


typedef struct _SfxBankType {
	unsigned long	numSamples;		// number of samples in this bank
	unsigned long	baseAddr;		// if the bank was downloaded entirely, the base address
	SfxSampleType	*sample;		// array of SfxSampleType's
	unsigned char	*sampleData;	// pointer to all sample data in this bank
} SfxBankType;



// Reverb types
#define SFX_REVERB_MODE_OFF				0	// needs 0 bytes workspace
#define SFX_REVERB_MODE_ROOM			1	// needs 9920 bytes workspace
#define SFX_REVERB_MODE_SMALL_STUDIO	2	// needs 8000 bytes workspace
#define SFX_REVERB_MODE_MEDIUM_STUDIO	3	// needs 18496 bytes workspace
#define SFX_REVERB_MODE_LARGE_STUDIO	4	// needs 28640 bytes workspace
#define SFX_REVERB_MODE_HALL			5	// needs 44512 bytes workspace
#define SFX_REVERB_MODE_SPACE			6	// needs 63168 bytes workspace
#define SFX_REVERB_MODE_ECHO			7	// needs 98368 bytes workspace
#define SFX_REVERB_MODE_DELAY			8	// needs 98368 bytes workspace
#define SFX_REVERB_MODE_PIPE			9	// needs 15360 bytes workspace


/**************************************************************************
	FUNCTION:	sfxInitialise()
	PURPOSE:	Initialise the sound library
	PARAMETERS:	reverb mode
	RETURNS:	none
**************************************************************************/

void sfxInitialise(int reverbMode);


/**************************************************************************
	FUNCTION:	sfxDestroy()
	PURPOSE:	Shutdown the sound library
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxDestroy();


/**************************************************************************
	FUNCTION:	sfxFixupSampleBankHeader()
	PURPOSE:	fix up sample bank header already in ram
	PARAMETERS:	pointer to sample bank header, spu base address
	RETURNS:	pointer to sample bank header
**************************************************************************/

SfxBankType *sfxFixupSampleBankHeader(SfxBankType *bank, unsigned long spuAddr);


/**************************************************************************
	FUNCTION:	sfxLoadSampleBankBody()
	PURPOSE:	Load a sample bank into spu ram
	PARAMETERS:	filename of sample bank (minus extension)
	RETURNS:	spu base address
**************************************************************************/

unsigned long sfxLoadSampleBankBody(char *fileName);


/**************************************************************************
	FUNCTION:	sfxLoadSampleBank()
	PURPOSE:	Load a sample bank into ram
	PARAMETERS:	filename of sample bank (minus extension)
	RETURNS:	pointer to sample bank
**************************************************************************/

SfxBankType *sfxLoadSampleBank(char *fileName);


/**************************************************************************
	FUNCTION:	sfxFindSampleInBank()
	PURPOSE:	Find a sample in a bank
	PARAMETERS:	sample name
	RETURNS:	pointer to sample, or NULL if not found
**************************************************************************/

SfxSampleType *sfxFindSampleInBank(char *sampleName, SfxBankType *bank);


/**************************************************************************
	FUNCTION:	sfxFindSampleInAllBanks()
	PURPOSE:	Find a sample in all loaded banks
	PARAMETERS:	sample name
	RETURNS:	pointer to sample, or NULL if not found
**************************************************************************/

SfxSampleType *sfxFindSampleInAllBanks(char *sampleName);


/**************************************************************************
	FUNCTION:	sfxDownloadSample
	PURPOSE:	Download sample to SPU ram
	PARAMETERS:	pointer to sample
	RETURNS:	pointer to sample, or NULL if failed
**************************************************************************/

SfxSampleType *sfxDownloadSample(SfxSampleType *sample);


/**************************************************************************
	FUNCTION:	sfxUnloadSample
	PURPOSE:	Unload sample from SPU ram
	PARAMETERS:	pointer to sample
	RETURNS:	pointer to sample, or NULL if failed
**************************************************************************/

SfxSampleType *sfxUnloadSample(SfxSampleType *sample);


/**************************************************************************
	FUNCTION:	sfxUnloadSampleBank
	PURPOSE:	Unload sample bank from SPU ram
	PARAMETERS:	pointer to sample bank
	RETURNS:	pointer to sample bank, or NULL if failed
**************************************************************************/

SfxBankType *sfxUnloadSampleBank(SfxBankType *bank);


/**************************************************************************
	FUNCTION:	sfxDestroySampleBank
	PURPOSE:	Destroy ram copy of sample data
	PARAMETERS:	pointer to sample bank
	RETURNS:	pointer to samplebank, or NULL if failed
**************************************************************************/

SfxBankType *sfxDestroySampleBank(SfxBankType *bank);


/**************************************************************************
	FUNCTION:	sfxRemoveSampleBank
	PURPOSE:	Remove sample bank from list
	PARAMETERS:	pointer to sample bank
	RETURNS:	1 if removed, 0 if not found in list
**************************************************************************/

int sfxRemoveSampleBank(SfxBankType *bank);


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
	FUNCTION:	sfxSetGlobalVolume()
	PURPOSE:	Set global sample volume
	PARAMETERS:	volume
	RETURNS:	none
**************************************************************************/

void sfxSetGlobalVolume(int vol);


/**************************************************************************
	FUNCTION:	sfxSetReverb()
	PURPOSE:	Set reverb level
	PARAMETERS:	delay (0 - 127), depth (0 - 255).
	RETURNS:	1 if successful, 0 if failure
	NOTE:		delay is only effective on SFX_REVERB_MODE_ECHO & SFX_REVERB_MODE_DELAY
**************************************************************************/

int sfxSetReverb(int delay, int depth);


/**************************************************************************
	FUNCTION:	sfxPlaySample()
	PURPOSE:	Plays the requested sample
	PARAMETERS:	pointer to sample, left & right volume (0 - 255), pitch (Hz, 0 for default)
	RETURNS:	channel number, or -1 if failure
**************************************************************************/

int sfxPlaySample(SfxSampleType *sample, int volL, int volR, int pitch);


/**************************************************************************
	FUNCTION:	sfxStopSample()
	PURPOSE:	Stop the requested sample
	PARAMETERS:	pointer to sample, channel number (0 - 23) or -1 for all
	RETURNS:	1 if ok, 0 if failure
**************************************************************************/

int sfxStopSample(SfxSampleType *sample, int channel);


/**************************************************************************
	FUNCTION:	sfxStopChannel
	PURPOSE:	Stop a channel from playing
	PARAMETERS:	channel number (0 - 23), or -1 for all
	RETURNS:	none
**************************************************************************/

void sfxStopChannel(int channel);


/**************************************************************************
	FUNCTION:	sfxGetSampleStatus
	PURPOSE:	get the status of a sample
	PARAMETERS:	pointer to sample
	RETURNS:	Bitfield of channels currently playing sample
**************************************************************************/

unsigned long sfxGetSampleStatus(SfxSampleType *sample);


/**************************************************************************
	FUNCTION:	sfxGetChannelStatus
	PURPOSE:	get the status of a channel
	PARAMETERS:	channel number (0 - 23)
	RETURNS:	Currently playing sample, or NULL if no sample playing
**************************************************************************/

SfxSampleType *sfxGetChannelStatus(int channel);


/**************************************************************************
	FUNCTION:	sfxSetChannelReverb
	PURPOSE:	Turn reverb on/off for a channel
	PARAMETERS:	channel number (0 - 23), status (1 = on, 0 = off)
	RETURNS:	1 if OK, 0 if failure
**************************************************************************/

int sfxSetChannelReverb(int channel, int status);


/**************************************************************************
	FUNCTION:	sfxGetChannelReverb
	PURPOSE:	Get reverb status for a channel
	PARAMETERS:	channel number (0 - 23)
	RETURNS:	1 if on, 0 if off, -1 if failure
**************************************************************************/

int sfxGetChannelReverb(int channel);


/**************************************************************************
	FUNCTION:	sfxSetChannelPitch()
	PURPOSE:	set pitch of channel
	PARAMETERS:	channel number, pitch (Hz)
	RETURNS:	none
**************************************************************************/

void sfxSetChannelPitch(int channel, int pitch);


/**************************************************************************
	FUNCTION:	sfxSetChannelVolume()
	PURPOSE:	set volume of channel
	PARAMETERS:	channel number, left vol, right vol (0 - 255)
	RETURNS:	none
**************************************************************************/

void sfxSetChannelVolume(int channel, int volL, int volR);


/**************************************************************************
	FUNCTION:	sfxSetSampleVolume()
	PURPOSE:	set volume of requested sample
	PARAMETERS:	pointer to sample, channel number (0 - 23) or -1 for all, left and right volumes (0 - 255)
	RETURNS:	1 if ok, 0 if failure
**************************************************************************/

int sfxSetSampleVolume(SfxSampleType *sample, int channel, int volL, int volR);


/**************************************************************************
	FUNCTION:	sfxSetSamplePitch()
	PURPOSE:	set volume of requested sample
	PARAMETERS:	pointer to sample, channel number (0 - 23) or -1 for all, pitch (Hz)
	RETURNS:	1 if ok, 0 if failure
**************************************************************************/

int sfxSetSamplePitch(SfxSampleType *sample, int channel, int pitch);


/**************************************************************************
	FUNCTION:	sfxGetFreeSoundMemory
	PURPOSE:	Print the amount of free SPU ram
	PARAMETERS:	
	RETURNS:	free ram in bytes
**************************************************************************/

int sfxGetFreeSoundMemory();


#endif //__ISLSFX2_H__