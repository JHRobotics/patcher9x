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
