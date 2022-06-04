# cat/type
CAT=$(if $(filter $(OS),Windows_NT),type,cat)
# move/mv
#MV=$(if $(filter $(OS),Windows_NT),cmd /C move /Y,mv -f)
MV=mv
RUNPATH=$(if $(filter $(OS),Windows_NT),.\,./)
NULLOUT=$(if $(filter $(OS),Windows_NT),NUL,/dev/null)

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
DEPS_GUEST=Makefile system/bitstream.h vmm_patch.h version.h

OBJS_GUEST=mspack/cabc.g.o mspack/cabd.g.o mspack/crc32.g.o mspack/hlpc.g.o mspack/hlpd.g.o mspack/chmc.g.o mspack/chmd.g.o mspack/kwajc.g.o mspack/kwajd.g.o mspack/litc.g.o \
 mspack/litd.g.o mspack/lzssd.g.o mspack/lzxc.g.o mspack/lzxd.g.o mspack/mszipc.g.o mspack/mszipd.g.o mspack/oabc.g.o mspack/oabd.g.o mspack/qtmd.g.o mspack/system.g.o \
 mspack/szddc.g.o mspack/szddd.g.o system/filesystem.g.o system/bpatcher.g.o pe/pew.g.o pe/ds_compress.g.o pe/ds_decompress.g.o
 
OBJS_OUT=$(OBJS_GUEST) unpacker.g.o patch.g.o trace.g.o patcher9x.g.o

OBJS_HOST=system/bpatcher.h.o

TESTS=test_bitstream$(SUFIX) test_pe$(SUFIX) test_pe2$(SUFIX) test_ds$(SUFIX) test_ds_compress$(SUFIX) test_w3$(SUFIX) test_patch$(SUFIX) test_ds2$(SUFIX)

all: $(OUTNAME)

tests: $(TESTS)

.PHONY: all tests clean

%.g.o: %.c $(DEPS_GUEST)
	$(GUEST_CC) $(GUEST_CFLAGS) -c -o $@ $<

%.h.o: %.c $(DEPS_HOST)
	$(HOST_CC) $(HOST_CFLAGS) -c -o $@ $<

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

vmm/patched.bin: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P vmm/FlushMappedCacheBlock.asm -o vmm/patched.gen
	$(FASM) vmm/patched.gen vmm/patched.bin
 
vmm/original.bin: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode vmm/FlushMappedCacheBlock.asm -o vmm/original.gen
	$(FASM) vmm/original.gen vmm/original.bin

vmm/reloc.bin: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate vmm/FlushMappedCacheBlock.asm -o vmm/reloc.gen
	$(FASM) vmm/reloc.gen vmm/reloc.bin

vmm/patchedme.bin: vmm/FlushMappedCacheBlockME.asm
	$(CPP) -nostdinc -E -P vmm/FlushMappedCacheBlockME.asm -o vmm/patchedme.gen
	$(FASM) vmm/patchedme.gen vmm/patchedme.bin
 
vmm/originalme.bin: vmm/FlushMappedCacheBlockME.asm
	$(CPP) -nostdinc -E -P -Doriginalcode vmm/FlushMappedCacheBlockME.asm -o vmm/originalme.gen
	$(FASM) vmm/originalme.gen vmm/originalme.bin

vmm/relocme.bin: vmm/FlushMappedCacheBlockME.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate vmm/FlushMappedCacheBlockME.asm -o vmm/relocme.gen
	$(FASM) vmm/relocme.gen vmm/relocme.bin

makepatch$(HOST_SUFIX): $(OBJS_HOST) vmm/makepatch.h.o
	$(HOST_CC) $(HOST_CFLAGS) -o makepatch$(HOST_SUFIX) $(OBJS_HOST) vmm/makepatch.h.o $(HOST_LDFLAGS)
	
makediff$(HOST_SUFIX): $(OBJS_HOST) vmm/makediff.h.o
	$(HOST_CC) $(HOST_CFLAGS) -o makediff$(HOST_SUFIX) $(OBJS_HOST) vmm/makediff.h.o $(HOST_LDFLAGS)

vmm_patch.h: vmm/patched.bin vmm/original.bin vmm/reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) 1040 448 492 > vmm_patch.tmp
	$(MV) vmm_patch.tmp vmm_patch.h

vmm/fasmdiff.h: vmm/original.bin vmm/dump.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) fasmdiff vmm/dump.bin vmm/original.bin > vmm/fasmdiff.tmp
	$(MV) vmm/fasmdiff.tmp vmm/fasmdiff.h

vmm/fasmdiff_me.h: vmm/originalme.bin vmm/dumpme.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) fasmdiff_me vmm/dumpme.bin vmm/originalme.bin 0x1D8 0x3F20 > vmm/fasmdiff_me.tmp
	$(MV) vmm/fasmdiff_me.tmp vmm/fasmdiff_me.h

clean:
	-$(RM) *.o
	-cd mspack && $(RM) *.o
	-cd pe && $(RM) *.o
	-cd system && $(RM) *.o
	-cd test && $(RM) *.o
	-cd vmm && $(RM) *.o
	-cd vmm && $(RM) *.gen
	-$(RM) $(OUTNAME)
	-$(RM) makepatch$(HOST_SUFIX)
	-$(RM) makediff$(HOST_SUFIX)
	-$(RM) $(TESTS)
