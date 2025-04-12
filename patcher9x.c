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
 ******************************************************************************/
#include "patcher9x.h"
#include <stdarg.h>

#ifdef RUN_WITHOUT_ARGS
# define HELP_EXTRA
#endif

#ifdef DOS_MODE
# define HELP_SHORT
# include "help.h"
# undef HELP_SHORT
# define HELP_NAME help_long
# include "help.h"
#else
# include "help.h"
#endif

static char userpath[MAX_PATH];
char patcher9x_default_path[MAX_PATH] = DEFAULT_PATH;

static const char *question_dir_select[] = 
{
	"patch files, VMM32.VXD will be extracted, extracted files patched",
	"patch files, VMM32.VXD will be patched directly",
	"do 1 and 2 simultaneously"	,
	"scan *.CAB archives, extract files and patch them (VMM32 directly)",
};

static const char *question_file_selelect[] = 
{
	"patch file directly",
	"handle as CAB file, extract files and patch them (VMM32 directly)",
};

#define PROGRAMS_MAX 16

static void print_help(const char *progname, int longer)
{
#ifdef DOS_MODE
	if(longer > 1)
	{
		printf(help_long, HELP_LONG_REP_ARG(progname));
		batch_help();
	}
	else
	{
		printf(help, HELP_SHORT_REP_ARG(progname));
	}
#else
	printf(help, HELP_LONG_REP_ARG(progname));
	batch_help();
#endif
}

/**
 * Parse argc/argv
 *
 **/
static int read_arg(options_t *options, int argc, char **argv, int *batch_id, int *batch_argc, char **batch_argv)
{
	int i;
	memset(options, 0, sizeof(options_t));
	options->mode = MODE_INTERACTIVE;
	int unk_args = 0;
	int unk_arg  = 0;
	
	*batch_argc = 0;
	
	for(i = 1; i < argc; i++)
	{
		const char *arg = argv[i];
		if(istrcmp(arg, "-h") == 0 || istrcmp(arg, "/?") == 0)
		{
			options->print_help = 1;
		}
		#ifdef DOS_MODE
		else if(istrcmp(arg, "-hh") == 0)
		{
			options->print_help = 2;
		}
		#endif
		else if(istrcmp(arg, "-v") == 0)
		{
			options->print_version = 1;
		}
		else if(istrcmp(arg, "-cputest") == 0)
		{
			options->cputest = 1;
		}
		else if(istrcmp(arg, "-force-w3") == 0)
		{
			options->patches |= PATCH_FORCE_W3;
			options->force_w3 = 1;
		}
		else if(istrcmp(arg, "-force-w4") == 0)
		{
			options->patches |= PATCH_FORCE_W4;
			options->force_w4 = 1;
		}
		else if(istrcmp(arg, "-force-tlb") == 0)
		{
			options->patches |= PATCH_VMM_ALL;
		}
		else if(istrcmp(arg, "-force-cpuspeed") == 0)
		{
			options->patches |= PATCH_CPU_SPEED_ALL;
		}
		else if(istrcmp(arg, "-force-cpuspeed-ndis") == 0)
		{
			options->patches |= PATCH_CPU_SPEED_NDIS_ALL;
		}
		else if(istrcmp(arg, "-no-tlb") == 0)
		{
			options->unmask |= PATCH_VMM_ALL;
		}
		else if(istrcmp(arg, "-no-cpuspeed") == 0)
		{
			options->unmask |= PATCH_CPU_SPEED_ALL;
		}
		else if(istrcmp(arg, "-no-cpuspeed-ndis") == 0)
		{
			options->unmask |= PATCH_CPU_SPEED_NDIS_ALL;
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
			// ignore
		}
		else if(istrcmp(arg, "-auto") == 0 || istrcmp(arg, "-y") == 0 )
		{
			if(options->mode == MODE_INTERACTIVE)
			{
				options->mode = MODE_AUTO;
			}
		}
		else if(batch_arg(arg) >= 0)
		{
			options->mode = MODE_BATCH;
			if(*batch_id >= 0)
			{
				fprintf(stderr, "Possible to use only ONE batch function in ONE run\n");
				return -2;
			}
			*batch_id = batch_arg(arg);
		}
		else
		{
			if(options->path == NULL)
			{
				options->path = arg;
			}
			else
			{
				if(unk_args++ == 0)
				{
					unk_arg = i;
				}
			}
			
			batch_argv[*batch_argc] = (char*)arg;
			(*batch_argc)++;
		}
	}
	
	if(options->mode != MODE_BATCH && unk_args > 0)
	{
		fprintf(stderr, "Unknown argument: %s\n", argv[unk_arg]);
		return -1;
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
					if(c != '\r')
					{
						input_buffer[i++] = c;
					}
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
	
	printf("0: cancel execution");
	if(ans_default == 0)
	{
		printf(" [default]");
	}
	printf("\n");
	
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
					if(c != '\r')
					{
						input_buffer[i++] = c;
					}
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

#define USER_NO   0
#define USER_YES  1
#define USER_MORE 2

/**
 * Wait for user to press Enter
 *
 **/
static int ask_user_patch(options_t *options)
{
	int ans = USER_NO;
	int c;
	
	if(options->mode == MODE_AUTO)
	{
		return USER_YES;
	}
	
	printf("Apply patch/patches? (Y - Yes, N - No, M - print more information): ");	
	
	do
	{
		c = fgetc(stdin);
		if(c == '\r') continue;
		
		switch(c)
		{
			case 'y':
			case 'Y':
				ans = USER_YES;
				break;
			case 'N':
			case 'n':
				ans = USER_NO;
				break;
			case 'M':
			case 'm':
				ans = USER_MORE;
				break;
		}
	} while(!(c == '\n' || c == EOF));
	
	return ans;
}

/**
 * Run and interactive with user
 *
 **/
static int run_interactive(options_t *options)
{
  char *upath = ask_user_path(options, "Enter path to WINDOWS\\SYSTEM, or Windows installation\n", patcher9x_default_path);
  int upath_dir = 0;
  int user_ans  = 0;
  int patch_success = 0;
  uint32_t lookup_flags = 0;
  
 	pmodfiles_t list;
 	int cnt = 0;
  
  /*
   * Detection where the path entered by user leads
   */
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
  	
  	test_file = fs_path_get(upath, "WIN95_02.CAB", NULL); /* Windows 95 install */
  	if(test_file)
  	{
  		if(fs_file_exists(test_file))
  		{
  			dir_is_install = 1;
  		}
  		fs_path_free(test_file);
  	}
  	
  	test_file = fs_path_get(upath, "WIN9X30.CAB", NULL); /* "Memphis" install */
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
  		/* default selection for now */
  		default_ans = 2;
  	}
  	else if(dir_is_windows)
  	{
  		fprintf(stderr, "Warning: Path looks like Windows directory, please choose \"Windows\\system\" directory!\n");
  	}
  	
  	user_ans = ask_user(options, "Select patch mode", question_dir_select, 4, default_ans);
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
  			default_ans = 1;
  			break;
  		case PE_NO_IS_MSCAB:
  			default_ans = 2;
  			break;
  		default:
  			fprintf(stderr, "Warning: can't determine file type! %d\n", type);
  			break;
  	}
  	
  	user_ans = ask_user(options, "Select patch mode", question_file_selelect, 2, default_ans);
  }
  else
  {
  	fprintf(stderr, "Error: Path (%s) must lead to directory or file\n", upath);
  }
  		
 	if(user_ans == 0)
 	{
 		return EXIT_SUCCESS;
 	}
  
  /* if dir on individual CAB*/
  if(upath_dir != 0)
  {
	  if(user_ans == 1) /* extract from VMM32.VXD */
	  {
	  	lookup_flags |= PATCH_LOOKUP_EXTRACTWX;
	  	lookup_flags |= PATCH_LOOKUP_NO_VMM32;
	  }
	  /*
	  else if(user_ans == 2){} // patch VMM32.VXD
	  */
	  else if(user_ans == 3) /* extract from VMM32.VXD and patch VMX.VMD */
	  {
	  	lookup_flags |= PATCH_LOOKUP_EXTRACTWX;
	  }
	  else if(user_ans == 4) /* extract from CABs */
	  {
	  	lookup_flags |= PATCH_LOOKUP_CABS;
	  }
  	
  	if(fs_is_writeable_dir(upath, NULL) == 0)
 		{
			fprintf(stderr, "Error: %s is not writeable directory\n", upath);
			goto run_interactive_fail;
		}
		else
		{
  		list = files_lookup(upath, options->patches, options->unmask, lookup_flags);
  		if(list == NULL)
  		{
  			report_error(PATCH_E_MEM);
  		}
  	}
  }
  else // upath_dir == 0
  {
  	if(user_ans == 1) /* patch file */
  	{
  		list = files_apply(upath, options->patches, options->unmask);
  		if(list == NULL)
  		{
  			report_error(PATCH_E_MEM);
  		}
  	}
  	else if(user_ans == 2) /* extrac specific CAB */
	  {
  		list = files_lookup(upath, options->patches, options->unmask, PATCH_LOOKUP_ONE_CAB);
  		if(list == NULL)
  		{
  			report_error(PATCH_E_MEM);
  		}
	  }
  }
  
	if(list != NULL)
	{
		cnt = files_status(list);
		if(cnt > 0)
		{
	 		int ask_retry = 0;
  		do
  		{
  			ask_retry = 0;
  			int ans = ask_user_patch(options);
  			switch(ans)
  			{
  				case USER_YES:
  					patch_success = files_commit(&list, options->no_backup);
  					break;
  				case USER_NO:
  					files_cleanup(&list);
  					printf("Operation aborted by user\n");
 						return EXIT_SUCCESS;
  					break;
  				case USER_MORE:
  					files_print(list);
  					ask_retry = 1;
  					break;
  			}
  		} while(ask_retry);
  	}
  	else
  	{
  		files_cleanup(&list);
  	}
  }
  
run_interactive_fail:
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
  int batch_id = -1;
  int batch_argc = 0;
  char **batch_argv = NULL;
  options_t options;
  
  set_default_path(patcher9x_default_path);
  
  /* self test by mspack */
  MSPACK_SYS_SELFTEST(test);
  if(test != MSPACK_ERR_OK)
  {
  	fprintf(stderr, "FATAL: CAB selftest failure\n");
  	return EXIT_FAILURE;
  }
  
  batch_argv = calloc(sizeof(char*), argc);
  
  if(read_arg(&options, argc, argv, &batch_id, &batch_argc, batch_argv) == 0)
  {
  	if(options.cputest)
  	{
  		cputest();
  		return EXIT_SUCCESS;
  	}
  	else if(options.print_version)
  	{
  		printf("%s\n", PATCHER9X_VERSION_STR);
  		return EXIT_SUCCESS;
  	}
  	else if(options.print_help)
  	{
  		print_help(argv[0], options.print_help);
  		return EXIT_SUCCESS;
  	} 
  	/*else if(options.mode == MODE_EXACT)
  	{
  		return run_exact(&options);
  	}*/
  	else if(options.mode == MODE_BATCH)
  	{
  		return batch_run(&options, batch_id, batch_argc, batch_argv);
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
