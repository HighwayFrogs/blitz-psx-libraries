/******************************************************************************************
	AM2 PS   (c) 1999-2001 ISL

	psfont.c:			Font support
******************************************************************************************/

#include "..\shell\shell.h"
#include "..\isltex\isltex.h"
#include "..\islmem\islmem.h"
#include "..\islfile\islfile.h"
#include "islfont.h"


// Richard's funky macros for calculating vram related stuff

#define VRAM_PAGECOLS		8
#define VRAM_PAGEROWS		2
#define VRAM_PAGES			16
#define VRAM_PAGEW			32
#define VRAM_PAGEH			32
#define VRAM_SETX(X)		(X)
#define VRAM_SETY(Y)		((Y)*VRAM_PAGEW)
#define VRAM_SETXY(X,Y)		((X)+((Y)*VRAM_PAGEW))
#define VRAM_SETPAGE(P)		((P)<<16)
#define VRAM_GETX(HND)		((HND) & (VRAM_PAGEW-1))
#define VRAM_GETY(HND)		(((HND)/VRAM_PAGEW) & (VRAM_PAGEW-1))
#define VRAM_GETXY(HND)		((HND) & 0xffff)
#define VRAM_GETPAGE(HND)	((HND)>>16)
#define VRAM_CALCVRAMX(HND)	(512+((VRAM_GETPAGE(HND)%(VRAM_PAGECOLS))*64)+(VRAM_GETX(HND)*2))
#define VRAM_CALCVRAMY(HND)	(((VRAM_GETPAGE(HND)/(VRAM_PAGECOLS))*256)+(VRAM_GETY(HND)*8))


unsigned char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=+[]{}:;\"'|\\,<.>/?"
					"\xe0\xe8\xec\xf2\xf9\xc0\xc8\xcc\xd2\xd9\xe1\xe9\xed\xf3\xfa\xfd\xc1\xc9\xcd\xd3\xda\xdd\xe2\xea\xee\xf4\xfb\xc2\xca\xce\xd4\xdb\xe3\xf1\xf5\xc3\xd1\xd5\xe4\xeb\xef\xf6\xfc\xff\xc4\xcb\xcf\xd6\xdc\xe5\xc5\xe6\xc6\xe7\xc7\xf0\xd0\xf8\xd8\xbf\xa1\xdf";



static void fontDownload(psFont *font, char *fontdata, int character)
{
	int			    w,h, handle;
	RECT			rect;
	unsigned char	*offset;
	TextureType		*txPtr = &font->txPtr[character];
	
	w = (font->width[character]+7)/8;
	h = (font->height+7)/8;
	handle = textureVRAMalloc(w,h);
	rect.x = VRAM_CALCVRAMX(handle);
	rect.y = VRAM_CALCVRAMY(handle);
	rect.w = (font->width[character]+3)/4;
	rect.h = font->height;

	offset = fontdata + font->dataoffset[character];
	DrawSync(0);
	LoadImage(&rect,(unsigned long *)offset);
	DrawSync(0);

	txPtr->tpage = getTPage(0,0,rect.x,rect.y);
	txPtr->x = VRAM_GETX(handle)*8;
	txPtr->y = rect.y;
	txPtr->w = font->width[character];
	txPtr->h = font->height;
   	txPtr->u0 = txPtr->x;
   	txPtr->v0 = txPtr->y;
   	txPtr->u1 = txPtr->x+font->width[character]-1;
   	txPtr->v1 = txPtr->y;
   	txPtr->u2 = txPtr->x;
   	txPtr->v2 = txPtr->y+font->height-1;
   	txPtr->u3 = txPtr->x+font->width[character]-1;
   	txPtr->v3 = txPtr->y+font->height-1;
	txPtr->clut = font->clut;
	txPtr->handle = handle;
}


/**************************************************************************
	FUNCTION:	fontLoad()
	PURPOSE:	Load font into VRAM ready for use
	PARAMETERS:	Filename
	RETURNS:	Ptr to font info
**************************************************************************/

psFont *fontLoad(char *fontname)
{
	int				loop;
	unsigned char	*fontdata;
	int				*fontptr;
	psFont			*font;
//	unsigned char			*str;

	font = (psFont *)MALLOC(sizeof(psFont));
	if ((fontdata = fileLoad(fontname, NULL))==NULL)
		return NULL;

	fontptr = (int *)fontdata;
	font->numchars = *fontptr++;
	font->height = *fontptr++;
	font->clut = textureAddCLUT16((unsigned short *)fontptr);

	fontptr = (int *)fontdata + 10;

	font->txPtr = (TextureType *)MALLOC(sizeof(TextureType)*font->numchars);
	memset(font->charlookup, -1, 256);
/*	str = "‡ËÏÚ˘¿»Ã“Ÿ·ÈÌÛ˙˝¡…Õ”⁄›‚ÍÓÙ˚¬ Œ‘€„Òı√—’‰ÎÔˆ¸ˇƒÀœ÷‹Â≈Ê∆Á«–¯ÿø°ﬂ";
	for(loop=0; loop<strlen(str); loop++)
	{
		debugPrintf("\\x%02x\n", str[loop]);
	}
	debugPrintf("FONT: ");*/
	for(loop = 0; loop < font->numchars; loop++)
	{
		font->dataoffset[loop] = *fontptr++;
		font->width[loop] = *fontptr++;
		font->pixelwidth[loop] = *fontptr++;
		fontDownload(font, fontdata, loop);
		font->charlookup[alphabet[loop]] = loop;
//		debugPrintf("%c ", alphabet[loop]);
	}
//	debugPrintf("\n");
	FREE(fontdata);

	font->alpha = 0;

	return font;
}


/**************************************************************************
	FUNCTION:	fontUnload()
	PURPOSE:	Unload font from VRAM and free info
	PARAMETERS:	Font ptr
	RETURNS:	
**************************************************************************/

void fontUnload(psFont *font)
{
	int loop;

	for(loop=0; loop<font->numchars; loop++)
		textureUnload(&font->txPtr[loop]);
	FREE(font->txPtr);
	font->txPtr = NULL;
	FREE(font);
}


static void fontDispChar(TextureType *tex, short x,short y, unsigned char r, unsigned char g, unsigned char b, unsigned char alpha)
{
	POLY_FT4 	*ft4; 

	BEGINPRIM(ft4, POLY_FT4);
	setPolyFT4(ft4);
	setXYWH(ft4, x,y, tex->w,tex->h-1);
	setRGB0(ft4, r,g,b);
	setUVWH(ft4, tex->x,tex->y, tex->w-1, tex->h-1);
	setSemiTrans(ft4, alpha);
	ft4->tpage = tex->tpage;
	if(alpha)
		SETSEMIPRIM(ft4, alpha);
	ft4->clut = tex->clut;
	ENDPRIM(ft4, 0, POLY_FT4);
}


/**************************************************************************
	FUNCTION:	fontPrint()
	PURPOSE:	Display string in font at pos in colour
	PARAMETERS:	font ptr, x,y, string, r,g,b
	RETURNS:	
**************************************************************************/

void fontPrint(psFont *font, short x,short y, char *text, unsigned char r, unsigned char g, unsigned char b)
{
	unsigned char	*strPtr, c;
	int				cx,cy, loop;
	TextureType		*letter;

	strPtr = text;
	cx = x;
	cy = y;
	while(*strPtr)
	{
		c = *strPtr;
		switch(c)
		{
		case '\n':
			y += font->height;
			cx = x;
			cy = y;
			break;
		case ' ':
			x += font->height/2;
			break;
		default:
			loop = font->charlookup[c];
			if (loop<font->numchars)
			{
				letter = &font->txPtr[loop];
				if ((x>-350)&&(x<320))
					fontDispChar(letter, x,y-((c>127)?(1):(0)), r,g,b, font->alpha);
				x += font->pixelwidth[loop]+2;
			}
		}
		strPtr++;
	}
}


/**************************************************************************
	FUNCTION:	fontExtentW()
	PURPOSE:	Get width of string using font
	PARAMETERS:	font ptr, string
	RETURNS:	Width in pixels
**************************************************************************/

int fontExtentW(psFont *font, char *text)
{
	unsigned char	*strPtr, c;
	int		loop, x, maxW = 0;

	strPtr = text;
	x = 0;
	while(*strPtr)
	{
		c = *strPtr;
		switch(c)
		{
		case '\n':
			if (x>maxW)
				maxW = x;
			x = 0;
			break;
		case ' ':
			x += font->height/2;
			break;
		default:
			loop = font->charlookup[c];
			if (loop<font->numchars)
				x += font->pixelwidth[loop]+2;
		}
		strPtr++;
	}
	if (x>maxW)
		maxW = x;
	return maxW;
}


/**************************************************************************
	FUNCTION:	fontFitToWidth()
	PURPOSE:	Wrap string to width into buffer
	PARAMETERS:	font ptr, width, string, buffer
	RETURNS:	Number of lines
**************************************************************************/

int fontFitToWidth(psFont *font, int width, char *text, char *buffer)
{
	char	buf[256], *bufPtr, *outPtr = buffer;
	int		pixLen, lines = 0;

	memset(buf, 0, sizeof(buf));
	bufPtr = buf;
	while(1)
	{
		if (*text>=' ')
			*bufPtr++ = *text;
		pixLen = fontExtentW(font, buf);
		if ((pixLen>width)||(*text=='\0'))
		{
			if (pixLen>width)
			{
				while(*bufPtr!=' ')
				{
					bufPtr--;
					text--;
				}
				text++;
			}
			*bufPtr = '\0';
			strcpy(outPtr, buf);
			outPtr += strlen(buf)+1;
			lines++;
			bufPtr = buf;
			memset(buf, 0, sizeof(buf));
			if (*text=='\0')
				break;
		}
		text++;
	}
/*	debugPrintf("%d lines: \n", lines);
	outPtr = buffer;
	for(pixLen=0; pixLen<lines; pixLen++)
	{
		debugPrintf("  '%s'\n", outPtr);
		outPtr += strlen(outPtr)+1;
	}*/
	return lines;
}



/**************************************************************************
	FUNCTION:	fontPrintN()
	PURPOSE:	Display (partial) string in font at pos in colour
	PARAMETERS:	font ptr, x,y, string, r,g,b, max chars
	RETURNS:	
**************************************************************************/

void fontPrintN(psFont *font, short x,short y, char *text, unsigned char r, unsigned char g, unsigned char b, int n)
{
	unsigned char		*strPtr, c;
	int			cx,cy, loop;
	TextureType	*letter;

	strPtr = text;
	cx = x;
	cy = y;
	while ((*strPtr) && (n>0))
	{
		c = *strPtr;
		switch(c)
		{
		case '\n':
			y += font->height;
			cx = x;
			cy = y;
			break;
		case ' ':
			x += font->height/2;
			break;
		case '@':
/*			switch(*(strPtr+1))
			{
			case 'X':
			   	mapScreenDispSquare(gameInfo.buttons[0], x+16,y+6, 128,128,128, 0, 0, 4096,-1,0);
				strPtr++;
				x += 32;
				break;
			case 'C':
			   	mapScreenDispSquare(gameInfo.buttons[1], x+16,y+6, 128,128,128, 0, 0, 4096,-1,0);
				strPtr++;
				x += 32;
				break;
			case 'S':
			   	mapScreenDispSquare(gameInfo.buttons[2], x+16,y+6, 128,128,128, 0, 0, 4096,-1,0);
				strPtr++;
				x += 32;
				break;
			case 'T':
			   	mapScreenDispSquare(gameInfo.buttons[3], x+16,y+6, 128,128,128, 0, 0, 4096,-1,0);
				strPtr++;
				x += 32;
				break;
			}*/
			break;
		default:
			loop = font->charlookup[c];
			if (loop<font->numchars)
			{
				letter = &font->txPtr[loop];
				if ((x>-350)&&(x<320))
					fontDispChar(letter, x,y-((c>127)?(1):(0)), r,g,b,font->alpha);
				x += font->pixelwidth[loop]+2;
			}
		}
		strPtr++;
		n--;
	}
}


