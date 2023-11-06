



input:

    .getc:
        mov rax, 3
        call VESA.putc_UBMcs
        dec qword[VESA.loc]
        dec qword[VESA.num]

        pushf
        cmp qword[KDATA.SYS_input_flags], 1
        je .getc_COM_in

        ;If none set Keyboard.
        or r15, 1;Flag to read keyboard.
        jmp .getc_go

        .getc_COM_in:
            or r15, 10b
            mov rax, 1111111111111111111111111111111111111111111111111111111111111110b
            and r15, rax

        .getc_go:
        xor al, al
        xor bl, bl
        sti
        .getc_loop:
            hlt
            cmp bl, 1;Serial input.
            je .getc_end
            test al, 0x80
            jnz .getc_loop

        .getc_end:
            popf
            cmp qword[KDATA.SYS_input_flags], 1
            je .getc_COM_in_end

            ;If none Keyboard.
            xor r15, 1;Keyboard.
            jmp .getc_go_end

            .getc_COM_in_end:
                xor r15, 10b
                jmp .getc_past_keyboard_interpet

            .getc_go_end:
            xor r15, 1;Flag to read keyboard.

            cmp al, 0x3A
            je .set_cl

            cmp al, 0x0e
            je .get_back

            and al, 0x7f
            lea rbx, [.OS_ascii_table]
            dec al
            xlatb

            .getc_past_keyboard_interpet:

            cmp al, 0
            je .getc
            cmp byte [.capl], 1
            je .cplf
            .cplfe:
            cmp al, 0x0d
            je .getc_end_nl
            cmp al, 0x0a
            jne .getc_end_no_nl
            .getc_end_nl:
                putc ' '
                mov al, 0x0a
            .getc_end_no_nl:
                call VESA.putc
                ret
        .get_back:
            mov al, 08
            jmp .getc_end_no_nl

        .cplf:
            push rsi

            mov rsi, .other_key

            .cpl_find_other:
                cmp byte[rsi], 0
                je .cpl_find_other_dnf
                cmp al, [rsi]
                je .cpl_find_other_found
                inc rsi
                jmp .cpl_find_other

            .cpl_find_other_found:
                push rbx
                sub rsi, .other_key
                mov al, sil
                mov ebx, .other_val
                xlatb
                pop rbx
                pop rsi
                jmp .cplfe


            .cpl_find_other_dnf:
                pop rsi

                cmp al, 0x61
                jl .cplfe
                cmp al, 0x7a
                jg .cplfe

                xor al, 100000b
                jmp .cplfe

        .set_cl:
            cmp byte [.capl], 1
            je .set_clo
            mov byte [.capl], 1
            jmp .getc

        .set_clo:
            mov byte [.capl], 0
            jmp .getc



    .other_key: db  "`1234567890-=[]\;',./", 0
    .other_val: db  '~!@#$%^&*()_+{}|:"<>?', 0
;                                                  tab                  ENTER                         rightshift          leftshift  alt keys
    .OS_ascii_table: db 0x01, "1234567890-=", 0x0E, 0x0, 'qwertyuiop[]', 0x0a, 0, "asdfghjkl;'", 0x0a, 0x0, 0, "zxcvbnm,./", 0, 0x0, 0x00, " ", 0, 0,0

        .capl: db 0, 0


    .gets:
        push r9
        push rax
        xchg rsi, rdi
        mov r9, rdi
        .gets_loop:
            xor ax, ax
            call .getc
            cmp al, 0x0a
            je .gets_loop_end
            cmp al, 08
            je .gets_backspace
            stosb
            cmp rsi, rdi
            je .gets_loop_endE
            jmp	.gets_loop
        .gets_loop_end:
            xor al, al
            stosb
            pop rax
            pop r9
            ret
        .gets_loop_endE:
            pop rax
            pop r9
            ret
        .gets_backspace:
            cmp rdi, r9
            jle .gets_loop
            dec qword[VESA.loc]
            dec qword[VESA.num]
            mov al, 0x00
            call VESA.putc
            dec qword[VESA.loc]
            dec qword[VESA.num]
            dec qword[VESA.loc]
            dec qword[VESA.num]
            dec rdi
            xor al, al
            stosb
            dec rdi
            jmp .gets_loop


    .wait_til_key:
        push rax
        mov r15, 'k'
        .wait_til_key_loop:
            hlt
            test al, 0x80
            jnz .wait_til_key_loop
            pop rax
            xor r15, r15
            ret


ps2:

.key_pressed:
    pushfq
    push rax
    cmp byte [STARTED], 0
    je .starting

    ;get key
    in al, 0x60

    ;Check for function key.
    cmp al, 0x3f
    je .int_0x21_h_f5;F5 key is terminate.
    cmp al, 0x40
    je .int_0x21_h_f6;F6 key is system info now.
    cmp al, 0x41
    je POWER.shut_down;F7 key is poweroff system.
    cmp al, 0x42
    je POWER.reboot;F8 key is reboot system.
    cmp al, 0x43
    je .int_0x21_h_f9;F9 key is Togle COM1 input.

    test r15, 1
    jnz .key_board_is_on

    .starting:;if key board is not being used.
        pop rax
        popfq
        ret

    .key_board_is_on:;if key board is not being used keep rax.
        pop qword[void]
        popfq
        ret




    ;function keys
    .int_0x21_h_f5:;Does not respect function key pause.
        cli

        mov rcx, [curent_task]
        call Kill

        sti
        jmp $;Wait for task to take its place.


    .int_0x21_h_f6:
        cmp byte [function_key_pause], 1
        je .starting
        mov byte [function_key_pause], 1
        call kernel.system_info_now
        ;Timer IRQ off
        push rax
        mov al, 11111101b
        out 0x21, al
        mov al, 11111101b
        out 0xa1, al
        sti
        .wait_til_key_loop:
            hlt
            test al, 0x80
            jnz .wait_til_key_loop
            ;PICs on
            mov al, PIC1_on
            out 0x21, al
            mov al, PIC1_on
            out 0xa1, al
            pop rax

        call kernel.clear
        mov byte [function_key_pause], 0
        jmp .starting

    ;Togle COM1 input.
    .int_0x21_h_f9:
            xor qword[KDATA.SYS_input_flags], 1
            jmp .starting











