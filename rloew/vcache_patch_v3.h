#ifndef __VCACHE_V3__PATCH_INCLUDED__
#define __VCACHE_V3__PATCH_INCLUDED__

#include <patch.h>

const spatch_data_t vcache_v3_data[] = {
	{0x00007B36, 2, "\x20\x03", "\x00\x02"},
	{0x00007B3D, 2, "\x20\x03", "\x00\x02"},
	{0, 0, NULL, NULL}
};

const spatch_t vcache_v3_sp = {32310, vcache_v3_data};

#endif /*__VCACHE_V3__PATCH_INCLUDED__*/
