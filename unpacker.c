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
#include "patcher9x.h"

#define cfhdrPREV_CABINET       0x0001
#define cfhdrNEXT_CABINET       0x0002
#define cfhdrRESERVE_PRESENT    0x0004

/*
 * MS CAB format
 *
 * FROM: https://docs.microsoft.com/en-us/previous-versions//bb267310(v=vs.85)?redirectedfrom=MSDN
 */
#define CAB_MAX_STR 255

#pragma pack(push)
#pragma pack(1)
typedef struct _mscab_t
{
	uint8_t   signature[4];  /*inet file signature */
  uint32_t  reserved1;     /* reserved */
  uint32_t  cbCabinet;     /* size of this cabinet file in bytes */
  uint32_t  reserved2;     /* reserved */
  uint32_t  coffFiles;     /* offset of the first CFFILE entry */
  uint32_t  reserved3;     /* reserved */
  uint8_t   versionMinor;  /* cabinet file format version, minor */
  uint8_t   versionMajor;  /* cabinet file format version, major */
  uint16_t  cFolders;      /* number of CFFOLDER entries in this */
                           /*    cabinet */
  uint16_t  cFiles;        /* number of CFFILE entries in this cabinet */
  uint16_t  flags;         /* cabinet file option indicators */
  uint16_t  setID;         /* must be the same for all cabinets in a */
                           /*    set */
  uint16_t  iCabinet;      /* number of this cabinet file in a set */
} mscab_t;
#pragma pack(pop)

/* CAB list item */
typedef struct _cab_list_item_t
{
	char *filename;
	struct mscabd_cabinet *cab;
	struct _cab_list_item_t *next;
} cab_list_item_t;

/* CAB list */
typedef struct _cab_list_t
{
	cab_list_item_t *first;
	cab_list_item_t *last;
} cab_list_t;

/* list of already scanned files */
struct scanned_files_item
{
	char *filename;
	struct scanned_files_item *next;
};

struct scanned_files_list
{
	struct scanned_files_item *first;
	struct scanned_files_item *last;
};

/* allocate and add item to the end of list */
static cab_list_item_t *cab_list_add(cab_list_t *list)
{
	cab_list_item_t *item = malloc(sizeof(cab_list_item_t));
	if(item)
	{
		memset(item, 0, sizeof(cab_list_item_t));
		item->next = NULL;
		if(list->last == NULL)
		{
			list->last = list->first = item;
		}
		else
		{
			list->last->next = item;
			list->last = item;
		}
		
		return item;
	}
	
	return NULL;
}

/* free all items in list and call fs_path_free on all stored filename */
static void cab_list_clear(cab_list_t *list)
{
	cab_list_item_t *item;
	while(list->first != NULL)
	{
		item = list->first;
		list->first = item->next;
		
		if(item->filename != NULL)
		{
			fs_path_free(item->filename);
		}
		
		free(item);
	}
	list->last = NULL;
}

/* init list for first usage */
static void cab_list_init(cab_list_t *list)
{
	list->first = NULL;
	list->last  = NULL;
}

/* init file scan list */
static void scanned_files_list_init(scanned_files_list_t *list)
{
	list->first = NULL;
	list->last  = NULL;
}

static void scanned_files_list_add(scanned_files_list_t *list, const char *fn)
{
	size_t len = strlen(fn);
	struct scanned_files_item *item = malloc(sizeof(struct scanned_files_item) + len + 1);
	if(item != NULL)
	{
		item->filename = (char *)(item+1);
		strcpy(item->filename, fn);
		item->next = NULL;
		
		if(list->last == NULL)
		{
			list->first = item;
			list->last  = item;
		}
		else
		{
			list->last->next = item;
			list->last = item;
		}
	}
}

static int scanned_files_lookup(scanned_files_list_t *list, const char *fn)
{
	struct scanned_files_item *item = list->first;
	
	while(item != NULL)
	{
		if(istrcmp(item->filename, fn) == 0)
		{
			return 1;
		}
		
		item = item->next;
	}
	
	return 0;
}

static void scanned_files_clean(scanned_files_list_t *list)
{
	struct scanned_files_item *item = list->first;
	
	while(item != NULL)
	{
		struct scanned_files_item *del_item = item;
		item = item->next;
		
		free(del_item);
	}
	
	list->first = NULL;
	list->last  = NULL;
}

/**
 * Read string form CAB header
 *
 * @param fp: ponter to open file
 * @param buf: destination to write string bytes. String will be terminated with '\0'
 *             Destination should be at last CAB_MAX_STR bytes len.
 *             Could be NULL, if value doesn't matter and you need to seek to next header.
 *
 **/
static void cab_read_str(FILE *fp, char *buf)
{
	int c = 0;
	int i = 0;
	for(i = 0; i < CAB_MAX_STR; i++)
	{
		c = fgetc(fp);
		if(c == EOF || c == '\0')
		{
			if(buf != NULL)
			{
				*buf = '\0';
			}
			break;
		}
		if(buf != NULL)
		{
			*buf = c;
			buf++;
		}
	}
	
	if(buf != NULL)
	{
		*buf = '\0';
	}
}

/**
 * Open CAB and try to find prev and next part if archive is multivolume
 *
 * @param srccab: patch to archive
 * @param prevcab: pointer to pointer to file name of previous part of archive if the part is exists.
 *                 If not destination will be set to NULL. Paramater could be NULL if on previous part not matter.
 *                 Unused result should be free with 'fs_path_free'
 * @param nextcab: same behavior as prevcab but for next part of archive
 *
 **/
static void cab_analyze(const char *srccab, char **prevcab, char **nextcab)
{
	char prevfile[CAB_MAX_STR];
	char nextfile[CAB_MAX_STR];
	
	FILE *fr = fopen(srccab, "rb");
	char *ptr;
	if(fr != NULL)
	{
		mscab_t header;
		fread(&header, sizeof(mscab_t), 1, fr);
		if(memcmp(header.signature, "MSCF", 4) == 0)
		{
			if((header.flags & cfhdrRESERVE_PRESENT) != 0)
			{
				uint16_t cbCFHeader;
				fread(&cbCFHeader, sizeof(uint16_t), 1, fr);
				cab_read_str(fr, NULL); /* read (and ignore) cbCFFolder */
				cab_read_str(fr, NULL); /* read (and ignore) cbCFData */
				fseek(fr, cbCFHeader, SEEK_CUR); /* skip abReserve */
			}
			
			if((header.flags & cfhdrPREV_CABINET) != 0)
			{
				cab_read_str(fr, prevfile); /* read szCabinetPrev */
				cab_read_str(fr, NULL); /* read (and ignore) szDiskPrev */
				
				if(prevcab != NULL)
				{
					*prevcab = NULL;
					mscab_t header_test;
					FILE *test;
					int valid = 0;
					
					ptr = fs_path_get2(srccab, prevfile, NULL);
					if(ptr != NULL)
					{
						test = fopen(ptr, "rb");
						if(test)
						{
							fread(&header_test, sizeof(mscab_t), 1, test);
							
							/* check if is CAB and setID match with source */
							if(memcmp(header_test.signature, "MSCF", 4) == 0 &&
								header_test.setID == header.setID)
							{
								/* check sequence */
								if(header.iCabinet == header_test.iCabinet+1)
								{
									*prevcab = ptr;
									valid = 1;
								}
							}
							
							fclose(test);
						}
						
						if(!valid)
						{
							fs_path_free(ptr);
						}
					}
				}
			} // cfhdrPREV_CABINET
		
			if((header.flags & cfhdrNEXT_CABINET) != 0)
			{
				cab_read_str(fr, nextfile); /* read szCabinetNext */
				cab_read_str(fr, NULL); /* read (and ignore) szDiskNext */
				
				if(nextcab != NULL)
				{
					*nextcab = NULL; /* clear */
					mscab_t header_test;
					FILE *test;
					int valid = 0;
					
					ptr = fs_path_get2(srccab, nextfile, NULL);
					if(ptr != NULL)
					{
						test = fopen(ptr, "rb");
						if(test)
						{
							fread(&header_test, sizeof(mscab_t), 1, test);
							
							/* check if is CAB and setID match with source */
							if(memcmp(header_test.signature, "MSCF", 4) == 0 &&
								header_test.setID == header.setID)
							{
								/* check sequence */
								if(header.iCabinet+1 == header_test.iCabinet)
								{
									*nextcab = ptr;
									valid = 1;
								}
							}
							
							fclose(test);
						}
						
						if(!valid)
						{
							fs_path_free(ptr);
						}
					}
				}
			} // cfhdrNEXT_CABINET
		} // is MS CAB
		
		fclose(fr);
	} // fr != NULL
}

/**
 * Unpack file form CAB archive, multivolume archives are supported too
 * 
 * @param srccab: path to archive
 * @param infilename: filename in archive to extract
 * @param out: path to extact (with filename)
 *
 * @return: number of extracet files (should be 0 or 1)
 **/
int cab_unpack(const char *srccab, const char *infilename, const char *out, scanned_files_list_t *sclist)
{
	int cnt = 0;
	char *search;
  struct mscab_decompressor *cabd;
  struct mscabd_cabinet *cab;
  struct mscabd_cabinet *cabptr;
  struct mscabd_file *file;
  cab_list_item_t *item;
  cab_list_item_t *item_first_cab = NULL;
  cab_list_item_t *item_last_cab = NULL;
  
  // printf("CAB: %s\n", srccab);
  
  if((cabd = mspack_create_cab_decompressor(NULL)) == NULL)
	{
		return 0;
	}

	cab_list_t prevlist;
	cab_list_t nextlist;
	
	cab_list_init(&prevlist);
	cab_list_init(&nextlist);
	
	/* add file to list */
	if(sclist != NULL) scanned_files_list_add(sclist, fs_basename(srccab));
	
	/* search for archive previous parts */
	search = (char*)srccab;
	do
	{
		char *prev = NULL;
		
		cab_analyze(search, &prev, NULL);
		if(prev != NULL)
		{
			item = cab_list_add(&prevlist);
			if(item != NULL)
			{
				item->filename = prev;
				item->cab = cabd->open(cabd, item->filename);
				
				if(sclist != NULL) scanned_files_list_add(sclist, item->filename);
				// printf("PREV: %s\n", item->filename);
			}
		}
		search = prev;
	} while(search != NULL);
	
	/* search for archive next parts */
	search = (char*)srccab;
	do
	{
		char *next = NULL;
		
		cab_analyze(search, NULL, &next);
		if(next != NULL)
		{
			item = cab_list_add(&nextlist);
			if(item != NULL)
			{
				item->filename = next;
				item->cab = cabd->open(cabd, item->filename);
				
				if(sclist != NULL) scanned_files_list_add(sclist, item->filename);
				// printf("NEXT: %s\n", item->filename);
			}
		}
		search = next;
	} while(search != NULL);
	
	cab = cabd->open(cabd, srccab);
	if(cab)
	{
		/* links archives - prev */
		cabptr = cab;
		for(item = prevlist.first; item != NULL; item = item->next)
		{
			if(item->cab)
			{
				cabd->prepend(cabd, cabptr, item->cab);
				cabptr = item->cab;
			}
			
			if(item->next == NULL)
			{
				item_first_cab = item;
			}
		}
		
		/* links archives - next */
		cabptr = cab;
		for(item = nextlist.first; item != NULL; item = item->next)
		{
			if(item->cab)
			{
				cabd->append(cabd, cabptr, item->cab);
				cabptr = item->cab;
			}
			
			if(item->next == NULL)
			{
				item_last_cab = item;
			}
		}
		
		cabptr = cab;
		if(item_first_cab != NULL && item_first_cab->cab != NULL)
		{
			/* try start with first archive */
			cabptr = item_first_cab->cab;
		}
		
		for (file = cabptr->files; file; file = file->next)
		{
			if(istrcmp(file->filename, infilename) == 0)
			{
				/* nice message for user */
				char *cab_first = NULL;
				char *cab_last  = NULL;
				
				if(item_first_cab == NULL && item_last_cab == NULL)
				{
					cab_first = fs_basename(srccab);
					if(cab_first)
					{
						printf("Extracting %s from %s ... ", file->filename, cab_first);
						fs_path_free(cab_first);
					}
				}
				else
				{
					if(item_first_cab != NULL)
					{
						cab_first = fs_basename(item_first_cab->filename);
					}
					else
					{
						cab_first = fs_basename(srccab);
					}
					
					if(item_last_cab != NULL)
					{
						cab_last = fs_basename(item_last_cab->filename);
					}
					else
					{
						cab_last = fs_basename(srccab);
					}
					
					if(cab_first != NULL && cab_last != NULL)
					{
						printf("Extracting %s from multivolume (%s - %s) ... ", file->filename, cab_first, cab_last);
					}
					
					if(cab_first != NULL) fs_path_free(cab_first);
					if(cab_last  != NULL) fs_path_free(cab_last);
				}
				
				int t = cabd->extract(cabd, file, out);
				if(t == MSPACK_ERR_OK)
				{
					printf("SUCCESS\n");
					cnt++;
					break;
				}
				else if(t == MSPACK_ERR_OPEN) /* R/O destination */
				{
					printf("FAILURE (cannot create file!)\n");
				}
				else if(t == MSPACK_ERR_NONEXT) /* special for multivolume error (no next file) */
				{
					printf("FAILURE (multivolume archive - next archive part is missing)\n");
				}
				else if(t == MSPACK_ERR_NOPREV) /* special for multivolume error (no previous file) */
				{
					printf("FAILURE (multivolume archive - previous archive part is missing)\n");
				}
				else
				{
					printf("FAILURE (%d)\n", t);
				}
			} // infilename
    } // for files
    
    cabd->close(cabd, cab);    
  } // cab
  
  cab_list_clear(&prevlist);
  cab_list_clear(&nextlist);
  
  mspack_destroy_cab_decompressor(cabd);
  
  return cnt;
}

/**
 * Search in folder for CAB files and in them search for specified file. If file names is found function will stop
 * search for next archives.
 *
 * @param dirname: dirname with cab files
 * @param infilename: filename in archive to extract
 * @param out: path to extact (with filename)
 *
 * @return: number of extracet files (should be 0 or 1)
 *
 **/
int cab_search_unpack(const char *dirname, const char *infilename, const char *out)
{
	fs_dir_t *dir = fs_dir_open(dirname);
	int cnt = 0;
	const char *fn;
	
	scanned_files_list_t sclist;
	scanned_files_list_init(&sclist);
	
  if(dir)
  {
  	while((fn = fs_dir_read(dir, FS_FILTER_FILE)) != NULL)
  	{
  		if(fs_ext_match(fn, "cab") != 0)
  		{
  			char *cabfile = fs_path_get(dirname, fn, NULL);
  			
  			if(!scanned_files_lookup(&sclist, fn))
  			{
  				if(cab_unpack(cabfile, infilename, out, &sclist) > 0)
  				{
	  				//printf("extracted: %s\n", infilename);
  					cnt++;
  				}
  				fs_path_free(cabfile);
  			}
  		}
  		
  		if(cnt > 0)
  		{
  			break;
  		}
  	}
  	fs_dir_close(&dir);
  }
  
  scanned_files_clean(&sclist);
  
  return cnt;
}

/**
 * Extract driver form VMM32.VXD or diffent W3/W4 file.
 * 
 * @param src: path to W3/W4 file
 * @param infilename: driver in archive to extract (without *.VXD extension)
 * @param out: path to extact (with filename)
 * @param tmpname: path to temporary file in writeable location
 *
 * @return: PATCH_OK on success otherwise one of PATCH_E_* error code
 ***/
int wx_unpack(const char *src, const char *infilename, const char *out, const char *tmpname)
{
	dos_header_t dos, dos2;
	pe_header_t  pe, pe2;
	pe_w3_t     *w3;
	FILE        *fp, *fp2;
	int          t;
	int status = PATCH_OK;
	int exist_temp = 0;
	
	if(fs_file_exists(tmpname))
	{
		fp = FOPEN_LOG(tmpname, "rb");
		exist_temp = 1;
	}
	else
	{
		fp = FOPEN_LOG(src, "rb");
	}
	
	if(fp)
	{
		t = pe_read(&dos, &pe, fp);
		if(t == PE_W3)
		{
			w3 = pe_w3_read(&dos, &pe, fp);
			if(w3 != NULL)
			{
				char *path_without_ext = fs_path_get(NULL, infilename, "");
				int status_extract = 0;
				
				if(path_without_ext != NULL)
				{
					status_extract = pe_w3_extract(w3, path_without_ext, out);
					fs_path_free(path_without_ext);
				}
				else
				{
					status_extract = pe_w3_extract(w3, infilename, out);
				}
				
				if(status_extract == PE_OK)
				{
					status = PATCH_OK;
				}
				
				pe_w3_free(w3);
			}
			else
			{
				status = PATCH_E_READ;
			}
			
			fclose(fp);
		}
		else if(t == PE_W4)
		{
			fclose(fp);
			
			if(exist_temp)
			{
				status = PATCH_E_CONVERT;
			}
			else
			{	
				if((status = wx_to_w3(src, tmpname)) == PATCH_OK)
				{
					fp2 = FOPEN_LOG(tmpname, "rb");
					if(fp2 != NULL)
					{
						if(pe_read(&dos2, &pe2, fp2) == PE_W3)
						{
							w3 = pe_w3_read(&dos2, &pe2, fp2);
							if(w3 != NULL)
							{
								char *path_without_ext = fs_path_get(NULL, infilename, "");
								int status_extract = 0;
								
								if(path_without_ext != NULL)
								{
									status_extract = pe_w3_extract(w3, path_without_ext, out);
									fs_path_free(path_without_ext);
								}
								else
								{
									status_extract = pe_w3_extract(w3, infilename, out);
								}
								
								if(status_extract == PE_OK)
								{
									status = PATCH_OK;
								}
								
								pe_w3_free(w3);
							}
							else
							{
								//printf("pe_w3_read FAIL\n");
								status = PATCH_E_READ;
							}
						}
						
						fclose(fp2);
						fs_unlink(tmpname);
					}
					else
					{
						status = PATCH_E_READ;
					}
				} // wx_to_w3
			}
		}
		else
		{
			fclose(fp);
			status = PATCH_E_WRONG_TYPE;
		}
	} // fopen
	else
	{
		status = PATCH_E_READ;
	}
	
	return status;
}

/**
 * Convert W4/W3 file to W3 file. If file is already in W3 format only copy it.
 *
 * @param in: input filename
 * @param out: output filename
 *
 * @return: PATCH_OK on success
 **/
int wx_to_w3(const char *in, const char *out)
{
	FILE *fp;
	dos_header_t dos;
	pe_header_t  pe;
	pe_w4_t     *w4;
	int t;
	int status = PATCH_E_CONVERT;
	
	fp = FOPEN_LOG(in, "rb");
	if(fp)
	{
		t = pe_read(&dos, &pe, fp);
		if(t == PE_W4)
		{
			w4 = pe_w4_read(&dos, &pe, fp);
			if(w4 != NULL)
			{
				if(pe_w4_to_w3(w4, out) == PE_OK)
				{
					status = PATCH_OK;;
				}
				
				pe_w4_free(w4);
			}
		}
		else if(t == PE_W3)
		{
			FILE *fw = fopen(out, "wb");
			if(fw)
			{
				fseek(fp, 0, SEEK_SET);
				
				fs_file_copy(fp, fw, 0);
				status = PATCH_OK;
				
				fclose(fw);
			}
		}
		fclose(fp);
	}
	
	return status;
}

/**
 * Convert W4/W3 file to W4 file. If file is already in W4 format only copy it.
 *
 * @param in: input filename
 * @param out: output filename
 *
 * @return: PATCH_OK on success
 **/
int wx_to_w4(const char *in, const char *out)
{
	FILE *fp;
	dos_header_t dos;
	pe_header_t  pe;
	pe_w3_t     *w3;
	int t;
	int status = PATCH_E_CONVERT;
	
	fp = FOPEN_LOG(in, "rb");
	if(fp)
	{
		t = pe_read(&dos, &pe, fp);
		if(t == PE_W3)
		{
			w3 = pe_w3_read(&dos, &pe, fp);
			if(w3 != NULL)
			{
				if(pe_w3_to_w4(w3, out) == PE_OK)
				{
					status = PATCH_OK;;
				}
				
				pe_w3_free(w3);
			}
		}
		else if(t == PE_W4)
		{
			FILE *fw = fopen(out, "wb");
			if(fw)
			{
				fseek(fp, 0, SEEK_SET);
				
				fs_file_copy(fp, fw, 0);
				status = PATCH_OK;
				
				fclose(fw);
			}
		}
		fclose(fp);
	}
	
	return status;
}

/* context listing - CABS */
struct cab_filelist
{
  struct mscab_decompressor *cabd;
  struct mscabd_cabinet *cab;
  struct mscabd_file *file;
};

/**
 * Open cab for file listing
 *
 **/
cab_filelist_t *cab_filelist_open(const char *file)
{
	cab_filelist_t *list = malloc(sizeof(cab_filelist_t));
	list->cabd = NULL;
	list->cab  = NULL;
	list->file = NULL;
	  
  if((list->cabd = mspack_create_cab_decompressor(NULL)) == NULL)
	{
		free(list);
		return NULL;
	}
	
	list->cab = list->cabd->open(list->cabd, file);
	if(list->cab == NULL)
	{
		mspack_destroy_cab_decompressor(list->cabd);
		free(list);
		return NULL;
	}
	
	list->file = list->cab->files;
  
  return list;
}

/**
 * Return file name and move pointer to another file
 *
 **/
const char *cab_filelist_get(cab_filelist_t *list)
{
	if(list->file != NULL)
	{
		const char *s = list->file->filename;
		list->file = list->file->next;
		
		return s;
	}
	return NULL;
}

/**
 * Close CAB
 *
 **/
void cab_filelist_close(cab_filelist_t *list)
{
	if(list->cab)
	{
  	list->cabd->close(list->cabd, list->cab);
  }
  
  if(list->cabd)
  {
  	mspack_destroy_cab_decompressor(list->cabd);
  }
  
  free(list);
}

/* context listting - VXD */
struct vxd_filelist
{
	pe_w3_t *w3;
	size_t act;
	const char *tmp;
};

/**
 * Open VXD (W3/W4) for file listting, if W4 convert to W3 using tmp
 *
 **/
vxd_filelist_t *vxd_filelist_open(const char *file, const char *tmp)
{
	dos_header_t dos;
	pe_header_t pe;
	int type;
	FILE *fr;
	
	vxd_filelist_t *list = malloc(sizeof(vxd_filelist_t));
	if(list == NULL)
	{
		return NULL;
	}
	
	list->w3 = NULL;
	list->act = 0;
	list->tmp = NULL;
	
	fr = fopen(file, "rb");
	if(!fr)
	{
		free(list);
		return NULL;
	}
	
	type = pe_read(&dos, &pe, fr);
	if(type == PE_W3)
	{
		list->w3 = pe_w3_read(&dos, &pe, fr);
		fclose(fr);
	}
	else if(type == PE_W4)
	{
		fclose(fr);
		if(wx_to_w3(file, tmp) == PATCH_OK)
		{
			fr = fopen(tmp, "rb");
			if(fr)
			{
				type = pe_read(&dos, &pe, fr);
				if(type == PE_W3)
				{
					list->w3 = pe_w3_read(&dos, &pe, fr);
					list->tmp = tmp;
				}
				fclose(fr);
			}
		}
	}
	else
	{
		fclose(fr);
	}
	
	if(list->w3 == NULL)
	{
		free(list);
		return NULL;
	}
	
	return list;
}

/**
 * Return file name and move pointer to another file
 *
 **/
const char *vxd_filelist_get(vxd_filelist_t *list)
{
	static char cname[PE_W3_FILE_NAME_SIZE+1];
	
	if(list->act < list->w3->files_cnt)
	{
		uint8_t *ptr = list->w3->files[list->act].name;
		memcpy(cname, ptr, PE_W3_FILE_NAME_SIZE);
		cname[PE_W3_FILE_NAME_SIZE] = '\0';
		
		list->act++;
		return cname;
	}
	
	return NULL;
}

/**
 * Close the file and delete temp if was used
 *
 **/
void vxd_filelist_close(vxd_filelist_t *list)
{
	if(list->w3 != NULL)
	{
		pe_w3_free(list->w3);
	}
	
	if(list->tmp)
	{
		fs_unlink(list->tmp);
	}
	
	free(list);
}

