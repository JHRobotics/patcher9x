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
  CFLAGS ?= -std=gnu99 -g0 -static -Os -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -fdata-sections -ffunction-sections
else
  ifdef SANITIZE
    CFLAGS ?= -std=gnu99 -g -O0 -fsanitize=address -Wall -Wextra -Wno-unused-parameter -Wno-unused-function
  else
    CFLAGS ?= -std=gnu99 -g -O1 -static -Wall -Wextra -Wno-unused-parameter -Wno-unused-function
  endif
endif

CFLAGS := $(CFLAGS) -I./mspack -I./system -I./pe -I./nocrt -I./rloew -I./crfix

HOST_CC      ?= $(CC)
HOST_CFLAGS  ?= $(CFLAGS)
HOST_LDFLAGS ?= $(LDFLAGS)

GUEST_CC      ?= $(CC)
GUEST_CFLAGS  ?= $(CFLAGS)
GUEST_LDFLAGS ?= $(LDFLAGS)

WINDRES       ?= windres
HOST_WINDRES  ?= $(WINDRES)
GUEST_WINDRES ?= $(WINDRES)

OUTNAME=patcher9x$(SUFIX)

OBJS_GUEST :=

ifdef PROFILE
  ifeq ($(PROFILE),djgpp)
    GUEST_CC := i586-pc-msdosdjgpp-gcc
    SUFIX    := .exe
    STRIP    := i586-pc-msdosdjgpp-strip
    OUTNAME  := patch9x$(SUFIX)
  endif
  ifeq ($(PROFILE),nocrt)
    GUEST_LDFLAGS += -nostdlib -nodefaultlibs -lgcc -lkernel32 -luser32
    GUEST_CFLAGS  += -DNOCRT -DNOCRT_FLOAT -DNOCRT_MEM -DNOCRT_FILE -ffreestanding -march=pentium2
    OBJS_GUEST    += nocrt/nocrt.g.o nocrt/nocrt_exe.g.o nocrt/nocrt_file_win.g.o nocrt/nocrt_math.g.o nocrt/nocrt_mem_win.g.o
  endif
  ifeq ($(PROFILE),nocrt64)
    GUEST_LDFLAGS += -nostdlib -nodefaultlibs -lgcc -lkernel32 -luser32
    GUEST_CFLAGS  += -DNOCRT -DNOCRT_FLOAT -DNOCRT_CALC -DNOCRT_MEM -DNOCRT_FILE -ffreestanding
    OBJS_GUEST    += nocrt/nocrt.g.o nocrt/nocrt_exe.g.o nocrt/nocrt_file_win.g.o nocrt/nocrt_math.g.o nocrt/nocrt_math_calc.g.o nocrt/nocrt_mem_win.g.o
    RES_FLAGS     += --target=pe-x86-64
  endif
endif

ifdef VERSION_PATCH
 HOST_CFLAGS  += -DPATCHER9X_PATCH=$(VERSION_PATCH)
 GUEST_CFLAGS += -DPATCHER9X_PATCH=$(VERSION_PATCH)
 RES_FLAGS    += -DPATCHER9X_PATCH=$(VERSION_PATCH)
endif

DEPS_HOST=Makefile system/bitstream.h version.h
DEPS_GUEST=Makefile system/bitstream.h version.h

OBJS_GUEST +=mspack/cabc.g.o mspack/cabd.g.o mspack/crc32.g.o mspack/hlpc.g.o mspack/hlpd.g.o mspack/chmc.g.o mspack/chmd.g.o mspack/kwajc.g.o mspack/kwajd.g.o mspack/litc.g.o \
 mspack/litd.g.o mspack/lzssd.g.o mspack/lzxc.g.o mspack/lzxd.g.o mspack/mszipc.g.o mspack/mszipd.g.o mspack/oabc.g.o mspack/oabd.g.o mspack/qtmd.g.o mspack/system.g.o \
 mspack/szddc.g.o mspack/szddd.g.o system/filesystem.g.o system/bpatcher.g.o pe/pew.g.o pe/ds_compress.g.o pe/ds_decompress.g.o pwin32.g.o
 
OBJS_OUT=$(OBJS_GUEST) unpacker.g.o patch.g.o trace.g.o patcher9x.g.o cputest.g.o files.g.o batch.g.o

OBJS_HOST=system/bpatcher.h.o

OBJS_MAKEPATCH = vmm/makepatch.h.o
OBJS_MAKEDIFF  = vmm/makediff.h.o
OBJS_BINDIFF   = rloew/bindiff.h.o

ifeq ($(filter $(OS),Windows_NT),Windows_NT)
  OBJS_MAKEPATCH += vmm/makepatch.h.res
  OBJS_MAKEDIFF  += vmm/makediff.h.res
  OBJS_OUT       += patcher9x.g.res
endif


TESTS=test_bitstream$(SUFIX) test_pe$(SUFIX) test_pe2$(SUFIX) test_ds$(SUFIX) test_ds_compress$(SUFIX) test_w3$(SUFIX) test_patch$(SUFIX) test_ds2$(SUFIX)

all: $(OUTNAME)

tests: $(TESTS)

.PHONY: all tests clean fasmdiff soliddiff doscross$(SUFIX) get-version get-help

%.g.o: %.c $(DEPS_GUEST)
	$(GUEST_CC) $(GUEST_CFLAGS) -c -o $@ $<

%.h.o: %.c $(DEPS_HOST)
	$(HOST_CC) $(HOST_CFLAGS) -c -o $@ $<

%.h.res: %.rc
	$(HOST_WINDRES) $(RES_FLAGS) --input $< --output $@ --output-format=coff

%.g.res: %.rc
	$(GUEST_WINDRES) $(RES_FLAGS) --input $< --output $@ --output-format=coff

%.h: %.h.tmp
	$(CP) $< $@

%.bin: %.asm.gen
	$(FASM) $< $@

$(OUTNAME): $(OBJS_OUT) $(EXTRA_DEPS)
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

########## patcher .h generators

makepatch$(HOST_SUFIX): $(OBJS_HOST) $(OBJS_MAKEPATCH)
	$(HOST_CC) $(HOST_CFLAGS) -o $@ $^ $(HOST_LDFLAGS)
	
makediff$(HOST_SUFIX): $(OBJS_HOST) $(OBJS_MAKEDIFF)
	$(HOST_CC) $(HOST_CFLAGS) -o $@ $^ $(HOST_LDFLAGS)

bindiff$(HOST_SUFIX): $(OBJS_HOST) $(OBJS_BINDIFF)
	$(HOST_CC) $(HOST_CFLAGS) -o $@ $^ $(HOST_LDFLAGS)

########## VMM patch (SE only)

vmm/patched.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Dvmmbugfix1 $< -o $@
 
vmm/original.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode $< -o $@

vmm/reloc.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dnofuncrelocate $< -o $@

# Version 2

vmm/patched_v2.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Dvmmbugfix1 -Dversion_2 $< -o $@
 
vmm/original_v2.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_2 $< -o $@

vmm/reloc_v2.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dnofuncrelocate -Dversion_2 $< -o $@

########## VMM patch (ME all)

vmm/patchedme.asm.gen: vmm/FlushMappedCacheBlockMe.asm
	$(CPP) -nostdinc -E -P -Dvmmbugfix2 $< -o $@
 
vmm/originalme.asm.gen: vmm/FlushMappedCacheBlockMe.asm
	$(CPP) -nostdinc -E -P -Doriginalcode $< -o $@

vmm/relocme.asm.gen: vmm/FlushMappedCacheBlockMe.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate $< -o $@

########## update old versions of patch (SE only)
vmm/original_old.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P $< -o $@

vmm/reloc_old.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Drelocate -Dnofuncrelocate $< -o $@

vmm/original_old_v2.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Dversion_2 $< -o $@

vmm/reloc_old_v2.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Drelocate -Dversion_2 -Dnofuncrelocate $< -o $@


########## VMM patch (simpler version: 98 ALL)

vmm/patched_simple.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P $< -o $@
 
vmm/original_simple.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode $< -o $@

vmm/reloc_simple.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate $< -o $@

# Version 2

vmm/patched_simple_v2.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Dversion_2 $< -o $@
 
vmm/original_simple_v2.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_2 $< -o $@

vmm/reloc_simple_v2.asm.gen: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_2 $< -o $@

########## headers VMM

# V1 (SE)

vmm_patch.h: vmm/patched.bin vmm/original.bin vmm/reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) 98 vmm_patch 1040 448 592 $@

# V2 (SE)

vmm_patch_v2.h: vmm/patched_v2.bin vmm/original_v2.bin vmm/reloc_v2.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) 98v2 vmm_patch_v2 1052 448 604 $@

# ME (2 parts)

vmm_patch_me1.h: vmm/patchedme.bin vmm/originalme.bin vmm/relocme.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) me vmm_patch_me1 16320 12 461 $@

vmm_patch_me2.h: vmm/patchedme.bin vmm/originalme.bin vmm/relocme.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) me vmm_patch_me2 16320 16160 160 $@

vmm/fasmdiff.h: vmm/original.bin vmm/dump.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) fasmdiff $@ vmm/dump.bin vmm/original.bin

vmm/fasmdiff_v2.h: vmm/original_v2.bin vmm/dump_v2.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) fasmdiff_v2 $@ vmm/dump_v2.bin vmm/original_v2.bin

vmm/fasmdiff_me.h: vmm/originalme.bin vmm/dumpme.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) fasmdiff_me $@ vmm/dumpme.bin vmm/originalme.bin 0x1D8 0x3F20

vmm/fasmdiff_old.h: vmm/original_old.bin vmm/dump_old.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) fasmdiff_old $@ vmm/dump_old.bin vmm/original_old.bin

vmm/fasmdiff_old_v2.h: vmm/original_old_v2.bin vmm/dump_old_v2.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) fasmdiff_old_v2 $@ vmm/dump_old_v2.bin vmm/original_old_v2.bin

vmm_patch_old.h: vmm/patched.bin vmm/original_old.bin vmm/reloc_old.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) 98_old vmm_patch_old 1040 448 592 $@
	
vmm_patch_old_v2.h: vmm/patched_v2.bin vmm/original_old_v2.bin vmm/reloc_old_v2.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) 98_old_v2 vmm_patch_old_v2 1052 448 604 $@

vmm_patch_simple.h: vmm/patched_simple.bin vmm/original_simple.bin vmm/reloc_simple.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) 98_simple vmm_patch_simple 1040 448 592 $@
	
vmm_patch_simple_v2.h: vmm/patched_simple_v2.bin vmm/original_simple_v2.bin vmm/reloc_simple_v2.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) 98_simple_v2 vmm_patch_simple_v2 1052 448 604 $@

# cpu speed V1
cpuspeed/speed_v1.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Dversion_1 cpuspeed/speed_v1.asm -o $@

cpuspeed/speed_v1_orig.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_1 cpuspeed/speed_v1.asm -o $@

cpuspeed/speed_v1_reloc.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_1 cpuspeed/speed_v1.asm -o $@

cpuspeed/speed_v1_diff.h: cpuspeed/speed_v1_dump.bin cpuspeed/speed_v1_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v1_diff $@ cpuspeed/speed_v1_dump.bin cpuspeed/speed_v1_orig.bin

cpuspeed_patch_v1.h: cpuspeed/speed_v1.bin cpuspeed/speed_v1_orig.bin cpuspeed/speed_v1_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v1 cpuspeed_patch_v1 53 0 53 $@


# cpu speed V2
cpuspeed/speed_v2.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Dversion_2 cpuspeed/speed_v1.asm -o $@

cpuspeed/speed_v2_orig.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_2 cpuspeed/speed_v1.asm -o $@

cpuspeed/speed_v2_reloc.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_2 cpuspeed/speed_v1.asm -o $@

cpuspeed/speed_v2_diff.h: cpuspeed/speed_v2_dump.bin cpuspeed/speed_v2_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v2_diff $@ cpuspeed/speed_v2_dump.bin cpuspeed/speed_v2_orig.bin

cpuspeed_patch_v2.h: cpuspeed/speed_v2.bin cpuspeed/speed_v2_orig.bin cpuspeed/speed_v2_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v2 cpuspeed_patch_v2 53 0 53 $@

# cpu speed V4
cpuspeed/speed_v4.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Dversion_4 cpuspeed/speed_v1.asm -o $@

cpuspeed/speed_v4_orig.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_4 cpuspeed/speed_v1.asm -o $@

cpuspeed/speed_v4_reloc.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_4 cpuspeed/speed_v1.asm -o $@

cpuspeed/speed_v4_diff.h: cpuspeed/speed_v4_dump.bin cpuspeed/speed_v4_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v4_diff $@ cpuspeed/speed_v4_dump.bin cpuspeed/speed_v4_orig.bin

cpuspeed_patch_v4.h: cpuspeed/speed_v4.bin cpuspeed/speed_v4_orig.bin cpuspeed/speed_v4_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v4 cpuspeed_patch_v4 53 0 53 $@

# cpu speed V3
cpuspeed/speed_v3.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v3.asm
	$(CPP) -nostdinc -E -P -Dversion_3 cpuspeed/speed_v3.asm -o $@

cpuspeed/speed_v3_orig.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v3.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_3 cpuspeed/speed_v3.asm -o $@

cpuspeed/speed_v3_reloc.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v3.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_3 cpuspeed/speed_v3.asm -o $@

cpuspeed/speed_v3_diff.h: cpuspeed/speed_v3_dump.bin cpuspeed/speed_v3_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v3_diff $@ cpuspeed/speed_v3_dump.bin cpuspeed/speed_v3_orig.bin

cpuspeed_patch_v3.h: cpuspeed/speed_v3.bin cpuspeed/speed_v3_orig.bin cpuspeed/speed_v3_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v3 cpuspeed_patch_v3 44 0 44 $@

# cpu speed V5
cpuspeed/speed_v5.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Dversion_5 cpuspeed/speed_v5.asm -o $@

cpuspeed/speed_v5_orig.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_5 cpuspeed/speed_v5.asm -o $@

cpuspeed/speed_v5_reloc.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_5 cpuspeed/speed_v5.asm -o $@

cpuspeed/speed_v5_diff.h: cpuspeed/speed_v5_dump.bin cpuspeed/speed_v5_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v5_diff $@ cpuspeed/speed_v5_dump.bin cpuspeed/speed_v5_orig.bin

cpuspeed_patch_v5.h: cpuspeed/speed_v5.bin cpuspeed/speed_v5_orig.bin cpuspeed/speed_v5_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v5 cpuspeed_patch_v5 69 0 69 $@

# cpu speed V6
cpuspeed/speed_v6.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Dversion_6 cpuspeed/speed_v5.asm -o $@

cpuspeed/speed_v6_orig.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_6 cpuspeed/speed_v5.asm -o $@

cpuspeed/speed_v6_reloc.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_6 cpuspeed/speed_v5.asm -o $@

cpuspeed/speed_v6_diff.h: cpuspeed/speed_v6_dump.bin cpuspeed/speed_v6_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v6_diff $@ cpuspeed/speed_v6_dump.bin cpuspeed/speed_v6_orig.bin

cpuspeed_patch_v6.h: cpuspeed/speed_v6.bin cpuspeed/speed_v6_orig.bin cpuspeed/speed_v6_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v6 cpuspeed_patch_v6 69 0 69 $@

# cpu speed V7
cpuspeed/speed_v7.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Dversion_7 cpuspeed/speed_v5.asm -o $@

cpuspeed/speed_v7_orig.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_7 cpuspeed/speed_v5.asm -o $@

cpuspeed/speed_v7_reloc.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v5.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_7 cpuspeed/speed_v5.asm -o $@

cpuspeed/speed_v7_diff.h: cpuspeed/speed_v7_dump.bin cpuspeed/speed_v7_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v7_diff $@ cpuspeed/speed_v7_dump.bin cpuspeed/speed_v7_orig.bin

cpuspeed_patch_v7.h: cpuspeed/speed_v7.bin cpuspeed/speed_v7_orig.bin cpuspeed/speed_v7_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v7 cpuspeed_patch_v7 69 0 69 $@

# cpu speed V8
cpuspeed/speed_v8.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v8.asm
	$(CPP) -nostdinc -E -P -Dversion_8 cpuspeed/speed_v8.asm -o $@

cpuspeed/speed_v8_orig.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v8.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_8 cpuspeed/speed_v8.asm -o $@

cpuspeed/speed_v8_reloc.asm.gen: cpuspeed/speed.inc cpuspeed/speed_v8.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_8 cpuspeed/speed_v8.asm -o $@

cpuspeed/speed_v8_diff.h: cpuspeed/speed_v8_dump.bin cpuspeed/speed_v8_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speed_v8_diff $@ cpuspeed/speed_v8_dump.bin cpuspeed/speed_v8_orig.bin

cpuspeed_patch_v8.h: cpuspeed/speed_v8.bin cpuspeed/speed_v8_orig.bin cpuspeed/speed_v8_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speed_v8 cpuspeed_patch_v8 60 0 60 $@


# cpu speed NDIS.VXD V1
cpuspeed/speedndis_v1.asm.gen: cpuspeed/speed.inc cpuspeed/speedndis_v1.asm
	$(CPP) -nostdinc -E -P -Dversion_1 cpuspeed/speedndis_v1.asm -o $@

cpuspeed/speedndis_v1_orig.asm.gen: cpuspeed/speed.inc cpuspeed/speedndis_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_1 cpuspeed/speedndis_v1.asm -o $@

cpuspeed/speedndis_v1_reloc.asm.gen: cpuspeed/speed.inc cpuspeed/speedndis_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_1 cpuspeed/speedndis_v1.asm -o $@

cpuspeed/speedndis_v1_diff.h: cpuspeed/speedndis_v1_dump.bin cpuspeed/speedndis_v1_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speedndis_v1_diff $@ cpuspeed/speedndis_v1_dump.bin cpuspeed/speedndis_v1_orig.bin

cpuspeed_ndis_patch_v1.h: cpuspeed/speedndis_v1.bin cpuspeed/speedndis_v1_orig.bin cpuspeed/speedndis_v1_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speedndis_v1 cpuspeed_ndis_patch_v1 112 0 112 $@

# cpu speed NDIS.VXD V3
cpuspeed/speedndis_v3.asm.gen: cpuspeed/speed.inc cpuspeed/speedndis_v1.asm
	$(CPP) -nostdinc -E -P -Dversion_3 cpuspeed/speedndis_v1.asm -o $@

cpuspeed/speedndis_v3_orig.asm.gen: cpuspeed/speed.inc cpuspeed/speedndis_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_3 cpuspeed/speedndis_v1.asm -o $@

cpuspeed/speedndis_v3_reloc.asm.gen: cpuspeed/speed.inc cpuspeed/speedndis_v1.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_3 cpuspeed/speedndis_v1.asm -o $@

cpuspeed/speedndis_v3_diff.h: cpuspeed/speedndis_v3_dump.bin cpuspeed/speedndis_v3_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speedndis_v3_diff $@ cpuspeed/speedndis_v3_dump.bin cpuspeed/speedndis_v3_orig.bin

cpuspeed_ndis_patch_v3.h: cpuspeed/speedndis_v3.bin cpuspeed/speedndis_v3_orig.bin cpuspeed/speedndis_v3_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speedndis_v3 cpuspeed_ndis_patch_v3 112 0 112 $@

# cpu speed NDIS.VXD V2
cpuspeed/speedndis_v2.asm.gen: cpuspeed/speed.inc cpuspeed/speedndis_v2.asm
	$(CPP) -nostdinc -E -P -Dversion_2 cpuspeed/speedndis_v2.asm -o $@

cpuspeed/speedndis_v2_orig.asm.gen: cpuspeed/speed.inc cpuspeed/speedndis_v2.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_2 cpuspeed/speedndis_v2.asm -o $@

cpuspeed/speedndis_v2_reloc.asm.gen: cpuspeed/speed.inc cpuspeed/speedndis_v2.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_2 cpuspeed/speedndis_v2.asm -o $@

cpuspeed/speedndis_v2_diff.h: cpuspeed/speedndis_v2_dump.bin cpuspeed/speedndis_v2_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speedndis_v2_diff $@ cpuspeed/speedndis_v2_dump.bin cpuspeed/speedndis_v2_orig.bin

cpuspeed_ndis_patch_v2.h: cpuspeed/speedndis_v2.bin cpuspeed/speedndis_v2_orig.bin cpuspeed/speedndis_v2_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speedndis_v2 cpuspeed_ndis_patch_v2 128 0 128 $@

# cpu speed NDIS.386 V4
cpuspeed/speedndis_v4.asm.gen: cpuspeed/speed.inc cpuspeed/speedndis_v4.asm
	$(CPP) -nostdinc -E -P -Dversion_4 cpuspeed/speedndis_v4.asm -o $@

cpuspeed/speedndis_v4_orig.asm.gen: cpuspeed/speed.inc cpuspeed/speedndis_v4.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Dversion_4 cpuspeed/speedndis_v4.asm -o $@

cpuspeed/speedndis_v4_reloc.asm.gen: cpuspeed/speed.inc cpuspeed/speedndis_v4.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate -Dversion_4 cpuspeed/speedndis_v4.asm -o $@

cpuspeed/speedndis_v4_diff.h: cpuspeed/speedndis_v4_dump.bin cpuspeed/speedndis_v4_orig.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) speedndis_v4_diff $@ cpuspeed/speedndis_v4_dump.bin cpuspeed/speedndis_v4_orig.bin

cpuspeed_ndis_patch_v4.h: cpuspeed/speedndis_v4.bin cpuspeed/speedndis_v4_orig.bin cpuspeed/speedndis_v4_reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) speedndis_v4 cpuspeed_ndis_patch_v4 113 0 113 $@

patch.g.o: vmm_patch.h vmm_patch_v2.h vmm_patch_me1.h vmm_patch_me2.h cpuspeed_ndis_patch_v1.h cpuspeed_ndis_patch_v2.h cpuspeed_ndis_patch_v3.h cpuspeed_ndis_patch_v4.h \
  cpuspeed_patch_v1.h cpuspeed_patch_v2.h cpuspeed_patch_v3.h cpuspeed_patch_v4.h cpuspeed_patch_v5.h cpuspeed_patch_v6.h cpuspeed_patch_v7.h cpuspeed_patch_v8.h \
  vmm_patch_old.h vmm_patch_old_v2.h vmm_patch_simple.h vmm_patch_simple_v2.h

fasmdiff: vmm/fasmdiff.h vmm/fasmdiff_v2.h vmm/fasmdiff_me.h \
  cpuspeed/speed_v1_diff.h cpuspeed/speed_v2_diff.h cpuspeed/speed_v3_diff.h cpuspeed/speed_v4_diff.h \
  cpuspeed/speedndis_v1_diff.h cpuspeed/speedndis_v2_diff.h cpuspeed/speedndis_v3_diff.h cpuspeed/speedndis_v4_diff.h \
  cpuspeed/speed_v5_diff.h cpuspeed/speed_v6_diff.h cpuspeed/speed_v7_diff.h cpuspeed/speed_v8_diff.h \
  vmm/fasmdiff_old.h vmm/fasmdiff_old_v2.h

soliddiff: rloew/vcache_patch_v1.h rloew/vcache_patch_v2.h rloew/vcache_patch_v3.h \
  rloew/vmm98_patch_v1.h rloew/vmm98_patch_v2.h \
  rloew/vmmme_patch_v1.h rloew/vmmme_patch_v2.h \
  rloew/vmm95_patch_v1.h \
  rloew/w3_patch_v1.h rloew/w3_patch_v2.h

rloew/vcache_patch_v1.h: bindiff$(HOST_SUFIX)
	$(RUNPATH)bindiff$(HOST_SUFIX) vcache_v1 rloew/dump/vcache98.org rloew/dump/vcache98.fix $@

rloew/vcache_patch_v2.h: bindiff$(HOST_SUFIX)
	$(RUNPATH)bindiff$(HOST_SUFIX) vcache_v2 rloew/dump/vcache95.org rloew/dump/vcache95.fix $@

rloew/vcache_patch_v3.h: bindiff$(HOST_SUFIX)
	$(RUNPATH)bindiff$(HOST_SUFIX) vcache_v3 rloew/dump/vcacheme.org rloew/dump/vcacheme.fix $@

# 98SE
rloew/vmm98_patch_v1.h: bindiff$(HOST_SUFIX)
	$(RUNPATH)bindiff$(HOST_SUFIX) vmm98_v1 rloew/dump/vmm98.org rloew/dump/vmm98.fix $@

# 98FE
rloew/vmm98_patch_v2.h: bindiff$(HOST_SUFIX)
	$(RUNPATH)bindiff$(HOST_SUFIX) vmm98_v2 rloew/dump/vmm98fe.org rloew/dump/vmm98fe.fix $@

# 95
rloew/vmm95_patch_v1.h: bindiff$(HOST_SUFIX)
	$(RUNPATH)bindiff$(HOST_SUFIX) vmm95_v1 rloew/dump/vmm95.org rloew/dump/vmm95.fix $@

# ME
rloew/vmmme_patch_v1.h: bindiff$(HOST_SUFIX)
	$(RUNPATH)bindiff$(HOST_SUFIX) vmmme_v1 rloew/dump/vmmme.org rloew/dump/vmmme.fix $@

# ME+Q29677
rloew/vmmme_patch_v2.h: bindiff$(HOST_SUFIX)
	$(RUNPATH)bindiff$(HOST_SUFIX) vmmme_v2 rloew/dump/vmmme_q296773.org rloew/dump/vmmme_q296773.fix $@

# 98 W3 loader patch
rloew/w3_patch_v1.h: bindiff$(HOST_SUFIX)
	$(RUNPATH)bindiff$(HOST_SUFIX) w3_v1 rloew/dump/w3_98.org rloew/dump/w3_98.fix $@ 65536

# 95 W3 loader patch
rloew/w3_patch_v2.h: bindiff$(HOST_SUFIX)
	$(RUNPATH)bindiff$(HOST_SUFIX) w3_v2 rloew/dump/w3_95.org rloew/dump/w3_95.fix $@

doscross$(HOST_SUFIX): builder/doscross.c version.h help.h batch.h
	@$(HOST_CC) $(HOST_CFLAGS) -o $@ $< $(HOST_LDFLAGS)

get-version: doscross$(HOST_SUFIX)
	@$(RUNPATH)doscross$(HOST_SUFIX) version

get-help: doscross$(HOST_SUFIX)
	@$(RUNPATH)doscross$(HOST_SUFIX) help $(OUTNAME)

clean:
	-$(RM) $(OBJS_OUT)
	-$(RM) $(OBJS_HOST)
	-$(RM) $(OBJS_MAKEPATCH)
	-$(RM) $(OBJS_MAKEDIFF)
	-$(RM) $(OBJS_BINDIFF)
	-$(RM) *.tmp
	-$(RM) *.res
	-cd vmm && $(RM) *.gen
	-cd vmm && $(RM) *.tmp
	-cd vmm && $(RM) *.res
	-cd cpuspeed && $(RM) *.gen
	-cd cpuspeed && $(RM) *.tmp
	-$(RM) $(OUTNAME)
	-$(RM) makepatch$(HOST_SUFIX)
	-$(RM) makediff$(HOST_SUFIX)
	-$(RM) bindiff$(HOST_SUFIX)
	-$(RM) doscross$(HOST_SUFIX)
	-$(RM) $(TESTS)
