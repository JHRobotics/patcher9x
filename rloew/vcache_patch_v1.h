#ifndef __VCACHE_V1__PATCH_INCLUDED__
#define __VCACHE_V1__PATCH_INCLUDED__

#include <patch.h>

const spatch_data_t vcache_v1_data[] = {
	{0x0000007C, 4, "\x00\x00\x00\x00", "\x52\x52\x4C\x30"},
	{0x00005D25, 2, "\x20\x03", "\x00\x02"},
	{0x00005D2C, 2, "\x20\x03", "\x00\x02"},
	{0, 0, NULL, NULL}
};

const spatch_t vcache_v1_sp = {36606, vcache_v1_data};

#endif /*__VCACHE_V1__PATCH_INCLUDED__*/
