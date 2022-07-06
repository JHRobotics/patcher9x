;  ************************** FLAT assembler source **************************
;                         Windows 95/98 CPU speed patch
;                 for more informations see 'speed.inc' header
; ****************************************************************************
; 
; In this file: V3 (version_3)
;

use32
org 0x00000000

#include "speed.inc"

#if defined(originalcode)
  mov ecx,0x989680                                   ;00000000  B980969800
#else
  mov ecx,NEW_SPEED 
#endif

VMMcall_Get_System_Time                              ;00000005  CD203F000100
mov esi,eax                                          ;0000000B  8BF0

loop_repeat:
  loop loop_repeat                                   ;0000000D  E2FE

VMMcall_Get_System_Time                              ;0000000F  CD203F000100
sub eax,esi                                          ;00000015  2BC6

#if defined(originalcode)
  push eax                                           ;00000017  50
  mov eax,0xff4f                                     ;00000018  B84FFF0000

  sub edx,edx                                        ;0000001D  2BD2
  mov ecx,0x2710                                     ;0000001F  B910270000 (eax * ecx = 653590000)
                                                     ; ----- 13 bytes -----
#else
  jnz skip_inc                                       ; 7501
    inc eax                                          ; 40
  skip_inc:
  push eax                                           ; 50
  push NEW_SPEED_MULB2                               ; 6A40
  pop  ecx                                           ; 59
  mov  eax,NEW_SPEED_MULB1                           ; B9 FD 4F 6F 02 (eax * ecx = 5228720000)
                                                     ; ----- 12 bytes -----
  nop                                                ; 90 (padding)
#endif

mul ecx                                              ;00000024  F7E1

pop ecx                                              ;00000026  59

div ecx                                              ;00000027  F7F1
mov [edi+0x14],eax                                   ;00000029  894714

