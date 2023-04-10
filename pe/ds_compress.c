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
#include "nocrt.h"

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
	
#ifdef DEBUG
	assert(count >= 2);
	assert(pos   < 4415);
#endif
	
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
	
#ifdef DEBUG
	assert(num >= 0);
#endif
	
	buf2 |= num << (zeroes + pos2 + 1);
	
	pos2 += zeroes*2 + 1;
	
	//printf("  buf: %08X\n", buf2);
	
#ifdef DEBUG
	assert(pos2 <= 32);
#endif
	
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
		buf2 = ((c & 0x7F) << 2) | 0x1; // 0b01
	}
	else
	{
		buf2 = ((c & 0x7F) << 2) | 0x2; // 0b10
	}
	
	bs_write_bit_le(out, buf2, 9);
}

/**
 * Find nearest and largest same string in block
 * NOTE: dummy and slow (O2 complexity)
 *
 **/
static int ds_find(uint8_t *block, size_t startpos, size_t endpos, size_t *dst_pos, size_t *dst_len)
{
	size_t max_len = endpos - startpos;
	
	if(max_len > startpos)
	{
		max_len = startpos;
	}
	
	//printf("max_len = %zu\n", max_len);
	
	while(max_len >= 2)
	{
		size_t pos = startpos-max_len;
		
		do
		{
			if(startpos - pos > 4414)
			{
				break;
			}
			
			if(memcmp(block+pos, block+startpos, max_len) == 0)
			{
				*dst_pos = pos;
				*dst_len = max_len;
				return 1;
			}
		} while(pos--);
		
		max_len--;
	}
	
	return 0;	
}

/**
 * Find nearest and largest same string in block
 * NOTE: still dummy (same bad complexity) but little faster
 *
 **/
static int ds_find_fast(uint8_t *block, size_t startpos, size_t endpos, size_t *dst_pos, size_t *dst_len)
{
	size_t max_len = endpos - startpos;
	
	if(max_len > startpos)
	{
		max_len = startpos;
	}
		
	while(max_len >= 2)
	{
		size_t pos = startpos-max_len;
		
		for(;;)
		{
			if(startpos - pos > 1024)
			{
				break;
			}
			
			if(memcmp(block+pos, block+startpos, max_len) == 0)
			{
				*dst_pos = pos;
				*dst_len = max_len;
				
				return 1;
			}
			
			if(pos > max_len)
			{
				pos -= max_len;
			}
			else
			{
				break;
			}
		}
		
		if(max_len > 32)
		{
			max_len /= 2;
		}
		else if(max_len > 10)
		{
			max_len -= 2;
		}
		else
		{
			max_len--;
		}
		
	}
	
	return 0;	
}

INLINE int ds_rle(uint8_t *block, size_t startpos, size_t endpos, size_t *dst_len)
{
	size_t pos = startpos;
	size_t len = 0;
	//printf("ds_rle(mem, %zu, %zu, dst_len);\n", startpos, endpos);
	
	while(pos < endpos)
	{
		if(block[pos] != block[startpos])
		{
			break;
		}
		
		len++;
		pos++;
	}
	
	if(len >= 3)
	{
		*dst_len = len;
		return 1;
	}
	
	return 0;
}

/**
 * Compress memory block to bitstream using DS (DriveSpace/DoubleSpace) compression
 *
 * NOTE: THIS IS VERY SIMPLE IMPEMENTATION, compress radio is somewere as produces RLE.
 *
 * @param block: source data
 * @param block_size: source size
 * @param out: output bitstream
 *
 **/
void ds_compress(void *block, size_t block_size, bitstream_t *out)
{
	uint8_t *ptr = block;
	size_t j, i;
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
						
		for(j = 0; j < smax; /*j++*/)
		{
			size_t act_pos = (i*FS_SEGMENT_SIZE) + j;
			size_t act_end = (i*FS_SEGMENT_SIZE) + smax;
			size_t find_pos = 0;
			size_t find_len = 0;
			size_t rle_len  = 0;
			
			if(ds_find_fast(ptr, act_pos, act_end, &find_pos, &find_len))
			{
				#ifdef HEAVY_DEBUG
				printf("ds_out_pos() = %zu %zu\n", find_pos, find_len);
				#endif
				ds_out_pos(find_len, act_pos-find_pos, out);
				j += find_len;
			}
			else if(ds_rle(ptr, act_pos, act_end, &rle_len))
			{
				#ifdef HEAVY_DEBUG
				printf("ds_rle() = %zu\n", rle_len);
				#endif
				ds_out_chr(ptr[act_pos], out);
				ds_out_pos(rle_len-1, 1, out);
				j += rle_len;
			}
			else
			{
				ds_out_chr(ptr[act_pos], out);
				j++;
			}
		} // for j
		
		/* sector break */
		bs_write_bit_le(out, ((4415-320) << 3) | 0x7, 15);
	} // for i

	/* in original files there's no terminate blocks, but VXDLIB add it,
	   so here is for safety
	 */
	bs_write_bit_le(out, 0, 8);
	
	bs_write_flush_le(out);
}
