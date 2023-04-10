# Files for FreeDOS boot floppy/disk

For creating your own disk, you need FreeDOS Floppy Edition from here:
[freedos.org/download/](http://www.freedos.org/download/)

Pick boot disk `x86BOOT.img` from folder `144m` in the archive. You can remove all files except these:
```
CONFIG   SYS
COMMAND  COM
KERNEL   SYS
FDISKPT  INI
FDCONFIG SYS
```

Now you probably wants add some drivers:
```
HIMEMX   EXE
JEMM386  EXE
SHSUCDX  COM
UDVD2    SYS
DEVLOAD  COM
```
You can find these files some were over floppies or download CD Edition if you want them on one place. 

Now you create/edit these files (all are text files):
```
AUTOEXEC BAT
FDCONFIG SYS
```
Or you can use my set of config files. Don’t load DOS to hight memory, Windows installer doesn’t like it (by documentation it doesn’t like non-Microsoft memory managers at all, but if DOS is loaded to low memory it’ll usualy works.)

To run DOS version of **Patcher9x** you need `cwsdpmi.exe` from its author site: [sandmann.dotster.com/cwsdpmi/](http://sandmann.dotster.com/cwsdpmi/). This is memory manager for DJGPP executables so program can access more memory and not need residential memory manager - useful in safe mode or minimal configurations.
```
CWSDPMI  EXE
PATCH9X  EXE
```

To my distribution, I included other tools, they listed in [info.txt](info.txt). All are form FreeDOS distribution.

