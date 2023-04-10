/******************************************************************************
 * Copyright (c) 2022-2023 Jaroslav Hensl                                     *
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
 ******************************************************************************/
#include "patcher9x.h"

/**
 * Split pair source|destination to individual string, if name has no '|', source and destination are same
 *
 * return: 0 on failure
 *
 * NOTE: free both string with free()
 *
 **/
static int name_split(const char *arg, char **source, char **dest)
{
	char *ptr = strchr(arg, '|');
	
	if(ptr != NULL)
	{
		int p1 = ptr - arg;
		int p2 = strlen(ptr+1);
		
		if(p1 > 0 && p2 > 0)
		{
			*source = malloc(p1 + 1);
			if(*source == NULL)
			{
				return 0;
			}
			*dest = malloc(p2 + 1);
			if(*dest == NULL)
			{
				free(*source);
				*source = NULL;
				return 0;
			}
			
			memcpy(*source, arg, p1);
			(*source)[p1] = '\0';
			strcpy(*dest, ptr+1);
			
			return 1;
		}
	}
	
	int p = strlen(arg);

	*source = malloc(p + 1);
	if(*source == NULL)
	{
		return 0;
	}
	
	*dest = malloc(p + 1);
	if(*dest == NULL)
	{
		free(*source);
		*source = NULL;
		return 0;
	}
	
	strcpy(*source, arg);
	strcpy(*dest,   arg);
	
	return 1;
}

/**
 * The list of files in single CAB file
 *
 * required argv[0]: source archive
 *
 **/
int batch_cab_list(options_t *options, int argc, char **argv)
{
	if(argc == 0)
	{
		fprintf(stderr, "missing archive name\n");
		return -1;
	}
	
	cab_filelist_t *list = cab_filelist_open(argv[0]);
	
	if(!list)
	{
		fprintf(stderr, "Failed to open CAB %s\n", argv[0]);
		return -1;
	}
	
	char *basename = fs_basename(argv[0]);
	
	printf("--------------------------------\n");
	printf("CAB: %s\n", basename);
	printf("--------------------------------\n");
	
	const char *fn;
	while((fn = cab_filelist_get(list)) != NULL)
	{
		printf("%s\n", fn);
	}
	
	printf("--------------------------------\n");
	
	if(basename)
	{
		fs_path_free(basename);
	}
	
	cab_filelist_close(list);
	
	return 0;
}

/**
 * Extract one or more files from archive
 *
 * required argv[0]: source archive
 * required argv[1]: filename|destination
 * optional argv[2]: another filename|destination
 *          argv[ ]: ...
 * 
 **/
int batch_cab_extract(options_t *options, int argc, char **argv)
{
	if(argc == 0){fprintf(stderr, "missing archive name\n"); return -1;}
	if(argc == 1){fprintf(stderr, "missing name of file in archive\n"); return -1;}
		
	int r = PATCH_OK;
	int i;
	
	for(i = 1; i < argc; i++)
	{
		char *inname = NULL;
		char *outname = NULL;
		if(!name_split(argv[i], &inname, &outname))
		{
			fprintf(stderr, "Memory allocation fail!\n");
			return -2;
		}
		
		if(cab_unpack(argv[0], inname, outname, NULL) <= 0)
		{
			fprintf(stderr, "File %s in %s not found!\n", inname, argv[0]);
			r = PATCH_E_NOTFOUNDINCABS;
		}
		
		free(inname);
		free(outname);
	}
	
	return r;
}

/**
 * Extract one or more files from archive
 *
 * required argv[0]: directory with *.CAB
 * required argv[1]: filename|destination
 * optional argv[2]: another filename|destination
 *          argv[ ]: ...
 * 
 **/
int batch_cabs_extract(options_t *options, int argc, char **argv)
{
	if(argc == 0){fprintf(stderr, "missing archive name\n"); return -1;}
	if(argc == 1){fprintf(stderr, "missing name of file in archive\n"); return -1;}
		
	int r = PATCH_OK;
	int i;
	
	for(i = 1; i < argc; i++)
	{
		char *inname = NULL;
		char *outname = NULL;
		if(!name_split(argv[i], &inname, &outname))
		{
			fprintf(stderr, "Memory allocation fail!\n");
			return -2;
		}
		
		if(cab_search_unpack(argv[0], inname, outname) <= 0)
		{
			fprintf(stderr, "File %s in *.cab on %s not found!\n", inname, argv[0]);
			r = PATCH_E_NOTFOUNDINCABS;
		}
		
		free(inname);
		free(outname);
	}
	
	return r;
}

/**
 * The list of files in VXD archive (W3 or W4)
 *
 * required argv[0]: source archive
 *
 **/
int batch_vxd_list(options_t *options, int argc, char **argv)
{
	if(argc == 0){fprintf(stderr, "Missing archive name\n"); return -1;}
	
	char *tmpname = fs_path_get3(argv[0], "_vxd.tmp", NULL);
	if(!tmpname)
	{
		fprintf(stderr, "Memory allocation fail!\n");
		return -2;
	}
	
	vxd_filelist_t *list = vxd_filelist_open(argv[0], tmpname);
	if(!list)
	{
		fprintf(stderr, "Failed to open VXD archive (invalid file or read-only filesystem)\n");
		fs_path_free(tmpname);
		return -1;
	}
	
	printf("--------------------------------\n");
	printf("Int.name |  File name\n");
	printf("--------------------------------\n");
	
	const char *fn;
	char fnfull[PE_W3_FILE_NAME_SIZE + 1 + 4];
	while((fn = vxd_filelist_get(list)) != NULL)
	{
		strcpy(fnfull, fn);
		char *s = strchr(fnfull, ' ');
		if(s)
		{
			*s = '\0';
		}
		strcat(fnfull, ".VXD");
		
		printf("%s => %s\n", fn, fnfull);
	}
	
	printf("--------------------------------\n");
	
	vxd_filelist_close(list);
	fs_path_free(tmpname);
	
	return 0;
}

/**
 * Extract one or more files from VXD archive
 *
 * required argv[0]: source archive
 * required argv[1]: driver|destination
 * optional argv[2]: another driver|destination
 *          argv[ ]: ...
 * 
 **/
int batch_vxd_extract(options_t *options, int argc, char **argv)
{
	if(argc == 0){fprintf(stderr, "missing archive name\n"); return -1;}
	if(argc == 1){fprintf(stderr, "missing name of driver in archive\n"); return -1;}	
		
	int r = PATCH_OK;
	int i;
	
	for(i = 1; i < argc; i++)
	{
		char *inname = NULL;
		char *outname = NULL;
		char *tmpname = NULL;
		if(!name_split(argv[i], &inname, &outname))
		{
			fprintf(stderr, "Memory allocation fail!\n");
			return -2;
		}
		
		tmpname = fs_path_get3(outname, "_vxd.tmp", NULL);
		if(tmpname == NULL)
		{
			free(inname);
			free(outname);
			fprintf(stderr, "Memory allocation fail!\n");
			return -2;
		}
		
		int t = wx_unpack(argv[0], inname, outname, tmpname);
		if(t != PATCH_OK)
		{
			fprintf(stderr, "File %s in %s not found!\n", inname, argv[0]);
			report_error(t);
			r = t;
		}
		
		fs_path_free(tmpname);
		free(inname);
		free(outname);
	}
	
	return r;
}

/**
 * Convert W3->W4 and vice-versa
 * 
 *
 **/
int batch_vxd_convert(options_t *options, int argc, char **argv)
{
	int r = PATCH_OK;
	
	if(argc == 0){fprintf(stderr, "missing archive name\n"); return -1;}
		
	char *inname = NULL;
	char *outname = NULL;
	char *tmpname = NULL;
	
	if(!name_split(argv[0], &inname, &outname))
	{
		fprintf(stderr, "Memory allocation fail!\n");
		return -2;
	}
	
	tmpname = fs_path_get3(outname, "_vxd.tmp", NULL);
	if(tmpname == NULL)
	{
		free(inname);
		free(outname);
		fprintf(stderr, "Memory allocation fail!\n");
		return -2;
	}
		
	if(options->force_w3)
	{
		int t = wx_to_w3(inname, tmpname);
		if(t == PATCH_OK)
		{
			fs_rename(tmpname, outname);
		}
		else
		{
			report_error(t);
			r = t;
		}
	}
	else if(options->force_w4)
	{
		int t = wx_to_w4(inname, tmpname);
		if(t == PATCH_OK)
		{
			fs_rename(tmpname, outname);
		}
		else
		{
			report_error(t);
			r = t;
		}
	}
	else
	{
		printf("specify -force-w3 or -force-w4 to choose target format\n");
	}
	
	fs_path_free(tmpname);
	free(inname);
	free(outname);
	return r;
}

/**
 * Extract one or more files from VXD archive
 *
 * required argv[0]: source VXD archive
 * required argv[1]: destination dir
 * 
 **/
int batch_vxd_extract_all(options_t *options, int argc, char **argv)
{
	int r = PATCH_OK;
	
	if(argc == 0){fprintf(stderr, "missing archive name\n"); return -1;}
	if(argc == 1){fprintf(stderr, "missing the destination dir\n"); return -1;}
		
	char *tmpname = fs_path_get(argv[1], "_vxd.tmp", NULL);
	if(tmpname == NULL)
	{
		fprintf(stderr, "Memory allocation fail!\n");
		return -2;
	}
	
	vxd_filelist_t *list = vxd_filelist_open(argv[0], tmpname);
	
	if(!list)
	{
		fprintf(stderr, "Failed to open VXD archive (invalid file or read-only filesystem)\n");
		fs_path_free(tmpname);
		return -1;
	}
		
	const char *fn;
	char fnfull[PE_W3_FILE_NAME_SIZE + 1 + 4];
	while((fn = vxd_filelist_get(list)) != NULL)
	{
		strcpy(fnfull, fn);
		char *s = strchr(fnfull, ' ');
		if(s)
		{
			*s = '\0';
		}
		strcat(fnfull, ".VXD");
		
		int t = PATCH_E_MEM;
		char *out = fs_path_get(argv[1], fnfull, NULL);
		if(out)
		{
			if(fs_file_exists(out))
			{
				printf("%s ignored, file (%s) exist\n", fnfull, out);
				fs_path_free(out);
				continue;
			}
			t = wx_unpack(argv[0], fnfull, out, tmpname);
			fs_path_free(out);
		}
		if(t != PATCH_OK)
		{
			report_error(t);
			r = t;
		}
		else
		{
			printf("Extracted %s to %s\n", fnfull, argv[1]);
		}
	}
	
	vxd_filelist_close(list);
	fs_path_free(tmpname);
	
	return r;
}

/**
 * Apply patch to file
 *
 **/
static int batch_patch_sel(options_t *options, int argc, char **argv, uint32_t patches)
{
	int r = PATCH_OK;
	int type;
	int i;
	
	if(argc == 0){fprintf(stderr, "missing at least one file tu patch\n"); return -1;}
	
	for(i = 0; i < argc; i++)
	{
		char *inname = NULL;
		char *outname = NULL;
		if(!name_split(argv[i], &inname, &outname))
		{
			fprintf(stderr, "Memory allocation fail!\n");
			return -2;
		}
		
		dos_header_t dos;
		pe_header_t pe;
		
		char *temp1 = fs_path_get3(outname, "_wx.tmp", NULL);
		if(temp1 == NULL)
		{
			free(inname);
			free(outname);
			
			fprintf(stderr, "Memory allocation fail!\n");
			return -2;
		}
		
		char *temp2 = fs_path_get3(outname, "_vxd.tmp", NULL);
		if(temp2 == NULL)
		{
			free(inname);
			free(outname);
			fs_path_free(temp1);
			
			fprintf(stderr, "Memory allocation fail!\n");
			return -2;
		}
		
		FILE *fr = fopen(inname, "rb");
	
		if(!fr)
		{
			fprintf(stderr, "Can't open %s!\n", inname);
			
			free(inname);
			free(outname);
			fs_path_free(temp1);
			fs_path_free(temp2);
			continue;
		}
		
		type = pe_read(&dos, &pe, fr);
		if(type == PE_W3 || type == PE_W4)
		{
			fclose(fr);
			
			int t = patch_apply_wx(inname, temp1, temp2, patches);
			if(t == PATCH_OK)
			{
				fs_rename(temp1, outname);
			}
			else
			{
				report_error(t);
				//fprintf(stderr, "error: %d\n", t);
			}
			
			fs_path_free(temp1);
			fs_path_free(temp2);
			free(inname);
			free(outname);
			
			continue;
		} // is archive
		else
		{
			if(type != PE_LE)
			{
				fprintf(stderr, "Warning: file %s is not driver!\n", argv[i]);
			}
			
			fseek(fr, 0, SEEK_SET);
			int t = patch_selected(fr, temp1, patches, NULL, NULL);
			fclose(fr);
			if(t == PATCH_OK)
			{
				fs_rename(temp1, outname);
			}
			else
			{
				report_error(t);
				//fprintf(stderr, "error: %d\n", t);
			}
			
			fs_path_free(temp1);
			fs_path_free(temp2);
			free(inname);
			free(outname);
		} // is not archive
	} // for
	
	return r;
}

int batch_patch_all(options_t *options, int argc, char **argv)
{
	return batch_patch_sel(options, argc, argv, PATCH_CPU_SPEED_ALL | PATCH_CPU_SPEED_NDIS_ALL | PATCH_VMM_ALL);
}

int batch_patch_tlb(options_t *options, int argc, char **argv)
{
	return batch_patch_sel(options, argc, argv, PATCH_VMM_ALL);
}

int batch_patch_cpuspeed(options_t *options, int argc, char **argv)
{
	return batch_patch_sel(options, argc, argv, PATCH_CPU_SPEED_ALL);
}

int batch_patch_cpuspeed_ndis(options_t *options, int argc, char **argv)
{
	return batch_patch_sel(options, argc, argv, PATCH_CPU_SPEED_NDIS_ALL);
}

#include "batch.h"

int batch_arg(const char *arg)
{
	batch_action_t *action = &(actions[0]);
	int i;
	
	for(i = 0; action->func != NULL; action++,i++)
	{
		if(istrcmp(action->name, arg) == 0)
		{
			return i;
		}
	}
	
	return -1;
}

int batch_run(options_t *options, int id, int argc, char **argv)
{
	return actions[id].func(options, argc, argv);
}

