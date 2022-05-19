#include "patcher9x.h"
#include <bpatcher.h>
#include "vmm_patch.h"

int patch_apply(const char *srcfile, const char *dstfile)
{
	int status = PATCH_OK;
	FILE        *fp;
	
	fp = FOPEN_LOG(srcfile, "rb");
	if(fp)
	{
		ssize_t pos;
		bitstream_t check, patch;
		bs_mem(&check, (uint8_t*)vmm_orig_check, sizeof(vmm_orig_check));
			
		pos = search_sieve_file(fp, vmm_orig, sizeof(vmm_orig), &check);
		//printf("search_sieve_file: %lld\n", pos);
		if(pos >= 0)
		{
			FILE *fw = FOPEN_LOG(dstfile, "wb");
			if(fw != NULL)
			{
				void *buf = malloc(sizeof(vmm_patch));
				if(buf != NULL)
				{
					fseek(fp, 0, SEEK_SET);
					fs_file_copy(fp, fw, 0);
					
					fseek(fp, pos, SEEK_SET);
					if(fread(buf, 1, sizeof(vmm_patch), fp) == sizeof(vmm_patch))
					{
						bs_mem(&patch, (uint8_t*)vmm_patch_modif, sizeof(vmm_patch_modif));
						patch_sieve(buf, vmm_patch, sizeof(vmm_patch), &patch);
						
						fseek(fw, pos, SEEK_SET);
						fwrite(buf, 1, sizeof(vmm_patch), fw);							
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
		}
		else
		{
			status = PATCH_E_CHECK;
		}
		fclose(fp);
	}
	else
	{
		status = PATCH_E_READ;
	}
	
	return status;
}

int patch_apply_wx(const char *srcfile, const char *dstfile, const char *tmpname, int force_format)
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
				//printf("here1\n");
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
			status = patch_apply(tmpname, dstfile);
			if(force_format != PATCH_FORCE_W3)
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
			if(is_w3 && force_format == PATCH_FORCE_W4)
			{
				status = patch_apply(srcfile, tmpname);
				w4_decompres = 1;
			}
			else
			{
				status = patch_apply(srcfile, dstfile);
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
					//printf("here1\n");
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
	}
	
	return status;
}

int patch_backup_file(const char *path)
{
	FILE *fr, *fw;
	int status = -1;
	char *backupname = fs_path_get2(path, NULL, "bak");
	
	if(backupname)
	{
		//printf("BAKUP: %s -> %s\n", path, backupname);
		
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
			
			fs_path_free(backupname);
		}
		else
		{
			status = PATCH_OK;
		}
	}
	else
	{
		status = PATCH_E_MEM;
	}
	
	return status;
}

