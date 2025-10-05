                    WINDOWS 95/98/SE/ME RAM LIMITATION PATCH
================================================================================

                                 Original README:

--------------------------------------------------------------------------------

Version 7.2

06/11/2017

Copyright (C) 2007-2017
By Rudolph R. Loew

--------------------------------------------------------------------------------

The Windows 95/98/SE/ME RAM Limitation Patch Program patches Windows 95, 98,
98SE or ME to support Computers with more than 512MB of RAM. Unmodified
Windows 95, 98, 98SE or ME can malfunction or crash when more than 512MB of
RAM is present.

This Patch has an option to increase a memory resource that is required to
prevent the "VFAT Device Initialization Failed" Error or other Error that
occurs when the resource is depleted. This often happens with newer Ethernet
Drivers or if you have a very large Registry.


--------------------------------------------------------------------------------

                            Patcher9x modification:

--------------------------------------------------------------------------------

When Rudolph (Rudy) Loew passed away in 2019 his works made available also
with source code. I was trying to integrate his sources to Patcher9x, but
it operate with different schematic - patching original (and compressed)
VMM32.VXD instead on disintegrate driver archive to pieces, patch
components individually and reconstruct the archive after works is done.

So I simply applied original patch to set of operations systems and binary
compared results. If you wish to know how dump is done, please read
dump_howto.txt.

When patch is applied, the code is same as Rudy's, but the greatest difference
is, that Patcher9x allows to patch the files in every state of installation or
offline when is needed. Also user has comfort to apply all patches at ones.

-- Jaroslav Hensl, 2025
