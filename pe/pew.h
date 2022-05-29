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
#ifndef __PEW_H__INCLUDED__
#define __PEW_H__INCLUDED__

#include <stdint.h>
#include <stdio.h>
#include <cextra.h>

#pragma pack(push)
#pragma pack(1)

typedef struct _dos_header_t
{
	uint8_t  magic[2];
	uint8_t  header[58];
	uint32_t nextheader;
} dos_header_t;

typedef struct _pe_header_t
{
	uint8_t  magic[2];
	union
	{
		struct _w3
		{
			uint8_t  os_low;
			uint8_t  os_hi;
			uint16_t vxd_count;
			uint8_t  padding[10];
		} w3;
		
		struct _w4
		{
			uint8_t  os_low;
			uint8_t  os_hi;
			uint16_t chunk_size;
			uint16_t chunk_count;
			uint8_t  compression[2];
			uint8_t  padding[6];
		} w4;
	};
} pe_header_t;


/* https://faydoc.tripod.com/formats/exe-LE.htm#Info_Block */
typedef struct _le_header_t
{
	uint8_t  magic[2];
	uint8_t  byte_order;
	uint8_t  word_order;
	uint32_t executable_format_level;
	uint16_t CPU_type;
	uint16_t target_OS;
	uint32_t module_version;
	uint32_t module_type_flags;
	uint32_t number_of_memory_pages;
	uint32_t initial_object_CS_number;
	uint32_t initial_EIP;
	uint32_t initial_object_SS_number;
	uint32_t initial_ESP;
	uint32_t memory_page_size;
	uint32_t bytes_on_last_page;
	uint32_t fixup_section_size;
	uint32_t fixup_section_checksum;
	uint32_t loader_section_size;
	uint32_t loader_section_checksum;
	uint32_t offset_of_object_table;
	uint32_t object_table_entries;
	uint32_t object_page_map_offset;
	uint32_t object_iterate_data_map_offset;
	uint32_t resource_table_offset;
	uint32_t resource_table_entries;
	uint32_t resident_names_table_offset;
	uint32_t entry_table_offset;
	uint32_t module_directives_table_offset;
	uint32_t module_directives_entries;
	uint32_t fix_up_page_table_offset;
	uint32_t fix_up_record_table_offset;
	uint32_t imported_modules_name_table_offset;
	uint32_t imported_modules_count;
	uint32_t imported_procedure_name_table_offset;
	uint32_t perpage_checksum_table_offset;
	uint32_t data_pages_offset_from_top_of_file; /* recalculate */
	uint32_t preload_page_count;
	uint32_t nonresident_names_table_offset_from_top_of_file; /* recalculate ??? */
	uint32_t nonresident_names_table_length;
	uint32_t nonresident_names_table_checksum;
	uint32_t automatic_data_object;
	uint32_t debug_information_offset;
	uint32_t debug_information_length;
	uint32_t preload_instance_pages_number;
	uint32_t demand_instance_pages_number;
	uint32_t extra_heap_allocation;
	uint8_t  padding[4];
} le_header_t;

#pragma pack(pop)

typedef struct _pe_w4_t
{
	pe_header_t *pe;
	size_t   pe_pos;
	FILE     *fp;
	size_t   chunks_cnt;
	uint32_t chunks[0];
} pe_w4_t;

#define PE_W3_FILE_NAME_SIZE 8

typedef struct _pe_w3_file_t
{
	uint8_t name[PE_W3_FILE_NAME_SIZE];
	uint32_t file_offset;
	uint32_t header_size;
} pe_w3_file_t;

typedef struct _pe_w3_t
{
	pe_header_t *pe;
	size_t   pe_pos;
	FILE     *fp;
	size_t   files_cnt;
	size_t   file_size;
	pe_w3_file_t files[0];
} pe_w3_t;

#define MAGIC_DOS "MZ"
#define MAGIC_W3  "W3"
#define MAGIC_W4  "W4"
#define MAGIC_LE  "LE"
#define MAGIC_MSCAB "MSCF"

#define BLOCK_OK    0
#define BLOCK_END   1
#define BLOCK_BOUND 2
#define BLOCK_COUNT_ERR 3

#define PE_OK 0
#define PE_NO_MZ_FILE 1
#define PE_UNKNOWN 2
#define PE_W3 3
#define PE_W4 4
#define PE_LE 5
#define PE_NO_IS_MSCAB  6
#define PE_ERROR_FOPEN  7
#define PE_ERROR_FREAD  8
#define PE_ERROR_FWRITE 9
#define PE_ERROR_FSEEK  10
#define PE_ERROR_NO_FOUND 11
#define PE_ERROR_COMPAT  12

#define PE_W4_CHUNKSIZE 8192

int      pe_read(dos_header_t *dos, pe_header_t *pe, FILE *fp);
pe_w4_t *pe_w4_read(dos_header_t *dos, pe_header_t *pe, FILE *fp);
pe_w4_t *pe_w4_alloc(size_t data_size);
void     pe_w4_free(pe_w4_t *w4);
int      pe_w4_check(pe_w4_t *w4);

pe_w3_t *pe_w3_read(dos_header_t *dos, pe_header_t *pe, FILE *fp);
void pe_w3_free(pe_w3_t *w4);

size_t pe_w4_decompress(pe_w4_t *w4, void *buf, size_t chunk_id);
int pe_w4_to_w3(pe_w4_t *w4, const char *dst);
int pe_w3_to_w4(pe_w3_t *w3, const char *dst);
int pe_w3_extract(pe_w3_t *w3, const char *file, const char *dst);


#endif /* __W4_H__INCLUDED__ */
