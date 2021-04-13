#ifndef __PSP_H__
#define __PSP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*#define malloc malloc_psp
#define free free_psp
#define calloc calloc_psp
#define realloc realloc_psp
*/

#define RELEASE
#define SNAPSHOT_ACTIVE

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
//#include <fcntl.h>
#include <string.h>
#include <math.h>

#include "pg.h"
#include "psplib.h"
//#include "malloc.h"

//#define memmove memmove_psp

void ErrorMsg(char *msg);

extern char launchDir[256];
extern char tmp_str[256];

#define VRAM_ADDR	(0x04000000)

#define SCREEN_WIDTH	480
#define SCREEN_HEIGHT	272

#define POWER_CB_POWER		0x80000000 
#define POWER_CB_HOLDON		0x40000000 
#define POWER_CB_STANDBY	0x00080000 
#define POWER_CB_RESCOMP	0x00040000 
#define POWER_CB_RESUME		0x00020000 
#define POWER_CB_SUSPEND	0x00010000 
#define POWER_CB_EXT		0x00001000 
#define POWER_CB_BATLOW		0x00000100 
#define POWER_CB_BATTERY	0x00000080 
#define POWER_CB_BATTPOWER	0x0000007F 

#define MAX_PATH 256

#define VERSION_MAJOR 0
#define VERSION_MINOR 5

#define VERSION_STR "0.5"
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PSP_H__ */
