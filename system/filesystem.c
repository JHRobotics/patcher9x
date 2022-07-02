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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "filesystem.h"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#endif

#define INT_MAGIC 0xF011EECC

static char name_buffer[MAX_PATH];

typedef struct _fs_dir_int_t
{
	uint32_t magic;
	#ifdef _WIN32
	WIN32_FIND_DATA ffd;
	BOOL   ffd_valid;
	HANDLE hFind;
	#else
	DIR *dir;
	#endif
} fs_dir_int_t;

static fs_dir_int_t *resource_alloc()
{
	fs_dir_int_t *res = (fs_dir_int_t*)malloc(sizeof(fs_dir_int_t));
	if(res != NULL)
	{
		res->magic = INT_MAGIC;
		#ifdef _WIN32
		res->hFind = INVALID_HANDLE_VALUE;
		memset(&(res->ffd), 0, sizeof(WIN32_FIND_DATA));
		#else
		res->dir = NULL;
		#endif
	}
	
	return res;
}

static fs_dir_int_t *resource_load(fs_dir_t *dir)
{
	fs_dir_int_t *res  = (fs_dir_int_t*)dir;
	
	if(res != NULL)
	{
		if(res->magic == INT_MAGIC)
		{
			return res;
		}
	}
	
	return NULL;
}

static void resource_free(fs_dir_t **dir)
{
	fs_dir_int_t *res = resource_load(*dir);
	
	if(res != NULL)
	{
		free(res);
		*dir = NULL;
	}
}

/**
 * Open directory for listing
 *
 * @param path: path to direcory
 * @return: resource of type fs_dir_t, NULL on error
 *
 **/
fs_dir_t *fs_dir_open(const char *path)
{
	fs_dir_int_t *res = resource_alloc();
	
	if(res == NULL)
	{
		return NULL;
	}
	
	#ifdef _WIN32
	if(strlen(path) + strlen("\\*") + 1 >= MAX_PATH)
	{
		resource_free((fs_dir_t**)&res);
		return NULL;
	}
	
	strcpy(name_buffer, path);
	strcat(name_buffer, "\\*");
	
	res->hFind = FindFirstFileA(name_buffer, &(res->ffd));
	
	if(res->hFind == INVALID_HANDLE_VALUE)
	{
		resource_free((fs_dir_t**)&res);
		return NULL;
	}
	else
	{
		res->ffd_valid = TRUE;
	}
	
	return (fs_dir_t*)res;
	#else
	res->dir = opendir(path);
	if(res->dir != NULL)
	{
		return (fs_dir_t*)res;
	}
	else
	{
		resource_free((fs_dir_t**)&res);
		return NULL;
	}
	#endif
	
	return NULL;
}

/**
 * Close directory
 *
 * @param dir: pointer to return of fs_dir_open, destination will be set to NULL
 *
 **/
void fs_dir_close(fs_dir_t **dir)
{
	fs_dir_int_t *res = resource_load(*dir);
	
	if(res != NULL)
	{
		#ifdef _WIN32
		if(res->hFind != INVALID_HANDLE_VALUE)
		{
			FindClose(res->hFind);
		}
		#else
		if(res->dir != NULL)
		{
			closedir(res->dir);
		}
		#endif
		
		resource_free(dir);
	}
}

/**
 * Read files from directory
 * @param dir: pointer to return of fs_dir_open
 * @param filter: FS_FILTER_FILE or FS_FILTER_DIR for filtering result
 *
 * @return: file name readed from directory, NULL if no other files
 *          are this directory.
 *
 * NOTE: result is saved in static buffer, so any other directory operation
 *       could overwrite it. Use for example fs_path_dup to copy it.
 *
 **/
const char *fs_dir_read(fs_dir_t *dir, int filter)
{
	fs_dir_int_t *res = resource_load(dir);
	if(res == NULL)
	{
		return NULL;
	}
	
	#ifdef _WIN32
	if(res->ffd_valid != FALSE)
	{
		if((filter & FS_FILTER_FILE) != 0)
		{
			while((res->ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
				res->ffd_valid = FindNextFileA(res->hFind, &(res->ffd));
				if(res->ffd_valid == FALSE)
				{
					break;
				}
			}
			
			if(res->ffd_valid != FALSE)
			{
				strcpy(name_buffer, res->ffd.cFileName);
				res->ffd_valid = FindNextFileA(res->hFind, &(res->ffd));
			}
			else
			{
				return NULL;
			}
		}
		else if((filter & FS_FILTER_DIR) != 0)
		{
			while((res->ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				res->ffd_valid = FindNextFileA(res->hFind, &(res->ffd));
				if(res->ffd_valid == FALSE)
				{
					break;
				}
			}
			
			if(res->ffd_valid != FALSE)
			{
				strcpy(name_buffer, res->ffd.cFileName);
				res->ffd_valid = FindNextFileA(res->hFind, &(res->ffd));
			}
			else
			{
				return NULL;
			}
		}
		else
		{
			strcpy(name_buffer, res->ffd.cFileName);
			res->ffd_valid = FindNextFileA(res->hFind, &(res->ffd));
		}
		
		return (const char *)name_buffer;
	}
	#else
	if(res->dir != NULL)
	{
		int valid = 0;
		struct dirent *entry;
		
		while((entry = readdir(res->dir)) != NULL)
		{
			if((filter & FS_FILTER_FILE) != 0)
			{
				if(entry->d_type == DT_REG)
				{
					//printf("file: %s\n", entry->d_name);
					strcpy(name_buffer, entry->d_name);
					valid = 1;
					break;
				}
			}
			else if((filter & FS_FILTER_DIR) != 0)
			{
				if(entry->d_type == DT_DIR)
				{
					strcpy(name_buffer, entry->d_name);
					valid = 1;
					break;
				}
			}
			else
			{
				strcpy(name_buffer, entry->d_name);
				valid = 1;
				break;
			}
		}
		
		if(valid)
		{
			return (const char *)name_buffer;;
		}
		
	}
	#endif
	
	return NULL;
}

/**
 * Construct path
 *
 * @param dirname: directory name, could be NULL
 * @param filename: name of file (with or without extension), cannot be NULL
 * @param extname: extension name, could be NULL (extension is leaved as is in
 *                 filename), could be empty string ("", extension will be
 *                 deleted) or it can be any string (extension will be replaced)
 *
 * @return: new path, call fs_path_free to free it
 *
 * NOTE: this function does not read any information from filesystem.
 *
 **/
char *fs_path_get(const char *dirname, const char *filename, const char *extname)
{
	char *extptr = NULL;
	size_t len = 0;
	
	if(filename == NULL)
	{
		return NULL; /* filename couldn't be NULL */
	}
	len = strlen(filename) + 1;
	
	if(dirname != NULL)
	{
		len += strlen(dirname) + sizeof(PATH_SEPARATOR);
	}
	
	if(extname != NULL)
	{
		extptr = strrchr(filename, '.');
		if(extptr != NULL)
		{
			len -= strlen(filename) - (extptr - filename) - 1;
		}
		
	  len += strlen(extname) + 1;
	}
	
	if(len <= MAX_PATH)
	{
		char *buf = (char*)malloc(len);
		buf[0] = '\0';
		
		if(dirname != NULL)
		{
			strcpy(buf, dirname);
			strcat(buf, PATH_SEPARATOR);
		}
		
		if(extptr == NULL)
		{
			strcat(buf, filename);
		}
		else
		{
			strncat(buf, filename, (extptr - filename));
		}
		
		if(extname != NULL && strlen(extname) > 0)
		{
			strcat(buf, ".");
			strcat(buf, extname);
		}
		
		return buf;
	}
	
	return NULL;
}

/**
 * Replacing parts of path
 *
 * @param path: to filename or directory
 * @param: filename: filename to replace (if NULL leave as is)
 * @param extname: extension name to replace (if NULL leave as is)
 *
 * @return: new path, call fs_path_free to free it
 *
 * NOTE: this function READ information of target from filesystem to decide if
 *       is file or directory (if isn't exists assume that is file).
 *
 **/
char *fs_path_get2(const char *target, const char *filename, const char *extname)
{
	if(fs_is_dir(target))
	{
		return fs_path_get(target, filename, extname);
	}
	else
	{
		return fs_path_get3(target, filename, extname);
	}
	
	return NULL;
}

/**
 * Replacing parts of path
 *
 * @param filepath: path to file
 * @param filename: filename to replace (if NULL leave as is)
 * @param extname: extension name to replace (if NULL leave as is)
 *
 * @return: new path, call fs_path_free to free it
 *
 * NOTE: this function does not read any information from filesystem.
 *
 **/
char *fs_path_get3(const char *filepath, const char *filename, const char *extname)
{
	char *basename = NULL;
	char *path     = NULL;
	char *dirname  = fs_dirname(filepath);
		
	if(filename == NULL)
	{
		basename = fs_basename(filepath);
		path = fs_path_get(dirname, basename, extname);
	}
	else
	{
		path = fs_path_get(dirname, filename, extname);
	}
	
	if(basename)
	{
		fs_path_free(basename);
	}
		
	if(dirname)
	{
		fs_path_free(dirname);
	}
		
	return path;
}

void fs_path_free(char *path)
{
	free(path);
}

/**
 * Duplicate (alloc + copy) path string
 *
 * @param path: string to duplicate
 * @return: duplicated string, use fs_path_free to free this resource
 *
 **/
char *fs_path_dup(const char *path)
{
	char *new_path = NULL;
	size_t len;
	
	if(path != NULL)
	{
		len = strlen(path);
		new_path = malloc(len);
		memcpy(new_path, path, len+1);		
	}
	
	return new_path;
}

/**
 * Check if file extension match
 *
 * @param filename: filename 
 * @param ext: extension name to check
 *
 * @return: 0 if extensions NOT match, extensions are compare without
 *          case-sensitivity.
 *
 **/
int fs_ext_match(const char *filename, const char *ext)
{
	char *ptr = strrchr(filename, '.');
	if(ptr != NULL)
	{
		int c1, c2;
		char *ptr_ext = (char*)ext;
		++ptr;
		
		do
		{
			c1 = *ptr++;
			c2 = *ptr_ext++;
			
			if(c1 >= 'A' && c1 <= 'Z')
			{
				c1 = c1 - 'A' + 'a';
			}
			
			if(c2 >= 'A' && c2 <= 'Z')
			{
				c2 = c2 - 'A' + 'a';
			}
			
			if(c1 != c2)
			{
				return 0;
			}
		}
		while(c1 != '\0' && c2 != '\0');
		
		return 1;
	}
	
	return 0;
}

/**
 * Copy source to destination
 * 
 * @param src: readable FILE resource
 * @param dst: writable FILE resource
 * @param size: size of copy, if 0 copy to EOF
 *
 * @return: number of copy bytes, negative on error
 *
 **/
ssize_t fs_file_copy(FILE *src, FILE *dst, size_t size)
{
	uint8_t *buf = malloc(FS_COPY_BUF_SIZE);
	size_t copy_size, to_read, readed = 0;
	if(buf == NULL)
	{
		return -2;
	}
	
	/* copy to file end */
	if(size == 0)
	{
		for(copy_size = 0; !feof(src); copy_size += readed)
		{
			readed = fread(buf, 1, FS_COPY_BUF_SIZE, src);
			fwrite(buf, 1, readed, dst);
		}
	}
	else
	{
		for(copy_size = 0; copy_size < size && !feof(src); copy_size += readed)
		{
			to_read = size - copy_size;
			if(to_read >= FS_COPY_BUF_SIZE)
			{
				to_read = FS_COPY_BUF_SIZE;
			}
			
			readed = fread(buf, 1, to_read, src);
			fwrite(buf, 1, readed, dst);
		}
	}
	
	free(buf);
	
	return copy_size;
}

/**
 * Copy source to destination
 * 
 * @param src: path to source file
 * @param dst: path to destination file
 *
 * @return: number of copy bytes, negative on error
 *
 **/
ssize_t fs_file_fullcopy(const char *src, const char *dst)
{
	ssize_t result = -1;
	
	FILE *fr = fopen(src, "rb");
	if(fr)
	{
		FILE *fw = fopen(dst, "wb");
		if(fw)
		{
			
			result = fs_file_copy(fr, fw, 0);
			fclose(fw);
		}
		
		fclose(fr);
	}
	
	return result;
}

/**
 * Return file size
 *
 * @param path: path to file
 * @return: positive number indicate file size, negative error
 *
 **/
ssize_t fs_file_size(const char *path)
{
	ssize_t result = -1;
	FILE *fr;
	
	fr = fopen(path, "rb");
	if(fr)
	{
		fseek(fr, 0, SEEK_END);
		result = ftell(fr);
		fclose(fr);
	}
	
	return result;
}

/**
 * Check if file exists and is readable
 *
 * @param filename: path to file, must be file, function not works for
 *                  directory
 * @return: 0 if file NOT exists.
 *
 **/
int fs_file_exists(const char *filename)
{
	int result = 0;
	FILE *fp = fopen(filename, "rb");
	if(fp)
	{
		result = 1;
		fclose(fp);
	}
	
	return result;
}

/**
 * Return directory name from path.
 *
 * @param path: path to file
 *
 * @return: path with striped element after last or NULL if last element
 *          is first one
 *
 * NOTE: This is only simple string operation, function NOT checking if directory
 *       exists on file system. 
 *
 **/
char *fs_dirname(const char *path)
{
	ssize_t len;
	int break_next = 0;

	/* REMOVED: in lots of cases this function isn't called on real FS paths,
	 *            which could lead to confusing bahavior.
	 * if(fs_is_dir(path)) // if path is real directory simply return path as it's
	 * {	
	 *   fs_path_dup(path);
	 * }
	 */
	
	for(len = strlen(path); len > 0; len--)
	{
		if(path[len-1] == (PATH_SEPARATOR)[0])
		{
			break_next = 1;
		}
#ifdef PATH_SEPARATOR_ALT
		else if(path[len-1] == (PATH_SEPARATOR_ALT)[0])
		{
			break_next = 1;
		}
#endif
		else if(break_next)
		{
			break;
		}
	}
	
	if(len > 0)
	{
		char *str = malloc(len+1);
		if(str != NULL)
		{
			memcpy(str, path, len);
			str[len] = '\0';
			return str;
		}
	}
	
	return NULL;
}

/**
 * Return file name from path.
 *
 * @param path: path to file
 *
 * @return: file name without path
 *
 * NOTE: This is only simple string operation, function NOT checking if directory
 *       exists on file system. 
 *
 **/
char *fs_basename(const char *path)
{
	ssize_t len;
	int path_len = strlen(path);
	
	for(len = path_len; len > 0; len--)
	{
		if(path[len-1] == (PATH_SEPARATOR)[0])
		{
			break;
		}
#ifdef PATH_SEPARATOR_ALT
		else if(path[len-1] == (PATH_SEPARATOR_ALT)[0])
		{
			break;
		}
#endif
	}
	
	if(len > 0)
	{
		char *str = malloc(path_len-len+1);
		if(str != NULL)
		{
			memcpy(str, path+len, path_len-len);
			str[path_len-len] = '\0';
			
			return str;
		}
	}
	else
	{
		/* path is basename only, only copy */
		char *str = malloc(path_len+1);
		if(str)
		{
			memcpy(str, path, path_len+1);
			
			return str;
		}
	}
	
	return NULL;
}

/**
 * Check if path lead to diractory
 * 
 * @param path: path to check
 * @return: non zero if is path directory otherwise 0
 *
 **/
int fs_is_dir(const char *path)
{
	#ifdef _WIN32
	DWORD atrs = GetFileAttributesA(path);
	if(atrs != INVALID_FILE_ATTRIBUTES)
	{
		return (atrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
	#else
	struct stat path_stat;
	memset(&path_stat, 0, sizeof(struct stat));
	stat(path, &path_stat);
	return S_ISDIR(path_stat.st_mode);
	#endif
	return 0;
}

/**
 * Delete file
 *
 * @param path: file to delete
 * @return: 0 on success
 *
 **/
int fs_unlink(const char *path)
{
	return unlink(path);
}

/**
 * Check if dir is writeable
 * 
 * @param path: path to check
 * @param tmpname: termporaty file name for testing, if NULL use default test file
 *
 * @return 0 if is NOT writable otherwise non zero
 **/
int fs_is_writeable_dir(const char *path, const char *tmpname)
{
	FILE *fp;
	char *testfile = NULL;
	int result = 0;
	const char readbuf[16] = "QWERTYUIOP123456";
	char writebuf[16];
	
	if(tmpname == NULL)
	{
		tmpname = "_w.tmp";
	}
	
	if(fs_is_dir(path))
	{
		testfile = fs_path_get(path, tmpname, NULL);
	}
	else
	{
		char *dirname = fs_dirname(path);
		testfile = fs_path_get(dirname, tmpname, NULL);
		if(dirname)
		{
			fs_path_free(dirname);
		}
	}
	
	if(testfile != NULL)
	{
		fp = fopen(testfile, "wb");
		if(fp)
		{
			fwrite(readbuf, sizeof(readbuf), 1, fp);
			fclose(fp);
			
			fp = fopen(testfile, "rb");
			if(fp)
			{
				if(fread(writebuf, 1, sizeof(readbuf), fp) == sizeof(readbuf))
				{
					if(memcmp(writebuf, readbuf, sizeof(readbuf)) == 0)
					{
						result = 1;
					}
				}
				fclose(fp);
				
				fs_unlink(testfile);
			}
		}
		
		fs_path_free(testfile);
	}
	
	return result;	
}

/**
 * Rename file, this operation is NOT atomic!
 * (due to platform depended implementation of rename)
 *  This function use copy if file cannot be renamed.
 * 
 * @param oldname: file to rename
 * @param newname: destination
 *
 * @return: 0 on success
 *
 **/
int fs_rename(const char *oldname, const char *newname)
{
	if(fs_file_exists(oldname))
	{
		int t;
		fs_unlink(newname);
		t = rename(oldname, newname);
		if(t != 0)
		{
			if(fs_file_fullcopy(oldname, newname) >= 0)
			{
				if(fs_unlink(oldname) == 0)
				{
					return 0;
				}
			}
		}
		else
		{
			return 0;
		}
	}
	
	return -1;
}

/**
 * Create the directory
 * 
 * @param dirname: new directory path. Function is system implementation
 *                 depended, but it in generally creates only last segment of
 *                 directory tree. Example: if you want to create /tmp/foo/boo
 *                 (and only /tmp exists), you should call:
 *                    fs_mkdir("/tmp/foo");
 *                    fs_mkdir("/tmp/foo/boo");
 *
 * @return: 0 on success
 *
 **/
int fs_mkdir(const char *dirname)
{
#ifdef _WIN32
	return _mkdir(dirname);
#else
	return mkdir(dirname, 0777);
#endif
}
