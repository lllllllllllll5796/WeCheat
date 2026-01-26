.CODE

vmcall_intel PROC
mov rax,r8
vmcall
ret
vmcall_intel ENDP

vmcall_amd PROC
mov rax,r8
vmmcall
ret
vmcall_amd ENDP

RunWithKernelStack PROC
stac
swapgs
mov gs:[10h],rsp
mov rsp,gs:[1A8h]
sub rsp,40h					;The kernel stack must be aligned as 0x10 bytes
sti
call rdx
cli
add rsp,40h
mov rsp,gs:[10h]
swapgs
clac
ret
RunWithKernelStack ENDP

FuckStack_Stub PROC
    pop r11
    add rsp, 8
    mov rax, [rsp + 24]
  
    mov r10, [rax]
    mov [rsp], r10
        
    mov r10, [rax + 8]
    mov [rax + 8], r11
     
    mov [rax + 16], rdi
    lea rdi, fixup
    mov [rax], rdi
    mov rdi, rax
    
    jmp r10
     
fixup:
    sub rsp, 16
    mov rcx, rdi
    mov rdi, [rcx + 16]
    jmp QWORD PTR [rcx + 8]
FuckStack_Stub ENDP


Asm_Vf_Pending_DB PROC
    mov r10,rcx
    mov qword ptr[r10],0
    pushfq
    mov ecx, ss
    or dword ptr [rsp], 0100h
    popfq
    mov ss, ecx
    cpuid
    mov qword ptr[r10],1
    nop
    ret
Asm_Vf_Pending_DB ENDP


END