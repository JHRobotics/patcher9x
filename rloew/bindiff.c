/******************************************************************************
 * Copyright (c) 2025 Jaroslav Hensl                                          *
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
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <patch.h>
#include <string.h>

#define BUF_SIZE 1024

char oldbuf[BUF_SIZE];
char newbuf[BUF_SIZE];

static spatch_data_t lastdiff = 
{
	0,
	0,
	oldbuf,
	newbuf
};

static void upper_case(const char *src, char *dst, size_t dstsize)
{
	size_t i;
	size_t len = strlen(src);
	if(len >= dstsize)
	{
		len = dstsize-1;
	}
	
	for(i = 0; i < len; i++)
	{
		dst[i] = toupper(src[i]);
	}
	dst[len] = '\0';
}

#define PREFIX_MAX 256
static char prefix_upper[PREFIX_MAX];

static void print_diff(FILE *stream)
{
	uint32_t i;
	fprintf(stream, "\t{0x%08X, %u, \"", lastdiff.offset, lastdiff.size);
	for(i = 0; i < lastdiff.size; i++)
	{
		//if(i != 0) fputc(',', stream);
		fprintf(stream, "\\x%02X", (uint8_t)lastdiff.olddata[i]);
	}
	fprintf(stream, "\", \"");
	for(i = 0; i < lastdiff.size; i++)
	{
		//if(i != 0) fputc(',', stream);
		fprintf(stream, "\\x%02X", (uint8_t)lastdiff.newdata[i]);
	}
	fprintf(stream, "\"},\n");
}

int main(int argc, char *argv[])
{
	int close_out = 0;
	uint32_t limit = 0;
	
	if(argc < 5)
	{
		printf("Usage: %s <prefix> <original file> <patched file> <output.h> [limit]\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	const char *prefix  = argv[1];
	const char *oldname = argv[2];
	const char *newname = argv[3];
	const char *outname = argv[4];
	
	if(argc >= 6)
	{
		limit = strtoul(argv[5], NULL, 0);
	}

	upper_case(prefix, prefix_upper, PREFIX_MAX);
	
	FILE *oldf = fopen(oldname, "rb");
	if(oldf == NULL)
	{
		fprintf(stderr, "Cannot open %s!\n", oldname);
		return EXIT_FAILURE;
	}

	FILE *newf = fopen(newname, "rb");
	if(newf == NULL)
	{
		fprintf(stderr, "Cannot open %s!\n", newname);
		fclose(oldf);
		return EXIT_FAILURE;
	}

	FILE *out = stdout;
	if(strcmp(outname, "-") != 0)
	{
		out = fopen(outname, "wb");
		close_out = 1;
	}
	if(out == NULL)
	{
		fprintf(stderr, "Cannot open output %s!\n", outname);
		fclose(newf);
		fclose(oldf);
		return EXIT_FAILURE;
	}

	fseek(oldf, 0, SEEK_END);
	uint32_t fs = ftell(oldf);
	fseek(oldf, 0, SEEK_SET);
	
	fprintf(out,
		"#ifndef __%s__PATCH_INCLUDED__\n"
		"#define __%s__PATCH_INCLUDED__\n\n"
		"#include <patch.h>\n\n"
		"const spatch_data_t %s_data[] = {\n",
		prefix_upper, prefix_upper,
		prefix
	);
	
	uint32_t offset = 0;
	for(;;offset++)
	{
		int c1 = fgetc(oldf);
		int c2 = fgetc(newf);
		
		if(limit)
		{
			if(offset >= limit)
			{
				break;
			}
		}
		
		if(c1 == EOF || c2 == EOF)
		{
			break;
		}
		
		if(c1 != c2)
		{
			if(lastdiff.offset+lastdiff.size == offset)
			{
				lastdiff.olddata[lastdiff.size] = c1;
				lastdiff.newdata[lastdiff.size] = c2;
				lastdiff.size++;
			}
			else
			{
				if(lastdiff.size != 0)
				{
					print_diff(out);
				}
				lastdiff.offset = offset;
				lastdiff.size   = 1;
				lastdiff.olddata[0] = c1;
				lastdiff.newdata[0] = c2;
			}
		}
	}

	if(lastdiff.size != 0)
	{
		print_diff(out);
	}
	
	fprintf(out,
		"\t{0, 0, NULL, NULL}\n"
		"};\n\n"
		"const spatch_t %s_sp = {%u, %s_data};\n\n"
		"#endif /*__%s__PATCH_INCLUDED__*/\n",
		prefix, fs, prefix, prefix_upper
	);
	
	fclose(newf);
	fclose(oldf);
	if(close_out)
	{
		fclose(out);
	}

	return EXIT_SUCCESS;
}
