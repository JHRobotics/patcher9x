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
#include "pew.h"

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

/* maximum distance for DS compressed string */
#define MAX_DEPTH 4414

/* length of test block */
#define BS_TEST_COUNT 11 // + 4
static const size_t bs_tries[BS_TEST_COUNT+1] = {/*512, 256, 128, 64, */40, 28, 22, 18, 14, 10, 8, 6, 4, 3, 2, 0};

#if 0
/**
 * Find nearest and largest same string in block.
 * NOTE: dummy and slow (O2 complexity), usable for reference only
 *
 **/
static int ds_find(uint8_t *block, size_t startpos, size_t endpos, size_t *dst_pos, size_t *dst_len, size_t max_depth)
{
	const size_t test_max = endpos - startpos;
	const size_t *max_len;

	if(max_depth == 0) return 0;

	if(max_depth > MAX_DEPTH)
	{
		max_depth = MAX_DEPTH;
	}

	for(max_len = bs_tries; (*max_len) != 0; max_len++)
	{
		if((*max_len) > startpos || (*max_len) > test_max)
		{
			continue;
		}

		//size_t pos = startpos - 1;
		ssize_t pos = startpos - (*max_len);
		do
		{
			if(startpos - pos > max_depth)
			{
				break;
			}
			
			if(memcmp(block+pos, block+startpos, *max_len) == 0)
			{
				*dst_pos = pos;
				*dst_len = *max_len;
				
				return 1;
			}
			
			pos--;
		} while(pos >= 0);
	}
	
	return 0;	
}
#endif

/* DS hash table structures */
#define HT_LIST_MAX 31
#define HT_PRIME 1777

#ifdef __DJGPP__
/* tiny align to save some memory */
#pragma pack(push)
#pragma pack(1)
#endif

typedef struct ds_ht_item
{
	uint16_t cnt;
	uint16_t pos[HT_LIST_MAX];
} ds_ht_item_t;

#ifdef __DJGPP__
#pragma pack(pop)
#endif

typedef struct ds_ht_ts
{
	ds_ht_item_t items[HT_PRIME];
} ds_ht_ts_t;

typedef struct ds_ht
{
	ds_ht_ts_t tables[BS_TEST_COUNT];
} ds_ht_t;


/**
 * Calculate hast table index from string.
 * Based on djb2 function (http://www.cse.yorku.ca/~oz/hash.html).
 * 
 * @param buf: memory buffer
 * @param size: memory size
 *
 * @return: hash
 *
*/
INLINE size_t ht_ds_hash(const uint8_t *buf, size_t size)
{
	uint32_t hash = 5381;
	while(size)
	{
		hash = ((hash << 5) + hash) + (*buf); /* hash * 33 + c */
		buf++;
		size--;
	}
	
	return hash % HT_PRIME;
}

/**
 * Create and init empty hash table for string lookup
 *
 * @return: hash table on success allocation or NULL
 *
 **/
static ds_ht_t *ht_ds_alloc()
{
#if 1
	ds_ht_t *ht = malloc(sizeof(ds_ht_t));
	if(ht != NULL)
	{
		int i, j;
		for(i = 0; i < BS_TEST_COUNT; i++)
		{
			for(j = 0; j < HT_PRIME; j++)
			{
				ht->tables[i].items[j].cnt = 0;
			}
		}
	}
	
	return ht;
#else
	return NULL;
#endif
}

/**
 * Free the hash table
 *
 * @param ht: pointer to ht_ds_alloc result
 *
 **/
static void ht_ds_free(ds_ht_t **ht)
{
	if((*ht) != NULL)
	{
		free(*ht);
		*ht = NULL;
	}
}

/**
 * Update the hash table
 *
 * @param ht: hash table allocated by ht_ds_alloc
 * @param block: memory block
 * @param start: index to start updating (on fresh block start=0)
 * @param cb_updates: number of bytes tu update
 *
 */
static void ds_ht_update(ds_ht_t *ht, uint8_t *block, size_t start, size_t cb_updates)
{
	size_t i, cnt;

	for(cnt = 1; cnt <= cb_updates; cnt++)
	{
		for(i = 0; i < BS_TEST_COUNT; i++)
		{
			const size_t s = bs_tries[i];
			if(start + cnt >= s)
			{
				int found = 0;
				const size_t pos = start + cnt - s;
				size_t h = ht_ds_hash(block + pos, s);
				size_t m = ht->tables[i].items[h].cnt;
				size_t p;

				if(m >= HT_LIST_MAX)
					m -= HT_LIST_MAX;
				else
					m = 0;
				
				for(p = m; p < ht->tables[i].items[h].cnt; p++)
				{
					size_t pi = p % HT_LIST_MAX;
					if(memcmp(block+pos, block+ht->tables[i].items[h].pos[pi], s) == 0)
					{
						ht->tables[i].items[h].pos[pi] = pos;
						found = 1;
						break;
					}
				}
				
				if(!found)
				{
					p = ht->tables[i].items[h].cnt;
					ht->tables[i].items[h].pos[p % HT_LIST_MAX] = pos;
					ht->tables[i].items[h].cnt++;
#if 0
					if(p >= HT_LIST_MAX)
					{
						printf("overlist: size=%d, p=%d (pos=%d)\n", s, p, pos);
					}
#endif
				}
			}
		}
	}
}

/**
 * Lookup for repeat string on memory block
 *
 * @param ht: hash table allocated by ht_ds_alloc and updated by ds_ht_update
 * @param block: memory block
 * @param startpos: beginning position of string to lookup
 * @param endpos: boundary of string length
 * @param dst_pos: when success result index on block
 * @param dst_len: when success result string length
 *
 * @return: 1 when lookup is success
 **/
static int ds_lookup(ds_ht_t *ht, uint8_t *block, size_t startpos, size_t endpos, size_t *dst_pos, size_t *dst_len)
{
	const size_t test_max = endpos - startpos;
	size_t index;
	
	for(index = 0; index < BS_TEST_COUNT; index++)
	{
		const size_t s = bs_tries[index];
		if(startpos >= s && test_max >= s)
		{
			size_t h = ht_ds_hash(block + startpos, s);
			size_t c = ht->tables[index].items[h].cnt;
			size_t m;
			
			for(m = 1; m <= HT_LIST_MAX && m <= c; m++)
			{
				const size_t p = (c-m) % HT_LIST_MAX;
				const size_t pos = ht->tables[index].items[h].pos[p];
				
				/* position out of reach */
				if(startpos - pos > MAX_DEPTH) continue;
				
				if(memcmp(block+pos, block+startpos, s) == 0)
				{
					*dst_pos = pos;
					*dst_len = s;
					return 1;
				}
			}
		}
	}
	return 0;
}

/**
 * Check when memory is compressable RLE-like mehod. 
 *
 * @param block: memory block
 * @param startpos: start index
 * @param endpos: maximum index (block boundaries)
 * @param dst_len: result lenth (number of byte repeat)
 *
 * @param: 1 when effective
 *
 **/
INLINE int ds_rle(uint8_t *block, size_t startpos, size_t endpos, size_t *dst_len)
{
	size_t pos = startpos;
	size_t len = 0;

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
void ds_compress(void *block, size_t block_size, bitstream_t *out, int rle_only)
{
	uint8_t *ptr = block;
	size_t j, i;
	size_t segment_count = block_size/FS_SEGMENT_SIZE;
	ds_ht_t *ht = NULL;
	
	if(block_size % FS_SEGMENT_SIZE != 0)
	{
		segment_count++;
	}
	
	if(!rle_only)
	{
		ht = ht_ds_alloc();
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

			if(ht && ds_lookup(ht, ptr, act_pos, act_end, &find_pos, &find_len))
			{
				#ifdef HEAVY_DEBUG
				printf("ds_out_pos() = %zu %zu\n", find_pos, find_len);
				#endif
				
				/* RLE could be sometimes better */
				if(ds_rle(ptr, act_pos, act_end, &rle_len))
				{
					if(rle_len > find_len)
					{
						ds_out_chr(ptr[act_pos], out);
						ds_out_pos(rle_len-1, 1, out);
						j += rle_len;
						ds_ht_update(ht, ptr, act_pos, rle_len);
						continue;
					}
				}
				
				ds_out_pos(find_len, act_pos-find_pos, out);
				j += find_len;
				ds_ht_update(ht, ptr, act_pos, find_len);
			}
			else if(ds_rle(ptr, act_pos, act_end, &rle_len))
			{
				#ifdef HEAVY_DEBUG
				printf("ds_rle() = %zu\n", rle_len);
				#endif
				ds_out_chr(ptr[act_pos], out);
				ds_out_pos(rle_len-1, 1, out);
				j += rle_len;

				if(ht)
				{
					ds_ht_update(ht, ptr, act_pos, rle_len);
				}
			}
			else
			{
				ds_out_chr(ptr[act_pos], out);
				j++;

				if(ht)
				{
					ds_ht_update(ht, ptr, act_pos, 1);
				}
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
	
	ht_ds_free(&ht);
	
}
