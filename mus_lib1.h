//
// Header for the LIBAMP Library
//
extern int amp_bitrate,amp_samprat;
extern int amp_mpg_ver,amp_layer,amp_stereo,amp_pollsize;
extern int amp_playing,amp_loaded,amp_reverse_phase;
extern int amp_dec_frame,amp_dec_time;
extern int amp_frame,amp_time;

extern unsigned short*amp_buf_left,*amp_buf_right;
extern int amp_buf_len;

extern unsigned short*amp_play_left,*amp_play_right;
extern int amp_play_len;

int install_amp(void);
int load_amp(char*filename,int loop);
#ifdef AMP_POLL
int poll_amp(void);
int run_amp(void);
#else
int amp_decode(void);
void amp_interrupt(void);
#endif
void amp_pause(void);
void amp_setvolume(int vol);
void amp_resume(void);
int replay_amp(void);
int seek_amp_abs(int frame);
int seek_amp_rel(int framecnt);
int unload_amp(void);
int amp_downmix(int mix);
