/* Allegro Player sample */
/* Written by Ove Kaaven <ovek@arcticnet.no> */
/* Improvements are very welcome */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "allegro.h"
#include "libamp.h"

//#define GFX

char file[80];

#ifdef GFX

char timestr[32],ratestr[32];

int idler(int msg,DIALOG*d,int c)
{
 if (msg==MSG_IDLE) {
/* poll_amp() decodes one frame,
   while run_amp() fills up entire buffer...

   poll_amp() pros:
    -uses little time per call (less jerky in loops)
    -returns whether any processing could be done at all
   poll_amp() cons:
    -does not fill buffer if not called often enough

   run_amp() pros:
    -fills buffer, does not need to be called as often
   run_amp() cons:
    -may use more time, jerky in loops
*/

/*
  run_amp();
*/

/* in this program, poll_amp() will be called often enough */
  if (poll_amp()<=0) __dpmi_yield();
  return(D_O_K);
 }
 return(D_O_K);
}

int phase_proc(int msg,DIALOG*d,int c)
{
 int ret=d_button_proc(msg,d,c);

 if ((d->flags&D_SELECTED)&&!amp_reverse_phase) {
  amp_reverse_phase=1;
  seek_amp_rel(0); /* recalculate waveform immediately */
 } else
 if (!(d->flags&D_SELECTED)&&amp_reverse_phase) {
  amp_reverse_phase=0;
  seek_amp_rel(0); /* recalculate waveform immediately */
 }
 return(ret);
}

int ffwd_proc(int msg,DIALOG*d,int c)
{
 int ret=d_button_proc(msg,d,c);

 if (d->flags&D_SELECTED) {
  seek_amp_rel(4*amp_samprat/amp_pollsize); /* seek 4 seconds */
  d->flags&=~D_SELECTED;
  show_mouse(NULL);
  d_button_proc(MSG_DRAW,d,0);
  show_mouse(screen);
 }
 return(ret);
}

int rew_proc(int msg,DIALOG*d,int c)
{
 int ret=d_button_proc(msg,d,c);

 if (d->flags&D_SELECTED) {
  seek_amp_rel(-4*amp_samprat/amp_pollsize); /* seek 4 seconds */
  d->flags&=~D_SELECTED;
  show_mouse(NULL);
  d_button_proc(MSG_DRAW,d,0);
  show_mouse(screen);
 }
 return(ret);
}

int replay_proc(int msg,DIALOG*d,int c)
{
 int ret=d_button_proc(msg,d,c);

 if (d->flags&D_SELECTED) {
  replay_amp();
  d->flags&=~D_SELECTED;
  show_mouse(NULL);
  d_button_proc(MSG_DRAW,d,0);
  show_mouse(screen);
 }
 return(ret);
}

int timer_proc(int msg,DIALOG*d,int c)
{
 static int last_time;
 int this_time=amp_time,ret=D_O_K;

 sprintf(timestr,"%3d:%02d",this_time/60,this_time%60);

 if (msg==MSG_IDLE) {
  if (this_time!=last_time) {
   show_mouse(NULL);
   ret=d_text_proc(MSG_DRAW,d,c);
   show_mouse(screen);
   last_time=this_time;
  }
 } else {
  ret=d_text_proc(msg,d,c);
 }
 return(ret);
}

int vol_proc(void*dp3,int d2)
{
 set_volume(d2,-1);
 return(D_O_K);
}

#define BARS
#undef DOTS

#define XF 4

int scope_proc(int msg,DIALOG*d,int c)
{
 static BITMAP*scope_view=NULL;
 static unsigned short*last_left;
 unsigned short*this_left=amp_play_left,*this_right=amp_play_right;
 int ret=D_O_K,cnt;
 int d_left,d_right;

 if (msg==MSG_START) {
  if (!scope_view)
  scope_view=create_bitmap(amp_play_len/XF,128);
 } else
 if (msg==MSG_END) {
  destroy_bitmap(scope_view);
  scope_view=NULL;
 } else
 if (msg==MSG_IDLE) {
  if ((this_left!=last_left)&&(this_left)) {
   clear(scope_view);
#ifdef BARS
   xor_mode(TRUE);
   for (cnt=0; cnt<amp_play_len/XF; cnt++) {
    d_left=this_left[cnt*XF]/512;
    d_right=this_right[cnt*XF]/512;
    vline(scope_view,cnt,64,d_left,1);
    vline(scope_view,cnt,64,d_right,4);
   }
   hline(scope_view,0,64,cnt,2);
   xor_mode(FALSE);
#else
#ifdef DOTS
   for (cnt=0; cnt<amp_play_len/XF; cnt++) {
    d_left=this_left[cnt*XF]/512;
    d_right=this_right[cnt*XF]/512;
    if (d_left==d_right)
     putpixel(scope_view,cnt,d_left,5);
    else {
     putpixel(scope_view,cnt,d_left,1);
     putpixel(scope_view,cnt,d_right,4);
    }
    putpixel(scope_view,cnt,64,7);
   }
#endif
#endif
   if (mouse_x>(d->x+amp_play_len/XF)) freeze_mouse_flag=1;
    else show_mouse(NULL);
   blit(scope_view,screen,0,0,d->x,d->y,amp_play_len/XF,128);
   freeze_mouse_flag=0;
   show_mouse(screen);
   last_left=this_left;
  }
 }
 return(ret);
}

DIALOG tracker[]={
/* proc           x   y   w   h   fg bg   k  flags  1 2  dp */
 {d_clear_proc},
 {d_ctext_proc,  160,  4,  0,  8, 255,0,  0 ,     0,0,0, "LibAmp 0.2 Demo"},
 {d_ctext_proc,  160, 16,  0,  8, 255,0,  0 ,     0,0,0, file},
 {d_button_proc, 256,104, 56, 14, 255,0, 'o',D_EXIT,0,0, "&Open"},
 {phase_proc,    256,120, 56, 14, 255,0, 'p',     0,0,0, "&Phase"},
 {ffwd_proc,     256,136, 56, 14, 255,0, 'f',     0,0,0, "&Ffwd"},
 {rew_proc,      256,152, 56, 14, 255,0, 'w',     0,0,0, "Re&w"},
 {replay_proc,   256,168, 56, 14, 255,0, 'r',     0,0,0, "&Replay"},
 {d_button_proc, 256,184, 56, 14, 255,0, 'x',D_EXIT,0,0, "E&xit"},

 {d_slider_proc, 272, 32, 24, 64, 255,0,  0 ,     0,255,128,NULL,vol_proc},

 {d_text_proc,     0, 32, 64,  8, 255,0,  0 ,     0,0,0, "Time:"},
 {timer_proc,     64, 32, 64,  8, 255,0,  0 ,     0,0,0, timestr},
 {d_text_proc,     0, 40, 64,  8, 255,0,  0 ,     0,0,0, "Rate:"},
 {d_text_proc,    64, 40, 64,  8, 255,0,  0 ,     0,0,0, ratestr},
 {scope_proc,      0, 64,256,128, 255,0,  0 ,     0,0,0, NULL},

 {idler},
 {NULL}
};

#endif

int main(int argc,char**argv)
{
 allegro_init();
 i_love_bill = TRUE;
 if (install_sound(DIGI_AUTODETECT,MIDI_NONE,NULL)) {
  printf("Unable to install Allegro sound driver\n");
  return(1);
 }
#ifdef GFX
 install_keyboard();
 install_mouse();
 install_timer();
#endif
 install_amp();
#ifdef GFX
 set_gfx_mode(GFX_AUTODETECT,320,200,0,0);
 set_palette(desktop_palette);
/* file selector that asks for MP3 file */
 if (argc<2) {
  getcwd(file,sizeof(file));
  if (file[strlen(file)-1]!='\\') strcat(file,"\\");
  if (!file_select("Select MP3 file",file,"MP3")) return(1);
 } else {
  strcpy(file,argv[1]);
 }
#else
 if (argc<2) {
  printf("Syntax: %s song.mp3\n",get_filename(argv[0]));
  return(1);
 }
 if (argc>2) amp_downmix(TRUE);
 strcpy(file,argv[1]);
#endif
 set_volume(128,-1);
 amp_reverse_phase=0; /* SURROUND SOUND */
 while (load_amp(file,1)) {
#ifdef GFX
  sprintf(ratestr,"%dkbps/%dHz",amp_bitrate,amp_samprat);
  if (do_dialog(tracker,-1)!=3) break;
  unload_amp();
  if (!file_select("Select MP3 file",file,"MP3")) return(1);
#else
  while (amp_decode()>=0);
  break;
#endif
 }
 unload_amp();
 return(0);
}

