/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islsfx2.h:		Sound fx handling 2.0

************************************************************************************/

#ifndef __ISLSFX2_H__
#define __ISLSFX2_H__

typedef struct _SfxSampleType {

	unsigned char	inSPURam;		// is this sample in SPU ram?
	unsigned long	spuOffset;		// offset in SPU ram
	unsigned long	nameCRC;		// CRC of sample name
	unsigned long	sampleSize;		// length of sample data
	unsigned char	*sampleData;	// pointer to sample data in RAM

} SfxSampleType;


typedef struct _SfxBankType {

	int				numSamples;		// number of samples in this bank
	SfxSampleType	*sample;		// array of SfxSampleType's
	unsigned long	baseAddr;		// if the bank was downloaded entirely, the base address
	unsigned char	*sampleData;

} SfxBankType;




/**************************************************************************
	FUNCTION:	sfxInitialise()
	PURPOSE:	Initialise the sound library
	PARAMETERS:	maximum reverb setting
	RETURNS:	none
**************************************************************************/

void sfxInitialise(int maxReverb);


/**************************************************************************
	FUNCTION:	sfxDestroy()
	PURPOSE:	Shutdown the sound library
	PARAMETERS:	none
	RETURNS:	none
**************************************************************************/

void sfxDestroy();


/**************************************************************************
	FUNCTION:	sfxFixupSampleBankHeader()
	PURPOSE:	fix up sample bank header
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


#endif //__ISLSFX2_H__