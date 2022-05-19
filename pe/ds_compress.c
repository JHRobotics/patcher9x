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
#include <stdio.h>
#include <assert.h>
#include "doublespace.h"

#define FS_SEGMENT_SIZE 512

/**
 * Fast integer calculation of Ln2
 *
 * @param n: number between 1 and SIZE_MAX
 * @return: Ln(n) rounded down (exm.: fast_ln2(3) = 1)
 *
 **/
static size_t fast_ln2(size_t n)
{
	size_t r = 0;
	
	do
	{
		n >>= 1;
		if(n)
		{
			r++;
		}
	} while(n);

	return r;	
}

/**
 * Write Count and Depth to buffer
 *
 * @param count: Count of chars (between 1 and 4414)
 * @param pos: Depth of string copy (between 1 and 512)
 * @param buf: pointer to bit buffer
 * @param buf_pos: pointer to bit buffer position (0 means empty buffer)
 *
 **/
static void ds_out_pos(size_t count, size_t pos, bitstream_t *out)
{
	uint32_t buf2 = 0;
	size_t pos2 = 0;
	size_t zeroes = 0;
	
	assert(count >= 2);
	assert(pos   < 4415);
	
	if(pos < 64)
	{
		buf2 = (pos << 2) | 0x0;
		pos2 = 8;
	}
	else if(pos < 320)
	{
		buf2 = ((pos-64) << 3) | 0x3; // 0b011
		pos2 = 11;
	}
	else if(pos < 4415)
	{
		buf2 = ((pos-320) << 3) | 0x7; // 0b111
		pos2 = 15;
	}
	
	int num = 0;
		
	zeroes = fast_ln2(count-1);
	buf2 |= 1 << (zeroes + pos2);
	
	num = count - (1 << zeroes) - 1;
	
	//printf("count: %u num: %d zeroes: %u\n", count, num, zeroes);
	
	assert(num >= 0);
	
	buf2 |= num << (zeroes + pos2 + 1);
	
	pos2 += zeroes*2 + 1;
	
	//printf("  buf: %08X\n", buf2);
	
	assert(pos2 <= 32);
	
	bs_write_bit_le(out, buf2, pos2);
}

/**
 * Write char to bit buffer
 *
 * @param c: 8bit char to write
 * @param buf: pointer to bit buffer
 * @param buf_pos: pointer to bit buffer position (0 means empty buffer)
 *
 **/
void ds_out_chr(uint8_t c, bitstream_t *out)
{
	uint32_t buf2 = 0;
	
	if((c & 0x80) != 0)
	{
		buf2 = (c & 0x7F) << 2 | 0x1; // 0b01
	}
	else
	{
		buf2 = (c & 0x7F) << 2 | 0x2; // 0b10
	}
	
	bs_write_bit_le(out, buf2, 9);
}

/**
 * Compress memory block to bitstream using DS (DriveSpace/DoubleSpace) compression
 *
 * NOTE: THIS IS VERY SIMPLE IMPEMENTATION, compress radio is equvalent as produces RLE.
 *
 * @param block: source data
 * @param block_size: source size
 * @param out: output bitstream
 *
 **/
void ds_compress(void *block, size_t block_size, bitstream_t *out)
{
	uint8_t *ptr;
	size_t j, i, k;
	size_t segment_count = block_size/FS_SEGMENT_SIZE;
	if(block_size % FS_SEGMENT_SIZE != 0)
	{
		segment_count++;
	}
	
	for(i = 0; i < segment_count; i++)
	{
		size_t smax = block_size - (i*FS_SEGMENT_SIZE);
		if(smax > FS_SEGMENT_SIZE)
		{
			smax = FS_SEGMENT_SIZE;
		}
			
		ptr = ((uint8_t*)block)+(i*FS_SEGMENT_SIZE);
				
		for(j = 0; j < smax; j++)
		{
			uint8_t c = ptr[j];
			size_t repeat = 0;
			for(k = j+1; k < smax; k++,repeat++)
			{
				if(ptr[k] != c)
				{
					break;
				}
			}
			
			//printf("(%u) Write char: %02X, repeat: %d\n", j, c, repeat);	
			ds_out_chr(c, out);
			
			if(repeat >= 2)
			{
				//printf("(%u) %02X, repeat: %d\n", j, c, repeat);
				ds_out_pos(repeat, 1, out);
				j += repeat;
			}
		} // for j
		
		/* sector break */
		bs_write_bit_le(out, ((4415-320) << 3) | 0x7, 15);
		
	} // for i
	
	bs_write_flush(out);
}
