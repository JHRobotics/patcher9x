#ifndef __VCACHE_V2__PATCH_INCLUDED__
#define __VCACHE_V2__PATCH_INCLUDED__

#include <patch.h>

const spatch_data_t vcache_v2_data[] = {
	{0x0000283F, 2, "\x20\x03", "\x00\x02"},
	{0x00002846, 2, "\x20\x03", "\x00\x02"},
	{0, 0, NULL, NULL}
};

const spatch_t vcache_v2_sp = {18890, vcache_v2_data};

#endif /*__VCACHE_V2__PATCH_INCLUDED__*/
