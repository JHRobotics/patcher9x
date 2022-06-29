;  ************************** FLAT assembler source **************************
;                         Windows 95/98 CPU speed patch
;                 for more informations see 'speed.inc' header
; ****************************************************************************
; 
; In this file: V1 (version_1)
;               V2 (version_2)
;               V4 (version_4)
;

use32
org 0x00000000

#include "speed.inc"

#if defined(version_2) && defined(originalcode)
  mov ecx,0x001E8480                                 ;00000000  B980841E00
#elif defined(version_4) && defined(originalcode)
  mov ecx,0x00989680                                 ;00000000  B980969800
#elif defined(originalcode)
  mov ecx,0x000f4240                                 ;00000000  B940420F00
#else
  mov ecx,0x04C4B400                                 ;00000000
#endif

VMMcall_Get_System_Time                              ;00000005  CD203F000100
mov esi,eax                                          ;0000000B  8BF0

loop_repeat:
  loop loop_repeat                                   ;0000000D  E2FE

VMMcall_Get_System_Time                              ;0000000F  CD203F000100
sub eax,esi                                          ;00000015  2BC6

; Check if interval IS NOT zero, if is, fix it
#if !defined(originalcode)
  jnz skip_inc                                       ; 7501
    inc eax                                          ; 40
  skip_inc:
#endif

push eax                                             ;00000017  50

#if defined(version_2) && defined(originalcode)
  mov eax,0x001E8480                                 ;00000018  B880841E00
#elif defined(version_4) && defined(originalcode)
  mov eax,0x0000FF4F                                 ;00000018  B84FFF0000
#elif defined(originalcode)
  mov eax,0x000f4240                                 ;00000018  B840420F00
#else
  mov eax,0x0007FA78
#endif

sub edx,edx                                          ;0000001D  2BD2
mov ecx,0x2710                                       ;0000001F  B910270000
mul ecx                                              ;00000024  F7E1

pop ecx                                              ;00000026  59
div ecx                                              ;00000027  F7F1


#if defined(version_4) && defined(originalcode)
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
#elif defined(originalcode)
  sub edx,edx                                        ;00000029  2BD2
  mov ecx,0x99                                       ;0000002B  B999000000
  div ecx                                            ;00000030  F7F1
#else
  nop
  nop
  nop
  nop
  nop
  nop
#endif
mov [edi+14h],eax                                    ; 00000032 894714
