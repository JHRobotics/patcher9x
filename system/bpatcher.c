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
#include <string.h>
#include "bpatcher.h"

/**
 * Search for 'needle' in 'haystack' using binary sieve on 'needle'.
 * It binary sieve's bit on position of 'needle' byte is ONE compare this byte
 * to haystack. If ZERO this byte doesn't matter and compare si skipped.
 *
 * @param haystack: buffer to search
 * @param haystack_size: buffer search in bytes
 * @param needle: string to search for
 * @param needle_size: needle size in bytes
 * @param: sieve: binary sieve to select important bytes in needle
 *
 * @return: if positive return position needle in haystack.
 *          Negative if not found.
 *
 **/
ssize_t search_sieve(const uint8_t *haystack, size_t haystack_size,
                     const uint8_t *needle, size_t needle_size, bitstream_t *sieve)
{
	uint32_t b;
	size_t j;
	
	for(j = 0; j < haystack_size-(needle_size-1); j++)
	{
		size_t i;
		
		bs_reset(sieve);
		for(i = 0; i < needle_size; i++)
		{
			b = bs_read_bit(sieve, 1);
			if(b > 0)
			{
				if(haystack[j+i] != needle[i])
				{
					goto search_sieve_next;
				}
			}
			
			
/*			if(i > 3)
			{
				printf("sieve: %d %d\n", i, j);
			}*/
		}
		return j;
		
		search_sieve_next:
			; /* NOP */
	}
	
	return -1;	
}

/**
 * Copy new data to buffer bytes that are on ONE at potition in binary sieve
 *
 * @param dst: destionation
 * @param newdata: data to write to buffer
 * @param data_size: size of newdata
 * @param sieve: binary sieve to select bytes in newdata to write
 *
 **/
void patch_sieve(uint8_t *dst, const uint8_t *newdata, size_t data_size,
                 bitstream_t *sieve)
{
	uint32_t b;
	size_t i;
	for(i = 0; i < data_size; i++)
	{
		b = bs_read_bit(sieve, 1);
		if(b > 0)
		{
			dst[i] = newdata[i];
		}
	}
}

/**
 * Compare two buffers set different bytes in binary sieve
 *
 * @param data_a: first buffer
 * @param data_b: secont buffer
 * @param data_size: size of buffers, if sizes of data_a and data_b
 *                   are diffent, this should be size of shorter one
 * @param sieve: output binary sieve
 *
 **/
void diff_sieve(const uint8_t *data_a, const uint8_t *data_b, size_t data_size,
                bitstream_t *sieve)
{
	uint32_t b;
	size_t i;
	for(i = 0; i < data_size; i++)
	{
		if(data_a[i] == data_b[i])
		{
			b = 0;
		}
		else
		{
			b = 1;
		}
		
		bs_write_bit(sieve, b, 1);
	}
	
	bs_write_flush(sieve);
}

/**
 * Search for 'needle' in opened file 'haystack_fp' using binary sieve on 'needle'.
 * It binary sieve's bit on position of 'needle' byte is ONE compare this byte
 * to haystack. If ZERO this byte doesn't matter and compare si skipped.
 *
 * @param haystack_fp: file opened by fopen
 * @param needle: string to search for
 * @param needle_size: needle size in bytes
 * @param: sieve: binary sieve to select important bytes in needle
 *
 * @return: if positive return file offet of needle in haystack.
 *          Else -1 if not found or -2 on error.
 *
 **/
ssize_t search_sieve_file(FILE *haystack_fp,
                          const uint8_t *needle, size_t needle_size,
                          bitstream_t *sieve)
{
	ssize_t offset = 0;
	ssize_t readed = 0;
	ssize_t result = -1;
	ssize_t res_search;
	size_t  buf_size = BPATCHER_FILE_BUF + needle_size - 1;
	uint8_t *buf = malloc(buf_size);
	if(buf == NULL)
	{
		return -2;
	}
	
	while(!feof(haystack_fp))
	{
		if(offset == 0)
		{
			readed = fread(buf, 1, buf_size, haystack_fp);
		}
		else
		{
			memcpy(buf, buf+BPATCHER_FILE_BUF, needle_size-1);
			readed = fread(buf+needle_size-1, 1, BPATCHER_FILE_BUF, haystack_fp);
		}
		
		if(readed != 0)
		{
			// FIXME: scan only actual (readed) size!
			res_search = search_sieve(buf, buf_size, needle, needle_size, sieve);
			if(res_search >= 0)
			{
				result = offset + res_search;
				if(offset != 0)
				{
					result -= needle_size-1;
				}
				break;
			}
		}
		
		offset += readed;
	}
	
	free(buf);
	
	return result;
}
