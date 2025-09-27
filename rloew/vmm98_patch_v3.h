#ifndef __VMM98_V3__PATCH_INCLUDED__
#define __VMM98_V3__PATCH_INCLUDED__

#include <patch.h>

const spatch_data_t vmm98_v3_data[] = {
	{0x00000124, 2, "\x1C\x58", "\x00\xEA"},
	{0x0003140C, 2, "\x34\x08", "\x90\x0E"},
	{0x00031413, 2, "\x34\x08", "\x90\x0E"},
	{0x00031539, 1, "\x52", "\x91"},
	{0x00031541, 1, "\x52", "\x91"},
	{0x00031568, 1, "\x68", "\xB8"},
	{0x0003156D, 4, "\xE8\xBE\x0E\x00", "\xE9\xAA\x1C\x01"},
	{0x0003180C, 1, "\x06", "\x0A"},
	{0x0003184B, 1, "\x60", "\xA0"},
	{0x00031C84, 1, "\x81", "\x01"},
	{0x00031C9E, 1, "\x84", "\x04"},
	{0x0004321C, 3, "\x00\x00\x00", "\x05\x80\x59"},
	{0x00043221, 11, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", "\x50\xE8\x09\xF2\xFE\xFF\xE9\x46\xE3\xFE\xFF"},
	{0, 0, NULL, NULL}
};

const spatch_t vmm98_v3_sp = {409076, vmm98_v3_data};

#endif /*__VMM98_V3__PATCH_INCLUDED__*/
