# Patch for Windows 95/98/98 SE/Me to fix CPU issues
Virtualization of Microsoft Windows 9x systems is a bit problematic due to 2 major bugs:
[TLB invalidation bug](#patch-for-windows-98me-to-fix-tlb-invalidation-bug) and [CPU speed limit bug](#patch-for-windows-9598-to-fix-cpu-speed-limit-bug).

This program contains a set of patches to fix these bugs, and can be booted from a floppy on a virtual machine. It either applies the patch to the installed system, or it patches the installation files in order to create (relatively) bug-free installation media.

## Patch for Windows 98/Me to fix TLB invalidation bug

MS Windows 98 won't run on newer CPU (even in a virtual machine) due to the "TLB invalidation bug". The bug is described here: https://blog.stuffedcow.net/2015/08/win9x-tlb-invalidation-bug/

![Bug animation on Windows 98](/doc/shell32.gif)

If you want to run a virtual machine without restriction with Windows 98 on AMD Zen 2 and newer (Ryzen 3000+) or Intel Core 11th generation and newer (code names Rocket Lake, Tiger Lake), you probably need this patch.

## Patch for Windows 95/98 to fix CPU speed limit bug

Windows 95 and 98 FE (first edition) won't run on any CPU with frequency of ~2GHz and more due to dividing by zero in the CPU speed test. The bug is described and located here: https://www.betaarchive.com/forum/viewtopic.php?t=29224

![Bug animation on Windows 95](/doc/ndis.gif)

There is also an older patch for this bug [FIX95CPU](http://www.tmeeco.eu/9X4EVER/GOODIES/FIX95CPU_V3_FINAL.ZIP), but it's only for Windows 95, doesn't integrate the "divide by zero" protection and also it's incrementing the number of cycles in the testing loop only 10 times of original which probably isn't enough for future CPUs. You can test your CPU speed with this command:

```
patcher9x -cputest
```

## Requirements

Currently supported operation systems:

- Windows 95
- Windows 98
- Windows 98 SE
- Windows Me

Windows 95, 98 and 98 SE have the CPU speed limit bug. Windows 98 SE is a special case because it has "divide by zero" protection, but it also has a short loop test, so its timing information is probably useless and could potentially lead to other bugs.

Windows 98, 98 SE and Me have the TLB invalidation bug. The Millennium edition has this bug in the code, but from my observation, the system calls this code very rarely (for example with driver install/system update), so the bug isn't as obvious as in Windows 98. An older version of my patcher program also has a special mode for Windows Me, but in the current version it is no longer needed.

Some updates install newer version of some files, for example `VMM.VXD`: Q242161 (98 FE), Q288430 (98 SE), ME296773 (Me); `NTKERN.VXD`, `IOS.VXD`, `ESDI_506.PDR`, `SCSIPORT.PDR` and `NDIS.VXD`: Unofficial FIX95CPU. The patcher is compatible with these updates but is required to apply patches again after install. The best way is probably to run patcher from Windows (DOS or 32-bit version) before reboot - Windows 9x does not have any system files protection and you can overwrite system files in the running system. If you do not catch it and the system is rebooted to an error message, you will need to boot from a bootable floppy and start the patcher from it.

## Updating

If you already applied this patch, you don't to need install patcher update - currently all the changes are for the patcher program, the patched code is the same. If you want to overwrite previous patches, you can safely run the patcher again and it automatically determines which files are suitable for patching and which are already patched.

## Download

Binary files and bootable floppy image are in [Releases](https://github.com/JHRobotics/patcher9x/releases/)

IMA file is a bootable floppy (FREEDOS) usable in a virtual machine to simple boot and patch the installed system. Binaries for win32 are Windows 98 compatible, so they can be run from safe mode (Hold CTRL on start-up). Binaries for other systems are for creating patched installation (in theory you can mount virtual HDD image and patch installed system on it, but doing it using boot the floppy is much simpler).

## Installation

The simplest way is by downloading bootable floppy image. After booting (you will see `A:\`), run:
```
patch9x
```
Patch will be run in interactive mode and the default strategy (*patch files, VMM32.VXD will be patched directly*) is probably the best way even for later updates. After rebooting operation system should start successfully.

![Successfuly working Windows 98 - Intel](/doc/intel-i5-1135.gif)

![Successfuly working Windows 98 - AMD](/doc/amd-5-3500u.png)

## Operation modes

### Interactive mode
This is default mode, the program asking questions and user answear. You can just double click on EXE (or type `patch9x` to DOS command prompt) and program guide to you in patching process. For Linux build, the help is prinded if no arguments are given (default behaviour for UNIX programs) so you need specify *path*.

### Automatic mode
Same as interactive but don't ask anything. Can be enabled with `-auto` switch and *path* to CAB files or 9x WINDOWS/SYSTEM directory needs to be specified.

### Batch mode
In this mode program operate with single steps. Examples:

Extract `VMM32.VXD` from instalation media
```
patcher9x --cabs-extract D:\WIN98 VMM32.VXD
```

Extract VMM.VXD from VMM32.VXD
```
patcher9x --vxd-extract VMM32.VXD VMM.VXD
```

Patch individual File
```
patcher9x --patch-tlb VMM.VXD
```

Patch VXD archive
```
patcher9x --patch-all VMM32.VXD
```

## Boot floppy
Boot floppy now contain CD driver and few utilities to prepare system disk. If you wish run Windows Installer from boot floppy, add `/NM` switch to `setup.exe` (because of different memory manager, the setup cannot determine real RAM size). Utilities are listed in [boot/info.txt](boot/info.txt).

## Patching installation media
Copy the content of *win9x* folder (or *win95* - for Windows 95 or *win98* - for Windows 98)
from a CD / extract it from an ISO image. Then run: 
```
patcher9x /path/to/folder/win98
```
If the patch is successful, you can copy the modified files back to the image. These are the files marked with the 'N' flag in the patcher output. Or it will be one of these:
- `VMM32.VXD`
- `VMM.VXD`
- `NTKERN.VXD`
- `IOS.VXD`
- `ESDI_506.PDR`
- `SCSIPORT.PDR`
- `NDIS.VXD`

The Windows installer primarily takes files from the installation folder and if it can't find them, it'll scan the CAB archives instead.

For Windows 95, it is better to patch the installation media, or you will need to install the patch twice - after the first reboot and again after the installation of the network.

**Please note, that file `VMM32.VXD` from installation IS NOT THE SAME as the file in `Windows/system` folder. Don't interchange them! See the _Patching process_ section to know more about the VMM files.**

## More informations

Check **patcher9x** thread at Vogons: https://www.vogons.org/viewtopic.php?f=24&t=88284

Check GPU driver for Windows 9x: https://github.com/JHRobotics/softgpu

## Build from source

To build from source you need:
- GNU C compiler compatible C compiler (minimal version 4.6, MinGW or DJGPP works)
- Flat assembler (https://flatassembler.net/)
- GNU Make (minimal version 3.81)

To build the binary for your computer, simply type:
```
make
```

To cross compile, specify the `HOST_CC` and `GUEST_CC` variables to choose compiler, for example cross compiling to 32bit Windows:
```
make HOST_CC=gcc GUEST_CC=mingw-w64-i686-gcc
```

To produce a production binary, add `RELEASE=1` to `make`.

After compiling, you can strip the binary (reduce some space):
```
make strip
```

There is a special profile for DOS cross-compilation -- if you have DJGPP compiler, you can produce the DOS executable this way:
```
make RELEASE=1 PROFILE=djgpp
make strip
```

MinGW compiled programs are linked with `msvcrt.dll` by default. To eliminate this depency (`msvcrt.dll` is missing all Windows 9x versions where isn't IE 4 or better), this project can use my [NOCRT](https://github.com/JHRobotics/nocrt) library. Use `make PROFILE=nocrt` or `make PROFILE=nocrt64` to compile program for Windows with depency only to `kernel32.dll`.

**Executable file name for most real operation systems is called `patcher9x`. For DOS, it is called `patch9x.exe`  because file names are limited to 8+3 characters.**

## Patching process

### TLB invalidation bug patching

The patch itself is relatively simple: injecting 2 instructions (`mov ecx,cr3` and `mov cr3,ecx`) to the code in the `VMM.VXD` driver. The patch totally modifies 29 bytes in this file (for Windows 98). 

The problem is that `VMM.VXD` isn't in a normal state on the HDD. `VMM.VXD` is a part of `VMM32.VXD` which is a compressed archive of several `VXD` drivers. `VMM32.VXD` isn't a generic file -- it is generated by the installer exclusively for your HW configuration.

The system for loading drivers first searches in the `SYSTEM\VMM32` folder, and if the driver isn't there, it will search in the `VMM32.VXD` file.

If you want to know more about the code, see file [FlushMappedCacheBlock.asm](vmm/FlushMappedCacheBlock.asm)

[More info about W3/W4 files](doc/VXDLIB_UTF8.txt)

### CPU speed limit bug patching

In the beginning, I increased the number of cycles of older patches by 8 times (80 times more than the original code and in the case of NDIS.VXD, 100 times more than the original code). I also injected a small "divide by zero" protection, which looks like this:
```
VMMcall Get_System_Time ; original - system time (in ms) to EAX
sub eax,esi             ; original - compare with last time in ESI
jnz skip_inc            ; new      - skip if result is non-zero
  inc eax               ; new      - increase zero result by one
skip_inc:
```

There are two variants of this code - one for `NDIS.VXD` and one for all other system files.

The new number of test cycles is now *80 000 000* -- in older patches it was *10 000 000*, but a 4-year-old Ryzen 5 1400 CPU only needs 3 ms to run it through, so if you have a CPU which is 3 times faster[^1] you could have a problem here.

If you want to know more about the code, see file [speed.inc](cpuspeed/speed.inc)

[^1]: Single thread performance in `LOOP` instruction, number of cores, special instructions etc. are irrelevant.

## Development

In future I would like include ~~patch "CPU speed limit" (95, 98 FE)~~ and patch 48-bit LBA (95, 98, ME). Memory limit patch I want include too.
