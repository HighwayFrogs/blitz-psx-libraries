#include <stddef.h>
#include <stdio.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include "islex.h"

/*
enum {
OFS_AT, OFS_V0, OFS_V1, OFS_A0, OFS_A1, OFS_A2, OFS_A3, OFS_T0, OFS_T1, OFS_T2, OFS_T3,
OFS_T4, OFS_T5, OFS_T6, OFS_T7, OFS_S0, OFS_S1, OFS_S2, OFS_S3, OFS_S4, OFS_S5, OFS_S6,
OFS_S7, OFS_T8, OFS_T9, OFS_GP, OFS_SP, OFS_FP, OFS_RA, OFS_HI, OFS_LO, OFS_SR, OFS_CA,
OFS_EPC,
};
*/

enum {
	OFS_RA,
	OFS_CA,
	OFS_EPC,
};

typedef struct {
	DRAWENV	draw;
	DISPENV	disp;
} EXC_DB;

static void exc_swap(void);

static EXC_DB exc_db[2];
static int exc_id;

static char *exc_txt[]={
	"external interrupt",
	"tlb modification exception",
	"tlb miss (load or fetch)",
	"tlb miss (store)",
	"address error (load or fetch)",
	"address error (store)",
	"bus error (fetch)",
	"bus error (load or store)",
	"syscall",
	"break",
	"reserved instruction",
	"coprocessor unusable",
	"arithmetic overflow",
	"unknown exception",
	"unknown exception",
	"unknown exception"};

static char *break_txt[]={
	"6 (overflow)",
	"7 (div by zero)"};

static char str_reg[1200];
static char *p_sr;

/* --------------------------------- exc_c ---------------------------------- */
void utilHandleException(void) {

	int i,*p_exc;

	ResetCallback();
	ResetGraph(0);
	SetGraphDebug(0);
	SetVideoMode(MODE_NTSC);
	SetDispMask(1);

	FntLoad(640,0);
	SetDumpFnt(FntOpen(8,8,320-8,240-8,0,1024));

	SetDefDispEnv(&exc_db[0].disp,0,0,320,240);
	SetDefDrawEnv(&exc_db[0].draw,320,0,320,240);
	SetDefDispEnv(&exc_db[1].disp,320,0,320,240);
	SetDefDrawEnv(&exc_db[1].draw,0,0,320,240);

	exc_db[0].draw.isbg=exc_db[1].draw.isbg=1;
	exc_db[0].disp.screen.x=exc_db[1].disp.screen.x=0;
	exc_db[0].disp.screen.y=exc_db[1].disp.screen.y=12;
	setRGB0(&exc_db[0].draw,0,0,64);
	setRGB0(&exc_db[1].draw,0,0,64);

/*---- cause ----*/
	p_sr=str_reg;

	p_exc=(int *)reg_lst[OFS_EPC];

	p_sr+=sprintf(p_sr," %s",exc_txt[reg_lst[OFS_CA]>>2&0x1f]);

	if((reg_lst[OFS_CA]>>2&0x1f)==9) {
		i=(*p_exc>>16)-6;
		if(i==0||i==1)
			p_sr+=sprintf(p_sr," %s",break_txt[i]);
	}

	p_sr+=sprintf(p_sr,"\n at %08x",(int)p_exc);

	if((reg_lst[OFS_CA]&0x80000000)==0x80000000)
		p_sr+=sprintf(p_sr," in branch delay slot\n\n");
	else
		p_sr+=sprintf(p_sr,"\n\n");

	p_sr += sprintf(p_sr," called from %08x\n\n\n",reg_lst[OFS_RA]);

	
/*---- reg-dump ----*/
/*
	p_sr+=sprintf(p_sr," at=%08x t4=%08x s7=%08x\n",
		reg_lst[OFS_AT],reg_lst[OFS_T4],reg_lst[OFS_S7]);
	p_sr+=sprintf(p_sr," v0=%08x t5=%08x t8=%08x\n",
		reg_lst[OFS_V0],reg_lst[OFS_T5],reg_lst[OFS_T8]);
	p_sr+=sprintf(p_sr," v1=%08x t6=%08x t9=%08x\n",
		reg_lst[OFS_V1],reg_lst[OFS_T6],reg_lst[OFS_T9]);
	p_sr+=sprintf(p_sr," a0=%08x t7=%08x gp=%08x\n",
		reg_lst[OFS_A0],reg_lst[OFS_T7],reg_lst[OFS_GP]);
	p_sr+=sprintf(p_sr," a1=%08x s0=%08x sp=%08x\n",
		reg_lst[OFS_A1],reg_lst[OFS_S0],reg_lst[OFS_SP]);
	p_sr+=sprintf(p_sr," a2=%08x s1=%08x fp=%08x\n",
		reg_lst[OFS_A2],reg_lst[OFS_S1],reg_lst[OFS_FP]);
	p_sr+=sprintf(p_sr," a3=%08x s2=%08x ra=%08x\n",
		reg_lst[OFS_A3],reg_lst[OFS_S2],reg_lst[OFS_RA]);
	p_sr+=sprintf(p_sr," t0=%08x s3=%08x hi=%08x\n",
		reg_lst[OFS_T0],reg_lst[OFS_S3],reg_lst[OFS_HI]);
	p_sr+=sprintf(p_sr," t1=%08x s4=%08x lo=%08x\n",
		reg_lst[OFS_T1],reg_lst[OFS_S4],reg_lst[OFS_LO]);
	p_sr+=sprintf(p_sr," t2=%08x s5=%08x sr=%08x\n",
		reg_lst[OFS_T2],reg_lst[OFS_S5],reg_lst[OFS_SR]);
	p_sr+=sprintf(p_sr," t3=%08x s6=%08x ca=%08x\n",
		reg_lst[OFS_T3],reg_lst[OFS_S6],reg_lst[OFS_CA]);
*/
/*---- mem-dump ----*/

#define NR_DUMP		9

	exc_id=1;

	//p_sp=(int *)reg_lst[OFS_SP];
	//cur_sp=p_sp+(NR_DUMP-1);
	//cur_exc=p_exc+(-NR_DUMP/2);

	while(1) {

		DrawSync(0);	
		VSync(0);
		exc_id=exc_id? 0: 1;
		exc_swap();

		FntPrint("\nisl exception handler build %s\n\n",__DATE__);
		FntPrint("\ncrash type:-\n");
		FntPrint(str_reg);
        
		FntFlush(-1);
	}
}

/* ------------------------------- exc_swap ------------------------------ */
static void exc_swap(void) {

	PutDrawEnv(&exc_db[exc_id].draw);
	PutDispEnv(&exc_db[exc_id].disp);
}

