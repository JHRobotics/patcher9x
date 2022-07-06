;  ************************** FLAT assembler source **************************
;                         Windows 95/98 CPU speed patch
;                 for more informations see 'speed.inc' header
; ****************************************************************************
; 
; In this file: NDIS V1 (version_1)
;               NDIS V1 (version_3)
;

use32
org 0x00000000

#include "speed.inc"

sub     ecx,ecx                                   ; 2BC9
VMMcall_Get_System_Time                           ; CD203F000100
mov     edx,eax                                   ; 8BD0
add     edx,80h                                   ; 81C280000000

loc_00021400:
  inc     ecx                                     ; 41
  VMMcall_Get_System_Time                         ; CD203F000100
  cmp     edx,eax                                 ; 3BD0
  ja      loc_00021400                            ; 77F5
mov     [DATA_ADR(0x0)],ecx                       ; 890D00000000

in      al,21h                                    ; E421
jmp     loc_00021415                              ; EB00
  loc_00021415:

push    eax                                       ; 50
mov     al,0FEh                                   ; B0FE
out     21h,al                                    ; E621
VMMcall_Get_System_Time                           ; CD203F000100
mov     edx,eax                                   ; 8BD0
wait_begin:                                       ; wait begin of inteval
  VMMcall_Get_System_Time                         ; CD203F000100
  cmp     edx,eax                                 ; 3BD0
  jz      wait_begin                              ; 74F6
mov     edx,eax                                   ; 8BD0
#if defined(originalcode) && defined(version_3)
  mov     ecx,0xA00000                            ; B90000A000
#elif defined(originalcode)
  mov     ecx,0x100000                            ; B900001000
#else
  mov     ecx,NEW_SPEED_NDIS
#endif

#if defined(originalcode)
  nop                                             ; 90 (1 saved byte)
#endif

loop_repeat:
  loop    loop_repeat                             ; E2FE

VMMcall_Get_System_Time                           ; CD203F000100
sub     eax,edx                                   ; 2BC2

#if !defined(originalcode)
  jnz skip_inc                                    ; 7501
    inc eax                                       ; 40
  skip_inc:
#endif

#if defined(originalcode) && defined(version_3)
  mov     ecx,0x0064                              ; B964000000
#elif defined(originalcode)
  mov     ecx,0x03E8                              ; B9E8030000
#else
;  mov     ecx,NEW_SPEED_NDIS_MUL
  push NEW_SPEED_NDIS_MUL                         ; 6A32
  pop ecx                                         ; 59
                                                  ; (2 saved byte)
#endif

mul     ecx                                       ; F7E1

mov     [DATA_ADR(0x0)],eax                       ; A300000000
mov     ecx,eax                                   ; 8BC8
mov     eax,100000h                               ; B800001000
sub     edx,edx                                   ; 2BD2
div     ecx                                       ; F7F1
inc     eax                                       ; 40
mov     [DATA_ADR(0x0)],eax                       ; A300000000
pop     eax                                       ; 58
out     21h,al                                    ; E621
cld                                               ; FC
ret                                               ; C3
