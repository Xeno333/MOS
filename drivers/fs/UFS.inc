;UFS.inc
;Driver for UFS Filesystem.
;State = Under development.
;UFS driver modual for Kernel.
;Follows MVFS FSDI.

;Notes:
;Must use local vars!






;Parms
;RAX = Function.
;Others.
;Returns
;RAX = Return code
UFS_call:
    cmp rax, 0;Mount.
    je UFS_DRIVER.mount

    cmp rax, 1;Read.
    je UFS_DRIVER.read

    cmp rax, 3;Get FS type ID.
    je UFS_DRIVER.Get_FS_type_ID

    cmp rax, 4;Ping.
    je UFS_DRIVER.Ping

    cmp rax, 5;Unmount.
    je UFS_DRIVER.unmount

    cmp rax, 6;List.
    je UFS_DRIVER.list

    cmp rax, 7;Check for right FS.
    je UFS_DRIVER.check_right_FS


    mov rax, 0xffffffffffffffff;No such Function.
    ret




UFS_DRIVER:



    ;Mount UFS Filesystem
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

        mov rax, [UFS_DATA.UFS_SIG]
        cmp [rdi], rax
        jne .mount_err


        ;Version
        mov rax, [UFS_DATA.UFS_Version_suport_MAX]
        cmp [rdi+0x50], rax
        jg .mount_err

        xor rax, rax
        jmp .mount_done

        .mount_err_disk:
            mov rax, 0x07
            jmp .mount_done

        .mount_err:
            mov rax, 0x02
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



    ;Read for UFS FS.
    ;Prams:
    ;R8 = Sector start of partion.
    ;R11 = Partion Length.
    ;RDX = Disk num.
    ;RSI = Pointer to file path+name.
    ;RCX = Directory seperator char. (ASCII)
    ;RDI = Load to.
    ;Returns
    ;RAX = Return code(0 = secsess, 2 = File not found, 3 = Not a file.)
    ;Notes:
    ;rbp-8 = disk
    ;rbp-16 = Partion offset.
    ;rbp-24 = Bytes per cluster.
    ;rbp-32 = Data Section start.
    ;rbp-40 = Root Entry.
    ;rbp-48 = 512 secotrs per cluster.
    ;rbp-512-16-32-8 = Addrs of resurved block, size = cluster size.
    .read:
        ;Save Regs
        pushfq
        push rdi
        push rsi
        push r14
        push r13
        push r12
        push r10
        push r9
        push rbx
        push rcx
        push rdx
        push rbp
        push rsi
        push rdi
        push r8
        push r11
        mov rbp, rsp

        sub rsp, 8
        mov [rbp-8], rdx

        ;Partion offset
        sub rsp, 8
        mov [rbp-16], r8

        mov rax, r8
        sub rsp, 512
        lea rdi, [rbp-512-16]
        mov rcx, 1
        call Disk_io_funtions.read

        ;BPC.
        mov rcx, [rdi+8]
        sub rsp, 8
        mov [rbp-512-16-8], rcx
        mov r13, rcx;save
        ;DS start.
        mov rbx, [rdi+(0x28)]
        sub rsp, 8
        mov [rbp-512-16-16], rbx
        ;RE.
        mov rax, [rdi+0x30]
        sub rsp, 8
        mov [rbp-512-16-24], rax

        ;512 sectors per cluster.
        mov rax, 512
        xor rdx, rdx
        xchg rax, rcx
        div rcx
        sub rsp, 8
        mov [rbp-512-16-32], rax

        ;Put Root (512)sector in rax.
        mov rax, [rbp-512-16-24]
        add rax, [rbp-512-16-16]
        mul qword [rbp-512-16-32]
        add rax, r8
        mov r9, rax


        sub rsp, 8
        mov rax, r13;use
        call kmalloc
        mov [rbp-512-16-32-8], rsi


        ;R9 = Directory entry.
        .read_search_loop:
            mov rax, r9
            mov rdi, [rbp-512-16-32-8]
            mov rdx, [rbp-8]
            mov rcx, [rbp-512-16-8]
            call Disk_io_funtions.read_block_size_in_bytes

            test qword[rdi], 1;Test for F0, that is the file flag.
            jnz .read_err_FNF

            ;Get offset.
            mov rax, [rdi+0x18];Contense list.
            add rax, [rbp-512-16-16]
            xor rbx, rbx
            mul qword [rbp-512-16-32]
            add rax, r8
            mov r10, rax

            mov r12, [rdi+0x20];Number of objects.

            ;Get dir name.
            mov rsi, [rbp+8*3]
            sub rsp, 0xF8
            lea rdi, [rbp-512-16-32-8-0xF8];memloc
            mov rcx, [rbp+8*6];Dir sep char.
            mov rax, 0xF8;Max file name length.
            .read_loop_get_next_dir_name:
                dec rax
                jz .read_loop_get_next_dir_name_done
                cmp byte[rsi], cl
                je .read_loop_get_next_dir_name_done
                movsb
                cmp byte [rsi], 0
                jne .read_loop_get_next_dir_name

            .read_loop_get_next_dir_name_done:
            mov byte[rdi], 0
            inc rsi
            mov [rbp+8*3], rsi

            ;Get number of entries per Contense List.
            mov rax, [rbp-512-16-8]
            mov rcx, 8
            xor rdx, rdx
            div rcx
            mov r14, 2

            xor r13, r13;Entry always skip 0 it point to next entry.
            ;R14 = number of entries per contens list
            ;R13 = Entry number.
            ;R12 = Num of objects.
            ;Search for file name in contense list.
            .read_search_loop_l2:
                inc r13
                mov rax, r10
                mov rdi, [rbp-512-16-32-8]
                mov rdx, [rbp-8]
                mov rcx, [rbp-512-16-8]
                call Disk_io_funtions.read_block_size_in_bytes

                ;Get offset.
                mov rax, [rdi+(r13*8)];Pointer.
                add rax, [rbp-512-16-16]
                xor rbx, rbx
                mul qword [rbp-512-16-32]
                add rax, r8
                ;Read.
                mov rdi, [rbp-512-16-32-8]
                mov rdx, [rbp-8]
                mov rcx, [rbp-512-16-8]
                call Disk_io_funtions.read_block_size_in_bytes


                ;Get num of entries and store in rcx.
                mov rax, [rbp-512-16-8]
                mov rcx, 256;Size of entry.
                xor rdx, rdx
                div rcx
                mov rcx, rax

                ;Find file name.
                .read_search_loop_l3:
                    lea rsi, [rbp-512-16-32-8-0xF8]
                    call string.strcmp
                    cmp rax, 0
                    je .read_file_found

                    ;Check for max number of objects.
                    dec r12
                    jz .read_err_FNF

                    add rdi, 0x100
                    dec rcx
                    jnz .read_search_loop_l3




            dec r14
            jnz .read_search_loop_l2

            xor r13, r13;Entry always skip 0 it point to next entry.

            ;Get number of entries per Contense List.
            mov rax, [rbp-512-16-8]
            mov rcx, 8
            xor rdx, rdx
            div rcx
            mov r14, rax

            ;Next entry.
            mov rax, r10
            mov rdi, [rbp-512-16-32-8]
            mov rdx, [rbp-8]
            mov rcx, [rbp-512-16-8]
            call Disk_io_funtions.read_block_size_in_bytes
            mov r10, [rdi]

            jmp .read_search_loop_l2




        .read_file_found:
            mov rax, [rdi+0xF8]
            add rax, [rbp-512-16-16]
            xor rbx, rbx
            mul qword [rbp-512-16-32]
            add rax, r8
            mov r9, rax

            mov rsi, [rbp+8*3]
            cmp byte[rsi-1], 0;Check if last byte of file that we are searching for was NULL
            jne .read_search_loop

            ;rax set by code 2 sections up.
            mov rdi, [rbp-512-16-32-8]
            mov rdx, [rbp-8]
            mov rcx, [rbp-512-16-8]
            call Disk_io_funtions.read_block_size_in_bytes

            test qword[rdi], 1;Test for F0, that is the file flag.
            jz .read_err_FNF

            mov r12, [rdi+0x20];Number of objects.

            ;Get offset of Contense list.
            mov rax, [rdi+0x18];Contense list.
            add rax, [rbp-512-16-16]
            xor rbx, rbx
            mul qword [rbp-512-16-32]
            add rax, r8
            mov r10, rax

            mov rax, [rbp-512-16-8]
            add rsp, 0xF0;For old name but leave 8 bytes.
            call kmalloc
            mov [rbp-512-16-32-8-8], rsi


            ;Do read here.
            .read_read_contents_list:
                mov rcx, [rbp-512-16-8]
                mov rax, r10
                mov rdi, [rbp-512-16-32-8]
                mov rdx, [rbp-8]
                call Disk_io_funtions.read_block_size_in_bytes

                ;Get next block.
                mov rax, [rdi]
                add rax, [rbp-512-16-16]
                xor rbx, rbx
                mul qword [rbp-512-16-32]
                add rax, r8
                mov r10, rax

                mov rax, [rbp-512-16-8]
                mov rcx, 8;Size of entry.
                xor rdx, rdx
                div rcx
                mov rcx, rax

                xor r13, r13

                .read_read_blocks:
                    ;Get offset.
                    inc r13
                    mov rax, [rdi+(r13*8)];Pointer.
                    ;Offset from start of disk.
                    add rax, [rbp-512-16-16]
                    xor rbx, rbx
                    mul qword [rbp-512-16-32]
                    add rax, r8

                    .read_read_blocks_read_block:
                        mov rcx, [rbp-512-16-8]
                        mov rdi, [rbp-512-16-32-8-8]
                        mov rdx, [rbp-8]
                        call Disk_io_funtions.read_block_size_in_bytes

                        mov rsi, rdi
                        mov rdi, [rbp+8*2]
                        mov rcx, [rbp-512-16-8]
                        .read_read_blocks_read_block_copy:
                            movsb
                            dec r12
                            jz .read_readfile_done
                            dec rcx
                            jnz .read_read_blocks_read_block_copy

                        mov [rbp+8*2], rdi

                    dec rcx
                    jnz .read_read_blocks




                jmp .read_read_contents_list


        .read_readfile_done:
            mov byte[rdi], 0

            mov rsi, [rbp-512-16-32-8-8]
            call kfree

            mov rsi, [rbp-512-16-32-8]
            call kfree

            xor rax, rax
            jmp .read_done


        .read_err_disk:
            mov rax, 0x07
            jmp .read_done

        .read_err_FNF:
            mov rsi, [rbp-512-16-32-8]
            call kfree
            mov rax, 0x02

        .read_done:
            ;Restore Regs
            mov rsp, rbp
            pop r11
            pop r8
            pop rdi
            pop rsi
            pop rbp
            pop rdx
            pop rcx
            pop rbx
            pop r9
            pop r10
            pop r12
            pop r13
            pop r14
            pop rsi
            pop rdi
            popfq

            ret


    ;Get_FS_type_ID
    ;Prams:
    ;None.
    ;Returns
    ;RSI = Memory location of FS name string.
    .Get_FS_type_ID:
        lea rsi, [UFS_DATA.UFS_ID]
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

        cmp r9, [UFS_DATA.GPT_TYPE_low]
        jne .check_right_FS_err
        cmp r10, [UFS_DATA.GPT_TYPE_high]
        jne .check_right_FS_err

        xor rax, rax
        ret


        .check_right_FS_MBR:
            cmp R8, [UFS_DATA.MBR_FS_ID]
            jne .check_right_FS_err

            xor rax, rax
            ret

        .check_right_FS_err:
            mov rax, 1
            ret



UFS_DATA:
    .MBR_FS_ID: dq 0x49
    .GPT_TYPE_low: dq 0x0123456789ABCDEF
    .GPT_TYPE_high: dq 0x0000000000000000
    .UFS_ID: db 'UFS     ', 0
    .UFS_SIG: db 'UFS PART', 0
    .UFS_Version_suport_MAX dq 0x1000;v0.1.00



