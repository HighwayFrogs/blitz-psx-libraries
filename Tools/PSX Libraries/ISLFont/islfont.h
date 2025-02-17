/************************************************************************************
	ISL PSX LIBRARY	(c) 1999 Interactive Studios Ltd.

	islfont.h:		Font support

************************************************************************************/

#ifndef __ISLFONT_H__
#define __ISLFONT_H__

typedef struct _psFont
{
	int				numchars;
	int				height;
	int				dataoffset[256];
	int				width[256];
	int				pixelwidth[256];
	TextureType		*txPtr;
	unsigned short	clut;
	unsigned char	charlookup[256];
	unsigned char	alpha;
	int				spriteYOffset;
	
} psFont;


/**************************************************************************
	FUNCTION:	fontLoad()
	PURPOSE:	Load font into VRAM ready for use
	PARAMETERS:	Filename
	RETURNS:	Ptr to font info
**************************************************************************/

psFont *fontLoad(char *fontname);


/**************************************************************************
	FUNCTION:	fontUnload()
	PURPOSE:	Unload font from VRAM and free info
	PARAMETERS:	Font ptr
	RETURNS:	
**************************************************************************/

void fontUnload(psFont *font);


/**************************************************************************
	FUNCTION:	fontPrint()
	PURPOSE:	Display string in font at pos in colour
	PARAMETERS:	font ptr, x,y, string, r,g,b
	RETURNS:	
**************************************************************************/

void fontPrint(psFont *font, short x,short y, char *text, unsigned char r, unsigned char g, unsigned char b);


/**************************************************************************
	FUNCTION:	fontExtentW()
	PURPOSE:	Get width of string using font
	PARAMETERS:	font ptr, string
	RETURNS:	Width in pixels
**************************************************************************/

int fontExtentW(psFont *font, char *text);


/**************************************************************************
	FUNCTION:	fontFitToWidth()
	PURPOSE:	Wrap string to width into buffer
	PARAMETERS:	font ptr, width, string, buffer
	RETURNS:	Number of lines
**************************************************************************/

int fontFitToWidth(psFont *font, int width, char *text, char *buffer);


/**************************************************************************
	FUNCTION:	fontPrintN()
	PURPOSE:	Display (partial) string in font at pos in colour
	PARAMETERS:	font ptr, x,y, string, r,g,b, max chars
	RETURNS:	
**************************************************************************/

void fontPrintN(psFont *font, short x,short y, char *text, unsigned char r, unsigned char g, unsigned char b, int n);


/**************************************************************************
	FUNCTION:	fontRegisterButtonSprites()
	PURPOSE:	Set sprites to be used for button display
	PARAMETERS:	pointers to sprites
	RETURNS:	
**************************************************************************/

void fontRegisterButtonSprites(TextureType *triangle, TextureType *circle, TextureType *cross, TextureType *square);


/**************************************************************************
	FUNCTION:	fontRegisterOtherSprites()
	PURPOSE:	Set sprites to be used for display
	PARAMETERS:	character code ,pointers to sprite
	RETURNS:	
**************************************************************************/

void fontRegisterOtherSprites(char code, TextureType *sprite);


/**************************************************************************
	FUNCTION:	fontSetPrintDepth()
	PURPOSE:	Set depth for font printing
	PARAMETERS:	depth
	RETURNS:	
**************************************************************************/

void fontSetPrintDepth(int depth);


#endif //__ISLFONT_H__
