/******************************************************************************************
	AM2 PS   (c) 1999-2001 ISL

	main.c:			Top level shell
******************************************************************************************/

#include "global.h"
#include "main.h"
#include "utils.h"
#include "gamesave.h"
#include "xa.h"
#include "texture.h"
#include "psfont.h"
#include "sfx.h"

extern char __bss_orgend[];



UBYTE	*version = VERSIONSTR;
UBYTE	quitMainLoop;
UBYTE	hardReset = 1;
ULONG	RAMstart, RAMsize;
ULONG	frame;

UBYTE	memoryCardCheck;

static RECT VRAMarea = {0,0,1024,512};


displayPageType displayPage[2], *currentDisplayPage;


static POLY_G4 *pp;
psFont *font;



/**************************************************************************
	FUNCTION:	vsyncCallback()
	PURPOSE:	Vsync interrupt callback
	PARAMETERS:	
	RETURNS:	
**************************************************************************/

static void vsyncCallback()
{
	frame++;
#if GOLDCD==0
	asm("break 1024");
#endif
}


/**************************************************************************
	FUNCTION:	videoInit()
	PURPOSE:	Initialise video mode/GPU
	PARAMETERS:	Order table depth, max number of GT4 primitives needed or 0
	RETURNS:	
**************************************************************************/

void videoInit(int otDepth, int maxPrims)
{
	if (maxPrims>0)
	{
		displayPage[0].ot = MALLOC(otDepth*4);
		displayPage[1].ot = MALLOC(otDepth*4);
		displayPage[0].primPtr = displayPage[0].primBuffer = MALLOC(maxPrims*sizeof(POLY_GT4));
		displayPage[1].primPtr = displayPage[1].primBuffer = MALLOC(maxPrims*sizeof(POLY_GT4));
	}

	VSync(0);
	ResetGraph(1);
	VSync(0);
#if PALMODE==1
	SetVideoMode(MODE_PAL);
	GsInitGraph(512,273, 4,1,0);
	GsInit3D();
	SetGeomOffset(512/2, 256/2);
#else
	SetVideoMode(MODE_NTSC);
	GsInitGraph(512,240, 4,1,0);
	GsInit3D();
	SetGeomOffset(512/2, 240/2);
#endif
	SetDefDrawEnv(&displayPage[0].drawenv, 0,   0, 512,256);
	SetDefDrawEnv(&displayPage[1].drawenv, 0, 256, 512,256);
	SetDefDispEnv(&displayPage[0].dispenv, 0, 256, 512,256);
	SetDefDispEnv(&displayPage[1].dispenv, 0,   0, 512,256);
	displayPage[0].dispenv.screen.y = displayPage[1].dispenv.screen.y = 24;
  	displayPage[0].drawenv.ofs[0] = displayPage[1].drawenv.ofs[0] = 512/2;
  	displayPage[0].drawenv.ofs[1] = 120+PALMODE*8;
	displayPage[1].drawenv.ofs[1] = 256+120+PALMODE*8;
	displayPage[0].drawenv.isbg = displayPage[1].drawenv.isbg = 1;
	GsSetProjection(950);
	SetDispMask(1);
	VSync(0);
}


void silly()
{
	int			loop, x,y;
	static int	xx = 0, yy = 0;

	pp = (POLY_G4 *)currentDisplayPage->primPtr;
	setPolyG4(pp);
	setXY4(pp, -50,-50, 50,-50, -50,50, 50,50);
	setRGB0(pp, 255,0,0);
	setRGB1(pp, 0,255,0);
	setRGB2(pp, 0,0,255);
	setRGB3(pp, 255,255,255);
	addPrim(currentDisplayPage->ot+512, pp);
	currentDisplayPage->primPtr += sizeof(POLY_G4);

	for(loop=0; loop<250; loop++)
	{
		x = -280+(loop%25)*20+xx;
		y = -120+(loop/25)*20+yy;
		pp = (POLY_G4 *)currentDisplayPage->primPtr;
		setPolyG4(pp);
		setXY4(pp, -8+x,-8+y, 8+x,-8+y, -8+x,8+y, 8+x,8+y);
		setRGB0(pp, 255,0,0);
		setRGB1(pp, 0,255,0);
		setRGB2(pp, 0,0,255);
		setRGB3(pp, 255,255,255);
		addPrim(currentDisplayPage->ot+700, pp);
		currentDisplayPage->primPtr += sizeof(POLY_G4);
	}

	if (padData.digital[0] & PAD_LEFT)
		xx--;
	if (padData.digital[0] & PAD_RIGHT)
		xx++;
	if (padData.digital[0] & PAD_UP)
		yy--;
	if (padData.digital[0] & PAD_DOWN)
		yy++;

	fontPrint(font, 0,0, "Hello", 128,128,128);
}



/**************************************************************************
	FUNCTION:	main()
	PURPOSE:	Top level shell function
	PARAMETERS:	
	RETURNS:	0
**************************************************************************/

int main()
{ 
// NO LOCAL ALLOWED
	utilInstallException();
	while(1)
	{
		memset((void *)0x1f8000,0,0x8000);
		debugPrintf("\n\nAM2 %s\n\n",version);
		initCRC();
		seedRandomInt(398623);
		ResetCallback();
		RAMstart = (ULONG)__bss_orgend;
		RAMsize = (0x1fff00 - RAMstart)-8192;
		debugPrintf("\nRAM start 0x%x  0x%x (%d)\n", RAMstart, RAMsize, RAMsize);
		memoryInitialise(RAMstart, RAMsize);
		fileInitialise();
#if GOLDCD==0
		XAenable = CdInit();
#else
		XAenable = 1;
#endif
		MemCardInit(1);
		MemCardStart();
		padInitialise();

		videoInit(1024, 3000);
		textureInitVRAM();

		sfxInitialise();
//		sfxStartSound();

		memoryCardCheck = 1;
		gameSaveInitialise();

		gameTextInit(gameTextLang);
//		gameInitialise();

		frame = 0;
		VSyncCallback(&vsyncCallback);


		sfxLoadBank("AM");
		font = fontLoad("FONT12.FON");
		textureDownloadBank(textureLoadBank("ACTMAN_D.SPT"));
		memoryShow();

		quitMainLoop = 0;
		while(!quitMainLoop)
		{
			TIMER_START(TIMER_TOTAL);
			currentDisplayPage = (currentDisplayPage==displayPage)?(&displayPage[1]):(&displayPage[0]);
			ClearOTagR(currentDisplayPage->ot, 1024);
			currentDisplayPage->primPtr = currentDisplayPage->primBuffer;
			timerDisplay();

			if (padData.debounce[4] & PAD_TRIANGLE)
			{
				sfxQueueSound(1,0x2000,2000);
			}
			
			soundFrame();
			silly();

//			gameFrame();
#if GOLDCD==0
			if (padData.debounce[4] & PAD_R1)
				timerActive = !timerActive;
			if (padData.debounce[4] & PAD_L1)
			{
				textureShowVRAM();
				continue;
			}
#endif
//			debugPrintf("Frame %d\n", frame);
			padHandler();
			if (padData.digital[0] == (PAD_SELECT|PAD_START|PAD_L1|PAD_R1|PAD_L2|PAD_R2))
				quitMainLoop = 1;
			TIMER_START(TIMER_DRAWSYNC);
			DrawSync(0);
			TIMER_STOP(TIMER_DRAWSYNC);
			TIMER_STOP(TIMER_TOTAL);
			VSync(2);
			PutDispEnv(&currentDisplayPage->dispenv);
			PutDrawEnv(&currentDisplayPage->drawenv);
			DrawOTag(currentDisplayPage->ot+(1024-1));
//			DumpOTag(currentDisplayPage->ot+1024-1);
			TIMER_ENDFRAME;
		}
		debugPrintf("\nAM2 QUIT/RESET\n");
		DrawSync(0);
		ClearImage2(&VRAMarea, 0,0,0);
//		SpuClearReverbWorkArea(SPU_OFF);
		StopCallback();
		PadStopCom();
		ResetGraph(3);
		VSync(10);
	}
	return 0;
}
