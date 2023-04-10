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
#include "doublespace.h"
#include "nocrt.h"

/**
 * Read block "count"
 * @param ptr_buf: pointer to bit buffer
 * @param ptr_buf_len: pointer to number of bits in buffer
 *
 * @return: on succes number between 2-4414; on failure 0
 *
 **/
static size_t ds_count(uint32_t *ptr_buf, size_t *ptr_buf_len)
{
	uint32_t buf = *ptr_buf;
	uint32_t mask = 0;
	size_t zeroes = 0;
	size_t sz = 0;
	
	for(zeroes = 0; zeroes < 9; zeroes++)
	{
		if((buf & 0x1) != 0)
		{
			break;
		}
		buf >>= 1;
		mask = (mask << 1) | 0x1;
	}
	
	if(zeroes == 9)
	{
		/* bad block */
		return 0;
	}
	buf >>= 1;
	
	sz = (1 << zeroes) + (buf & mask) + 1;
	buf >>= zeroes;
	
	*ptr_buf_len = (*ptr_buf_len) - (1 + zeroes*2);	
	*ptr_buf     = buf;
	#ifdef HEAVY_DEBUG
	printf("DS count %u\n", sz);
	#endif
	
	return sz;
}

/**
 * Decompress DS (DriveSpace/DoubleSpace) compression in bitstream to memory block
 *
 * @param in: input bitstream
 * @param block: destination memory for recompressed data
 * @param block_size: destionation size
 * 
 * @return: number of bytes writen to 'block', 0 on failure
 *
 **/
size_t ds_decompress(bitstream_t *in, void *block, size_t block_size)
{
	uint8_t  *ptr = (uint8_t*)block;
	size_t    ptr_pos;
	size_t    copy_pos = 0;
	size_t    copy_size = 0;
	uint32_t  buf = 0;
	size_t    buf_len = 0;
	
	for(ptr_pos = 0; ptr_pos < block_size;)
	{
		if(buf_len < 32)
		{
			buf     |= bs_read_bit_le(in, 32-buf_len) << buf_len;
			buf_len = 32;
		}
		//printf("buffer 0x%08X\n", buf);
		
		switch(buf & 0x3)
		{
			case 0x1: // 0b01
				ptr[ptr_pos++] = 0x80 | ((buf >> 2) & 0x7F);
				buf    >>= 9;
				buf_len -= 9;
				continue;
				break;
			case 0x2: // 0b10
				ptr[ptr_pos++] = (buf >> 2) & 0x7F;
				buf    >>= 9;
				buf_len -= 9;
				continue;
				break;
			case 0x0: // 0b00
				copy_pos = (buf >> 2) & 0x3F;
				buf    >>= 8;
				buf_len -= 8;
				
				if(copy_pos)
				{
					copy_size = ds_count(&buf, &buf_len);
				}
				else
				{
					/* end block */
					copy_size = 0;
				}
				break;
			case 0x3: // 0b11
				switch(buf & 0x7)
				{
					case 3: // 0b011:
						copy_pos = ((buf >> 3) & 0xFF) + 64;
						buf    >>= 11;
						buf_len -= 11;
						
						copy_size = ds_count(&buf, &buf_len);
						break;
					case 7: // 0b111:
						copy_pos = ((buf >> 3) & 0xFFF) + 320;
						buf    >>= 15;
						buf_len -= 15;
						
						if(copy_pos == 4415)
						{
							/* sector break */
							#ifdef HEAVY_DEBUG
							printf("sector break: %d\n", ptr_pos);
							#endif
							continue;
						}
						
						copy_size = ds_count(&buf, &buf_len);
						break;
				}
				break;
		}
		
		if(copy_pos == 0)
		{
			return ptr_pos; /* reach end block */
		}
		
		if(copy_size == 0)
		{
			return 0; /* invalid block */
		}
		
		if(copy_pos > ptr_pos)
		{
			return 0; /* underflow */
		}
		
		if((ptr_pos + copy_size) <= block_size)
		{
			while(copy_size--)
			{
				ptr[ptr_pos] = ptr[ptr_pos-copy_pos];
				ptr_pos++;
			}
		}
		else
		{
			/* overflow */
			return ptr_pos;
		}
	}
	
	/* bufer full, done */
	return ptr_pos;
}

