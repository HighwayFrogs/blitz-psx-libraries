/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islsound2.h:		Sound fx handling 2.0

************************************************************************************/


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




