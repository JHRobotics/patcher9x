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

static int action_extract_cabs(options_t *options, const char *path, const char *out)
{
	const char *in_cab_name = DEFAULT_INPUT_CAB;
	if(options->input)
	{
		in_cab_name = options->input;
	}
	
	if(cab_search_unpack(path, in_cab_name, out) > 0)
	{
		return PATCH_OK;
	}
	
	return PATCH_E_NOTFOUNDINCABS;
}

static int action_extract_cab(options_t *options, const char *path, const char *out)
{
	const char *in_cab_name = DEFAULT_INPUT_CAB;
	if(options->input)
	{
		in_cab_name = options->input;
	}
	
	if(cab_unpack(path, in_cab_name, out, NULL) > 0)
	{
		return PATCH_OK;
	}
	
	return PATCH_E_NOTFOUNDINCAB;
}

static int action_extract_vxd(options_t *options, const char *path, const char *out)
{
	char *tmpname = NULL;
	const char *in_driver = DEFAULT_INPUT_VX;
	if(options->input)
	{
		in_driver = options->input;
	}
	
	tmpname = fs_path_get2(out, "VMM32.@W4", NULL);
	if(tmpname != NULL)
	{
		int t = wx_unpack(path, in_driver, out, tmpname);
		fs_path_free(tmpname);
		return t;
	}
	
	return PATCH_E_MEM;
}

static int action_patch(options_t *options, const char *path, const char *out)
{
	uint64_t flags = 0;
	char *tmpname;
	int t = PATCH_E_MEM;
	
	if(options->mode != MODE_EXACT)
	{
		flags = PATCH_VMM_ALL;
	}
	else
	{
		flags = options->patches;
	}
	
	if(options->force_w3)
	{
		flags |= PATCH_FORCE_W3;
	}
	else if(options->force_w4)
	{
		flags |= PATCH_FORCE_W4;
	}
	
	tmpname = fs_path_get2(out, "VMM32.@W4", NULL);
	
	if(tmpname != NULL)
	{
		if(strcmp(path, out) != 0)
		{
			t = patch_apply_wx(path, out, tmpname, flags);
		}
		else
		{
			char *tmpname2 = fs_path_get2(out, "VMM32.@WL", NULL);
			if(tmpname2 != NULL)
			{
				if((t = patch_apply_wx(path, tmpname2, tmpname, flags)) == PATCH_OK)
				{
					fs_rename(tmpname2, out);
				}
				fs_path_free(tmpname2);
			}
		}
		
		fs_path_free(tmpname);
	}
	
	return t;
}


/**
 * Run and do exactly what is on command line
 *
 **/
int run_exact(options_t *options)
{
  char *out = (char*)options->output;
  char *out2 = NULL;
  char *out3 = NULL;
  char *out_mem = NULL;
  char *out2_mem = NULL;
  char *out3_mem = NULL;
  		
  char *upath = (char*)options->path;
  if(upath == NULL)
  {
  	upath = (char*)patcher9x_default_path;
  }
  		
  if(options->cab_extract)
  {
  	int r;
  	if(out == NULL)
  	{
  		out = "VMM32.@WX";
  	}
  	else
  	{
  		out_mem = out = fs_path_get2(out, NULL, "@WX");
  	}
  			
  	if(fs_is_dir(upath))
  	{
  		r = action_extract_cabs(options, upath, out);
  	}
  	else
  	{
  		r = action_extract_cab(options, upath, out);
  	}
  			
  	if(options->wx_extract)
  	{
  		out2_mem = out2 = fs_path_get2(out, NULL, "@WL");
  		
  		r = action_extract_vxd(options, out, out2);
  				
  		if(r == PATCH_OK)
  		{
				if(options->patch)
				{
					out3_mem = out3 = fs_path_get2(out, NULL, "@WP");
					r = action_patch(options, out2, out3);
	  					
	  			if(r == PATCH_OK)
	  			{
						if(options->output)
						{
							fs_rename(out3, options->output);
						}
						else
						{
							fs_rename(out3, DEFAULT_OUTPUT_LE);
						}
						
						fs_unlink(out2);
						fs_unlink(out);
	  			}
	  			else
			  	{
			  		report_error(r);
			  	}
	  		}
	  		else
	  		{
		  		if(options->output)
		  		{
		  			fs_rename(out2, options->output);
		  		}
		  		else
		  		{
		  			fs_rename(out2, DEFAULT_OUTPUT_LE);
		  		}
		  		fs_unlink(out);
	  		}
	  	}
	  	else
	  	{
	  		report_error(r);
	  	}
  	}
  	else if(options->patch)
  	{
	  	out2_mem = out2 = fs_path_get2(out, NULL, "@WP");
	  	r = action_patch(options, out, out2);
	  				
	  	if(r == PATCH_OK)
	  	{
		 		if(options->output)
		 		{
		 			fs_rename(out2, options->output);
		 		}
		 		else
		 		{
		 			fs_rename(out2, DEFAULT_OUTPUT_LE);
		 		}
		 		
		 		fs_unlink(out);
	  	}
	  	else
		 	{
		 		report_error(r);
		 	}
  	}
  	else
  	{
  		if(options->output)
  		{
  			fs_rename(out, options->output);
  		}
  		else
  		{
  			fs_rename(out, DEFAULT_OUTPUT_VX);
  		}
 		}
  			
  } // cab_extract
  else if(options->wx_extract)
  {
  	int r;
  	if(out == NULL)
  	{
  		out = DEFAULT_OUTPUT_LE;
  	}
  	else
  	{
  		out = fs_path_get2(out, NULL, "@WP");
  	}
  			
  	r = action_extract_vxd(options, upath, out);
  				
  	if(r == PATCH_OK)
  	{
	  	if(options->patch)
	  	{
	  		out2 = fs_path_get2(out, NULL, "@WP");
	  		r = action_patch(options, out, out2);
	  		
	  		if(r == PATCH_OK)
	  		{
					if(options->output)
					{
						fs_rename(out2, options->output);
					}
					else
					{
						fs_rename(out2, DEFAULT_OUTPUT_LE);
					}
					
					fs_unlink(out);
	  		}
	  		else
				{
					report_error(r);
				}
	  	}
	  	else
	  	{
		  	if(options->output)
		  	{
		  		fs_rename(out, options->output);
		  	}
		  	else
		  	{
		  		if(options->input)
		  		{
		  			char *out_vxd = fs_path_get(NULL, options->input, "vxd");
		  			if(out_vxd)
		  			{
		  				fs_rename(out, out_vxd);
		  				
		  				fs_path_free(out_vxd);
		  			}
		  		}
		  		else
		  		{
		  			fs_rename(out, DEFAULT_OUTPUT_LE);
		  		}
		  	}
	  	}
	  }
	  else
	  {
	  	report_error(r);
	  }
  } // VX extact
  else if(options->patch)
  {
  	int r;
  	if(out == NULL)
  	{
  		out = DEFAULT_OUTPUT_LE;
  	}
  	else
  	{
  		out_mem = out = fs_path_get2(out, NULL, "@WP");
  	}
  	
		r = action_patch(options, upath, out);
		//printf("action_patch: %d\n", r);
	  
	  if(r == PATCH_OK)
	  {
		 	if(options->output)
		 	{
		 		fs_rename(out, options->output);
		 	}
		 	else
		 	{
		 		fs_rename(out, upath);
		 	}
	  }
	  else
		{
			report_error(r);
	  }
	} // patch
	
	/* alloc cleanup */
	if(out_mem  != NULL) fs_path_free(out_mem);
	if(out2_mem != NULL) fs_path_free(out2_mem);
	if(out3_mem != NULL) fs_path_free(out3_mem);
	
	return EXIT_SUCCESS;
}

