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
#include <bpatcher.h>
#include "vmm_patch.h"
#include "vmm_patch_v2.h"
#include "vmm_patch_me1.h"
#include "vmm_patch_me2.h"

#include "vmm_patch_old.h"
#include "vmm_patch_old_v2.h"

#include "vmm_patch_simple.h"
#include "vmm_patch_simple_v2.h"

#include "cpuspeed_patch_v1.h"
#include "cpuspeed_patch_v2.h"
#include "cpuspeed_patch_v3.h"
#include "cpuspeed_patch_v4.h"

#include "cpuspeed_patch_v5.h"
#include "cpuspeed_patch_v6.h"
#include "cpuspeed_patch_v7.h"
#include "cpuspeed_patch_v8.h"

#include "cpuspeed_ndis_patch_v1.h"
#include "cpuspeed_ndis_patch_v2.h"
#include "cpuspeed_ndis_patch_v3.h"
#include "cpuspeed_ndis_patch_v4.h"

typedef struct _ppatch_t
{
	int id;
	const char    *name;
	const uint8_t *patch_data;
	size_t         patch_size;
	const uint8_t *orig_data;
	size_t         orig_size;
	const uint8_t *check_data;
	size_t         check_size;
	const uint8_t *modif_data;
	size_t         modif_size;
} ppatch_t;

#define PPATCH_FILL(_name) _name, sizeof(_name), \
	_name##_orig,         sizeof(_name##_orig), \
	_name##_orig_check,   sizeof(_name##_orig_check), \
	_name##_modif,        sizeof(_name##_modif)


ppatch_t ppathes[] = {
	{PATCH_VMM98,             "W98 TLB patch #1 (for SE, updated)",                        PPATCH_FILL(vmm_patch)},
	{PATCH_VMM98_V2,          "W98 TLB patch #2 (for SE, Q242161, Q288430)",               PPATCH_FILL(vmm_patch_v2)},
	{PATCH_VMMME,             "WinMe  TLB patch",                                          PPATCH_FILL(vmm_patch_me1)},
	/* ^ only first part, for this case is there special function */
	{PATCH_CPU_SPEED_V1,      "CPU Speed #1 (1 000 000 LOOPs)",                            PPATCH_FILL(cpuspeed_patch_v1)},
	{PATCH_CPU_SPEED_V2,      "CPU Speed #2 (2 000 000 LOOPs)",                            PPATCH_FILL(cpuspeed_patch_v2)},
	{PATCH_CPU_SPEED_V3,      "CPU Speed #3 (10 000 000 LOOPs, FIX95)",                    PPATCH_FILL(cpuspeed_patch_v3)},
	{PATCH_CPU_SPEED_V4,      "CPU Speed #4 (10 000 000 LOOPs, rloew's patch)",            PPATCH_FILL(cpuspeed_patch_v4)},
	{PATCH_CPU_SPEED_V5,      "CPU Speed #5 (1 000 000 LOOPs)",                            PPATCH_FILL(cpuspeed_patch_v5)},
	{PATCH_CPU_SPEED_V6,      "CPU Speed #6 (2 000 000 LOOPs)",                            PPATCH_FILL(cpuspeed_patch_v6)},
	{PATCH_CPU_SPEED_V7,      "CPU Speed #7 (10 000 000 LOOPs, FIX95)",                    PPATCH_FILL(cpuspeed_patch_v7)},
	{PATCH_CPU_SPEED_V8,      "CPU Speed #8 (10 000 000 LOOPs, rloew's patch)",            PPATCH_FILL(cpuspeed_patch_v8)},
	{PATCH_CPU_SPEED_NDIS_V1, "CPU Speed NDIS.VXD #1 (1 048 576 LOOPs, W95+W98FE)",        PPATCH_FILL(cpuspeed_ndis_patch_v1)},
	{PATCH_CPU_SPEED_NDIS_V2, "CPU Speed NDIS.VXD #2 (1 048 576 LOOPs, W98SE)",            PPATCH_FILL(cpuspeed_ndis_patch_v2)},
	{PATCH_CPU_SPEED_NDIS_V3, "CPU Speed NDIS.VXD #3 (10 485 760, LOOPs, rloew's patch)",  PPATCH_FILL(cpuspeed_ndis_patch_v3)},	
	{PATCH_CPU_SPEED_NDIS_V4, "CPU Speed NDIS.386 #4 (10 485 760, LOOPs, WFW3.11)",        PPATCH_FILL(cpuspeed_ndis_patch_v4)},
	{PATCH_VMM98_OLD,         "W98 TLB patch #1 UPGRADE",                                  PPATCH_FILL(vmm_patch_old)},
	{PATCH_VMM98_OLD_V2,      "W98 TLB patch #2 UPGRADE",                                  PPATCH_FILL(vmm_patch_old_v2)},
	{PATCH_VMM98_SIMPLE,      "W98 TLB patch #1 (simple version)",                         PPATCH_FILL(vmm_patch_simple)},
	{PATCH_VMM98_SIMPLE_V2,   "W98 TLB patch #2 (simple version, Q242161, Q288430)",       PPATCH_FILL(vmm_patch_simple_v2)},
	{0, NULL, NULL, 0, NULL, 0, NULL, 0, NULL, 0}
};

/* special case for ME patch */
static int patch_select_me(FILE *fp, const char *dstfile, int *file_copied, uint32_t *applied, uint32_t *exists);

/**
 * Apply selected set of patches
 *
 * @param fp: pointer to original file, source must support binary read and seeking
 * @param dstfile: file name of destination, destination will be overwriten
 * @param to_apply: constants PATCH_* from above, use binary OR for specifing multiple values
 * @param out_applied: if is not NULL, it will be filled with PATCH_* constants which indicated successfull patching
 * @param out_exists: if is not NULL, it will be filled with PATCH_* constants which indicated already patched original file
 *
 * @return: PATCH_OK on success (at last one patch will be applied) or one of PATCH_E_* codes
 *
 **/
int patch_selected(FILE *fp, const char *dstfile, uint32_t to_apply, uint32_t *out_applied, uint32_t *out_exists)
{
	int status = PATCH_OK;
	ppatch_t *patch;
	uint32_t applied = 0;
	uint32_t exists  = 0;
	int file_copied  = 0;
	
	for(patch = ppathes; patch->patch_data != NULL; patch++)
	{
		//printf("patch: %X\n", patch->id);
		
		if((to_apply & patch->id) != 0 && patch->id == PATCH_VMMME)
		{
			status = patch_select_me(fp, dstfile, &file_copied, &applied, &exists);
		}
		else if((to_apply & patch->id) != 0)
		{
			ssize_t pos;
			bitstream_t bs_check, bs_patch;
			bs_mem(&bs_check, (uint8_t*)patch->check_data, patch->check_size);
			
			/* if already applied updated patch ignore simple version */
			if((applied & (PATCH_VMM98 | PATCH_VMM98_V2)) != 0)
			{
				if(patch->id == PATCH_VMM98_SIMPLE || patch->id == PATCH_VMM98_SIMPLE_V2)
				{
					continue;
				}
			}
			
			fseek(fp, 0, SEEK_SET);
			
			pos = search_sieve_file(fp, patch->orig_data, patch->orig_size, &bs_check);
			if(pos >= 0)
			{
				applied |= patch->id;
				
				if((to_apply & PATCH_DRY) == 0)
				{
					FILE *fw;
					if(file_copied == 0)
					{					
						fw = FOPEN_LOG(dstfile, "wb");
						fseek(fp, 0, SEEK_SET);
						fs_file_copy(fp, fw, 0);
						file_copied = 1;
					}
					else
					{
						fw = FOPEN_LOG(dstfile, "r+b");
					}
					
					if(fw != NULL)
					{
						void *buf = malloc(patch->patch_size);
						if(buf != NULL)
						{
							do
							{
								fseek(fp, pos, SEEK_SET);
								if(fread(buf, 1, patch->patch_size, fp) == patch->patch_size)
								{
									bs_mem(&bs_patch, (uint8_t*)patch->modif_data, patch->modif_size);
									patch_sieve(buf, patch->patch_data, patch->patch_size, &bs_patch);
						
									fseek(fw, pos, SEEK_SET);
									fwrite(buf, 1, patch->patch_size, fw);				
								}
								else
								{
									status = PATCH_E_READ;
								}
								
								/* some files has to be patched more times */
								bs_reset(&bs_check);
								pos = search_sieve_file(fp, patch->orig_data, patch->orig_size, &bs_check);
								//printf("another pos: %zd\n", pos);
								
							} while(pos >= 0);
							
							free(buf);
						}
						else
						{
							status = PATCH_E_MEM;
						}
						fclose(fw);
					}
					else
					{
						status = PATCH_E_WRITE;
					}
				} // ! dry run
			} // 
			else
			{
				/* original data not found, but we'll is pach isn't applied  */
				fseek(fp, 0, SEEK_SET);
				bs_reset(&bs_check);
				pos = search_sieve_file(fp, patch->patch_data, patch->patch_size, &bs_check);
				if(pos >= 0)
				{
					exists |= patch->id;
				}
			}
		} // if apply
	} // for patch in patches
	
	if(out_applied != NULL)
	{
		*out_applied = applied;
	}
	
	if(out_exists != NULL)
	{
		*out_exists = exists;
	}
	
	if(status == PATCH_OK)
	{
		if(applied == 0)
		{
			if(exists == 0)
			{
				return PATCH_E_CHECK;
			}
			else
			{
				return PATCH_E_PATCHED;
			}
		}
	}
	
	return status;
}

#define ME_BLOCK_DISTANCE 16148

#define SIZEOF_MAX(_a, _b) (sizeof(_a) > sizeof(_b) ? sizeof(_a) : sizeof(_b)) 

static int patch_select_me(FILE *fp, const char *dstfile, int *file_copied, uint32_t *applied, uint32_t *exists)
{
	int status = PATCH_OK;
	ssize_t pos_a, pos_b;
	bitstream_t bs_check, bs_patch;
	
	fseek(fp, 0, SEEK_SET);
	
	bs_mem(&bs_check, (uint8_t*)vmm_patch_me1_orig_check, sizeof(vmm_patch_me1_orig_check));
	pos_a = search_sieve_file(fp, vmm_patch_me1_orig, sizeof(vmm_patch_me1_orig), &bs_check);
	
	fseek(fp, 0, SEEK_SET);
	
	bs_mem(&bs_check, (uint8_t*)vmm_patch_me2_orig_check, sizeof(vmm_patch_me2_orig_check));
	pos_b = search_sieve_file(fp, vmm_patch_me2_orig, sizeof(vmm_patch_me2_orig), &bs_check);
		
	if(pos_a >= 0 && (pos_b - pos_a) == ME_BLOCK_DISTANCE)
	{
		FILE *fw;
		if(*file_copied == 0)
		{
			fw = FOPEN_LOG(dstfile, "wb");
			fseek(fp, 0, SEEK_SET);
			fs_file_copy(fp, fw, 0);
			*file_copied = 1;
		}
		else
		{
			fw = FOPEN_LOG(dstfile, "r+b");
		}

		if(fw != NULL)
		{
			void *buf = malloc(SIZEOF_MAX(vmm_patch_me1, vmm_patch_me2));
			if(buf != NULL)
			{
				fseek(fp, pos_a, SEEK_SET);
				if(fread(buf, 1, sizeof(vmm_patch_me1), fp) == sizeof(vmm_patch_me1))
				{
					bs_mem(&bs_patch, (uint8_t*)vmm_patch_me1_modif, sizeof(vmm_patch_me1_modif));
					patch_sieve(buf, vmm_patch_me1, sizeof(vmm_patch_me1), &bs_patch);
					
					fseek(fw, pos_a, SEEK_SET);
					fwrite(buf, 1, sizeof(vmm_patch_me1), fw);							
				}
				else
				{
					status = PATCH_E_READ;
				}
						
				fseek(fp, pos_b, SEEK_SET);
				if(fread(buf, 1, sizeof(vmm_patch_me2), fp) == sizeof(vmm_patch_me2))
				{
					bs_mem(&bs_patch, (uint8_t*)vmm_patch_me2_modif, sizeof(vmm_patch_me2_modif));
					patch_sieve(buf, vmm_patch_me2, sizeof(vmm_patch_me2), &bs_patch);
					
					fseek(fw, pos_b, SEEK_SET);
					fwrite(buf, 1, sizeof(vmm_patch_me2), fw);							
				}
				else
				{
					status = PATCH_E_READ;
				}
				
				free(buf);
			}
			else
			{
				status = PATCH_E_MEM;
			}
			fclose(fw);
		}
		else
		{
			status = PATCH_E_WRITE;
		}
		
		if(status == PATCH_OK)
		{
			*applied |= PATCH_VMMME;
		}
	}
	else
	{
		fseek(fp, 0, SEEK_SET);
		bs_mem(&bs_check, (uint8_t*)vmm_patch_me1_orig_check, sizeof(vmm_patch_me1_orig_check));
		pos_a = search_sieve_file(fp, vmm_patch_me1, sizeof(vmm_patch_me1), &bs_check);
		if(pos_a >= 0)
		{
			*exists |= PATCH_VMMME;
		}
	}

	return status;
}

/**
 * Apply patch and if check failure check if applied.
 *
 **/
int patch_apply(const char *srcfile, const char *dstfile, int flags, int *applied)
{
	int status = PATCH_E_NOTFOUND;
	FILE        *fp;
	
	fp = FOPEN_LOG(srcfile, "rb");
	if(fp)
	{
		status = patch_selected(fp, dstfile, flags, NULL, NULL);
		
		fclose(fp);
	}
	else
	{
		status = PATCH_E_READ;
	}
	
	return status;
}

/**
 * Apply patch on W3/W4 file and compress/decompress if it is W4 file
 *
 **/
int patch_apply_wx(const char *srcfile, const char *dstfile, const char *tmpname, int flags)
{
	int status = PATCH_OK;
	int w4_decompres = 0;
	int is_w3        = 0;
	dos_header_t dos;
	pe_header_t  pe;
	pe_w4_t     *w4;
	pe_w3_t     *w3;
	FILE        *fp;
	int          t;
	
	fp = FOPEN_LOG(srcfile, "rb");
	if(fp)
	{
		t = pe_read(&dos, &pe, fp);
		if(t == PE_W4)
		{
			w4 = pe_w4_read(&dos, &pe, fp);
			if(w4 != NULL)
			{
				if(pe_w4_to_w3(w4, tmpname) == PE_OK)
				{
					w4_decompres = 1;
				}
				else
				{
					status = PATCH_E_CONVERT;
				}
				pe_w4_free(w4);
			}
			else
			{
				status = PATCH_E_READ;
			}
		}
		else if(t == PE_W3)
		{
			is_w3 = 1;
		}
		fclose(fp);
	}
	
	if(status == PATCH_OK)
	{
		if(w4_decompres)
		{
			status = patch_apply(tmpname, dstfile, flags, NULL);
			if((flags & PATCH_FORCE_W3) == 0)
			{
				fs_rename(dstfile, tmpname);
			}
			else
			{
				w4_decompres = 0;
			}
		}
		else
		{
			if(is_w3 && (flags & PATCH_FORCE_W4) != 0)
			{
				status = patch_apply(srcfile, tmpname, flags, NULL);
				w4_decompres = 1;
			}
			else
			{
				status = patch_apply(srcfile, dstfile, flags, NULL);
			}
		}
	}
	
	if(status == PATCH_OK && w4_decompres)
	{
		fp = FOPEN_LOG(tmpname, "rb");
		
		if(fp)
		{
			t = pe_read(&dos, &pe, fp);
			if(t == PE_W3)
			{
				w3 = pe_w3_read(&dos, &pe, fp);
				if(w3 != NULL)
				{
					printf("Compressing file, please wait...\n");
					if(pe_w3_to_w4(w3, dstfile) != PE_OK)
					{
						status = PATCH_E_CONVERT;
					}
					pe_w3_free(w3);
				}
				else
				{
					status = PATCH_E_READ;
				}
			}
			else
			{
				status = PATCH_E_READ;
			}
			fclose(fp);
		}
		
		if(status == PATCH_OK)
		{
			w4 = NULL;
			fp = FOPEN_LOG(dstfile, "rb");
			if(fp != NULL)
			{
				t = pe_read(&dos, &pe, fp);
				if(t == PE_W4)
				{
					w4 = pe_w4_read(&dos, &pe, fp);
					if(w4 != NULL)
					{
						if(pe_w4_check(w4) != PE_OK)
						{
							printf("Warning: W4 file is not loadable, leaving in W3 format!\n");
							
							pe_w4_free(w4);
							w4 = NULL;
							fclose(fp);
							fp = NULL;
							
							fs_unlink(dstfile);
							fs_rename(tmpname, dstfile);
						}
						else
						{
							/* file is valid, remove temp */
							fs_unlink(tmpname);
						}
					}
				}
				else
				{
					status = PATCH_E_READ;
				}
			}
			else
			{
				status = PATCH_E_READ;
			}
			
			if(w4 != NULL) pe_w4_free(w4);
			if(fp != NULL) fclose(fp);		
		}
	}
	
	return status;
}

/**
 * Backup file
 *
 **/
int patch_backup_file(const char *path, int nobackup)
{
	FILE *fr, *fw;
	int status = -1;
	char *backupname;
	
	if(nobackup)
	{
		return PATCH_OK;
	}
	
	backupname = fs_path_get2(path, NULL, "bak");
	
	if(backupname)
	{
		if(!fs_file_exists(backupname))
		{
			fr = FOPEN_LOG(path, "rb");
		
			if(fr != NULL)
			{
				fw = FOPEN_LOG(backupname, "wb");
				if(fw != NULL)
				{
					fs_file_copy(fr, fw, 0);
					status = PATCH_OK;
					fclose(fw);
				}
				else
				{
					status = PATCH_E_WRITE;
				}
				
				fclose(fr);
			}
			else
			{
				status = PATCH_E_READ;
			}
		}
		else
		{
			status = PATCH_OK;
		}
		
		fs_path_free(backupname);
	}
	else
	{
		status = PATCH_E_MEM;
	}
	
	return status;
}

void patch_print(uint32_t patches)
{
	ppatch_t *patch;
	int cnt = 0;
	
	for(patch = ppathes; patch->patch_data != NULL; patch++)
	{
		if((patch->id & patches) != 0)
		{
			if(cnt++ > 0)
			{
				printf(", ");
			}
			
			printf("%s", patch->name);			
		}
	}
}

