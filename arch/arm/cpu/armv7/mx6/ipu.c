/***************************************************************************
*
*    ipu.c
*
*    IPU initialization, connect each module and build a link from memory to  
* display .
*
*
***************************************************************************/

#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/errno.h>
#include <linux/string.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/ipuv3h_reg_def.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_ipu.h>


//#define DISPLAY_EBA0 		(PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE - 0x2000000)
#define DISPLAY_EBA0		CONFIG_FB_BASE

// DI counter definitions
#define DI_COUNTER_BASECLK 	0
#define DI_COUNTER_IHSYNC   	1
#define DI_COUNTER_1		   	1
#define DI_COUNTER_HSYNC   		2
#define DI_COUNTER_VSYNC   		3
#define DI_COUNTER_AFIELD    	4
#define DI_COUNTER_ALINE    		5
#define DI_COUNTER_APIXEL   		6
#define DI_COUNTER_7   			7

#define DI_COMMAND_WAVEFORM 0
#define DI_DATAWR_WAVEFORM  1
#define DI_DATARD_WAVEFORM  2
#define DI_SDC_WAVEFORM     3
#define DI_SERIAL_WAVEFORM  4

#define DI_RS_SIGNAL        0
#define DI_WR_SIGNAL        1
#define DI_RD_SIGNAL        1
#define DI_SER_CLK_SIGNAL   1
#define DI_CS_SIGNAL        2
#define DI_NOUSE_SIGNAL     3

#define DI_DEN_SIGNAL       0

#define DC_CHANNEL_READ					0
#define DC_CHANNEL_DC_SYNC_OR_ASYNC	1
#define DC_CHANNEL_DC_ASYNC				2
#define DC_CHANNEL_COMMAND_1			3
#define DC_CHANNEL_COMMAND_2			4
#define DC_CHANNEL_DP_PRIMARY			5
#define DC_CHANNEL_DP_SECONDARY			6

typedef struct {
	unsigned int channel;
	unsigned int xv;
	unsigned int yv;
	unsigned int xb;
	unsigned int yb;
	unsigned int nsb_b;
	unsigned int cf;
	unsigned int sx;
	unsigned int sy;
	unsigned int ns;
	unsigned int sdx;
	unsigned int sm;
	unsigned int scc;
	unsigned int sce;
	unsigned int sdy;
	unsigned int sdrx;
	unsigned int sdry;
	unsigned int bpp;
	unsigned int dec_sel;
	unsigned int dim;
	unsigned int so;
	unsigned int bndm;
	unsigned int bm;
	unsigned int rot;
	unsigned int hf;
	unsigned int vf;
	unsigned int the;
	unsigned int cap;
	unsigned int cae;
	unsigned int fw;
	unsigned int fh;
	unsigned int eba0;
	unsigned int eba1;
	unsigned int ilo;
	unsigned int npb;
	unsigned int pfs;
	unsigned int alu;
	unsigned int albm;
	unsigned int id;
	unsigned int th;
	unsigned int sl;
	unsigned int wid0;
	unsigned int wid1;
	unsigned int wid2;
	unsigned int wid3;
	unsigned int ofs0;
	unsigned int ofs1;
	unsigned int ofs2;
	unsigned int ofs3;
	unsigned int ubo;
	unsigned int vbo;
}ipu_channel_parameter_t;


void ipu_write_field(unsigned int IPU, unsigned int ID_addr, unsigned int ID_mask, unsigned int data)
{
	unsigned int rdata; 
	unsigned int IPU_BASE_ADDRE = IPU1_ARB_BASE_ADDR;
	
	if (IPU == 2)
		IPU_BASE_ADDRE = IPU2_ARB_BASE_ADDR;

	ID_addr += IPU_BASE_ADDRE;
	rdata = readl(ID_addr);
	rdata &= ~ID_mask;
	rdata |= (data*(ID_mask & -ID_mask))&ID_mask;
	writel(rdata, ID_addr);
}

void config_idmac_interleaved_channel(unsigned int IPU, ipu_channel_parameter_t ipu_channel_params)
{
	unsigned int IPU_BASE_ADDRE = IPU1_ARB_BASE_ADDR;
	int w0_d0=0, w0_d1=0, w0_d2=0, w0_d3=0, w0_d4=0, w1_d0=0, w1_d1=0, w1_d2=0, w1_d3=0, w1_d4=0;

	if (IPU == 2)
		IPU_BASE_ADDRE = IPU2_ARB_BASE_ADDR;
    
	w0_d0 = ipu_channel_params.xb<<19 | ipu_channel_params.yv<<10  |ipu_channel_params.xv;
	w0_d1 = ipu_channel_params.sy<<26 | ipu_channel_params.sx<<14  | ipu_channel_params.cf<<13  | ipu_channel_params.nsb_b<<12 \
		| ipu_channel_params.yb;
	w0_d2 = ipu_channel_params.sm<<22 | ipu_channel_params.sdx<<15 | ipu_channel_params.ns<<5   | ipu_channel_params.sy>>6;
	w0_d3 = ipu_channel_params.fw<<29 | ipu_channel_params.cae<<28 | ipu_channel_params.cap<<27 | ipu_channel_params.the<<26  \
		| ipu_channel_params.vf<<25 | ipu_channel_params.hf<<24 | ipu_channel_params.rot<<23 | ipu_channel_params.bm<<21 \
		| ipu_channel_params.bndm<<18 | ipu_channel_params.so<<17 | ipu_channel_params.dim<<16 | ipu_channel_params.dec_sel<<14 \
		| ipu_channel_params.bpp<<11 | ipu_channel_params.sdry<<10 | ipu_channel_params.sdrx<<9 | ipu_channel_params.sdy<<2 \
		| ipu_channel_params.sce<<1 | ipu_channel_params.scc;
	w0_d4 = ipu_channel_params.fh<<10 | ipu_channel_params.fw>>3;

	w1_d0 = ipu_channel_params.eba1<<29 | ipu_channel_params.eba0;
	w1_d1 = ipu_channel_params.ilo<<26  | ipu_channel_params.eba1>>3;
	w1_d2 = ipu_channel_params.th<<31   | ipu_channel_params.id<<29   | ipu_channel_params.albm<<26 \
		| ipu_channel_params.alu<<25  | ipu_channel_params.pfs<<21 | ipu_channel_params.npb<<14 | ipu_channel_params.ilo>>6;
	w1_d3 = ipu_channel_params.wid3<<29 | ipu_channel_params.wid2<<26 | ipu_channel_params.wid1<<23 | ipu_channel_params.wid0<<20 \
		| ipu_channel_params.sl<<6   | ipu_channel_params.th>>1;
	w1_d4 = ipu_channel_params.ofs3<<15 | ipu_channel_params.ofs2<<10 | ipu_channel_params.ofs1<<5 | ipu_channel_params.ofs0;

	writel(w0_d0, IPU_BASE_ADDRE + CPMEM_WORD0_DATA0_INT__ADDR + (ipu_channel_params.channel<<6));
	writel(w0_d1, IPU_BASE_ADDRE + CPMEM_WORD0_DATA1_INT__ADDR + (ipu_channel_params.channel<<6));
	writel(w0_d2, IPU_BASE_ADDRE + CPMEM_WORD0_DATA2_INT__ADDR + (ipu_channel_params.channel<<6));
	writel(w0_d3, IPU_BASE_ADDRE + CPMEM_WORD0_DATA3_INT__ADDR + (ipu_channel_params.channel<<6));
	writel(w0_d4, IPU_BASE_ADDRE + CPMEM_WORD0_DATA4_INT__ADDR + (ipu_channel_params.channel<<6));

	writel(w1_d0, IPU_BASE_ADDRE + CPMEM_WORD1_DATA0_INT__ADDR + (ipu_channel_params.channel<<6));
	writel(w1_d1, IPU_BASE_ADDRE + CPMEM_WORD1_DATA1_INT__ADDR + (ipu_channel_params.channel<<6));
	writel(w1_d2, IPU_BASE_ADDRE + CPMEM_WORD1_DATA2_INT__ADDR + (ipu_channel_params.channel<<6));
	writel(w1_d3, IPU_BASE_ADDRE + CPMEM_WORD1_DATA3_INT__ADDR + (ipu_channel_params.channel<<6));
	writel(w1_d4, IPU_BASE_ADDRE + CPMEM_WORD1_DATA4_INT__ADDR + (ipu_channel_params.channel<<6));
}

void dmfc_config(unsigned int IPU)
{
	unsigned int IPU_BASE_ADDRE = IPU1_ARB_BASE_ADDR;

	if(IPU == 2)
		IPU_BASE_ADDRE = IPU2_ARB_BASE_ADDR;

	// Same as _ipu_dmfc_init() in kernel
	writel(0x202020F6, IPU_BASE_ADDRE + IPU_DMFC_WR_CHAN_DEF__ADDR);
#ifdef IPU_USE_DC_CHANNEL
	writel(0x00000088, IPU_BASE_ADDRE + IPU_DMFC_WR_CHAN__ADDR);
	writel(0x00009694, IPU_BASE_ADDRE + IPU_DMFC_DP_CHAN__ADDR);
#else
	writel(0x00000090, IPU_BASE_ADDRE + IPU_DMFC_WR_CHAN__ADDR);
	writel(0x0000968A, IPU_BASE_ADDRE + IPU_DMFC_DP_CHAN__ADDR);
#endif
	writel(0x2020F6F6, IPU_BASE_ADDRE + IPU_DMFC_DP_CHAN_DEF__ADDR);

	// Set sync refresh channels and CSI->mem channel as high priority, copy from kernel
	writel(0x18800001, IPU_BASE_ADDRE + IPU_IDMAC_CH_PRI_1__ADDR);

	// Set MCU_T to divide MCU access window into 2, copy from kernel
	writel(0x00400000 | (8 << 18), IPU_BASE_ADDRE + IPU_IPU_DISP_GEN__ADDR);
}

void dp_config(unsigned int IPU)
{
	ipu_write_field(IPU, SRM_DP_COM_CONF_SYNC__DP_GAMMA_EN_SYNC, 0);
	ipu_write_field(IPU, SRM_DP_COM_CONF_SYNC__DP_CSC_YUV_SAT_MODE_SYNC, 0); //SAT mode is zero
	ipu_write_field(IPU, SRM_DP_COM_CONF_SYNC__DP_CSC_GAMUT_SAT_EN_SYNC, 0); //GAMUT en (RGB...)
	ipu_write_field(IPU, SRM_DP_COM_CONF_SYNC__DP_CSC_DEF_SYNC, 0); //CSC Disable
	ipu_write_field(IPU, SRM_DP_COM_CONF_SYNC__DP_COC_SYNC, 0); //no cursor
	ipu_write_field(IPU, SRM_DP_COM_CONF_SYNC__DP_GWCKE_SYNC, 0); //color keying disabled
	ipu_write_field(IPU, SRM_DP_COM_CONF_SYNC__DP_GWAM_SYNC, 1); //1=global alpha,0=local alpha
	ipu_write_field(IPU, SRM_DP_COM_CONF_SYNC__DP_GWSEL_SYNC, 1); //1=graphic is FG,0=graphic is BG
}

void microcode_config (int ipu_num, int word, int stop, char opcode[10], int lf, int af, int operand, int mapping, int waveform, int gluelogic, int sync)
{
	unsigned int IPU_BASE_ADDRE = IPU1_ARB_BASE_ADDR;
	unsigned int LowWord = 0;
	unsigned int HighWord = 0;
	unsigned int opcode_fixed;

	if  (ipu_num==2)
	   IPU_BASE_ADDRE=IPU2_ARB_BASE_ADDR;

//=========================================================================================================	
	//HLG - HOLD WORD GENERIC:  hold operand in register for next operating, without display access 
	if (!strcmp(opcode, "HLG")){
							    //[4:0] = 15'b0
		LowWord = LowWord | (operand << 5);         //[31:5]

		HighWord = HighWord | (operand >> 27);      //[36:32]
		opcode_fixed = 0x0;            		    //0-0
		HighWord = HighWord | (opcode_fixed << 5);  //[40:37]
		HighWord = HighWord | (stop << 9);          //[41]
    	}
//=========================================================================================================	
	//WRITE DATA to display
	if (!strcmp(opcode, "WROD")){
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11]
		LowWord = LowWord | (mapping   << 15);      //[19:15]
	//Generic data, this data is attached with OR to 16 MSB of mapped address	
		LowWord = LowWord | (operand << 20);        //[31:20]

		HighWord = HighWord | (operand >> 12);      //[35:32]
		opcode_fixed = 0x18 | (lf << 1);            //1-1-0-lf-0
		HighWord = HighWord | (opcode_fixed << 4);  //[40:36]
		HighWord = HighWord | (stop << 9);          //[41]
    	}
//=========================================================================================================	
	//ATTACH AND HOLD ADDRESS in REGISTER, Adding Mapped Address to held data and hold in register
	if (!strcmp(opcode, "HLOAR")){
		sync = 0;//fixed
		gluelogic = 0;//fixed
		waveform = 0;//fixed
		operand = 0;//fixed
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11]
		LowWord = LowWord | (mapping   << 15);      //[19:15]
		LowWord = LowWord | (operand << 20);        //[31:20]

		HighWord = HighWord | (operand >> 12);      //[32]
		//1 0 0 0 1 1 1 AF:  AF - Address shift flag is defined by user:
		//0: 24bit LSB operating or no operating,
		//1: 8 bit right shift, 24 MSB operating,
		opcode_fixed = 0x8E | (af << 0);            //1-0-0-0-1-1-1-AF
		HighWord = HighWord | (opcode_fixed << 1);  //[40:33]
		HighWord = HighWord | (stop << 9);          //[41]
    	}
//=========================================================================================================	
	//ATTACH AND WRITE ADDRESS to DISPLAY, Adding Mapped Address to held data and write to display
	if (!strcmp(opcode, "WROAR")){
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11]
		LowWord = LowWord | (mapping   << 15);      //[19:15]
		LowWord = LowWord | (operand << 20);        //[31:20]

		HighWord = HighWord | (operand >> 12);      //[32]
		//1 1 0 0 1 1 1 AF:  AF - Address shift flag is defined by user:
		//0: 24bit LSB operating or no operating,
		//1: 8 bit right shift, 24 MSB operating,
		opcode_fixed = 0xCE | (af << 0);            //1-1-0-0-1-1-1-AF
		HighWord = HighWord | (opcode_fixed << 1);  //[40:33]
		HighWord = HighWord | (stop << 9);          //[41]
    	}
//=========================================================================================================	
	//ATTACH AND HOLD DATA in REGISTER, Adding Mapped Address to held data and hold in register
	if (!strcmp(opcode, "HLODR")){
		sync = 0;//fixed
		gluelogic = 0;//fixed
		waveform = 0;//fixed
		operand = 0;//fixed
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11]
		LowWord = LowWord | (mapping   << 15);      //[19:15]
		LowWord = LowWord | (operand << 20);        //[31:20]

		HighWord = HighWord | (operand >> 12);      //[32]
		opcode_fixed = 0x8C;                         //
		HighWord = HighWord | (opcode_fixed << 1);  //[40:33]
		HighWord = HighWord | (stop << 9);          //[41]
    	}
//=========================================================================================================	
	//WRODR -  WRITE_DATA_OPCODE - ATTACH AND WRITE DATA to DISPLAY, 
	//Adding Mapped Data to hold data in internal register and write to display
	if (!strcmp(opcode, "WRODR")){
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11]
		LowWord = LowWord | (mapping   << 15);      //[19:15]
		LowWord = LowWord | (operand << 20);        //[31:20]

		HighWord = HighWord | (operand >> 12);      //[32]
		opcode_fixed = 0xCC;                         //
		HighWord = HighWord | (opcode_fixed << 1);  //[40:33]
		HighWord = HighWord | (stop << 9);          //[41]
    	}
//=========================================================================================================	
	//WRITE BIT CHANNEL. merging 1bit mask from IDMAC mask channel with data and write to display
	if (!strcmp(opcode, "WRBC")){
		operand = 0;//fixed
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11]
		LowWord = LowWord | (mapping   << 15);      //[19:15]
		LowWord = LowWord | (operand << 20);        //[31:20]

		opcode_fixed = 0x19B;                       //
		HighWord = HighWord | (opcode_fixed << 0);  //[40:32]
		HighWord = HighWord | (stop << 9);          //[41]
    	}
//=========================================================================================================	
	//WAIT CLOCK - Waiting N clocks
	if (!strcmp(opcode, "WCLK")){
		sync = 0;//fixed
		gluelogic = 0;//fixed
		waveform = 0;//fixed
		mapping = 0;//fixed
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11]
		LowWord = LowWord | (mapping   << 15);      //[19:15]
		LowWord = LowWord | (operand << 20);        //[31:20]

		HighWord = HighWord | (operand >> 12);      //[32]
		opcode_fixed = 0xC9;                         //
		HighWord = HighWord | (opcode_fixed << 1);  //[40:33]
		HighWord = HighWord | (stop << 9);          //[41]
    	}
//=========================================================================================================	
	//WAIT STATUS - 3 microcodes command loop for  checking display status by POLLING READ. THE WSTS_II has to be used immediately after WSTS _III
	if (!strcmp(opcode, "WSTS_III")){
		stop = 0;//fixed
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11]
		LowWord = LowWord | (mapping   << 15);      //[19:15]
		//number of IPU's clock to latch data from DI. after WSTS cycle start, defined by user
		LowWord = LowWord | (operand << 20);        //[31:20]

		HighWord = HighWord | (operand >> 12);      //[32]
		opcode_fixed = 0x8B;                         //
		HighWord = HighWord | (opcode_fixed << 1);  //[40:33]
		HighWord = HighWord | (stop << 9);          //[41]
    	}
//=========================================================================================================	
	//WAIT STATUS - second command in  3 microcode commands loop or first command in 2 microcode commands loop for  checking display status by POLLING READ. THE WSTS_I has to be used immediately after WSTS_II
	if (!strcmp(opcode, "WSTS_II")){
		stop = 0;//fixed
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11]
		LowWord = LowWord | (mapping   << 15);      //[19:15]
		//number of IPU's clock to latch data from DI. after WSTS cycle start, defined by user
		LowWord = LowWord | (operand << 20);        //[31:20]

		HighWord = HighWord | (operand >> 12);      //[32]
		opcode_fixed = 0x8A;                        //
		HighWord = HighWord | (opcode_fixed << 1);  //[40:33]
		HighWord = HighWord | (stop << 9);          //[41]
    	}
//=========================================================================================================	
	//WAIT STATUS - third command in  3 microcode commands loop or second command in 2 microcode commands loop or first command in one command loop for  checking display status by POLLING READ.
	if (!strcmp(opcode, "WSTS_I")){
		stop = 0;//fixed
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11]
		LowWord = LowWord | (mapping   << 15);      //[19:15]
		//number of IPU's clock to latch data from DI. after WSTS cycle start, defined by user
		LowWord = LowWord | (operand << 20);        //[31:20]

		HighWord = HighWord | (operand >> 12);      //[32]
		opcode_fixed = 0x89;                         //
		HighWord = HighWord | (opcode_fixed << 1);  //[40:33]
		HighWord = HighWord | (stop << 9);          //[41]
    	}
//=========================================================================================================	
	//HOLD ADDRESS in REGISTER: display's address which is calculated by IPU, is stored in register
	if (!strcmp(opcode, "HLOA")){
		sync = 0;//fixed
		gluelogic = 0;//fixed
		waveform = 0;//fixed
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11]
		LowWord = LowWord | (mapping   << 15);      //[19:15]
	//Generic data, this data is attached with OR to 16 MSB of mapped address	
		LowWord = LowWord | (operand << 20);        //[31:20]

		HighWord = HighWord | (operand >> 12);      //[35:32]
		//1 0 1 0 AF:  AF - Address shift flag is defined by user: 
		//0: 24bit LSB operating or no operating, 
		//1: 8 bit right shift, 24 MSB operating,
		opcode_fixed = 0x14 | (af << 0);            //1-0-1-0-AF
		HighWord = HighWord | (opcode_fixed << 4);  //[40:36]
		HighWord = HighWord | (stop << 9);          //[41]
    	}
//=========================================================================================================	
	//HOLD ADDRESS in REGISTER: display's address which is calculated by IPU, is stored in register
	if (!strcmp(opcode, "WROA")){
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11]
		LowWord = LowWord | (mapping   << 15);      //[19:15]
	//Generic data, this data is attached with OR to 16 MSB of mapped address	
		LowWord = LowWord | (operand << 20);        //[31:20]

		HighWord = HighWord | (operand >> 12);      //[35:32]
		//1 0 1 0 AF:  AF - Address shift flag is defined by user: 
		//0: 24bit LSB operating or no operating, 
		//1: 8 bit right shift, 24 MSB operating,
		opcode_fixed = 0x1C | (af << 0);            //1-1-1-0-AF
		HighWord = HighWord | (opcode_fixed << 4);  //[40:36]
		HighWord = HighWord | (stop << 9);          //[41]
    	}
//=========================================================================================================	
	//HOLD DATA in REGISTER
	if (!strcmp(opcode, "HLOD")){
		sync = 0;//fixed
		gluelogic=0;//fixed
		waveform=0;//fixed
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11]
		LowWord = LowWord | (mapping   << 15);      //[19:15]
	//Generic data, this data is attached with OR to 16 MSB of mapped address	
		LowWord = LowWord | (operand << 20);        //[31:20]

		HighWord = HighWord | (operand >> 12);      //[35:32]
		opcode_fixed = 0x10;                        //1-0-0-0-0
		HighWord = HighWord | (opcode_fixed << 4);  //[40:36]
		HighWord = HighWord | (stop << 9);          //[41]
    	}
//=========================================================================================================	
	//WRG - Write 24bit word to DI and Hold the word in register
	//WRITE WORD GENERIC
	//This opcode is used for sending "a user's code", which is stored in microcode memory  to  a display
	//NO MAPPING
	if (!strcmp(opcode, "WRG")){
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11]
		LowWord = LowWord | (operand << 15);        //[31:15]

		HighWord = HighWord | (operand >> 17);      //[38:32]
		opcode_fixed = 1;                           //0-1
		HighWord = HighWord | (opcode_fixed << 7);  //[40:39]
		HighWord = HighWord | (stop << 9);          //[41]		
	}
//=========================================================================================================	
        //RD - Read command parameters:
	if (!strcmp(opcode, "RD")){
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11] = synchronization
		LowWord = LowWord | (mapping   << 15);      //[19:15]
		//operand - means delay value in DI_CLK for display's data latching by DI, defined by user
		//number of IPU's clock to latch data from DI. after WSTS cycle start
		LowWord = LowWord | (operand << 20);        //[31:20]

		HighWord = HighWord | (operand >> 12);      //[32]
		opcode_fixed = 0x88;                        //
		HighWord = HighWord | (opcode_fixed << 1);  //[40:33]
		HighWord = HighWord | (stop << 9);          //[41]

	}
//=========================================================================================================	
        //WAIT FOR ACKNOWLEDGE
	if (!strcmp(opcode, "WACK")){
		mapping = 0;//fixed
		operand = 0;//fixed
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11] = synchronization
		LowWord = LowWord | (mapping   << 15);      //[19:15]
		//operand - means delay value in DI_CLK for display's data latching by DI, defined by user
		//number of IPU's clock to latch data from DI. after WSTS cycle start
		LowWord = LowWord | (operand << 20);        //[31:20]

		opcode_fixed = 0x11A;                       //
		HighWord = HighWord | (opcode_fixed << 0);  //[40:32]
		HighWord = HighWord | (stop << 9);          //[41]

	}
//=========================================================================================================	
        //HMA  - HOLD_MICROCODE_ADDRESS - hold operand in special Microcode address register
	if (!strcmp(opcode, "HMA")){
		                                            //[4:0]
		//Microcode address defined by user
		LowWord = LowWord | (operand << 5);         //[12:5]
		opcode_fixed = 0x2;                         //
		HighWord = HighWord | (opcode_fixed << 5);  //[40:37]
		HighWord = HighWord | (stop << 9);          //[41]

	}
//========================================================================================================= 
		//HMA1  - HOLD_MICROCODE_ADDRESS - hold operand in special Microcode address register
	if (!strcmp(opcode, "HMA1")){
													//[4:0]
		//Microcode address defined by user
		LowWord = LowWord | (operand << 5); 		//[12:5]
		opcode_fixed = 0x1; 						//
		HighWord = HighWord | (opcode_fixed << 5);	//[40:37]
		HighWord = HighWord | (stop << 9);			//[41]

	}
//=========================================================================================================	
        //BMA  - BRANCH_MICROCODE_ADDRESS jump to Microcode address which is stored at 
	//special Microcode address register
	if (!strcmp(opcode, "BMA")){
		                                            //[36:0] ==0
		LowWord = LowWord | sync; 		//[2:0]
		LowWord = LowWord | (operand << 5); 		//[12:5]
		opcode_fixed = 0x3;                         //
		HighWord = HighWord | (af << 3);	//[35]
		HighWord = HighWord | (lf << 4);	//[36]
		HighWord = HighWord | (opcode_fixed << 5);  //[40:37]
		HighWord = HighWord | (stop << 9);          //[41]

	}
//=========================================================================================================	
        //Additional information for event masking
	if (!strcmp(opcode, "MSK")){
		sync = 0;//fixed
		gluelogic = 0;//fixed
		waveform = 0;//fixed
		stop = 0;//fixed 
		//(no mapping)
		LowWord = LowWord | sync;                   //[3:0]
		LowWord = LowWord | (gluelogic << 4);       //[10:4]
		LowWord = LowWord | (waveform  << 11);      //[14:11] = synchronization
		//[32-28] - 0
                //[27] e0m- event 0 mask, defined by user
                //[26] e1m - event 1 mask, defined by user
                //[25] e2m - event 2 mask, defined by user
                //[24] e3m - event 3 mask, defined by user
                //[23] nfm- new frame mask , defined by user
                //[22] nlm- new line mask , defined by user
                //[21] nfldm- new field mask , defined by user
                //[20] eofm- end of frame mask, defined by user
                //[19] eolm- end of line mask, defined by user
                //[18] eofldm-  end of field mask, defined by user
                //[17] nadm- new address mask, defined by user
                //[16] ncm- new channel mask, defined by user
                //[15] dm - data mask, defined by user 
		LowWord = LowWord | (operand   << 15);      //[19:15]

		HighWord = HighWord | (operand >> 17);      //[32]
		opcode_fixed = 0xC8;                        //
		HighWord = HighWord | (opcode_fixed << 1);  //[40:33]
		HighWord = HighWord | (stop << 9);          //[41]

	}
//=========================================================================================================	
		
	writel(LowWord, IPU_BASE_ADDRE + IPU_MEM_DC_MICROCODE_BASE_ADDR + word * 8);
	writel(HighWord, IPU_BASE_ADDRE + IPU_MEM_DC_MICROCODE_BASE_ADDR + word * 8 + 4);
}

void microcode_event(int ipu_num, int channel, char event[8], int priority, int address)
{
   if(channel == 0){

     if(!strcmp(event, "NL")){
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_0__COD_NL_START_CHAN_0, address);
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_0__COD_NL_PRIORITY_CHAN_0, priority);
     }//NL
     if(!strcmp(event, "NF")){
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_0__COD_NF_START_CHAN_0, address);
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_0__COD_NF_PRIORITY_CHAN_0, priority);
     }//NF
     if(!strcmp(event, "NFIELD")){
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_0__COD_NFIELD_START_CHAN_0, address);
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_0__COD_NFIELD_PRIORITY_CHAN_0, priority);
     }//NFIELD
      if(!strcmp(event, "EOF")){
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_0__COD_EOF_START_CHAN_0, address);
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_0__COD_EOF_PRIORITY_CHAN_0, priority);
     }//EOF
      if(!strcmp(event, "EOFIELD")){
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_0__COD_EOFIELD_START_CHAN_0, address);
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_0__COD_EOFIELD_PRIORITY_CHAN_0, priority);
     }//EOFIELD
      if(!strcmp(event, "EOL")){
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_0__COD_EOL_START_CHAN_0, address);
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_0__COD_EOL_PRIORITY_CHAN_0, priority);
     }//EOL
      if(!strcmp(event, "NEW_CHAN")){
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_0__COD_NEW_CHAN_START_CHAN_0, address);
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_0__COD_NEW_CHAN_PRIORITY_CHAN_0, priority);
     }//NEW_CHAN
      if(!strcmp(event, "NEW_ADDR")){
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_0__COD_NEW_ADDR_START_CHAN_0, address);
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_0__COD_NEW_ADDR_PRIORITY_CHAN_0, priority);
     }//NEW_ADDR
      if(!strcmp(event, "NEW_DATA")){
       ipu_write_field(ipu_num, IPU_DC_RL4_CH_0__COD_NEW_DATA_START_CHAN_0, address);
       ipu_write_field(ipu_num, IPU_DC_RL4_CH_0__COD_NEW_DATA_PRIORITY_CHAN_0, priority);
     }//NEW_DATA
   }//channel 0 ******************************************************************************

//*************************************************************************************   
   if(channel == 1){

     if(!strcmp(event, "NL")){
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_1__COD_NL_START_CHAN_1, address);
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_1__COD_NL_PRIORITY_CHAN_1, priority);
     }//NL
     if(!strcmp(event, "NF")){
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_1__COD_NF_START_CHAN_1, address);
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_1__COD_NF_PRIORITY_CHAN_1, priority);
     }//NF
     if(!strcmp(event, "NFIELD")){
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_1__COD_NFIELD_START_CHAN_1, address);
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_1__COD_NFIELD_PRIORITY_CHAN_1, priority);
     }//NFIELD
      if(!strcmp(event, "EOF")){
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_1__COD_EOF_START_CHAN_1, address);
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_1__COD_EOF_PRIORITY_CHAN_1, priority);
     }//EOF
      if(!strcmp(event, "EOFIELD")){
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_1__COD_EOFIELD_START_CHAN_1, address);
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_1__COD_EOFIELD_PRIORITY_CHAN_1, priority);
     }//EOFIELD
      if(!strcmp(event, "EOL")){
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_1__COD_EOL_START_CHAN_1, address);
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_1__COD_EOL_PRIORITY_CHAN_1, priority);
     }//EOL
      if(!strcmp(event, "NEW_CHAN")){
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_1__COD_NEW_CHAN_START_CHAN_1, address);
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_1__COD_NEW_CHAN_PRIORITY_CHAN_1, priority);
     }//NEW_CHAN
      if(!strcmp(event, "NEW_ADDR")){
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_1__COD_NEW_ADDR_START_CHAN_1, address);
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_1__COD_NEW_ADDR_PRIORITY_CHAN_1, priority);
     }//NEW_ADDR
      if(!strcmp(event, "NEW_DATA")){
       ipu_write_field(ipu_num, IPU_DC_RL4_CH_1__COD_NEW_DATA_START_CHAN_1, address);
       ipu_write_field(ipu_num, IPU_DC_RL4_CH_1__COD_NEW_DATA_PRIORITY_CHAN_1, priority);
     }//NEW_DATA
   }//channel 1 ****************************************************************************************

//*************************************************************************************   
   if(channel == 2){

     if(!strcmp(event, "NL")){
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_2__COD_NL_START_CHAN_2, address);
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_2__COD_NL_PRIORITY_CHAN_2, priority);
     }//NL
     if(!strcmp(event, "NF")){
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_2__COD_NF_START_CHAN_2, address);
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_2__COD_NF_PRIORITY_CHAN_2, priority);
     }//NF
     if(!strcmp(event, "NFIELD")){
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_2__COD_NFIELD_START_CHAN_2, address);
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_2__COD_NFIELD_PRIORITY_CHAN_2, priority);
     }//NFIELD
      if(!strcmp(event, "EOF")){
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_2__COD_EOF_START_CHAN_2, address);
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_2__COD_EOF_PRIORITY_CHAN_2, priority);
     }//EOF
      if(!strcmp(event, "EOFIELD")){
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_2__COD_EOFIELD_START_CHAN_2, address);
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_2__COD_EOFIELD_PRIORITY_CHAN_2, priority);
     }//EOFIELD
      if(!strcmp(event, "EOL")){
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_2__COD_EOL_START_CHAN_2, address);
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_2__COD_EOL_PRIORITY_CHAN_2, priority);
     }//EOL
      if(!strcmp(event, "NEW_CHAN")){
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_2__COD_NEW_CHAN_START_CHAN_2, address);
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_2__COD_NEW_CHAN_PRIORITY_CHAN_2, priority);
     }//NEW_CHAN
      if(!strcmp(event, "NEW_ADDR")){
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_2__COD_NEW_ADDR_START_CHAN_2, address);
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_2__COD_NEW_ADDR_PRIORITY_CHAN_2, priority);
     }//NEW_ADDR
      if(!strcmp(event, "NEW_DATA")){
       ipu_write_field(ipu_num, IPU_DC_RL4_CH_2__COD_NEW_DATA_START_CHAN_2, address);
       ipu_write_field(ipu_num, IPU_DC_RL4_CH_2__COD_NEW_DATA_PRIORITY_CHAN_2, priority);
     }//NEW_DATA
   }//channel 2 ****************************************************************************************
   
//*************************************************************************************   
   if(channel == 5){

     if(!strcmp(event, "NL")){
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_5__COD_NL_START_CHAN_5, address);
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_5__COD_NL_PRIORITY_CHAN_5, priority);
     }//NL
     if(!strcmp(event, "NF")){
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_5__COD_NF_START_CHAN_5, address);
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_5__COD_NF_PRIORITY_CHAN_5, priority);
     }//NF
     if(!strcmp(event, "NFIELD")){
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_5__COD_NFIELD_START_CHAN_5, address);
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_5__COD_NFIELD_PRIORITY_CHAN_5, priority);
     }//NFIELD
      if(!strcmp(event, "EOF")){
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_5__COD_EOF_START_CHAN_5, address);
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_5__COD_EOF_PRIORITY_CHAN_5, priority);
     }//EOF
      if(!strcmp(event, "EOFIELD")){
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_5__COD_EOFIELD_START_CHAN_5, address);
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_5__COD_EOFIELD_PRIORITY_CHAN_5, priority);
     }//EOFIELD
      if(!strcmp(event, "EOL")){
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_5__COD_EOL_START_CHAN_5, address);
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_5__COD_EOL_PRIORITY_CHAN_5, priority);
     }//EOL
      if(!strcmp(event, "NEW_CHAN")){
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_5__COD_NEW_CHAN_START_CHAN_5, address);
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_5__COD_NEW_CHAN_PRIORITY_CHAN_5, priority);
     }//NEW_CHAN
      if(!strcmp(event, "NEW_ADDR")){
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_5__COD_NEW_ADDR_START_CHAN_5, address);
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_5__COD_NEW_ADDR_PRIORITY_CHAN_5, priority);
     }//NEW_ADDR
      if(!strcmp(event, "NEW_DATA")){
       ipu_write_field(ipu_num, IPU_DC_RL4_CH_5__COD_NEW_DATA_START_CHAN_5, address);
       ipu_write_field(ipu_num, IPU_DC_RL4_CH_5__COD_NEW_DATA_PRIORITY_CHAN_5, priority);
     }//NEW_DATA
   }//channel 5 ****************************************************************************************

//*************************************************************************************   
   if(channel == 6){

     if(!strcmp(event, "NL")){
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_6__COD_NL_START_CHAN_6, address);
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_6__COD_NL_PRIORITY_CHAN_6, priority);
     }//NL
     if(!strcmp(event, "NF")){
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_6__COD_NF_START_CHAN_6, address);
       ipu_write_field(ipu_num, IPU_DC_RL0_CH_6__COD_NF_PRIORITY_CHAN_6, priority);
     }//NF
     if(!strcmp(event, "NFIELD")){
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_6__COD_NFIELD_START_CHAN_6, address);
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_6__COD_NFIELD_PRIORITY_CHAN_6, priority);
     }//NFIELD
      if(!strcmp(event, "EOF")){
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_6__COD_EOF_START_CHAN_6, address);
       ipu_write_field(ipu_num, IPU_DC_RL1_CH_6__COD_EOF_PRIORITY_CHAN_6, priority);
     }//EOF
      if(!strcmp(event, "EOFIELD")){
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_6__COD_EOFIELD_START_CHAN_6, address);
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_6__COD_EOFIELD_PRIORITY_CHAN_6, priority);
     }//EOFIELD
      if(!strcmp(event, "EOL")){
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_6__COD_EOL_START_CHAN_6, address);
       ipu_write_field(ipu_num, IPU_DC_RL2_CH_6__COD_EOL_PRIORITY_CHAN_6, priority);
     }//EOL
      if(!strcmp(event, "NEW_CHAN")){
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_6__COD_NEW_CHAN_START_CHAN_6, address);
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_6__COD_NEW_CHAN_PRIORITY_CHAN_6, priority);
     }//NEW_CHAN
      if(!strcmp(event, "NEW_ADDR")){
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_6__COD_NEW_ADDR_START_CHAN_6, address);
       ipu_write_field(ipu_num, IPU_DC_RL3_CH_6__COD_NEW_ADDR_PRIORITY_CHAN_6, priority);
     }//NEW_ADDR
      if(!strcmp(event, "NEW_DATA")){
       ipu_write_field(ipu_num, IPU_DC_RL4_CH_6__COD_NEW_DATA_START_CHAN_6, address);
       ipu_write_field(ipu_num, IPU_DC_RL4_CH_6__COD_NEW_DATA_PRIORITY_CHAN_6, priority);
     }//NEW_DATA
   }//channel 6 ****************************************************************************************

   
}


#ifdef DISPLAY_INTERLACED
void dc_config_interlaced(unsigned int IPU, unsigned int DI)
{
	unsigned int microCodeAddr_DATA;
	int mapping = 1;

#if (DISPLAY_IF_BPP == 24)
	mapping = 1;
#endif

#if (DISPLAY_IF_BPP == 18)
	mapping = 2;
#endif

#if (DISPLAY_IF_BPP == 16)
	mapping = 4;
#endif

	microCodeAddr_DATA = 1;

	//DATA
	microcode_config(
	                 IPU, 
	                 microCodeAddr_DATA,//word address
	                 1,//stop
	                 "WROD",//OPCODE
	                 0, //LF
	                 0, //AF
	                 0, //OPERAND
	                 mapping, //MAPPING
	                 DI_SDC_WAVEFORM + 1, //WAVEFORM
	                 0, //GLUELOGIC
	                 DI_COUNTER_APIXEL);//SYNC

#ifdef IPU_USE_DC_CHANNEL
	/*********************************************************************
	* link events to routines
	**********************************************************************/
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "NL",			3,	microCodeAddr_DATA);  //prior=3
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "NF",			0,	0);
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "NFIELD",		0,	0);
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "EOF",		 	0,	0);
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "EOFIELD",		0,	0);
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "EOL",			2,	microCodeAddr_DATA);
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "NEW_CHAN",	0,	0);
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "NEW_ADDR",	0,	0);
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "NEW_DATA",	1,	microCodeAddr_DATA); 

	//WR_CH_CONF_1  
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__PROG_START_TIME_1, 0);  //no anti-tearing elimination used
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__CHAN_MASK_DEFAULT_1, 0);  //only the highest priority event will be served
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__PROG_CHAN_TYP_1, 4);  //normal mode without anti-tearing.DO NOT CHANGE!!
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__PROG_DISP_ID_1, 1);  //select dc_display 1 to link channel #1
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__PROG_DI_ID_1, DI);  //DC channel 1 associated to current DI
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__PROG_DI_ID_5, 1 - DI);  //DC channel 5 associated to another DI
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__W_SIZE_1, 2);  //component size access to DC set to 24bit MSB.
#ifdef DISPLAY_INTERLACED
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__FIELD_MODE_1, 1);  //field mode enable
#else
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__FIELD_MODE_1, 0);  //frame mode enable
#endif
	ipu_write_field(IPU, IPU_DC_WR_CH_ADDR_1__ST_ADDR_1, 0);  //START ADDRESS OF CHANNEL

#else

	/*********************************************************************
	* link events to routines
	**********************************************************************/
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "NL",			3,	microCodeAddr_DATA);  //prior=3
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "NF",			0,	0);
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "NFIELD",		0,	0);
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "EOF",		 	0,	0);
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "EOFIELD",		0,	0);
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "EOL",			2,	microCodeAddr_DATA);
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "NEW_CHAN",	0,	0);
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "NEW_ADDR",	0,	0);
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "NEW_DATA",	1,	microCodeAddr_DATA); 

	//WR_CH_CONF_5  
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__PROG_START_TIME_5, 0);  //no anti-tearing elimination used
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__CHAN_MASK_DEFAULT_5, 0);  //only the highest priority event will be served
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__PROG_CHAN_TYP_5, 4);  //normal mode without anti-tearing.DO NOT CHANGE!!
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__PROG_DISP_ID_5, 1);  //select dc_display 1 to link channel #5
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__PROG_DI_ID_5, DI);  //DC channel 5 associated to current DI
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__PROG_DI_ID_1, 1 - DI);  //DC channel 1 associated to another DI
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__W_SIZE_5, 2);  //component size access to DC set to 24bit MSB.
#ifdef DISPLAY_INTERLACED
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__FIELD_MODE_5, 1);  //field mode enable
#else
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__FIELD_MODE_5, 0);  //frame mode enable
#endif
	ipu_write_field(IPU, IPU_DC_WR_CH_ADDR_5__ST_ADDR_5, 0);  //START ADDRESS OF CHANNEL
#endif

	//GENERAL 
	ipu_write_field(IPU, IPU_DC_GEN__SYNC_PRIORITY_5, 1);  //sets the priority of channel #5 to high.
	ipu_write_field(IPU, IPU_DC_GEN__SYNC_PRIORITY_1, 1);  //sets the priority of channel #1 to high.
	ipu_write_field(IPU, IPU_DC_GEN__MASK4CHAN_5, 0);  // mask channel is associated to the sync flow via DC (without DP)
	ipu_write_field(IPU, IPU_DC_GEN__MASK_EN, 0);  // mask channel is disabled.(mask channel can associated with different IDMAC channels)
	ipu_write_field(IPU, IPU_DC_GEN__DC_CH5_TYPE, 0);  // alternate sync or asyn flow
	ipu_write_field(IPU, IPU_DC_GEN__SYNC_1_6, 2);  //DC channel #1 handles sync flow

	//DISP_CONF  
	ipu_write_field(IPU, IPU_DC_DISP_CONF1_1__ADDR_INCREMENT_1, 0);  //automatical address increase by 1
	ipu_write_field(IPU, IPU_DC_DISP_CONF1_1__DISP_TYP_1, 2);  //paralel display without byte enable
	ipu_write_field(IPU, IPU_DC_DISP_CONF2_1__SL_1, DISPLAY_WIDTH);  //stride line

	//DC_UGDE 
	ipu_write_field(IPU, IPU_DC_UGDE1_0__NF_NL_1, 0);  //NL->0,NF->1,NFIELD->2
	ipu_write_field(IPU, IPU_DC_UGDE1_0__AUTORESTART_1, 0);  //no autorestart
	ipu_write_field(IPU, IPU_DC_UGDE1_0__ODD_EN_1, 0);  // disable 'odd'
	ipu_write_field(IPU, IPU_DC_UGDE1_0__COD_ODD_START_1, 1);  //words 1 1st part
	ipu_write_field(IPU, IPU_DC_UGDE1_0__COD_EV_START_1, 5);  //word 2 2nd part
	ipu_write_field(IPU, IPU_DC_UGDE1_0__COD_EV_PRIORITY_1,  1);  //enabled. all others are disabled.
#ifdef IPU_USE_DC_CHANNEL
	ipu_write_field(IPU, IPU_DC_UGDE1_0__ID_CODED_1, 0x1);  //DC_CHANNEL_DC_SYNC_OR_ASYNC
#else
	ipu_write_field(IPU, IPU_DC_UGDE1_0__ID_CODED_1, 0x3);  //DC_CHANNEL_DP_PRIMARY
#endif
	ipu_write_field(IPU, IPU_DC_UGDE1_1__STEP_1, 0);  //every data
	ipu_write_field(IPU, IPU_DC_UGDE1_2__OFFSET_DT_1, 0);  //no offset
	ipu_write_field(IPU, IPU_DC_UGDE1_3__STEP_REPEAT_1, (DISPLAY_WIDTH - 1));

	//DC_MAP, same as _ipu_init_dc_mappings() in kernel
	// config DC mapping point 1, IPU_PIX_FMT_RGB24
	ipu_write_field(IPU, IPU_DC_MAP_CONF_15__MD_OFFSET_0, 7);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_15__MD_MASK_0, 0xFF);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_15__MD_OFFSET_1, 15);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_15__MD_MASK_1, 0xFF);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_16__MD_OFFSET_2, 23);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_16__MD_MASK_2, 0xFF);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_0__MAPPING_PNTR_BYTE0_0, 0);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_0__MAPPING_PNTR_BYTE1_0, 1);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_0__MAPPING_PNTR_BYTE2_0, 2);

	// config DC mapping point 2, IPU_PIX_FMT_RGB666
	ipu_write_field(IPU, IPU_DC_MAP_CONF_16__MD_OFFSET_3, 5);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_16__MD_MASK_3, 0xFC);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_17__MD_OFFSET_4, 11);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_17__MD_MASK_4, 0xFC);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_17__MD_OFFSET_5, 17);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_17__MD_MASK_5, 0xFC);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_0__MAPPING_PNTR_BYTE0_1, 3);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_0__MAPPING_PNTR_BYTE1_1, 4);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_0__MAPPING_PNTR_BYTE2_1, 5);

	// config DC mapping point 3, IPU_PIX_FMT_YUV444
	ipu_write_field(IPU, IPU_DC_MAP_CONF_18__MD_OFFSET_6, 15);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_18__MD_MASK_6, 0xFF);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_18__MD_OFFSET_7, 23);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_18__MD_MASK_7, 0xFF);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_19__MD_OFFSET_8, 7);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_19__MD_MASK_8, 0xFF);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_1__MAPPING_PNTR_BYTE0_2, 6);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_1__MAPPING_PNTR_BYTE1_2, 7);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_1__MAPPING_PNTR_BYTE2_2, 8);

	// config DC mapping point 4, IPU_PIX_FMT_RGB565
	ipu_write_field(IPU, IPU_DC_MAP_CONF_19__MD_OFFSET_9, 4);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_19__MD_MASK_9, 0xF8);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_20__MD_OFFSET_10, 10);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_20__MD_MASK_10, 0xFC);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_20__MD_OFFSET_11, 15);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_20__MD_MASK_11, 0xF8);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_1__MAPPING_PNTR_BYTE0_3, 9);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_1__MAPPING_PNTR_BYTE1_3, 10);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_1__MAPPING_PNTR_BYTE2_3, 11);

	// config DC mapping point 5, IPU_PIX_FMT_LVDS666
	ipu_write_field(IPU, IPU_DC_MAP_CONF_21__MD_OFFSET_12, 5);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_21__MD_MASK_12, 0xFC);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_21__MD_OFFSET_13, 13);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_21__MD_MASK_13, 0xFC);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_22__MD_OFFSET_14, 21);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_22__MD_MASK_14, 0xFC);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_2__MAPPING_PNTR_BYTE0_4, 12);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_2__MAPPING_PNTR_BYTE1_4, 13);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_2__MAPPING_PNTR_BYTE2_4, 14);

	// config DC mapping point 6,7, IPU_PIX_FMT_VYUY
	ipu_write_field(IPU, IPU_DC_MAP_CONF_22__MD_OFFSET_15, 7);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_22__MD_MASK_15, 0xFF);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_23__MD_OFFSET_16, 0);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_23__MD_MASK_16, 0x00);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_23__MD_OFFSET_17, 15);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_23__MD_MASK_17, 0xFF);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_2__MAPPING_PNTR_BYTE0_5, 15);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_2__MAPPING_PNTR_BYTE1_5, 16);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_2__MAPPING_PNTR_BYTE2_5, 17);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_24__MD_OFFSET_18, 0);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_24__MD_MASK_18, 0x00);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_24__MD_OFFSET_19, 7);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_24__MD_MASK_19, 0xFF);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_25__MD_OFFSET_20, 15);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_25__MD_MASK_20, 0xFF);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_3__MAPPING_PNTR_BYTE0_6, 18);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_3__MAPPING_PNTR_BYTE1_6, 19);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_3__MAPPING_PNTR_BYTE2_6, 20);

	// config DC mapping point 8,9, IPU_PIX_FMT_UYVY
	ipu_write_field(IPU, IPU_DC_MAP_CONF_3__MAPPING_PNTR_BYTE0_7, 18);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_3__MAPPING_PNTR_BYTE1_7, 19);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_3__MAPPING_PNTR_BYTE2_7, 20);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_4__MAPPING_PNTR_BYTE0_8, 15);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_4__MAPPING_PNTR_BYTE1_8, 16);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_4__MAPPING_PNTR_BYTE2_8, 17);

	// config DC mapping point 10,11, IPU_PIX_FMT_YUYV
	ipu_write_field(IPU, IPU_DC_MAP_CONF_4__MAPPING_PNTR_BYTE0_9, 17);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_4__MAPPING_PNTR_BYTE1_9, 16);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_4__MAPPING_PNTR_BYTE2_9, 15);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_5__MAPPING_PNTR_BYTE0_10, 16);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_5__MAPPING_PNTR_BYTE1_10, 17);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_5__MAPPING_PNTR_BYTE2_10, 15);

	// config DC mapping point 12,13, IPU_PIX_FMT_YVYU
	ipu_write_field(IPU, IPU_DC_MAP_CONF_5__MAPPING_PNTR_BYTE0_11, 16);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_5__MAPPING_PNTR_BYTE1_11, 17);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_5__MAPPING_PNTR_BYTE2_11, 15);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_6__MAPPING_PNTR_BYTE0_12, 17);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_6__MAPPING_PNTR_BYTE1_12, 16);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_6__MAPPING_PNTR_BYTE2_12, 15);

	// config DC mapping point 14, IPU_PIX_FMT_GBR24, IPU_PIX_FMT_VYU444
	ipu_write_field(IPU, IPU_DC_MAP_CONF_6__MAPPING_PNTR_BYTE0_13, 2);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_6__MAPPING_PNTR_BYTE1_13, 0);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_6__MAPPING_PNTR_BYTE2_13, 1);

	// config DC mapping point 15, IPU_PIX_FMT_BGR24
	ipu_write_field(IPU, IPU_DC_MAP_CONF_7__MAPPING_PNTR_BYTE0_14, 2);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_7__MAPPING_PNTR_BYTE1_14, 1);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_7__MAPPING_PNTR_BYTE2_14, 0);
}
#else
void dc_config(unsigned int IPU, unsigned int DI)
{
	unsigned int microCodeAddr_NL, microCodeAddr_EOL, microCodeAddr_DATA;
	int mapping = 1;

#if (DISPLAY_IF_BPP == 24)
	mapping = 1;
#endif

#if (DISPLAY_IF_BPP == 18)
	mapping = 2;
#endif

#if (DISPLAY_IF_BPP == 16)
	mapping = 4;
#endif

	microCodeAddr_NL = 2;
	microCodeAddr_EOL = 3;
	microCodeAddr_DATA = 4;

	//MICROCODE 
	microcode_config(
	                 IPU, 
	                 1,//word address
	                 1,//stop
	                 "WROD",//OPCODE
	                 0, //LF
	                 0, //AF
	                 0, //OPERAND
	                 mapping, //MAPPING
	                 DI_SDC_WAVEFORM + 1, //WAVEFORM
	                 0, //GLUELOGIC
	                 DI_COUNTER_APIXEL);//SYNC

	microcode_config(
			 IPU, 
			 5,//word address
			 1,//stop
			 "WROD",//OPCODE
			 0, //LF
			 0, //AF
			 0, //OPERAND
			 mapping, //MAPPING
			 DI_SDC_WAVEFORM + 1, //WAVEFORM
			 0, //GLUELOGIC
			 DI_COUNTER_APIXEL);//SYNC
 
	//NL                 
	microcode_config(
	                 IPU, 
	                 microCodeAddr_NL,//word address
	                 1,//stop
	                 "WROD",//OPCODE
	                 0, //LF
	                 0, //AF
	                 0, //OPERAND
	                 mapping, //MAPPING
	                 DI_SDC_WAVEFORM + 1, //WAVEFORM
	                 0, //GLUELOGIC
	                 DI_COUNTER_APIXEL);//SYNC

	//EOL		 
	microcode_config(
	                 IPU, 
	                 microCodeAddr_EOL,//word address
	                 1,//stop
	                 "WROD",//OPCODE
	                 0, //LF
	                 0, //AF
	                 0, //OPERAND
	                 mapping, //MAPPING
	                 DI_SDC_WAVEFORM + 1, //WAVEFORM
	                 0, //GLUELOGIC
	                 DI_COUNTER_APIXEL);//SYNC

	//DATA
	microcode_config(
	                 IPU, 
	                 microCodeAddr_DATA,//word address
	                 1,//stop
	                 "WROD",//OPCODE
	                 0, //LF
	                 0, //AF
	                 0, //OPERAND
	                 mapping, //MAPPING
	                 DI_SDC_WAVEFORM + 1, //WAVEFORM
	                 0, //GLUELOGIC
	                 DI_COUNTER_APIXEL);//SYNC

#ifdef IPU_USE_DC_CHANNEL
	/*********************************************************************
	* link events to routines
	**********************************************************************/
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "NL",			3,	microCodeAddr_NL);
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "NF",			0,	0);
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "NFIELD",		0,	0);
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "EOF",		 	0,	0);
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "EOFIELD",		0,	0);
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "EOL",			2,	microCodeAddr_EOL);
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "NEW_CHAN",	0,	0);
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "NEW_ADDR",	0,	0);
	microcode_event(IPU, DC_CHANNEL_DC_SYNC_OR_ASYNC, "NEW_DATA",	1,	microCodeAddr_DATA); 

	//WR_CH_CONF_1  
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__PROG_START_TIME_1, 0);  //no anti-tearing elimination used
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__CHAN_MASK_DEFAULT_1, 0);  //only the highest priority event will be served
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__PROG_CHAN_TYP_1, 4);  //normal mode without anti-tearing.DO NOT CHANGE!!
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__PROG_DISP_ID_1, 1);  //select dc_display 1 to link channel #1
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__PROG_DI_ID_1, DI);  //DC channel 1 associated to current DI
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__PROG_DI_ID_5, 1 - DI);  //DC channel 5 associated to another DI
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__W_SIZE_1, 2);  //component size access to DC set to 24bit MSB.
#ifdef DISPLAY_INTERLACED
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__FIELD_MODE_1, 1);  //field mode enable
#else
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__FIELD_MODE_1, 0);  //frame mode enable
#endif
	ipu_write_field(IPU, IPU_DC_WR_CH_ADDR_1__ST_ADDR_1, 0);  //START ADDRESS OF CHANNEL

#else

	/*********************************************************************
	* link events to routines
	**********************************************************************/
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "NL",			3,	microCodeAddr_NL);
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "NF",			0,	0);
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "NFIELD",		0,	0);
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "EOF",		 	0,	0);
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "EOFIELD",		0,	0);
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "EOL",			2,	microCodeAddr_EOL);
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "NEW_CHAN",	0,	0);
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "NEW_ADDR",	0,	0);
	microcode_event(IPU, DC_CHANNEL_DP_PRIMARY, "NEW_DATA",	1,	microCodeAddr_DATA); 

	//WR_CH_CONF_5  
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__PROG_START_TIME_5, 0);  //no anti-tearing elimination used
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__CHAN_MASK_DEFAULT_5, 0);  //only the highest priority event will be served
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__PROG_CHAN_TYP_5, 4);  //normal mode without anti-tearing.DO NOT CHANGE!!
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__PROG_DISP_ID_5, 1);  //select dc_display 1 to link channel #5
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__PROG_DI_ID_5, DI);  //DC channel 5 associated to current DI
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_1__PROG_DI_ID_1, 1 - DI);  //DC channel 1 associated to another DI
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__W_SIZE_5, 2);  //component size access to DC set to 24bit MSB.
#ifdef DISPLAY_INTERLACED
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__FIELD_MODE_5, 1);  //field mode enable
#else
	ipu_write_field(IPU, IPU_DC_WR_CH_CONF_5__FIELD_MODE_5, 0);  //frame mode enable
#endif
	ipu_write_field(IPU, IPU_DC_WR_CH_ADDR_5__ST_ADDR_5, 0);  //START ADDRESS OF CHANNEL
#endif

	//GENERAL 
	ipu_write_field(IPU, IPU_DC_GEN__SYNC_PRIORITY_5, 1);  //sets the priority of channel #5 to high.
	ipu_write_field(IPU, IPU_DC_GEN__SYNC_PRIORITY_1, 1);  //sets the priority of channel #1 to high.
	ipu_write_field(IPU, IPU_DC_GEN__MASK4CHAN_5, 0);  // mask channel is associated to the sync flow via DC (without DP)
	ipu_write_field(IPU, IPU_DC_GEN__MASK_EN, 0);  // mask channel is disabled.(mask channel can associated with different IDMAC channels)
	ipu_write_field(IPU, IPU_DC_GEN__DC_CH5_TYPE, 0);  // alternate sync or asyn flow
	ipu_write_field(IPU, IPU_DC_GEN__SYNC_1_6, 2);  //DC channel #1 handles sync flow

	//DISP_CONF  
	ipu_write_field(IPU, IPU_DC_DISP_CONF1_1__ADDR_INCREMENT_1, 0);  //automatical address increase by 1
	ipu_write_field(IPU, IPU_DC_DISP_CONF1_1__DISP_TYP_1, 2);  //paralel display without byte enable
	ipu_write_field(IPU, IPU_DC_DISP_CONF2_1__SL_1, DISPLAY_WIDTH);  //stride line

	//DC_UGDE 
	ipu_write_field(IPU, IPU_DC_UGDE1_0__NF_NL_1, 0);  //NL->0,NF->1,NFIELD->2
	ipu_write_field(IPU, IPU_DC_UGDE1_0__AUTORESTART_1, 0);  //no autorestart
	ipu_write_field(IPU, IPU_DC_UGDE1_0__ODD_EN_1, 0);  // disable 'odd'
	ipu_write_field(IPU, IPU_DC_UGDE1_0__COD_ODD_START_1, 1);  //words 1 1st part
	ipu_write_field(IPU, IPU_DC_UGDE1_0__COD_EV_START_1, 5);  //word 2 2nd part
	ipu_write_field(IPU, IPU_DC_UGDE1_0__COD_EV_PRIORITY_1,  1);  //enabled. all others are disabled.
#ifdef IPU_USE_DC_CHANNEL
	ipu_write_field(IPU, IPU_DC_UGDE1_0__ID_CODED_1, 0x1);  //DC_CHANNEL_DC_SYNC_OR_ASYNC
#else
	ipu_write_field(IPU, IPU_DC_UGDE1_0__ID_CODED_1, 0x3);  //DC_CHANNEL_DP_PRIMARY
#endif
	ipu_write_field(IPU, IPU_DC_UGDE1_1__STEP_1, 0);  //every data
	ipu_write_field(IPU, IPU_DC_UGDE1_2__OFFSET_DT_1, 0);  //no offset
	ipu_write_field(IPU, IPU_DC_UGDE1_3__STEP_REPEAT_1, (DISPLAY_WIDTH - 1));

	//DC_MAP, same as _ipu_init_dc_mappings() in kernel
	// config DC mapping point 1, IPU_PIX_FMT_RGB24
	ipu_write_field(IPU, IPU_DC_MAP_CONF_15__MD_OFFSET_0, 7);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_15__MD_MASK_0, 0xFF);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_15__MD_OFFSET_1, 15);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_15__MD_MASK_1, 0xFF);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_16__MD_OFFSET_2, 23);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_16__MD_MASK_2, 0xFF);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_0__MAPPING_PNTR_BYTE0_0, 0);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_0__MAPPING_PNTR_BYTE1_0, 1);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_0__MAPPING_PNTR_BYTE2_0, 2);

	// config DC mapping point 2, IPU_PIX_FMT_RGB666
	ipu_write_field(IPU, IPU_DC_MAP_CONF_16__MD_OFFSET_3, 5);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_16__MD_MASK_3, 0xFC);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_17__MD_OFFSET_4, 11);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_17__MD_MASK_4, 0xFC);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_17__MD_OFFSET_5, 17);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_17__MD_MASK_5, 0xFC);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_0__MAPPING_PNTR_BYTE0_1, 3);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_0__MAPPING_PNTR_BYTE1_1, 4);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_0__MAPPING_PNTR_BYTE2_1, 5);

	// config DC mapping point 3, IPU_PIX_FMT_YUV444
	ipu_write_field(IPU, IPU_DC_MAP_CONF_18__MD_OFFSET_6, 15);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_18__MD_MASK_6, 0xFF);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_18__MD_OFFSET_7, 23);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_18__MD_MASK_7, 0xFF);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_19__MD_OFFSET_8, 7);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_19__MD_MASK_8, 0xFF);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_1__MAPPING_PNTR_BYTE0_2, 6);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_1__MAPPING_PNTR_BYTE1_2, 7);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_1__MAPPING_PNTR_BYTE2_2, 8);

	// config DC mapping point 4, IPU_PIX_FMT_RGB565
	ipu_write_field(IPU, IPU_DC_MAP_CONF_19__MD_OFFSET_9, 4);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_19__MD_MASK_9, 0xF8);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_20__MD_OFFSET_10, 10);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_20__MD_MASK_10, 0xFC);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_20__MD_OFFSET_11, 15);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_20__MD_MASK_11, 0xF8);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_1__MAPPING_PNTR_BYTE0_3, 9);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_1__MAPPING_PNTR_BYTE1_3, 10);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_1__MAPPING_PNTR_BYTE2_3, 11);

	// config DC mapping point 5, IPU_PIX_FMT_LVDS666
	ipu_write_field(IPU, IPU_DC_MAP_CONF_21__MD_OFFSET_12, 5);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_21__MD_MASK_12, 0xFC);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_21__MD_OFFSET_13, 13);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_21__MD_MASK_13, 0xFC);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_22__MD_OFFSET_14, 21);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_22__MD_MASK_14, 0xFC);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_2__MAPPING_PNTR_BYTE0_4, 12);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_2__MAPPING_PNTR_BYTE1_4, 13);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_2__MAPPING_PNTR_BYTE2_4, 14);

	// config DC mapping point 6,7, IPU_PIX_FMT_VYUY
	ipu_write_field(IPU, IPU_DC_MAP_CONF_22__MD_OFFSET_15, 7);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_22__MD_MASK_15, 0xFF);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_23__MD_OFFSET_16, 0);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_23__MD_MASK_16, 0x00);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_23__MD_OFFSET_17, 15);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_23__MD_MASK_17, 0xFF);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_2__MAPPING_PNTR_BYTE0_5, 15);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_2__MAPPING_PNTR_BYTE1_5, 16);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_2__MAPPING_PNTR_BYTE2_5, 17);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_24__MD_OFFSET_18, 0);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_24__MD_MASK_18, 0x00);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_24__MD_OFFSET_19, 7);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_24__MD_MASK_19, 0xFF);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_25__MD_OFFSET_20, 15);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_25__MD_MASK_20, 0xFF);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_3__MAPPING_PNTR_BYTE0_6, 18);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_3__MAPPING_PNTR_BYTE1_6, 19);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_3__MAPPING_PNTR_BYTE2_6, 20);

	// config DC mapping point 8,9, IPU_PIX_FMT_UYVY
	ipu_write_field(IPU, IPU_DC_MAP_CONF_3__MAPPING_PNTR_BYTE0_7, 18);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_3__MAPPING_PNTR_BYTE1_7, 19);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_3__MAPPING_PNTR_BYTE2_7, 20);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_4__MAPPING_PNTR_BYTE0_8, 15);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_4__MAPPING_PNTR_BYTE1_8, 16);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_4__MAPPING_PNTR_BYTE2_8, 17);

	// config DC mapping point 10,11, IPU_PIX_FMT_YUYV
	ipu_write_field(IPU, IPU_DC_MAP_CONF_4__MAPPING_PNTR_BYTE0_9, 17);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_4__MAPPING_PNTR_BYTE1_9, 16);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_4__MAPPING_PNTR_BYTE2_9, 15);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_5__MAPPING_PNTR_BYTE0_10, 16);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_5__MAPPING_PNTR_BYTE1_10, 17);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_5__MAPPING_PNTR_BYTE2_10, 15);

	// config DC mapping point 12,13, IPU_PIX_FMT_YVYU
	ipu_write_field(IPU, IPU_DC_MAP_CONF_5__MAPPING_PNTR_BYTE0_11, 16);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_5__MAPPING_PNTR_BYTE1_11, 17);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_5__MAPPING_PNTR_BYTE2_11, 15);

	ipu_write_field(IPU, IPU_DC_MAP_CONF_6__MAPPING_PNTR_BYTE0_12, 17);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_6__MAPPING_PNTR_BYTE1_12, 16);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_6__MAPPING_PNTR_BYTE2_12, 15);

	// config DC mapping point 14, IPU_PIX_FMT_GBR24, IPU_PIX_FMT_VYU444
	ipu_write_field(IPU, IPU_DC_MAP_CONF_6__MAPPING_PNTR_BYTE0_13, 2);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_6__MAPPING_PNTR_BYTE1_13, 0);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_6__MAPPING_PNTR_BYTE2_13, 1);

	// config DC mapping point 15, IPU_PIX_FMT_BGR24
	ipu_write_field(IPU, IPU_DC_MAP_CONF_7__MAPPING_PNTR_BYTE0_14, 2);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_7__MAPPING_PNTR_BYTE1_14, 1);
	ipu_write_field(IPU, IPU_DC_MAP_CONF_7__MAPPING_PNTR_BYTE2_14, 0);
}
#endif

void di_sync_config(
                int ipu_num, 
                int di, 
                int pointer,
                int run_value_m1,           
                int run_resolution,         
                int offset_value,           
                int offset_resolution,      
                int cnt_auto_reload,        
                int step_repeat,           
                int cnt_clr_sel,            
                int cnt_polarity_gen_en,    
                int cnt_polarity_trigger_sel,
                int cnt_polarity_clr_sel,   
                int cnt_up,                 
                int cnt_down, 
                int gentime_sel
                )
{
    if(di == 0){
        switch(pointer){
            case  1:{
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_1__DI0_RUN_VALUE_M1_1, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_1__DI0_RUN_RESOLUTION_1, run_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_1__DI0_OFFSET_VALUE_1, offset_value);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_1__DI0_OFFSET_RESOLUTION_1, offset_resolution);                                                                          
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_1__DI0_CNT_POLARITY_GEN_EN_1, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_1__DI0_CNT_AUTO_RELOAD_1, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_1__DI0_CNT_CLR_SEL_1, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_1__DI0_CNT_DOWN_1, cnt_down);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_1__DI0_CNT_POLARITY_TRIGGER_SEL_1, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_1__DI0_CNT_POLARITY_CLR_SEL_1, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_1__DI0_CNT_UP_1, cnt_up);                                                                             
                ipu_write_field(ipu_num, IPU_DI0_STP_REP_1__DI0_STEP_REPEAT_1, step_repeat);
                break;
            }
            case  2:{
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_2__DI0_RUN_VALUE_M1_2, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_2__DI0_RUN_RESOLUTION_2, run_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_2__DI0_OFFSET_VALUE_2, offset_value);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_2__DI0_OFFSET_RESOLUTION_2, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_2__DI0_CNT_POLARITY_GEN_EN_2, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_2__DI0_CNT_AUTO_RELOAD_2, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_2__DI0_CNT_CLR_SEL_2, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_2__DI0_CNT_DOWN_2, cnt_down);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_2__DI0_CNT_POLARITY_TRIGGER_SEL_2, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_2__DI0_CNT_POLARITY_CLR_SEL_2, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_2__DI0_CNT_UP_2, cnt_up);
                ipu_write_field(ipu_num, IPU_DI0_STP_REP_1__DI0_STEP_REPEAT_2, step_repeat);
                break;
            }
            case  3:{
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_3__DI0_RUN_VALUE_M1_3, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_3__DI0_RUN_RESOLUTION_3, run_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_3__DI0_OFFSET_VALUE_3, offset_value);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_3__DI0_OFFSET_RESOLUTION_3, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_3__DI0_CNT_POLARITY_GEN_EN_3, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_3__DI0_CNT_AUTO_RELOAD_3, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_3__DI0_CNT_CLR_SEL_3, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_3__DI0_CNT_DOWN_3, cnt_down);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_3__DI0_CNT_POLARITY_TRIGGER_SEL_3, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_3__DI0_CNT_POLARITY_CLR_SEL_3, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_3__DI0_CNT_UP_3, cnt_up);
                ipu_write_field(ipu_num, IPU_DI0_STP_REP_2__DI0_STEP_REPEAT_3, step_repeat);
                break;
            }
            case  4:{
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_4__DI0_RUN_VALUE_M1_4, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_4__DI0_RUN_RESOLUTION_4, run_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_4__DI0_OFFSET_VALUE_4, offset_value);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_4__DI0_OFFSET_RESOLUTION_4, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_4__DI0_CNT_POLARITY_GEN_EN_4, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_4__DI0_CNT_AUTO_RELOAD_4, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_4__DI0_CNT_CLR_SEL_4, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_4__DI0_CNT_DOWN_4, cnt_down);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_4__DI0_CNT_POLARITY_TRIGGER_SEL_4, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_4__DI0_CNT_POLARITY_CLR_SEL_4, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_4__DI0_CNT_UP_4, cnt_up);
                ipu_write_field(ipu_num, IPU_DI0_STP_REP_2__DI0_STEP_REPEAT_4, step_repeat);
                break;
            }
            case  5:{
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_5__DI0_RUN_VALUE_M1_5, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_5__DI0_RUN_RESOLUTION_5, run_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_5__DI0_OFFSET_VALUE_5, offset_value);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_5__DI0_OFFSET_RESOLUTION_5, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_5__DI0_CNT_POLARITY_GEN_EN_5, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_5__DI0_CNT_AUTO_RELOAD_5, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_5__DI0_CNT_CLR_SEL_5, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_5__DI0_CNT_DOWN_5, cnt_down);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_5__DI0_CNT_POLARITY_TRIGGER_SEL_5, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_5__DI0_CNT_POLARITY_CLR_SEL_5, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_5__DI0_CNT_UP_5, cnt_up);
                ipu_write_field(ipu_num, IPU_DI0_STP_REP_3__DI0_STEP_REPEAT_5, step_repeat);
                break;
            }
            case  6:{
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_6__DI0_RUN_VALUE_M1_6, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_6__DI0_RUN_RESOLUTION_6, run_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_6__DI0_OFFSET_VALUE_6, offset_value);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_6__DI0_OFFSET_RESOLUTION_6, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_6__DI0_CNT_POLARITY_GEN_EN_6, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_6__DI0_CNT_AUTO_RELOAD_6, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_6__DI0_CNT_CLR_SEL_6, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_6__DI0_CNT_DOWN_6, cnt_down);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_6__DI0_CNT_POLARITY_TRIGGER_SEL_6, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_6__DI0_CNT_POLARITY_CLR_SEL_6, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_6__DI0_CNT_UP_6, cnt_up);
                ipu_write_field(ipu_num, IPU_DI0_STP_REP_3__DI0_STEP_REPEAT_6, step_repeat);
                break;
            }
            case  7:{
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_7__DI0_RUN_VALUE_M1_7, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_7__DI0_RUN_RESOLUTION_7, run_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_7__DI0_OFFSET_VALUE_7, offset_value);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_7__DI0_OFFSET_RESOLUTION_7, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_7__DI0_CNT_POLARITY_GEN_EN_7, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_7__DI0_CNT_AUTO_RELOAD_7, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_7__DI0_CNT_CLR_SEL_7, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_7__DI0_CNT_DOWN_7, cnt_down);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_7__DI0_CNT_POLARITY_TRIGGER_SEL_7, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_7__DI0_CNT_POLARITY_CLR_SEL_7, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_7__DI0_CNT_UP_7, cnt_up);
                ipu_write_field(ipu_num, IPU_DI0_STP_REP_4__DI0_STEP_REPEAT_7, step_repeat);
                break;
            }
            case  8:{
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_8__DI0_RUN_VALUE_M1_8, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_8__DI0_RUN_RESOLUTION_8, run_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_8__DI0_OFFSET_VALUE_8, offset_value);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_8__DI0_OFFSET_RESOLUTION_8, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_8__DI0_CNT_POLARITY_GEN_EN_8, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_8__DI0_CNT_AUTO_RELOAD_8, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_8__DI0_CNT_CLR_SEL_8, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_8__DI0_CNT_DOWN_8, cnt_down);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_8__DI0_CNT_POLARITY_TRIGGER_SEL_8, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_8__DI0_CNT_POLARITY_CLR_SEL_8, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_8__DI0_CNT_UP_8, cnt_up);
                ipu_write_field(ipu_num, IPU_DI0_STP_REP_4__DI0_STEP_REPEAT_8, step_repeat);
                break;
            }
            case  9:{
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_9__DI0_RUN_VALUE_M1_9, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_9__DI0_RUN_RESOLUTION_9, run_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_9__DI0_OFFSET_VALUE_9, offset_value);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN0_9__DI0_OFFSET_RESOLUTION_9, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_9__DI0_GENTIME_SEL_9, gentime_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_9__DI0_CNT_AUTO_RELOAD_9, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_9__DI0_CNT_CLR_SEL_9, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_9__DI0_CNT_DOWN_9, cnt_down);
                ipu_write_field(ipu_num, IPU_DI0_SW_GEN1_9__DI0_CNT_UP_9, cnt_up);
                ipu_write_field(ipu_num, IPU_DI0_STP_REP_9__DI0_STEP_REPEAT_9, step_repeat);
                break;
            }
        }
    }
    else
    {
        switch(pointer){
            case  1:{
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_1__DI1_RUN_VALUE_M1_1, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_1__DI1_RUN_RESOLUTION_1, run_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_1__DI1_OFFSET_VALUE_1, offset_value);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_1__DI1_OFFSET_RESOLUTION_1, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_1__DI1_CNT_POLARITY_GEN_EN_1, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_1__DI1_CNT_AUTO_RELOAD_1, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_1__DI1_CNT_CLR_SEL_1, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_1__DI1_CNT_DOWN_1, cnt_down);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_1__DI1_CNT_POLARITY_TRIGGER_SEL_1, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_1__DI1_CNT_POLARITY_CLR_SEL_1, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_1__DI1_CNT_UP_1, cnt_up);
                ipu_write_field(ipu_num, IPU_DI1_STP_REP_1__DI1_STEP_REPEAT_1, step_repeat);
                break;
            }
            case  2:{
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_2__DI1_RUN_VALUE_M1_2, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_2__DI1_RUN_RESOLUTION_2, run_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_2__DI1_OFFSET_VALUE_2, offset_value);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_2__DI1_OFFSET_RESOLUTION_2, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_2__DI1_CNT_POLARITY_GEN_EN_2, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_2__DI1_CNT_AUTO_RELOAD_2, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_2__DI1_CNT_CLR_SEL_2, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_2__DI1_CNT_DOWN_2, cnt_down);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_2__DI1_CNT_POLARITY_TRIGGER_SEL_2, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_2__DI1_CNT_POLARITY_CLR_SEL_2, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_2__DI1_CNT_UP_2, cnt_up);
                ipu_write_field(ipu_num, IPU_DI1_STP_REP_1__DI1_STEP_REPEAT_2, step_repeat);
                break;
            }
            case  3:{
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_3__DI1_RUN_VALUE_M1_3, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_3__DI1_RUN_RESOLUTION_3, run_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_3__DI1_OFFSET_VALUE_3, offset_value);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_3__DI1_OFFSET_RESOLUTION_3, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_3__DI1_CNT_POLARITY_GEN_EN_3, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_3__DI1_CNT_AUTO_RELOAD_3, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_3__DI1_CNT_CLR_SEL_3, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_3__DI1_CNT_DOWN_3, cnt_down);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_3__DI1_CNT_POLARITY_TRIGGER_SEL_3, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_3__DI1_CNT_POLARITY_CLR_SEL_3, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_3__DI1_CNT_UP_3, cnt_up);
                ipu_write_field(ipu_num, IPU_DI1_STP_REP_2__DI1_STEP_REPEAT_3, step_repeat);
                break;
            }
            case  4:{
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_4__DI1_RUN_VALUE_M1_4, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_4__DI1_RUN_RESOLUTION_4, run_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_4__DI1_OFFSET_VALUE_4, offset_value);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_4__DI1_OFFSET_RESOLUTION_4, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_4__DI1_CNT_POLARITY_GEN_EN_4, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_4__DI1_CNT_AUTO_RELOAD_4, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_4__DI1_CNT_CLR_SEL_4, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_4__DI1_CNT_DOWN_4, cnt_down);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_4__DI1_CNT_POLARITY_TRIGGER_SEL_4, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_4__DI1_CNT_POLARITY_CLR_SEL_4, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_4__DI1_CNT_UP_4, cnt_up);
                ipu_write_field(ipu_num, IPU_DI1_STP_REP_2__DI1_STEP_REPEAT_4, step_repeat);
                break;
            }
            case  5:{
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_5__DI1_RUN_VALUE_M1_5, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_5__DI1_RUN_RESOLUTION_5, run_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_5__DI1_OFFSET_VALUE_5, offset_value);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_5__DI1_OFFSET_RESOLUTION_5, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_5__DI1_CNT_POLARITY_GEN_EN_5, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_5__DI1_CNT_AUTO_RELOAD_5, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_5__DI1_CNT_CLR_SEL_5, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_5__DI1_CNT_DOWN_5, cnt_down);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_5__DI1_CNT_POLARITY_TRIGGER_SEL_5, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_5__DI1_CNT_POLARITY_CLR_SEL_5, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_5__DI1_CNT_UP_5, cnt_up);
                ipu_write_field(ipu_num, IPU_DI1_STP_REP_3__DI1_STEP_REPEAT_5, step_repeat);
                break;
            }
            case  6:{
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_6__DI1_RUN_VALUE_M1_6, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_6__DI1_RUN_RESOLUTION_6, run_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_6__DI1_OFFSET_VALUE_6, offset_value);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_6__DI1_OFFSET_RESOLUTION_6, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_6__DI1_CNT_POLARITY_GEN_EN_6, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_6__DI1_CNT_AUTO_RELOAD_6, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_6__DI1_CNT_CLR_SEL_6, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_6__DI1_CNT_DOWN_6, cnt_down);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_6__DI1_CNT_POLARITY_TRIGGER_SEL_6, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_6__DI1_CNT_POLARITY_CLR_SEL_6, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_6__DI1_CNT_UP_6, cnt_up);
                ipu_write_field(ipu_num, IPU_DI1_STP_REP_3__DI1_STEP_REPEAT_6, step_repeat);
                break;
            }
            case  7:{
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_7__DI1_RUN_VALUE_M1_7, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_7__DI1_RUN_RESOLUTION_7, run_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_7__DI1_OFFSET_VALUE_7, offset_value);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_7__DI1_OFFSET_RESOLUTION_7, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_7__DI1_CNT_POLARITY_GEN_EN_7, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_7__DI1_CNT_AUTO_RELOAD_7, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_7__DI1_CNT_CLR_SEL_7, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_7__DI1_CNT_DOWN_7, cnt_down);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_7__DI1_CNT_POLARITY_TRIGGER_SEL_7, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_7__DI1_CNT_POLARITY_CLR_SEL_7, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_7__DI1_CNT_UP_7, cnt_up);
                ipu_write_field(ipu_num, IPU_DI1_STP_REP_4__DI1_STEP_REPEAT_7, step_repeat);
            }
            case  8:{
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_8__DI1_RUN_VALUE_M1_8, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_8__DI1_RUN_RESOLUTION_8, run_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_8__DI1_OFFSET_VALUE_8, offset_value);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_8__DI1_OFFSET_RESOLUTION_8, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_8__DI1_CNT_POLARITY_GEN_EN_8, cnt_polarity_gen_en);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_8__DI1_CNT_AUTO_RELOAD_8, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_8__DI1_CNT_CLR_SEL_8, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_8__DI1_CNT_DOWN_8, cnt_down);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_8__DI1_CNT_POLARITY_TRIGGER_SEL_8, cnt_polarity_trigger_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_8__DI1_CNT_POLARITY_CLR_SEL_8, cnt_polarity_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_8__DI1_CNT_UP_8, cnt_up);
                ipu_write_field(ipu_num, IPU_DI1_STP_REP_4__DI1_STEP_REPEAT_8, step_repeat);
                break;
            }
            case  9:{
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_9__DI1_RUN_VALUE_M1_9, run_value_m1);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_9__DI1_RUN_RESOLUTION_9, run_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_9__DI1_OFFSET_VALUE_9, offset_value);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN0_9__DI1_OFFSET_RESOLUTION_9, offset_resolution);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_9__DI1_GENTIME_SEL_9, gentime_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_9__DI1_CNT_AUTO_RELOAD_9, cnt_auto_reload);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_9__DI1_CNT_CLR_SEL_9, cnt_clr_sel);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_9__DI1_CNT_DOWN_9, cnt_down);
                ipu_write_field(ipu_num, IPU_DI1_SW_GEN1_9__DI1_CNT_UP_9, cnt_up);
                ipu_write_field(ipu_num, IPU_DI1_STP_REP_9__DI1_STEP_REPEAT_9, step_repeat);
                break;
            }
        }
    }
}

void di_pointer_config(int ipu_num, int di, int pointer, int access, int component, int cst, int pt0, int pt1, int pt2, int pt3, int pt4, int pt5, int pt6)
{
    if(di == 0){
        switch(pointer){
            case  0:{
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_0__DI0_ACCESS_SIZE_0,access);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_0__DI0_COMPONNENT_SIZE_0,component);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_0__DI0_CST_0,cst);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_0__DI0_PT_0_0,pt0);      
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_0__DI0_PT_1_0,pt1);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_0__DI0_PT_2_0,pt2);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_0__DI0_PT_3_0,pt3);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_0__DI0_PT_4_0,pt4);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_0__DI0_PT_5_0,pt5);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_0__DI0_PT_6_0,pt6);
                break;
            }
            case  1:{
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_1__DI0_ACCESS_SIZE_1,access);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_1__DI0_COMPONNENT_SIZE_1,component);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_1__DI0_CST_1,cst);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_1__DI0_PT_0_1,pt0);      
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_1__DI0_PT_1_1,pt1);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_1__DI0_PT_2_1,pt2);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_1__DI0_PT_3_1,pt3);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_1__DI0_PT_4_1,pt4);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_1__DI0_PT_5_1,pt5);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_1__DI0_PT_6_1,pt6);
                break;
            }
            case  2:{
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_2__DI0_ACCESS_SIZE_2,access);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_2__DI0_COMPONNENT_SIZE_2,component);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_2__DI0_CST_2,cst);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_2__DI0_PT_0_2,pt0);      
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_2__DI0_PT_1_2,pt1);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_2__DI0_PT_2_2,pt2);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_2__DI0_PT_3_2,pt3);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_2__DI0_PT_4_2,pt4);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_2__DI0_PT_5_2,pt5);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_2__DI0_PT_6_2,pt6);
                break;
            }
            case  3:{
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_3__DI0_ACCESS_SIZE_3,access);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_3__DI0_COMPONNENT_SIZE_3,component);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_3__DI0_CST_3,cst);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_3__DI0_PT_0_3,pt0);      
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_3__DI0_PT_1_3,pt1);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_3__DI0_PT_2_3,pt2);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_3__DI0_PT_3_3,pt3);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_3__DI0_PT_4_3,pt4);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_3__DI0_PT_5_3,pt5);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_3__DI0_PT_6_3,pt6);
                break;
            }
            case  4:{
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_4__DI0_ACCESS_SIZE_4,access);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_4__DI0_COMPONNENT_SIZE_4,component);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_4__DI0_CST_4,cst);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_4__DI0_PT_0_4,pt0);      
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_4__DI0_PT_1_4,pt1);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_4__DI0_PT_2_4,pt2);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_4__DI0_PT_3_4,pt3);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_4__DI0_PT_4_4,pt4);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_4__DI0_PT_5_4,pt5);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_4__DI0_PT_6_4,pt6);
                break;
            }
            case  5:{
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_5__DI0_ACCESS_SIZE_5,access);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_5__DI0_COMPONNENT_SIZE_5,component);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_5__DI0_CST_5,cst);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_5__DI0_PT_0_5,pt0);      
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_5__DI0_PT_1_5,pt1);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_5__DI0_PT_2_5,pt2);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_5__DI0_PT_3_5,pt3);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_5__DI0_PT_4_5,pt4);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_5__DI0_PT_5_5,pt5);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_5__DI0_PT_6_5,pt6);
                break;
            }
            case  6:{
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_6__DI0_ACCESS_SIZE_6,access);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_6__DI0_COMPONNENT_SIZE_6,component);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_6__DI0_CST_6,cst);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_6__DI0_PT_0_6,pt0);      
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_6__DI0_PT_1_6,pt1);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_6__DI0_PT_2_6,pt2);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_6__DI0_PT_3_6,pt3);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_6__DI0_PT_4_6,pt4);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_6__DI0_PT_5_6,pt5);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_6__DI0_PT_6_6,pt6);
                break;
            }
            case  7:{
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_7__DI0_ACCESS_SIZE_7,access);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_7__DI0_COMPONNENT_SIZE_7,component);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_7__DI0_CST_7,cst);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_7__DI0_PT_0_7,pt0);      
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_7__DI0_PT_1_7,pt1);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_7__DI0_PT_2_7,pt2);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_7__DI0_PT_3_7,pt3);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_7__DI0_PT_4_7,pt4);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_7__DI0_PT_5_7,pt5);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_7__DI0_PT_6_7,pt6);
                break;
            }
            case  8:{
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_8__DI0_ACCESS_SIZE_8,access);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_8__DI0_COMPONNENT_SIZE_8,component);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_8__DI0_CST_8,cst);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_8__DI0_PT_0_8,pt0);      
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_8__DI0_PT_1_8,pt1);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_8__DI0_PT_2_8,pt2);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_8__DI0_PT_3_8,pt3);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_8__DI0_PT_4_8,pt4);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_8__DI0_PT_5_8,pt5);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_8__DI0_PT_6_8,pt6);
                break;
            }
            case  9:{
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_9__DI0_ACCESS_SIZE_9,access);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_9__DI0_COMPONNENT_SIZE_9,component);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_9__DI0_CST_9,cst);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_9__DI0_PT_0_9,pt0);      
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_9__DI0_PT_1_9,pt1);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_9__DI0_PT_2_9,pt2);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_9__DI0_PT_3_9,pt3);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_9__DI0_PT_4_9,pt4);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_9__DI0_PT_5_9,pt5);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_9__DI0_PT_6_9,pt6);
                break;
            }
            case  10:{
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_10__DI0_ACCESS_SIZE_10,access);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_10__DI0_COMPONNENT_SIZE_10,component);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_10__DI0_CST_10,cst);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_10__DI0_PT_0_10,pt0);      
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_10__DI0_PT_1_10,pt1);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_10__DI0_PT_2_10,pt2);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_10__DI0_PT_3_10,pt3);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_10__DI0_PT_4_10,pt4);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_10__DI0_PT_5_10,pt5);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_10__DI0_PT_6_10,pt6);
                break;
            }
            case  11:{
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_11__DI0_ACCESS_SIZE_11,access);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_11__DI0_COMPONNENT_SIZE_11,component);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_11__DI0_CST_11,cst);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_11__DI0_PT_0_11,pt0);      
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_11__DI0_PT_1_11,pt1);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_11__DI0_PT_2_11,pt2);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_11__DI0_PT_3_11,pt3);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_11__DI0_PT_4_11,pt4);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_11__DI0_PT_5_11,pt5);
                ipu_write_field(ipu_num, IPU_DI0_DW_GEN_11__DI0_PT_6_11,pt6);
                break;
            }
        }
    }
    else {
        switch(pointer){
            case  0:{
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_0__DI1_ACCESS_SIZE_0,access);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_0__DI1_COMPONNENT_SIZE_0,component);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_0__DI1_CST_0,cst);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_0__DI1_PT_0_0,pt0);      
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_0__DI1_PT_1_0,pt1);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_0__DI1_PT_2_0,pt2);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_0__DI1_PT_3_0,pt3);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_0__DI1_PT_4_0,pt4);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_0__DI1_PT_5_0,pt5);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_0__DI1_PT_6_0,pt6);
                break;
            }
            case  1:{
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_1__DI1_ACCESS_SIZE_1,access);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_1__DI1_COMPONNENT_SIZE_1,component);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_1__DI1_CST_1,cst);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_1__DI1_PT_0_1,pt0);      
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_1__DI1_PT_1_1,pt1);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_1__DI1_PT_2_1,pt2);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_1__DI1_PT_3_1,pt3);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_1__DI1_PT_4_1,pt4);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_1__DI1_PT_5_1,pt5);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_1__DI1_PT_6_1,pt6);
                break;
            }
            case  2:{
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_2__DI1_ACCESS_SIZE_2,access);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_2__DI1_COMPONNENT_SIZE_2,component);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_2__DI1_CST_2,cst);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_2__DI1_PT_0_2,pt0);      
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_2__DI1_PT_1_2,pt1);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_2__DI1_PT_2_2,pt2);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_2__DI1_PT_3_2,pt3);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_2__DI1_PT_4_2,pt4);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_2__DI1_PT_5_2,pt5);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_2__DI1_PT_6_2,pt6);
                break;
            }
            case  3:{
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_3__DI1_ACCESS_SIZE_3,access);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_3__DI1_COMPONNENT_SIZE_3,component);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_3__DI1_CST_3,cst);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_3__DI1_PT_0_3,pt0);      
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_3__DI1_PT_1_3,pt1);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_3__DI1_PT_2_3,pt2);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_3__DI1_PT_3_3,pt3);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_3__DI1_PT_4_3,pt4);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_3__DI1_PT_5_3,pt5);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_3__DI1_PT_6_3,pt6);
                break;
            }
            case  4:{
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_4__DI1_ACCESS_SIZE_4,access);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_4__DI1_COMPONNENT_SIZE_4,component);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_4__DI1_CST_4,cst);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_4__DI1_PT_0_4,pt0);      
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_4__DI1_PT_1_4,pt1);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_4__DI1_PT_2_4,pt2);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_4__DI1_PT_3_4,pt3);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_4__DI1_PT_4_4,pt4);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_4__DI1_PT_5_4,pt5);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_4__DI1_PT_6_4,pt6);
                break;
            }
            case  5:{
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_5__DI1_ACCESS_SIZE_5,access);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_5__DI1_COMPONNENT_SIZE_5,component);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_5__DI1_CST_5,cst);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_5__DI1_PT_0_5,pt0);      
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_5__DI1_PT_1_5,pt1);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_5__DI1_PT_2_5,pt2);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_5__DI1_PT_3_5,pt3);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_5__DI1_PT_4_5,pt4);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_5__DI1_PT_5_5,pt5);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_5__DI1_PT_6_5,pt6);
                break;
            }
            case  6:{
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_6__DI1_ACCESS_SIZE_6,access);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_6__DI1_COMPONNENT_SIZE_6,component);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_6__DI1_CST_6,cst);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_6__DI1_PT_0_6,pt0);      
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_6__DI1_PT_1_6,pt1);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_6__DI1_PT_2_6,pt2);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_6__DI1_PT_3_6,pt3);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_6__DI1_PT_4_6,pt4);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_6__DI1_PT_5_6,pt5);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_6__DI1_PT_6_6,pt6);
                break;
            }
            case  7:{
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_7__DI1_ACCESS_SIZE_7,access);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_7__DI1_COMPONNENT_SIZE_7,component);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_7__DI1_CST_7,cst);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_7__DI1_PT_0_7,pt0);      
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_7__DI1_PT_1_7,pt1);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_7__DI1_PT_2_7,pt2);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_7__DI1_PT_3_7,pt3);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_7__DI1_PT_4_7,pt4);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_7__DI1_PT_5_7,pt5);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_7__DI1_PT_6_7,pt6);
                break;
            }
            case  8:{
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_8__DI1_ACCESS_SIZE_8,access);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_8__DI1_COMPONNENT_SIZE_8,component);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_8__DI1_CST_8,cst);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_8__DI1_PT_0_8,pt0);      
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_8__DI1_PT_1_8,pt1);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_8__DI1_PT_2_8,pt2);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_8__DI1_PT_3_8,pt3);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_8__DI1_PT_4_8,pt4);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_8__DI1_PT_5_8,pt5);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_8__DI1_PT_6_8,pt6);
                break;
            }
            case  9:{
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_9__DI1_ACCESS_SIZE_9,access);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_9__DI1_COMPONNENT_SIZE_9,component);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_9__DI1_CST_9,cst);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_9__DI1_PT_0_9,pt0);      
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_9__DI1_PT_1_9,pt1);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_9__DI1_PT_2_9,pt2);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_9__DI1_PT_3_9,pt3);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_9__DI1_PT_4_9,pt4);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_9__DI1_PT_5_9,pt5);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_9__DI1_PT_6_9,pt6);
                break;
            }
            case  10:{
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_10__DI1_ACCESS_SIZE_10,access);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_10__DI1_COMPONNENT_SIZE_10,component);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_10__DI1_CST_10,cst);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_10__DI1_PT_0_10,pt0);      
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_10__DI1_PT_1_10,pt1);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_10__DI1_PT_2_10,pt2);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_10__DI1_PT_3_10,pt3);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_10__DI1_PT_4_10,pt4);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_10__DI1_PT_5_10,pt5);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_10__DI1_PT_6_10,pt6);
                break;
            }
            case  11:{
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_11__DI1_ACCESS_SIZE_11,access);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_11__DI1_COMPONNENT_SIZE_11,component);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_11__DI1_CST_11,cst);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_11__DI1_PT_0_11,pt0);      
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_11__DI1_PT_1_11,pt1);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_11__DI1_PT_2_11,pt2);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_11__DI1_PT_3_11,pt3);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_11__DI1_PT_4_11,pt4);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_11__DI1_PT_5_11,pt5);
                ipu_write_field(ipu_num, IPU_DI1_DW_GEN_11__DI1_PT_6_11,pt6);
                break;
            }
        }
    }
}

void di_up_down_config(int ipu_num, int di, int pointer, int set, int up, int down)
{
    if(di == 0){
        switch(pointer){
        case  0:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_0__DI0_DATA_CNT_UP0_0,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_0__DI0_DATA_CNT_DOWN0_0,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_0__DI0_DATA_CNT_UP1_0,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_0__DI0_DATA_CNT_DOWN1_0,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_0__DI0_DATA_CNT_UP2_0,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_0__DI0_DATA_CNT_DOWN2_0,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_0__DI0_DATA_CNT_UP3_0,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_0__DI0_DATA_CNT_DOWN3_0,down);
                }
                break;}
        case  1:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_1__DI0_DATA_CNT_UP0_1,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_1__DI0_DATA_CNT_DOWN0_1,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_1__DI0_DATA_CNT_UP1_1,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_1__DI0_DATA_CNT_DOWN1_1,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_1__DI0_DATA_CNT_UP2_1,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_1__DI0_DATA_CNT_DOWN2_1,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_1__DI0_DATA_CNT_UP3_1,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_1__DI0_DATA_CNT_DOWN3_1,down);
                }
                break;}
        case  2:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_2__DI0_DATA_CNT_UP0_2,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_2__DI0_DATA_CNT_DOWN0_2,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_2__DI0_DATA_CNT_UP1_2,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_2__DI0_DATA_CNT_DOWN1_2,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_2__DI0_DATA_CNT_UP2_2,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_2__DI0_DATA_CNT_DOWN2_2,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_2__DI0_DATA_CNT_UP3_2,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_2__DI0_DATA_CNT_DOWN3_2,down);
                }
                break;}
        case  3:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_3__DI0_DATA_CNT_UP0_3,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_3__DI0_DATA_CNT_DOWN0_3,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_3__DI0_DATA_CNT_UP1_3,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_3__DI0_DATA_CNT_DOWN1_3,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_3__DI0_DATA_CNT_UP2_3,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_3__DI0_DATA_CNT_DOWN2_3,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_3__DI0_DATA_CNT_UP3_3,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_3__DI0_DATA_CNT_DOWN3_3,down);
                }
                break;}
        case  4:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_4__DI0_DATA_CNT_UP0_4,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_4__DI0_DATA_CNT_DOWN0_4,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_4__DI0_DATA_CNT_UP1_4,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_4__DI0_DATA_CNT_DOWN1_4,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_4__DI0_DATA_CNT_UP2_4,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_4__DI0_DATA_CNT_DOWN2_4,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_4__DI0_DATA_CNT_UP3_4,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_4__DI0_DATA_CNT_DOWN3_4,down);
                }
                break;}
        case  5:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_5__DI0_DATA_CNT_UP0_5,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_5__DI0_DATA_CNT_DOWN0_5,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_5__DI0_DATA_CNT_UP1_5,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_5__DI0_DATA_CNT_DOWN1_5,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_5__DI0_DATA_CNT_UP2_5,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_5__DI0_DATA_CNT_DOWN2_5,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_5__DI0_DATA_CNT_UP3_5,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_5__DI0_DATA_CNT_DOWN3_5,down);
                }
                break;}
        case  6:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_6__DI0_DATA_CNT_UP0_6,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_6__DI0_DATA_CNT_DOWN0_6,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_6__DI0_DATA_CNT_UP1_6,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_6__DI0_DATA_CNT_DOWN1_6,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_6__DI0_DATA_CNT_UP2_6,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_6__DI0_DATA_CNT_DOWN2_6,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_6__DI0_DATA_CNT_UP3_6,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_6__DI0_DATA_CNT_DOWN3_6,down);
                }
                break;}
        case  7:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_7__DI0_DATA_CNT_UP0_7,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_7__DI0_DATA_CNT_DOWN0_7,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_7__DI0_DATA_CNT_UP1_7,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_7__DI0_DATA_CNT_DOWN1_7,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_7__DI0_DATA_CNT_UP2_7,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_7__DI0_DATA_CNT_DOWN2_7,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_7__DI0_DATA_CNT_UP3_7,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_7__DI0_DATA_CNT_DOWN3_7,down);
                }
                break;}
        case  8:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_8__DI0_DATA_CNT_UP0_8,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_8__DI0_DATA_CNT_DOWN0_8,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_8__DI0_DATA_CNT_UP1_8,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_8__DI0_DATA_CNT_DOWN1_8,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_8__DI0_DATA_CNT_UP2_8,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_8__DI0_DATA_CNT_DOWN2_8,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_8__DI0_DATA_CNT_UP3_8,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_8__DI0_DATA_CNT_DOWN3_8,down);
                }
                break;}
        case  9:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_9__DI0_DATA_CNT_UP0_9,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_9__DI0_DATA_CNT_DOWN0_9,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_9__DI0_DATA_CNT_UP1_9,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_9__DI0_DATA_CNT_DOWN1_9,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_9__DI0_DATA_CNT_UP2_9,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_9__DI0_DATA_CNT_DOWN2_9,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_9__DI0_DATA_CNT_UP3_9,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_9__DI0_DATA_CNT_DOWN3_9,down);
                }
                break;}
        case 10:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_10__DI0_DATA_CNT_UP0_10,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_10__DI0_DATA_CNT_DOWN0_10,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_10__DI0_DATA_CNT_UP1_10,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_10__DI0_DATA_CNT_DOWN1_10,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_10__DI0_DATA_CNT_UP2_10,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_10__DI0_DATA_CNT_DOWN2_10,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_10__DI0_DATA_CNT_UP3_10,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_10__DI0_DATA_CNT_DOWN3_10,down);
                }
                break;}
        case 11:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_11__DI0_DATA_CNT_UP0_11,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET0_11__DI0_DATA_CNT_DOWN0_11,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_11__DI0_DATA_CNT_UP1_11,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET1_11__DI0_DATA_CNT_DOWN1_11,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_11__DI0_DATA_CNT_UP2_11,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET2_11__DI0_DATA_CNT_DOWN2_11,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_11__DI0_DATA_CNT_UP3_11,up);
                    ipu_write_field(ipu_num, IPU_DI0_DW_SET3_11__DI0_DATA_CNT_DOWN3_11,down);
                }
                break;}
		default:{
			    break;}	
        }
    }
    else {
        switch(pointer){
        case  0:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_0__DI1_DATA_CNT_UP0_0,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_0__DI1_DATA_CNT_DOWN0_0,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_0__DI1_DATA_CNT_UP1_0,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_0__DI1_DATA_CNT_DOWN1_0,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_0__DI1_DATA_CNT_UP2_0,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_0__DI1_DATA_CNT_DOWN2_0,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_0__DI1_DATA_CNT_UP3_0,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_0__DI1_DATA_CNT_DOWN3_0,down);
                }
                break;}
        case  1:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_1__DI1_DATA_CNT_UP0_1,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_1__DI1_DATA_CNT_DOWN0_1,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_1__DI1_DATA_CNT_UP1_1,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_1__DI1_DATA_CNT_DOWN1_1,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_1__DI1_DATA_CNT_UP2_1,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_1__DI1_DATA_CNT_DOWN2_1,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_1__DI1_DATA_CNT_UP3_1,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_1__DI1_DATA_CNT_DOWN3_1,down);
                }
                break;}
        case  2:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_2__DI1_DATA_CNT_UP0_2,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_2__DI1_DATA_CNT_DOWN0_2,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_2__DI1_DATA_CNT_UP1_2,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_2__DI1_DATA_CNT_DOWN1_2,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_2__DI1_DATA_CNT_UP2_2,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_2__DI1_DATA_CNT_DOWN2_2,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_2__DI1_DATA_CNT_UP3_2,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_2__DI1_DATA_CNT_DOWN3_2,down);
                }
                break;}
        case  3:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_3__DI1_DATA_CNT_UP0_3,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_3__DI1_DATA_CNT_DOWN0_3,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_3__DI1_DATA_CNT_UP1_3,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_3__DI1_DATA_CNT_DOWN1_3,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_3__DI1_DATA_CNT_UP2_3,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_3__DI1_DATA_CNT_DOWN2_3,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_3__DI1_DATA_CNT_UP3_3,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_3__DI1_DATA_CNT_DOWN3_3,down);
                }
                break;}
        case  4:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_4__DI1_DATA_CNT_UP0_4,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_4__DI1_DATA_CNT_DOWN0_4,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_4__DI1_DATA_CNT_UP1_4,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_4__DI1_DATA_CNT_DOWN1_4,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_4__DI1_DATA_CNT_UP2_4,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_4__DI1_DATA_CNT_DOWN2_4,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_4__DI1_DATA_CNT_UP3_4,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_4__DI1_DATA_CNT_DOWN3_4,down);
                }
                break;}
        case  5:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_5__DI1_DATA_CNT_UP0_5,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_5__DI1_DATA_CNT_DOWN0_5,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_5__DI1_DATA_CNT_UP1_5,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_5__DI1_DATA_CNT_DOWN1_5,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_5__DI1_DATA_CNT_UP2_5,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_5__DI1_DATA_CNT_DOWN2_5,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_5__DI1_DATA_CNT_UP3_5,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_5__DI1_DATA_CNT_DOWN3_5,down);
                }
                break;}
        case  6:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_6__DI1_DATA_CNT_UP0_6,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_6__DI1_DATA_CNT_DOWN0_6,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_6__DI1_DATA_CNT_UP1_6,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_6__DI1_DATA_CNT_DOWN1_6,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_6__DI1_DATA_CNT_UP2_6,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_6__DI1_DATA_CNT_DOWN2_6,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_6__DI1_DATA_CNT_UP3_6,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_6__DI1_DATA_CNT_DOWN3_6,down);
                }
                break;}
        case  7:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_7__DI1_DATA_CNT_UP0_7,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_7__DI1_DATA_CNT_DOWN0_7,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_7__DI1_DATA_CNT_UP1_7,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_7__DI1_DATA_CNT_DOWN1_7,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_7__DI1_DATA_CNT_UP2_7,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_7__DI1_DATA_CNT_DOWN2_7,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_7__DI1_DATA_CNT_UP3_7,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_7__DI1_DATA_CNT_DOWN3_7,down);
                }
                break;}
        case  8:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_8__DI1_DATA_CNT_UP0_8,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_8__DI1_DATA_CNT_DOWN0_8,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_8__DI1_DATA_CNT_UP1_8,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_8__DI1_DATA_CNT_DOWN1_8,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_8__DI1_DATA_CNT_UP2_8,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_8__DI1_DATA_CNT_DOWN2_8,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_8__DI1_DATA_CNT_UP3_8,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_8__DI1_DATA_CNT_DOWN3_8,down);
                }
                break;}
        case  9:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_9__DI1_DATA_CNT_UP0_9,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_9__DI1_DATA_CNT_DOWN0_9,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_9__DI1_DATA_CNT_UP1_9,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_9__DI1_DATA_CNT_DOWN1_9,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_9__DI1_DATA_CNT_UP2_9,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_9__DI1_DATA_CNT_DOWN2_9,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_9__DI1_DATA_CNT_UP3_9,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_9__DI1_DATA_CNT_DOWN3_9,down);
                }
                break;}
        case 10:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_10__DI1_DATA_CNT_UP0_10,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_10__DI1_DATA_CNT_DOWN0_10,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_10__DI1_DATA_CNT_UP1_10,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_10__DI1_DATA_CNT_DOWN1_10,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_10__DI1_DATA_CNT_UP2_10,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_10__DI1_DATA_CNT_DOWN2_10,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_10__DI1_DATA_CNT_UP3_10,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_10__DI1_DATA_CNT_DOWN3_10,down);
                }
                break;}
        case 11:{if (set == 0){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_11__DI1_DATA_CNT_UP0_11,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET0_11__DI1_DATA_CNT_DOWN0_11,down);
                }
                if (set == 1){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_11__DI1_DATA_CNT_UP1_11,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET1_11__DI1_DATA_CNT_DOWN1_11,down);
                }
                if (set == 2){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_11__DI1_DATA_CNT_UP2_11,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET2_11__DI1_DATA_CNT_DOWN2_11,down);
                }
                if (set == 3){
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_11__DI1_DATA_CNT_UP3_11,up);
                    ipu_write_field(ipu_num, IPU_DI1_DW_SET3_11__DI1_DATA_CNT_DOWN3_11,down);
                }
                break;}
		default:{
			    break;}	
        }
    }
}

void enable_display(unsigned int IPU, unsigned int DI)
{
	if(DI == 0)
	{
		//enable DI0 (display interface 0)
		ipu_write_field(IPU, IPU_IPU_CONF__DI0_EN, 1);
	}
	else if(DI == 1)
	{
		//enable DI1 (display interface 1)
		ipu_write_field(IPU, IPU_IPU_CONF__DI1_EN, 1);
	}

#ifndef IPU_USE_DC_CHANNEL
	//enble DP (display processor)
	ipu_write_field(IPU, IPU_IPU_CONF__DP_EN, 1);
#endif

	//enble DC (display controller)
	ipu_write_field(IPU, IPU_IPU_CONF__DC_EN, 1);

	//enble DMFC (display multi-fifo controller)
	ipu_write_field(IPU, IPU_IPU_CONF__DMFC_EN, 1);
}

void idmac_config(unsigned int IPU)
{
	ipu_channel_parameter_t ipu_channel_params;

	memset(&ipu_channel_params, 0, sizeof(ipu_channel_parameter_t));

#ifdef IPU_USE_DC_CHANNEL
	ipu_channel_params.channel = 28;
#else
	ipu_channel_params.channel	 = 23;
#endif
	ipu_channel_params.eba0 = DISPLAY_EBA0 / 8;
//	ipu_channel_params.eba0 = (unsigned int)(&logobmp[0]) / 8;
	ipu_channel_params.eba1 = ipu_channel_params.eba0;
	ipu_channel_params.fw = DISPLAY_WIDTH - 1;  //frame width
	ipu_channel_params.fh = DISPLAY_HEIGHT - 1;  //frame hight
	ipu_channel_params.sl = DISPLAY_WIDTH * (DISPLAY_BPP / 8) - 1;
	ipu_channel_params.npb = 15;  //16 pixels per burst
	ipu_channel_params.pfs = 7;  //7->RGB

#if (DISPLAY_BPP == 16)
	ipu_channel_params.bpp = 3;  //0->32bpp; 1->24bpp; 3->16bpp
	ipu_channel_params.wid0 = 5 - 1;
	ipu_channel_params.wid1 = 6 - 1;
	ipu_channel_params.wid2 = 5 - 1;
	ipu_channel_params.wid3 = 0;
	ipu_channel_params.ofs0 = 0;
	ipu_channel_params.ofs1 = 5;
	ipu_channel_params.ofs2 = 11;
	ipu_channel_params.ofs3 = 0;
#endif
#if (DISPLAY_BPP == 24)
	ipu_channel_params.bpp = 1;  //0->32bpp; 1->24bpp; 3->16bpp
	ipu_channel_params.wid0 = 8 - 1;
	ipu_channel_params.wid1 = 8 - 1;
	ipu_channel_params.wid2 = 8 - 1;
	ipu_channel_params.wid3 = 0;
	ipu_channel_params.ofs0 = 0;
	ipu_channel_params.ofs1 = 8;
	ipu_channel_params.ofs2 = 16;
	ipu_channel_params.ofs3 = 0;
#endif
#if (DISPLAY_BPP == 32)
	ipu_channel_params.bpp = 0;  //0->32bpp; 1->24bpp; 3->16bpp
	ipu_channel_params.wid0 = 8 - 1;
	ipu_channel_params.wid1 = 8 - 1;
	ipu_channel_params.wid2 = 8 - 1;
	ipu_channel_params.wid3 = 8 - 1;
	ipu_channel_params.ofs0 = 8;
	ipu_channel_params.ofs1 = 16;
	ipu_channel_params.ofs2 = 24;
	ipu_channel_params.ofs3 = 0;
#endif

#ifdef DISPLAY_INTERLACED
	ipu_channel_params.so = 1;	//Scan order is interlaced
	ipu_channel_params.ilo = (ipu_channel_params.sl + 1) / 8;  //Interlace Offset
	ipu_channel_params.sl = (ipu_channel_params.sl + 1) * 2 - 1;  //Stride Line
#endif

	config_idmac_interleaved_channel(IPU, ipu_channel_params);
}

#ifdef DISPLAY_INTERLACED
void di_config_interlaced(unsigned int IPU, unsigned int DI)
{
	/*********************************************************************
	*	DI configuration:
	**********************************************************************/

	int clkUp, clkDown;
#ifdef DI_CLOCK_EXTERNAL_MODE
#if defined(IPU_OUTPUT_MODE_LVDS)  || defined(IPU_OUTPUT_MODE_HDMI)
	int ipuDiClk = DISPLAY_PIX_CLOCK;  //For iMX6Q, external clock, ipu_di_clk
#else
	int ipuDiClk = DISPLAY_PIX_CLOCK * 4;  //For iMX6Q, external clock, ipu_di_clk
#endif
#else
	int ipuDiClk = CONFIG_IPUV3_CLK;  //For iMX6, internel IPU clock, ipu_hsp_clk
#endif
	int typPixClk = DISPLAY_PIX_CLOCK; // typical value of pixel clock, (hSyncStartWidth + hSyncEndWidth + hSyncWidth + hDisp) * (vSyncStartWidth + vSyncEndWidth + vSyncWidth + vDisp) * refresh rate (60Hz)
	int div = (int)((float)ipuDiClk / (float)typPixClk + 0.5);// get the nearest value of typical pixel clock
	int hSyncStartWidth = DISPLAY_HSYNC_START;
	int hSyncWidth = DISPLAY_HSYNC_WIDTH;
	int hSyncEndWidth = DISPLAY_HSYNC_END;
	int hDisp = DISPLAY_WIDTH;
	int vSyncStartWidth = DISPLAY_VSYNC_START;
	int vSyncWidth = DISPLAY_VSYNC_WIDTH;
	int vSyncEndWidth = DISPLAY_VSYNC_END;
	int vDisp = DISPLAY_HEIGHT;

	// config (DRDY signal) DI_PIN15
	//di_pointer_config(IPU, di, pointer, access, componnent, cst, pt0, pt1, pt2, pt3, pt4, pt5, pt6);
	di_pointer_config(IPU, DI, DI_SDC_WAVEFORM, div - 1, div - 1, DI_NOUSE_SIGNAL, DI_NOUSE_SIGNAL, DI_NOUSE_SIGNAL, DI_NOUSE_SIGNAL, DI_NOUSE_SIGNAL, DI_DEN_SIGNAL, DI_NOUSE_SIGNAL, DI_NOUSE_SIGNAL);		

	//di_up_down_config(IPU, di, pointer, set, up, down)
	di_up_down_config(IPU, DI, DI_SDC_WAVEFORM, DI_DEN_SIGNAL, 0, div * 2);
	di_up_down_config(IPU, DI, DI_SDC_WAVEFORM, DI_NOUSE_SIGNAL, 0, 0);

	// set clk for DI
	// generate base clock for di
	clkUp = 0;
	clkDown = div;
	if(DI == 0)
	{
		ipu_write_field(IPU, IPU_DI0_BS_CLKGEN0__DI0_DISP_CLK_OFFSET, 0);
		ipu_write_field(IPU, IPU_DI0_BS_CLKGEN0__DI0_DISP_CLK_PERIOD, div << 4);
		ipu_write_field(IPU, IPU_DI0_BS_CLKGEN1__DI0_DISP_CLK_DOWN, clkDown);
		ipu_write_field(IPU, IPU_DI0_BS_CLKGEN1__DI0_DISP_CLK_UP, clkUp);

		//DI_SCR, set the screen height for field0
		ipu_write_field(IPU, IPU_DI0_SCR_CONF__DI0_SCREEN_HEIGHT, (vDisp + vSyncStartWidth + vSyncEndWidth + vSyncWidth) / 2 - 1);
	}
	else if(DI == 1)
	{
		ipu_write_field(IPU, IPU_DI1_BS_CLKGEN0__DI1_DISP_CLK_OFFSET, 0);
		ipu_write_field(IPU, IPU_DI1_BS_CLKGEN0__DI1_DISP_CLK_PERIOD, div << 4);
		ipu_write_field(IPU, IPU_DI1_BS_CLKGEN1__DI1_DISP_CLK_DOWN, clkDown);
		ipu_write_field(IPU, IPU_DI1_BS_CLKGEN1__DI1_DISP_CLK_UP, clkUp);

		//DI_SCR, set the screen height for field0
		ipu_write_field(IPU, IPU_DI1_SCR_CONF__DI1_SCREEN_HEIGHT, (vDisp + vSyncStartWidth + vSyncEndWidth + vSyncWidth) / 2 - 1);
	}

	// COUNTER_1: Internal VSYNC for each frame
	di_sync_config(
		IPU, // ipu num
		DI, // di
		DI_COUNTER_1, // pointer
		(vDisp + vSyncStartWidth + vSyncEndWidth + vSyncWidth) * 2 - 1, // run value for auto reload
		3, // run resolution, counter 1 can reference to counter 6,7,8 with run_resolution=2,3,4
		1, // offset value
		3, // offset resolution, 3=counter 7
		1, // auto reload mode
		0, // step repeat
		0, // counter clear source selection
		0, // counter polarity
		0, // counter polarity trigger selection
		0, // counter polarity clear selection
		0, // counter up
		0, // counter down
		0 // gentime select
	);

	//COUNTER_2: HSYNC waveform on DI_PIN02
	di_sync_config(
		IPU, // ipu num
		DI, // di
		DI_COUNTER_HSYNC, // pointer
		hDisp + hSyncStartWidth + hSyncEndWidth + hSyncWidth - 1, // run value for auto reload
		DI_COUNTER_BASECLK + 1, // run resolution
		0, // offset value
		0, // offset resolution
		1, // auto reload mode
		0, // step repeat
		0, // counter clear source selection
		1, // counter polarity
		DI_COUNTER_BASECLK + 1, // counter polarity trigger selection
		0, // counter polarity clear selection
		0, // counter up
		hSyncWidth << 1, // counter down
		0 // gentime select
	);

	//COUNTER_3: VSYNC waveform on DI_PIN03
	di_sync_config(
		IPU, // ipu num
		DI, // di
		DI_COUNTER_VSYNC, // pointer
		vDisp + vSyncStartWidth + vSyncEndWidth + vSyncWidth - 1, // run value for auto reload
		4, // run resolution, counter 3 can reference to counter 7 with run_resolution=4
		1, // offset value
		4, // offset resolution, 4=counter 7
		0, // auto reload mode
		2, // step repeat
		DI_COUNTER_1 + 1, // counter clear source selection
		1, // counter polarity
		4, // counter polarity trigger selection, 4=counter 7
		0, // counter polarity clear selection
		0, // counter up
		vSyncWidth << 1, // counter down
		0 // gentime select
	);

	//COUNTER_4: Active Field
	di_sync_config(
		IPU, // ipu num
		DI, // di
		DI_COUNTER_AFIELD, // pointer
		((vDisp + vSyncStartWidth + vSyncEndWidth + vSyncWidth) / 2 + 1) - 1, // run value for auto reload
		DI_COUNTER_HSYNC + 1, // run resolution
		(hDisp + hSyncStartWidth + hSyncEndWidth + hSyncWidth) / 2, // offset value
		DI_COUNTER_BASECLK + 1, // offset resolution
		0, // auto reload mode
		2, // step repeat
		DI_COUNTER_1 + 1, // counter clear source selection
		0, // counter polarity
		0, // counter polarity trigger selection
		0, // counter polarity clear selection
		0, // counter up
		0, // counter down
		0 // gentime select
	);

	//COUNTER_5: Active Line
	di_sync_config(
		IPU, // ipu num
		DI, // di
		DI_COUNTER_ALINE, // pointer
		0, // run value for auto reload
		DI_COUNTER_HSYNC + 1, // run resolution
		(vSyncStartWidth + vSyncWidth) / 2, // offset value
		DI_COUNTER_HSYNC + 1, // offset resolution
		0, // auto reload mode
		vDisp / 2, // step repeat
		DI_COUNTER_AFIELD + 1, // counter clear source selection
		0, // counter polarity
		0, // counter polarity trigger selection
		0, // counter polarity clear selection
		0, // counter up
		0, // counter down
		0 // gentime select
	);

	//COUNTER_6: Active Pixel
	di_sync_config(
		IPU, // ipu num
		DI, // di
		DI_COUNTER_APIXEL, // pointer
		0, // run value for auto reload
		DI_COUNTER_BASECLK + 1, // run resolution
		hSyncStartWidth + hSyncWidth, // offset value
		DI_COUNTER_BASECLK + 1, // offset resolution
		0, // auto reload mode
		hDisp, // step repeat
		DI_COUNTER_ALINE + 1, // counter clear source selection
		0, // counter polarity
		0, // counter polarity trigger selection
		0, // counter polarity clear selection
		0, // counter up
		0, // counter down
		0 // gentime select
	);

	//COUNTER_7: Half line HSYNC
	di_sync_config(
		IPU, // ipu num
		DI, // di
		DI_COUNTER_7, // pointer
		(hDisp + hSyncStartWidth + hSyncEndWidth + hSyncWidth) / 2 - 1, // run value for auto reload
		DI_COUNTER_BASECLK + 1, // run resolution
		0, // offset value
		0, // offset resolution
		1, // auto reload mode
		0, // step repeat
		0, // counter clear source selection
		0, // counter polarity
		0, // counter polarity trigger selection
		0, // counter polarity clear selection
		0, // counter up
		0, // counter down
		0 // gentime select
	);

	if(DI == 0)
	{
		ipu_write_field(IPU, IPU_DI0_SYNC_AS_GEN__DI0_SYNC_START, 0);  //0 lines predictions
		ipu_write_field(IPU, IPU_DI0_SYNC_AS_GEN__DI0_VSYNC_SEL, DI_COUNTER_VSYNC - 1); //pin3/counter 3 as VSYNC
#ifdef DI_CLOCK_EXTERNAL_MODE
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_CLK_EXT, 1); // select external generated clock 
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_VSYNC_EXT, 1); // select external VSYNC 
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_ERM_VSYNC_SEL, 1);
#else
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_CLK_EXT, 0); // select internal generated clock 
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_VSYNC_EXT, 0); // select internal VSYNC 
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_ERM_VSYNC_SEL, 0);
#endif
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_POLARITY_DISP_CLK, DISPLAY_CLOCK_POLARITY); 
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_POLARITY_2, DISPLAY_HSYNC_POLARITY);
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_POLARITY_3, DISPLAY_VSYNC_POLARITY);
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_DISP_Y_SEL, DI_COUNTER_HSYNC - 1);  //pin2/counter 2 as HSYNC

		ipu_write_field(IPU, IPU_DI0_POL__DI0_DRDY_DATA_POLARITY, DISPLAY_DATA_POLARITY);
		ipu_write_field(IPU, IPU_DI0_POL__DI0_DRDY_POLARITY_15, DISPLAY_DATA_ENABLE_POLARITY);

		//release ipu DI counter
		ipu_write_field(IPU, IPU_IPU_DISP_GEN__DI0_COUNTER_RELEASE, 1);
	}
	else if(DI == 1)
	{
		ipu_write_field(IPU, IPU_DI1_SYNC_AS_GEN__DI1_SYNC_START, 0);  //0 lines predictions
		ipu_write_field(IPU, IPU_DI1_SYNC_AS_GEN__DI1_VSYNC_SEL, DI_COUNTER_VSYNC - 1); //pin3/counter 3 as VSYNC
#ifdef DI_CLOCK_EXTERNAL_MODE
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_CLK_EXT, 1); // select external generated clock 
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_VSYNC_EXT, 1); // select external VSYNC 
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_ERM_VSYNC_SEL, 1);
#else
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_CLK_EXT, 0); // select internal generated clock 
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_VSYNC_EXT, 0); // select internal VSYNC 
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_ERM_VSYNC_SEL, 0);
#endif
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_POLARITY_DISP_CLK, DISPLAY_CLOCK_POLARITY); 
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_POLARITY_2, DISPLAY_HSYNC_POLARITY);
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_POLARITY_3, DISPLAY_VSYNC_POLARITY);
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_DISP_Y_SEL, DI_COUNTER_HSYNC - 1);  //pin2/counter 2 as HSYNC

		ipu_write_field(IPU, IPU_DI1_POL__DI1_DRDY_DATA_POLARITY, DISPLAY_DATA_POLARITY);
		ipu_write_field(IPU, IPU_DI1_POL__DI1_DRDY_POLARITY_15, DISPLAY_DATA_ENABLE_POLARITY);

		//release ipu DI counter
		ipu_write_field(IPU, IPU_IPU_DISP_GEN__DI1_COUNTER_RELEASE, 1);
	}
}

#else

void di_config(unsigned int IPU, unsigned int DI)
{
	/*********************************************************************
	*	DI configuration:
	**********************************************************************/

	int clkUp, clkDown;
#ifdef DI_CLOCK_EXTERNAL_MODE
#if defined(IPU_OUTPUT_MODE_LVDS)  || defined(IPU_OUTPUT_MODE_HDMI)
	int ipuDiClk = DISPLAY_PIX_CLOCK;  //For iMX6Q, external clock, ipu_di_clk
#else
	int ipuDiClk = DISPLAY_PIX_CLOCK * 4;  //For iMX6Q, external clock, ipu_di_clk
#endif
#else
	int ipuDiClk = CONFIG_IPUV3_CLK;  //For iMX6, internel IPU clock, ipu_hsp_clk
#endif
	int typPixClk = DISPLAY_PIX_CLOCK; // typical value of pixel clock, (hSyncStartWidth + hSyncEndWidth + hSyncWidth + hDisp) * (vSyncStartWidth + vSyncEndWidth + vSyncWidth + vDisp) * refresh rate (60Hz)
	int div = (int)((float)ipuDiClk / (float)typPixClk + 0.5);// get the nearest value of typical pixel clock
	int hSyncStartWidth = DISPLAY_HSYNC_START;
	int hSyncWidth = DISPLAY_HSYNC_WIDTH;
	int hSyncEndWidth = DISPLAY_HSYNC_END;
	int hDisp = DISPLAY_WIDTH;
	int vSyncStartWidth = DISPLAY_VSYNC_START;
	int vSyncWidth = DISPLAY_VSYNC_WIDTH;
	int vSyncEndWidth = DISPLAY_VSYNC_END;
	int vDisp = DISPLAY_HEIGHT;

	// config (DRDY signal) DI_PIN15
	//di_pointer_config(IPU, di, pointer, access, componnent, cst, pt0, pt1, pt2, pt3, pt4, pt5, pt6);
	di_pointer_config(IPU, DI, DI_SDC_WAVEFORM, div - 1, div - 1, DI_NOUSE_SIGNAL, DI_NOUSE_SIGNAL, DI_NOUSE_SIGNAL, DI_NOUSE_SIGNAL, DI_NOUSE_SIGNAL, DI_DEN_SIGNAL, DI_NOUSE_SIGNAL, DI_NOUSE_SIGNAL);		

	//di_up_down_config(IPU, di, pointer, set, up, down)
	di_up_down_config(IPU, DI, DI_SDC_WAVEFORM, DI_DEN_SIGNAL, 0, div * 2);
	di_up_down_config(IPU, DI, DI_SDC_WAVEFORM, DI_NOUSE_SIGNAL, 0, 0);

	// set clk for DI
	// generate base clock for di
	clkUp = 0;
	clkDown = div;
	if(DI == 0)
	{
		ipu_write_field(IPU, IPU_DI0_BS_CLKGEN0__DI0_DISP_CLK_OFFSET, 0);
		ipu_write_field(IPU, IPU_DI0_BS_CLKGEN0__DI0_DISP_CLK_PERIOD, div << 4);
		ipu_write_field(IPU, IPU_DI0_BS_CLKGEN1__DI0_DISP_CLK_DOWN, clkDown);
		ipu_write_field(IPU, IPU_DI0_BS_CLKGEN1__DI0_DISP_CLK_UP, clkUp);

		//DI_SCR, set the screen height   
		ipu_write_field(IPU, IPU_DI0_SCR_CONF__DI0_SCREEN_HEIGHT, vDisp + vSyncStartWidth + vSyncEndWidth + vSyncWidth - 1);
	}
	else if(DI == 1)
	{
		ipu_write_field(IPU, IPU_DI1_BS_CLKGEN0__DI1_DISP_CLK_OFFSET, 0);
		ipu_write_field(IPU, IPU_DI1_BS_CLKGEN0__DI1_DISP_CLK_PERIOD, div << 4);
		ipu_write_field(IPU, IPU_DI1_BS_CLKGEN1__DI1_DISP_CLK_DOWN, clkDown);
		ipu_write_field(IPU, IPU_DI1_BS_CLKGEN1__DI1_DISP_CLK_UP, clkUp);

		//DI_SCR, set the screen height   
		ipu_write_field(IPU, IPU_DI1_SCR_CONF__DI1_SCREEN_HEIGHT, vDisp + vSyncStartWidth + vSyncEndWidth + vSyncWidth - 1);
	}

	di_sync_config(
		IPU, // ipu num
		DI, // di
		DI_COUNTER_IHSYNC, // pointer
		hDisp + hSyncStartWidth + hSyncEndWidth + hSyncWidth - 1, // run value for auto reload
		DI_COUNTER_BASECLK + 1, // run resolution
		0, // offset value
		0, // offset resolution
		1, // auto reload mode
		0, // step repeat
		0, // counter clear source selection
		0, // counter polarity
		0, // counter polarity trigger selection
		0, // counter polarity clear selection
		0, // counter up
		0, // counter down
		0 // gentime select
	);

	//Output HSYNC
	di_sync_config(
		IPU, // ipu num
		DI, // di
		DI_COUNTER_HSYNC, // pointer
		hDisp + hSyncStartWidth + hSyncEndWidth + hSyncWidth - 1, // run value for auto reload
		DI_COUNTER_BASECLK + 1, // run resolution
		0, // offset value
		0, // offset resolution
		1, // auto reload mode
		0, // step repeat
		0, // counter clear source selection
		1, // counter polarity
		DI_COUNTER_BASECLK + 1, // counter polarity trigger selection
		0, // counter polarity clear selection
		0, // counter up
		hSyncWidth << 1, // counter down
		0 // gentime select
	);

	//Output VSYNC
	di_sync_config(
		IPU, // ipu num
		DI, // di
		DI_COUNTER_VSYNC, // pointer
		vDisp + vSyncStartWidth + vSyncEndWidth + vSyncWidth - 1, // run value for auto reload
		DI_COUNTER_HSYNC + 1, // run resolution
		0, // offset value
		0, // offset resolution
		1, // auto reload mode
		0, // step repeat
		0, // counter clear source selection
		1, // counter polarity
		DI_COUNTER_HSYNC + 1, // counter polarity trigger selection
		0, // counter polarity clear selection
		0, // counter up
		vSyncWidth << 1, // counter down
		0 // gentime select
	);

	//Active Lines start points
	di_sync_config(
		IPU, // ipu num
		DI, // di
		DI_COUNTER_ALINE, // pointer
		0, // run value for auto reload	
		DI_COUNTER_HSYNC + 1, // run resolution
		vSyncStartWidth + vSyncWidth, // offset value
		DI_COUNTER_HSYNC + 1, // offset resolution
		0, // auto reload mode
		vDisp, // step repeat
		DI_COUNTER_VSYNC + 1, // counter clear source selection
		0, // counter polarity
		0, // counter polarity trigger selection
		0, // counter polarity clear selection
		0, // counter up
		0, // counter down
		0 // gentime select
	);

	//Active clock start points
	di_sync_config(
		IPU, // ipu num
		DI, // di
		DI_COUNTER_APIXEL, // pointer
		0, // run value for auto reload
		DI_COUNTER_BASECLK + 1, // run resolution
		hSyncStartWidth + hSyncWidth, // offset value
		DI_COUNTER_BASECLK + 1, // offset resolution
		0, // auto reload mode
		hDisp, // step repeat
		DI_COUNTER_ALINE + 1, // counter clear source selection
		0, // counter polarity
		0, // counter polarity trigger selection
		0, // counter polarity clear selection
		0, // counter up
		0, // counter down
		0 // gentime select
	);

	if(DI == 0)
	{
		ipu_write_field(IPU, IPU_DI0_SYNC_AS_GEN__DI0_SYNC_START, 2);  // 2 lines predictions
		ipu_write_field(IPU, IPU_DI0_SYNC_AS_GEN__DI0_VSYNC_SEL, DI_COUNTER_VSYNC - 1);  //select PIN3 as VSYNC
#ifdef DI_CLOCK_EXTERNAL_MODE
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_CLK_EXT, 1);  //select external generated clock 
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_VSYNC_EXT, 1);  //select external VSYNC 
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_ERM_VSYNC_SEL, 1);
#else
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_CLK_EXT, 0);  //select internal generated clock 
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_VSYNC_EXT, 0);  //select internal VSYNC 
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_ERM_VSYNC_SEL, 0);
#endif
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_POLARITY_DISP_CLK, DISPLAY_CLOCK_POLARITY); 
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_POLARITY_2, DISPLAY_HSYNC_POLARITY);  //HSYNC polarity
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_POLARITY_3, DISPLAY_VSYNC_POLARITY);  //VSYNC polarity
		ipu_write_field(IPU, IPU_DI0_GENERAL__DI0_DISP_Y_SEL, DI_COUNTER_HSYNC - 1);  //pin2/counter 2 as HSYNC

		ipu_write_field(IPU, IPU_DI0_POL__DI0_DRDY_DATA_POLARITY, DISPLAY_DATA_POLARITY);
		ipu_write_field(IPU, IPU_DI0_POL__DI0_DRDY_POLARITY_15, DISPLAY_DATA_ENABLE_POLARITY);  //VIDEO_DATA_EN POLARITY

		//release ipu DI counter
		ipu_write_field(IPU, IPU_IPU_DISP_GEN__DI0_COUNTER_RELEASE, 1);
	}
	else if(DI == 1)
	{
		ipu_write_field(IPU, IPU_DI1_SYNC_AS_GEN__DI1_SYNC_START, 2);  // 2 lines predictions
		ipu_write_field(IPU, IPU_DI1_SYNC_AS_GEN__DI1_VSYNC_SEL, DI_COUNTER_VSYNC - 1);  //select PIN3 as VSYNC
#ifdef DI_CLOCK_EXTERNAL_MODE
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_CLK_EXT, 1);	//select external generated clock 
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_VSYNC_EXT, 1);  //select external VSYNC 
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_ERM_VSYNC_SEL, 1);
#else
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_CLK_EXT, 0);	//select internal generated clock 
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_VSYNC_EXT, 0);  //select internal VSYNC 
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_ERM_VSYNC_SEL, 0);
#endif
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_POLARITY_DISP_CLK, DISPLAY_CLOCK_POLARITY); 
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_POLARITY_2, DISPLAY_HSYNC_POLARITY);	//HSYNC polarity
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_POLARITY_3, DISPLAY_VSYNC_POLARITY);	//VSYNC polarity
		ipu_write_field(IPU, IPU_DI1_GENERAL__DI1_DISP_Y_SEL, DI_COUNTER_HSYNC - 1);  //pin2/counter 2 as HSYNC

		ipu_write_field(IPU, IPU_DI1_POL__DI1_DRDY_DATA_POLARITY, DISPLAY_DATA_POLARITY);
		ipu_write_field(IPU, IPU_DI1_POL__DI1_DRDY_POLARITY_15, DISPLAY_DATA_ENABLE_POLARITY);	//VIDEO_DATA_EN POLARITY

		//release ipu DI counter
		ipu_write_field(IPU, IPU_IPU_DISP_GEN__DI1_COUNTER_RELEASE, 1);
	}
}
#endif

void ipu_setup(unsigned int IPU, unsigned int DI)
{
	unsigned int reg;
	struct src *src_regs = (struct src *)SRC_BASE_ADDR;
	struct mxc_ccm_reg *imx_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	//Disable IPU and DI clocks
	reg = __raw_readl(&imx_ccm->CCGR3);
	reg &= ~0xFFFF;
	__raw_writel(reg, &imx_ccm->CCGR3);

#ifdef CONFIG_MX6DL
	//Switch the IPU clock to PLL3_PFD1_540M for iMX6S/DL
	__raw_writel(BM_ANADIG_PFD_480_PFD1_CLKGATE, &imx_ccm->analog_pfd_480_set);

	reg = __raw_readl(&imx_ccm->cscdr3);
	reg &= ~(0x7 << 11);  //Set ipu1_hsp_podf to 0 for divider = 1.
	__raw_writel(reg, &imx_ccm->cscdr3);
	printf("delay.\r\n");

	reg |= (0x3 << 9);  //Switch ipu1_hsp_clk
	__raw_writel(reg, &imx_ccm->cscdr3);
	printf("delay.\r\n");

	reg |= (0x1 << 11);  //Set ipu1_hsp_podf to 1 for divider = 2.
	__raw_writel(reg, &imx_ccm->cscdr3);
	printf("delay.\r\n");

	__raw_writel(BM_ANADIG_PFD_480_PFD1_CLKGATE, &imx_ccm->analog_pfd_480_clr);
#endif

#ifdef DI_CLOCK_EXTERNAL_MODE
#ifdef IPU_OUTPUT_MODE_LVDS
#ifdef LVDS_SPLIT_MODE
	if (IPU == 1)
	{
		if (DI == 0)
			display_clk_config(MXC_IPU1_LVDS_DI0_CLK, DISPLAY_PIX_CLOCK * 7 / 2);
		else if(DI == 1)
			display_clk_config(MXC_IPU1_LVDS_DI1_CLK, DISPLAY_PIX_CLOCK * 7 / 2);
	}
	else if(IPU == 2)
	{
		if (DI == 0)
			display_clk_config(MXC_IPU2_LVDS_DI0_CLK, DISPLAY_PIX_CLOCK * 7 / 2);
		else if(DI == 1)
			display_clk_config(MXC_IPU2_LVDS_DI1_CLK, DISPLAY_PIX_CLOCK * 7 / 2);
	}
#else
	if (IPU == 1)
	{
		if (DI == 0)
			display_clk_config(MXC_IPU1_LVDS_DI0_CLK, DISPLAY_PIX_CLOCK * 7);
		else if(DI == 1)
			display_clk_config(MXC_IPU1_LVDS_DI1_CLK, DISPLAY_PIX_CLOCK * 7);
	}
	else if(IPU == 2)
	{
		if (DI == 0)
			display_clk_config(MXC_IPU2_LVDS_DI0_CLK, DISPLAY_PIX_CLOCK * 7);
		else if(DI == 1)
			display_clk_config(MXC_IPU2_LVDS_DI1_CLK, DISPLAY_PIX_CLOCK * 7);
	}
#endif
#endif  //IPU_OUTPUT_MODE_LVDS

#ifdef IPU_OUTPUT_MODE_HDMI
	if (IPU == 1)
	{
		if(DI == 0)
			display_clk_config(MXC_IPU1_DI0_CLK, DISPLAY_PIX_CLOCK);
		else
			display_clk_config(MXC_IPU1_DI1_CLK, DISPLAY_PIX_CLOCK);
	}
	else if(IPU == 2)
	{
		if(DI == 0)
			display_clk_config(MXC_IPU2_DI0_CLK, DISPLAY_PIX_CLOCK);
		else
			display_clk_config(MXC_IPU2_DI1_CLK, DISPLAY_PIX_CLOCK);
	}
#endif  //IPU_OUTPUT_MODE_HDMI

#ifdef IPU_OUTPUT_MODE_LCD
	if (IPU == 1)
	{
		if(DI == 0)
			display_clk_config(MXC_IPU1_DI0_CLK, DISPLAY_PIX_CLOCK * 4);
		else
			display_clk_config(MXC_IPU1_DI1_CLK, DISPLAY_PIX_CLOCK * 4);
	}
	else if(IPU == 2)
	{
		if(DI == 0)
			display_clk_config(MXC_IPU2_DI0_CLK, DISPLAY_PIX_CLOCK * 4);
		else
			display_clk_config(MXC_IPU2_DI1_CLK, DISPLAY_PIX_CLOCK * 4);
	}
#endif  //IPU_OUTPUT_MODE_LCD
#endif  //DI_CLOCK_EXTERNAL_MODE

	if (IPU == 1)
	{
		// Reset IPU
		reg = __raw_readl(&src_regs->scr);
		reg |= 0x8;
		__raw_writel(reg, &src_regs->scr);

		// Wait for reset done
		reg = __raw_readl(&src_regs->scr);
		while (reg & 0x8)
			reg = __raw_readl(&src_regs->scr);

		// Enable IPU clock
		reg = __raw_readl(&imx_ccm->CCGR3);
		reg |= (3 << 0);
		__raw_writel(reg, &imx_ccm->CCGR3);

		// Reset IPU memory 
		writel(0x807FFFFF, IPU1_ARB_BASE_ADDR + IPU_IPU_MEM_RST__ADDR);
		while (readl(IPU1_ARB_BASE_ADDR + IPU_IPU_MEM_RST__ADDR) & 0x80000000);

		if (DI == 0)
		{
			//Enable DI0 clock
			reg = __raw_readl(&imx_ccm->CCGR3);
			reg |= (3 << 2);
			__raw_writel(reg, &imx_ccm->CCGR3);
		}
		else if(DI == 1)
		{
			//Enable DI1 clock
			reg = __raw_readl(&imx_ccm->CCGR3);
			reg |= (3 << 4);
			__raw_writel(reg, &imx_ccm->CCGR3);
		}
	}
	else if(IPU == 2)
	{
		// Reset IPU
		reg = __raw_readl(&src_regs->scr);
		reg |= 0x1000;
		__raw_writel(reg, &src_regs->scr);

                // Wait for reset done
                reg = __raw_readl(&src_regs->scr);
                while (reg & 0x1000)
                        reg = __raw_readl(&src_regs->scr);

		// Enable IPU clock
		reg = __raw_readl(&imx_ccm->CCGR3);
		reg |= (3 << 6);
		__raw_writel(reg, &imx_ccm->CCGR3);

		// Reset IPU memory	
		writel(0x807FFFFF, IPU2_ARB_BASE_ADDR + IPU_IPU_MEM_RST__ADDR);
		while (readl(IPU2_ARB_BASE_ADDR + IPU_IPU_MEM_RST__ADDR) & 0x80000000);

		if (DI == 0)
		{
			//Enable DI0 clock
			reg = __raw_readl(&imx_ccm->CCGR3);
			reg |= (3 << 8);
			__raw_writel(reg, &imx_ccm->CCGR3);
		}
		else if(DI == 1)
		{
			//Enable DI1 clock
			reg = __raw_readl(&imx_ccm->CCGR3);
			reg |= (3 << 10);
			__raw_writel(reg, &imx_ccm->CCGR3);
		}
	}

#ifdef CONFIG_MX6QP
	reg = __raw_readl(&imx_ccm->CCGR6);
	reg |= MXC_CCM_CCGR6_PRG_CLK0_MASK;
	__raw_writel(reg, &imx_ccm->CCGR6);

	/* Bypass IPU1 QoS generator */
	__raw_writel(0x00000002, 0x00bb048c);
	/* Bypass IPU2 QoS generator */
	__raw_writel(0x00000002, 0x00bb050c);
	/* Bandwidth THR for of PRE0 */
	__raw_writel(0x00000200, 0x00bb0690);
	/* Bandwidth THR for of PRE1 */
	__raw_writel(0x00000200, 0x00bb0710);
	/* Bandwidth THR for of PRE2 */
	__raw_writel(0x00000200, 0x00bb0790);
	/* Bandwidth THR for of PRE3 */
	__raw_writel(0x00000200, 0x00bb0810);
	/* Saturation THR for of PRE0 */
	__raw_writel(0x00000010, 0x00bb0694);
	/* Saturation THR for of PRE1 */
	__raw_writel(0x00000010, 0x00bb0714);
	/* Saturation THR for of PRE2 */
	__raw_writel(0x00000010, 0x00bb0794);
	/* Saturation THR for of PRE */
	__raw_writel(0x00000010, 0x00bb0814);
#endif

#ifdef IPU_OUTPUT_MODE_LVDS
	//Enable LDB DI clocks
	reg = __raw_readl(&imx_ccm->CCGR3);
	reg |= (0xF << 12);
	__raw_writel(reg, &imx_ccm->CCGR3);
#endif

#ifdef IPU_OUTPUT_MODE_HDMI
	//Enable HDMI clock
	reg = __raw_readl(&imx_ccm->CCGR2);
	reg |= (3 << 0);
	reg |= (3 << 4);
	__raw_writel(reg, &imx_ccm->CCGR2);
#endif

	dmfc_config(IPU);
	idmac_config(IPU);

#ifndef IPU_USE_DC_CHANNEL
	dp_config(IPU);
#endif

#ifdef DISPLAY_INTERLACED
	dc_config_interlaced(IPU, DI);
	di_config_interlaced(IPU, DI);
#else
	dc_config(IPU, DI);
	di_config(IPU, DI);
#endif

	enable_display(IPU, DI);

#ifdef IPU_USE_DC_CHANNEL
//	ipu_write_field(IPU, IPU_IPU_CH_DB_MODE_SEL_0__DMA_CH_DB_MODE_SEL_28, 1);
	ipu_write_field(IPU, IPU_IPU_CH_TRB_MODE_SEL_0__DMA_CH_TRB_MODE_SEL_28, 1);
	ipu_write_field(IPU, IPU_IPU_CH_BUF0_RDY0__DMA_CH_BUF0_RDY_28, 1);
	ipu_write_field(IPU, IPU_IDMAC_CH_EN_1__IDMAC_CH_EN_28, 0x1);  //enable channel 28
#else
//	ipu_write_field(IPU, IPU_IPU_CH_DB_MODE_SEL_0__DMA_CH_DB_MODE_SEL_23, 1);
	ipu_write_field(IPU, IPU_IPU_CH_TRB_MODE_SEL_0__DMA_CH_TRB_MODE_SEL_23, 1);
	ipu_write_field(IPU, IPU_IPU_CH_BUF0_RDY0__DMA_CH_BUF0_RDY_23, 1);
	ipu_write_field(IPU, IPU_IDMAC_CH_EN_1__IDMAC_CH_EN_23, 0x1);  //enable channel 23
#endif
}

#ifdef IPU_OUTPUT_MODE_HDMI
extern void hdmi_display_enable(unsigned int mCode);
void setup_hdmi(unsigned int IPU, unsigned int DI)
{
	if(IPU == 1)
	{
		if(DI == 0)
			imx_iomux_set_gpr_register(3, 2, 2, 0);
		else if(DI == 1)
			imx_iomux_set_gpr_register(3, 2, 2, 1);
	}

	if(IPU == 2)
	{
		if(DI == 0)
			imx_iomux_set_gpr_register(3, 2, 2, 2);
		else if(DI == 1)
			imx_iomux_set_gpr_register(3, 2, 2, 3);
	}

#ifdef DISPLAY_INTERLACED
#if (DISPLAY_WIDTH == 1920) && (DISPLAY_HEIGHT == 1080)
	hdmi_display_enable(5);  //1080i@60Hz
#endif

#else

#if (DISPLAY_WIDTH == 1920) && (DISPLAY_HEIGHT == 1080)
	hdmi_display_enable(16);  //1080p@60Hz
#else
	hdmi_display_enable(4);  //720p@60Hz
#endif
#endif  //DISPLAY_INTERLACED
}
#endif

#ifdef IPU_OUTPUT_MODE_LVDS
void setup_lvds(unsigned int IPU, unsigned int DI)
{
	unsigned int reg;

#if (LVDS_PORT == 0)
	if(IPU == 1)
	{
		if(DI == 0)
			imx_iomux_set_gpr_register(3, 6, 2, 0);
		else if(DI == 1)
			imx_iomux_set_gpr_register(3, 6, 2, 1);
	}

	if(IPU == 2)
	{
		if(DI == 0)
			imx_iomux_set_gpr_register(3, 6, 2, 2);
		else if(DI == 1)
			imx_iomux_set_gpr_register(3, 6, 2, 3);
	}
#endif

#if (LVDS_PORT == 1)
	if(IPU == 1)
	{
		if(DI == 0)
			imx_iomux_set_gpr_register(3, 8, 2, 0);
		else if(DI == 1)
			imx_iomux_set_gpr_register(3, 8, 2, 1);
	}

	if(IPU == 2)
	{
		if(DI == 0)
			imx_iomux_set_gpr_register(3, 8, 2, 2);
		else if(DI == 1)
			imx_iomux_set_gpr_register(3, 8, 2, 3);
	}
#endif

	reg = 0;
	if (DI == 0)
		reg |= (DISPLAY_VSYNC_POLARITY << 9);
	else if(DI == 1)
		reg |= (DISPLAY_VSYNC_POLARITY << 10);

#if (LVDS_PORT == 0)
#if (DISPLAY_IF_BPP == 24)
	reg |= (1 << 5);
#ifdef LVDS_SPLIT_MODE
	reg |= (1 << 7);
#endif
#endif

	if (DI == 0)
		reg |= (1 << 0);
	else if(DI == 1)
		reg |= (3 << 0);

#ifdef LVDS_SPLIT_MODE
	reg |= (1 << 4);
	if (DI == 0)
		reg |= (1 << 2);
	else if(DI == 1)
		reg |= (3 << 2);
#endif
#endif

#if (LVDS_PORT == 1)
#if (DISPLAY_IF_BPP == 24)
	reg |= (1 << 7);
#ifdef LVDS_SPLIT_MODE
	reg |= (1 << 5);
#endif
#endif

	if (DI == 0)
		reg |= (1 << 2);
	else if(DI == 1)
		reg |= (3 << 2);
#ifdef LVDS_SPLIT_MODE
	reg |= (1 << 4);
	if (DI == 0)
		reg |= (1 << 0);
	else if(DI == 1)
		reg |= (3 << 0);
#endif
#endif

	writel(reg, IOMUXC_BASE_ADDR + 0x8);  //Set LDB_CTRL
}
#endif

int ipu_display_setup(int ipunum,int ipudi)
{
//	ipu_setup(IPU_NUM, DI_NUM);
	ipu_setup(ipunum, ipudi);

#ifdef IPU_OUTPUT_MODE_HDMI
//	setup_hdmi(IPU_NUM, DI_NUM);
	setup_hdmi(ipunum, ipudi);
#endif

#ifdef IPU_OUTPUT_MODE_LVDS
//	setup_lvds(IPU_NUM, DI_NUM);
	setup_lvds(ipunum, ipudi);
#endif

	return 0;
}

