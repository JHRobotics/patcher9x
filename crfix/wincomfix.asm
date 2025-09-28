; ==============================================================================
; MIT No Attribution
;
; Copyright 2025 Jaroslav Hensl <emulator@emulace.cz>
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
; THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
; IN THE SOFTWARE.
;
; ==============================================================================
;
org 100h

; Address with free space to insert patched code
patch_pos  = 04500h
; jump address from original win.com to continue execiting
return_pos = 01073h

jmp cr_cleanup ; 3 bytes

rb (patch_pos-03h)

db "PATCH",0   ; reserve 6 bytes
cr_cleanup:
mov     di,di ; 2b nop (hot patch)
push    eax
push    ecx
push    edx

; when we're in V86 don't clean
smsw    ax
and     eax,1

jnz     skip_clean

  mov     eax, cr0
  and     eax, 0E005003Fh
  mov     cr0, eax

  xor     eax, eax
  mov     cr2, eax
  mov     cr3, eax

  mov     eax, cr4
  and     eax, 040207h  ; clear all except VME PVI TSD + OSFXSR OSXSAVE (SIMD95)
  mov     cr4, eax

  xor     eax, eax
  xor     edx, edx
  mov     ecx, 0C0000080h
  wrmsr

  ; same as 'mov dx,msg' but position independent
  call near @f
  @@:
  pop    dx
  add    dx, (msg - @b)

	; DOS print
  mov     ah,09h
  int     21h

skip_clean:

pop edx
pop eax
pop eax
jmp code_end

msg: db "(Patcher9x) Control Registers are clean!",0Dh,0Ah,'$'
rb 9 ; pad code block to 16 bytes

code_end:
  jmp return_pos
