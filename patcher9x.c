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

#define DEFAULT_PATH "C:\\Windows\\System"
#define DEFAULT_INPUT_VX  "VMM"
#define DEFAULT_INPUT_CAB "VMM32.VXD"
#define DEFAULT_OUTPUT_LE "VMM.VXD"
#define DEFAULT_OUTPUT_VX "VMM32.VXD"

static const char help[] = "Patch for Windows 98/Me for run on newest CPUs - AMD ZEN 2+, Intel Tiger Lake+\n"
	"Version: " PATCHER9X_VERSION_STR "\n\n"
	"Usage:\n%s [path] [batch options]\n"
	"path: path to installed windows directory or directory with windows instalation\n"
	"options:\n"
#ifndef DOS_MODE /* shorten help a bit to fit 80x25 screen */
	"\t-h,/?: print this help\n"
	"\t-v: print program version\n"
#endif
	"\t-auto,-y: use default actions (if path given) and don't bother user\n"
	"\t-cab-extract: extract WMM32.WXD from MS cab archive\n"
	"\t-wx-extract: extract WMM.VXD from WMM32.VXD\n"
	"\t-patch-tlb: apply to file patch\n"
	"\t-force-w3: when patching WMM32.VXD, leave it as W3 file\n"
	"\t-force-w4: when patching WMM32.VXD, always compress to W4 file\n"
	"\t-no-backup: don't backup overwrited files\n"
	"\t-millennium: use patch code for Windows Me - UNTESTED!\n"
	"\t-i <file>: override input file name\n"
	"\t-o <file>: override output file name\n"
	"\n"
	"Options can be chained:\n"
	"example: %s D:\\WIN98 -cab-extract -wx-extract -patch-tlb -o C:\\windows\\system\\VMM32\\VMM.VXD\n"
	"results patched VMM.VXD copied to system from instalation drive\n"
#ifndef DOS_MODE
	"\n"
	"example: %s C:\\WINDOWS\\SYSTEM\\VMM32.WXD -wx-extract -i INT13 -o INT13.VXD\n"
	"extract INT13.VXD from VMM32.WXD\n"
	"\n"
	"example: %s D:\\WIN98 -cab-extract -i IOS.VXD -o IOS.VXD\n"
	"search CAB archives in D:\\WIN98 and extract from them IOS.VXD\n"
	"\n"
	"Defaults:\n"
	"\t[path] = " DEFAULT_PATH "\n"
	"\t<input> = " DEFAULT_INPUT_VX " (if [path] leads to VMM32.VXD)\n"
	"\t<input> = " DEFAULT_INPUT_CAB " (if [path] leads to CAB archive)\n"
	"\t<output> = " DEFAULT_OUTPUT_LE " (if output is extracted from VMM32.VXD)\n"
	"\t<output> = " DEFAULT_OUTPUT_VX " (if output is patched VMM32.VXD)\n"
	"\n"
#endif
#ifdef RUN_WITHOUT_ARGS
	"\n"
	"When running without options program operate in interactive mode (ask user)\n"
#endif
;

static char userpath[MAX_PATH];
static const char default_path[] = DEFAULT_PATH;

static const char *question_dir_select[] = 
{
	"extract VMM.VXD from VMM32.VXD, patch it and save to VMM32 folder",
	"patch VMM.VXD/VMM32.VXD directly",
	"do 1 and 2 simultaneously"	,
	"scan *.CAB archives, extract VMM32.VXD and patch it directly",
	"extract VMM32.VXD from selected CAB and patch it directly",
};

#define MODE_AUTO        1 /* automaticly determine action from path */
#define MODE_INTERACTIVE 2 /* same as auto but ask user if sure */
#define MODE_EXACT       3 /* use steps by command line */

typedef struct _options_t
{
	int mode;
	const char *path;
	int print_help;
	int print_version;
	int cab_extract;
	int wx_extract;
	int patch;
	int force_w3;
	int force_w4;
	int no_backup;
	int millennium;
	const char *input;
	const char *output;
} options_t;

#define report_error(_code) print_error(_code, __FILE__, __LINE__);

static void print_error(int code, const char *file, int line)
{
	const char *msg = "Unknown";
	switch(code)
	{
		case PATCH_OK:
			msg = "success";
			break;
		case PATCH_E_READ:
			msg = "file read error";
			break;
		case PATCH_E_WRITE:
			msg = "file write error";
			break;
		case PATCH_E_CHECK:
			msg = "can not apply patch - original data sequence not found in the file";
			break;
		case PATCH_E_MEM:
			msg = "out of memory";
			break;
		case PATCH_E_OVERWRITE:
			msg = "can not overwrite existing file";
			break;
		case PATCH_E_WRONG_TYPE:
			msg = "wrong/unknown format format";
			break;
		case PATCH_E_CONVERT:
			msg = "Conversion error";
			break;
		case PATCH_E_NOTFOUND:
			msg = "File not found";
			break;
		case PATCH_E_PATCHED:
			msg = "File is already patched";
			break;
	}
	
	fprintf(stderr, "Error: %s\n(trace: %s on %d)\n", msg, file, line);
	
	switch(code)
	{
		case PATCH_E_READ:
		case PATCH_E_WRITE:
		case PATCH_E_OVERWRITE:
		case PATCH_E_NOTFOUND:
			print_trace();
			break;
	}
}

static void print_help(const char *progname)
{
#ifdef DOS_MODE
	printf(help, progname, progname);
#else
	printf(help, progname, progname, progname, progname);
#endif
}

/**
 * Parse argc/argv
 *
 **/
static int read_arg(options_t *options, int argc, char **argv)
{
	int i;
	memset(options, 0, sizeof(options_t));
	options->mode = MODE_INTERACTIVE;
	
	for(i = 1; i < argc; i++)
	{
		const char *arg = argv[i];
		if(istrcmp(arg, "-h") == 0 || istrcmp(arg, "/?") == 0)
		{
			options->print_help = 1;
		}
		else if(istrcmp(arg, "-v") == 0)
		{
			options->print_version = 1;
		}
		else if(istrcmp(arg, "-cab-extract") == 0)
		{
			options->cab_extract = 1;
			options->mode = MODE_EXACT;
		}
		else if(istrcmp(arg, "-wx-extract") == 0)
		{
			options->wx_extract = 1;
			options->mode = MODE_EXACT;
		}
		else if(istrcmp(arg, "-patch-tlb") == 0)
		{
			options->patch = 1;
			options->mode = MODE_EXACT;
		}
		else if(istrcmp(arg, "-force-w3") == 0)
		{
			options->force_w3 = 1;
		}
		else if(istrcmp(arg, "-force-w4") == 0)
		{
			options->force_w4 = 1;
		}
		else if(istrcmp(arg, "-no-backup") == 0)
		{
			options->no_backup = 1;
		}
		else if(istrcmp(arg, "-millennium") == 0
			/* for idiots like me, who cannot write "millennium" correctly */
			|| istrcmp(arg, "-milenium") == 0
			|| istrcmp(arg, "-millenium") == 0
			|| istrcmp(arg, "-milennium") == 0
		)
		{
			options->millennium = 1;
		}
		else if(istrcmp(arg, "-auto") == 0 || istrcmp(arg, "-y") == 0 )
		{
			if(options->mode == MODE_INTERACTIVE)
			{
				options->mode = MODE_AUTO;
			}
		}
		else if(istrcmp(arg, "-i") == 0)
		{
			if(i+1 < argc)
			{
				options->input = argv[++i];
				options->mode = MODE_EXACT;
			}
			else
			{
				fprintf(stderr, "Missing argument: expected file name\n");
				return -1;
			}
		}
		else if(istrcmp(arg, "-o") == 0)
		{
			if(i+1 < argc)
			{
				options->output = argv[++i];
			}
			else
			{
				fprintf(stderr, "Missing argument: expected file name\n");
				return -1;
			}
		}
		else
		{
			if(options->path == NULL)
			{
				options->path = arg;
			}
			else
			{
				fprintf(stderr, "Unknown argument: %s\n", arg);
				return -1;
			}
		}
	}
	
	if(options->force_w3 && options->force_w4)
	{
		fprintf(stderr, "Incompatible arguments: -force-w3 and -force-w4\n");
		return -1;
	}
	
	/* on non dos and Windows x86 system, print help if run without arguments */
#ifndef RUN_WITHOUT_ARGS
	if(options->path == NULL && options->mode == MODE_INTERACTIVE)
	{
		options->print_help = 1;
	}
#endif
	/* auto mode require path */
	if(options->path == NULL && options->mode == MODE_AUTO)
	{
		printf("Missing argument: path\n");
		return -1;
	}
	
	return 0;
}

static char *ask_user_path(options_t *options, const char *q, const char *default_path)
{
	char input_buffer[MAX_PATH+1];
	int i, c;
	
	if(options->path != NULL)
	{
		strcpy(userpath, options->path);
	}
	else if(options->mode == MODE_AUTO)
	{
		strcpy(userpath, default_path);
	}
	else
	{
		printf("%s [%s]: ", q, default_path);
		
		i = 0;
		do
		{
			c = fgetc(stdin);
			if(c != '\n' && c != EOF)
			{
				if(i < MAX_PATH)
				{
					input_buffer[i++] = c;
				}
			}
		} while(c != '\n' && c != EOF);
		input_buffer[i] = '\0';
		
		if(i > 0)
		{
			memcpy(userpath, input_buffer, i+1);
		}
		else
		{
			strcpy(userpath, default_path);
		}
	}
	
	return userpath;
}

#define USER_IN_MAX 16

static int ask_user(options_t *options, const char *q, const char **ans, int ans_count, int ans_default)
{
	char input_buffer[USER_IN_MAX+1];
	int i, c;
	long m = 0;
	char *ptr = NULL;
	
	if(options->mode == MODE_AUTO && ans_default != 0)
	{
		return ans_default;
	}
	
	printf("0: cancel execution\n");
	
	for(i = 0; i < ans_count; i++)
	{
		printf("%d: %s", i+1, ans[i]);
		if(i+1 == ans_default)
		{
			printf(" [default]");
		}
		printf("\n");
	}
	
	for(;;)
	{
		printf("Choose (0-%d): ", ans_count);
		
		i = 0;
		do
		{
			c = fgetc(stdin);
			if(c != '\n' && c != EOF)
			{
				if(i < USER_IN_MAX)
				{
					input_buffer[i++] = c;
				}
			}
		} while(c != '\n' && c != EOF);
		input_buffer[i] = '\0';
		
		if(i == 0)
		{
			return ans_default;
		}
	
		m = strtol(input_buffer, &ptr, 0);
		if(m == 0 && ptr != input_buffer)
		{
			break;
		}
		else if(m >= 1 && m <= ans_count)
		{
			break;
		}
	}
	
	return (int)m;
}

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
	
	return PATCH_E_NOTFOUND;
}

static int action_extract_cab(options_t *options, const char *path, const char *out)
{
	const char *in_cab_name = DEFAULT_INPUT_CAB;
	if(options->input)
	{
		in_cab_name = options->input;
	}
	
	if(cab_unpack(path, in_cab_name, out) > 0)
	{
		return PATCH_OK;
	}
	
	return PATCH_E_NOTFOUND;
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
	int flags = APPLY_VMM_98;
	char *tmpname;
	int t = PATCH_E_MEM;
	
	if(options->force_w3)
	{
		flags |= PATCH_FORCE_W3;
	}
	else if(options->force_w4)
	{
		flags |= PATCH_FORCE_W4;
	}
	
	if(options->millennium)
	{
		flags |= APPLY_VMM_ME;
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
 * Wait for user to press Enter
 *
 **/
static void wait_enter()
{
	int c;
	do
	{
		c = fgetc(stdin);
	} while(!(c == '\n' || c == EOF));
}

/**
 * Run and do exactly what is on command line
 *
 **/
static int run_exact(options_t *options)
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
  	upath = (char*)default_path;
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
		  		fs_rename(out, DEFAULT_OUTPUT_LE);
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
	  		
	  if(r == PATCH_OK)
	  {
		 	if(options->output)
		 	{
		 		fs_rename(out, options->output);
		 	}
		 	else
		 	{
		 		fs_rename(out, DEFAULT_OUTPUT_LE);
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


/**
 * Run and interactive with user
 *
 **/
static int run_interactive(options_t *options)
{
  char *upath = ask_user_path(options, "Enter path to WINDOWS\\SYSTEM, or Windows instalation\n", default_path);
  int upath_dir = 0;
  int user_ans  = 0;
  int patch_success = 0;
  		
  if(fs_is_dir(upath)) /* source is dir */
  {
  	int dir_is_system  = 0;
  	int dir_is_install = 0;
  	int dir_is_windows = 0;
  	int dir_have_vmm32 = 0;
  	int default_ans    = 0;
  	char *test_file;
  			
  	upath_dir = 1;
  			
  	test_file = fs_path_get(upath, "BASE4.CAB", NULL); /* Windows 98 install */
  	if(test_file)
  	{
  		if(fs_file_exists(test_file))
  		{
  			dir_is_install = 1;
  		}
  		fs_path_free(test_file);
  	}
  	
  	test_file = fs_path_get(upath, "BASE2.CAB", NULL); /* Windows Me install */
  	if(test_file)
  	{
  		if(fs_file_exists(test_file))
  		{
  			dir_is_install = 1;
  		}
  		fs_path_free(test_file);
  	}
  			
  	test_file = fs_path_get(upath, "KERNEL32.DLL", NULL); /* SYSTEM folder */
  	if(test_file)
  	{
  		if(fs_file_exists(test_file))
  		{
  			dir_is_system = 1;
  		}
  		fs_path_free(test_file);
  	}
  			
  	test_file = fs_path_get(upath, "COMMAND.COM", NULL); /* WINDOWS folder */
  	if(test_file)
  	{
  		if(fs_file_exists(test_file))
  		{
  			dir_is_windows = 1;
  		}
  		fs_path_free(test_file);
  	}
  			
  	test_file = fs_path_get(upath, "VMM32.VXD", NULL);
  	if(test_file)
  	{
  		if(fs_file_exists(test_file))
  		{
  			dir_have_vmm32 = 1;
  		}
  		fs_path_free(test_file);
  	}
  			
		if(dir_is_install)
		{
			default_ans = 4;
		}
  	else if(dir_is_system && dir_have_vmm32)
  	{
  		default_ans = 1;
  		if(options->millennium)
  		{
  			default_ans = 2;
  		}
  	}
  	else if(dir_is_windows)
  	{
  		fprintf(stderr, "Warning: Path looks like Windows directory, please choose \"Windows\\system\" directory!\n");
  	}
  			
  	user_ans = ask_user(options, "Select patch mode", question_dir_select, 5, default_ans);
  }
  else if(fs_file_exists(upath)) /* source is file */
  {
  	int default_ans = 0;
  	dos_header_t dos;
  	pe_header_t pe;
  	int type = 0;
  	FILE *fp = fopen(upath, "rb");
  			
  	type = pe_read(&dos, &pe, fp);
  	fclose(fp);
  			
  	switch(type)
  	{
  		case PE_W3:
  		case PE_W4:
  		case PE_LE:
  			default_ans = 2;
  			break;
  		case PE_NO_IS_MSCAB:
  			default_ans = 5;
  			break;
  		default:
  			fprintf(stderr, "Warning: can't determine file type! %d\n", type);
  			break;
  	}
  			
  	user_ans = ask_user(options, "Select patch mode", question_dir_select, 5, default_ans);
  }
  else
  {
  	fprintf(stderr, "Error: Path (%s) must lead to directory or file\n", upath);
  }
  		
 	if(user_ans == 0)
 	{
 		return EXIT_SUCCESS;
 	}
  	
 	/* extract VMM.VXD */
  if(user_ans == 1 || user_ans == 3)
  {
 		if(options->output == NULL) /* default to VMM32/VMM.VXD */
 		{
 			char *out   = NULL;
 			char *inptr = NULL;
 			char *in    = NULL;
 			char *tmp   = NULL;
  		char *dirname = fs_dirname(upath);
  		
  		if(upath_dir)
  		{
  			in = inptr = fs_path_get(upath, "VMM32.VXD", NULL);
  		}
  		else
  		{
  			in = upath;
  		}
  				
  		if(in != NULL)
  		{
				char *out_vmm32_dir = fs_path_get(dirname, "VMM32", NULL); /* windows/system/vmm32 folder */
				if(out_vmm32_dir != NULL)
	  		{
	  			if(!fs_is_dir(out_vmm32_dir))
	  			{
	  				if(fs_mkdir(out_vmm32_dir) != 0)
	  				{
	  					fprintf(stderr, "Error: VMM32 directory missing and cannot be created\n");
	  				}
	  			}
	  					
	  			tmp = fs_path_get(out_vmm32_dir, "VMM.@LE", NULL);
	  			if(tmp)
	  			{
		  			out = fs_path_get(out_vmm32_dir, "VMM.VXD", NULL);
		  			if(out)
		  			{
		  				if(fs_is_writeable_dir(out, NULL))
		  				{
		  					fprintf(stderr, "Error: %s is not writeable directory\n", out_vmm32_dir);
		  				}
		  				else
		  				{
		  					int t;
		  					
		  					patch_backup_file(out, options->no_backup);
		  					if((t = action_extract_vxd(options, in, tmp)) == PATCH_OK)
		  					{
		  						if((t = action_patch(options, tmp, out)) == PATCH_OK)
		  						{
		  							fs_unlink(tmp);
		  							patch_success++;
		  						}
		  						else
		  						{
		  							fs_unlink(tmp);
		  							report_error(t);
		  						}
		  					}
		  					else
		  					{
		  						report_error(t);
		  					}
		  				}
		  						
		  				fs_path_free(out);
		  			}
		  			fs_path_free(tmp);
		  		}
	  					
	  			fs_path_free(out_vmm32_dir);
	  			if(dirname != NULL)
	  			{
	  				fs_path_free(dirname);
	  			}
	  		} // out_vmm32_dir
	  				
	  		if(inptr != NULL)
	  		{
	  			fs_path_free(inptr);
	  		}
	  				
	  	} // in != NULL
  	}
  	else /* out from user */
  	{
  		int t;
  		patch_backup_file(options->output, options->no_backup);
  		if((t = action_extract_vxd(options, upath, options->output)) == PATCH_OK)
  		{
  			if((t = action_patch(options, options->output, options->output)) == PATCH_OK)
  			{
  				patch_success++;
  			}
  			else
  			{
  				report_error(t);
  			}
  		}
  		else
  		{
  			report_error(t);
  		}
  	}
  }
  			
  /* patch VMM32.VXD */
  if(user_ans == 2 || user_ans == 3)
  {
  	if(options->output == NULL) /* default to VMM32/VMM.VXD */
 		{
 			int t = 0;
 			char *out = NULL;
 			char *out_ptr = NULL;
 			
 			if(upath_dir)
 			{
 				out_ptr = out = fs_path_get2(upath, "VMM32" PATH_SEPARATOR "VMM.VXD", NULL);
 				
 				if(!fs_file_exists(out_ptr))
 				{
 					fs_path_free(out);
 					out_ptr = out = NULL;
  				out_ptr = out = fs_path_get2(upath, "VMM32.VXD", NULL);
  			}
  		}
  		else
  		{
  			out = upath;
  		}
  				
  		patch_backup_file(out, options->no_backup);
  		
  		if((t = action_patch(options, out, out)) == PATCH_OK)
  		{
  			patch_success++;
  		}
  		else
  		{
  			report_error(t);
  		}
  				
  		if(out_ptr)
  		{
 		 		fs_path_free(out_ptr);
 			}
 		}
 		else
 		{
 			int t;
 			patch_backup_file(options->output, options->no_backup);
 			
 			if((t = action_patch(options, options->output, options->output)) == PATCH_OK)
 			{
 				patch_success++;
 			}
 			else
 			{
 				report_error(t);
 			}
 		}
 	}
  			
 	/* scan CABs */
 	if(user_ans == 4)
 	{
 		int t = PATCH_E_MEM;
 		if(options->output == NULL)
 		{
 			char *out = fs_path_get2(upath, "VMM32.VXD", NULL);
 			if(out != NULL)
 			{
 				patch_backup_file(out, options->no_backup);
 				
				if(upath_dir)
				{
					if((t = action_extract_cabs(options, upath, out)) != PATCH_OK)
					{
						report_error(t);
					}
				}
				else
				{
					char *dirname = fs_dirname(upath);
					if(dirname != NULL)
					{
						if((t = action_extract_cabs(options, upath, out)) != PATCH_OK)
						{
							report_error(t);
						}
						fs_path_free(dirname);
					}
				}
				
				if(t == PATCH_OK)
	  		{
		  		if((t = action_patch(options, out, out)) == PATCH_OK)
		  		{
		  			patch_success++;
		  		}
		  		else
		  		{
		  			report_error(t);
		  		}
	  		}
	  		
	  		fs_path_free(out);
	 		}
 		}
 		else
 		{
 			patch_backup_file(options->output, options->no_backup);
 			
 			if((t = action_extract_cabs(options, upath, options->output)) != PATCH_OK)
 			{
 				report_error(t);
 			}
 			else
 			{
				if((t = action_patch(options, options->output, options->output)) == PATCH_OK)
				{
					patch_success++;
				}
		  	else
		  	{
		  		report_error(t);
		  	}
 			}
 		}
  } // CAB
  			
 	/* extract specific CAB */
 	if(user_ans == 5)
 	{
 		if(options->output == NULL)
 		{
			if(upath_dir)
	 		{
	 			fprintf(stderr, "Error: path must lead to file\n");
	 		}
	 		else
	 		{
	 			char *out = fs_path_get2(upath, "VMM32.VXD", NULL);
 				if(out != NULL)
 				{
 					int t;
 					
 					patch_backup_file(out, options->no_backup);
 						
					if((t = action_extract_cab(options, upath, out)) != PATCH_OK)
					{
						report_error(t);
					}
					else
					{
		  			if((t = action_patch(options, out, out)) != PATCH_OK)
		  			{
		  				report_error(t);
		  			}
		  			else
		  			{
		  				patch_success++;
		  			}
					}
 				}
			}
	 	}
 		else
 		{
 			int t;
 			
 			patch_backup_file(options->output, options->no_backup);
 				
 			if((t = action_extract_cabs(options, upath, options->output)) != PATCH_OK)
 			{
 				report_error(t);
 			}
 			else
 			{
	 			if((t = action_patch(options, options->output, options->output)) != PATCH_OK)
	 			{
	 				report_error(t);
	 			}
	 			else
	 			{
	 				patch_success++;
	 			}
 			}
 		}
	} // CABs
  	
	if(patch_success > 0)
	{
		printf("Patch applied successfully!\n");
	}
	else
	{
		printf("Patching proccess failure\n");
	}

	if(options->mode == MODE_INTERACTIVE)
	{
		printf("Press enter to exit...\n");
		wait_enter();
	}
  		
  if(patch_success > 0)
	{
		return EXIT_SUCCESS;
	}
	
	return EXIT_FAILURE;
}

int main(int argc, char **argv)
{
  int test;
  options_t options;
  
  /* self test by mspack */
  MSPACK_SYS_SELFTEST(test);
  if(test != MSPACK_ERR_OK)
  {
  	fprintf(stderr, "FATAL: CAB selftest failure\n");
  	return EXIT_FAILURE;
  }
  
  if(read_arg(&options, argc, argv) == 0)
  {
  	if(options.print_version)
  	{
  		printf("%s\n", PATCHER9X_VERSION_STR);
  		return EXIT_SUCCESS;
  	}
  	else if(options.print_help)
  	{
  		print_help(argv[0]);
  		return EXIT_SUCCESS;
  	} 
  	else if(options.mode == MODE_EXACT)
  	{
  		return run_exact(&options);
  	}
  	else /* run interactive */
  	{
  		return run_interactive(&options);
  	}
  }
  else
  {
  	printf("Command line error!\nUse %s -h to see help\n", argv[0]);
  }
  
  return EXIT_FAILURE;
}
