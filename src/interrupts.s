extern general_handler

section .text

%macro MAKEHANDLERS 1
    global exception_%1
    exception_%1:
    %if %1 = 8 || (%1 >= 10 && %1 <= 14) || %1 = 17
    push %1
    %else
    push 0
    push %1
    %endif
    jmp common_stub
%endmacro

%assign i 0
%rep 32
    MAKEHANDLERS i
%assign i i+1
%endrep

.hang:
    hlt
    jmp .hang

common_stub:
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp
    mov rsi, cr2
    mov rbx, rsp
    and rsp, -16
    sub rsp, 8
    cld
    call general_handler
    mov rsp, rbx
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax

    add rsp, 16
    
    iretq