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
#ifndef __BITSTREAM_H__INCLUDED__
#define __BITSTREAM_H__INCLUDED__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cextra.h>

#ifdef NOCRT_FILE
#include "nocrt.h"
#endif

/* stream types */
#define BITSTREAM_MEM  0
#define BITSTREAM_FILE 1

/**
 * defining BS_LSB_FIRST and BS_MSB_FIRST which way output bits from memory
 * 
 * Example sequence:       0x12       0x34       0x56       0x78       0x9A
                     0b00010010 0b00110100 0b01010110 0b01111000 0b10011010
 *
 * if defined BS_MSB_FIRST:
 *   bs_read_bit(..., 12) == 0x123 (0b000100100011)
 *
 * if defined BS_LSB_FIRST:
 *   bs_read_bit(..., 12) == 0x482 (0b010010000010)
 *
 * result of bs_read_bit_le not depend on these defines:
 *   bs_read_bit_le(..., 12) == 0x412 (0b000110011100)
 *
 **/
#ifndef BS_LSB_FIRST
#define BS_MSB_FIRST
#endif

#if defined(BS_LSB_FIRST) && defined(BS_MSB_FIRST)
#error "please define BS_LSB_FIRST xor BS_MSB_FIRST"
#endif

typedef struct _bitstream_t
{
	int type; /* BITSTREAM_MEM or BITSTREAM_FILE */
	union
	{
		struct _bs_file
		{
			FILE   *fp;   /* file pointer from fopen */
			uint8_t byte; /* one byte buffer */
			int     bpos; /* number of bits that are in buffer */
		} bs_file;
		struct _bs_mem
		{
			uint8_t *mem; /* buffer */
			size_t  size; /* size of buffer */
			size_t  pos;  /* position in buffer */
			uint8_t byte; /* one byte extra buffer */
			int     bpos; /* number of bits that are in extra buffer */
		} bs_mem;
	};
} bitstream_t;

/**
 * Init bitstream structure to stream from/to file
 *
 * @param bs: unused and allocated structure
 * @param fp: pointer retuned by fopen/freopen function
 *
 **/
INLINE void bs_file(bitstream_t *bs, FILE *fp)
{
	memset(bs, 0, sizeof(bitstream_t));
	bs->type = BITSTREAM_FILE;
	bs->bs_file.fp = fp;
}

/**
 * Init bitstream structure to stream from/to memory
 *
 * @param bs: unused and allocated structure
 * @param mem: allocated memory buffer
 * @param size: size of memory
 *
 **/
INLINE void bs_mem(bitstream_t *bs, void *mem, size_t size)
{
	memset(bs, 0, sizeof(bitstream_t));
	bs->type = BITSTREAM_MEM;
	bs->bs_mem.mem = (uint8_t*)mem;
	bs->bs_mem.size = size;
}

/**
 * Init bitstream structure to stream from/to memory with memory
 * allocation.
 *
 * @param bs: unused and allocated structure
 * @param size: size of memory
 * @return: pointer to new memory on NULL on allocation failure
 *
 **/
INLINE void *bs_mem_alloc(bitstream_t *bs, size_t size)
{
	memset(bs, 0, sizeof(bitstream_t));
	bs->type = BITSTREAM_MEM;
	bs->bs_mem.mem = (uint8_t*)malloc(size);
	if(bs->bs_mem.mem != NULL)
	{
		bs->bs_mem.size = size;
	}
	return bs->bs_mem.mem;
}

/**
 * Free memory which allocate bs_mem_free function
 * 
 * @param bs: stream to cleanup
 *
 **/
INLINE void bs_mem_free(bitstream_t *bs)
{
	if(bs->type == BITSTREAM_MEM)
	{
		if(bs->bs_mem.mem != NULL)
		{
			free(bs->bs_mem.mem);
			bs->bs_mem.mem = NULL;
			bs->bs_mem.size = 0;
		}
	}
}

/**
 * Clean bitfield memory (if bitstream type is BITSTREAM_MEM)
 * (set all bits to zero)
 *
 * @param bs: bit stream
 *
 **/
INLINE void bs_mem_zero(bitstream_t *bs)
{
	if(bs->type == BITSTREAM_MEM)
	{
		memset(bs->bs_mem.mem, 0, bs->bs_mem.size);
	}
}

/**
 * Calculate memory size in bytes which need bitfield specifed in bits
 *
 * @param bitsize: size on bits
 * @return: size in bytes
 *
 **/
INLINE size_t bs_calc_size(size_t bitsize)
{
	size_t bytes = bitsize/8;
	if(bytes % 8 != 0)
	{
		bytes++;
	}
	return bytes;	
}

/**
 * Reset buffer in stream
 * If stream is memory, reset position to begin of buffer.
 * If stream is file, with file position IS NOT manipulated! If you want
 * move to begin of the file, call rewind(bs->bs_file.fp) after called
 * bs_reset.
 * 
 * @param bs: stream to reset
 *
 **/
INLINE void bs_reset(bitstream_t *bs)
{
	if(bs->type == BITSTREAM_MEM)
	{
		bs->bs_mem.pos  = 0;
		bs->bs_mem.byte = 0;
		bs->bs_mem.bpos = 0;
	}
	else if(bs->type == BITSTREAM_FILE)
	{
		bs->bs_file.byte = 0;
		bs->bs_file.bpos = 0;
	}
}

/**
 * Read number of bits from stream
 *
 * @param bs: bit stream
 * @param cnt: number of bytes to read (0-32)
 *
 * @return: readed bits in 32bit buffer
 *
 **/
INLINE uint32_t bs_read_bit(bitstream_t *bs, int cnt)
{
	uint32_t b = 0; 
	if(bs->type == BITSTREAM_MEM)
	{
		while(cnt--)
		{
			if(bs->bs_mem.bpos == 0)
			{
				if(bs->bs_mem.pos < bs->bs_mem.size)
				{
					bs->bs_mem.byte = bs->bs_mem.mem[bs->bs_mem.pos++];
				}
				else
				{
					bs->bs_mem.byte = 0;
				}
				bs->bs_mem.bpos = 8;
			}
	
#ifdef BS_MSB_FIRST	
			b <<= 1;
			b |= (bs->bs_mem.byte >> 7) & 0x1;
			bs->bs_mem.byte <<= 1;
#endif

#ifdef BS_LSB_FIRST
			b |= ((bs->bs_mem.byte) & 0x1) << cnt;
			bs->bs_mem.byte >>= 1;
#endif

			bs->bs_mem.bpos--;
		}
	}
	else if(bs->type == BITSTREAM_FILE)
	{
		while(cnt--)
		{
			if(bs->bs_file.bpos == 0)
			{
				bs->bs_file.byte = fgetc(bs->bs_file.fp);
				bs->bs_file.bpos = 8;
			}

#ifdef BS_MSB_FIRST			
			b <<= 1;
			b |= (bs->bs_file.byte >> 7) & 0x1;
			bs->bs_file.byte <<= 1;
#endif

#ifdef BS_LSB_FIRST
			b |= ((bs->bs_file.byte) & 0x1) << cnt;
			bs->bs_file.byte >>= 1;
#endif

			bs->bs_file.bpos--;
		}
	}
	
	return b;
}

/**
 * Read number of bits from stream, read in little endian order.
 * 
 * @param bs: bit stream
 * @param cnt: number of bytes to read (0-32)
 *
 * @return: readed bits in 32bit buffer
 *
 **/
INLINE uint32_t bs_read_bit_le(bitstream_t *bs, int cnt)
{
	uint32_t b = 0;
	int by, bt;
	
	if(bs->type == BITSTREAM_MEM)
	{
		for(by = 0; by < 4 && cnt > 0; by++)
		{
			for(bt = 0; bt < 8 && cnt > 0; bt++)
			{
				if(bs->bs_mem.bpos == 0)
				{
					if(bs->bs_mem.pos < bs->bs_mem.size)
					{
						bs->bs_mem.byte = bs->bs_mem.mem[bs->bs_mem.pos++];
					}
					else
					{
						bs->bs_mem.byte = 0;
					}
					bs->bs_mem.bpos = 8;
				}
				
				b |= (((uint32_t)bs->bs_file.byte & 0x1) << bt) << (by * 8);
				bs->bs_file.byte >>= 1;
				bs->bs_file.bpos--;
					
				cnt--;
			}
		}
	}
	else if(bs->type == BITSTREAM_FILE)
	{
		for(by = 0; by < 4 && cnt > 0; by++)
		{
			for(bt = 0; bt < 8 && cnt > 0; bt++)
			{
				if(bs->bs_file.bpos == 0)
				{
					int c = fgetc(bs->bs_file.fp);
					if(c != EOF)
					{
						bs->bs_file.byte = c;
					}
					else
					{
						bs->bs_file.byte = 0;
					}
					bs->bs_file.bpos = 8;
				}
				
				b |= (((uint32_t)bs->bs_file.byte & 0x1) << bt) << (by * 8);
				bs->bs_file.byte >>= 1;
				bs->bs_file.bpos--;
					
				cnt--;
			}
		}
	}
	
	return b;
}

/**
 * Write numner of bits in buffer to stream
 * 
 * @param bs: bit stream
 * @param b: buffer
 * @param cnt: number of bytes to write (0-32)
 *
 **/
INLINE void bs_write_bit(bitstream_t *bs, uint32_t b, int cnt)
{
	if(bs->type == BITSTREAM_MEM)
	{
		while(cnt--)
		{
#if (defined(BS_MSB_FIRST) && !defined(BS_REVERSE_WRITE)) || (defined(BS_LSB_FIRST) && defined(BS_REVERSE_WRITE))
			bs->bs_mem.bpos++;
			bs->bs_mem.byte |= ((b >> cnt) & 0x1) << (8 - bs->bs_mem.bpos);
#endif
			
#if (defined(BS_LSB_FIRST) && !defined(BS_REVERSE_WRITE)) || (defined(BS_MSB_FIRST) && defined(BS_REVERSE_WRITE))
			bs->bs_mem.byte >>= 1;
			bs->bs_mem.byte |= ((b >> cnt) & 0x1) << 7;
			bs->bs_mem.bpos++;
#endif

			if(bs->bs_mem.bpos == 8)
			{
				if(bs->bs_mem.pos < bs->bs_mem.size)
				{
					bs->bs_mem.mem[bs->bs_mem.pos++] = bs->bs_mem.byte;
				}
				bs->bs_mem.bpos = 0;
				bs->bs_mem.byte = 0;
			}
		}
	}
	else if(bs->type == BITSTREAM_FILE)
	{
		while(cnt--)
		{
#ifdef BS_MSB_FIRST
			bs->bs_file.bpos++;
			bs->bs_file.byte |= ((b >> cnt) & 0x1) << (8 - bs->bs_file.bpos);
#endif

#ifdef BS_LSB_FIRST
			bs->bs_file.byte >>= 1;
			bs->bs_file.byte |= ((b >> cnt) & 0x1) << 7;
			bs->bs_file.bpos++;
#endif

			if(bs->bs_file.bpos == 8)
			{
				fputc(bs->bs_file.byte, bs->bs_file.fp);
				bs->bs_file.bpos = 0;
				bs->bs_file.byte = 0;
			}
		}
	}
}

/**
 * Write numner of bits in buffer to stream, write in little endian order.
 * 
 * @param bs: bit stream
 * @param b: buffer
 * @param cnt: number of bytes to write (0-32)
 *
 **/
INLINE void bs_write_bit_le(bitstream_t *bs, uint32_t b, int cnt)
{
	if(bs->type == BITSTREAM_MEM)
	{
		while(cnt--)
		{
			bs->bs_mem.byte >>= 1;
			bs->bs_mem.byte |= (b & 0x1) << 7;
			b >>= 1;
			bs->bs_mem.bpos++;
			
			if(bs->bs_mem.bpos == 8)
			{
				if(bs->bs_mem.pos < bs->bs_mem.size)
				{
					bs->bs_mem.mem[bs->bs_mem.pos++] = bs->bs_mem.byte;
				}
				bs->bs_mem.bpos = 0;
				bs->bs_mem.byte = 0;
			}
		}
	}
	else if(bs->type == BITSTREAM_FILE)
	{
		while(cnt--)
		{			
			bs->bs_file.byte >>= 1;
			bs->bs_file.byte |= (b & 0x1) << 7;
			b >>= 1;
			bs->bs_file.bpos++;
			
			if(bs->bs_file.bpos == 8)
			{
				fputc(bs->bs_file.byte, bs->bs_file.fp);
				bs->bs_file.bpos = 0;
				bs->bs_file.byte = 0;
			}
			
		}
	}
}

/**
 * Return number of bits, thats in internal cache and
 * still needs be writen in stream
 *
 * @param bs: bit stream
 *
 **/
INLINE int bs_cnt_tmp_bits(bitstream_t *bs)
{
	if(bs->type == BITSTREAM_MEM)
	{
		return bs->bs_mem.bpos;
	}
	else if(bs->type == BITSTREAM_FILE)
	{
		return bs->bs_file.bpos;
	}
	return 0;
}

/**
 * Write final byte to stream. Unused bits are padded with 0
 *
 * @param bs: bit stream
 *
 **/
INLINE void bs_write_flush(bitstream_t *bs)
{
	uint32_t zero = 0;
	int waiting  = bs_cnt_tmp_bits(bs);
	if(waiting != 0)
	{
		bs_write_bit(bs, zero, 8 - waiting);
	}
}

/**
 * Write final byte to stream. Unused bits are padded with 0
 *
 * @param bs: bit stream
 *
 **/
INLINE void bs_write_flush_le(bitstream_t *bs)
{
	uint32_t zero = 0;
	int waiting  = bs_cnt_tmp_bits(bs);
	if(waiting != 0)
	{
		bs_write_bit_le(bs, zero, 8 - waiting);
	}
}

/* logical function for bs_logic */
#define BS_NOT  0
#define BS_AND  1
#define BS_OR   2
#define BS_XOR  3
#define BS_NAND 4
#define BS_NOR  5
#define BS_XNOR 6

/**
 * Do logic function between streams
 *
 * @param func: type of operation listed above
 * @param a: first stream
 * @param b: second stream (could by NULL on unary operations like BS_NOT)
 * @param y: result
 * @param bitsize: number of bits to compare 
 *
 **/
INLINE void bs_logic(int func, bitstream_t *a, bitstream_t *b, bitstream_t *y, size_t bitsize)
{
	uint32_t buf_y;
	size_t to_read;
	ssize_t i;
	
	for(i = bitsize; i >= 0; i -= 32)
	{
		to_read = i;
		if(to_read > 32)
		{
			to_read = 32;
		}
		else if(to_read % 8 != 0)
		{
			to_read += to_read % 8; /* pad operation to whole byte */
		}
			
		switch(func)
		{
			case BS_NOT:
				buf_y = ~bs_read_bit(a, to_read);
				break;
			case BS_AND:
				buf_y = bs_read_bit(a, to_read) & bs_read_bit(b, to_read);
				break;
			case BS_OR:
				buf_y = bs_read_bit(a, to_read) | bs_read_bit(b, to_read);
				break;
			case BS_XOR:
				buf_y = bs_read_bit(a, to_read) ^ bs_read_bit(b, to_read);
				break;
			case BS_NAND:
				buf_y = ~(bs_read_bit(a, to_read) & bs_read_bit(b, to_read));
				break;
			case BS_NOR:
				buf_y = ~(bs_read_bit(a, to_read) | bs_read_bit(b, to_read));
				break;
			case BS_XNOR:
				buf_y = ~(bs_read_bit(a, to_read) ^ bs_read_bit(b, to_read));
				break;
			default:
				return;
		}
		
		bs_write_bit(y, buf_y, to_read);
	}
}

/**
 * Test if all bits in stream are zeroes
 *
 * @param bs: bit stream
 * @param bitsize: number of bits to check 
 *
 **/
INLINE int bs_is_zero(bitstream_t *bs, size_t bitsize)
{
	uint32_t buf_test;
	size_t to_read;
	ssize_t i;
	
	for(i = bitsize; i >= 0; i -= 32)
	{
		to_read = i;
		if(to_read > 32)
		{
			to_read = 32;
		}
		else if(to_read % 8 != 0) /* pad operation to whole byte */
		{
			to_read += to_read % 8;
		}
		
		buf_test = bs_read_bit(bs, to_read);
		if(buf_test != 0)
		{
			return 0;
		}
	}
	
	return 1;
}

/**
 * Swap byte order from MSB to LSB and vice versa
 *
 * @param byte: 32bit word
 * @param size: size of word in bits
 *
 **/
INLINE uint32_t bs_bitswap(uint32_t byte, int size)
{
	uint32_t out = 0;
	int i = 0;
	
	for(i = 0; i < size; i++)
	{
		out <<= 1;
		out |= byte & 0x1;
		byte >>= 1;
	}
	
	return out;
}

#endif /* __BITSTREAM_H__INCLUDED__ */
