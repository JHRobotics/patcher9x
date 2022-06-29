;  ************************** FLAT assembler source **************************
;                         Windows 95/98 CPU speed patch
;                 for more informations see 'speed.inc' header
; ****************************************************************************
; 
; In this file: V8 (version_8)
;

use32
org 0x00000000

#include "speed.inc"

getcpuspeed:                                         ;00002786
    enter   4,0                                      ;00002786 C8040000
    push    eax                                      ;0000278A 50
    push    ecx                                      ;0000278B 51
    push    edx                                      ;0000278C 52
#if defined(originalcode)
    mov     ecx,0x00989680                           ;0000278D B980969800
#else
    mov     ecx,0x04C4B400                           ;0000278D
#endif
    VMMcall_Get_System_Time                          ;00002792 CD203F000100
    mov     [ebp-4],eax                              ;00002798 8945FC
loop_repeat:                                         ;0000279B
    loop    loop_repeat                              ;0000279B E2FE
    VMMcall_Get_System_Time                          ;0000279D CD203F000100
    sub     eax,[ebp-4]                              ;000027A3 2B45FC

#if defined(originalcode)
    push    eax                                      ;000027A6 50
    mov     eax,0x0FF4F                              ;000027A7 B84FFF0000
    sub     edx,edx                                  ;000027AC 2BD2
    mov     ecx,0x2710                               ;000027AE B910270000 (eax * ecx = 653 590 000)
                                                     ; ----- 13 bytes -----
#else
    jnz skip_inc                                     ; 7501
      inc eax                                        ; 40
    skip_inc:
    push eax                                         ; 50
    push 0x40                                        ; 6A40
    pop  eax                                         ; 59
    mov  eax,0x04DE9FBE                              ; B8BE9FDE04 (eax * ecx = 5 228 720 000)
                                                     ; ----- 12 bytes -----
    nop                                              ; 90 (padding)
#endif

    mul     ecx                                      ;000027B3 F7E1
    pop     ecx                                      ;000027B5 59
    div     ecx                                      ;000027B6 F7F1
    mov     [DATA_ADR(0x1370)],eax                   ;000027B8 A370130000
    pop     edx                                      ;000027BD 5A
    pop     ecx                                      ;000027BE 59
    pop     eax                                      ;000027BF 58
    leave                                            ;000027C0 C9
    ret                                              ;000027C1 C3
