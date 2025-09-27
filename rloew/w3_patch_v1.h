#ifndef __W3_V1__PATCH_INCLUDED__
#define __W3_V1__PATCH_INCLUDED__

#include <patch.h>

const spatch_data_t w3_v1_data[] = {
	{0x00000A8E, 1, "\x08", "\x88"},
	{0x00000A93, 12, "\x0B\xC0\x0F\x84\xB3\x00\x66\x0F\xB7\xC8\x8B\xD0", "\x66\x09\xC0\x74\x13\x66\x89\xC1\x66\x89\xC2\x90"},
	{0x00000AA2, 1, "\x09", "\x89"},
	{0, 0, NULL, NULL}
};

const spatch_t w3_v1_sp = {66560, w3_v1_data};

#endif /*__W3_V1__PATCH_INCLUDED__*/
