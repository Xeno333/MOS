;USTAR.inc
;Driver for USTAR Filesystem.
;State = Under development.
;USTAR driver modual for Kernel.
;Follows MVFS FSDI.

;Notes:
;Must use local vars!






;Parms
;RAX = Function.
;Others.
;Returns
;RAX = Return code
USTAR_call:
    cmp rax, 0;Mount.
    je USTAR_DRIVER.mount

    cmp rax, 1;Read.
    je USTAR_DRIVER.read

    cmp rax, 3;Get FS type ID.
    je USTAR_DRIVER.Get_FS_type_ID

    cmp rax, 4;Ping.
    je USTAR_DRIVER.Ping

    cmp rax, 5;Unmount.
    je USTAR_DRIVER.unmount

    cmp rax, 6;List.
    je USTAR_DRIVER.list

    cmp rax, 7;Check for right FS.
    je USTAR_DRIVER.check_right_FS


    mov rax, 0xffffffffffffffff;No such Function.
    ret




USTAR_DRIVER:

    ;PRAMS:
    ;RSI = oct string.
    ;RBX = String length.
    ;Returns:
    ;RAX = bin value.
    .oct_to_bin:
        push rbx
        push rdx
        push rcx
        push rsi

        xor rax, rax
        xor rdx, rdx
        mov rcx, 8
        .oct_to_bin_l:
            mul rcx
            mov dl, [rsi]
            sub rdx, '0'
            add rax, rdx
            inc rsi
            dec rbx
            jnz .oct_to_bin_l

        pop rsi
        pop rcx
        pop rdx
        pop rbx
        ret



    ;Mount USTAR Filesystem
    ;Prams:
    ;R8 = Sector start of partion.
    ;RDX = Disk num.
    ;RBX = Partion.
    ;Returns
    ;RAX = Return code(0 = secsess, 1 = Filesystem not found (Corrupted?))
    ;Local vars
    .mount:
        ;Save Regs
        pushfq
        push rcx
        push rbp
        push rsi
        push rdi
        mov rbp, rsp

        mov rax, r8
        sub rsp, 512
        lea rdi, [rbp-512]
        mov rcx, 1
        call Disk_io_funtions.read

        cmp rax, 'ND'
        je .mount_err_disk

        xor rax, rax
        jmp .mount_done

        .mount_err_disk:
            mov rax, 0x07
            jmp .mount_done

        .mount_done:
            ;Restore Regs
            mov rsp, rbp
            pop rdi
            pop rsi
            pop rbp
            pop rcx
            popfq

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



    ;Read for USTAR FS.
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
        ;Save Regs
        pushfq
        push rbx
        push rcx
        push rdx
        push rbp
        push rsi
        push rdi
        push r9
        mov rbp, rsp

        sub rsp, 8
        mov [rbp-8], rdx

        xor r9, r9
        sub rsp, 512
        .read_find_loop:
            ;Read block.
            mov rax, r8
            add rax, r9
            lea rdi, [rbp-520]
            mov rcx, 1
            mov rdx, [rbp-8]
            call Disk_io_funtions.read

            cmp rax, 'ND'
            je .read_err_disk

            ;Compare.
            push rdi
            mov rsi, [rbp+16]
            .read_find_loop_check:
                cmp byte[rsi], 0
                je .read_file_found
                cmpsb
                je .read_find_loop_check
            pop rdi

            ;Check for 'ustar' sig.
            cmp dword[rdi+257], 'usta'
            jne .read_err_FNF
            cmp byte[rdi+257+4], 'r'
            jne .read_err_FNF

            mov rbx, 11
            lea rsi, [rdi+124]
            call .oct_to_bin
            ;Get next file location.
            add rax, 511
            mov rbx, 512
            xor rdx, rdx
            div rbx
            inc rax

            add r9, rax
            jmp .read_find_loop

        .read_file_found:
            cmp byte[rdi], 0
            pop rdi
            jne .read_find_loop

            ;Make sure it is a file not a directory.
            cmp byte[rdi+156], '0'
            jg .read_find_loop

            ;Get size in sectors.
            mov rbx, 11
            lea rsi, [rdi+124]
            call .oct_to_bin
            add rax, 511
            mov rbx, 512
            xor rdx, rdx
            div rbx

            ;Read
            inc r9
            mov rcx, rax
            mov rax, r8
            add rax, r9
            mov rdi, [rbp+8]
            mov rdx, [rbp-8]
            call Disk_io_funtions.read

            cmp rax, 'ND'
            je .read_err_disk

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
            pop r9
            pop rdi
            pop rsi
            pop rbp
            pop rdx
            pop rcx
            pop rbx
            popfq

            ret


    ;Get_FS_type_ID
    ;Prams:
    ;None.
    ;Returns
    ;RSI = Memory location of FS name string.
    .Get_FS_type_ID:
        lea rsi, [USTAR_DATA.USTAR_ID]
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
        ;Save Regs
        pushfq
        push rbx
        push rcx
        push rdx
        push rbp
        push rsi
        push rdi
        push r9
        mov rbp, rsp

        sub rsp, 8
        mov [rbp-8], rdx

        xor r9, r9
        sub rsp, 512
        .list_find_loop:
            ;Read block.
            mov rax, r8
            add rax, r9
            lea rdi, [rbp-520]
            mov rcx, 1
            mov rdx, [rbp-8]
            call Disk_io_funtions.read

            cmp rax, 'ND'
            je .list_err_disk

            ;Check for 'ustar' sig.
            cmp dword[rdi+257], 'usta'
            jne .list_list_done
            cmp byte[rdi+257+4], 'r'
            jne .list_list_done

            ;Compare.
            mov rsi, rdi
            call kernel.puts
            mov al, 0x0a
            call kernel.putc

            mov rbx, 11
            lea rsi, [rdi+124]
            call .oct_to_bin
            ;Get next file location.
            add rax, 511
            mov rbx, 512
            xor rdx, rdx
            div rbx
            inc rax

            add r9, rax
            jmp .list_find_loop

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
            ;Restore Regs
            mov rsp, rbp
            pop r9
            pop rdi
            pop rsi
            pop rbp
            pop rdx
            pop rcx
            pop rbx
            popfq

            ret



    ;Prams:
    ;RBX = Sector start of partion.
    ;RCX = Partion Length.
    ;RDX = Disk num.
    ;R8 = MBR Type ID
    ;R9:R10 = GPT ID only if RBX = 0. Not supported Yet.
    ;Returns:
    ;RAX = Return code.
    .check_right_FS:
        cmp r8, 0
        jne .check_right_FS_MBR

        cmp r9, [USTAR_DATA.GPT_TYPE_low]
        jne .check_right_FS_err
        cmp r10, [USTAR_DATA.GPT_TYPE_high]
        jne .check_right_FS_err

        xor rax, rax
        ret


        .check_right_FS_MBR:
            cmp R8, [USTAR_DATA.MBR_FS_ID]
            jne .check_right_FS_err

            xor rax, rax
            ret

        .check_right_FS_err:
            mov rax, 1
            ret



USTAR_DATA:
    .MBR_FS_ID: dq 0x37
    .GPT_TYPE_low: dq 0xF123456789ABCDEF
    .GPT_TYPE_high: dq 0xFEDCBA987654321F
    .USTAR_ID: db 'USTAR   ', 0

    .USTAR_in_archive_ID: db 'ustar', 0



