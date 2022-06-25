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
#endif

/*
 * Theres assembly code is only for X86/AMD64. 
 */
#if defined(__x86_64__) || defined(__i386__) || defined(__DJGPP__) || \
	 (defined(_WIN32 ) && (!defined(_M_ARM) || !defined(_M_ARM64)))

#if defined(_WIN32)
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
	typedef uclock_t tick_t;
	#define tick(_tick) _tick=uclock()
	#define TICK_MAX ((sizeof(tick_t) > 4) ? ULLONG_MAX : ULONG_MAX)
		
	static double clock2ms(tick_t tick)
	{
		return (((double)tick) * 1000)/UCLOCKS_PER_SEC;
	}
	
	#define tick_init() ;
#else	
	typedef clock_t tick_t;
	#define tick(_tick) _tick=clock()
	#define TICK_MAX ((sizeof(tick_t) > 4) ? ULLONG_MAX : ULONG_MAX)
		
	static double clock2ms(tick_t tick)
	{
		return (((double)tick) * 1000)/CLOCKS_PER_SEC;
	}
	
	#define tick_init() ;
#endif

#define SPEED_V1   0xf4240
#define SPEED_V2  0x100000
#define SPEED_V3  0x989680
#define SPEED_V4  0xA00000
#define SPEED_V5 0x4C4B400

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
	uint32_t info_buf[13];
	memset(info_buf, 0, sizeof(info_buf));
	
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
	printf("### compatibility:\n");
	if(clock2ms(t1) < 2.5)
	{
		printf("Windows 95: CPU IS TOO FAST\n");
	}
	else
	{
		printf("Windows 95: OK\n");
	}
	
	if(clock2ms(t3) < 2.5)
	{
		printf("Windows 95 + FIX95CPU: CPU IS still TOO FAST\n");
	}
	else
	{
		printf("Windows 95 + FIX95CPU: OK\n");
	}
	
	if(clock2ms(t1) < 2.5)
	{
		printf("Windows 98 pre-release: CPU IS TOO FAST\n");
	}
	else
	{
		printf("Windows 98 pre-release: OK\n");
	}
	
	if(clock2ms(t2) < 2.5)
	{
		printf("Windows 98 FE: CPU IS TOO FAST\n");
	}
	else
	{
		printf("Windows 98 FE: OK\n");
	}
	
	if(clock2ms(t2) < 2.5)
	{
		printf("Windows 98 SE: potencial problem with NDIS.VXD\n");
	}
	else
	{
		printf("Windows 98 SE: OK\n");
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
	else
	{
		printf("patch is highly recomended!\n");
	}
	
	#if defined(__MSDOS__)
		printf("Precision: benchmark ran from DOS, results are precize\n");
	#else
		printf("Precision: benchmark ran from OS, results are not completely accurate\n");
	#endif
	
	print_cpu();
	
}

#else /* x86/AMD64 */

void cputest()
{
	printf("CPU speed test only works for X86/AMD64 CPU!\n");
}

#endif

