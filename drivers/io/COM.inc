



serial:

;COM1
.COM1_data_in:
    ;Check if COM1 is being used or if to ignor.
    test r15, 10b
    jz .pass

    push rdx

    mov dx, 0x3f8
    in al, dx
    mov bl, 1;Accepted.

    ;clear r15 flag
    xor r15, 10b

    pop rdx
    ret

    ;If not wanting input.
    .pass:
        ret


.get_com1:
    pushf
    or r15, 10b
    sti
    .get_com1_loop:
        hlt
        cmp bl, 1
        jne .get_com1_loop

    popf
    ret

.send_com1:
    push rdx
    mov dx, 0x3f8
    out dx, al
    pop rdx
    ret
