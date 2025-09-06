; SPDX-License-Identifier: LSL-1.4
;
; -- BEGIN LICENSE HEADER --
; The Wind/Tempest Project
;
; File:        sys/arch/amd64/boot.asm
; Author(s):   Russian95 <russian95@tempestfoundation.org>
;              (https://github.com/Russian95CrE)
; Maintainer:  Tempest Foundation <development@tempestfoundation.org>
; Link:        https://wtsrc.tempestfoundation.org
;
; Copyright (C) 2025 Tempest Foundation
; Licensed under the Liberty Software License, Version 1.4
; -- END OF LICENSE HEADER --
;

section .multiboot_header
align 8
header_start:
    dd 0xE85250D6         ; magic
    dd 0                  ; architecture
    dd header_end - header_start ; header length
    dd 0x100000000 - (0xE85250D6 + 0 + (header_end - header_start)) ; checksum

    ; Framebuffer tag
    dw 5                  ; type
    dw 0                  ; flags (0=required)
    dd 20                 ; size = 2 (type) + 2 (flags) + 4 (size) + 4 (width) + 4 (height) + 4 (depth)
    dd 1024               ; width
    dd 768                ; height
    dd 32                 ; depth

    ; Padding to align the end tag to 8 bytes.
    dd 0

    ; End tag
    dw 0                  ; type
    dw 0                  ; flags
    dd 8                  ; size

header_end:

section .text
bits 32
global _start
_start:
    ; Save the multiboot info pointer. It's in EBX.
    mov ebp, ebx

    ; Zero out all tables
    mov edi, pml4_table
    mov ecx, 4096 * 6
    xor eax, eax
    rep stosb

    ; PML4[0] -> PML3
    mov eax, pml3_table
    or eax, 0b11
    mov [pml4_table], eax

    ; PML4[256] -> PML3 (for high virtual addresses 0xFFFF800000000000)
    mov eax, pml3_table
    or eax, 0b11
    mov [pml4_table + 256*8], eax

    ; PML3[0..3] -> PML2_0..3
    mov eax, pml2_table_0
    or eax, 0b11
    mov [pml3_table + 0*8], eax
    mov eax, pml2_table_1
    or eax, 0b11
    mov [pml3_table + 1*8], eax
    mov eax, pml2_table_2
    or eax, 0b11
    mov [pml3_table + 2*8], eax
    mov eax, pml2_table_3
    or eax, 0b11
    mov [pml3_table + 3*8], eax

    ; Map 4 GiB using 2MiB pages
    mov ecx, 0
.map_4gib:
    mov eax, ecx
    imul eax, 0x200000
    or eax, 0b10000011
    mov edx, ecx
    shr edx, 9         ; edx = which PML2 table (0..3)
    mov esi, ecx
    and esi, 0x1FF     ; esi = index in PML2 table (0..511)
    
    ; Select the correct PML2 table
    mov edi, pml2_table_0
    cmp edx, 0
    je .set_entry
    mov edi, pml2_table_1
    cmp edx, 1
    je .set_entry
    mov edi, pml2_table_2
    cmp edx, 2
    je .set_entry
    mov edi, pml2_table_3
.set_entry:
    mov [edi + esi*8], eax
    inc ecx
    cmp ecx, 2048
    jl .map_4gib

    ; Enable PAE.
    mov eax, cr4
    ; Enable PAE (bit 5) and SSE support: OSFXSR (bit 9) | OSXMMEXCPT (bit 10)
    or eax, (1 << 5) | (1 << 9) | (1 << 10)
    mov cr4, eax

    ; Load PML4 address into CR3.
    mov eax, pml4_table
    mov cr3, eax

    ; Enable Long Mode.
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable Paging.
    mov eax, cr0
    ; Enable Paging (bit 31) and MP (bit 1) required for SSE/FPU
    or eax, (1 << 31) | (1 << 1)
    mov cr0, eax

    ; Load the GDT.
    lgdt [gdt_pointer]

    ; Jump to long mode.
    jmp 0x08:long_mode

section .bss
align 4096
pml4_table:      resb 4096
pml3_table:      resb 4096
pml2_table_0:    resb 4096
pml2_table_1:    resb 4096
pml2_table_2:    resb 4096
pml2_table_3:    resb 4096
stack_bottom:    resb 16384
stack_top:

section .rodata
gdt:
    dq 0 ; Null descriptor.
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53) ; Code descriptor.
gdt_pointer:
    dw $ - gdt - 1
    dq gdt

section .text
bits 64
long_mode:
    ; Point all segment registers to the null descriptor.
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax

    ; Set up the stack.
    mov rsp, stack_top
    
    ; Pass the multiboot info pointer to the kernel.
    mov rdi, rbp

    ; Call the C kernel.
    extern start_kernel
    call start_kernel

    ; Hang forever if the kernel returns.
.hang:
    cli
    hlt
    jmp .hang 
