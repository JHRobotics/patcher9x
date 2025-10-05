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
#include <patch.h>
#include <pew.h>

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

#include "vcache_patch_v1.h"
#include "vcache_patch_v2.h"
#include "vcache_patch_v3.h"

#include "vmm98_patch_v1.h"
#include "vmm98_patch_v2.h"

#include "vmmme_patch_v1.h"
#include "vmmme_patch_v2.h"

#include "vmm95_patch_v1.h"

#include "w3_patch_v1.h"
#include "w3_patch_v2.h"

#include "crfix.h"

typedef struct _ppatch_t
{
	uint64_t id;
	const char     *name;
	const cpatch_t *cpatch;
	const spatch_t *spatch;
} ppatch_t;

#define SPATCH_BUF 512

ppatch_t ppathes[] = {
	{PATCH_VMM98,               "W98 TLB patch #1 (for SE, updated)",                        &vmm_patch_cp,              NULL},
	{PATCH_VMM98_V2,            "W98 TLB patch #2 (for SE, Q242161, Q288430)",               &vmm_patch_v2_cp,           NULL},
	{PATCH_VMMME,               "WinMe  TLB patch",                                          &vmm_patch_me1_cp,          NULL},
	/* ^ only first part, for this case is there special function */
	{PATCH_CPU_SPEED_V1,        "CPU Speed #1 (1 000 000 LOOPs)",                            &cpuspeed_patch_v1_cp,      NULL},
	{PATCH_CPU_SPEED_V2,        "CPU Speed #2 (2 000 000 LOOPs)",                            &cpuspeed_patch_v2_cp,      NULL},
	{PATCH_CPU_SPEED_V3,        "CPU Speed #3 (10 000 000 LOOPs, FIX95)",                    &cpuspeed_patch_v3_cp,      NULL},
	{PATCH_CPU_SPEED_V4,        "CPU Speed #4 (10 000 000 LOOPs, rloew's patch)",            &cpuspeed_patch_v4_cp,      NULL},
	{PATCH_CPU_SPEED_V5,        "CPU Speed #5 (1 000 000 LOOPs)",                            &cpuspeed_patch_v5_cp,      NULL},
	{PATCH_CPU_SPEED_V6,        "CPU Speed #6 (2 000 000 LOOPs)",                            &cpuspeed_patch_v6_cp,      NULL},
	{PATCH_CPU_SPEED_V7,        "CPU Speed #7 (10 000 000 LOOPs, FIX95)",                    &cpuspeed_patch_v7_cp,      NULL},
	{PATCH_CPU_SPEED_V8,        "CPU Speed #8 (10 000 000 LOOPs, rloew's patch)",            &cpuspeed_patch_v8_cp,      NULL},
	{PATCH_CPU_SPEED_NDIS_V1,   "CPU Speed NDIS.VXD #1 (1 048 576 LOOPs, W95+W98FE)",        &cpuspeed_ndis_patch_v1_cp, NULL},
	{PATCH_CPU_SPEED_NDIS_V2,   "CPU Speed NDIS.VXD #2 (1 048 576 LOOPs, W98SE)",            &cpuspeed_ndis_patch_v2_cp, NULL},
	{PATCH_CPU_SPEED_NDIS_V3,   "CPU Speed NDIS.VXD #3 (10 485 760, LOOPs, rloew's patch)",  &cpuspeed_ndis_patch_v3_cp, NULL},
	{PATCH_CPU_SPEED_NDIS_V4,   "CPU Speed NDIS.386 #4 (10 485 760, LOOPs, WFW3.11)",        &cpuspeed_ndis_patch_v4_cp, NULL},
	{PATCH_VMM98_OLD,           "W98 TLB patch #1 UPGRADE",                                  &vmm_patch_old_cp,          NULL},
	{PATCH_VMM98_OLD_V2,        "W98 TLB patch #2 UPGRADE",                                  &vmm_patch_old_v2_cp,       NULL},
	{PATCH_VMM98_SIMPLE,        "W98 TLB patch #1 (simple version)",                         &vmm_patch_simple_cp,       NULL},
	{PATCH_VMM98_SIMPLE_V2,     "W98 TLB patch #2 (simple version, Q242161, Q288430)",       &vmm_patch_simple_v2_cp,    NULL},
	{PATCH_MEM_VCACHE98,        "W98 memory limit - VCACHE.VXD (rloew's patch)",             NULL,                       &vcache_v1_sp},
	{PATCH_MEM_VCACHE95,        "W95 memory limit - VCACHE.VXD (rloew's patch)",             NULL,                       &vcache_v2_sp},
	{PATCH_MEM_VCACHEME,        "ME memory limit - VCACHE.VXD (rloew's patch)",              NULL,                       &vcache_v3_sp},
	{PATCH_MEM98SE_PATCHMEM,    "W98 memory limit - VMM.VXD (SE or FE+Q242161, rloew's patch)", NULL,                   &vmm98_v1_sp},
	{PATCH_MEM98FE_PATCHMEM,    "W98 memory limit - VMM.VXD (FE, rloew's patch)",            NULL,                       &vmm98_v2_sp},
	{PATCH_MEMME_PATCHMEM,      "ME memory limit - VMM.VXD (rloew's patch)",                 NULL,                       &vmmme_v1_sp},
	{PATCH_MEMME_PATCHMEM_V2,   "ME memory limit - VMM.VXD (Q296773, rloew's patch)",        NULL,                       &vmmme_v2_sp},
	{PATCH_MEM95_PATCHMEM,      "W95 memory limit - VMM.VXD (rloew's patch)",                NULL,                       &vmm95_v1_sp},
	{PATCH_MEM_W3_98,           "W98 memory limit - W3 loader patch (rloew's patch)",        NULL,                       &w3_v1_sp},
	{PATCH_MEM_W3_95,           "W95 memory limit - W3 loader patch (rloew's patch)",        NULL,                       &w3_v2_sp},
	{PATCH_WIN_COM,             "win.com - control registry cleanup",                        NULL,                       NULL},
	{0, NULL, NULL, NULL}
};

/* special case for ME patch */
static int patch_select_me(FILE *fp, const char *dstfile, int *file_copied, uint64_t *applied, uint64_t *exists);

/* binary patch case */
static int spatch_apply(FILE *fp, const char *dstfile, int *file_copied, uint32_t offset, uint32_t fs, uint64_t patch_id, const spatch_t *spatch, uint64_t *applied, uint64_t *exists, int dry);

/* special case for win.com patch */
static int patch_select_wincom(FILE *fp, const char *dstfile, int *file_copied, uint64_t *applied, uint64_t *exists, int dry_run);

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
int patch_selected(FILE *fp, const char *dstfile, uint64_t to_apply, uint64_t *out_applied, uint64_t *out_exists)
{
	int status = PATCH_OK;
	ppatch_t *patch;
	uint64_t applied = 0;
	uint64_t exists  = 0;
	int file_copied  = 0;
	
	for(patch = ppathes; patch->name != NULL; patch++)
	{
		if((to_apply & patch->id) != 0)
		{
			//printf("patch: %llX\n", patch->id);
			
			if(patch->id == PATCH_VMMME)
			{
				status = patch_select_me(fp, dstfile, &file_copied, &applied, &exists);
			}
			else if(patch->id == PATCH_WIN_COM)
			{
				status = patch_select_wincom(fp, dstfile, &file_copied, &applied, &exists, to_apply & PATCH_DRY);
			}
			else
			{
				if(patch->cpatch)
				{
					ssize_t pos;
					bitstream_t bs_check, bs_patch;
					bs_mem(&bs_check, (uint8_t*)patch->cpatch->check_data, patch->cpatch->check_size);
					
					/* if already applied updated patch ignore simple version */
					if((applied & (PATCH_VMM98 | PATCH_VMM98_V2)) != 0)
					{
						if(patch->id == PATCH_VMM98_SIMPLE || patch->id == PATCH_VMM98_SIMPLE_V2)
						{
							continue;
						}
					}
					
					fseek(fp, 0, SEEK_SET);
					
					pos = search_sieve_file(fp, patch->cpatch->orig_data, patch->cpatch->orig_size, &bs_check);
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
								void *buf = malloc(patch->cpatch->patch_size);
								if(buf != NULL)
								{
									do
									{
										fseek(fp, pos, SEEK_SET);
										if(fread(buf, 1, patch->cpatch->patch_size, fp) == patch->cpatch->patch_size)
										{
											bs_mem(&bs_patch, (uint8_t*)patch->cpatch->modif_data, patch->cpatch->modif_size);
											patch_sieve(buf, patch->cpatch->patch_data, patch->cpatch->patch_size, &bs_patch);
								
											fseek(fw, pos, SEEK_SET);
											fwrite(buf, 1, patch->cpatch->patch_size, fw);				
										}
										else
										{
											status = PATCH_E_READ;
										}
										
										/* some files has to be patched more times */
										bs_reset(&bs_check);
										pos = search_sieve_file(fp, patch->cpatch->orig_data, patch->cpatch->orig_size, &bs_check);
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
						/* original data not found, lets assume that patch isn't applied  */
						fseek(fp, 0, SEEK_SET);
						bs_reset(&bs_check);
						pos = search_sieve_file(fp, patch->cpatch->patch_data, patch->cpatch->patch_size, &bs_check);
						if(pos >= 0)
						{
							exists |= patch->id;
						}
					}
				} // cpatch
				else if(patch->spatch)
				{
					/* try apply spatch normaly from file begin */
					uint32_t fs = 0;
	
					if(patch->id == PATCH_MEM_W3_95 || patch->id == PATCH_MEM_W3_98)
					{
						/* special case to apply on W3 archive */
						status = spatch_apply(fp, dstfile, &file_copied, 0, 0, patch->id, patch->spatch, &applied, &exists, to_apply & PATCH_DRY);
					}
					else
					{
						/* try to apply on every object in file */
						dos_header_t dos;
						pe_header_t pe;
						int pe_test = 0;
						
						fseek(fp, 0, SEEK_END);
						fs = ftell(fp);
						fseek(fp, 0, SEEK_SET);
						pe_test = pe_read(&dos, &pe, fp, 1);
	
						if(pe_test != PE_W3) /* single file */
						{
							uint32_t off = 0;
							if(pe_test == PE_LE)
							{
								off = DOS_PROGRAM_LE_SIZE;
								fs -= DOS_PROGRAM_LE_SIZE;
							}
							
							status = spatch_apply(fp, dstfile, &file_copied, off, fs, patch->id, patch->spatch, &applied, &exists, to_apply & PATCH_DRY);
						}
						else /* W3 file */
						{
							pe_w3_t *w3 = pe_w3_read(&dos, &pe, fp);
							if(w3 != NULL)
							{
								size_t j;
								for(j = 0; j < w3->files_cnt; j++)
								{
									size_t file_size = 0;
									size_t file_offset = w3->files[j].file_offset;
									if(j+1 < w3->files_cnt)
									{
										file_size = w3->files[j+1].file_offset - file_offset;
									}
									else
									{
										file_size = w3->file_size - file_offset;
									}
									//printf("W3: %s at %u (%u)\n", w3->files[j].name, file_offset, file_size);
									status = spatch_apply(fp, dstfile, &file_copied, file_offset, file_size, patch->id, patch->spatch, &applied, &exists, to_apply & PATCH_DRY);
								}
								pe_w3_free(w3);
							}				
						} // W3
					} // not yet applied
				} // spatch
			} // normal patches
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

static int patch_select_me(FILE *fp, const char *dstfile, int *file_copied, uint64_t *applied, uint64_t *exists)
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
 * Apply binary patch
 *
 * @param offset: byte offset to file start
 * @param fs: file size of fp
 *
 **/
static int spatch_apply(FILE *fp, const char *dstfile, int *file_copied, uint32_t offset, uint32_t fs, uint64_t patch_id, const spatch_t *spatch, uint64_t *applied, uint64_t *exists, int dry)
{
	int status = PATCH_OK;
	
	/* there are localised strings at file end, so file size can be
	   different across national versions. So check files size +- 10%
	 */
	uint32_t step = spatch->filesize/10;
	uint32_t fs_min = spatch->filesize-step;
	uint32_t fs_max = spatch->filesize+step;
	int valid = 0;
	int patch_exists = 0;
	void *buf = malloc(SPATCH_BUF);
	
	if(buf == NULL)
	{
		return PATCH_E_MEM;
	}

	if((fs == 0) || (fs >= fs_min && fs <= fs_max))
	{
		const spatch_data_t *pdata = spatch->data;
		while(pdata->olddata != NULL)
		{
			fseek(fp, offset + pdata->offset, SEEK_SET);
			if(fread(buf, 1, pdata->size, fp) != pdata->size)
			{
				status = PATCH_E_READ;
				break;
			}

			if(memcmp(buf, pdata->olddata, pdata->size) != 0)
			{
				if(memcmp(buf, pdata->newdata, pdata->size) != 0)
				{
					break;
				}
				patch_exists = 1;
			}
			pdata++;
		}

		if(pdata->olddata == NULL)
		{
			valid = 1;
		}
	}

	if(valid && patch_exists)
	{
		(*exists) |= patch_id;
	}
	else if(valid)
	{
		(*applied) |= patch_id;
		if(!dry)
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
				const spatch_data_t *pdata = spatch->data;
				while(pdata->newdata != NULL)
				{
					fseek(fw, offset + pdata->offset, SEEK_SET);
					fwrite(pdata->newdata, 1, pdata->size, fw);									
					pdata++;
				}
				fclose(fw);
			}
			else
			{
				status = PATCH_E_WRITE;
			}
		}
	}
	
	free(buf);
	return status;
}

static int is_zero(uint8_t *ptr, size_t s)
{
	while(s)
	{
		if(ptr[s-1] != 0x00)
		{
			break;
		}
		s--;
	}
	return s == 0;
}

static uint32_t good_addrs[] = {0x4400, 0x4500, 0x4600, 0x2000, 0x800, 0};

/**
 * Apply special case for win.com patch
 *
 */
static int patch_select_wincom(FILE *fp, const char *dstfile, int *file_copied, uint64_t *applied, uint64_t *exists, int dry_run)
{
	int status = PATCH_OK;
	uint8_t data[4];
	fseek(fp, 0, SEEK_SET);
	fread(data, 1, 3, fp);
	if(data[0] == X86_JMP)
	{
		uint32_t def_jmp = data[1] | (((uint32_t)data[2]) << 8);
		def_jmp += 3 + COM_ORG;
		
		uint8_t *test_buf = malloc(sizeof(crfix_code));
		if(test_buf != NULL)
		{
			fseek(fp, JMP_TO_FILEOFF(def_jmp-CRFIX_CODE_START), SEEK_SET);
			//printf("check offset = %X (addr=0x%X)\n", JMP_TO_FILEOFF(def_jmp-CRFIX_CODE_START), def_jmp);
			
			fread(test_buf, 1, CRFIX_CHECK_BYTES, fp);
			if(memcmp(crfix_code, test_buf, CRFIX_CHECK_BYTES) != 0)
			{
				uint32_t *test_addr;
				for(test_addr = &good_addrs[0]; *test_addr != 0; test_addr++)
				{
					fseek(fp, JMP_TO_FILEOFF(*test_addr), SEEK_SET);	
					if(fread(test_buf, 1, sizeof(crfix_code), fp) == sizeof(crfix_code))
					{
						if(is_zero(test_buf, sizeof(crfix_code)))
						{
							if(dry_run == 0)
							{
								FILE *fw = NULL;
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
									/* copy code block */
									uint32_t jmp_im = JMP_TO_INST16_OFF(def_jmp, (*test_addr) + CRFIX_JMP_POS);
									//printf("return jmp: %X\n", jmp_im);
									
									memcpy(test_buf, crfix_code, sizeof(crfix_code));
									
									/* jump to original code */
									test_buf[CRFIX_JMP_POS]   = X86_JMP;
									test_buf[CRFIX_JMP_POS+1] = jmp_im & 0xFF;
									test_buf[CRFIX_JMP_POS+2] = (jmp_im >> 8) & 0xFF;

									/* write block */
									fseek(fw, JMP_TO_FILEOFF(*test_addr), SEEK_SET);
									fwrite(test_buf, 1, sizeof(crfix_code), fw);
									
									/* jump to new code at beginning */
									jmp_im = JMP_TO_INST16_OFF(*test_addr + CRFIX_CODE_START, COM_ORG);
									test_buf[0]   = X86_JMP;
									test_buf[1]   = jmp_im & 0xFF;
									test_buf[2]   = (jmp_im >> 8) & 0xFF;

									/* write begin 3 bytes */
									fseek(fw, 0, SEEK_SET);
									fwrite(test_buf, 1, 3, fw);

									fclose(fw);
								} // fw
							}
							*applied |= PATCH_WIN_COM;
							break;
						} // is_zero
					}
				} // for
			}
			else
			{
				*exists |= PATCH_WIN_COM;
			}
			
			free(test_buf);
		}
		else
		{
			status = PATCH_E_MEM;
		}
	}
	
	return status;
}

/**
 * Apply patch and if check failure check if applied.
 *
 **/
int patch_apply(const char *srcfile, const char *dstfile, uint64_t flags, int *applied)
{
	int status = PATCH_E_NOTFOUND;
	FILE        *fp;
	
	fp = FOPEN_LOG(srcfile, "rb");
	if(fp)
	{
		status = patch_selected(fp, dstfile, flags, NULL, NULL);
		
		fclose(fp);
		//printf("complete\n");
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
int patch_apply_wx(const char *srcfile, const char *dstfile, const char *tmpname, uint64_t flags)
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
		t = pe_read(&dos, &pe, fp, 1);
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
			t = pe_read(&dos, &pe, fp, 1);
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
				t = pe_read(&dos, &pe, fp, 1);
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

void patch_print(uint64_t patches)
{
	ppatch_t *patch;
	int cnt = 0;
	
	for(patch = ppathes; patch->name != NULL; patch++)
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

