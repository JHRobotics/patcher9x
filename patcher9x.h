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
#include "nocrt.h"

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

/* TLB patch for windows 98 (old version) */
#define PATCH_VMM98_OLD 0x00010000

/* TLB patch for windows 98 (oldversion) (for updated VMM.VXD) */
#define PATCH_VMM98_OLD_V2 0x00020000

/* TLB patch for windows 98 (older but more universal) */
#define PATCH_VMM98_SIMPLE 0x00040000

/* TLB patch for windows 98 (older but more universal - for updated VMM.VXD) */
#define PATCH_VMM98_SIMPLE_V2 0x00080000

/* CPU speed patch (for NDIS) with 10 485 760 (0x00A00000) LOOP - patched (Windows 3.11) */
#define PATCH_CPU_SPEED_NDIS_V4 0x00100000

/* rloew's patchmem (vcache.vxd) */
#define PATCH_MEM_VCACHE98        (1ULL << 32)
#define PATCH_MEM_VCACHE95        (1ULL << 33)
#define PATCH_MEM_VCACHEME        (1ULL << 34)

/* rloew's patchmem (vmm.vxd) */
#define PATCH_MEM98SE_PATCHMEM    (1ULL << 35) /* SE, FE+Q242161 */
#define PATCH_MEM98FE_PATCHMEM    (1ULL << 36) /* FE */
#define PATCH_MEM95_PATCHMEM      (1ULL << 37)
#define PATCH_MEMME_PATCHMEM      (1ULL << 38)
#define PATCH_MEMME_PATCHMEM_V2   (1ULL << 39) /* ME, Q296773 */

/* W3 unpack patch - 98 */
#define PATCH_MEM_W3_98           (1ULL << 40)
#define PATCH_MEM_W3_95           (1ULL << 41)

/* control registry fix */
#define PATCH_WIN_COM             (1ULL << 42)

/* sumary defs */
#define PATCH_CPU_SPEED_ALL (PATCH_CPU_SPEED_V1|PATCH_CPU_SPEED_V2|PATCH_CPU_SPEED_V3|PATCH_CPU_SPEED_V4|\
	PATCH_CPU_SPEED_V5|PATCH_CPU_SPEED_V6|PATCH_CPU_SPEED_V7|PATCH_CPU_SPEED_V8)

#define PATCH_CPU_SPEED_NDIS_ALL (PATCH_CPU_SPEED_NDIS_V1|PATCH_CPU_SPEED_NDIS_V2|PATCH_CPU_SPEED_NDIS_V3|PATCH_CPU_SPEED_NDIS_V4)

#define PATCH_VMM_ALL (PATCH_VMM98|PATCH_VMMME|PATCH_VMM98_V2| \
	PATCH_VMM98_OLD|PATCH_VMM98_OLD_V2|PATCH_VMM98_SIMPLE|PATCH_VMM98_SIMPLE_V2| \
	PATCH_MEM_VCACHE98|PATCH_MEM_VCACHE95|PATCH_MEM_VCACHEME|\
	PATCH_MEM98SE_PATCHMEM|PATCH_MEM98FE_PATCHMEM| \
	PATCH_MEM95_PATCHMEM| \
	PATCH_MEMME_PATCHMEM|PATCH_MEMME_PATCHMEM_V2|\
	PATCH_MEM_W3_95|PATCH_MEM_W3_98)

/* program modes */
#define MODE_AUTO        1 /* automaticly determine action from path */
#define MODE_INTERACTIVE 2 /* same as auto but ask user if sure */
#define MODE_EXACT       3 /* use steps by command line */
#define MODE_BATCH       4 /* use one step on command line */

/* patch lookup flags */
#define PATCH_LOOKUP_CABS      1 /* scan cab files */
#define PATCH_LOOKUP_EXTRACTWX 2 /* extract files from VX archives */
#define PATCH_LOOKUP_NO_VMM32  4 /* dont touch VMM32.VXD */
#define PATCH_LOOKUP_ONE_CAB   8 /* extract files from one specified cab */

/* paths and files */
#define DEFAULT_PATH           "C:\\Windows\\System"
#define DEFAULT_INPUT_VX       "VMM"
#define DEFAULT_INPUT_CAB      "VMM32.VXD"
#define DEFAULT_OUTPUT_LE      "VMM.VXD"
#define DEFAULT_OUTPUT_VX      "VMM32.VXD"

/*
 * Platform selection
 */

#if defined(__MSDOS__) || defined(_WIN32)
#define RUN_WITHOUT_ARGS
#endif

#if defined(__MSDOS__)
#define DOS_MODE
#endif

/*
 * Structures
 */
typedef struct _options_t
{
	int mode;
	const char *path;
	int print_help;
	int print_version;
	int cputest;
	int cab_extract;
	int wx_extract;
	int patch;
	int force_w3;
	int force_w4;
	int no_backup;
	uint64_t patches;
	uint64_t unmask;
	//int millennium;
	const char *input;
	const char *output;
} options_t;

typedef struct _pmodfiles_t *pmodfiles_t;

struct scanned_files_list;
typedef struct scanned_files_list scanned_files_list_t;

typedef struct _cab_scan_file_t
{
	char filename[MAX_PATH];
	char cabname[MAX_PATH];
	int  used;
	struct _cab_scan_file_t *next;
} cab_scan_file_t;

/*
 * Functions
 */
 
/* unpack.c */
int cab_search_unpack(const char *dirname, const char *infilename, const char *out);
int cab_unpack(const char *srccab, const char *infilename, const char *out, scanned_files_list_t *list);
int cab_lookup_build(const char *dir, cab_scan_file_t *interests);

int wx_unpack(const char *src, const char *infilename, const char *out, const char *tmpname);
int wx_to_w3(const char *in, const char *out);
int wx_to_w4(const char *in, const char *out);

struct cab_filelist;
typedef struct cab_filelist cab_filelist_t;
cab_filelist_t *cab_filelist_open(const char *file);
const char *cab_filelist_get(cab_filelist_t *list);
void cab_filelist_close(cab_filelist_t *list);

struct vxd_filelist;
typedef struct vxd_filelist vxd_filelist_t;
vxd_filelist_t *vxd_filelist_open(const char *file, const char *tmp);
const char *vxd_filelist_get(vxd_filelist_t *list);
void vxd_filelist_close(vxd_filelist_t *list);

/* patch.c */
int patch_apply(const char *srcfile, const char *dstfile, uint64_t flags, int *applied);
int patch_apply_wx(const char *srcfile, const char *dstfile, const char *tmpname, uint64_t flags);
int patch_backup_file(const char *path, int nobackup);
int patch_selected(FILE *fp, const char *dstfile, uint64_t to_apply, uint64_t *out_applied, uint64_t *out_exists);
void patch_print(uint64_t patches);

/* files.c */
pmodfiles_t files_lookup(const char *path, uint64_t global_flags, uint64_t global_unmask, uint32_t lookup_flags);
pmodfiles_t files_apply(const char *upath, uint64_t global_flags, uint64_t global_unmask);
int files_status(pmodfiles_t list);
void files_cleanup(pmodfiles_t *plist);
int files_commit(pmodfiles_t *plist, int nobackup);
void files_print(pmodfiles_t list);

/* exact.c */
int run_exact(options_t *options);

/* batch.c */
int batch_arg(const char *arg);
int batch_run(options_t *options, int id, int argc, char **argv);
void batch_help();

/* trace.c */
void print_trace();
FILE *fopen_log(const char *fn, const char *mode, const char *file, int line);
#define FOPEN_LOG(_fn, _mode) fopen_log(_fn, _mode, __FILE__, __LINE__)

void print_error(int code, const char *file, int line);
#define report_error(_code) print_error(_code, __FILE__, __LINE__);

/* cputest.c */
void cputest();

/* pwin32.c */
void set_default_path(char *default_path);

/*
 * Globals
 */

extern char patcher9x_default_path[MAX_PATH];

#endif /* __PATCHER9X_INCLUDED__ */
