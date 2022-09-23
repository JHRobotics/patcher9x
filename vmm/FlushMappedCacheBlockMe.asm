;/*** FLAT assembler source
;
; Windows Millennium Edition TLB patch
; for more informations see FlushMappedCacheBlock.asm
;
; ***/
#ifdef relocate
#define DATA_ADR(_a) (not _a)
#define CODE_ADR(_a) (not _a)
#else
#define DATA_ADR(_a) _a
#define CODE_ADR(_a) _a
#endif

use32

; padding to 16 bytes
org 0x7F000000
db 0xED, 0x33, 0xC0, 0x3B, 0xDE, 0x5F, 0x5E, 0x0F, 0x95, 0xC0,0x5B,0xC3

org 0x00000000

; Address = (DUMPLX disassembly) - 0xC95C
;            0x10903 - 0xC95C
;

; address in diassembler 0000C95C
_FlushMappedCacheBlock:
push ebp                                             ;00000000  55
mov ebp,esp                                          ;00000001  8BEC
sub esp, 0xc                                         ;00000003  83EC0C
inc dword [dword DATA_ADR(0x0)]                      ;00000006  FF0500000000
inc dword [dword DATA_ADR(0x0)]                      ;0000000C  FF0500000000
push ebx                                             ;00000012  53
push esi                                             ;00000013  56
push edi                                             ;00000014  57
push 0x0                                             ;00000015  6A00
push dword [ebp+0x8]                                 ;00000017  FF7508
call CODE_ADR(0xfffffeb0)                            ;0000001A  E891FEFFFF
mov esi,eax                                          ;0000001F  8BF0
test esi,esi                                         ;00000021  85F6
mov [ebp-0x4],esi                                    ;00000023  8975FC
jz near 0x1c2                                        ;00000026  0F8496010000
jmp short 0x31                                       ;0000002C  EB03
mov esi,[ebp-0x4]                                    ;0000002E  8B75FC
mov ebx,[esi+0x4]                                    ;00000031  8B5E04
push ebx                                             ;00000034  53
call CODE_ADR(0xfffff460)                            ;00000035  E826F4FFFF
neg eax                                              ;0000003A  F7D8
sbb eax,eax                                          ;0000003C  1BC0
inc eax                                              ;0000003E  40
mov [ebp-0x8],eax                                    ;0000003F  8945F8
jz 0x66                                              ;00000042  7422
push ebx                                             ;00000044  53
call CODE_ADR(0xfffff483)                            ;00000045  E839F4FFFF
inc dword [dword DATA_ADR(0x0)]                      ;0000004A  FF0500000000
push 0x0                                             ;00000050  6A00
push dword [ebp+0x8]                                 ;00000052  FF7508
call CODE_ADR(0xfffffeb0)                            ;00000055  E856FEFFFF
test eax,eax                                         ;0000005A  85C0
mov [ebp-0x4],eax                                    ;0000005C  8945FC
jnz 0x2e                                             ;0000005F  75CD
jmp 0x1c2                                            ;00000061  E95C010000
push  0x1                                            ;00000066  6A01
push ebx                                             ;00000068  53
call CODE_ADR(0xfffffe50)                            ;00000069  E8E2FDFFFF
mov ecx,[esi+0x8]                                    ;0000006E  8B4E08
mov [ebp+0x8],ecx                                    ;00000071  894D08
mov esi,[ecx+0x10]                                   ;00000074  8B7110
shr esi,byte 0xc                                     ;00000077  C1EE0C
mov eax,[esi*4-0x800000]                             ;0000007A  8B04B5000080FF
lea edi,[esi*4-0x800000]                             ;00000081  8D3CB5000080FF
shr eax,byte 0xc                                     ;00000088  C1E80C
shl eax,byte 0x4                                     ;0000008B  C1E004
add eax,[dword DATA_ADR(0x0)]                        ;0000008E  030500000000
mov [ebp-0x8],eax                                    ;00000094  8945F8
test byte [ebx+0x3],0x8                              ;00000097  F6430308
jz 0x112                                             ;0000009B  7475
push ecx                                             ;0000009D  51
call CODE_ADR(0x3890)                                ;0000009E  E8ED370000
mov al,[ebx+0x2]                                     ;000000A3  8A4302
inc dword [dword DATA_ADR(0x0)]                      ;000000A6  FF0500000000
and eax, 0x7f                                        ;000000AC  83E07F
push dword [eax*4+0x0]                               ;000000AF  FF348500000000
push ebx                                             ;000000B6  53
call CODE_ADR(0x21a5)                                ;000000B7  E8E9200000
mov eax,[ebp-0x8]                                    ;000000BC  8B45F8
push dword [ebp-0x4]                                 ;000000BF  FF75FC
mov esi,[eax+0x4]                                    ;000000C2  8B7004
call CODE_ADR(0xfffffef6)                            ;000000C5  E82CFEFFFF
mov eax,[0x0]                                        ;000000CA  A100000000
inc dword [dword DATA_ADR(0x0)]                      ;000000CF  FF0500000000
mov [eax*4+0x0],ebx                                  ;000000D5  891C8500000000
inc eax                                              ;000000DC  40
cmp eax, 0x1a                                        ;000000DD  83F81A
mov [0x0],eax                                        ;000000E0  A300000000
jl 0xee                                              ;000000E5  7C07
and dword [dword DATA_ADR(0x0)], 0x0                 ;000000E7  83250000000000
mov [ebx+0x6],esi                                    ;000000EE  897306
mov al,[ebx+0x3]                                     ;000000F1  8A4303
and al,0xe7                                          ;000000F4  24E7
mov [ebx+0x3],al                                     ;000000F6  884303
push ebx                                             ;000000F9  53
call CODE_ADR(0xfffff483)                            ;000000FA  E884F3FFFF
push dword [ebp+0x8]                                 ;000000FF  FF7508
call CODE_ADR(0x3881)                                ;00000102  E87A370000
dec dword [dword DATA_ADR(0x0)]                      ;00000107  FF0D00000000
jmp 0x1c2                                            ;0000010D  E9B0000000
lea eax,[ebp-0xc]                                    ;00000112  8D45F4
push eax                                             ;00000115  50
push 0x1                                             ;00000116  6A01
push esi                                             ;00000118  56
call CODE_ADR(0xfffff6bd)                            ;00000119  E89FF5FFFF
sub eax,[dword DATA_ADR(0x0)]                        ;0000011E  2B0500000000
push  0xc                                            ;00000124  6A0C
xor edx,edx                                          ;00000126  33D2
pop ecx                                              ;00000128  59
div ecx                                              ;00000129  F7F1
mov ecx,[edi]                                        ;0000012B  8B0F
and ecx,0x5ff                                        ;0000012D  81E1FF050000
shl eax,byte 0xc                                     ;00000133  C1E00C
or eax,ecx                                           ;00000136  0BC1
mov [edi],eax                                        ;00000138  8907
call CODE_ADR(0xfffff4d6)                            ;0000013A  E897F3FFFF
push 0x8                                             ;0000013F  6A08
push 0x0                                             ;00000141  6A00
push 0x3                                             ;00000143  6A03
push 0x1                                             ;00000145  6A01
push esi                                             ;00000147  56
call CODE_ADR(0x14d)                                 ;00000148  E800000000
add esp, 0x14                                        ;0000014D  83C414
mov eax,[DATA_ADR(0x0)]                              ;00000150  A100000000
dec eax                                              ;00000155  48
mov [DATA_ADR(0x0)],eax                              ;00000156  A300000000
mov eax,[DATA_ADR(0x0)]                              ;0000015B  A100000000
dec eax                                              ;00000160  48
mov [DATA_ADR(0x0)],eax                              ;00000161  A300000000
mov eax,[DATA_ADR(0x0)]                              ;00000166  A100000000
imul eax,eax,dword 0xffffff                          ;0000016B  69C0FFFFFF00
add eax,[ebx+0x6]                                    ;00000171  034306
mov ecx,[dword DATA_ADR(0x0)]                        ;00000174  8B0D00000000
mov esi,[dword DATA_ADR(0x0)]                        ;0000017A  8B3500000000
shl eax,byte 0x8                                     ;00000180  C1E008
mov edx,eax                                          ;00000183  8BD0
xor edx,[ecx]                                        ;00000185  3311
and edx,0xfff                                        ;00000187  81E2FF0F0000
xor edx,eax                                          ;0000018D  33D0
mov eax,[ebp+0x8]                                    ;0000018F  8B4508
mov [ecx],edx                                        ;00000192  8911

#ifdef originalcode
  mov ecx,0x400                                      ;00000194  B900040000
  mov edi,[eax+0x10]                                 ;00000199  8B7810
  push eax                                           ;0000019C  50
  rep movsd                                          ;0000019D  F3A5
                                                     ;          ----------------
                                                     ;          9 bytes
#elif defined(vmmbugfix2)
  ; version 2 - do flush soon as possible
  mov ecx,cr3                                        ;          0f20d9
  mov cr3,ecx                                        ;          0f22d9
  jmp FMCB_copy_block                                ;          E8XXXXXXXX
  FMCB_back:
#else
  call FlushTLB
  mov edi,[eax+0x10]                                 ;00000199  8B7810
  push eax                                           ;0000019C  50
  rep movsd                                          ;0000019D  F3A5
#endif

call CODE_ADR(0x3881)                                ;0000019F  E8DD360000
push dword [ebp+0x8]                                 ;000001A4  FF7508
dec dword [dword DATA_ADR(0x0)]                      ;000001A7  FF0D00000000
call CODE_ADR(0x38ae)                                ;000001AD  E8FC360000
push ebx                                             ;000001B2  53
call CODE_ADR(0xfffff483)                            ;000001B3  E8CBF2FFFF
push 0x0                                             ;000001B8  6A00
push dword [ebp-0x4]                                 ;000001BA  FF75FC
call CODE_ADR(0x4b0f)                                ;000001BD  E84D490000
dec dword [dword DATA_ADR(0x0)]                      ;000001C2  FF0D00000000
pop edi                                              ;000001C8  5F
pop esi                                              ;000001C9  5E
pop ebx                                              ;000001CA  5B
leave                                                ;000001CB  C9
ret                                                  ;000001CC  C3
_FlushMappedCacheBlock_end:

; zeroes between blocks
repeat (0x00010870-0x0000C95C)-_FlushMappedCacheBlock_end
db 0x00
end repeat

; address in diassembler 00010870
mov edx,[esp+0x4]                                    ;00003F14  8B542404
push esi                                             ;00003F18  56
push edi                                             ;00003F19  57
mov ecx,[edx+0x4]                                    ;00003F1A  8B4A04
mov esi,[edx]                                        ;00003F1D  8B32
mov eax,[ecx+0x4]                                    ;00003F1F  8B4104
test eax,0x8000000                                   ;00003F22  A900000008
jz 0x3f49                                            ;00003F27  7420
mov edx,[esi]                                        ;00003F29  8B16
and eax,0x7ffffff                                    ;00003F2B  25FFFFFF07
sub edx,eax                                          ;00003F30  2BD0
push ecx                                             ;00003F32  51
mov [esi],edx                                        ;00003F33  8916
call CODE_ADR(0x544e)                                ;00003F35  E814150000
mov dword [esi+0x4d8],0xffffffff                     ;00003F3A  C786D8040000FFFFFFFF
pop edi                                              ;00003F44  5F
pop esi                                              ;00003F45  5E
ret 0x4                                              ;00003F46  C20400
mov edi,[esi+0x8]                                    ;00003F49  8B7E08
and eax,0x7ffffff                                    ;00003F4C  25FFFFFF07
add edi,eax                                          ;00003F51  03F8
cmp eax,0x12c0                                       ;00003F53  3DC0120000
mov [esi+0x8],edi                                    ;00003F58  897E08
ja 0x3f8f                                            ;00003F5B  7732
mov edi,[ecx+0x8]                                    ;00003F5D  8B7908
push ebx                                             ;00003F60  53
mov ebx,[dword DATA_ADR(0x0)]                        ;00003F61  8B1D00000000
and edi,0xffc00000                                   ;00003F67  81E70000C0FF
cmp edi,ebx                                          ;00003F6D  3BFB
pop ebx                                              ;00003F6F  5B
jz 0x3f8f                                            ;00003F70  741D
dec eax                                              ;00003F72  48
pop edi                                              ;00003F73  5F
shr eax,byte 0x4                                     ;00003F74  C1E804
mov edx,[esi+eax*4+0x18]                             ;00003F77  8B548618
mov [ecx],edx                                        ;00003F7B  8911
mov [esi+eax*4+0x18],ecx                             ;00003F7D  894C8618
mov dword [esi+0x4d8],0xffffffff                     ;00003F81  C786D8040000FFFFFFFF
ret_duplicate:
pop esi                                              ;00003F8B  5E
ret 0x4                                              ;00003F8C  C20400
push 0x0                                             ;00003F8F  6A00
push edx                                             ;00003F91  52
call CODE_ADR(0x44d4)                                ;00003F92  E83D050000
mov dword [esi+0x4d8],0xffffffff                     ;00003F97  C786D8040000FFFFFFFF
pop edi                                              ;00003FA1  5F

#ifdef originalcode
  pop esi                                            ;00003FA2  5E
  ret 0x4                                            ;00003FA3  C20400
  nop                                                ;00003FA6  90
  nop                                                ;00003FA7  90


  nop                                                ;00003FA8  90
  nop                                                ;00003FA9  90
  nop                                                ;00003FAA  90
  nop                                                ;00003FAB  90
  nop                                                ;00003FAC  90
  nop                                                ;00003FAD  90
  nop                                                ;00003FAE  90
  nop                                                ;00003FAF  90
  nop                                                ;00003FB0  90
  nop                                                ;00003FB1  90
  nop                                                ;00003FB2  90
  nop                                                ;00003FB3  90
#elif defined(vmmbugfix2)
  jmp ret_duplicate
  FMCB_copy_block:
    mov ecx,0x400                                    ; B900040000
    mov edi,[eax+0x10]                               ; 8B7810
    push eax                                         ; 50
    rep movsd                                        ; F3A5
    jmp FMCB_back                                    ; E9XXXXXXXX
                                                     ; -----------
                                                     ; 16 bytes
#else
  pop esi                                            ;00003FA2  5E
  ret 0x4                                            ;00003FA3  C20400
  nop                                                ;00003FA6  90
  nop                                                ;00003FA7  90
  FlushTLB:
    mov ecx,cr3                                      ; 0f20d9
    mov cr3,ecx                                      ; 0f22d9
    mov ecx,0x400                                    ; B900040000
    ret                                              ; CB
                                                     ; -----------
                                                     ; 12 bytes
#endif
