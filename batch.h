/******************************************************************************
 * Copyright (c) 2023 Jaroslav Hensl                                          *
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

/*
 * Typedef and function names in separate file for README.txt ganeration
 */


#ifdef HELP_PROGNAME
	#define FUNC_NAME(_n) NULL
	struct _options_t;
	typedef struct _option_t options_t;
#else
	#define FUNC_NAME(_n) _n
#endif

typedef int (*batch_f)(options_t *options, int argc, char **argv);

typedef struct _batch_action_t
{
	batch_f func;
	const char *name;
	const char *desc;
} batch_action_t;

batch_action_t actions[] = {
	{FUNC_NAME(batch_cab_list),              "--cab-list", "archive.cab"},
	{FUNC_NAME(batch_cab_extract),           "--cab-extract", "archive.cab file1 [file2 [...]]"},
	{FUNC_NAME(batch_cabs_extract),          "--cabs-extract", "dir-to-search file1 [file2 [...]]"},
	{FUNC_NAME(batch_vxd_list),              "--vxd-list", "archive.vxd"},
	{FUNC_NAME(batch_vxd_extract),           "--vxd-extract", "archive.vxd file1 [file2 [...]]"},
	{FUNC_NAME(batch_vxd_convert),           "--vxd-convert", "archive.vxd"},
	{FUNC_NAME(batch_vxd_extract_all),       "--vxd-extract-all", "archive.vxd [destination-dir]"},
	{FUNC_NAME(batch_patch_all),             "--patch-all", "file.vxd [file2.vxd [...]]"},
	{FUNC_NAME(batch_patch_tlb),             "--patch-tlb", "file.vxd [file2.vxd [...]]"},
	{FUNC_NAME(batch_patch_cpuspeed),        "--patch-cpuspeed", "file.vxd [file2.vxd [...]]"},
	{FUNC_NAME(batch_patch_cpuspeed_ndis),   "--patch-cpuspeed-ndis", "file.vxd [file2.vxd [...]]"},
	{NULL, NULL, NULL}
};

void batch_help()
{
	batch_action_t *action = &(actions[0]);

  printf("\nBatch mode functions:\n");
	for(; action->name != NULL; action++)
	{
		printf("\t%-25s %s\n", action->name, action->desc);
	}
}

