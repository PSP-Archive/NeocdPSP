/**************************************
****  NEOCD.C  - Main Source File  ****
**************************************/

//-- Include Files -----------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
//#include <ctype.h>
#include <string.h>

#include "neocd.h"
#include "imageio.h"

extern volatile int bLoop,bSleep;

extern int nb_interlace;

int neogeo_autofireA=0,neogeo_autofireB=0,neogeo_autofireC=0,neogeo_autofireD=0;
int neogeo_hardrender=1;
int neogeo_speedlimit=1;
int neogeo_showfps=0;

volatile int neogeo_soundmute;

static struct timeval	tv_next1,tv_now;


struct timeval	tv_start;


#define timercmp(a, b, CMP)	(((a)->tv_sec == (b)->tv_sec) ? ((a)->tv_usec CMP (b)->tv_usec) : ((a)->tv_sec CMP (b)->tv_sec))



int neogeo_emulating=0;
int	neogeo_sound_enable=1;
int	neogeo_mp3_enable=1;
int	neogeo_frameskip=0;

s32 neogeo_frameskip_count;
s32 neogeo_frame_count;
static int fps_framecount;

static int num_resets;

unsigned z80_cycles_inited;
#define Z80_CYCLES_INIT 90


static char		neocd_wm_title[255]=VERSION1;


int neo4all_skip_next_frame=0;
u32 neo4all_z80_cycles=NEO4ALL_Z80_NORMAL_CYCLES;
u32 neo4all_68k_cycles=NEO4ALL_68K_NORMAL_CYCLES;
u32 neo4all_z80_cycles_idx=1;
u32 neo4all_68k_cycles_idx=1;


//-- Global Variables --------------------------------------------------------
char			*neogeo_rom_memory = NULL;
char			*neogeo_prg_memory = NULL;
char			*neogeo_fix_memory = NULL;
char			*neogeo_spr_memory = NULL;
char			*neogeo_pcm_memory = NULL;

char			global_error[80];
int			neogeo_prio_mode;
int			neogeo_ipl_done;
int			neogeo_region=REGION;
unsigned char		config_game_name[80];




//-- 68K Core related stuff --------------------------------------------------
int				mame_debug;
int				previouspc;
int				ophw;
int				cur_mrhard;


//-- Function Prototypes -----------------------------------------------------
void	neogeo_init(void);
void	neogeo_reset(void);
void	neogeo_shutdown(void);
void	MC68000_Cause_Interrupt(int);
void	neogeo_exception(void);
void	neogeo_run(void);
void	draw_main(void);
void	neogeo_quit(void);
void	not_implemented(void);
void	neogeo_machine_settings(void);
void	neogeo_debug_mode(void);
void	neogeo_cdda_check(void);
void	neogeo_cdda_control(void);
void	neogeo_do_cdda( int command, int trck_number_bcd);
void	neogeo_read_gamename(void);

void neogeo_initvar(){
	fps_framecount=0;
	neogeo_frameskip_count=1;
	neogeo_frame_count=1;
	num_resets=0;
	neogeo_soundmute=0;
	z80_cycles_inited=0;
	neogeo_prio_mode = 0;
	neogeo_ipl_done = 0;
	
	mame_debug = 0;
	previouspc = 0;
	ophw = 0;
	cur_mrhard = 0;
}

void print_frameskip(void)
{
}

void neogeo_setregion(){
	m68k_write_memory_8(0x10FD83,neogeo_region);
}

void init_autoframeskip(void)
{		
	/* update neocd time */			
	neo4all_skip_next_frame=0;

  
  /*timer & z80 stuff*/
	//my_timer_init();
	my_timer(); 
	z80_cycles = neo4all_z80_cycles/nb_interlace;   
	

  /*timing for fskip & speed limit*/
	sceKernelLibcGettimeofday( &tv_next1, 0 );
	tv_next1.tv_usec += 1000000/FPS;
		if ( tv_next1.tv_usec >= 1000000 ){
			tv_next1.tv_sec += tv_next1.tv_usec / 1000000;
			tv_next1.tv_usec %= 1000000;
		}
		
  fps_framecount=0;
  sceKernelLibcGettimeofday( &tv_start, 0 );
}

static unsigned char startup_bin[3328];

void ram_startup(int passed)
{
	memset(neogeo_prg_memory,0, 0x200000);

	memset(subcpu_memspace,0, 0x10000);

	if (!passed)
	{
		passed++;
		// Load startup RAM
		sprintf(tmp_str,"%s%s",launchDir,"startup.bin");
		FILE *fp = fopen(tmp_str,"rb");		
		if (fp==NULL) {
			//console_println();
			//console_printf("Fatal Error: Could not load STARTUP.BIN\n");
			//console_wait();
			ErrorMsg("Fatal Error: Could not load STARTUP.BIN\n");pgwaitPress();
			//exit(0);
			ExitCallback();
		}
		fread((void *)&startup_bin,1,3328,fp);
		fclose(fp);
		swab(startup_bin, startup_bin, 3328);
	}
	memcpy(neogeo_prg_memory + 0x10F300, (void *)&startup_bin, 3328);
}


static void now_loading(void)
{		
	sprintf(tmp_str,"%s%s",launchDir,"loading.bmp");
	FILE *fd = fopen(tmp_str,"rb");	
	if (fd==NULL) {		
		ErrorMsg("can't load loading image\n");
		return;
	}			
	IMAGE* img = load_bmp(fd);
	fclose(fd);
	if (img) {
		image_put((SCREEN_WIDTH-img->width)/2,(SCREEN_HEIGHT-img->height)/2,img,0);
		image_free(img);
	}		
}

//----------------------------------------------------------------------------
int	neo_main()
{
	FILE			*fp;
	int				result;
	//int				a, b, c,w,h,i;
	
	neogeo_initvar();
	
	neogeo_emulating=0;
	neogeo_ipl_done=0;
	char * fixtmp;


	// Initialise SDL
//	if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_CDROM|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK)==-1)) { 
/*	if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_CDROM|SDL_INIT_JOYSTICK)==-1)) { 
            console_printf("Could not initialize SDL: %s.\n", SDL_GetError());
            exit(-1);
        }*/
        g_init();

	// Register exit procedure
	//atexit(neogeo_shutdown);

	
	// Video init
	result = video_init();
	if (!result)
	{
		video_mode = 0;
		debug_log(global_error);
		return 0;
	}

	//show_icon();

	// Allocate needed memory buffers
	
	debug_log("NEOGEO: Allocating memory...");

	neogeo_prg_memory = calloc(1, 0x200000);
	if (neogeo_prg_memory==NULL) {
		ErrorMsg("failed allocating prg memory!");
		return 0;
	}
	
	

	neogeo_spr_memory = calloc(1, 0x400000);
	if (neogeo_spr_memory==NULL) {
		ErrorMsg("failed allocating spr memory!");
		return 0;
	}
	
	neogeo_rom_memory = calloc(1, 0x80000);
	if (neogeo_rom_memory==NULL) {
		ErrorMsg("failed allocating rom memory!");
		return 0;
	}

	neogeo_fix_memory = /*video_buffer+VIDEOBUF_PITCH*224+4096*2;*/calloc(1, 0x20000);
	if (neogeo_fix_memory==NULL) {
		ErrorMsg("failed allocating fix memory!");
		return 0;
	}

	neogeo_pcm_memory = calloc(1, 0x100000);
	if (neogeo_pcm_memory==NULL) {
		ErrorMsg("failed allocating pcm memory!");
		return 0;
	}
	
	

	// Initialize Memory Mapping
	initialize_memmap();
	memcard_init();

	// Load BIOS
	sprintf(tmp_str,"%s%s",launchDir,"neocd.bin");
	fp = fopen(tmp_str,"rb");
	if (fp==NULL) {

		//console_println();
		//console_printf("Fatal Error: Could not load NEOCD.BIN\n");
		//console_wait();
		ErrorMsg("Fatal Error: Could not load NEOCD.BIN\n");

/*		init_text(0);
		drawNoBIOS();*/

		return 0;
	}
	fread(neogeo_rom_memory, 1, 0x80000, fp);
	fclose(fp);
	


	fixtmp=calloc(1, 65536);
	memcpy(fixtmp,&neogeo_rom_memory[458752],65536);
	
	fix_conv(fixtmp,&neogeo_rom_memory[458752],65536,rom_fix_usage);
	/*swab(neogeo_rom_memory, neogeo_rom_memory, 131072);*/
	free(fixtmp);
	fixtmp=NULL;
	


	ram_startup(0);

	// Check BIOS validity
	if (*((short*)(neogeo_rom_memory+0xA822)) != 0x4BF9)
	{
		//console_println();
		//console_printf("Fatal Error: Invalid BIOS file.");
		//console_wait();
		ErrorMsg("Fatal Error: Invalid BIOS file.\n");
		return 0;
	}



	now_loading();


	/*** Patch BIOS load files w/ now loading message ***/
	*((short*)(neogeo_rom_memory+0x552)) = 0xFABF;
	*((short*)(neogeo_rom_memory+0x554)) = 0x4E75;
	/*** Patch BIOS load files w/out now loading ***/
	*((short*)(neogeo_rom_memory+0x564)) = 0xFAC0;
	*((short*)(neogeo_rom_memory+0x566)) = 0x4E75;
	/*** Patch BIOS CDROM Check ***/
	*((short*)(neogeo_rom_memory+0xB040)) = 0x4E71;
	*((short*)(neogeo_rom_memory+0xB042)) = 0x4E71;
	/*** Patch BIOS upload command ***/
	*((short*)(neogeo_rom_memory+0x546)) = 0xFAC1;
	*((short*)(neogeo_rom_memory+0x548)) = 0x4E75;

	/*** Patch BIOS CDDA check ***/
	*((short*)(neogeo_rom_memory+0x56A)) = 0xFAC3;
	*((short*)(neogeo_rom_memory+0x56C)) = 0x4E75;

	/*** Full reset, please ***/
	*((short*)(neogeo_rom_memory+0xA87A)) = 0x4239;
	*((short*)(neogeo_rom_memory+0xA87C)) = 0x0010;
	*((short*)(neogeo_rom_memory+0xA87E)) = 0xFDAE;

	/*** Trap exceptions ***/
	*((short*)(neogeo_rom_memory+0xA5B6)) = 0x4AFC;

	// Initialise input
	input_init();


	// Sound init
	init_sdl_audio();
	z80_init();


	//init_text(1);

	if (!neogeo_cdrom_init1())
		ExitCallback();


	//console_pag();
	//console_println();
	debug_log(VERSION1);
	debug_log(VERSION2);
	//console_println();

	//cdda_current_drive=neogeo_cdrom_current_drive;
	cdda_init();
	

	// Initialize everything
	neogeo_init();
	neogeo_run();
	neogeo_shutdown();
	return 0;
}

//----------------------------------------------------------------------------
void	neogeo_init(void)
{
	_68k_init();
	_68k_reset();
}

//----------------------------------------------------------------------------
int	neogeo_hreset(void)
{	
	//console_pag();
	//console_println();
	debug_log("Reset!\n");
	if (num_resets) sound_unmute();
	
	
	
	pd4990a_init();
	
	
	
	video_reset();
	
	
	
	//input_reset();
/*#ifdef Z80_EMULATED
	_z80reset();
	YM2610_sh_reset();
	AY8910_reset();
	_z80exec(100000);
	my_timer();
	_z80exec(100000);
#endif*/



	ram_startup(1);
	if (num_resets)
	{
//		SDL_Delay(333);
		sound_mute();
	}


	
	neogeo_ipl_done = 0;
	while(!neogeo_ipl_done)
	{
		unsigned fmax=0;
		
		neogeo_cdrom_load_title();
		
		if (!neogeo_cdrom_process_ipl(0)) {
			//console_println();
			//console_printf("Error: Error while processing IPL.TXT.\n");
			//console_wait();
			ErrorMsg("Error: Error while processing IPL.TXT.\n");
			ExitCallback();

		}
		else neogeo_ipl_done = 1;
	}

	FILE * fp;		
	/* read game name */
	neogeo_read_gamename();

	/* Special patch for Samurai Spirits RPG */
	if (strcmp(config_game_name, "TEST PROGRAM USA") == 0)
	{
		strcpy(config_game_name, "SAMURAI SPIRITS RPG");
		
		sprintf(tmp_str,"%s%s",launchDir,"patch.prg");
		fp = fopen(tmp_str,"rb");		
		if (fp == NULL) {
			//console_printf("Fatal Error: Couldnt open patch.prg.\n");
			//SDL_Delay(1000);
			ExitCallback();
		}
		

		fread(neogeo_prg_memory + 0x132000, 1, 112, fp);
		fclose(fp);
		swab(neogeo_prg_memory + 0x132000, neogeo_prg_memory + 0x132000, 112);
	
	}


	/* update window title with game name */
	strcat(neocd_wm_title," - ");
	strcat(neocd_wm_title,config_game_name);
	//SDL_WM_SetCaption(neocd_wm_title,neocd_wm_title);


	// First time init
	_68k_reset();
	_68k_set_register(_68K_REG_PC,0xc0a822);
	_68k_set_register(_68K_REG_SR,0x2700);
	_68k_set_register(_68K_REG_A7,0x10F300);
//	_68k_set_register(_68K_REG_ASP,0x10F400);
	
	m68k_write_memory_32(0x10F6EE, m68k_read_memory_32(0x68L)); // $68 *must* be copied at 10F6EE

	if (m68k_read_memory_8(0x107)&0x7E)
	{
		if (m68k_read_memory_16(0x13A))
		{
			m68k_write_memory_32(0x10F6EA, (m68k_read_memory_16(0x13A)<<1) + 0xE00000);
		}
		else
		{
			m68k_write_memory_32(0x10F6EA, 0);
			m68k_write_memory_8(0x00013B, 0x01);
		}
	}
	else
		m68k_write_memory_32(0x10F6EA, 0xE1FDF0);

	/* Set System Region */
	neogeo_setregion();


	cdda_current_track = 0;
	cdda_get_disk_info();


	z80_init(); 	
	YM2610_sh_reset();
	AY8910_reset();

	//if (!num_resets) menu_unraise();
	sound_unmute();

	init_autoframeskip();
  z80_cycles_inited=0;

	num_resets++;
	return 0;
}	

//----------------------------------------------------------------------------
void	neogeo_reset(void)
{

	_68k_reset();
	_68k_set_register(_68K_REG_PC,0x122);
	_68k_set_register(_68K_REG_SR,0x2700);
	_68k_set_register(_68K_REG_A7,0x10F300);
//	_68k_set_register(_68K_REG_ASP,0x10F400);

	m68k_write_memory_8(0x10FD80, 0x82);
	m68k_write_memory_8(0x10FDAF, 0x01);
	m68k_write_memory_8(0x10FEE1, 0x0A);
	m68k_write_memory_8(0x10F675, 0x01);
	m68k_write_memory_8(0x10FEBF, 0x00);
	m68k_write_memory_32(0x10FDB6, 0);
	m68k_write_memory_32(0x10FDBA, 0);

	/* System Region */
	neogeo_setregion();

	cdda_current_track = 0;

	z80_init(); 

}

//----------------------------------------------------------------------------
void	neogeo_shutdown(void)
{
	FILE	*fp;
	
	//console_println();
	//console_printf("NEOGEO: System Shutdown.\n");
	debug_log("NEOGEO: System Shutdown.\n");
	
	
	// Close everything and free memory
	cdda_shutdown();
	neogeo_cdrom_shutdown();

	sound_shutdown();

	input_shutdown();
	video_shutdown();
	memcard_shutdown();

	free(neogeo_prg_memory);
	free(neogeo_rom_memory);
	free(neogeo_spr_memory);
	free(neogeo_fix_memory);
	free(neogeo_pcm_memory);
	
	//SDL_Quit();
	
	return;
}

//----------------------------------------------------------------------------
void	neogeo_exception(void)
{
	//console_printf("NEOGEO: Exception Trapped at %08x !\n", previouspc);
	//SDL_Delay(1000);
	ExitCallback();
	//exit(0);
}	

//----------------------------------------------------------------------------
/*
void MC68000_Cause_Interrupt(int level)
{
	_68k_raise_interrupt(level,M68K_AUTOVECTORED_INT);
	_68k_emulate(10000);
	_68k_lower_interrupt(level);
}
*/

//----------------------------------------------------------------------------
void	neogeo_exit(void)
{
	ErrorMsg("NEOGEO: Exit requested by software...");
	//SDL_Delay(1000);
	//exit(0);
	ExitCallback();
}

void neogeo_adjust_frameskip(int new_frameskip)
{
	neogeo_frameskip=new_frameskip;
	neogeo_frameskip_count=0;
	init_autoframeskip();
}

void neogeo_adjust_cycles(int new_68k, int new_z80)
{
	switch(new_68k)
	{
		case 0:
			neo4all_68k_cycles=NEO4ALL_68K_UNDER_CYCLES;
			break;
		case 2:
			neo4all_68k_cycles=NEO4ALL_68K_OVER_CYCLES;
			break;
		default:
			neo4all_68k_cycles=NEO4ALL_68K_NORMAL_CYCLES;
	}
	switch(new_z80)
	{
		case 0:
			neo4all_z80_cycles=NEO4ALL_Z80_UNDER_CYCLES;
			break;
		case 2:
			neo4all_z80_cycles=NEO4ALL_Z80_OVER_CYCLES;
			break;
		default:
			neo4all_z80_cycles=NEO4ALL_Z80_NORMAL_CYCLES;
	}
}

//----------------------------------------------------------------------------
void	neogeo_run(void)
{		
    	int	i;unsigned int	diff;

	neogeo_emulating=1;
	debug_log("NEOGEO RUN ...");
	
	

	// If IPL.TXT not loaded, load it !
	if (!neogeo_ipl_done)	while(neogeo_hreset()) ;
			//drawNoNeoGeoCD();

	
	/* sound stuff*/	
	InitSoundThread();
			

	// Main loop
	unsigned long long ciclo=0;
	
	
	
	init_autoframeskip();
	debug_log("Starting!\n");
			
	while(bLoop)
	{
		if (bSleep){
			
			while(bSleep) pgWaitV();			
			
			pgWaitVn(120);
					
			if (cdda_playing) {				
				debug_log("cdda was playing");				
				int trk=cdda_current_track;
				cdda_current_track=0;
				debug_log("launch");
				cdda_play(trk);				
				if (cdda_playing==2) {
					debug_log("pause track");
					cdda_pause();
				}
			}
			
			init_autoframeskip();			
		}



	if (neogeo_sound_enable){		
		mz80int(0); 
         	for (i=0; i<nb_interlace; i++) { 
            	  if (z80_cycles>0) { 
               	    mz80exec(z80_cycles); 
               	    z80_cycles=0; 
                    my_timer(); 
                  } 
                  z80_cycles += neo4all_z80_cycles/nb_interlace; 
                } 
	}



		// One-vbl timeslice

		
		
		_68k_emulate(neo4all_68k_cycles);
		_68k_interrupt(2);


		// update pd4990a
		pd4990a_addretrace();
		

		// check the watchdog 
		if (watchdog_counter > 0) {
		    if (--watchdog_counter == 0) {
			//logerror("reset caused by the watchdog\n");
			neogeo_reset();
		    }
		}

		memcard_update();

		// Call display routine


		if (neogeo_prio_mode)
			video_draw_screen2();
		else
			video_draw_screen1();


		
		//pgPrintHex(50,0,31,psp_mem_avail(),1);
		
		// Check if there are pending commands for CDDA
		neogeo_cdda_check();
		//cdda_loop_check();

		/* Update keys and Joystick */
		processEvents();
		
		
		if (neogeo_speedlimit){
			sceKernelLibcGettimeofday( &tv_now, 0 );		
			
			if ( timercmp( &tv_next1, &tv_now, < ) ){
				unsigned int lag;
				lag = (tv_now.tv_sec - tv_next1.tv_sec) * 1000000 + tv_now.tv_usec - tv_next1.tv_usec;
				if ( lag >= 1000000 ){
					tv_next1 = tv_now;
				}
			}
			
			while ( timercmp( &tv_next1, &tv_now, > ) ){
				sceKernelLibcGettimeofday( &tv_now, 0 );
			}
			tv_next1.tv_usec += 1000000/FPS;
			if ( tv_next1.tv_usec >= 1000000 ){
				tv_next1.tv_sec += tv_next1.tv_usec / 1000000;
				tv_next1.tv_usec %= 1000000;
			}
		}
		

	/* show fps */
	if (neogeo_showfps){
		fps_framecount++;
		sceKernelLibcGettimeofday( &tv_now, 0 );	
		diff  = (tv_now.tv_sec - tv_start.tv_sec) * 1000000 + tv_now.tv_usec - tv_start.tv_usec;
		diff /= 1000000;
		if ( diff ){
			char			buf[128];
			buf[0] = ((fps_framecount / diff) / 100) + '0';			
			buf[1] = (((fps_framecount / diff) / 10)% 10) + '0';
			buf[2] = ((fps_framecount / diff) % 10) + '0';
			buf[3] = 'F';
			buf[4] = 'P';
			buf[5] = 'S';
			buf[6] = '\0';
			pgPrint( CMAX_X - 7, 0, 0xffff, buf ,1);
			tv_start = tv_now;
			fps_framecount  = 0;
		}
	}

		/* Speed Throttle */	
		switch(neogeo_frameskip){
			case 0:
				neo4all_skip_next_frame=0;
				break;
			case 1:
				if (!(neogeo_frameskip_count&0x1))
					neo4all_skip_next_frame=0;
				else
					neo4all_skip_next_frame=1;
				break;
			case 2:
				if (!(neogeo_frameskip_count%3))
					neo4all_skip_next_frame=0;
				else
					neo4all_skip_next_frame=1;
				break;
			case 3:
				if (!(neogeo_frameskip_count&0x3))
					neo4all_skip_next_frame=0;
				else
					neo4all_skip_next_frame=1;
				break;
			case 4:
				if (!(neogeo_frameskip_count%5))
					neo4all_skip_next_frame=0;
				else
					neo4all_skip_next_frame=1;
				break;
			case 5:
				if (!(neogeo_frameskip_count%6))
					neo4all_skip_next_frame=0;
				else
					neo4all_skip_next_frame=1;
				break;
			default:								//autofskip
				break;
		}
		neogeo_frameskip_count++;
		neogeo_frame_count++;

	}
	// Stop CDDA
	cdda_stop();
	StopSoundThread();
		
	return;
}

//----------------------------------------------------------------------------
// This is a really dirty hack to make SAMURAI SPIRITS RPG work
void	neogeo_prio_switch(void)
{
	if (_68k_get_register(_68K_REG_D7) == 0xFFFF)
		return;
	
	if (_68k_get_register(_68K_REG_D7) == 9 && 
	    _68k_get_register(_68K_REG_A3) == 0x10DED9 &&
		(_68k_get_register(_68K_REG_A2) == 0x1081d0 ||
		(_68k_get_register(_68K_REG_A2)&0xFFF000) == 0x102000)) {
		neogeo_prio_mode = 0;
		return;
	}
	
	if (_68k_get_register(_68K_REG_D7) == 8 && 
	    _68k_get_register(_68K_REG_A3) == 0x10DEC7 && 
		_68k_get_register(_68K_REG_A2) == 0x102900) {
		neogeo_prio_mode = 0;
		return;
	}
	
	if (_68k_get_register(_68K_REG_A7) == 0x10F29C)
	{
		if ((_68k_get_register(_68K_REG_D4)&0x4010) == 0x4010)
		{
			neogeo_prio_mode = 0;
			return;
		}
		
		neogeo_prio_mode = 1;
	}
	else
	{
		if (_68k_get_register(_68K_REG_A3) == 0x5140)
		{
			neogeo_prio_mode = 1;
			return;
		}

		if ( (_68k_get_register(_68K_REG_A3)&~0xF) == (_68k_get_register(_68K_REG_A4)&~0xF) )
			neogeo_prio_mode = 1;
		else
			neogeo_prio_mode = 0;
	}
}

//----------------------------------------------------------------------------
void	not_implemented(void)
{
		//console_printf("Error: This function isn't implemented.");
}

//----------------------------------------------------------------------------
void	neogeo_quit(void)
{
		//exit(0);
		ExitCallback();
}

//----------------------------------------------------------------------------
void neogeo_cdda_check(void)
{
#ifdef ENABLE_CDDA
	int		Offset;
	
	Offset = m68k_read_memory_32(0x10F6EA);
	if (Offset < 0xE00000)	// Invalid addr
		return;

	Offset -= 0xE00000;
	Offset >>= 1;


	if (neogeo_sound_enable)
		neogeo_do_cdda(subcpu_memspace[Offset&0xFFFF], subcpu_memspace[(Offset+1)&0xFFFF]);


#endif
}


//----------------------------------------------------------------------------
void neogeo_cdda_control(void)
{
#ifdef ENABLE_CDDA
	neogeo_do_cdda( (_68k_get_register(_68K_REG_D0)>>8)&0xFF, 
	                 _68k_get_register(_68K_REG_D0)&0xFF );
#endif
}

//----------------------------------------------------------------------------
void neogeo_do_cdda( int command, int track_number_bcd)
{
#ifdef ENABLE_CDDA
	int		track_number;
	int		offset;

	if ((command == 0)&&(track_number_bcd == 0))
		return;

	m68k_write_memory_8(0x10F64B, track_number_bcd);
	m68k_write_memory_8(0x10F6F8, track_number_bcd);
	m68k_write_memory_8(0x10F6F7, command);
	m68k_write_memory_8(0x10F6F6, command);

	offset = m68k_read_memory_32(0x10F6EA);

	if (offset)
	{
		offset -= 0xE00000;
		offset >>= 1;

		m68k_write_memory_8(0x10F678, 1);


		if (neogeo_sound_enable)
		{
			subcpu_memspace[offset&0xFFFF] = 0;
			subcpu_memspace[(offset+1)&0xFFFF] = 0;
		}

	}

	switch( command )
	{
		case	0:
		case	1:
		case	5:
		case	4:
		case	3:
		case	7:
			track_number = ((track_number_bcd>>4)*10) + (track_number_bcd&0x0F);
			if ((track_number == 0)&&(!cdda_playing))
			{
				//sound_mute();
				cdda_resume();
			}
			else if ((track_number>1)&&(track_number<99))
			{
				//sound_mute();
				cdda_play(track_number);
				cdda_autoloop = !(command&1);
			}
			break;
		case	6:
		case	2:
			if (cdda_playing)
			{
				//sound_mute();
				cdda_pause();
			}
			break;
	}
#endif
}

//----------------------------------------------------------------------------
void neogeo_read_gamename(void)
{

	unsigned char	*Ptr;
	int				temp;

	Ptr = neogeo_prg_memory + m68k_read_memory_32(0x11A);
	swab(Ptr, config_game_name, 80);

	for(temp=0;temp<80;temp++) {
		if (!isprint(config_game_name[temp])) {
			config_game_name[temp]=0;
			break;
		}
	}
}

