/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *      By Shawn Hargreaves,
 *      1 Salisbury Road,
 *      Market Drayton,
 *      Shropshire,
 *      England, TF9 1AJ.
 *
 *      Some definitions for internal use by the library code.
 *      This should not be included by user programs.
 *
 *      See readme.txt for copyright information.
 */


#ifndef INTERNDJ_H
#define INTERNDJ_H

#ifndef DJGPP
#error This file should only be used by the djgpp version of Allegro
#endif


#include <dos.h>


/* file access macros */
#define FILE_OPEN(filename, handle)             handle = open(filename, O_RDONLY | O_BINARY, S_IRUSR | S_IWUSR)
#define FILE_CREATE(filename, handle)           handle = open(filename, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)
#define FILE_CLOSE(handle)                      close(handle)
#define FILE_READ(handle, buf, size, sz)        sz = read(handle, buf, size)
#define FILE_WRITE(handle, buf, size, sz)       sz = write(handle, buf, size) 
#define FILE_SEARCH_STRUCT                      struct ffblk
#define FILE_FINDFIRST(filename, attrib, dta)   findfirst(filename, dta, attrib)
#define FILE_FINDNEXT(dta)                      findnext(dta)
#define FILE_ATTRIB                             ff_attrib
#define FILE_SIZE                               ff_fsize
#define FILE_NAME                               ff_name
#define FILE_TIME                               ff_ftime
#define FILE_DATE                               ff_fdate


/* macros to enable and disable interrupts */
#define DISABLE()   asm volatile ("cli")
#define ENABLE()    asm volatile ("sti")


__INLINE__ void enter_critical() 
{
   if (windows_version >= 3) {
      __dpmi_regs r;
      r.x.ax = 0x1681; 
      __dpmi_int(0x2F, &r);
   }

   DISABLE();
}


__INLINE__ void exit_critical() 
{
   if (windows_version >= 3) {
      __dpmi_regs r;
      r.x.ax = 0x1682; 
      __dpmi_int(0x2F, &r);
   }

   ENABLE();
}


/* interrupt hander stuff */
#define _map_irq(irq)   (((irq)>7) ? ((irq)+104) : ((irq)+8))

int _install_irq(int num, int (*handler)());
void _remove_irq(int num);
void _restore_irq(int irq);
void _enable_irq(int irq);
void _disable_irq(int irq);

#define _eoi(irq) { outportb(0x20, 0x20); if ((irq)>7) outportb(0xA0, 0x20); }

typedef struct _IRQ_HANDLER
{
   int (*handler)();             /* our C handler */
   int number;                   /* irq number */
   __dpmi_paddr old_vector;      /* original protected mode vector */
} _IRQ_HANDLER;


/* DPMI memory mapping routines */
int _create_physical_mapping(unsigned long *linear, int *segment, unsigned long physaddr, int size);
void _remove_physical_mapping(unsigned long *linear, int *segment);
int _create_linear_mapping(unsigned long *linear, unsigned long physaddr, int size);
void _remove_linear_mapping(unsigned long *linear);
int _create_selector(int *segment, unsigned long linear, int size);
void _remove_selector(int *segment);
void _unlock_dpmi_data(void *addr, int size);


/* bank switching routines */
void _accel_bank_stub();
void _accel_bank_stub_end();

void _accel_bank_switch();
void _accel_bank_switch_end();

void _vesa_window_1();
void _vesa_window_1_end();
void _vesa_window_2();
void _vesa_window_2_end();

void _vesa_pm_window_1();
void _vesa_pm_window_1_end();
void _vesa_pm_window_2();
void _vesa_pm_window_2_end();

void _vesa_pm_es_window_1();
void _vesa_pm_es_window_1_end();
void _vesa_pm_es_window_2();
void _vesa_pm_es_window_2_end();

void _ati_bank();
void _ati_bank_end();

void _mach64_write_bank();
void _mach64_write_bank_end();
void _mach64_read_bank();
void _mach64_read_bank_end();

void _cirrus64_write_bank();
void _cirrus64_write_bank_end();
void _cirrus64_read_bank();
void _cirrus64_read_bank_end();

void _cirrus54_bank();
void _cirrus54_bank_end();

void _paradise_write_bank();
void _paradise_read_bank();
void _paradise_write_bank_end();
void _paradise_read_bank_end();

void _s3_bank();
void _s3_bank_end();

void _trident_bank();
void _trident_bank_end();
void _trident_read_bank();
void _trident_read_bank_end();
void _trident_write_bank();
void _trident_write_bank_end();

void _et3000_write_bank();
void _et3000_write_bank_end();
void _et3000_read_bank();
void _et3000_read_bank_end();

void _et4000_write_bank();
void _et4000_write_bank_end();
void _et4000_read_bank();
void _et4000_read_bank_end();

void _video7_bank();
void _video7_bank_end();


/* stuff for the VESA and VBE/AF drivers */
extern __dpmi_regs _dpmi_reg;

extern int _window_2_offset;

extern void (*_pm_vesa_switcher)();
extern void (*_pm_vesa_scroller)();
extern void (*_pm_vesa_pallete)();

extern int _mmio_segment;

extern void *_accel_driver;

extern int _accel_active;

extern void *_accel_set_bank;
extern void *_accel_idle;


/* stuff for setting up bitmaps */
long _vesa_vidmem_check(long mem);


/* sound lib stuff */
extern int _fm_port;
extern int _mpu_port;
extern int _mpu_irq;
extern int _sb_freq;
extern int _sb_port; 
extern int _sb_dma; 
extern int _sb_irq; 

int _sb_read_dsp_version();
int _sb_reset_dsp(int data);
void _sb_voice(int state);
int _sb_set_mixer(int digi_volume, int midi_volume);

void _mpu_poll();

int _dma_allocate_mem(int bytes, int *sel, unsigned long *phys);
void _dma_start(int channel, unsigned long addr, int size, int auto_init, int input);
void _dma_stop(int channel);
unsigned long _dma_todo(int channel);
void _dma_lock_mem();


#endif          /* ifndef INTERNDJ_H */
