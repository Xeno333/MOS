;Filename = VESA.inc
;Type = part of kernel (for now).
;Description = VESA video Driver
;

;Notes:
;add VESA graphecs mode



;text
VESA:


.loc: dq vid_mem
.loc2: dq vid_mem
.num: dq 0
.nl: dq 0



;draws char on screen
.draw:
    pushf
    cld;Clear drirection flag.
    push rdi
    push rax

    cmp qword[.num], 100
    je .drawnl
    .drawret_from_nl:

    mov rdi, [.loc]
    inc qword [.loc]
    inc qword[.num]

    mov rax, [rsi]
    mov [rdi+100], al
    mov [rdi+200], ah
    ror rax, 16
    mov [rdi+300], al
    mov [rdi+400], ah
    ror rax, 16
    mov [rdi+500], al
    mov [rdi+600], ah
    ror rax, 16
    mov [rdi+700], al
    mov [rdi+800], ah

    mov rax, [rsi+8]
    mov [rdi+900], al
    mov [rdi+1000], ah
    ror rax, 16
    mov [rdi+1100], al
    mov [rdi+1200], ah
    ror rax, 16
    mov [rdi+1300], al
    mov [rdi+1400], ah
    ror rax, 16
    mov [rdi+1500], al
    mov [rdi+1600], ah

    pop rax
    pop rdi
    popf
    ret

    .drawnl:
        mov qword[.num], 0
        call .put_nl
        jmp .drawret_from_nl


;draws char on screen from reg rbx and rcx
.reg_putc:
    pushf
    cld;Clear drirection flag.
    push rdi
    push rax

    cmp qword[.num], 100
    je .reg_putcnl
    .reg_putcret_from_nl:

    mov rdi, [.loc]
    inc qword [.loc]
    inc qword[.num]

    mov rax, rbx
    mov [rdi+100], al
    mov [rdi+200], ah
    ror rax, 16
    mov [rdi+300], al
    mov [rdi+400], ah
    ror rax, 16
    mov [rdi+500], al
    mov [rdi+600], ah
    ror rax, 16
    mov [rdi+700], al
    mov [rdi+800], ah

    mov rax, rcx
    mov [rdi+900], al
    mov [rdi+1000], ah
    ror rax, 16
    mov [rdi+1100], al
    mov [rdi+1200], ah
    ror rax, 16
    mov [rdi+1300], al
    mov [rdi+1400], ah
    ror rax, 16
    mov [rdi+1500], al
    mov [rdi+1600], ah

    pop rax
    pop rdi
    popf
    ret

    .reg_putcnl:
        mov qword[.num], 0
        call .put_nl
        jmp .reg_putcret_from_nl



.put_nl:
    pushf
    cld;Clear drirection flag.
    push rax
    inc qword[.nl]

    cmp qword[.nl], 37
    je .put_nl_scroll

    add qword [.loc2], 100*16
    mov rax, [.loc2]
    mov qword [.loc], rax
    mov qword [.num], 0
    pop rax
    popf
    ret

    .put_nl_scroll:
        push rcx
        push rsi
        push rdi

        mov rcx, 100*75
        mov rsi, vid_mem+1600
        mov rdi, vid_mem

        rep movsq

        mov rax, [.loc2]
        mov qword [.loc], rax
        mov qword [.num], 0
        dec qword[.nl]

        pop rdi
        pop rsi
        pop rcx
        pop rax
        popf
        ret


;new putc
.putc:
    pushf
    cld;Clear drirection flag.
    push rdx
    cmp al, 0x0a
    je .putc_asciinl
    push rsi
    push rcx
    push rax
    mov rsi, UBM_char_standerd_map
    dec rsi
    xor rcx, rcx
    .putc_asciifindloop:
        inc rsi
        inc rcx

        cmp rsi, UBM_char_standerd_map_end
        je .putc_asciinotachar

        cmp al, [rsi]
        jne .putc_asciifindloop

        dec rcx
        mov rax, 16
        xchg rax, rcx
        mul rcx
        add rax, UBM_chars_8_by_16_format_chars

        mov rsi, rax
        call .draw

        pop rax
        pop rcx
        pop rsi
        pop rdx
        popf
        ret
    .putc_asciinl:
        call .put_nl
        pop rdx
        popf
        ret
    ;No char.
    .putc_asciinotachar:
        mov rsi, UBM_chars_8_by_16_format_chars.Unkown
        call .draw
        pop rax
        pop rcx
        pop rsi
        pop rdx
        popf
        ret




.putc_UBMcs:
    pushf
    cld;Clear drirection flag.
    push rax
    cmp rax, 1
    je .putc_UBMcsnl
    push rsi
    push rcx
    mov rcx, 16
    mul rcx
    add rax, UBM_chars_8_by_16_format_chars
    mov rsi, rax
    call .draw
    pop rcx
    pop rsi
    pop rax
    popf
    ret

    .putc_UBMcsnl:
        call .put_nl
        pop rax
        popf
        ret



.clear:
    pushf
    cld
    push rax
    push rcx
    push rdi

    mov rdi, vid_mem
    mov rcx, 7500
    xor rax, rax
    rep stosq

    mov qword[.loc], vid_mem
    mov qword[.loc2], vid_mem
    mov qword[.num], 0x00
    mov qword[.nl], 0x00

    pop rdi
    pop rcx
    pop rax
    popf
    ret











;Data

current:























