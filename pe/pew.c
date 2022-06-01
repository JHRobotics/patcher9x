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
#include <bitstream.h>
#include <extstring.h>
#include <filesystem.h>
#include "pew.h"
#include "doublespace.h"

/* header of LE file  */
static const uint8_t dos_program_le[] = 
{
	0x4D, 0x5A, 0x80, 0x00, 0x1F, 0x03, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, // MZ€.........ÿÿ..
	0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ¸.......@.......
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ................
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, // ............€...
	0x0E, 0x1F, 0xBA, 0x0E, 0x00, 0xB4, 0x09, 0xCD, 0x21, 0xB8, 0x01, 0x4C, 0xCD, 0x21, 0x54, 0x68, // ..º..´.Í!¸.LÍ!Th
	0x69, 0x73, 0x20, 0x70, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x20, 0x63, 0x61, 0x6E, 0x6E, 0x6F, // is program canno
	0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6E, 0x20, 0x69, 0x6E, 0x20, 0x44, 0x4F, 0x53, 0x20, // t be run in DOS 
	0x6D, 0x6F, 0x64, 0x65, 0x2E, 0x0D, 0x0A, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mode...$........
};

int pe_read(dos_header_t *dos, pe_header_t *pe, FILE *fp)
{
	memset(dos, 0, sizeof(dos_header_t));
	memset(pe,  0, sizeof(pe_header_t));
	
	if(fread(dos, sizeof(dos_header_t), 1, fp) != 1)
	{
		return PE_ERROR_FREAD;
	}
	
	if(memcmp(dos->magic, MAGIC_DOS, 2) != 0)
	{
		//printf("%02X%02X\n", dos->magic[0], dos->magic[1]);
		
		if(memcmp(dos->magic, MAGIC_MSCAB, 4) == 0)
		{
			return PE_NO_IS_MSCAB;
		}
		
		return PE_NO_MZ_FILE;
	}
	
	fseek(fp, dos->nextheader, SEEK_SET);
	
	fread(pe, sizeof(pe_header_t), 1, fp);
	
	if(memcmp(pe->magic, MAGIC_W3, 2) == 0)
	{
		return PE_W3;
	}
	
	if(memcmp(pe->magic, MAGIC_W4, 2) == 0)
	{
		return PE_W4;
	}
	
	if(memcmp(pe->magic, MAGIC_LE, 2) == 0)
	{
		return PE_LE;
	}
	
	return PE_UNKNOWN;
}

pe_w4_t *pe_w4_alloc(size_t data_size)
{
	pe_w4_t *w4;
	size_t chunk_count = data_size / PE_W4_CHUNKSIZE;
	size_t mem_size;
	
	/* round up */
	if(data_size % PE_W4_CHUNKSIZE > 0)
	{
		chunk_count++;
	}
	
	mem_size = sizeof(pe_w4_t) + sizeof(uint32_t)*(chunk_count + 1);
	
	w4 = (pe_w4_t*)malloc(mem_size);
	if(w4 == NULL)
	{
		return NULL;
	}
	
	memset(w4, 0, mem_size);
	
	w4->chunks_cnt = chunk_count;
	w4->pe = NULL;
	w4->pe = 0;
	w4->fp = NULL;
	
	return w4;	
}

pe_w4_t *pe_w4_read(dos_header_t *dos, pe_header_t *pe, FILE *fp)
{
	pe_w4_t *w4;
	
	if(memcmp(pe->magic, MAGIC_W4, 2) != 0)
	{
		return NULL;
	}
	
	w4 = (pe_w4_t*)malloc(sizeof(pe_w4_t) + sizeof(uint32_t)*(pe->w4.chunk_count + 1));
	w4->pe = pe;
	w4->chunks_cnt = pe->w4.chunk_count;
	w4->pe_pos = dos->nextheader;
	w4->fp = fp;
	
	if(w4 != NULL)
	{
		fread(&(w4->chunks[0]), pe->w4.chunk_count, sizeof(uint32_t), fp);
	}
	
	/* determinate last segment size by end of file */
	fseek(fp, 0, SEEK_END);
	w4->chunks[pe->w4.chunk_count] = ftell(fp);
	
	return w4;
}

void pe_w4_free(pe_w4_t *w4)
{
	if(w4 != NULL)
	{
		free(w4);
	}
}

pe_w3_t *pe_w3_read(dos_header_t *dos, pe_header_t *pe, FILE *fp)
{
	pe_w3_t *w3;
	
	if(memcmp(pe->magic, MAGIC_W3, 2) != 0)
	{
		return NULL;
	}
	
	w3 = (pe_w3_t*)malloc(sizeof(pe_w3_t) + sizeof(pe_w3_file_t)*pe->w3.vxd_count);
	if(w3 != NULL)
	{
		w3->pe = pe;
		w3->files_cnt = pe->w3.vxd_count;
		w3->pe_pos = dos->nextheader;
		w3->fp = fp;
	
		/* read files list */
	  fseek(fp, w3->pe_pos + sizeof(pe_header_t), SEEK_SET);
		fread(&(w3->files[0]), sizeof(pe_w3_file_t), pe->w3.vxd_count, fp);
	}
	
	fseek(fp, 0, SEEK_END);
	w3->file_size = ftell(fp);
	
	return w3;
}

void pe_w3_free(pe_w3_t *w3)
{
	if(w3 != NULL)
	{
		free(w3);
	}
}

/**
 * Decompress W4 chunk
 *
 **/
size_t pe_w4_decompress(pe_w4_t *w4, void *buf, size_t chunk_id)
{
	bitstream_t in;
	size_t size = 0;
	
	if(chunk_id >= w4->chunks_cnt)
	{
		return 0;
	}

	if(fseek(w4->fp, w4->chunks[chunk_id], SEEK_SET) == 0)
	{
		bs_file(&in, w4->fp);
		size = ds_decompress(&in, buf, w4->pe->w4.chunk_size);
	}
	
	return size;
}

/**
 * Decompress W4 file and save as W3 file 
 *
 **/
int pe_w4_to_w3(pe_w4_t *w4, const char *dst)
{
	void *buf;
	FILE *fw = fopen(dst, "wb");
	size_t i;
	size_t s;
	size_t buf_size = w4->pe_pos > w4->pe->w4.chunk_size ? w4->pe_pos : w4->pe->w4.chunk_size;
	
	//printf("here\n");
	
	if(fw != NULL)
	{
		buf = malloc(buf_size);
		if(buf != NULL)
		{
			fseek(w4->fp, 0, SEEK_SET);
			fread(buf,  w4->pe_pos, 1, w4->fp);
			fwrite(buf, w4->pe_pos, 1, fw);
			
			for(i = 0; i < w4->pe->w4.chunk_count; i++)
			{
				s = pe_w4_decompress(w4, buf, i);
				//printf("BLOCK: %d %d\n", i, s);
				if(s != 0)
				{
					fwrite(buf, s, 1, fw);
				}
			}
			
			free(buf);
		}
		else
		{
			return PE_ERROR_MALLOC;
		}
		
		fclose(fw);
	}
	else
	{
		return PE_ERROR_FOPEN;
	}
	
	return PE_OK;
}

/**
 * Compress W3 with DS compression ad save as W4 file
 *
 **/
int pe_w3_to_w4(pe_w3_t *w3, const char *dst)
{
	void *buf;
	pe_header_t hw4;
	pe_w4_t *w4;
	size_t w3_fs    = 0;
	FILE *fw;
	bitstream_t bsw4;
	
	fseek(w3->fp, 0, SEEK_END);
	w3_fs = ftell(w3->fp);
	
	w4 = pe_w4_alloc(w3_fs - w3->pe_pos);
	if(w4)
	{
		memset(&hw4, 0, sizeof(pe_header_t));
		
		hw4.magic[0] = 'W';
		hw4.magic[1] = '4';
		hw4.w4.os_low = w3->pe->w3.os_low;
		hw4.w4.os_hi  = w3->pe->w3.os_hi;
		hw4.w4.chunk_size = PE_W4_CHUNKSIZE;
		hw4.w4.chunk_count = w4->chunks_cnt;

		memcpy(&hw4.w4.compression, "DS", 2);
		
		fw = fopen(dst, "wb");
		if(fw != NULL)
		{
			/* copy header */
			fseek(w3->fp, 0, SEEK_SET);
			buf = malloc(w3->pe_pos);
			if(buf != NULL)
			{
				if(fread(buf, 1, w3->pe_pos, w3->fp) == w3->pe_pos)
				{
					fwrite(buf, 1, w3->pe_pos, fw);
				}
				
				free(buf);
			}
			
			/* write W4 header */
			fwrite(&hw4, sizeof(pe_header_t), 1, fw);
			
			/* write (still empty) blocklist */
			fwrite(&(w4->chunks), w4->chunks_cnt, sizeof(uint32_t), fw);
			
			buf = malloc(PE_W4_CHUNKSIZE);
			if(buf != NULL)
			{
				size_t i;
				bs_file(&bsw4, fw);
				
				/* compress 8k chunks */
				for(i = 0; i < w4->chunks_cnt; i++)
				{
					ssize_t block_size;
					
					w4->chunks[i] = ftell(fw);
					block_size = fread(buf, 1, PE_W4_CHUNKSIZE, w3->fp);
					//printf("Compress: %d source pos: %d\n", block_size, w4->chunks[i]);
					ds_compress(buf, block_size, &bsw4);
				}
				
				free(buf);
			}
			
			/* write chunk list */
			fseek(fw, w3->pe_pos + sizeof(pe_header_t), SEEK_SET);
			fwrite(&(w4->chunks), w4->chunks_cnt, sizeof(uint32_t), fw);
			
			fclose(fw);
		}
		
		pe_w4_free(w4);
	} // if w4
	
	return PE_OK;
}

/**
 * Check if W4 file could be decompresed by legacy loaders.
 *
 * NOTE: Simply compressed chunks cannot exceed decompessed size.
 *
 * @return: PE_OK if file is compatible
 **/
int pe_w4_check(pe_w4_t *w4)
{
	size_t i;
	
	if(w4->pe->w4.chunk_size != PE_W4_CHUNKSIZE)
	{
		return PE_ERROR_COMPAT;
	}
	
	for(i = 0; i < w4->chunks_cnt; i++)
	{
		if((w4->chunks[i+1] - w4->chunks[i]) > w4->pe->w4.chunk_size)
		{
			return PE_ERROR_COMPAT;
		}
	}
	
	return PE_OK;
}

/**
 * Extract VXD form W3 (*.VXD extension too) file.
 *
 * @param file: file name in archive (names are without file extension)
 *
 **/
int pe_w3_extract(pe_w3_t *w3, const char *file, const char *dst)
{
	uint8_t sname[PE_W3_FILE_NAME_SIZE+1];
	size_t len;
	size_t i;
	size_t next_file;
	le_header_t le_header;
	int result = PE_ERROR_NO_FOUND;
	
	len = strlen(file);
	if(len > PE_W3_FILE_NAME_SIZE)
	{
		len = PE_W3_FILE_NAME_SIZE;
	}
	memcpy(sname, file, len);
	/* space padding */
	for(;len < PE_W3_FILE_NAME_SIZE;len++)
	{
		sname[len] = ' ';
	}
	sname[PE_W3_FILE_NAME_SIZE] = '\0';
	
	for(i = 0; i < w3->files_cnt; i++)
	{
		if(istrncmp((char*)sname, (char*)w3->files[i].name, PE_W3_FILE_NAME_SIZE) == 0)
		{
			FILE *fw;
			size_t file_offset = w3->files[i].file_offset;
			
			memset(&le_header, 0, sizeof(le_header_t));
			
			//printf("found: %s, file_offset 0x%X, header_size: 0x%X\n", sname, file_offset, w3->files[i].header_size);
			
			if(i+1 < w3->files_cnt)
			{
				next_file = w3->files[i+1].file_offset;
			}
			else
			{
				next_file = w3->file_size;
			}
			
			fw = fopen(dst, "wb");
			if(fw == NULL)
			{
				return PE_ERROR_FOPEN;
			}
			
			fwrite(dos_program_le, sizeof(dos_program_le), 1, fw);
			fseek(w3->fp, file_offset, SEEK_SET);
			if(fread(&le_header, sizeof(le_header), 1, w3->fp) == 1)
			{
				le_header.data_pages_offset_from_top_of_file += sizeof(dos_program_le);
				le_header.data_pages_offset_from_top_of_file -= file_offset;
				
				/* WARNING: by documentation this SHOULD BY recalculate too */
				le_header.nonresident_names_table_offset_from_top_of_file += sizeof(dos_program_le);
				le_header.nonresident_names_table_offset_from_top_of_file -= file_offset;
				
				fwrite(&le_header, sizeof(le_header), 1, fw);
				
				fs_file_copy(w3->fp, fw, next_file-file_offset-sizeof(le_header_t));
				
				result = PE_OK;
			}
			
			fclose(fw);
			
			break; /// --+
		}        ///   |
	}          ///   |
	           /// <-+
	
	return result;
}
