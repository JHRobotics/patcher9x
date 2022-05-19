#include "patcher9x.h"

int cab_unpack(const char *srccab, const char *infilename, const char *out)
{
	int cnt = 0;
  struct mscab_decompressor *cabd;
  struct mscabd_cabinet *cab;
  struct mscabd_file *file;
  
	if ((cabd = mspack_create_cab_decompressor(NULL)))
	{
		if ((cab = cabd->open(cabd, srccab)))
		{
			for (file = cab->files; file; file = file->next)
			{
				if(istrcmp(file->filename, infilename) == 0)
				{
					//printf("\t%s\n", file->filename);
					
					int t = cabd->extract(cabd, file, out);
					if(t == MSPACK_ERR_OK)
					{
						cnt++;
						break;
					}
				}
      }
      cabd->close(cabd, cab);
    }
    mspack_destroy_cab_decompressor(cabd);
  }
  
  return cnt;
}

int cab_search_unpack(const char *dirname, const char *infilename, const char *out)
{
	fs_dir_t *dir = fs_dir_open(dirname);
	int cnt = 0;
	const char *fn;
  if(dir)
  {
  	while((fn = fs_dir_read(dir, FS_FILTER_FILE)) != NULL)
  	{
  		if(fs_ext_match(fn, "cab") != 0)
  		{			
  			char *cabfile = fs_path_get(dirname, fn, NULL);
  			
  			if(cab_unpack(cabfile, infilename, out) > 0)
  			{
  				cnt++;
  			}
  			fs_path_free(cabfile);
  		}
  		
  		if(cnt > 0)
  		{
  			break;
  		}
  	}
  	fs_dir_close(&dir);
  }
  
  return cnt;
}

int wx_unpack(const char *src, const char *infilename, const char *out, const char *tmpname)
{
	dos_header_t dos, dos2;
	pe_header_t  pe, pe2;
	pe_w3_t     *w3;
	pe_w4_t     *w4;
	FILE        *fp, *fp2;
	int          t;
	int status = PATCH_OK;
	
	fp = FOPEN_LOG(src, "rb");
	if(fp)
	{
		t = pe_read(&dos, &pe, fp);
		if(t == PE_W3)
		{
			w3 = pe_w3_read(&dos, &pe, fp);
			if(w3 != NULL)
			{
				pe_w3_extract(w3, infilename, out);
				
				pe_w3_free(w3);
			}
			else
			{
				status = PATCH_E_READ;
			}
		}
		else if(t == PE_W4)
		{
			w4 = pe_w4_read(&dos, &pe, fp);
			if(w4 != NULL)
			{
				//printf("here1\n");
				if(pe_w4_to_w3(w4, tmpname) != PE_OK)
				{
					status = PATCH_E_CONVERT;
				}
				else
				{
					fp2 = FOPEN_LOG(tmpname, "rb");
					if(fp2)
					{
						t = pe_read(&dos2, &pe2, fp2);
						if(t == PE_W3)
						{
							w3 = pe_w3_read(&dos2, &pe2, fp2);
							if(w3 != NULL)
							{
								pe_w3_extract(w3, infilename, out);
								
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
						fclose(fp2);
						
						fs_unlink(tmpname);
					}
					else
					{
						status = PATCH_E_READ;
					}
				}
				pe_w4_free(w4);
			}
			else
			{
				status = PATCH_E_READ;
			}
		}
		else
		{
			status = PATCH_E_WRONG_TYPE;
		}
		fclose(fp);
	}
	else
	{
		status = PATCH_E_READ;
	}
	
	return status;
}
