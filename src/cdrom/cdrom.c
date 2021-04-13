/**************************************
****    CDROM.C  -  File reading   ****
**************************************/

//-- Include files -----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
//#include <ctype.h>
//#include <string.h>

#include "cdrom.h"
#include "../neocd.h"



#define LOWERCASEFILES

#define CHANGECASE  tolower
#define IPL_TXT     "ipl.txt"
#define PRG     "prg"
#define FIX     "fix"
#define SPR      "spr"
#define Z80      "z80"
#define PAT      "pat"
#define PCM      "pcm"
#define JUE      "jue"
#define TITLE_X_SYS "title_x.sys"


/*-- Definitions -----------------------------------------------------------*/
#define    BUFFER_SIZE    131072
#define PRG_TYPE    0
#define FIX_TYPE    1
#define SPR_TYPE    2
#define Z80_TYPE    3
#define PAT_TYPE    4
#define PCM_TYPE    5
#define    min(a, b) ((a) < (b)) ? (a) : (b)

/*-- Exported Functions ----------------------------------------------------*/
int    neogeo_cdrom_init1(void);
int    neogeo_cdrom_load_prg_file(char *, unsigned int);
int    neogeo_cdrom_load_z80_file(char *, unsigned int);
int    neogeo_cdrom_load_fix_file(char *, unsigned int);
int    neogeo_cdrom_load_spr_file(char *, unsigned int);
int    neogeo_cdrom_load_pcm_file(char *, unsigned int);
int    neogeo_cdrom_load_pat_file(char *, unsigned int, unsigned int);
void    neogeo_cdrom_load_files(void);
int    neogeo_cdrom_process_ipl(int check);
void    neogeo_cdrom_shutdown(void);
void    fix_conv(unsigned char *, unsigned char *, int, unsigned char *);
void    spr_conv(unsigned char *, unsigned char *, int);//, unsigned char *);
void    neogeo_upload(void);
void    neogeo_cdrom_load_title(void);
void    neogeo_cdrom_apply_patch(short *source, int offset, int bank);

//-- Private Variables -------------------------------------------------------
static char    neogeo_cdrom_buffer[BUFFER_SIZE];


//-- Private Function --------------------------------------------------------
static    int    recon_filetype(char *);

//-- Exported Variables ------------------------------------------------------

int        img_display = 1;

extern char ziparchive_filename[256];
//----------------------------------------------------------------------------
int    neogeo_cdrom_init1(void)
{    
    
    
    int zfd = zip_open(ziparchive_filename);
		if (zfd==-1) {
			debug_log("can't open zip\n");		
			debug_log("trying uncompressed mode\n");		
			return 1;
		}
//    loading_pict=SDL_LoadBMP("loading.bmp");
//		extract_init();
    return 1;    
}

//----------------------------------------------------------------------------
void    neogeo_cdrom_shutdown(void)
{
    /* free loading picture surface ??? */
    zip_close();
}

//----------------------------------------------------------------------------
int    neogeo_cdrom_load_prg_file(char *FileName, unsigned int Offset)
{
    int    fp;    
    char    *Ptr;
    int        Readed;

//    sound_disable();


    fp = zopen(FileName);
    if (fp==-1) {
        ErrorMsg("Could not open %s");//, Path);
//    	sound_enfable();
        return 0;
    }
    
    Ptr = neogeo_prg_memory + Offset;
    
    do {
        Readed = zread(fp,neogeo_cdrom_buffer,  BUFFER_SIZE);
        swab(neogeo_cdrom_buffer, Ptr, Readed);
        Ptr += Readed;
    } while(Readed==BUFFER_SIZE);//Readed==BUFFER_SIZE &&
    
    zclose(fp);

//    sound_enable();
    return 1;
}

//----------------------------------------------------------------------------
int    neogeo_cdrom_load_z80_file(char *FileName, unsigned int Offset)
{
#ifdef Z80_EMULATED
    int fp;
    

//    sound_disable();

    fp = zopen(FileName);
    if (fp==-1) {
        ErrorMsg("Could not open %s");//, Path);
//    	sound_enable();
        return 0;
    }

    zread(fp, &subcpu_memspace[Offset],  0x10000-Offset);
    

    zclose(fp);
//    sound_enable();

#endif
    return 1;
}

//----------------------------------------------------------------------------
int    neogeo_cdrom_load_fix_file(char *FileName, unsigned int Offset)
{
    int fp;    
    char    *Ptr, *Src;
    int        Readed;

//    sound_disable();
    
    fp = zopen(FileName);
    if (fp==-1) {
        ErrorMsg("Could not open %s");//, Path);
//    	sound_enable();
        return 0;
    }
    
    Ptr = neogeo_fix_memory + Offset;
    
    do {
        memset(neogeo_cdrom_buffer, 0, BUFFER_SIZE);
        Readed = zread(fp,neogeo_cdrom_buffer, BUFFER_SIZE);
        Src = neogeo_cdrom_buffer;
        fix_conv(Src, Ptr, Readed, video_fix_usage + (Offset>>5));
        Ptr += Readed;
        Offset += Readed;
    } while( Readed == BUFFER_SIZE );
    
    zclose(fp);
    
    blit_fix_resetcache();
    
//    sound_enable();

    return 1;
}

//----------------------------------------------------------------------------
int    neogeo_cdrom_load_spr_file(char *FileName, unsigned int Offset)
{
    int fp;
    
    char    *Ptr;
    int        Readed;


//    sound_disable();

    fp = zopen(FileName);
    if (fp==-1) {
        ErrorMsg( "Could not open %s");//, Path);
//    	sound_enable();
        return 0;
    }
    
    Ptr = neogeo_spr_memory + Offset;
    
    do {
        memset(neogeo_cdrom_buffer, 0, BUFFER_SIZE);
        Readed = zread(fp,neogeo_cdrom_buffer,  BUFFER_SIZE);

        spr_conv(neogeo_cdrom_buffer, Ptr, Readed);//, video_spr_usage + (Offset>>7));
        Offset += Readed;
        Ptr += Readed;
    } while( Readed == BUFFER_SIZE );
    
    
    
    zclose(fp);
    
//    sound_enable();

    return 1;
}

//----------------------------------------------------------------------------
int    neogeo_cdrom_load_pcm_file(char *FileName, unsigned int Offset)
{
    int fp;
    
    char        *Ptr;


//    sound_disable();

    fp = zopen(FileName);
    if (fp==-1) {
        ErrorMsg("Could not open %s");//, Path);
//    	sound_enable();
        return 0;
    }

    Ptr = neogeo_pcm_memory + Offset;

    zread(fp,Ptr,  0x100000-Offset);

    zclose(fp);

//    sound_enable();

    return 1;
}

//----------------------------------------------------------------------------
int    neogeo_cdrom_load_pat_file(char *FileName, unsigned int Offset, unsigned int Bank)
{
    int fp;
    
    int        Readed;


//    sound_disable();
    
    
    fp = zopen(FileName);
    if (fp==-1) {
        ErrorMsg( "Could not open %s");//, Path);
//    	sound_enable();
        return 0;
    }
    
    Readed = zread(fp,neogeo_cdrom_buffer, BUFFER_SIZE);

    swab(neogeo_cdrom_buffer, neogeo_cdrom_buffer, Readed);
    neogeo_cdrom_apply_patch((short*)neogeo_cdrom_buffer, Offset, Bank);

    zclose(fp);

//    sound_enable();

    return 1;
}




int hextodec(char c) {
	switch (tolower(c)) {
	case '0':	return 0;
	case '1':	return 1;
	case '2':	return 2;
	case '3':	return 3;
	case '4':	return 4;
	case '5':	return 5;
	case '6':	return 6;
	case '7':	return 7;
	case '8':	return 8;
	case '9':	return 9;
	case 'a':	return 10;
	case 'b':	return 11;
	case 'c':	return 12;
	case 'd':	return 13;
	case 'e':	return 14;
	case 'f':	return 15;
	default:	return 0;
	}
}



//----------------------------------------------------------------------------
int    neogeo_cdrom_process_ipl(int check)
{
    int fp;    
    char    Line[32];
    char    FileName[16];
    int        FileType;
    int        Bnk;
    int        Off;
    int        i, j;
    int	       ret=0;
    char	buf[1024];

    sound_disable();
    
    debug_log("open ipl\n");
        
    fp = zopen(IPL_TXT);
    
    if (fp==-1) {
        ErrorMsg("Could not open IPL.TXT!");
    	sound_enable();
        return 0;
    }
    
    int len = zread(fp,buf,sizeof(buf));
		buf[len] = 0;
		
		zclose(fp);
		
		debug_log("read ipl ok\n");
		
		char* bufp = buf;
    while (1)
    {
    
    	char* p = strchr(bufp,'\n');
    	if (p==NULL) break;    	
    	p++;
    	int len = p-bufp;
    	memcpy(Line,bufp, len);
    	Line[len] = 0;
    	bufp = p;
    
        Bnk=0;
		Off=0;

		processEvents();
    
        i=0;
        j=0;
        while((Line[i] != ',')&&(Line[i]!=0))
            FileName[j++] = CHANGECASE(Line[i++]);
        FileName[j]=0;

        j -= 3;
        if (j>0) {
            FileType = recon_filetype(&FileName[j]);
            i++;
            j=0;
            while(Line[i] != ',') {
                Bnk*=10;
				Bnk+=Line[i]-'0';
				i++;
			}

            i++;
            j=0;

            while(Line[i] != 0x0D) {
			    Off*=16;
				Off+=hextodec(Line[i++]);
			}

            Bnk &= 3;

	    ret++;
	    /*if (!check)
	    {
		strcpy(Path, cdpath);
		strcat(Path, FileName);
		FILE *f=fopen_s(Path,"rb");
		if (!f)
			return 0;
		fclose(f);
	    }
	    else*/
	    {
            	switch( FileType ) {
            	  case PRG_TYPE:
                	if (!neogeo_cdrom_load_prg_file(FileName, Off)) {
                    		zclose(fp);
    		    		sound_enable();
                    		return 0;
                	}
               		break;
            	  case FIX_TYPE:
                	if (!neogeo_cdrom_load_fix_file(FileName, (Off>>1))) {
                    		zclose(fp);
    		    		sound_enable();
                   		return 0;
                	}
                	break;
            	  case SPR_TYPE:
                	if (!neogeo_cdrom_load_spr_file(FileName, (Bnk*0x100000) + Off)) {
                    		zclose(fp);
    		    		sound_enable();
                    		return 0;
                	}
                	break;
            	  case Z80_TYPE:
                	if (!neogeo_cdrom_load_z80_file(FileName, (Off>>1))) {
                    		zclose(fp);
    		    		sound_enable();
                    		return 0;
                	}
                	break;
            	  case PAT_TYPE:
                	if (!neogeo_cdrom_load_pat_file(FileName, Off, Bnk)) {
                    		zclose(fp);
    		    		sound_enable();
                    		return 0;
                	}
                	break;
            	  case PCM_TYPE:
                	if (!neogeo_cdrom_load_pcm_file(FileName, (Bnk*0x80000) + (Off>>1))) {
                   		zclose(fp);
    		    		sound_enable();
                    		return 0;
                	}
                	break;
            	}
	    }
        }
    }
  	zclose(fp);    
//#ifndef USE_VIDEO_GL
    video_draw_blank();
//#endif
    
    sound_enable();
    init_autoframeskip();
    z80_cycles_inited=0;
    return ret;
}

//----------------------------------------------------------------------------
int    recon_filetype(char *ext)
{    
    if (strcmp(ext, PRG)==0)
        return PRG_TYPE;
    
    if (strcmp(ext, FIX)==0)
        return FIX_TYPE;
    
    if (strcmp(ext, SPR)==0)
        return SPR_TYPE;
        
    if (strcmp(ext, Z80)==0)
        return Z80_TYPE;
        
    if (strcmp(ext, PAT)==0)
        return PAT_TYPE;
    
    if (strcmp(ext, PCM)==0)
        return PCM_TYPE;
        
    return    -1;
}


//----------------------------------------------------------------------------
unsigned int motorola_peek(unsigned char *address) 
{
    unsigned int a,b,c,d;
	
	a=address[0]<<24;
	b=address[1]<<16;
	c=address[2]<<8;
	d=address[3]<<0;
	
	return (a|b|c|d);
}

#ifdef PROFILER
extern Uint32 profiler_actual;
#endif

static unsigned neogeo_cdrom_test_files(int check)
{
    unsigned ret=0;
    char    Entry[32], FileName[13];
    char    *Ptr, *Ext;
    int     i, j, Bnk, Off, Type, Reliquat;

    Ptr = neogeo_prg_memory + _68k_get_register(_68K_REG_A0);

    do {
        Reliquat = ((int)Ptr)&1;

        if (Reliquat)
            Ptr--;

        swab(Ptr, Entry, 32);
        i=Reliquat;

        while((Entry[i]!=0)&&(Entry[i]!=';')) {
            FileName[i-Reliquat] = CHANGECASE(Entry[i]);
            i++;
        }

        FileName[i-Reliquat] = 0;

        if (Entry[i]==';')    /* 01/05/99 MSLUG2 FIX */
            i += 2;

        i++;

        Bnk = Entry[i++]&3;

        if (i&1)
            i++;


        Off = motorola_peek(&Entry[i]);
        i += 4;
        Ptr += i;


        j=0;

        while(FileName[j] != '.' && FileName[j] != '\0')
            j++;

        if(FileName[j]=='\0')
        {
	    if (!check)
		return 0;
            //console_printf("Internal Error loading file: %s",FileName);
	    //SDL_Delay(1000);
            ExitCallback();
        }

        j++;
        Ext=&FileName[j];

	ret++;
	if (!check)
	{
		//char Path[256];
		//strcpy(Path, cdpath);
		//strcat(Path, FileName);
		int f=zopen(FileName);
		if (f==-1)
			return 0;
		zclose(f);
	}
	else
	{
        	//console_printf("Loading File: %s %02x %08x\n", FileName, Bnk, Off);
		//text_draw_loading(ret,check);
        	Type = recon_filetype(Ext);
        	switch( Type ) {
        	  case PRG_TYPE:
            		neogeo_cdrom_load_prg_file(FileName, Off);
            		break;
        	  case FIX_TYPE:
            		neogeo_cdrom_load_fix_file(FileName, Off>>1);
            		break;
        	  case SPR_TYPE:
            		neogeo_cdrom_load_spr_file(FileName, (Bnk*0x100000) + Off);
            		break;
        	  case Z80_TYPE:
            		neogeo_cdrom_load_z80_file(FileName, Off>>1);
            		break;
        	  case PAT_TYPE:
            		neogeo_cdrom_load_pat_file(FileName, Off, Bnk);
            		break;
        	  case PCM_TYPE:
            		neogeo_cdrom_load_pcm_file(FileName, (Bnk*0x80000) + (Off>>1));
            		break;
#ifdef DEBUG_CDROM
		  default:
			printf("!!!!ATENCION TYPE %i NO IMPLEMENTADO!!!!!!\n",Type);
#endif
        	}
        	processEvents();
	}
    } while( Entry[i] != 0);
    return ret;
}
//----------------------------------------------------------------------------
void    neogeo_cdrom_load_files(void)
{
    unsigned nfiles=0;

    sound_disable();
#ifdef PROFILER
    Uint32 ahora=SDL_GetTicks();
#endif
    
    if (m68k_read_memory_8(_68k_get_register(_68K_REG_A0))==0)
    {
    	sound_enable();
        return;
    }


    cdda_stop();
    //menu_raise();
    cdda_current_track = 0;

    m68k_write_memory_32(0x10F68C, 0x00000000);
    m68k_write_memory_8(0x10F6C3, 0x00);
    m68k_write_memory_8(0x10F6D9, 0x01);
    m68k_write_memory_8(0x10F6DB, 0x01);
    m68k_write_memory_32(0x10F742, 0x00000000);
    m68k_write_memory_32(0x10F746, 0x00000000);
    m68k_write_memory_8(0x10FDC2, 0x01);
    m68k_write_memory_8(0x10FDDC, 0x00);
    m68k_write_memory_8(0x10FDDD, 0x00);
    m68k_write_memory_8(0x10FE85, 0x01);
    m68k_write_memory_8(0x10FE88, 0x00);
    m68k_write_memory_8(0x10FEC4, 0x01);

    nfiles=neogeo_cdrom_test_files(0);

    if (nfiles)
    {
	/*if (nfiles>5)
		sound_play_loading();*/
	neogeo_cdrom_test_files(nfiles);
    }
/*
    else
ERROR !!!!!!!!!!!!!!!!!!
*/
	
#ifndef USE_VIDEO_GL
    	video_draw_blank();
#endif

    //menu_unraise();
    sound_enable();
    init_autoframeskip();
    z80_cycles_inited=0;
}

//----------------------------------------------------------------------------
void    fix_conv(unsigned char *Src, unsigned char *Ptr, int Taille,unsigned char *usage_ptr)
{
    int        i;
    unsigned char    usage;
    
    for(i=Taille;i>0;i-=32) {
        usage = 0;
        *Ptr++ = *(Src+16);
        usage |= *(Src+16);
        *Ptr++ = *(Src+24);
        usage |= *(Src+24);
        *Ptr++ = *(Src);
        usage |= *(Src);
        *Ptr++ = *(Src+8);
        usage |= *(Src+8);
        Src++;
        *Ptr++ = *(Src+16);
        usage |= *(Src+16);
        *Ptr++ = *(Src+24);
        usage |= *(Src+24);
        *Ptr++ = *(Src);
        usage |= *(Src);
        *Ptr++ = *(Src+8);
        usage |= *(Src+8);
        Src++;
        *Ptr++ = *(Src+16);
        usage |= *(Src+16);
        *Ptr++ = *(Src+24);
        usage |= *(Src+24);
        *Ptr++ = *(Src);
        usage |= *(Src);
        *Ptr++ = *(Src+8);
        usage |= *(Src+8);
        Src++;
        *Ptr++ = *(Src+16);
        usage |= *(Src+16);
        *Ptr++ = *(Src+24);
        usage |= *(Src+24);
        *Ptr++ = *(Src);
        usage |= *(Src);
        *Ptr++ = *(Src+8);
        usage |= *(Src+8);
        Src++;
        *Ptr++ = *(Src+16);
        usage |= *(Src+16);
        *Ptr++ = *(Src+24);
        usage |= *(Src+24);
        *Ptr++ = *(Src);
        usage |= *(Src);
        *Ptr++ = *(Src+8);
        usage |= *(Src+8);
        Src++;
        *Ptr++ = *(Src+16);
        usage |= *(Src+16);
        *Ptr++ = *(Src+24);
        usage |= *(Src+24);
        *Ptr++ = *(Src);
        usage |= *(Src);
        *Ptr++ = *(Src+8);
        usage |= *(Src+8);
        Src++;
        *Ptr++ = *(Src+16);
        usage |= *(Src+16);
        *Ptr++ = *(Src+24);
        usage |= *(Src+24);
        *Ptr++ = *(Src);
        usage |= *(Src);
        *Ptr++ = *(Src+8);
        usage |= *(Src+8);
        Src++;
        *Ptr++ = *(Src+16);
        usage |= *(Src+16);
        *Ptr++ = *(Src+24);
        usage |= *(Src+24);
        *Ptr++ = *(Src);
        usage |= *(Src);
        *Ptr++ = *(Src+8);
        usage |= *(Src+8);
        Src+=25;
        *usage_ptr++ = usage;
    }
}


//----------------------------------------------------------------------------
#define COPY_BIT(a, b) { \
    a <<= 1; \
    a |= (b & 0x01); \
    b >>= 1; }

void extract8(char *src, char *dst) 
{ 
   int i; 

   unsigned char bh = *src++;
   unsigned char bl = *src++;
   unsigned char ch = *src++;
   unsigned char cl = *src; 
   unsigned char al, ah; 

   for(i = 0; i < 4; i++)
   { 
      al = ah = 0; 

      COPY_BIT(al, ch) 
      COPY_BIT(al, cl) 
      COPY_BIT(al, bh) 
      COPY_BIT(al, bl) 

      COPY_BIT(ah, ch) 
      COPY_BIT(ah, cl) 
      COPY_BIT(ah, bh) 
      COPY_BIT(ah, bl) 

      *dst++ = ((ah << 4) | al);
   } 
} 


//----------------------------------------------------------------------------
void spr_conv(unsigned char *src, unsigned char *dst, int len)//, unsigned char *usage_ptr)
{
    register int    i;
    int offset;

    for(i=0;i<len;i+=4) {
        if((i&0x7f)<64)
            offset=(i&0xfff80)+((i&0x7f)<<1)+4;
        else
            offset=(i&0xfff80)+((i&0x7f)<<1)-128;

        extract8(src,dst+offset);
        src+=4;
    }
    blit_spr_resetcache();
}

//----------------------------------------------------------------------------
void    neogeo_upload(void)
{
    int        Zone;
    int        Taille;
    int        Banque;
    int        Offset = 0;
    unsigned char    *Source;
    unsigned char    *Dest;
    // FILE            *fp;

    Zone = m68k_read_memory_8(0x10FEDA);

    /*fp = fopen_s("UPLOAD.LOG", "at");

    fprintf(fp, "%02x: %06x, %06x:%02x, %08x\n",
        Zone,
        m68k_read_memory_32(0x10FEF8),
        m68k_read_memory_32(0x10FEF4),
        m68k_read_memory_8(0x10FEDB),
        m68k_read_memory_32(0x10FEFC));

    fclose(fp);   */

    switch( Zone&0x0F )
    {
    case    0:    // PRG
        Source = neogeo_prg_memory + m68k_read_memory_32(0x10FEF8);
        Dest = neogeo_prg_memory + m68k_read_memory_32(0x10FEF4);
        Taille = m68k_read_memory_32(0x10FEFC);

        memcpy(Dest, Source, Taille);

        m68k_write_memory_32( 0x10FEF4, m68k_read_memory_32(0x10FEF4) + Taille );

        break;

    case    2:    // SPR
        Banque = m68k_read_memory_8(0x10FEDB);
        Source = neogeo_prg_memory + m68k_read_memory_32(0x10FEF8);
        Offset = m68k_read_memory_32(0x10FEF4) + (Banque<<20);
        Dest = neogeo_spr_memory + Offset;
        Taille = m68k_read_memory_32(0x10FEFC);

        
        do {
            memset(neogeo_cdrom_buffer, 0, BUFFER_SIZE);
            swab(Source, neogeo_cdrom_buffer, min(BUFFER_SIZE, Taille));
            spr_conv(neogeo_cdrom_buffer, Dest, min(BUFFER_SIZE, Taille));//,              video_spr_usage+(Offset>>7));
            Source += min(BUFFER_SIZE, Taille);
            Dest += min(BUFFER_SIZE, Taille);
            Offset += min(BUFFER_SIZE, Taille);
            Taille -= min(BUFFER_SIZE, Taille);
        } while(Taille!=0);
        
        // Mise � jour des valeurs
        Offset = m68k_read_memory_32( 0x10FEF4 );
        Banque = m68k_read_memory_8( 0x10FEDB );
        Taille = m68k_read_memory_8( 0x10FEFC );
        
        Offset += Taille;
        
        while (Offset > 0x100000 )
        {
            Banque++;
            Offset -= 0x100000;
        }
        
        m68k_write_memory_32( 0x10FEF4, Offset );
	m68k_write_memory_8(0x10FEDB, (Banque>>8)&0xFF);
	m68k_write_memory_8(0x10FEDC, Banque&0xFF);
        
        break;

    case    1:    // FIX
        Source = neogeo_prg_memory + m68k_read_memory_32(0x10FEF8);
        Offset = m68k_read_memory_32(0x10FEF4)>>1;
        Dest = neogeo_fix_memory + Offset;
        Taille = m68k_read_memory_32(0x10FEFC);

        do {
            memset(neogeo_cdrom_buffer, 0, BUFFER_SIZE);
            swab(Source, neogeo_cdrom_buffer, min(BUFFER_SIZE, Taille));
            fix_conv(neogeo_cdrom_buffer, Dest, min(BUFFER_SIZE, Taille),  video_fix_usage + (Offset>>5));
            Source += min(BUFFER_SIZE, Taille);
            Dest += min(BUFFER_SIZE, Taille);
            Offset += min(BUFFER_SIZE, Taille);
            Taille -= min(BUFFER_SIZE, Taille);
        } while(Taille!=0);
        
        Offset = m68k_read_memory_32( 0x10FEF4 );
        Taille = m68k_read_memory_32( 0x10FEFC );
        
        Offset += (Taille<<1);
        
        m68k_write_memory_32( 0x10FEF4, Offset);
        
        blit_fix_resetcache();
        
        break;

    case    3:    // Z80


        Source = neogeo_prg_memory + m68k_read_memory_32(0x10FEF8);
        Dest = subcpu_memspace + (m68k_read_memory_32(0x10FEF4)>>1);
        Taille = m68k_read_memory_32(0x10FEFC);
        
        swab( Source, Dest, Taille);


        m68k_write_memory_32( 0x10FEF4, m68k_read_memory_32(0x10FEF4) + (Taille<<1) );

        break;        

    case    5:    // Z80 patch

        Source = neogeo_prg_memory + m68k_read_memory_32(0x10FEF8);
        neogeo_cdrom_apply_patch((short*)Source, m68k_read_memory_32(0x10FEF4), m68k_read_memory_8(0x10FEDB));

        break;
    
    case    4:    // PCM
        Banque = m68k_read_memory_8(0x10FEDB);
        Source = neogeo_prg_memory + m68k_read_memory_32(0x10FEF8);
        Offset = (m68k_read_memory_32(0x10FEF4)>>1) + (Banque<<19);
        Dest = neogeo_pcm_memory + Offset;
        Taille = m68k_read_memory_32(0x10FEFC);
        
        swab( Source, Dest, Taille);        
        
        // Mise � jour des valeurs
        Offset = m68k_read_memory_32( 0x10FEF4 );
        Banque = m68k_read_memory_8( 0x10FEDB );
        Taille = m68k_read_memory_8( 0x10FEFC );
        
        Offset += (Taille<<1);
        
        while (Offset > 0x100000 )
        {
            Banque++;
            Offset -= 0x100000;
        }
        
        m68k_write_memory_32( 0x10FEF4, Offset );
	m68k_write_memory_8(0x10FEDB, (Banque>>8)&0xFF);
	m68k_write_memory_8(0x10FEDC, Banque&0xFF);

        break;    
        
    }

}

//----------------------------------------------------------------------------
void neogeo_cdrom_load_title(void)
{
    char            Path[256];
    char            jue[4] = JUE;
    char            file[12] = TITLE_X_SYS;
    int fp;
    char            *Ptr;
    int                Readed;
    int                x, y;

#ifdef USE_VIDEO_GL
    int filter=neo4all_filter;
    neogeo_adjust_filter(1);
#endif
//    sound_disable();
    //strcpy(Path, cdpath);

    file[6] = jue[m68k_read_memory_8(0x10FD83)&3];

    //strcat(Path, file);
    
    fp = zopen(file);
    if (fp!=-1)
    {
    
    	zread(fp,video_paletteram_pc,  0x5A0);
    	swab((char *)video_paletteram_pc, (char *)video_paletteram_pc, 0x5A0);

    	for(Readed=0;Readed<720;Readed++){
        	video_paletteram_pc[Readed] = video_color_lut[video_paletteram_pc[Readed]]|
        		(Readed&15?0:0x8000);
      }
			blit_fix_resetcache();			
			
    	Ptr = neogeo_spr_memory;
    
    	Readed = zread(fp,neogeo_cdrom_buffer, BUFFER_SIZE);

    	spr_conv(neogeo_cdrom_buffer, Ptr, Readed);//, video_spr_usage);
    	zclose(fp);


    	Readed = 0;
    	for(y=0;y<80;y+=16)
    	{
        	for(x=0;x<144;x+=16)
        	{

            		video_draw_spr(Readed, Readed, 0, 0,(304-144)/2+ x+16,(224-80)/2+ y+16, 15, 16);

            		Readed++;
        	}
    	}

    	blit();
    	//SDL_Delay(1500);
   }

    memset(neogeo_spr_memory, 0, 4194304);
    memset(neogeo_fix_memory, 0, 131072);
    //memset(video_spr_usage, 0, 32768);
    memset(video_fix_usage, 0, 4096);
    blit_spr_resetcache();
    blit_fix_resetcache();


//    sound_enable();
}

#ifdef Z80_EMULATED
#define PATCH_Z80(a, b) { \
                            subcpu_memspace[(a)] = (b)&0xFF; \
                            subcpu_memspace[(a+1)] = ((b)>>8)&0xFF; \
                        }
#else
#define PATCH_Z80(a, b) { }
#endif

void neogeo_cdrom_apply_patch(short *source, int offset, int bank)
{
    int master_offset;
    
    master_offset = (((bank*1048576) + offset)/256)&0xFFFF;
    
    while(*source != 0)
    {
        PATCH_Z80( source[0], ((source[1] + master_offset)>>1) );
        PATCH_Z80( source[0] + 2, (((source[2] + master_offset)>>1) - 1) );
        
        if ((source[3])&&(source[4]))
        {
            PATCH_Z80( source[0] + 5, ((source[3] + master_offset)>>1) );
            PATCH_Z80( source[0] + 7, (((source[4] + master_offset)>>1) - 1) );
        }
            
        source += 5;
    }
}

