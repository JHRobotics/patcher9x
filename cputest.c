/******************************************************************************
 * Copyright (c) 2022 Jaroslav Hensl                                          *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person                *
 * obtaining a copy of this software and associated documentation             *
 * files (the "Software"), to deal in the Software without                    *
 * restriction, including without limitation the rights to use,               *
 * copy, modify, merge, publish, distribute, sublicense, and/or sell          *
 * copies of the Software, and to permit persons to whom the                  *
 * Software is furnished to do so, subject to the following                   *
 * conditions:                                                                *
 *                                                                            *
 * The above copyright notice and this permission notice shall be             *
 * included in all copies or substantial portions of the Software.            *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,            *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES            *
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                   *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT                *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,               *
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING               *
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR              *
 * OTHER DEALINGS IN THE SOFTWARE.                                            *
 *                                                                            *
*******************************************************************************/
#include <time.h>
#include <cextra.h>
#include <limits.h>
#include "patcher9x.h"

#if defined(_WIN32)
#include <Windows.h>
/* ^ for QueryPerformanceCounter and QueryPerformanceFrequency */
#endif

/*
 * loop size definitions
 */

/* original WIN95: NTKERN.VXD, IOS.VXD, ESDI_506.PDR, SCSIPORT.PDR, CS3KIT.EX */
#define SPEED_V1   0xf4240UL

/* from WIN95/98: NDIS.VXD  */
#define SPEED_V2  0x100000UL

/* patched WIN95: NTKERN.VXD, IOS.VXD, ESDI_506.PDR, SCSIPORT.PDR, CS3KIT.EX */
#define SPEED_V3  0x989680UL

/* patched WIN95/98: NDIS.VXD */
#define SPEED_V4  0xA00000UL

/* new loop for: NTKERN.VXD, IOS.VXD, ESDI_506.PDR, SCSIPORT.PDR, CS3KIT.EX  */
#define SPEED_V5 0x4C4B400UL

/* new loop for: NDIS.VXD */
#define SPEED_V6 0x6400000UL

/*
 * All code is only for X86/AMD64. 
 *
 */
#if defined(__x86_64__) || defined(__i386__) || defined(__DJGPP__) || \
	 (defined(_WIN32 ) && (!defined(_M_ARM) || !defined(_M_ARM64)))

/*
 * Platfrom depended timer api
 */
#if defined(_WIN32)
	/* Windows have very uprecize clock() function, so using timer from WIN32 api */
	typedef uint64_t tick_t;
	static tick_t timer_freq = 0;
	#define tick(_tick) QueryPerformanceCounter((LARGE_INTEGER*)&(_tick));
	#define TICK_MAX ULLONG_MAX
	
	static double clock2ms(tick_t tick)
	{
		return (((double)tick) * 1000)/timer_freq;
	}
	
	static void tick_init()
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&timer_freq);
	}
	
#elif defined(__DJGPP__)
	/* DJGPP has special usec timer using it for better precision */
	typedef uclock_t tick_t;
	#define tick(_tick) _tick=uclock()
	#define TICK_MAX ((sizeof(tick_t) > 4) ? LLONG_MAX : LONG_MAX)
		
	static double clock2ms(tick_t tick)
	{
		return (((double)tick) * 1000)/UCLOCKS_PER_SEC;
	}
	
	#define tick_init() ;
#else	
  /* for most platform is clock() precise enought */
	typedef clock_t tick_t;
	#define tick(_tick) _tick=clock()
	#define TICK_MAX ((sizeof(tick_t) > 4) ? LLONG_MAX : LONG_MAX)
		
	static double clock2ms(tick_t tick)
	{
		return (((double)tick) * 1000)/CLOCKS_PER_SEC;
	}
	
	#define tick_init() ;
#endif

/**
 * Original timer function
 * 
 * @param repeats: number of loop (original defines in SPEED_* macros) 
 *
 **/
INLINE tick_t looptest(uint32_t repeats)
{
	tick_t c1, c2;

	tick(c1);
	
	asm volatile (
		"movl %0, %%ecx;"
		"loop_repeat:"
		"loop loop_repeat;"
		: 
		: "m" (repeats)
		: "%ecx"
	);
	
	tick(c2);
	
	return c2-c1;
}

static void print_cpu()
{
	uint32_t cpuid_support = 0;
	uint32_t info_buf[13];
	memset(info_buf, 0, sizeof(info_buf));
	
#if defined(_M_X64) || defined(__x86_64__) /* 64 bit CPUs are supporting CPUID */
	cpuid_support = 1;
#else /* 32 bit mode */	
	/* cpuid detection:
	 *  https://www.prowaretech.com/articles/current/assembly/x86/tutorial/page-13
	 */
	asm volatile (
		"pushfl;"               // push eflags on the stack
		"popl %%eax;"           // pop them into eax
		"movl %%eax, %%ebx;"    // save to ebx for restoring afterwards
		"xorl $0x200000,%%eax;" // toggle bit 21
		"pushl %%eax; "         // push the toggled eflags
		"popfl;"                // pop them back into eflags
		"pushfl;"               // push eflags
		"popl %%eax;"           // pop them back into eax
		"andl $0x200000,%%eax;" // test only bit 21
		"andl $0x200000,%%ebx;" // 
		"cmpl %%eax, %%ebx;"    // see if bit 21 was reset
		"jz not_supported;"
		"movl $1,%0;"           // set cpuid_support=1
		"not_supported:"
		: "=m" (cpuid_support)
		:
		: "%eax", "%ebx"
	);
#endif
	
	if(cpuid_support == 0)
	{
		printf("CPUID not supported\n");
		return;
	}
	
	asm volatile (
		"movl $1,%%eax;"
		"cpuid;"
		"cmp $0x80000004,%%eax;"
		"jl skip;"
		"movl $0x80000002,%%eax;"
		"cpuid;"
		"movl %%eax,%0;"
		"movl %%ebx,%1;"
		"movl %%ecx,%2;"
		"movl %%edx,%3;"
		"movl $0x80000003,%%eax;"
		"cpuid;"
		"movl %%eax,%4;"
		"movl %%ebx,%5;"
		"movl %%ecx,%6;"
		"movl %%edx,%7;"
		"movl $0x80000004,%%eax;"
		"cpuid;"
		"movl %%eax,%8;"
		"movl %%ebx,%9;"
		"movl %%ecx,%10;"
		"movl %%edx,%10;"
		"skip:"
		: "=m" (info_buf[0]), "=m" (info_buf[1]), "=m" (info_buf[2]),  "=m" (info_buf[3]),
			"=m" (info_buf[4]), "=m" (info_buf[5]), "=m" (info_buf[6]),  "=m" (info_buf[7]),
			"=m" (info_buf[8]), "=m" (info_buf[9]), "=m" (info_buf[10]), "=m" (info_buf[11])
		:
		: "%eax", "%ebx", "%ecx", "%edx"
	);
	
	info_buf[12] = 0;
	
	if(info_buf[0] == 0)
	{
		printf("CPU brand identification is not supported\n");
	}
	else
	{
		printf("CPU: %s\n", (char*)info_buf);
	}
}

#define MIN_TIME(ts, t1) if(t1 < ts){ts = t1;}
#define TEST_COUNT 3

void cputest()
{
	tick_t t,
		t1 = TICK_MAX,
		t2 = TICK_MAX,
		t3 = TICK_MAX,
		t4 = TICK_MAX,
		t5 = TICK_MAX;
	int i;
	
	printf("Preparing CPU test, please wait...\n");
	
	tick_init();
	
	looptest(SPEED_V1);
	looptest(SPEED_V2);
	looptest(SPEED_V3);
	looptest(SPEED_V4);
	looptest(SPEED_V5);
	
	for(i = 1; i <= TEST_COUNT; i++)
	{
		printf("### test %d/%d\n", i, TEST_COUNT);
		
		t = looptest(SPEED_V1);
		printf("LOOP V1: %.3lf ms\n", clock2ms(t));
		MIN_TIME(t1, t);
		
		t = looptest(SPEED_V2);
		printf("LOOP V2: %.3lf ms\n", clock2ms(t));
		MIN_TIME(t2, t);
		
		t = looptest(SPEED_V3);
		printf("LOOP V3: %.3lf ms\n", clock2ms(t));
		MIN_TIME(t3, t);
		
		t = looptest(SPEED_V4);
		printf("LOOP V4: %.3lf ms\n", clock2ms(t));
		MIN_TIME(t4, t);
		
		t = looptest(SPEED_V5);
		printf("LOOP V5: %.3lf ms\n", clock2ms(t));
		MIN_TIME(t5, t);
	}
	printf("### stats:\n");
	printf("LOOP V1: %.3lf ms (LOOP = %lu)\n", clock2ms(t1), SPEED_V1);
	printf("LOOP V2: %.3lf ms (LOOP = %lu)\n", clock2ms(t2), SPEED_V2);
	printf("LOOP V3: %.3lf ms (LOOP = %lu)\n", clock2ms(t3), SPEED_V3);
	printf("LOOP V4: %.3lf ms (LOOP = %lu)\n", clock2ms(t4), SPEED_V4);
	printf("LOOP V5: %.3lf ms (LOOP = %lu)\n", clock2ms(t5), SPEED_V5);
	
	printf("### compatibility:\n");
	
	printf("Windows 95: ");
	if(clock2ms(t1) < 2.1)
	{
		printf("CPU is too fast!\n");
	}
	else
	{
		printf("OK\n");
	}
	
	printf("Windows 95 + FIX95CPU: ");
	if(clock2ms(t3) < 2.1)
	{
		printf("CPU is still too fast!\n");
	}
	else
	{
		printf("OK\n");
	}
	
	printf("Windows 98 (pre 1411): ");
	if(clock2ms(t1) < 2.1)
	{
		printf("CPU is too fast!\n");
	}
	else
	{
		printf("OK\n");
	}
	
	printf("Windows 98 FE: ");
	if(clock2ms(t2) < 1.1)
	{
		printf("CPU is too fast!\n");
	}
	else if(clock2ms(t2) < 2.1)
	{
		printf("CPU speed is on bleeding edge\n");
	}
	else
	{
		printf("FE: OK\n");
	}
	
	printf("Windows 98 SE: ");
	if(clock2ms(t2) < 1.1)
	{
		printf("potencial problem with NDIS.VXD\n");
	}
	else
	{
		printf("OK\n");
	}
	
	printf("Windows ME: not affected\n");
	
	printf("### conclusion:\n");
	
	if(clock2ms(t3) > 1000.0)
	{
		printf("Status: CPU patch will have serious speed loss for CPU, please run patch process with -no-cpuspeed-fix parameter, sorry.\n");
	}
	else if(clock2ms(t1) < 2.5 && clock2ms(t3) > 2.5)
	{
		printf("Status: patch is recomended, but FIX95CPU works with your CPU too\n");
	}
	else if(clock2ms(t4) < 2.5)
	{
		printf("patch is highly recomended!\n");
	}
	else
	{
		printf("patch isn't required, but if it be applied, it'll work.\n");
	}
	
	#if defined(DOS_MODE)
		printf("Precision: benchmark ran from DOS, results are precize\n");
	#else
		printf("Precision: benchmark ran from OS, results are not completely accurate\n");
	#endif
	
	print_cpu();
	
}

#else /* x86/AMD64 */

/*
 * Failback for other platforms
 */
void cputest()
{
	printf("CPU speed test only works for X86/AMD64 CPU!\n");
}

#endif

