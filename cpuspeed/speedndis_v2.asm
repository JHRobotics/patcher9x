;  ************************** FLAT assembler source **************************
;                         Windows 95/98 CPU speed patch
;                 for more informations see 'speed.inc' header
; ****************************************************************************
; 
; In this file: NDIS V1 (version_2)
;

use32
org 0x00000000

#include "speed.inc"

sub     ecx,ecx                                   ; 2BC9
VMMcall_Get_System_Time                           ; CD203F000100
mov     edx,eax                                   ; 8BD0
add     edx,80h                                   ; 81C280000000
loc_00022DD0:
  inc     ecx                                     ; 41
  VMMcall_Get_System_Time                         ; CD203F000100
  cmp     edx,eax                                 ; 3BD0
  ja      loc_00022DD0                            ; 77F5

mov     [DATA_ADR(0)],ecx                         ; 890D28140000
in      al,21h                                    ; E421

jmp     loc_00022DE5                              ; EB00
loc_00022DE5:

push    eax                                       ; 50
mov     al,0FEh                                   ; B0FE
out     21h,al                                    ; E621
push    0                                         ; 6A00
loc_00022DEC:
VMMcall_Get_System_Time                           ; CD203F000100
mov     edx,eax                                   ; 8BD0
loc_00022DF4:
  VMMcall_Get_System_Time                         ; CD203F000100
  cmp     edx,eax                                 ; 3BD0
  jz      loc_00022DF4                            ; 74F6
mov     edx,eax                                   ; 8BD0

#if defined(originalcode)
  mov     ecx,100000h                             ; B900001000
#else
  mov     ecx,NEW_SPEED_NDIS
#endif

cs mov eax,eax                                    ; 2E8BC0

loop_repeat:
  loop  loop_repeat                               ; E2FE
VMMcall_Get_System_Time                           ; CD203F000100
sub     eax,edx                                   ; 2BC2
pop     ecx                                       ; 59
jnz     loc_00022E1E                              ; 7509
or      ecx,ecx                                   ; 0BC9
jnz     loc_00022E1D                              ; 7504
push    1                                         ; 6A01
jmp     loc_00022DEC                              ; EBCF
loc_00022E1D:
inc     eax                                       ; 40
loc_00022E1E:
#if defined(originalcode)
  mov     ecx,3E8h                                ; B9E8030000
#else
  mov     ecx,NEW_SPEED_NDIS_MUL
#endif
mul     ecx                                       ; F7E1
mov     [DATA_ADR(0)],eax                         ; A330140000
mov     ecx,eax                                   ; 8BC8
mov     eax,100000h                               ; B800001000
sub     edx,edx                                   ; 2BD2
div     ecx                                       ; F7F1
inc     eax                                       ; 40
mov     [DATA_ADR(0)],eax                         ; A32C140000
pop     eax                                       ; 58
out     21h,al                                    ; E621
cld                                               ; FC
ret                                               ; C3
