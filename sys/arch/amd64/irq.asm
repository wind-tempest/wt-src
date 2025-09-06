; SPDX-License-Identifier: LSL-1.4
;
; -- BEGIN LICENSE HEADER --
; The Wind/Tempest Project
;
; File:        sys/arch/amd64/irq.asm
; Author(s):   Russian95 <russian95@tempestfoundation.org>
;              (https://github.com/Russian95CrE)
; Maintainer:  Tempest Foundation <development@tempestfoundation.org>
; Link:        https://wtsrc.tempestfoundation.org
;
; Copyright (C) 2025 Tempest Foundation
; Licensed under the Liberty Software License, Version 1.4
; -- END OF LICENSE HEADER --
;

global irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7
global irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15

; Common IRQ stub
%macro IRQ 2
global irq%1
irq%1:
    cli
    push 0      ; A placeholder for the error code
    push %2     ; The interrupt number
    jmp irq_common_stub
%endmacro

; Define IRQs
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

; Common IRQ stub
extern irq_handler
irq_common_stub:
    ; Save all registers (push order must be the reverse of registers_t
    ; field order so that the top of the stack (lowest address) begins
    ; with r15 and the layout matches the structure exactly).
    ; registers_t fields order (top -> bottom when on stack):
    ;   r15 r14 r13 r12 r11 r10 r9 r8 rdi rsi rbp rbx rdx rcx rax int_no err_code
    ; Therefore we push from bottom-most field upwards:
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
    
    ; Call C handler
    mov rdi, rsp
    call irq_handler
    
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