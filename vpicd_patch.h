/******************************************************************************
 * MIT No Attribution
 *
 * Copyright 2026 Jaroslav Hensl <emulator@emulace.cz>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
*******************************************************************************/
#ifndef __PATCHER9X__VPICD_PATCH_H__INCLUDED__
#define __PATCHER9X__VPICD_PATCH_H__INCLUDED__

#include <patch.h>

/*
 * VPICD on Intel CPUs with APIC clears IA32_APIC_BASE.EN (bit 11) at APIC
 * detection and again before each 8259 reinit. On CPUs with x2APIC locked
 * (IA32_XAPIC_DISABLE_STATUS.LEGACY_XAPIC_DISABLED = 1) bit 10 (EXTD) is
 * set, and clearing EN while EXTD is set raises #GP per Intel SDM. Both
 * WRMSR opcodes are turned into NOPs.
 */

/* site 1: PIC reinit path (cmp [VPICD_apic_flag],0; jz +0xE; rdmsr; ...; wrmsr; mov al,11h) */
const uint8_t vpicd_x2apic_s1_orig[] = {
	0x80, 0x3D, 0x42, 0x2C, 0x00, 0x00, 0x00, 0x74, 0x0E, 0xB9, 0x1B, 0x00, 0x00, 0x00, 0x0F, 0x32,
	0x25, 0xFF, 0xF7, 0xFF, 0xFF, 0x0F, 0x30, 0xB0, 0x11,
};

const uint8_t vpicd_x2apic_s1_orig_check[] = {
	0xFF, 0xFF, 0xFF, 0x80,
};

const uint8_t vpicd_x2apic_s1[] = {
	0x80, 0x3D, 0x42, 0x2C, 0x00, 0x00, 0x00, 0x74, 0x0E, 0xB9, 0x1B, 0x00, 0x00, 0x00, 0x0F, 0x32,
	0x25, 0xFF, 0xF7, 0xFF, 0xFF, 0x90, 0x90, 0xB0, 0x11,
};

const uint8_t vpicd_x2apic_s1_modif[] = {
	0x00, 0x00, 0x06, 0x00,
};

const cpatch_t vpicd_x2apic_s1_cp = {
	vpicd_x2apic_s1, sizeof(vpicd_x2apic_s1),
	vpicd_x2apic_s1_orig, sizeof(vpicd_x2apic_s1_orig),
	vpicd_x2apic_s1_orig_check, sizeof(vpicd_x2apic_s1_orig_check),
	vpicd_x2apic_s1_modif, sizeof(vpicd_x2apic_s1_modif)
};

/* site 2: APIC detection (mov [VPICD_apic_flag],1; rdmsr; ...; wrmsr; add esp,4) */
const uint8_t vpicd_x2apic_s2_orig[] = {
	0xC6, 0x05, 0x42, 0x2C, 0x00, 0x00, 0x01, 0xB9, 0x1B, 0x00, 0x00, 0x00, 0x0F, 0x32, 0x25, 0xFF,
	0xF7, 0xFF, 0xFF, 0x0F, 0x30, 0x83, 0xC4, 0x04,
};

const uint8_t vpicd_x2apic_s2_orig_check[] = {
	0xFF, 0xFF, 0xFF,
};

const uint8_t vpicd_x2apic_s2[] = {
	0xC6, 0x05, 0x42, 0x2C, 0x00, 0x00, 0x01, 0xB9, 0x1B, 0x00, 0x00, 0x00, 0x0F, 0x32, 0x25, 0xFF,
	0xF7, 0xFF, 0xFF, 0x90, 0x90, 0x83, 0xC4, 0x04,
};

const uint8_t vpicd_x2apic_s2_modif[] = {
	0x00, 0x00, 0x18,
};

const cpatch_t vpicd_x2apic_s2_cp = {
	vpicd_x2apic_s2, sizeof(vpicd_x2apic_s2),
	vpicd_x2apic_s2_orig, sizeof(vpicd_x2apic_s2_orig),
	vpicd_x2apic_s2_orig_check, sizeof(vpicd_x2apic_s2_orig_check),
	vpicd_x2apic_s2_modif, sizeof(vpicd_x2apic_s2_modif)
};

#endif /* __PATCHER9X__VPICD_PATCH_H__INCLUDED__ */
