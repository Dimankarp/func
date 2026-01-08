
; Linux x86_64 specific
%define SYS_READ 0
%define SYS_WRITE 1
%define STD_IN 0
%define STD_OUT 1

section .text
global write, read


write:
    sub rsp, 16
    mov byte[rsp], dil
    
    mov rax, SYS_WRITE
    mov rdi, STD_OUT
    mov rsi, rsp
    mov rdx, 1
    syscall

    add rsp, 16
    
    ret
    
read:
    sub rsp, 16

    mov rax, SYS_READ
    mov rdi, STD_IN
    mov rsi, rsp
    mov rdx, 1
    syscall
    
    cmp rax, 1
    jne .fail

    movzx rax, byte[rsp]
    add rsp, 16
    ret

.fail:
    ; return rax
    add rsp, 16
    ret
