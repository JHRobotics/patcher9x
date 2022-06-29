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
#ifndef __PATCHER9X_INCLUDED__
#define __PATCHER9X_INCLUDED__

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mspack.h>
#include <filesystem.h>
#include <extstring.h>
#include <pew.h>

#include "version.h"

/*
 * Program flow constants
 */

/* error codes */
#define PATCH_OK           0
#define PATCH_E_READ       1
#define PATCH_E_WRITE      2
#define PATCH_E_CHECK      3
#define PATCH_E_MEM        4
#define PATCH_E_OVERWRITE  5
#define PATCH_E_WRONG_TYPE 6
#define PATCH_E_CONVERT    7
#define PATCH_E_NOTFOUND   8
#define PATCH_E_PATCHED    9
#define PATCH_E_NOTFOUNDINCAB    10
#define PATCH_E_NOTFOUNDINCABS   11

/* compresion control */
#define PATCH_FORCE_W3 0x01000000
#define PATCH_FORCE_W4 0x02000000

/*
 * Patches and it's versions
 */

/* target is W3/W4 file */
#define PATCH_VX_PACK 0x80000000

/* target could be part of VMM32 */
#define PATCH_VX_UNPACK 0x40000000

/* not really apply patch, only do search for them */
#define PATCH_DRY 0x10000000

/* TLB patch for windows 98 */
#define PATCH_VMM98 0x00000001

/* TLB patch for windows me */
#define PATCH_VMMME 0x00000002

/* TLB patch for windows 98 (for updated VMM.VXD) */
#define PATCH_VMM98_V2 0x00000004

/* CPU speed patch with 1 000 000 (0xF4240) LOOP */
#define PATCH_CPU_SPEED_V1 0x00000010

/* CPU speed patch with 2 000 000 (0x1E8480) LOOP */
#define PATCH_CPU_SPEED_V2 0x00000020

/* CPU speed patch with 10 000 000 (0x989680) LOOP */
#define PATCH_CPU_SPEED_V3 0x00000040

/* CPU speed patch with 10 000 000 (0x989680) LOOP + manually patched */
#define PATCH_CPU_SPEED_V4 0x00000080

#define PATCH_CPU_SPEED_V5 0x00000100
#define PATCH_CPU_SPEED_V6 0x00000200
#define PATCH_CPU_SPEED_V7 0x00000400
#define PATCH_CPU_SPEED_V8 0x00000800

/* CPU speed patch (for NDIS) with 1048576 (0x00100000) LOOP */
#define PATCH_CPU_SPEED_NDIS_V1 0x00001000

/* CPU speed patch (for NDIS) with 1 048 576 (0x00100000) LOOP with zero divide detection (WINDOWS 98 SE) */
#define PATCH_CPU_SPEED_NDIS_V2 0x00002000

/* CPU speed patch (for NDIS) with 10 485 760 (0x00A00000) LOOP - patched */
#define PATCH_CPU_SPEED_NDIS_V3 0x00008000

/* sumary defs */
#define PATCH_CPU_SPEED_ALL (PATCH_CPU_SPEED_V1|PATCH_CPU_SPEED_V2|PATCH_CPU_SPEED_V3|PATCH_CPU_SPEED_V4|\
	PATCH_CPU_SPEED_V5|PATCH_CPU_SPEED_V6|PATCH_CPU_SPEED_V7|PATCH_CPU_SPEED_V8)

#define PATCH_CPU_SPEED_NDIS_ALL (PATCH_CPU_SPEED_NDIS_V1|PATCH_CPU_SPEED_NDIS_V2|PATCH_CPU_SPEED_NDIS_V3)
#define PATCH_VMM_ALL (PATCH_VMM98|PATCH_VMMME|PATCH_VMM98_V2)

/*
 * Platform selection
 */

#if defined(__MSDOS__) || (defined(_WIN32) && !defined(_WIN64))
#define RUN_WITHOUT_ARGS
#endif

#if defined(__MSDOS__)
#define DOS_MODE
#endif

/*
 * Functions
 */
 
int cab_search_unpack(const char *dirname, const char *infilename, const char *out);
int cab_unpack(const char *srccab, const char *infilename, const char *out);
int cab_search_unpack(const char *dirname, const char *infilename, const char *out);

int wx_unpack(const char *src, const char *infilename, const char *out, const char *tmpname);

int patch_apply(const char *srcfile, const char *dstfile, int flags, int *applied);
int patch_apply_wx(const char *srcfile, const char *dstfile, const char *tmpname, int flags);
int patch_backup_file(const char *path, int nobackup);

void print_trace();
FILE *fopen_log(const char *fn, const char *mode, const char *file, int line);
#define FOPEN_LOG(_fn, _mode) fopen_log(_fn, _mode, __FILE__, __LINE__)

void cputest();

#endif /* __PATCHER9X_INCLUDED__ */
