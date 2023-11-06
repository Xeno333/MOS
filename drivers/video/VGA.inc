

    
VGA:
    .putc:
        pushf
        push rdi
        
        cmp al, 0x0a
        je .nl

        mov rdi, [.vgapos]
        mov byte [rdi], al
        mov byte [rdi+1], 15
        add qword [.vgapos], 2

        dec qword [.vga_char_left_on_line]
        dec qword [.vga_char_left_on_line]
        jz .nl

        inc qword[.c]
        cmp qword[.c], 79
        je .nl

        pop rdi
        popf
        ret


    .nl:
        push rcx

        inc qword [.nlc]
        cmp qword[.nlc], 24
        je .scroll

        mov rcx, [.vga_char_left_on_line]
        add [.vgapos], rcx

        mov qword [.vga_char_left_on_line], 160
        mov qword[.c], 0

        pop rcx 
        pop rdi
        popf
        ret
    
    .scroll:
        push rdi
        push rsi
        push rcx
        push rax

        mov rdi, 0xB8000
        mov rsi, 0xB8000+(160)
        mov rcx, 160*23

        .scroll_l:
            mov rax, [rsi]
            mov [rdi], rax
            inc rsi
            inc rdi
            dec rcx
            jne .scroll_l

        pop rax
        pop rcx
        pop rsi
        pop rdi

        mov qword[.nlc], 22

        mov qword[.vgapos], 0xB8000+(160*23)

        mov qword [.vga_char_left_on_line], 160
        mov qword[.c], 0

        pop rcx 
        pop rdi
        popf
        ret



.vars:
    .vgapos: dq 0xB8000
    .vga_char_left_on_line: dq 158
    .nlc: dq 0
    .c: dq 0