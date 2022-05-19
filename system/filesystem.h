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
#ifndef __FILESYSTEM_H__INCLUDED__
#define __FILESYSTEM_H__INCLUDED__

typedef void *fs_dir_t;

#define FS_FILTER_FILE 1
#define FS_FILTER_DIR  2

#define FS_COPY_BUF_SIZE 8192

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

fs_dir_t   *fs_dir_open(const char *path);
void        fs_dir_close(fs_dir_t **dir);
const char *fs_dir_read(fs_dir_t *dir, int filter);

char       *fs_path_get(const char *dirname, const char *filename, const char *extname);
char       *fs_path_get2(const char *target, const char *filename, const char *extname);
void        fs_path_free(char *path);

int         fs_ext_match(const char *filename, const char *ext);

int         fs_dir_create(const char *path);
int         fs_rename(const char *oldname, const char *newname);
ssize_t     fs_file_copy(FILE *src, FILE *dst, size_t size);
int         fs_file_exists(const char *filename);

int         fs_mkdir(const char *dirname);

char       *fs_dirname(const char *path);
char       *fs_basename(const char *path);
int         fs_is_dir(const char *path);
int         fs_unlink(const char *path);
int         fs_is_writeable_dir(const char *path, const char *tmpname);

#endif /* __FILESYSTEM_H__INCLUDED__ */
