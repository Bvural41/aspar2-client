#ifndef __INC_ETERBASE_RANDOM_H__
#define __INC_ETERBASE_RANDOM_H__

#if defined(__ANDROID__) || defined(__linux__)
#define random metin2_random
#define srandom metin2_srandom
#endif

extern void				srandom(unsigned long seed);
extern unsigned long	random();
extern float			frandom(float flLow, float flHigh);
extern long				random_range(long from, long to);

#endif