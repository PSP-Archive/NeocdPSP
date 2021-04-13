/******************************************
**** Fixed Text Layer Drawing Routines ****
******************************************/

#include <stdio.h>
#include <string.h>
//#include <ctype.h>


#include "../neocd.h"
#include "video.h"


/* Draw Single FIX character */
INLINE void draw_fix(u16 code, u16 colour, u16 sx, u16 sy, u16 * palette, char * fix_memory)
{
	u8 y;
	u32 mydword;
	u32 * fix=(u32*)&(fix_memory[code<<5]);
	u16 * dest = video_line_ptr[sy] + sx;
	u16 * paldata=&palette[colour];
	u16 col;
	
	for(y=0;y<8;y++)
	{
		mydword  = *fix++;
		
		col = (mydword>> 0)&0x0f; if (col) dest[0] = paldata[col];
		col = (mydword>> 4)&0x0f; if (col) dest[1] = paldata[col];
		col = (mydword>> 8)&0x0f; if (col) dest[2] = paldata[col];
		col = (mydword>>12)&0x0f; if (col) dest[3] = paldata[col];
		col = (mydword>>16)&0x0f; if (col) dest[4] = paldata[col];
		col = (mydword>>20)&0x0f; if (col) dest[5] = paldata[col];
		col = (mydword>>24)&0x0f; if (col) dest[6] = paldata[col];
		col = (mydword>>28)&0x0f; if (col) dest[7] = paldata[col];
		dest += VIDEOBUF_PITCH;
	}
}


/* Draw entire Character Foreground */
void video_draw_fix(void)
{
	u16 x, y;
	u16 code, colour;
	u16 * fixarea=(u16 *)&video_vidram[0xe004];


	for (y=0; y < 28; y++)
	{
		for (x = 0; x < 40; x++)
		{
			code = fixarea[x << 5];

			colour = (code&0xf000)>>8;
			code  &= 0xfff;

			if(video_fix_usage[code]) draw_fix(code,colour,(x<<3),(y<<3), video_paletteram_pc, neogeo_fix_memory);
		}
		fixarea++;
	}
}



/* FIX palette for fixputs*/
u16 palette[16]={0x0000,0xffff,0x0000,0x0000,
		    0x0000,0x0000,0x0000,0x0000,
		    0xffff,0x0000,0x0000,0x0000,
		    0x0000,0x0000,0x0000,0xffff};

void fixputs( u16 x, u16 y, const char * string )
{
	u8 i;
	int length=strlen(string);
	
	if ( y>27 ) return;
	
	if ( x+length > 40 ) {
		length=40-x;
	}
	
	if (length<0) return;

	
	y<<=3;
		
	for (i=0; i<length; i++) {	
		draw_fix(toupper(string[i])+0x300,0,(x+i)<<3,y,palette, &neogeo_rom_memory[458752]);
	}
	
	

	return;
}


/* Draw entire Character Foreground */
void video_draw_fixPSP(void)
{
	u16 x, y;
	u16 code, colour;
	u16 * fixarea=(u16 *)&video_vidram[0xe004];

	//blit_spritemode(8,8,8,8);

	for (y=0; y < 28; y++)
	{
		for (x = 0; x < 40; x++)
		{
			code = fixarea[x << 5];

			colour = (code&0xf000)>>8;
			code  &= 0xfff;

			if(video_fix_usage[code])	blit_drawfix(x<<3,y<<3,&video_paletteram_pc[colour],(u32*)&(neogeo_fix_memory[code<<5]),code,colour);	
		}
		fixarea++;
	}
}