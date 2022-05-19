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
#ifndef __BPATCHER_H__INCLUDED__
#define __BPATCHER_H__INCLUDED__

#include <bitstream.h>

#define BPATCHER_FILE_BUF 8192

ssize_t search_sieve(const uint8_t *haystack, size_t haystack_size,
                     const uint8_t *needle, size_t needle_size, bitstream_t *sieve);

void patch_sieve(uint8_t *dst, const uint8_t *newdata, size_t data_size,
                 bitstream_t *sieve);

void diff_sieve(const uint8_t *data_a, const uint8_t *data_b, size_t data_size,
                bitstream_t *sieve);

ssize_t search_sieve_file(FILE *haystack_fp,
                          const uint8_t *needle, size_t needle_size,
                          bitstream_t *sieve);

#endif /* __BPATCHER_H__INCLUDED__ */
