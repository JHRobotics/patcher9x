# Patch for Windows 9x to fix TLB invalidation bug

MS Windows 95/98/ME won't run on newer CPU (even in virtual machine) due to "TLB invalidation bug".
The Bug is described there: https://blog.stuffedcow.net/2015/08/win9x-tlb-invalidation-bug/

![Bug animation on Windows 98](/doc/shell32.gif)

## Requirements
This patch works for Windows 98 / Windows 98 SE. Work on port to other 9x system (95, ME) is in progress.

## Installation
Simplest way is download bootable floppy image. After boot (you will see `A:\`) run
```
patch9x
```
Patch will be run in interactive mode and the default strategy (extract `VMM.VXD` from `VMM32.VMX` and apply patch)
is probably best way. After reboot operation system could start successfully.

![Successfuly working Windows 98](/doc/intel-i5-1135.gif)

### Patching installation media
Copy content of win98 folder from CD / extract it from ISO image. Run 
```
patcher9x /path/to/folder/win98
```
If patch success you can copy file `VMM32.VXD` back to image. Windows installer primary takes files from
installation folder and if it not found them it'll scan CAB archives.

**Please note, that file `VMM32.VXD` from installation IS NOT SAME as file in `Windows/system` folder.
  Don't interchange them! See _Patching process_ section to know more about VMM files.**

## Build from source

For build from source you need:
- GNU C compiler compatible C compiler (minimal version 4.6, MinGW or DJGPP works)
- Flat assemler (https://flatassembler.net/)
- GNU Make (minimal version 3.81)

For building binary for your computer simply type
```
make
```

For cross compiling type specify `HOST_CC` and `GUEST_CC` variables to choose compiler, for example
cross compiling for 32bit Windows:
```
make HOST_CC=gcc GUEST_CC=mingw-w64-i686-gcc
```

For producing production binary pass `RELEASE=1` to `make`.

Afrer compile you can strip binary (reduce some space) by
```
make strip
```

There is special profile for DOS cross-compilation, if you have DJGPP compiler, you can produce
DOS executable this way:
```
make RELEASE=1 PROFILE=djgpp
make strip
```

**Executable file name for most real operation systems called `patcher9x`. For dos called `patch9x.exe`
  because file names are limited to 8+3 characters.**

## Patching proccess

Patch self is relative simple - injecting 2 instruction (`mov ecx,cr3` and `mov cr3,ecx`) to code
in `VMM.VXD` driver. Patch totally modifies 29 bytes in this file.

Problem is that file `VMM.VXD` isn't on normal state on HDD.

File `VMM.VXD` if part of `VMM32.VXD` which is compressed archive of more `VXD` drivers. File `VMM32.VXD`
isn't generic but is generated by installer exclusively for your HW configuration.

System for loading drivers looks first in `SYSTEM\VMM32` folder, and if driver isn't there, then it will
search in `VMM32.VXD` file.

[More info about W3/W4 files](doc/VXDLIB_UTF8.txt)

