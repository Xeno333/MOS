;Mem_Allacotion_manager.inc
;Allacatios and manages memory.

;For some resean it wasn't working now resureves 8 extra bytes.

;The heap allacaits data in a way simmaler to USTAR file system.
;If a block is freed it must be cleared allong with the header.
;Header:
;Length 8bytes long



kheap_init:
    mov ax, 0x0009;Heap allacotion table.
    call New_Page_manager_entry
    mov [Heap_section_list], rax;Save location
    mov qword [Heap_section_list+8], 0x200000;Save size

    ;Clear heap allacotion table.
    mov rsi, rax
    xor rax, rax
    mov rcx, 0x200000/8

    .loop:
        mov [rsi], rax
        add rsi, 8
        dec rcx
        jnz .loop

    ret


;prams
;RAX size.
;returns
;RAX return code.
;RSI location.
kmalloc:
    push rbx
    push rcx
    push rdx
    push rdi

    xor rcx, rcx
    mov rsi, [Heap_section_list]
    mov rdi, [Heap_section_list+8]
    add rdi, rsi;End.

    .loop_find_free:
        cmp rsi, rdi
        jge .next
        cmp qword[rsi], 0
        je .found

        add rsi, [rsi]

    .found:
        mov rdx, rsi
        add rsi, 8
        xor rbx, rbx
        add rax, 8;Extra 8 bytes?

        .loop_check_size:
            inc rbx
            cmp byte[rsi], 0
            jne .loop_find_free
            cmp rsi, rdi
            jge .next
            cmp rbx, rax
            jne .loop_check_size

        mov [rdx], rax
        lea rsi, [rdx+8]

        xor rax, rax
        pop rdi
        pop rdx
        pop rcx
        pop rbx
        ret

    ;Add "Add new page(s)."
    .error:
        mov rax, 1
        pop rdi
        pop rdx
        pop rcx
        pop rbx
        ret

    .next:
        inc rcx
        cmp qword [Heap_section_list+(rcx*2)], 0
        je .error

        mov rsi, [Heap_section_list+(rcx*2)]
        mov rdi, [Heap_section_list+(rcx*2)+8]
        add rdi, rsi
        jmp .loop_find_free

;RSI = block to free.

kfree:
    push rax
    push rcx
    push rsi
    xor rax, rax
    sub rsi, 8
    mov rcx, [rsi]

    .loop:
        mov [rsi], rax
        inc rsi
        dec rcx
        jnz .loop

    pop rsi
    pop rcx
    pop rax
    ret


;First qword is location second is size in bytes.
Heap_section_list: dq 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0




