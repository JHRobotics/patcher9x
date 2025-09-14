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
#ifndef __PATCHER9X_PATCH_H__INCLUDED__
#define __PATCHER9X_PATCH_H__INCLUDED__

#include <stdint.h>

/* patches with fasm code */
typedef struct _cpatch_t
{
	const uint8_t *patch_data;
	size_t         patch_size;
	const uint8_t *orig_data;
	size_t         orig_size;
	const uint8_t *check_data;
	size_t         check_size;
	const uint8_t *modif_data;
	size_t         modif_size;
} cpatch_t;

/* patches with binary difference */
typedef struct _spatch_data_t
{
	uint32_t offset;
	uint32_t size;
	char *olddata;
	char *newdata;
} spatch_data_t;

typedef struct _spatch_t
{
	uint32_t filesize;
	const spatch_data_t *data;
} spatch_t;

#endif /* __PATCHER9X_PATCH_H__INCLUDED__ */
