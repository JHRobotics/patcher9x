;  ************************** FLAT assembler source **************************
;                         Windows 95/98 CPU speed patch
;                 for more informations see 'speed.inc' header
; ****************************************************************************
; 
; In this file: V5 (version_5)
;               V6 (version_6)
;               V7 (version_7)
;

use32
org 0x00000000

#include "speed.inc"

getcpuspeed:                                       ;00004A0C
  enter   4,0                                      ;00004A0C C8040000
  push    eax                                      ;00004A10 50
  push    ecx                                      ;00004A11 51
  push    edx                                      ;00004A12 52
#if defined(version_6) && defined(originalcode)
  mov ecx,0x001E8480                               ;00004A13  B980841E00
#elif defined(version_7) && defined(originalcode)
  mov ecx,0x00989680                               ;00004A13  B980969800
#elif defined(originalcode)
  mov ecx,0x000f4240                               ;00004A13  B940420F00
#else
  mov ecx,NEW_SPEED                                ;00004A13
#endif
   
   
  VMMcall_Get_System_Time                          ;00004A18 CD203F000100
  mov     [ebp-4],eax                              ;00004A1E 8945FC
loop_repeat:                                       ;00004A21
  loop    loop_repeat                              ;00004A21 E2FE
  VMMcall_Get_System_Time                          ;00004A23 CD203F000100
  sub     eax,[ebp-4]                              ;00004A29 2B45FC
  
; Check if interval IS NOT zero, if is, fix it
#if !defined(originalcode)
  jnz skip_inc                                     ; 7501
    inc eax                                        ; 40
  skip_inc:
#endif
  
  push    eax                                      ;00004A2C 50
  
#if defined(version_6) && defined(originalcode)
  mov eax,0x001E8480                               ;00004A2D  B880841E00
#elif defined(version_7) && defined(originalcode)
  mov eax,0x0000FF4F                               ;00004A2D  B84FFF0000
#elif defined(originalcode)
  mov eax,0x000f4240                               ;00004A2D  B840420F00
#else
  mov eax,NEW_SPEED_MUL
#endif
  
  sub     edx,edx                                  ;00004A32 2BD2
  mov     ecx,2710h                                ;00004A34 B910270000
  mul     ecx                                      ;00004A39 F7E1
  
  pop     ecx                                      ;00004A3B 59
  div     ecx                                      ;00004A3C F7F1

#if defined(version_7) && defined(originalcode)
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
  sub     edx,edx                                  ;00004A3E 2BD2
  mov     ecx,99h                                  ;00004A40 B999000000
  div     ecx                                      ;00004A45 F7F1
#else
  nop
  nop
  nop
  nop
  nop
  nop
#endif

  mov     [DATA_ADR(0x1d04)],eax                   ;00004A47 A3041D0000
  pop     edx                                      ;00004A4C 5A
  pop     ecx                                      ;00004A4D 59
  pop     eax                                      ;00004A4E 58
  leave                                            ;00004A4F C9
  ret                                              ;00004A50 C3
