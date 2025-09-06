; SPDX-License-Identifier: LSL-1.4
;
; -- BEGIN LICENSE HEADER --
; The Wind/Tempest Project
;
; File:        sys/arch/amd64/isr.asm
; Author(s):   Russian95 <russian95@tempestfoundation.org>
;              (https://github.com/Russian95CrE)
; Maintainer:  Tempest Foundation <development@tempestfoundation.org>
; Link:        https://wtsrc.tempestfoundation.org
;
; Copyright (C) 2025 Tempest Foundation
; Licensed under the Liberty Software License, Version 1.4
; -- END OF LICENSE HEADER --
;

global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
global isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
global isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
global isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31

; Common ISR stub
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli
    push 0      ; A placeholder for the error code
    push %1     ; The interrupt number
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    ; The CPU pushes the error code automatically for these
    push %1     ; The interrupt number
    jmp isr_common_stub
%endmacro

; Define ISRs
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE 8
ISR_NOERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_ERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_ERRCODE 30
ISR_NOERRCODE 31

; Common ISR stub
extern isr_handler
isr_common_stub:
    ; Save all registers (push order must be the reverse of registers_t
    ; field order so that the layout matches the structure exactly).
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    ; The interrupt number and error code are already at the correct positions
    ; on the stack after all registers are pushed
    
    ; Call C handler
    mov rdi, rsp
    call isr_handler
    
    ; Restore all registers (reverse order of pushes)
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
    
    ; Remove error code and interrupt number
    add rsp, 16
    
    ; Return from interrupt
    iretq 