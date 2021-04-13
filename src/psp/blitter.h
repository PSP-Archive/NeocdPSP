#ifndef _blit_h_
#define _blit_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void blit_init(void);
void blit_deinit(void);
void guDrawBuffer(u16* video_buffer,int src_w,int src_h,int src_pitch,int dst_w,int dst_h);
void blit_fix_resetcache();
void blit_spr_resetcache();

void blit_start();
void blit_finish(float sw,float sh,float dw,float dh);
extern unsigned int __attribute__((aligned(16))) gulist[262144];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
