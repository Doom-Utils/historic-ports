#ifndef REAL
// I think double sounds better, but is a bit slower
// If you have a pentium you should define ARCH_i586
// This will also enable asm optimisations.
// Also, perhaps a fixed point implementation could
// be used, for slow 486's and processors without
// FPUs  MP3 decoding is *VERY* heavy on the FPU
#ifdef ARCH_i586
#define REAL double
#else
#define REAL float
#endif
#endif
