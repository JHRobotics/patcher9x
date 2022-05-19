;/*** FLAT assembler source
;
; This is larger disassembly of VMM.VXD at file position 0x1eaf0
; main bug is in function "FlushMappedCacheBlock" at aprox. line 290
;
; after assembly binary size should be exactly 1040 bytes
; NOTE that after assembly with defined originalcode=1 code isn't binary same as
; original code - thanks to Intel, x86 have multiple opcodes for same instruc-
; tion. On other hand thanks Intel to keep 8 and 16bit instruction so I could
; make enought space to inject modification of CR3 register.
;
; NOTE that some "symbols" are relocated and you'll need to modify relocation
; table in VMM.VXD if they're move. Define rellocate=1 and compare binary to
; see if some of this symbols moved.
;
; Origin of this bug is described there:
;   https://blog.stuffedcow.net/2015/08/pagewalk-coherence/
; and THIS bug is described there:
;   https://blog.stuffedcow.net/2015/08/win9x-tlb-invalidation-bug/
; Big thanks to Henry Wong (www.stuffedcow.net) for discovering specific
; code sequnce.
; ***/


use32

#ifdef relocate
	org 0x3FFECF8F
	dptr equ not
#else
	org 0xC0013070
	dptr equ 0+
#endif

func0:
push esi                                ; 56
push edi                                ; 57
mov esi,[esp+0xc]                       ; 8B74240C
mov dl,[esi+0x2]                        ; 8A5602
test dl,0x80                            ; F6C280
jnz L0                                  ; 7505
sub eax,eax                             ; 2BC0
pop edi                                 ; 5F
jmp short L1                            ; EB42
L0:
mov eax,esi                             ; 8BC6
shr eax,byte 0xa                        ; C1E80A
xor eax,esi                             ; 33C6
and eax,0x1fe0                          ; 25E01F0000
shr eax,byte 0x5                        ; C1E805
lea edi,[eax*4+(dptr 0x15a98)]          ; 8D3C85985A0100
mov eax,[edi]                           ; 8B07
L2:
cmp [eax+0x4],esi                       ; 397004
jz L3                                   ; 7406
mov edi,eax                             ; 8BF8
mov eax,[edi]                           ; 8B07
jmp short L2                            ; EBF5
L3:
cmp dword [esp+0x10],0x0                ; 837C241000
jz L4                                   ; 7417
and dl,0x7f                             ; 80E27F
mov [esi+0x2],dl                        ; 885602
mov ecx,[eax]                           ; 8B08
mov [edi],ecx                           ; 890F
mov ecx,[dword (dptr 0x14f50)]          ; 8B0D504F0100
dec ecx                                 ; 49
mov [dword (dptr 0x14f50)],ecx          ; 890D504F0100
L4:
pop edi                                 ; 5F
L1:
pop esi                                 ; 5E
ret 0x8                                 ; C20800

func1:
push esi                                ; 56
mov edx, (dptr 0x15a98)                 ; BA985A0100
mov ecx,[esp+0x8]                       ; 8B4C2408
L8:
mov esi,edx                             ; 8BF2
mov eax,[esi]                           ; 8B06
L7:
or eax,eax                              ; 0BC0
jz L5                                   ; 740B
cmp [eax+0x8],ecx                       ; 394808
jz L6                                   ; 7417
mov esi,eax                             ; 8BF0
mov eax,[esi]                           ; 8B06
jmp short L7                            ; EBF1
L5:
add edx, 0x4                            ; 83C204
cmp edx, (dptr 0x15e98)                 ; 81FA985E0100
jc L8                                   ; 72E2
sub eax,eax                             ; 2BC0
L9:
pop esi                                 ; 5E
ret 0x8                                 ; C20800
L6:
cmp dword [esp+0xc],0x0                 ; 837C240C00
jz L9                                   ; 74F5
mov ecx,[eax+0x4]                       ; 8B4804
and byte [ecx+0x2],0x7f                 ; 8061027F
mov edx,[eax]                           ; 8B10
mov [esi],edx                           ; 8916
mov ecx,[dword (dptr 0x14f50)]          ; 8B0D504F0100
dec ecx                                 ; 49
mov [dword (dptr 0x14f50)],ecx          ; 890D504F0100
jmp short L9                            ; EBDB

func2:
sub esp,0x8                             ; 83EC08
push ebx                                ; 53
push esi                                ; 56
push edi                                ; 57
push ebp                                ; 55
mov eax,[esp+0x1c]                      ; 8B44241C
mov ebx,[eax+0x8]                       ; 8B5808
mov eax,[ebx+0x10]                      ; 8B4310
shr eax,byte 0xc                        ; C1E80C
lea ecx,[eax*4-0x800000]                ; 8D0C85000080FF
mov ebp,[dword (dptr 0x14fa4)]          ; 8B2DA44F0100
mov [esp+0x14],ecx                      ; 894C2414
mov ecx,[ecx]                           ; 8B09
shr ecx,byte 0xc                        ; C1E90C
imul ecx,ecx,0xd                        ; 6BC90D
add ebp,ecx                             ; 03E9
mov ecx,[ebp+0x0]                       ; 8B4D00
mov [esp+0x10],ecx                      ; 894C2410
test byte [ecx+0x3],0x8                 ; F6410308
jz L10                                  ; 7429
cmp [dword (dptr 0x1507c)],ebp          ; 392D7C500100
jnz L11                                 ; 750A
mov eax,[dptr 0x15ed0]                       ; A1D05E0100
mov [dptr 0x1507c],eax                       ; A37C500100
L11:
push ebp                                ; 55
call 0xc0005e7a                         ; E80E2DFFFF
push ebx                                ; 53
call 0xc0008a28                         ; E8B658FFFF
mov eax,[dptr 0x14f54]                       ; A1544F0100
inc eax                                 ; 40
mov [dptr 0x14f54],eax                       ; A3544F0100
L10:
mov word [ebp+0x8],0xf188               ; 66C7450888F1
push dword [dword (dptr+0x15078)]       ; FF3578500100
call 0xc0004210                         ; E88210FFFF
mov esi,[esp+0x1c]                      ; 8B74241C
mov edi,eax                             ; 8BF8
add esi,0xc                             ; 83C60C
movsd                                   ; A5
movsd                                   ; A5
movsw                                   ; 66A5
mov ecx,[esp+0x14]                      ; 8B4C2414
and dword [ecx],0xfffff7ff              ; 8121FFF7FFFF
push ebx                                ; 53
mov [ebp+0x0],eax                       ; 894500
mov edx,[eax+0x6]                       ; 8B5006
mov eax,[esp+0x14]                      ; 8B442414
mov word [ebp+0xa],0x1                  ; 66C7450A0100
mov [ebp+0x4],edx                       ; 895504
and byte [eax+0x3],0xe7                 ; 806003E7
call 0xc0008a19                         ; E85758FFFF
mov eax,[dptr 0x14f54]                       ; A1544F0100
push ebx                                ; 53
dec eax                                 ; 48
mov [dptr 0x14f54],eax                       ; A3544F0100
call 0xc0008a46                         ; E87358FFFF
push 0x0                                ; 6A00
push dword [esp+0x20]                   ; FF742420
call 0xc00143dd                         ; E8FF110000
pop ebp                                 ; 5D
pop edi                                 ; 5F
pop esi                                 ; 5E
pop ebx                                 ; 5B
add esp,0x8                             ; 83C408
ret 0x4                                 ; C20400

_ReleaseMappedCacheBlock:
push ebx                                ; 53
push esi                                ; 56
push edi                                ; 57
push ebp                                ; 55
mov ebx,[dword (dptr 0x14f54)]          ; 8B1D544F0100
mov edi,[dword (dptr 0x15f6c)]                 ; 8B3D6C5F0100
mov esi,ebx                             ; 8BF3
mov eax,edi                             ; 8BC7
imul eax,[dword (dptr 0x12718)]                ; 0FAF0518270100
mov ebp,0x64                            ; BD64000000
sub edx,edx                             ; 2BD2
div ebp                                 ; F7F5
sub edi,eax                             ; 2BF8
cmp ebx,edi                             ; 3BDF
jna L12                                 ; 7611
L13:
push 0x0                                ; 6A00
call 0xc0005f31                         ; E8182DFFFF
mov ebx,[dword (dptr 0x14f54)]          ; 8B1D544F0100
cmp ebx,edi                             ; 3BDF
ja L13                                  ; 77EF
L12:
sub ebx,esi                             ; 2BDE
pop ebp                                 ; 5D
cmp ebx,0x1                             ; 83FB01
pop edi                                 ; 5F
sbb eax,eax                             ; 1BC0
pop esi                                 ; 5E
inc eax                                 ; 40
pop ebx                                 ; 5B
ret                                     ; C3

_FlushMappedCacheBlock:
sub esp,0x10                            ; 83EC10
push ebx                                ; 53
push esi                                ; 56
push edi                                ; 57
push ebp                                ; 55
mov eax,[dptr 0x14f64]                ; A1644F0100
mov esi,[esp+0x24]                      ; 8B742424
inc eax                                 ; 40
mov [dptr 0x14f64],eax                       ; A3644F0100
mov eax,[dptr 0x14f6c]                       ; A16C4F0100
inc eax                                 ; 40
mov [dptr 0x14f6c],eax                       ; A36C4F0100
L16:
push 0x0                                ; 6A00
push esi                                ; 56
call func1                              ; E870FEFFFF
mov ebp,eax                             ; 8BE8
or ebp,ebp                              ; 0BED
jz near L14                             ; 0F84A6010000
mov ebx,[ebp+0x4]                       ; 8B5D04
push ebx                                ; 53
call 0xc0003e7c                         ; E8100CFFFF
cmp eax,0x1                             ; 83F801
sbb edi,edi                             ; 1BFF
neg edi                                 ; F7DF
or edi,edi                              ; 0BFF
jz L15                                  ; 7415
push ebx                                ; 53
call 0xc0003e9d                         ; E8200CFFFF
mov eax,[dptr 0x14f70]                       ; A1704F0100
inc eax                                 ; 40
or edi,edi                              ; 0BFF
mov [dptr 0x14f70],eax                       ; A3704F0100
jnz L16                                 ; 75C5
L15:
push 0x1                                ; 6A01
push ebx                                ; 53
call func0                              ; E8DCFDFFFF
mov eax,[ebp+0x8]                       ; 8B4508
mov [esp+0x10],eax                      ; 89442410
mov edi,[eax+0x10]                      ; 8B7810
shr edi,byte 0xc                        ; C1EF0C
lea esi,[edi*4-0x800000]                ; 8D34BD000080FF
mov eax,[esi]                           ; 8B06
mov [esp+0x14],eax                      ; 89442414
mov ecx,eax                             ; 8BC8
shr ecx,byte 0xc                        ; C1E90C
imul ecx,ecx, 0xd                       ; 6BC90D
mov eax,[dptr 0x14fa4]                  ; A1A44F0100
add eax,ecx                             ; 03C1
test byte [ebx+0x3],0x8                 ; F6430308
mov [esp+0x18],eax                      ; 89442418
jz near L17                             ; 0F8486000000
push dword [esp+0x10]                   ; FF742410
call 0xc0008a28                         ; E85457FFFF
mov eax,[dptr 0x14f54]                  ; A1544F0100
inc eax                                 ; 40
mov [dptr 0x14f54],eax                  ; A3544F0100
mov al,[ebx+0x2]                        ; 8A4302
and eax, 0x7f                           ; 83E07F
push dword [eax*4+(dptr 0x16080)]       ; FF348580600100
push ebx                                ; 53
call 0xc00071be                         ; E8CC3EFFFF
mov ecx,[esp+0x18]                      ; 8B4C2418
push ebp                                ; 55
mov esi,[ecx+0x4]                       ; 8B7104
call func2                              ; E819FEFFFF
mov eax,[dptr 0x14f98]                  ; A1984F0100
inc eax                                 ; 40
mov [dptr 0x14f98],eax                  ; A3984F0100
mov eax,[dptr 0x1622c]                  ; A12C620100
mov [eax*4+(dptr 0x161c4)],ebx          ; 891C85C4610100
inc eax                                 ; 40
mov [dptr 0x1622c],eax                  ; A32C620100
cmp eax, 0x1a                           ; 83F81A
jl L18                                  ; 7C0A
mov dword [dword (dptr 0x1622c)],0x00000  ; C7052C6201000000
L18:
and byte [ebx+0x3],0xe7                 ; 806303E7
mov [ebx+0x6],esi                       ; 897306
push ebx                                ; 53
call 0xc0003e9d                         ; E8650BFFFF
push dword [esp+0x10]                   ; FF742410
call 0xc0008a19                         ; E8D856FFFF
mov eax,[dptr 0x14f54]                  ; A1544F0100
dec eax                                 ; 48
mov [dptr 0x14f54],eax                  ; A3544F0100
jmp L14                                 ; E9B8000000
L17:
lea eax,[esp+0x1c]                      ; 8D44241C
push eax                                ; 50
push  0x1                               ; 6A01
push edi                                ; 57
call 0xc00040e0                         ; E8820DFFFF
sub eax,[dword (dptr 0x15f5c)]          ; 2B055C5F0100
mov ecx,0xc                             ; B90C000000
sub edx,edx                             ; 2BD2
div ecx                                 ; F7F1
shl eax,cl                              ; D3E0
mov ecx,[esp+0x14]                      ; 8B4C2414
#ifdef originalcode
        and ecx,0xfff                   ; 81E1FF0F0000 
        or eax,ecx                      ; 0BC1 
        mov [esi],eax                   ; 8906
        and eax,0xfffffdff              ; 25FFFDFFFF
        mov [esi],eax                   ; 8906
        and eax,0xfffff7ff              ; 25FFF7FFFF
        ; code length = 22 bytes
#else
        and ch,0x0f                     ; 80e50f
        or  ax,cx                       ; 6609c8
        and ah,0xf5                     ; 80e4f5
        ; code length = 9 bytes (13 bytes saved)
#endif
mov [esi],eax                           ; 8906
#ifndef originalcode
        jmp skip_injected               ; eb0b
        flushtable:   
        mov ecx,cr3                     ; 0f20d9
				mov cr3,ecx                     ; 0f22d9
        nop                             ; 90
        xor ecx,ecx                     ; 31c9
        jmp flushtable_back             ; ebxx 
        skip_injected:
        ; code length = 12 bytes (+1 byte padded with NOP)
#endif
call 0xc0003ef0                         ; E8600BFFFF
push 0x8                                ; 6A08
push 0x0                                ; 6A00
push 0x3                                ; 6A03
push 0x1                                ; 6A01
push edi                                ; 57
call 0xbffe1420                         ; E882E0FCFF
add esp, 0x14                           ; 83C414
mov ecx,0xd                             ; B90D000000
mov eax,[ebx+0x6]                       ; 8B4306
sub edx,edx                             ; 2BD2
sub eax,[dword (dptr 0x14fa4)]          ; 2B05A44F0100
div ecx                                 ; F7F1
shl eax,byte 0xc                        ; C1E00C
mov ecx,[dword (dptr 0x15098)]          ; 8B0D98500100
mov edx,[ecx]                           ; 8B11
xor edx,eax                             ; 33D0
and edx,0xfff                           ; 81E2FF0F0000
xor edx,eax                             ; 33D0
mov eax,[esp+0x10]                      ; 8B442410
mov [ecx],edx                           ; 8911
; TLB BUG here, needs to insert something like this:
; mov eax,cr3
;	mov cr3,eax
#ifdef originalcode
        mov ecx,0x400                  ; B900040000
        ; ^ could be writen as "XOR ECX, ECX \n MOV CH, 0x4"
        ;   which is one byte shorter and is splitten between
        ;   two short instructions
        ; code length = 5 bytes
#else
        jmp flushtable                  ; ebxx
        nop                             ; 90
        flushtable_back:
        mov ch,0x4                      ; b504
        ; code length = 4 bytes (+1 byte padded with NOP)
#endif

mov edi,[eax+0x10]                      ; 8B7810
mov esi,[dword (dptr 0x16178)]          ; 8B3578610100
rep movsd                               ; F3A5
push dword [esp+0x10]                   ; FF742410
call 0xc0008a19                         ; E83256FFFF
mov eax,[dptr 0x14f54]                  ; A1544F0100
push dword [esp+0x10]                   ; FF742410
dec eax                                 ; 48
mov [dptr 0x14f54],eax                  ; A3544F0100
call 0xc0008a46                         ; E84B56FFFF
push ebx                                ; 53
call 0xc0003e9d                         ; E89C0AFFFF
push  0x0                               ; 6A00
push ebp                                ; 55
call 0xc00143dd                         ; E8D40F0000
L14:
mov eax,[dptr 0x14f64]                  ; A1644F0100
pop ebp                                 ; 5D
dec eax                                 ; 48
pop edi                                 ; 5F
mov [dptr 0x14f64],eax                  ; A3644F0100
pop esi                                 ; 5E
pop ebx                                 ; 5B
add esp, 0x10                           ; 83C410
ret                                     ; C3

func3:
push ebx                                ; 53
push esi                                ; 56
mov ecx,0x2                             ; B902000000
call 0xc000bac1                         ; E89986FFFF
mov bl,al                               ; 8AD8
push 0x0                                ; 6A00
push 0x0                                ; 6A00
push 0x0                                ; 6A00
push 0x1e                               ; 6A1E
call 0xc0014824                         ; E8ED130000
mov esi,eax                             ; 8BF0
mov ecx,ebx                             ; 8BCB
call 0xc000bae6                         ; E8A686FFFF
mov eax,[esp+0xc]                       ; 8B44240C
mov [esi+0x4],eax                       ; 894604
mov ecx,eax                             ; 8BC8
shr ecx,byte 0xa                        ; C1E90A
xor ecx,eax                             ; 33C8
and ecx,0x1fe0                          ; 81E1E01F0000
shr ecx,byte 0x5                        ; C1E905
mov eax,[ecx*4+(dptr 0x15a98)]          ; 8B048D985A0100
mov [esi],eax                           ; 8906
mov eax,[dptr 0x14f50]                  ; A1504F0100
mov [ecx*4+(dptr 0x15a98)],esi          ; 89348D985A0100
inc eax                                 ; 40
mov [dptr 0x14f50],eax                  ; A3504F0100
mov eax,esi                             ; 8BC6
pop esi                                 ; 5E
pop ebx                                 ; 5B
ret 0x4                                 ; C20400

db 0x00,0x00,0x00,0x00,0x00,0x00,0x00
