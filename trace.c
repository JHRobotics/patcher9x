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

#define PATCHER_TRACE_DEEP 16

typedef struct _patcher_trace_t
{
	char fn_name[32];
	char dst_name[MAX_PATH];
	char dst_mode[6];
	char src_file[64];
	int src_line;
	int valid;
} patcher_trace_t;

static patcher_trace_t patcher_trace[PATCHER_TRACE_DEEP] = {};
static unsigned int patcher_trace_act = 0;

#define PATCHER_TRACE_CNT_UP(_var) \
	(((_var) + 1)%PATCHER_TRACE_DEEP)
#define PATCHER_TRACE_CNT_DOWN(_var) \
	(((_var) + PATCHER_TRACE_DEEP - 1)%PATCHER_TRACE_DEEP)

FILE *fopen_log(const char *fn, const char *mode, const char *file, int line)
{
	FILE *r = fopen(fn, mode);
	
	if(r == NULL)
	{
		patcher_trace_t *act = &patcher_trace[patcher_trace_act];
		memset(act, 0, sizeof(patcher_trace_act));
		
		strncpy(act->fn_name, "fopen", sizeof(act->fn_name)-1);
		strncpy(act->dst_name, fn,     sizeof(act->dst_name)-1);
		strncpy(act->dst_mode, mode,   sizeof(act->dst_mode)-1);
		strncpy(act->src_file, file,   sizeof(act->src_file)-1);
		act->src_line = line;
		act->valid = 1;
		
		patcher_trace_act = PATCHER_TRACE_CNT_UP(patcher_trace_act);
	}
	
	return r;
}

void print_trace()
{
	int p = 1;
	unsigned int i = PATCHER_TRACE_CNT_DOWN(patcher_trace_act);
	
	if(patcher_trace[i].valid)
	{
		printf("Invalid file operations:\n");
		for(;;)
		{
			if(patcher_trace[i].valid)
			{
				printf("\t%d: %s, %d: %s(%s, %s)\n", p++,
				 patcher_trace[i].src_file, patcher_trace[i].src_line, patcher_trace[i].fn_name,
				 patcher_trace[i].dst_name, patcher_trace[i].dst_mode
				);
			}
			else
			{
				break;
			}
			
			i = PATCHER_TRACE_CNT_DOWN(i);
		}
	}
}

void print_error(int code, const char *file, int line)
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
		case PATCH_E_NOTFOUNDINCAB:
			msg = "Source file not found in CAB archive";
			break;
		case PATCH_E_NOTFOUNDINCABS:
			msg = "Source file not found in *.CAB archives";
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

