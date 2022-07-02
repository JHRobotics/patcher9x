# cat/type
CAT=$(if $(filter $(OS),Windows_NT),type,cat)
# move/mv
#MV=$(if $(filter $(OS),Windows_NT),cmd /C move /Y,mv -f)
MV=mv
RUNPATH=$(if $(filter $(OS),Windows_NT),.\,./)
NULLOUT=$(if $(filter $(OS),Windows_NT),NUL,/dev/null)
CP ?= cp

GIT      ?= git
GIT_IS   := $(shell $(GIT) rev-parse --is-inside-work-tree 2> $(NULLOUT))
ifeq ($(GIT_IS),true)
 VERSION_PATCH := $(shell $(GIT) rev-list --count main)
endif

SUFIX=
ifeq ($(filter $(OS),Windows_NT),Windows_NT)
  SUFIX=.exe
  HOST_SUFIX=.exe
endif

CPP    = cpp
CC    ?= gcc
STRIP ?= strip
FASM  ?= fasm

ifdef RELEASE
  CFLAGS ?= -g0 -static -Os -Wall -Wextra -Wno-unused-parameter -Wno-unused-function
else
  CFLAGS ?= -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function
endif

CFLAGS := $(CFLAGS) -I./mspack -I./system -I./pe

HOST_CC     ?= $(CC)
HOST_CFLAGS ?= $(CFLAGS)
HOST_LDFLAGS ?= 

GUEST_CC     ?= $(CC)
GUEST_CFLAGS ?= $(CFLAGS)
GUEST_LDFLAGS ?= 

OUTNAME=patcher9x$(SUFIX)

ifdef PROFILE
  ifeq ($(PROFILE),djgpp)
    GUEST_CC := i586-pc-msdosdjgpp-gcc
    SUFIX    := .exe
    STRIP    := i586-pc-msdosdjgpp-strip
    OUTNAME  := patch9x$(SUFIX)
  endif
endif

ifdef VERSION_PATCH
 HOST_CFLAGS  := $(HOST_CFLAGS)  -DPATCHER9X_PATCH=$(VERSION_PATCH)
 GUEST_CFLAGS := $(GUEST_CFLAGS) -DPATCHER9X_PATCH=$(VERSION_PATCH)
endif

DEPS_HOST=Makefile system/bitstream.h version.h
DEPS_GUEST=Makefile system/bitstream.h version.h

OBJS_GUEST=mspack/cabc.g.o mspack/cabd.g.o mspack/crc32.g.o mspack/hlpc.g.o mspack/hlpd.g.o mspack/chmc.g.o mspack/chmd.g.o mspack/kwajc.g.o mspack/kwajd.g.o mspack/litc.g.o \
 mspack/litd.g.o mspack/lzssd.g.o mspack/lzxc.g.o mspack/lzxd.g.o mspack/mszipc.g.o mspack/mszipd.g.o mspack/oabc.g.o mspack/oabd.g.o mspack/qtmd.g.o mspack/system.g.o \
 mspack/szddc.g.o mspack/szddd.g.o system/filesystem.g.o system/bpatcher.g.o pe/pew.g.o pe/ds_compress.g.o pe/ds_decompress.g.o
 
OBJS_OUT=$(OBJS_GUEST) unpacker.g.o patch.g.o trace.g.o patcher9x.g.o cputest.g.o files.g.o

OBJS_HOST=system/bpatcher.h.o

TESTS=test_bitstream$(SUFIX) test_pe$(SUFIX) test_pe2$(SUFIX) test_ds$(SUFIX) test_ds_compress$(SUFIX) test_w3$(SUFIX) test_patch$(SUFIX) test_ds2$(SUFIX)

all: $(OUTNAME)

tests: $(TESTS)

.PHONY: all tests clean fasmdiff

%.g.o: %.c $(DEPS_GUEST)
	$(GUEST_CC) $(GUEST_CFLAGS) -c -o $@ $<

%.h.o: %.c $(DEPS_HOST)
	$(HOST_CC) $(HOST_CFLAGS) -c -o $@ $<

%.h: %.h.tmp
	$(CP) $< $@

%.bin: %.asm.gen
	$(FASM) $< $@

$(OUTNAME): $(OBJS_OUT)
	$(GUEST_CC) $(GUEST_CFLAGS) -o $(OUTNAME) $(OBJS_OUT) $(GUEST_LDFLAGS)
	
strip: $(OUTNAME)
	$(STRIP) -s $(OUTNAME)

test_bitstream$(SUFIX): $(OBJS_GUEST) test/test_bitstream.g.o
	$(GUEST_CC) $(GUEST_CFLAGS) -o test_bitstream$(SUFIX) $(OBJS_GUEST) test/test_bitstream.g.o $(GUEST_LDFLAGS)

test_pe$(SUFIX): $(OBJS_GUEST) test/test_pe.g.o
	$(GUEST_CC) $(GUEST_CFLAGS) -o test_pe$(SUFIX) $(OBJS_GUEST) test/test_pe.g.o $(GUEST_LDFLAGS)

test_pe2$(SUFIX): $(OBJS_GUEST) test/test_pe2.g.o
	$(GUEST_CC) $(GUEST_CFLAGS) -o test_pe2$(SUFIX) $(OBJS_GUEST) test/test_pe2.g.o $(GUEST_LDFLAGS)

test_w3$(SUFIX): $(OBJS_GUEST) test/test_w3.g.o
	$(GUEST_CC) $(GUEST_CFLAGS) -o test_w3$(SUFIX) $(OBJS_GUEST) test/test_w3.g.o $(GUEST_LDFLAGS)

test_ds$(SUFIX): $(OBJS_GUEST) test/test_ds.g.o
	$(GUEST_CC) $(GUEST_CFLAGS) -o test_ds$(SUFIX) $(OBJS_GUEST) test/test_ds.g.o $(GUEST_LDFLAGS)

test_ds2$(SUFIX): $(OBJS_GUEST) test/test_ds2.g.o
	$(GUEST_CC) $(GUEST_CFLAGS) -o test_ds2$(SUFIX) $(OBJS_GUEST) test/test_ds2.g.o $(GUEST_LDFLAGS)

test_ds_compress$(SUFIX): $(OBJS_GUEST) test/test_ds_compress.g.o
	$(GUEST_CC) $(GUEST_CFLAGS) -o test_ds_compress$(SUFIX) $(OBJS_GUEST) test/test_ds_compress.g.o $(GUEST_LDFLAGS)
	
test_patch$(SUFIX): $(OBJS_GUEST) test/test_patch.g.o
	$(GUEST_CC) $(GUEST_CFLAGS) -o test_patch$(SUFIX) $(OBJS_GUEST) test/test_patch.g.o $(GUEST_LDFLAGS)

vmm/patched.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P vmm/FlushMappedCacheBlock.asm -o vmm/patched.asm.gen
 
vmm/original.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode vmm/FlushMappedCacheBlock.asm -o vmm/original.asm.gen

vmm/reloc.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate vmm/FlushMappedCacheBlock.asm -o vmm/reloc.asm.gen

vmm/patchedme.asm.gen: vmm/FlushMappedCacheBlockMe.asm
	$(CPP) -nostdinc -E -P vmm/FlushMappedCacheBlockMe.asm -o vmm/patchedme.asm.gen
 
vmm/originalme.asm.gen: vmm/FlushMappedCacheBlockMe.asm
	$(CPP) -nostdinc -E -P -Doriginalcode vmm/FlushMappedCacheBlockMe.asm -o vmm/originalme.asm.gen

vmm/relocme.asm.gen: vmm/FlushMappedCacheBlockMe.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate vmm/FlushMappedCacheBlockMe.asm -o vmm/relocme.asm.gen

vmm/patched_v2.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Dversion_2 vmm/FlushMappedCacheBlock.asm -o vmm/patched_v2.asm.gen
 
vmm/original_v2.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_2 vmm/FlushMappedCacheBlock.asm -o vmm/original_v2.asm.gen

vmm/reloc_v2.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_2 vmm/FlushMappedCacheBlock.asm -o vmm/reloc_v2.asm.gen

makepatch$(HOST_SUFIX): $(OBJS_HOST) vmm/makepatch.h.o
	$(HOST_CC) $(HOST_CFLAGS) -o makepatch$(HOST_SUFIX) $(OBJS_HOST) vmm/makepatch.h.o $(HOST_LDFLAGS)
	
makediff$(HOST_SUFIX): $(OBJS_HOST) vmm/makediff.h.o
	$(HOST_CC) $(HOST_CFLAGS) -o makediff$(HOST_SUFIX) $(OBJS_HOST) vmm/makediff.h.o $(HOST_LDFLAGS)

vmm_patch.h.tmp: vmm/patched.bin vmm/original.bin vmm/reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) 98 vmm_patch 1040 448 492 > vmm_patch.h.tmp
	
vmm_patch_v2.h.tmp: vmm/patched_v2.bin vmm/original_v2.bin vmm/reloc_v2.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) 98v2 vmm_patch_v2 1052 448 504 > vmm_patch_v2.h.tmp
	
vmm_patch_me1.h.tmp: vmm/patchedme.bin vmm/originalme.bin vmm/relocme.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) me vmm_patch_me1 16320 12 461 > vmm_patch_me1.h.tmp

vmm_patch_me2.h.tmp: vmm/patchedme.bin vmm/originalme.bin vmm/relocme.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) me vmm_patch_me2 16320 16160 160 > vmm_patch_me2.h.tmp

vmm/fasmdiff.h.tmp: vmm/original.bin vmm/dump.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) fasmdiff vmm/dump.bin vmm/original.bin > vmm/fasmdiff.h.tmp

vmm/fasmdiff_v2.h.tmp: vmm/original_v2.bin vmm/dump_v2.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) fasmdiff_v2 vmm/dump_v2.bin vmm/original_v2.bin > vmm/fasmdiff_v2.h.tmp

vmm/fasmdiff_me.h.tmp: vmm/originalme.bin vmm/dumpme.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) fasmdiff_me vmm/dumpme.bin vmm/originalme.bin 0x1D8 0x3F20 > vmm/fasmdiff_me.h.tmp

# cpu speed V1
cpuspeed/speed_v1.asm.gen: cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Dversion_1 cpuspeed/speed_v1.asm -o cpuspeed/speed_v1.asm.gen

cpuspeed/speed_v1_orig.asm.gen: cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_1 cpuspeed/speed_v1.asm -o cpuspeed/speed_v1_orig.asm.gen

cpuspeed/speed_v1_reloc.asm.gen: cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_1 cpuspeed/speed_v1.asm -o cpuspeed/speed_v1_reloc.asm.gen

cpuspeed/speed_v1_diff.h.tmp: cpuspeed/speed_v1_dump.bin cpuspeed/speed_v1_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v1_diff cpuspeed/speed_v1_dump.bin cpuspeed/speed_v1_orig.bin > cpuspeed/speed_v1_diff.h.tmp

cpuspeed_patch_v1.h.tmp: cpuspeed/speed_v1.bin cpuspeed/speed_v1_orig.bin cpuspeed/speed_v1_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v1 cpuspeed_patch_v1 53 0 53 > cpuspeed_patch_v1.h.tmp


# cpu speed V2
cpuspeed/speed_v2.asm.gen: cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Dversion_2 cpuspeed/speed_v1.asm -o cpuspeed/speed_v2.asm.gen

cpuspeed/speed_v2_orig.asm.gen: cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_2 cpuspeed/speed_v1.asm -o cpuspeed/speed_v2_orig.asm.gen

cpuspeed/speed_v2_reloc.asm.gen: cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_2 cpuspeed/speed_v1.asm -o cpuspeed/speed_v2_reloc.asm.gen

cpuspeed/speed_v2_diff.h.tmp: cpuspeed/speed_v2_dump.bin cpuspeed/speed_v2_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v2_diff cpuspeed/speed_v2_dump.bin cpuspeed/speed_v2_orig.bin > cpuspeed/speed_v2_diff.h.tmp

cpuspeed_patch_v2.h.tmp: cpuspeed/speed_v2.bin cpuspeed/speed_v2_orig.bin cpuspeed/speed_v2_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v2 cpuspeed_patch_v2 53 0 53 > cpuspeed_patch_v2.h.tmp

# cpu speed V4
cpuspeed/speed_v4.asm.gen: cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Dversion_4 cpuspeed/speed_v1.asm -o cpuspeed/speed_v4.asm.gen

cpuspeed/speed_v4_orig.asm.gen: cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_4 cpuspeed/speed_v1.asm -o cpuspeed/speed_v4_orig.asm.gen

cpuspeed/speed_v4_reloc.asm.gen: cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_4 cpuspeed/speed_v1.asm -o cpuspeed/speed_v4_reloc.asm.gen

cpuspeed/speed_v4_diff.h.tmp: cpuspeed/speed_v4_dump.bin cpuspeed/speed_v4_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v4_diff cpuspeed/speed_v4_dump.bin cpuspeed/speed_v4_orig.bin > cpuspeed/speed_v4_diff.h.tmp

cpuspeed_patch_v4.h.tmp: cpuspeed/speed_v4.bin cpuspeed/speed_v4_orig.bin cpuspeed/speed_v4_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v4 cpuspeed_patch_v4 53 0 53 > cpuspeed_patch_v4.h.tmp

# cpu speed V3
cpuspeed/speed_v3.asm.gen: cpuspeed/speed_v3.asm
	$(CPP) -nostdinc -E -P -Dversion_3 cpuspeed/speed_v3.asm -o cpuspeed/speed_v3.asm.gen

cpuspeed/speed_v3_orig.asm.gen: cpuspeed/speed_v3.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_3 cpuspeed/speed_v3.asm -o cpuspeed/speed_v3_orig.asm.gen

cpuspeed/speed_v3_reloc.asm.gen: cpuspeed/speed_v3.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_3 cpuspeed/speed_v3.asm -o cpuspeed/speed_v3_reloc.asm.gen

cpuspeed/speed_v3_diff.h.tmp: cpuspeed/speed_v3_dump.bin cpuspeed/speed_v3_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v3_diff cpuspeed/speed_v3_dump.bin cpuspeed/speed_v3_orig.bin > cpuspeed/speed_v3_diff.h.tmp

cpuspeed_patch_v3.h.tmp: cpuspeed/speed_v3.bin cpuspeed/speed_v3_orig.bin cpuspeed/speed_v3_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v3 cpuspeed_patch_v3 44 0 44 > cpuspeed_patch_v3.h.tmp

# cpu speed V5
cpuspeed/speed_v5.asm.gen: cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Dversion_5 cpuspeed/speed_v5.asm -o cpuspeed/speed_v5.asm.gen

cpuspeed/speed_v5_orig.asm.gen: cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_5 cpuspeed/speed_v5.asm -o cpuspeed/speed_v5_orig.asm.gen

cpuspeed/speed_v5_reloc.asm.gen: cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_5 cpuspeed/speed_v5.asm -o cpuspeed/speed_v5_reloc.asm.gen

cpuspeed/speed_v5_diff.h.tmp: cpuspeed/speed_v5_dump.bin cpuspeed/speed_v5_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v5_diff cpuspeed/speed_v5_dump.bin cpuspeed/speed_v5_orig.bin > cpuspeed/speed_v5_diff.h.tmp

cpuspeed_patch_v5.h.tmp: cpuspeed/speed_v5.bin cpuspeed/speed_v5_orig.bin cpuspeed/speed_v5_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v5 cpuspeed_patch_v5 69 0 69 > cpuspeed_patch_v5.h.tmp

# cpu speed V6
cpuspeed/speed_v6.asm.gen: cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Dversion_6 cpuspeed/speed_v5.asm -o cpuspeed/speed_v6.asm.gen

cpuspeed/speed_v6_orig.asm.gen: cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_6 cpuspeed/speed_v5.asm -o cpuspeed/speed_v6_orig.asm.gen

cpuspeed/speed_v6_reloc.asm.gen: cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_6 cpuspeed/speed_v5.asm -o cpuspeed/speed_v6_reloc.asm.gen

cpuspeed/speed_v6_diff.h.tmp: cpuspeed/speed_v6_dump.bin cpuspeed/speed_v6_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v6_diff cpuspeed/speed_v6_dump.bin cpuspeed/speed_v6_orig.bin > cpuspeed/speed_v6_diff.h.tmp

cpuspeed_patch_v6.h.tmp: cpuspeed/speed_v6.bin cpuspeed/speed_v6_orig.bin cpuspeed/speed_v6_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v6 cpuspeed_patch_v6 69 0 69 > cpuspeed_patch_v6.h.tmp

# cpu speed V7
cpuspeed/speed_v7.asm.gen: cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Dversion_7 cpuspeed/speed_v5.asm -o cpuspeed/speed_v7.asm.gen

cpuspeed/speed_v7_orig.asm.gen: cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_7 cpuspeed/speed_v5.asm -o cpuspeed/speed_v7_orig.asm.gen

cpuspeed/speed_v7_reloc.asm.gen: cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_7 cpuspeed/speed_v5.asm -o cpuspeed/speed_v7_reloc.asm.gen

cpuspeed/speed_v7_diff.h.tmp: cpuspeed/speed_v7_dump.bin cpuspeed/speed_v7_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v7_diff cpuspeed/speed_v7_dump.bin cpuspeed/speed_v7_orig.bin > cpuspeed/speed_v7_diff.h.tmp

cpuspeed_patch_v7.h.tmp: cpuspeed/speed_v7.bin cpuspeed/speed_v7_orig.bin cpuspeed/speed_v7_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v7 cpuspeed_patch_v7 69 0 69 > cpuspeed_patch_v7.h.tmp

# cpu speed V8
cpuspeed/speed_v8.asm.gen: cpuspeed/speed_v8.asm
	$(CPP) -nostdinc -E -P -Dversion_8 cpuspeed/speed_v8.asm -o cpuspeed/speed_v8.asm.gen

cpuspeed/speed_v8_orig.asm.gen: cpuspeed/speed_v8.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_8 cpuspeed/speed_v8.asm -o cpuspeed/speed_v8_orig.asm.gen

cpuspeed/speed_v8_reloc.asm.gen: cpuspeed/speed_v8.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_8 cpuspeed/speed_v8.asm -o cpuspeed/speed_v8_reloc.asm.gen

cpuspeed/speed_v8_diff.h.tmp: cpuspeed/speed_v8_dump.bin cpuspeed/speed_v8_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v8_diff cpuspeed/speed_v8_dump.bin cpuspeed/speed_v8_orig.bin > cpuspeed/speed_v8_diff.h.tmp

cpuspeed_patch_v8.h.tmp: cpuspeed/speed_v8.bin cpuspeed/speed_v8_orig.bin cpuspeed/speed_v8_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v8 cpuspeed_patch_v8 60 0 60 > cpuspeed_patch_v8.h.tmp


# cpu speed NDIS.VXD V1
cpuspeed/speedndis_v1.asm.gen: cpuspeed/speedndis_v1.asm
	$(CPP) -nostdinc -E -P -Dversion_1 cpuspeed/speedndis_v1.asm -o cpuspeed/speedndis_v1.asm.gen

cpuspeed/speedndis_v1_orig.asm.gen: cpuspeed/speedndis_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_1 cpuspeed/speedndis_v1.asm -o cpuspeed/speedndis_v1_orig.asm.gen

cpuspeed/speedndis_v1_reloc.asm.gen: cpuspeed/speedndis_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_1 cpuspeed/speedndis_v1.asm -o cpuspeed/speedndis_v1_reloc.asm.gen

cpuspeed/speedndis_v1_diff.h.tmp: cpuspeed/speedndis_v1_dump.bin cpuspeed/speedndis_v1_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speedndis_v1_diff cpuspeed/speedndis_v1_dump.bin cpuspeed/speedndis_v1_orig.bin > cpuspeed/speedndis_v1_diff.h.tmp

cpuspeed_ndis_patch_v1.h.tmp: cpuspeed/speedndis_v1.bin cpuspeed/speedndis_v1_orig.bin cpuspeed/speedndis_v1_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speedndis_v1 cpuspeed_ndis_patch_v1 112 0 112 > cpuspeed_ndis_patch_v1.h.tmp

# cpu speed NDIS.VXD V3
cpuspeed/speedndis_v3.asm.gen: cpuspeed/speedndis_v1.asm
	$(CPP) -nostdinc -E -P -Dversion_3 cpuspeed/speedndis_v1.asm -o cpuspeed/speedndis_v3.asm.gen

cpuspeed/speedndis_v3_orig.asm.gen: cpuspeed/speedndis_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_3 cpuspeed/speedndis_v1.asm -o cpuspeed/speedndis_v3_orig.asm.gen

cpuspeed/speedndis_v3_reloc.asm.gen: cpuspeed/speedndis_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_3 cpuspeed/speedndis_v1.asm -o cpuspeed/speedndis_v3_reloc.asm.gen

cpuspeed/speedndis_v3_diff.h.tmp: cpuspeed/speedndis_v3_dump.bin cpuspeed/speedndis_v3_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speedndis_v3_diff cpuspeed/speedndis_v3_dump.bin cpuspeed/speedndis_v3_orig.bin > cpuspeed/speedndis_v3_diff.h.tmp

cpuspeed_ndis_patch_v3.h.tmp: cpuspeed/speedndis_v3.bin cpuspeed/speedndis_v3_orig.bin cpuspeed/speedndis_v3_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speedndis_v3 cpuspeed_ndis_patch_v3 112 0 112 > cpuspeed_ndis_patch_v3.h.tmp

# cpu speed NDIS.VXD V2
cpuspeed/speedndis_v2.asm.gen: cpuspeed/speedndis_v2.asm
	$(CPP) -nostdinc -E -P -Dversion_2 cpuspeed/speedndis_v2.asm -o cpuspeed/speedndis_v2.asm.gen

cpuspeed/speedndis_v2_orig.asm.gen: cpuspeed/speedndis_v2.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_2 cpuspeed/speedndis_v2.asm -o cpuspeed/speedndis_v2_orig.asm.gen

cpuspeed/speedndis_v2_reloc.asm.gen: cpuspeed/speedndis_v2.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_2 cpuspeed/speedndis_v2.asm -o cpuspeed/speedndis_v2_reloc.asm.gen

cpuspeed/speedndis_v2_diff.h.tmp: cpuspeed/speedndis_v2_dump.bin cpuspeed/speedndis_v2_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speedndis_v2_diff cpuspeed/speedndis_v2_dump.bin cpuspeed/speedndis_v2_orig.bin > cpuspeed/speedndis_v2_diff.h.tmp

cpuspeed_ndis_patch_v2.h.tmp: cpuspeed/speedndis_v2.bin cpuspeed/speedndis_v2_orig.bin cpuspeed/speedndis_v2_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speedndis_v2 cpuspeed_ndis_patch_v2 128 0 128 > cpuspeed_ndis_patch_v2.h.tmp

patch.g.o: vmm_patch.h vmm_patch_v2.h vmm_patch_me1.h vmm_patch_me2.h cpuspeed_ndis_patch_v1.h cpuspeed_ndis_patch_v2.h cpuspeed_ndis_patch_v3.h \
  cpuspeed_patch_v1.h cpuspeed_patch_v2.h cpuspeed_patch_v3.h cpuspeed_patch_v4.h cpuspeed_patch_v5.h cpuspeed_patch_v6.h cpuspeed_patch_v7.h cpuspeed_patch_v8.h 

fasmdiff: vmm/fasmdiff.h vmm/fasmdiff_v2.h vmm/fasmdiff_me.h \
  cpuspeed/speed_v1_diff.h cpuspeed/speed_v2_diff.h cpuspeed/speed_v3_diff.h cpuspeed/speed_v4_diff.h \
  cpuspeed/speedndis_v1_diff.h cpuspeed/speedndis_v2_diff.h cpuspeed/speedndis_v3_diff.h \
  cpuspeed/speed_v5_diff.h cpuspeed/speed_v6_diff.h cpuspeed/speed_v7_diff.h cpuspeed/speed_v8_diff.h

clean:
	-$(RM) *.o
	-$(RM) *.tmp
	-cd mspack && $(RM) *.o
	-cd pe && $(RM) *.o
	-cd system && $(RM) *.o
	-cd test && $(RM) *.o
	-cd vmm && $(RM) *.o
	-cd vmm && $(RM) *.gen
	-cd vmm && $(RM) *.tmp
	-cd cpuspeed && $(RM) *.o
	-cd cpuspeed && $(RM) *.gen
	-cd cpuspeed && $(RM) *.tmp
	-$(RM) $(OUTNAME)
	-$(RM) makepatch$(HOST_SUFIX)
	-$(RM) makediff$(HOST_SUFIX)
	-$(RM) $(TESTS)
