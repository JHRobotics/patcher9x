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

#define PATCH_FORCE_W3 1
#define PATCH_FORCE_W4 2

#if defined(__MSDOS__) || (defined(_WIN32) && !defined(_WIN64))
#define RUN_WITHOUT_ARGS
#endif

#if defined(__MSDOS__)
#define DOS_MODE
#endif

int cab_search_unpack(const char *dirname, const char *infilename, const char *out);
int cab_unpack(const char *srccab, const char *infilename, const char *out);
int cab_search_unpack(const char *dirname, const char *infilename, const char *out);

int wx_unpack(const char *src, const char *infilename, const char *out, const char *tmpname);

int patch_apply(const char *srcfile, const char *dstfile);
int patch_apply_wx(const char *srcfile, const char *dstfile, const char *tmpname, int force_format);
int patch_backup_file(const char *path, int nobackup);

void print_trace();
FILE *fopen_log(const char *fn, const char *mode, const char *file, int line);
#define FOPEN_LOG(_fn, _mode) fopen_log(_fn, _mode, __FILE__, __LINE__)

#endif /* __PATCHER9X_INCLUDED__ */
