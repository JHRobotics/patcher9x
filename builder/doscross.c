#include <stdio.h>
#include <string.h>

#define DEFAULT_PATH           "C:\\Windows\\System"
#define DEFAULT_INPUT_VX       "VMM"
#define DEFAULT_INPUT_CAB      "VMM32.VXD"
#define DEFAULT_OUTPUT_LE      "VMM.VXD"
#define DEFAULT_OUTPUT_VX      "VMM32.VXD"

#define HELP_EXTRA

#include "../version.h"
#include "../help.h"

#define HELP_PROGNAME "patch9x"
#include "../batch.h"

int main(int argc, char **argv)
{
	if(argc >= 2)
	{
		if(strcmp(argv[1], "help") == 0)
		{
			const char *progname = "patch9x";
			if(argc >= 3)
			{
				progname = argv[2];
			}
			printf(help, progname, progname);
			batch_help();
		}
		else if(strcmp(argv[1], "version") == 0)
		{
			printf("%s\n", PATCHER9X_VERSION_STR);
		}
	}
	
	return 0;
}


