#ifndef __W3_V2__PATCH_INCLUDED__
#define __W3_V2__PATCH_INCLUDED__

#include <patch.h>

const spatch_data_t w3_v2_data[] = {
	{0x00000C96, 1, "\x08", "\x88"},
	{0x00000C9B, 12, "\x0B\xC0\x0F\x84\xB3\x00\x66\x0F\xB7\xC8\x8B\xD0", "\xE8\x0C\xCE\x74\x13\x66\x89\xC1\x66\x89\xC2\x90"},
	{0x00000CAA, 1, "\x09", "\x89"},
	{0x00000D51, 1, "\x40", "\x3E"},
	{0x0000DAAA, 5, "\x00\x00\x00\x00\x00", "\x66\x3D\xFF\xFF\x03"},
	{0x0000DAB0, 7, "\x00\x00\x00\x00\x00\x00\x00", "\x72\x06\x66\xB8\xFF\xFF\x03"},
	{0x0000DAB8, 4, "\x00\x00\x00\x00", "\x66\x09\xC0\xC3"},
	{0x00013FFC, 4, "\x00\x00\x00\x00", "\x52\x52\x4C\x22"},
	{0, 0, NULL, NULL}
};

const spatch_t w3_v2_sp = {81920, w3_v2_data};

#endif /*__W3_V2__PATCH_INCLUDED__*/
