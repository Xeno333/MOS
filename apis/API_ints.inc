

;Need to redo syscall area

;system
int_0x30_sys_call_c:
    cld
    push rax
    mov al, PIC1_on
    out 0x21, al
    mov al, PIC2_on
    out 0xa1, al
    pop rax

    cmp r8, 0
    jl .sys_call_ret
    cmp r8, 36
    jg .sys_call_ret

    .sys_call:
        ;V
        push .sys_call_ret
        ;0
        cmp r8, 1
        je kernel.putc
        cmp r8, 2
        je kernel.puts
        cmp r8, 3
        je VESA.clear
        cmp r8, 4
        je kernel.print_hex_block

        ;POWER
        cmp r8, 5
        je POWER.shut_down
        cmp r8, 6
        je POWER.reboot
        ;PS/2
        cmp r8, 7
        je input.getc
        cmp r8, 8
        je input.gets
        ;string
        cmp r8, 9
        je string.strcmp
        cmp r8, 10
        je kernel.atoi
        cmp r8, 11
        je input.wait_til_key
        ;VESA
        cmp r8, 12
        je VESA.reg_putc
        cmp r8, 13
        je kernel.puti
        ;14-15 Free for use
        cmp r8, 16
        je string.strcntc
        cmp r8, 17
        je string.strcntc
        ;other
        ;cmp r8, 18
        ;je kernel.shift_data_r_o1
        cmp r8, 19
        je kernel.shift_data_r
        cmp r8, 20
        je kernel.hex_to_char
        cmp r8, 21
        je string.str_copy
        cmp r8, 22
        je kernel.shtoi
        cmp r8, 23
        je string.str_len
        cmp r8, 24
        je kernel.read_to_nl
        cmp r8, 25
        je random
        cmp r8, 26
        je kernel.shift_data_l
        ;27-28
        cmp r8, 29
        je kernel.get_time
        ;Output
        cmp r8, 30
        je kernel.puth
        cmp r8, 31
        je kernel.putb
        ;Get sys info.
        cmp r8, 32
        je kernel.get_sys_info
        ;printf
        cmp r8, 33
        je kernel.printf
        ;Disk.
        cmp r8, 34
        je MVFS_call
        ;Disk.
        cmp r8, 35
        je New_task;start_proc
        ;strclear.
        cmp r8, 36
        je string.str_clear
        ;COM1
        cmp r8, 37
        je serial.get_com1
        cmp r8, 38
        je serial.send_com1
        ;print

        pop qword [void]
    .sys_call_ret:
        iretq

int_0x31c:;Resurved.
    iretq


int_0x32_diskio_c:;Disk.
    call Disk_io_funtions
    iretq

int_0x33_kill_c:
    cli

    mov rcx, [curent_task]
    call Kill

    sti
    jmp $




