;                         Shared part of CPU speed bug fix
; ==============================================================================
; Original description and identification by Rudolph R. Loew (rloew)
; (URL: https://www.betaarchive.com/forum/viewtopic.php?t=29224)
; ==============================================================================
; The CPU Speed Limit Problem in Windows 95, 98 and their Betas is due to
; a number of faulty CPU speed checks that were not designed for fast
; Processors. A step in the calculations, I have found, causes a Divide by Zero
; Error. If this occurs in the Drivers that are started during Bootup, you get
; a "Windows Protection Error" and the Computer stops; Other problems can occur
; if one of the Drivers is executed later.
;
; There are up to 6 Files that need to be Patched to correct the two speed
; check algorithms that I have identified.
;
; The following 5 Files have Algorithm #1:
;
; NTKERN.VXD
; IOS.VXD
; ESDI_506.PDR
; SCSIPORT.PDR
; CS3KIT.EXE (In two places)
; 
; NDIS.VXD has Algorithm #2.
;
; To Patch the Files with Algorithm #1 perform the following steps:
;
; 1.  Search for the Binary Sequence: 2B D2 B9 99 00 00 00 F7 F1. If not found
;     the File does not need fixing or has already been Patched.
; 2.  This location will be referred to as X for the later steps.
; 3.  At X-2 there should be F7 F1.
; 4.  At X-16 there should be either 40 42 0F 00 or 80 84 1E 00.
; 5.  At either X-40 or X-42 there should be the same sequence as at X-16.
; 6.  If everything is correct continue, otherwise the File has a different
;     Algorithm or is already Patched.
; 7.  Replace the Sequence at X-40 or X-42 (from Step #5) with 80 96 98 00.
; 8.  Replace the Sequence at X-16 with 4F FF 00 00
; 9.  Replace the Sequence at X with 90 90 90 90 90 90 90 90 90.
; 10. For CS3KIT.EXE, Repeat Steps 1 thru 9 using the Second Instance of the
;     Sequence shown in Step 1.
; 
; To Patch the NDIS.VXD File with Algorithm #2 perform the following steps:
; 
; 1. Search for the Binary Sequence: B9 00 00 10 00. If not found the File does
;    not need fixing or has already been Patched.
; 2. This location will be referred to as X for the later steps.
; 3. At X+16 there should be B9 E8 03 00 00.
; 4. If everything is correct continue, otherwise look for a later instance
;    of the Sequence in Step 1 and repeat. If no more, the File does not need
;    Fixing.
; 5. Change the Byte at X+3 from 10 to A0.
; 6. Change the two Bytes at X+17 from E8 03 to 64 00.
; 
; NTKERN.VXD and ESDI_506.PDR appear to have been fixed with the 1400 Version.
; IOS.VXD and SCSIPORT.PDR appear to have been fixed with the 1411 Version.
; CS3KIT.EXE appears to only have had the problem in the 1387 Version.
; NDIS.VXD has the problem in all Versions of Windows 95 and 98,
; only being fixed in the 98SE Betas.
;
; ==============================================================================
;                          In memory of Rudolph R. Loew
;                                 (1952-2019)
;
;                                Rest in peace 
;
; ==============================================================================
;
;
;
;
;
; A little addition: 10 000 000 (0098 9680h) is small nowadays (2022). I believe
; that some of nowadays CPUs (most from AMD, Intel has in LOOP instruction bit
; slower) could run through this code under 1 ms and if not, about 2 ms and
; bellow on this loop could trigger this bug too (because precision of
; timer). So, I increased it 8 times. Side effect of this is that machines with
; old slow CPU boot bit slower with this patch.
;
; On other hands, you probably want to apply my set of patches because you have
; new ultra-fast CPU...
; 
; I also injected 2 instructions to prevent division by zero. So future CPU (if
; it run through 80 000 000 LOOP under 1 ms) won''t crash on zero division.
; It''s on discussion how well drivers could work without precise timing
; information but at last the system doesn''t crash on start-up.
;
; The patch has 8 variants for various system files split between 4 assembler
; files and 4 another''s variant for NDIS.VXD in another 2 files.
;
; The list of variants:
; +---------+------------------+------------------------------------------------
; | Version | File             | Description
; +---------+------------------+------------------------------------------------
; |   V1    | speed_v1.asm     |  1 000 000 LOOPs (Windows 95, IOS.VXD)
; |   V2    | speed_v1.asm     |  2 000 000 LOOPs (V1 variant mention in post)
; |   V3    | speed_v3.asm     | 10 000 000 LOOPs (MS updates / FIX95 patch)
; |   V4    | speed_v1.asm     | 10 000 000 LOOPs (applied rloew''s manul patch)
; |   V5    | speed_v5.asm     |  1 000 000 LOOPs (Windows 95, SCSIPORT.PDR + ESDI_506.PDR)
; |   V6    | speed_v5.asm     |  2 000 000 LOOPs (V5 variant mention in post)
; |   V7    | speed_v5.asm     | 10 000 000 LOOPs (applied rloew''s manual patch)
; |   V8    | speed_v8.asm     | 10 000 000 LOOPs (MS updates / FIX95 patch)
; +---------+------------------+------------------------------------------------
; | NDIS V1 | speedndis_v1.asm |  1 048 576 LOOPs (Windows 95, 98 FE, NDIS.VXD)
; | NDIS V2*| speedndis_v2.asm |  1 048 576 LOOPs (Windows 98 SE, NDIS.VXD)
; | NDIS V3 | speedndis_v1.asm | 10 485 760 LOOPS (applied rloew''s manual patch)
; +---------+------------------+------------------------------------------------
;
; * In windows 98 SE is loop short but is here condition about zero division.
;   I saw some rare and unregularly errors caused by NDIS.VXD and I hope
;   working timing information helps solve some of them.
;
; After patch in V1-V8 version, new LOOP will repeat 80 000 000 (0x4C4B400) times
; and in NDIS V1-V3 version, new LOOP will repeat 104 857 600 (0x6400000) times.
;

#ifdef relocate
#define DATA_ADR(_a) (not _a)
#else
#define DATA_ADR(_a) _a
#endif

#define VMMcall_Get_System_Time db 0xCD,0x20,0x3F,0x00,0x01,0x00

#if !defined(patchforslowcpu)
; My first attempt (x8, x10), but due another bug, this values looks too large
; a boot was slow. So, I created another set of values, but at last problem
; wasn't in this values. And I think, it's best choise for most CPUs

; D = 0x7FA78 * 0x2710 = 0x137A7EF80 (5 228 720 000)
; D / SPEED = 65,359
# define NEW_SPEED         0x04C4B400
# define NEW_SPEED_MUL     0x0007FA78

; D = 0x4DE9FBE * 0x40 = 0x137A7EF80 (5 228 720 000)
# define NEW_SPEED_MULB1   0x4DE9FBE
# define NEW_SPEED_MULB2   0x40

; NEW_SPEED_NDIS * NEW_SPEED_NDIS_MUL = 0x3E80 0000 (1 048 576 000)
# define NEW_SPEED_NDIS     0x6400000
# define NEW_SPEED_NDIS_MUL 0x0A
#else
; My second set (x2) as compromise with divide by zero protection. 
; You can try use it if you have some performace problem.

; D = 0x1FE9E * 0x2710 = 0x4DE9FBE0 (1 307 180 000)
; D / SPEED = 65,359
# define NEW_SPEED         0x01312D00
# define NEW_SPEED_MUL     0x0001FE9E

; D = 0x26F4FDF * 0x20 = 0x4DE9FBE0 (1 307 180 000)
# define NEW_SPEED_MULB1   0x26F4FDF
# define NEW_SPEED_MULB2   0x20

; NEW_SPEED_NDIS * NEW_SPEED_NDIS_MUL = 0x3E80 0000 (1 048 576 000)
# define NEW_SPEED_NDIS     0x1400000
# define NEW_SPEED_NDIS_MUL 0x32

#endif
