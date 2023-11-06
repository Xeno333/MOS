;Filename = KERNEL_functions.inc
;Type = part of kernel.
;Description = All system functions.
;

;Notes:
;Need to add memcopy.
;Need to add COM1 port read and write.
;Need to add RTC.
;out 0x70, al
;in al, 0x71


random:
    ;check for rdrand support
    bt qword [supported_extensions], 0
    jnc .random_old

    ;new procsesers
    .random_rand:
        pushfq
        rdrand rax
        popfq
        ret

    ;old processers
    .random_old:
        push rbx

        mov rbx, [timer]
        or rbx, rax
        not rbx
        mov rax, rbx

        pop rbx
        ret

string:
    ;Compair strings.
    ;RSI
    ;RDI
    .strcmp:
        pushf
        cld
        push rsi
        push rdi
        .scloop:
            cmp byte [rsi], 0
            je .scloope0
            cmpsb
            je .scloop
        .scloope1:
            mov rax, 1
            pop rdi
            pop rsi
            popf
            ret
        .scloope0:
            cmp byte [rdi], 0
            jne .scloope1
            xor rax, rax
            pop rdi
            pop rsi
            popf
            ret

    ;String len
    ;PRAMS:
    ;RSI = pointer to string.
    ;RETURNS:
    ;RAX = len.
    .str_len:
        push rsi
        xor rax, rax
        dec rsi
        .str_len_l:
            inc rsi
            inc rax
            cmp byte [rsi], 0
            jne .str_len_l
        dec rax
        pop rsi
        ret

    ;String Copy
    ;PRAMS:
    ;RSI = Pointer to string.
    ;RDI = pointer to place to copy string to.
    .str_copy:
        push rdi
        push rsi
        push rax
        push rcx

        call string.str_len
        mov rcx, rax
        inc rcx
        rep movsb

        pop rcx
        pop rax
        pop rsi
        pop rdi
        ret

    ;Clears string.
    ;PRAMS:
    ;RCX = size.
    ;RSI = pointer to string.
    .str_clear:
        push rsi
        push rax
        xor rax, rax
        .str_clearl:
            mov byte [rsi], 0
            inc rsi
            inc rax
            cmp rax, rcx
            jne .str_clearl

            pop rax
            pop rsi
            ret

    ;String contains char.
    ;RSI string pointer.
    ;AL charicter.
    ;Returns RAX, 0 if charicter is found, 1 if not found.
    .strcntc:
        pushf
        cld
        push rsi
        .strcntc_loop:
            cmp [rsi], al
            je .strcntc_end_f
            cmp byte [rsi], 0
            je .strcntc_end_nf
            inc rsi
            jmp .strcntc_loop
        .strcntc_end_nf:
            mov rax, 1
            pop rsi
            popf
            ret
        .strcntc_end_f:
            xor rax, rax
            pop rsi
            popf
            ret

    ;String contains.
    ;RSI, RDI string pointer.
    ;Returns RAX, 0 if found, 1 if not found.
    .strcnt:
        pushf
        cld
        push rsi
        push rdi
        .strcnt_loop:
            cmp byte[rdi], 0
            je .strcnt_end_f
            cmp byte [rsi], 0
            je .strcnt_end_nf

            cmpsb

            je .strcnt_loop

            mov rdi, [rsp]

            cmp byte [rsi], 0
            je .strcnt_end_nf
            inc rsi
            jmp .strcnt_loop


        .strcnt_end_nf:
            mov rax, 1
            pop rsi
            pop rdi
            popf
            ret

        .strcnt_end_f:
            xor rax, rax
            pop rsi
            pop rdi
            popf
            ret







kernel:

    .atoi:
        push rcx
        push rax
        push rsi
        dec rsi
        mov word [.ans0], 0
        mov word [.mul], 1
        .atoil:
            mov al, [rsi]
            sub al, '0'
            dec rsi
            cmp al, 0
            je .atoil
            xor ah, ah
            mov cx, [.mul]
            mul cx
            add [.ans0], ax

            mov ax, [.mul]
            mov cx, 10
            mul cx
            mov [.mul], ax
            cmp rsi, rdi
            jl .atoie
            jmp .atoil

        .atoie:
        mov rdx, [.ans0]
        pop rdi
        pop rax
        pop rcx
        ret

        .ans0: dq 0
        .mul: dq 0

    .shift_data_r_o1:
        push rdi
        push rax
        mov byte [rdi+1], 0
        .shift_data_r_loop_o1:
            dec rdi
            mov al, byte[rdi-1]
            mov byte[rdi], al
            cmp rsi, rdi
            jne .shift_data_r_loop_o1
            dec rdi
            mov byte [rdi], 0
            pop rax
            pop rdi
            ret

    .shift_data_r:
        push rax
        xor rax, rax
        .shift_data_r_loop:
            inc rax
            call .shift_data_r_o1
            cmp rax, rcx
            jl .shift_data_r_loop
            pop rax
            ret


    .hex_to_char:
            pushf
            push rbx
            push rax

            mov rax, rdx

            mov rbx, KDATA.htab
            mov ah, al
            and al, 0xf ;grab just low nibble value
            xlatb
            xchg ah, al
            shr al, 4 ;to get high nibble
            and al, 0xf ;grab just low nibble value
            xlatb

            mov [rdi], ax

            pop rax
            pop rbx
            popf
            ret

    .shtoi:
        push rcx
        push rax
        push rsi
        mov word [.shtoians0], 0
        mov word [.shtoimul], 1
        .shtoil:
            mov al, [rsi-1]
            cmp al, 'F'
            je .shtoi_h_F
            cmp al, 'E'
            je .shtoi_h_E
            cmp al, 'D'
            je .shtoi_h_D
            cmp al, 'C'
            je .shtoi_h_C
            cmp al, 'B'
            je .shtoi_h_B
            cmp al, 'A'
            je .shtoi_h_A
            cmp al, 'f'
            je .shtoi_h_F
            cmp al, 'e'
            je .shtoi_h_E
            cmp al, 'd'
            je .shtoi_h_D
            cmp al, 'c'
            je .shtoi_h_C
            cmp al, 'b'
            je .shtoi_h_B
            cmp al, 'a'
            je .shtoi_h_A
            sub al, '0'
            .shtoi_h:
            cmp al, 0
            dec rsi
            je .shtoil
            xor ah, ah
            mov cx, [.shtoimul]
            mul cx
            add [.shtoians0], ax

            mov ax, [.shtoimul]
            mov cx, 16
            mul cx
            mov [.shtoimul], ax
            cmp rsi, rdi
            jl .shtoie
            jmp .shtoil

        .shtoie:
        mov rdx, [.shtoians0]
        pop rsi
        pop rax
        pop rcx
        ret

        .shtoi_h_F:
            mov al, 0x0f
            jmp .shtoi_h

        .shtoi_h_E:
            mov al, 0x0e
            jmp .shtoi_h

        .shtoi_h_D:
            mov al, 0x0d
            jmp .shtoi_h

        .shtoi_h_C:
            mov al, 0x0c
            jmp .shtoi_h

        .shtoi_h_B:
            mov al, 0x0b
            jmp .shtoi_h

        .shtoi_h_A:
            mov al, 0x0a
            jmp .shtoi_h

        .shtoians0: dq 0
        .shtoimul: dq 0




    .read_to_nl:
        push rdi
        push rsi
        push rax
        .read_to_nl_l:
            mov al, [rsi]
            mov [rdi], al
            inc rsi
            inc rdi
            cmp al, 0
            je .read_to_nl_end
            cmp al, 0x0a
            jne .read_to_nl_l
            .read_to_nl_end:

            mov byte [rdi-1], 0
            pop rax
            pop rsi
            pop rdi
            ret

    .shift_data_l:
        pushfq
        push rax
        push rdi
        push rsi
        push rdx

        mov rdx, rdi
        xor rax, rax

        .shift_data_l_l:
            mov rdi, rdx
            inc rax

            push rax

            .shift_data_l_l0:
                mov al, [rdi+1]
                mov [rdi], al
                inc rdi
                cmp rdi, rsi
                jne .shift_data_l_l0

            pop rax

            cmp rax, rcx
            jne .shift_data_l_l

            pop rdx
            pop rsi
            pop rdi
            pop rax
            popfq
            ret



    .get_time:
        mov rax, [timer]
        ret


    ;pass rax  =  number to wait
    .syswait:
        push rax

        mov al, PIC1_on
        out 0x21, al
        mov al, PIC2_on
        out 0xa1, al

        mov rax, [rsp]
        sti
        .syswait_loop:
            hlt
            dec rax
            jnz .syswait_loop

        pop rax
        ret

    
    .system_info_now:
        pushf
        push rax

        pushf
        push rbp
        push rsp
        push r15
        push r14
        push r13
        push r12
        push r11
        push r10
        push r9
        push r8
        push rdi
        push rsi
        push rdx
        push rcx
        push rbx
        push rax

        call .clear
        mov rax, rsp
        mov rsi, .system_info_now_str
        call kernel.printf
        add rsp, 17*8

        pop rax
        popf
        ret

        .system_info_now_str: db 'RAX: 0x%h', 0x0a, 'RBX: 0x%h', 0x0a, 'RCX: 0x%h', 0x0a, 'RDX: 0x%h', 0x0a, 'RSI: 0x%h', 0x0a, 'RDI: 0x%h', 0x0a, 'R8: 0x%h', 0x0a, 'R9: 0x%h', 0x0a, 'R10: 0x%h', 0x0a, 'R11: 0x%h', 0x0a, 'R12: 0x%h', 0x0a, 'R13: 0x%h', 0x0a, 'R14: 0x%h', 0x0a, 'R15: 0x%h', 0x0a, 'RSP: 0x%h', 0x0a, 'RBP: 0x%h', 0x0a, 'RFLAGS: %b', 0x0a, 0


;Get sys info.
    .get_sys_info:
        pushf
        cmp rax, 0
        je .get_sys_ver
        cmp rax, 1
        je .get_sys
        cmp rax, 2
        je .get_sys_emblem
        cmp rax, 3
        je .get_sys_mem

        popf
        ret

        ;get sys calls
        .get_sys_ver:
            push rsi

            mov rsi, OS_help_str4
            call string.str_copy

            pop rsi
            popf
            ret

        .get_sys:
            push rsi

            mov rsi, OS_kernel
            call string.str_copy

            pop rsi
            popf
            ret

        .get_sys_emblem:
            ;give adress
            mov rsi, emblem_flounder

            popf
            ret

        .get_sys_mem:
            mov rax, [ammount_of_mem_in_sys]

            popf
            ret











;Prints


    ;Prints data block located at RSI.
    ;RCX = size.
    .print_hex_block:
        pushfq
        push rsi
        push rax
        push rcx
        push rbx

        cmp rcx, 0
        je .print_hex_block_done

        cmp rbx, 0
        mov rax, 32
        cmove rbx, rax
        push rbx


        xor eax, eax
        .print_hex_block_l:
            lodsb
            xor ah, ah
            cmp ax, 0xF
            jg .print_hex_block_l_no_0

            push rax
            mov al, '0'
            call kernel.putc
            pop rax

            .print_hex_block_l_no_0:
            call kernel.puth

            dec rbx
            jz .print_hex_block_l_bpr_0

            mov al, ' '
            call kernel.putc

            .print_hex_block_l_bpr:

            dec rcx
            jnz .print_hex_block_l

        pop rbx
        pop rbx
        pop rcx
        pop rax
        pop rsi
        popfq
        .print_hex_block_done:
        ret

        .print_hex_block_l_bpr_0:
            mov rbx, [rsp]
            mov al, 0x0a
            call kernel.putc
            jmp .print_hex_block_l_bpr

    .putc:
        ;send to COM1
        call serial.send_com1
        cmp qword [KDATA.videomode], 1
        je VESA.putc
        cmp qword [KDATA.videomode], 0
        je VGA.putc
        ret
    

    .puts:
        pushf
        cld;Clear drirection flag.
        push rax
        push rsi
        .puts_loop:
            lodsb
            cmp al, '\'
            je .puts_var_escape
            call kernel.putc
            cmp al, 0x00
            jne .puts_loop

        .puts_end:
            dec qword[VESA.loc]
            dec qword[VESA.num]
            pop rsi
            pop rax
            popf
            ret

        .puts_var_escape:
            lodsb
            cmp al, 'n'
            je .puts_var_escape_n

            ;None of the above.
            push rax
            mov al, '\'
            call kernel.putc
            pop rax
            call kernel.putc
            jmp .puts_loop

        .puts_var_escape_n:
            mov al, 0x0a
            call kernel.putc
            jmp .puts_loop

    ;printf.
    ;Local vars.
    ;RBP-8 = printf_rax_backup
    .printf:
        pushf
        cld;Clear drirection flag.
        ;All args exept rsi are put on the stack. Push args on in backwoarfs order.
        push rax
        push rbp
        mov rbp, rsp
        sub rsp, 8;Reserve
        mov [rbp-8], rax;printf_rax_backup
        .printf_loop:
            lodsb
            cmp al, '%'
            je .printf_var_insert
            cmp al, '\'
            je .printf_var_escape

            call kernel.putc

            cmp al, 0x00
            jne .printf_loop

        .printf_end:
            dec qword[VESA.loc]
            dec qword[VESA.num]
            mov rsp, rbp
            pop rbp
            pop rax
            popf
            ret

        .printf_var_escape:
            lodsb
            cmp al, 'n'
            je .printf_var_escape_n

            ;None of the above.
            push rax
            mov al, '\'
            call kernel.putc
            pop rax
            call kernel.putc
            jmp .printf_loop

        .printf_var_escape_n:
            mov al, 0x0a
            call kernel.putc
            jmp .printf_loop


        .printf_var_insert:
            lodsb
            cmp al, 's'
            je .printf_var_insert_str
            cmp al, 'h'
            je .printf_var_insert_hex
            cmp al, 'x'
            je .printf_var_insert_hex
            cmp al, 'b'
            je .printf_var_insert_bin
            cmp al, 'd'
            je .printf_var_insert_dec

            ;None of the above.
            push rax
            mov al, '%'
            call kernel.putc
            pop rax
            call kernel.putc
            jmp .printf_loop

        .printf_var_insert_str:
            mov rax, [rbp-8];printf_rax_backup

            push rsi
            mov rsi, [rax]
            call kernel.puts
            pop rsi

            add qword [rbp-8], 8;printf_rax_backup
            jmp .printf_loop

        .printf_var_insert_hex:
            mov rax, [rbp-8];printf_rax_backup

            mov rax, [rax]
            call kernel.puth

            add qword [rbp-8], 8;printf_rax_backup
            jmp .printf_loop

        .printf_var_insert_dec:
            mov rax, [rbp-8];printf_rax_backup

            mov rax, [rax]
            call kernel.puti

            add qword [rbp-8], 8;printf_rax_backup
            jmp .printf_loop

        .printf_var_insert_bin:
            mov rax, [rbp-8];printf_rax_backup

            mov rax, [rax]
            call kernel.putb

            add qword [rbp-8], 8;printf_rax_backup
            jmp .printf_loop

    ;Puth .
    ;Prints hex value of RAX.
    ;Local vars
    ;RBP-8 = puth_code.
    ;Len 8 only 2 bytes used.
    .puth:
        pushf
        cld;Clear drirection flag.
        push rax
        push rcx
        push rbx
        push rdx
        push rbp
        mov rbp, rsp

        mov rcx, 17
        mov rdx, rax

        mov rbx, 0xf000000000000000

        .puth_prep:
            mov rax, rdx
            rol rdx, 4
            and rax, rbx ;grab just low nibble value
            dec rcx
            jz .puth_end_0
            cmp rax, 0
            je .puth_prep

        ror rdx, 4

        mov rbx, KDATA.htab
        .puth_byte:
            rol rdx, 4
            mov al, dl
            and al, 0xf ;grab just low nibble value
            xlatb
            call kernel.putc

            dec rcx
            jnz .puth_byte
            jmp .puth_end

        .puth_end_0:
            mov al, '0'
            call kernel.putc


        .puth_end:
            mov rsp, rbp
            pop rbp
            pop rdx
            pop rbx
            pop rcx
            pop rax
            popf
            ret

    ;Puti.
    ;Prints int value of RAX.
    ;Local vars
    ;RBP-8 = puth_code.
    ;Len 8 only 2 bytes used.
    .puti:
        pushf
        cld;Clear drirection flag.
        push rax
        push rcx
        push rbx
        push rdx
        push r10
        push rbp
        mov rbp, rsp

        mov rcx, 19
        mov rbx, 1000000000000000000


        bt rax, 63
        jnc .puti_not_singed

        neg rax

        push rax
        mov al, '-'
        call kernel.putc
        pop rax

        .puti_not_singed:

        cmp rax, 10
        jl .puti_end_one_digit

        .puti_prep:
            push rax
            xor rdx, rdx
            div rbx

            push rax
            xor rdx, rdx
            mov rax, rbx
            mov rbx, 10
            div rbx
            mov rbx, rax
            pop rax

            dec rcx
            jz .puti_end_0

            cmp rax, 0
            pop rax
            je .puti_prep


        push rax
        mov rax, 10
        mul rbx
        mov rbx, rax
        pop rax

        .puti_byte:
            push rax
            xor rdx, rdx
            div rbx

            ;Level just finished.
            push rax
            push rdx
            mul rbx
            mov r10, rax
            pop rdx
            pop rax

            add al, '0'
            call kernel.putc

            ;Next level down.
            push r10
            push rdx
            mov rax, rbx
            xor rdx, rdx
            mov r10, 10
            div r10
            mov rbx, rax
            pop rdx
            pop r10

            pop rax
            sub rax, r10;Clear highest 10s place.


            dec rcx
            jnz .puti_byte

        .puti_end_good:
            mov rax, rdx
            add rax, '0'
            call kernel.putc
            jmp .puti_end

        .puti_end_one_digit:
            add rax, '0'
            call kernel.putc
            jmp .puti_end

        .puti_end_0:
            pop rax
            mov al, '0'
            call kernel.putc


        .puti_end:
            mov rsp, rbp
            pop rbp
            pop r10
            pop rdx
            pop rbx
            pop rcx
            pop rax
            popf
            ret






    .putb:
        pushf
        cld;Clear drirection flag.
        push rax
        push rcx
        push rbx

        xchg rax, rcx;save
        mov al, '0'
        call kernel.putc
        mov al, 'b'
        call kernel.putc

        mov rax, rcx;restore
        mov rcx, 64;num_of times
        mov rbx, .bintab

        .putb_loop:
            rol rax, 1

            push rax
            and rax, 1
            xlatb
            call kernel.putc
            pop rax

            dec rcx
            jne .putb_loop

        .putb_end:
            pop rbx
            pop rcx
            pop rax
            popf
            ret

        .bintab: db "01"


    .clear:
        jmp VESA.clear
