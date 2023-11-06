
POWER:


    .shut_down:
        ;Add shutdown code here.
        mov rsi, .poweroff_string
        call kernel.puts
        ;qemu poweroff
        mov dx, 0x604
        mov ax, 0x2000
        out dx, ax
        ;else halt
        cli
        hlt

        .poweroff_string: db 0x0a, 0x0a, 'You can now safely poweroff your computer.', 0

    .reboot:
        ;Add shutdown code here.
        .wait1:
            in al, 0x64
            test al, 00000010b
            jne  .wait1

        mov  al, 0xFE
        out  0x64, al

        cli
        lidt [0]
        int 0

        lea rsi, [.error_reboot_string]
        call kernel.puts
        cli
        hlt

        .error_reboot_string: db 0x0a, 0x0a, 'Your computer could not reboot but you can now safely poweroff your computer and restart it yourself.', 0

    .sleep:
        push rax
        xor al, al
        write_port 0x21 , 11111101b
        .sleep_loop:
            hlt
            test al, 0x80
            jnz .sleep_loop

        write_port 0x21 , PIC1_on
        pop rax
        ret





