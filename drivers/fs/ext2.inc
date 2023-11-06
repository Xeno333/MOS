

;Parms
;RAX = Function.
;Others.
;Returns
;RAX = Return code
ext2_call:
    cmp rax, 0;Mount.
    je ext2_DRIVER.mount

    cmp rax, 1;Read.
    je ext2_DRIVER.read

    cmp rax, 3;Get FS type ID.
    je ext2_DRIVER.Get_FS_type_ID

    cmp rax, 4;Ping.
    je ext2_DRIVER.Ping

    cmp rax, 5;Unmount.
    je ext2_DRIVER.unmount

    cmp rax, 6;List.
    je ext2_DRIVER.list

    cmp rax, 7;Check for right FS.
    je ext2_DRIVER.check_right_FS


    mov rax, 0xffffffffffffffff;No such Function.
    ret




ext2_DRIVER:


    ;Mount Filesystem
    ;Prams:
    ;R8 = Sector start of partion.
    ;RDX = Disk num.
    ;RBX = Partion.
    ;Returns
    ;RAX = Return code(0 = secsess, 1 = Filesystem not found (Corrupted?))
    ;Local vars
    .mount:
        push rbp
        ;Save Regs here.
        mov rbp, rsp



        .mount_no_err:
            xor rax, rax
            jmp .mount_done

        .mount_err_disk:
            mov rax, 0x07
            jmp .mount_done

        .mount_done:
            ;Restore Regs here.
            mov rsp, rbp
            pop rbp
            ret





    ;Unmount
    ;Prams:
    ;R8 = Sector start of partion.
    ;RDX = Disk num.(Unused)
    ;RBX = Partion.(Unused)
    ;Returns
    ;RAX = Return code(0 = secsess, 2 = File not found, 3 = Not a file)
    .unmount:
        xor rax, rax
        ret



    ;Read.
    ;Prams:
    ;RDX = Disk num.
    ;RBX = Partion.
    ;RSI = Pointer to file name.
    ;RCX = Directory seperator char.
    ;RDI = Load to.
    ;Returns
    ;RAX = Return code(0 = secsess, 2 = File not found, 3 = Not a file.)
    ;Notes:
    .read:
        push rbp
        ;Save Regs Here
        mov rbp, rsp

        .read_no_err:
            xor rax, rax
            jmp .read_done

        .read_err_disk:
            mov rax, 0x07
            jmp .read_done

        .read_err_FNF:
            mov rax, 0x02
            jmp .read_done

        .read_done:
            ;Restore Regs
            mov rsp, rbp
            pop rbp
            ret


    ;Get_FS_type_ID
    ;Prams:
    ;None.
    ;Returns
    ;RSI = Memory location of FS name string.
    .Get_FS_type_ID:
        lea rsi, [ext2_DRIVER_DATA.FS_ID]
        ret



    ;Ping
    ;Prams:
    ;None.
    ;Returns
    ;RAX = 0x6(Ping recived.)
    .Ping:
        mov rax, 0x6
        ret


    .list:
        push rbp
        ;Save Regs Here
        mov rbp, rsp

        .list_list_done:
            xor rax, rax
            jmp .list_done

        .list_err_disk:
            mov rax, 0x07
            jmp .list_done

        .list_err_FNF:
            mov rax, 0x02
            jmp .list_done

        .list_done:
            ;Restore Regs here.
            mov rsp, rbp
            pop rbp
            ret


    .check_right_FS:
        push rbx
        push rcx
        push rdx
        push rbp
        mov rbp, rsp

        lea rax, [rbx+2]
        sub rsp, 512;Reserve
        lea rdi, [rbp-512]
        mov rcx, 1
        call Disk_io_funtions.read

        cmp word [rbp-512+56], 0xef53
        je .check_right_FS_ext2

        mov eax, 1
        jmp .check_right_FS_end

        .check_right_FS_ext2:
            xor eax, eax

        .check_right_FS_end:
        mov rsp, rbp
        pop rbp
        pop rdx
        pop rcx
        pop rbx
        ret



ext2_DRIVER_DATA:
    ;Put FS ID string here no more than 8 bytes Padded with ' '.
    .FS_ID: db 'ext2    ', 0



