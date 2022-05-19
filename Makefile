# cat/type
CAT=$(if $(filter $(OS),Windows_NT),type,cat)
# move/mv
#MV=$(if $(filter $(OS),Windows_NT),cmd /C move /Y,mv -f)
MV=mv
RUNPATH=$(if $(filter $(OS),Windows_NT),.\,./)

SUFIX=
ifeq ($(filter $(OS),Windows_NT),Windows_NT)
  SUFIX=.exe
  HOST_SUFIX=.exe
endif

FASM=fasm
HOST_CC=gcc
HOST_CFLAGS=-g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -I./mspack -I./system -I./pe

OUTNAME=patcher9x$(SUFIX)

CPP   = cpp
CC    ?= gcc
STRIP ?= strip

ifdef PROFILE
  ifeq ($(PROFILE),djgpp)
    CC      := i586-pc-msdosdjgpp-gcc
    SUFIX   := .exe
    STRIP   := i586-pc-msdosdjgpp-strip
    OUTNAME := patch9x$(SUFIX)
  endif
endif


DEPS_HOST=Makefile system/bitstream.h
DEPS_GUEST=Makefile system/bitstream.h vmm_patch.h
ifdef RELEASE
  CFLAGS ?= -g0 -static -Os -Wall -Wextra -Wno-unused-parameter -Wno-unused-function
else
  CFLAGS ?= -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function
endif

CFLAGS := $(CFLAGS) -I./mspack -I./system -I./pe

OBJS_GUEST=mspack/cabc.g.o mspack/cabd.g.o mspack/crc32.g.o mspack/hlpc.g.o mspack/hlpd.g.o mspack/chmc.g.o mspack/chmd.g.o mspack/kwajc.g.o mspack/kwajd.g.o mspack/litc.g.o \
 mspack/litd.g.o mspack/lzssd.g.o mspack/lzxc.g.o mspack/lzxd.g.o mspack/mszipc.g.o mspack/mszipd.g.o mspack/oabc.g.o mspack/oabd.g.o mspack/qtmd.g.o mspack/system.g.o \
 mspack/szddc.g.o mspack/szddd.g.o system/filesystem.g.o system/bpatcher.g.o pe/pew.g.o pe/ds_compress.g.o pe/ds_decompress.g.o
 
OBJS_OUT=$(OBJS_GUEST) unpacker.g.o patch.g.o trace.g.o patcher9x.g.o

OBJS_HOST=system/bpatcher.h.o

all: $(OUTNAME)

TESTS=test_bitstream$(SUFIX) test_pe$(SUFIX) test_pe2$(SUFIX) test_ds$(SUFIX) test_ds_compress$(SUFIX) test_w3$(SUFIX) test_patch$(SUFIX)

tests: $(TESTS)

.PHONY: all tests clean

%.g.o: %.c $(DEPS_GUEST)
	$(CC) $(CFLAGS) -c -o $@ $<

%.h.o: %.c $(DEPS_HOST)
	$(HOST_CC) $(HOST_CFLAGS) -c -o $@ $<

$(OUTNAME): $(OBJS_OUT)
	$(CC) $(CFLAGS) -o $(OUTNAME) $(OBJS_OUT)
	
strip: $(OUTNAME)
	$(STRIP) -s $(OUTNAME)

test_bitstream$(SUFIX): $(OBJS_GUEST) test/test_bitstream.g.o
	$(CC) $(CFLAGS) -o test_bitstream$(SUFIX) $(OBJS_GUEST) test/test_bitstream.g.o

test_pe$(SUFIX): $(OBJS_GUEST) test/test_pe.g.o
	$(CC) $(CFLAGS) -o test_pe$(SUFIX) $(OBJS_GUEST) test/test_pe.g.o

test_pe2$(SUFIX): $(OBJS_GUEST) test/test_pe2.g.o
	$(CC) $(CFLAGS) -o test_pe2$(SUFIX) $(OBJS_GUEST) test/test_pe2.g.o

test_w3$(SUFIX): $(OBJS_GUEST) test/test_w3.g.o
	$(CC) $(CFLAGS) -o test_w3$(SUFIX) $(OBJS_GUEST) test/test_w3.g.o

test_ds$(SUFIX): $(OBJS_GUEST) test/test_ds.g.o
	$(CC) $(CFLAGS) -o test_ds$(SUFIX) $(OBJS_GUEST) test/test_ds.g.o
	
test_ds_compress$(SUFIX): $(OBJS_GUEST) test/test_ds_compress.g.o
	$(CC) $(CFLAGS) -o test_ds_compress$(SUFIX) $(OBJS_GUEST) test/test_ds_compress.g.o
	
test_patch$(SUFIX): $(OBJS_GUEST) test/test_patch.g.o
	$(CC) $(CFLAGS) -o test_patch$(SUFIX) $(OBJS_GUEST) test/test_patch.g.o

vmm/patched.bin: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P vmm/FlushMappedCacheBlock.asm -o vmm/patched.gen
	$(FASM) vmm/patched.gen vmm/patched.bin
 
vmm/original.bin: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode vmm/FlushMappedCacheBlock.asm -o vmm/original.gen
	$(FASM) vmm/original.gen vmm/original.bin

vmm/reloc.bin: vmm/FlushMappedCacheBlock.asm
	$(CPP) -nostdinc -E -P -Doriginalcode -Drelocate vmm/FlushMappedCacheBlock.asm -o vmm/reloc.gen
	$(FASM) vmm/reloc.gen vmm/reloc.bin

makepatch$(HOST_SUFIX): $(OBJS_HOST) vmm/makepatch.h.o
	$(HOST_CC) $(HOST_CFLAGS) -o makepatch$(HOST_SUFIX) $(OBJS_HOST) vmm/makepatch.h.o
	
makediff$(HOST_SUFIX): $(OBJS_HOST) vmm/makediff.h.o
	$(HOST_CC) $(HOST_CFLAGS) -o makediff$(HOST_SUFIX) $(OBJS_HOST) vmm/makediff.h.o

vmm_patch.h: vmm/patched.bin vmm/original.bin vmm/reloc.bin makepatch$(HOST_SUFIX)
	$(RUNPATH)makepatch$(HOST_SUFIX) 1040 448 492 > vmm_patch.tmp
	$(MV) vmm_patch.tmp vmm_patch.h

vmm/fasmdiff.h: vmm/original.bin vmm/dump.bin makediff$(HOST_SUFIX)
	$(RUNPATH)makediff$(HOST_SUFIX) > vmm/fasmdiff.tmp
	$(MV) vmm/fasmdiff.tmp vmm/fasmdiff.h

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
