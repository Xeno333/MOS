;FAT16.inc
;Driver for FAT16 Filesystem.
;State = Under development.
;FAT16 driver modual for Kernel.
;Follows MVFS FSDI.

;Notes:






;Parms
;RAX = Function.
;Others.
;Returns
;RAX = Return code
FAT16_call:
    cmp rax, 0;Mount.
    je FAT16_DRIVER.mount

    cmp rax, 1;Read.
    je FAT16_DRIVER.read

    cmp rax, 2;Write.
    je FAT16_DRIVER.write

    cmp rax, 3;Get FS type ID.
    je FAT16_DRIVER.Get_FS_type_ID

    cmp rax, 4;Ping.
    je FAT16_DRIVER.Ping

    cmp rax, 5;Unmount.
    je FAT16_DRIVER.unmount

    cmp rax, 6;List.
    je FAT16_DRIVER.list

    cmp rax, 7;Check for right FS.
    je FAT16_DRIVER.check_right_FS


    mov rax, 0xffffffffffffffff;No such Function.
    ret




FAT16_DRIVER:
    ;Mount FAT16 Filesystem
    ;Prams:
    ;RDX = Disk num.
    ;RBX = Partion.
    ;Returns
    ;RAX = Return code(0 = secsess, 1 = Filesystem not found (Corrupted?))
    ;Local vars
    .mount:
        ;Save Regs
        pushfq
        push rbx
        push rcx
        push rdx
        push rbp
        mov rbp, rsp

        ;Pick disk
        sub rsp, 8;Reserve
        mov [rbp-8], rdx

        ;Sector of disk
        mov rax, r8

        ;Read BPB+EBPB
        sub rsp, 512
        lea rdi, [rbp-520]
        mov rcx, 1
        mov rdx, [rbp-8]
        call Disk_io_funtions.read

        cmp rax, 'ND'
        je .mount_err_disk


        ;RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec – 1)) / BPB_BytsPerSec

        xor rax, rax
        mov rcx, 32
        mov ax, [rbp-520+0x11];BPB_RootEntCnt
        mul rcx

        xor rcx, rcx
        mov cx, [rbp-520+0xb];BPB_BytsPerSec
        dec cx

        add rax, rcx

        xor rdx, rdx
        xor rcx, rcx
        mov cx, [rbp-520+0xb];BPB_BytsPerSec
        cmp cx, 0
        je .mount_err_not_FAT16
        div rcx


        ;Save RAX
        push rax

        ;If(BPB_FATSz16 != 0)
        ;FATSz = BPB_FATSz16;
        ;Else
        ;FATSz = BPB_FATSz32;
        xor rax, rax
        mov ax, [rbp-520+22]
        cmp ax, 0
        jne .mount_after_BPB_FATSz

        .mount_else_BPB_FATSz32:
        mov eax, [rbp-520+36]

        .mount_after_BPB_FATSz:
        mov rbx, rax


        ;If(BPB_TotSec16 != 0)
        ;TotSec = BPB_TotSec16;
        ;Else
        ;TotSec = BPB_TotSec32;
        xor rax, rax
        mov ax, [rbp-520+19]
        cmp ax, 0
        jne .mount_after_BPB_TotSec

        .mount_else_BPB_TotSec32:
        mov eax, [rbp-520+32]

        .mount_after_BPB_TotSec:
        mov rcx, rax


        ;DataSec = TotSec – (BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors);
        xor rax, rax
        mov al, [rbp-520+16]
        mul rbx

        xor rbx, rbx
        mov bx, [rbp-520+14]
        add rax, rbx

        pop rbx
        add rax, rbx

        sub rcx, rax
        mov rax, rcx

        ;CountofClusters = DataSec / BPB_SecPerClus;
        xor rcx, rcx
        mov cl, [rbp-520+13]
        cmp cl, 0
        je .mount_err_not_FAT16

        xor rdx, rdx
        div rcx

        cmp rax, 4085;FAT12
        jl .mount_err_not_FAT16

        cmp rax, 65525;FAT32
        jg .mount_err_not_FAT16



        .mount_done_no_err:
            ;Restore Regs
            mov rsp, rbp
            pop rbp
            pop rdx
            pop rcx
            pop rbx
            popfq

            ;RAX = 0 means passed.
            xor rax, rax
            ret

        .mount_err_disk:
            mov rax, 0x7
            jmp .mount_err

        .mount_err_not_FAT16:
            ;RAX = 1 means err Filesystem not FAT16 or Curupted.
            mov rax, 1

        .mount_err:
            ;Restore Regs
            mov rsp, rbp
            pop rbp
            pop rdx
            pop rcx
            pop rbx
            popfq

            ret





    ;Unmount
    ;Prams:
    ;RDX = Disk num.(Unused)
    ;RBX = Partion.(Unused)
    ;Returns
    ;RAX = Return code(0 = secsess, 2 = File not found, 3 = Not a file)
    .unmount:
        xor rax, rax
        ret



    ;write file from FAT16 Filesystem
    ;Prams:
    ;RDX = Disk num.
    ;RBX = Partion.
    ;RSI = Pointer to file name.
    ;RCX = Directory seperator char.
    ;RDI = Load from.
    ;Returns
    ;RAX = Return code(0 = secsess, 2 = File not found, 3 = Not a file.)
    ;Notes:
    .write:
        ;Save Regs
        pushfq
        push rdi
        push rsi
        push r14
        push r13
        push r12
        push r11
        push rbx
        push rcx
        push rdx
        push rbp
        push r8
        push r9
        push r10
        ;This is for the function.
        push rdi
        push rsi
        mov rbp, rsp

        xor r12, r12;This is a flag for the program.


        ;Pick disk
        sub rsp, 8;Reserve
        mov [rbp-8], rdx

        ;Sector of disk
        mov rax, r8;Backup for Partion offset

        ;Read BPB+EBPB.
        sub rsp, 512;Reserve
        lea rdi, [rbp-520]
        mov rcx, 1
        mov rdx, [rbp-8]
        call Disk_io_funtions.read

        cmp rax, 'ND'
        je .write_err_disk

        ;Check if FAT 16


        ;RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec – 1)) / BPB_BytsPerSec
        xor rax, rax
        mov rcx, 32
        mov ax, [rbp-520+0x11];BPB_RootEntCnt
        cmp ax, 0;Error, can't have 0 Root entry count.
        je .write_err_not_FAT16
        mul rcx
        xor rcx, rcx
        mov cx, [rbp-520+0xb];BPB_BytsPerSec
        cmp cx, 0;Error, can't have 0 bytes per sector.
        je .write_err_not_FAT16
        dec cx
        add rax, rcx
        xor rdx, rdx
        inc cx
        div rcx
        ;Save RAX
        push rax

        ;If(BPB_FATSz16 != 0)
        ;FATSz = BPB_FATSz16;
        ;Else
        ;FATSz = BPB_FATSz32;
        xor rax, rax
        mov ax, [rbp-520+22];BPB_FATSz16
        cmp ax, 0
        jne .write_after_BPB_FATSz
        .write_else_BPB_FATSz32:
        mov eax, [rbp-520+36];BPB_FATSz32
        .write_after_BPB_FATSz:
        mov rbx, rax

        ;If(BPB_TotSec16 != 0)
        ;TotSec = BPB_TotSec16;
        ;Else
        ;TotSec = BPB_TotSec32;
        xor rax, rax
        mov ax, [rbp-520+19];BPB_TotSec16
        cmp ax, 0
        jne .write_after_BPB_TotSec
        .write_else_BPB_TotSec32:
        mov eax, [rbp-520+32];BPB_TotSec32
        .write_after_BPB_TotSec:
        mov rcx, rax

        ;DataSec = TotSec – (BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors);
        xor rax, rax
        mov al, [rbp-520+16];BPB_NumFATs
        mul rbx
        xor rbx, rbx
        mov bx, [rbp-520+14];BPB_ResvdSecCnt
        add rax, rbx
        ;Get saved value.
        pop rbx
        add rax, rbx
        sub rcx, rax
        mov rax, rcx

        ;CountofClusters = DataSec / BPB_SecPerClus;
        xor rcx, rcx
        mov cl, [rbp-520+13];BPB_SecPerClus
        xor rdx, rdx
        div rcx

        cmp rax, 4085;FAT12
        jl .write_err_not_FAT16
        cmp rax, 65525;FAT32
        jg .write_err_not_FAT16




        ;Find and Read file.

        xor rax, rax
        mov al, [rbp-520+16];Num of FATs.
        xor rcx, rcx
        mov cx, [rbp-520+22];FAT16 size.

        mul rcx

        xor rcx, rcx
        mov cx, [rbp-520+14];Resurved sectors.
        add rax, rcx



        ;Get sector offset in 512 sectores.
        xor rbx, rbx
        mov bx, [rbp-520+0x0B];Bytes per sector.
        mul rbx
        xor rdx, rdx
        mov rcx, 512
        div rcx

        add rax, r8;Partion offset.
        mov r11, rax;Root offset.



        push rax
        ;Get offset of data section.
        xor rax, rax
        mov ax, [rbp-520+0x11]
        mov rcx, 32;Bytes per entry.
        mul rcx
        ;Get sector offset in 512 sectores.
        xor rdx, rdx
        mov rcx, 512
        div rcx

        ;Save data section offset.
        mov r13, r11
        add r13, rax



        ;Number of sectors.
        xor rax, rax
        mov ax, [rbp-520+0x0B];Bytes per sector.
        mov rbx, 512
        xor rdx, rdx
        div rbx

        xor rcx, rcx
        mov cl, [rbp-520+0x0D];Sectors per cluster.
        mul rcx
        mov r9, rax;Backup length of Cluster, in 512 byte sectors.

        ;Get length of Root.
        xor eax, eax
        mov ax, [rbp-520+17];Entries.
        ;Number of bytes.
        mov rcx, 32
        mul rcx
        ;512 byte sectors.
        mov rcx, 512
        div rcx
        mov rcx, rax

        ;Get location for block to be loaded to and size.
        push rcx
        mov rax, rcx
        mov rcx, 512
        mul rcx

        ;Allacate.
        call kmalloc
        mov rdi, rsi
        mov r12, rsi;Save Block in memory so it can be freed.
        ;Restore regs.
        pop rcx
        pop rax

        mov rdx, [rbp-8]
        call Disk_io_funtions.read

        cmp rax, 'ND'
        je .write_err_disk


        mov r10, 1;Root cluter number, number to add to resurved clusters to find cluster.


        xor ecx, ecx
        mov cx, [rbp-520+17];Entries for Root.

        mov r14, 0x1;Search for Dir Flag.
        sub rsi, 32;Back 1.

        .write_find_next_dir:
            mov rdi, [rbp]
            push rdi
            mov al, [rbp+56]
            .write_find_dir_pram_loop:
                cmp [rdi], al
                je .write_find_dir_pram_loop_end
                cmp byte [rdi], 0
                je .write_at_file
                inc rdi
                jmp .write_find_dir_pram_loop

            .write_find_dir_pram_loop_end:
                inc rdi
                mov [rbp], rdi
                pop rdi


            .write_loop_find_dir:
                add rsi, 32;Next entry.

                cmp rcx, 0;Check if we need to get next Block. It may have been missed last time
                je .write_write_loop_find_dir_get_next_Block

                ;Test for ATTR_VOLUME_ID.
                lea rax, [rsi+32]
                lea rbx, [rcx-1]
                test byte [rsi+11], 0x8;If any others are set this is VFAT
                cmovnz rsi, rax
                cmovnz rcx, rbx

                cmp rcx, 0;Check if we need to get next Block. It may have been missed last time
                je .write_write_loop_find_dir_get_next_Block

                ;Number of blocks.
                dec rcx


                ;Check if we are loking for a file or a directory.
                cmp r14, 0x0
                je .write_loop_find_dir_file

                ;Test for ATTR_DIRECTORY.
                test byte [rsi+11], 0x10
                jz .write_loop_find_dir
                jmp .write_loop_find_dir_file_done

                .write_loop_find_dir_file:
                    ;Test for ATTR_DIRECTORY.
                    test byte [rsi+11], 0x10
                    jnz .write_loop_find_dir

                .write_loop_find_dir_file_done:


                ;Check for end.
                cmp byte [rsi], 0
                je .write_err_DNF



                ;Save regs before name check.
                push rcx
                push rsi
                push rdi
                mov rcx, 11;Max length.
                cld
                .write_loop_find_dir_check:
                    cmpsb;Compare bytes
                    jne .write_loop_find_dir_check_no_end

                    dec rcx;Max 11 bytes.
                    jnz .write_loop_find_dir_check

                    mov al, [rbp+56]
                    cmp [rdi], al;If next byte is dir_sep_char done, we have found it.
                    je .write_loop_find_dir_check_end
                    cmp byte [rdi+1], 0;If next byte is 0 done, we have found it.
                    je .write_file_now

                ;If did not pass...
                .write_loop_find_dir_check_no_end:
                    mov al, [rbp+56]
                    cmp [rdi-1], al;If it was the end of RDI maby it is ' ' padded in RSI.
                    jne .write_loop_find_dir_check_no_end_end

                    ;...Check for ' ' padding.
                    dec rsi
                    .write_loop_find_dir_check_no_end_loop:
                        cmp byte[rsi], ' ';Check for ' ' padding.
                        jne .write_loop_find_dir_check_no_end_end;It was not ' ' padded.

                        dec rcx;Check for 11 byte limit.
                        jnz .write_loop_find_dir_check_no_end_loop

                    ;It was ' ' padded.
                    jmp .write_loop_find_dir_check_end

                .write_loop_find_dir_check_no_end_end:
                ;Restore and continue search.
                pop rdi
                pop rsi
                pop rcx

                cmp rcx, 0;Check if we need to get next Block.
                je .write_write_loop_find_dir_get_next_Block

                jmp .write_loop_find_dir


            ;Get next Cluster.
            .write_write_loop_find_dir_get_next_Block:
                cmp r10, 1
                je .write_err_DNF
                ;Get cluster number in FAT and offset for Entry.
                mov rax, r9;Size of Cluster in 512 Sectors.
                mov rcx, 512/2;Number of entries.
                mul rcx
                mov rcx, rax
                mov rax, r10
                xor rdx, rdx
                div rcx

                mov rbx, rdx;Number of entries.

                ;RAX = Cluster of FAT.
                add ax, [rbp-520+14];Resurved sectors.
                add rax, r8;Add partion offset.
                mov rcx, r9;Cluster length in 512 byte sectors.
                mov rdx, [rbp-8];Disk.
                mov rdi, r12;Location
                call Disk_io_funtions.read

                ;Get location of next cluster.
                lea rdx, [rbx*2];Entry.
                add rdx, rdi
                xor rax, rax
                mov ax, [rdx]

                ;Check for EOC.
                cmp ax, 0xffff
                je .write_err_DNF

                dec rax;Back one.

                push rax;Save RAX.

                mov rax, r13;Location of data section.
                ;Get sector offset of Next Cluster.
                push rax
                mov rax, r9;Sectors per cluster.
                mov rbx, r10;Get Cluster number.
                dec rbx;Go back one to get true Cluster number.
                mul rbx;SPC * Cluster = Sector Location.
                mov rcx, rax
                pop rax

                add rax, rcx;Add
                mov rcx, r9;Cluster length in 512 byte sectors.
                mov rdx, [rbp-8];Disk.
                mov rdi, r12;Location
                mov r11, rax
                call Disk_io_funtions.read

                ;Prepare for next search.
                mov rdi, rsi
                sub rsi, 32;Go back one.
                ;Number of entries per block.
                mov rax, r9;Get length of Block.
                mov rcx, 512
                mul rcx;Get byte value.
                mov rcx, 32;Bytes per entry.
                div rcx
                mov rcx, rax;Entries per Block.

                ;Save this clusters number.
                pop rax;Restore RAX.
                mov r10, rax

                jmp .write_loop_find_dir



            ;Found Dir move to next.
            .write_loop_find_dir_check_end:
                add rsp, 8;Pass the old RDI.
                pop rsi
                pop rcx
                push rdi


                xor rax, rax
                mov ax, [rsi+26];Cluster number.
                dec rax
                mov r10, rax

                mov rax, r13;Location of data section.
                ;Get sector offset of Next Cluster.
                push rax
                mov rax, r9;Sectors per cluster.
                mov rbx, r10;Get Cluster number.
                dec rbx;Go back one to get true Cluster number.
                mul rbx;SPC * Cluster = Sector Location.
                mov rcx, rax
                pop rax
                add rax, rcx;Add
                mov rcx, r9;Cluster length in 512 byte sectors.
                mov rdx, [rbp-8];Disk.
                mov rdi, r12;Location
                mov r11, rax
                call Disk_io_funtions.read

                ;Prepare for next search.
                mov rsi, r12
                sub rsi, 32;Go back one.
                ;Number of entries per block.
                mov rax, r9;Get length of Block.
                mov rcx, 512
                mul rcx;Get byte value.
                mov rcx, 64;Bytes per entry*2 since we are one the first one it will be skiped.
                div rcx
                mov rcx, rax;32 bit blocks per entry.

                pop rdi

                jmp .write_find_next_dir





        .write_at_file:
            ;When VFAT is added, Here is where I should check if paramater is LFN and if it is go to LFN function.

            pop rdi

            mov [rsp+1], rax
            sub rsp, 16;Make space on Stack to store File name in 8.3 format, VFAT not supported.
            push rsi
            lea rdi, [rsp+8]
            mov rsi, [rbp]
            mov rcx, 11

            .write_at_file_make_8_3_name_loop:
                mov al, [rsi]
                cmp al, 0
                je .write_at_file_make_8_3_name_loop_pad
                cmp al, '.'
                je .write_at_file_make_8_3_name_loop_pad_dot
                mov [rdi], al
                inc rsi
                inc rdi
                dec rcx
                jnz .write_at_file_make_8_3_name_loop

            .write_at_file_make_8_3_name_loop_pad_dot:
                inc rsi
                cmp rcx, 3
                jle .write_at_file_make_8_3_name_loop_pad_dot_ext
                mov byte [rdi], ' '
                inc rdi
                dec rsi
                dec rcx
                jmp .write_at_file_make_8_3_name_loop_pad_dot

            .write_at_file_make_8_3_name_loop_pad_dot_ext:
                mov al, [rsi]
                cmp al, 0
                je .write_at_file_make_8_3_name_loop_pad
                cmp rcx, 0
                je .write_at_file_make_8_3_name_loop_pad
                mov byte [rdi], al
                inc rdi
                inc rsi
                dec rcx
                jmp .write_at_file_make_8_3_name_loop_pad_dot_ext


            .write_at_file_make_8_3_name_loop_pad:
                cmp rcx, 0
                je .write_at_file_make_8_3_name_loop_pad_done
                mov byte [rdi], ' '
                inc rdi
                dec rcx
                jmp .write_at_file_make_8_3_name_loop_pad

            .write_at_file_make_8_3_name_loop_pad_done:
                mov byte [rdi], 0

                pop rsi

                ;Number of entries per block.
                mov rax, 512
                mul r9;Get byte value.
                mov rcx, 32;Bytes per entry.
                div rcx
                mov rcx, rax;32 bit blocks per entry.


                ;Prepare for next search.
                lea rdi, [rsp]
                xor r14, r14;Search for File flag.


                jmp .write_loop_find_dir




        .write_file_now:
            pop rdi
            pop rsi
            pop rcx

            mov rax, r9
            mov rbx, 512
            mul rbx

            mov rcx, [rbp+8*8];Size.
            xchg rax, rcx
            xor edx, edx
            div rcx
            cmp rdx, 0
            je .write_file_now_size_found

            inc rax

            .write_file_now_size_found:
                push rax

            xor r10, r10
            mov r10w, [rsi+26];Cluster number.
            cmp r10w, 0xffff
            jne .write_file_now_good_start

            mov r10, 2

            .write_file_now_loop_find_free:
                mov rax, r9;Sectors per cluster.
                mov rbx, 512
                mul rbx
                mov rcx, rax

                ;Get cluster number in FAT and offset for Entry.
                mov rax, r9;Size of Cluster in 512 Sectors.
                mov rcx, 512/2;Number of entries.
                mul rcx
                mov rcx, rax
                mov rax, r10
                xor rdx, rdx
                div rcx

                mov rbx, rdx;Number of entries.

                ;RAX = Cluster of FAT.
                add ax, [rbp-520+14];Resurved sectors.
                add rax, r8;Add partion offset.
                mov rcx, r9;Cluster length in 512 byte sectors.
                mov rdx, [rbp-8];Disk.
                mov rdi, r12;Location
                push rax
                call Disk_io_funtions.read

                ;Get location of next cluster.
                lea rdx, [rbx*2];Entry.
                add rdx, rdi
                xor rax, rax
                inc r10
                cmp word [rdx], 0
                pop rax
                jne .write_file_now_loop_find_free

                mov word [rdx], 0xffff

                ;RAX = Cluster.
                mov rcx, r9;Cluster length in 512 byte sectors.
                mov rdx, [rbp-8];Disk.
                mov rsi, r12;Location
                call Disk_io_funtions.write

                dec r10
                mov [rsi+26], r10w

            .write_file_now_good_start:

            mov rax, [rbp+8*8]
            mov [rsi+28], eax;Size in bytes.


            mov rcx, r9;Cluster length in 512 byte sectors.
            mov rdx, [rbp-8];Disk.
            mov rsi, r12;Location
            mov rax, r11
            call Disk_io_funtions.write

            pop rax
            dec rax

            push r10

            mov r11, rax


            .write_file_now_find_clusters:
                cmp r11, 0
                je .write_file_now_find_clusters_done

                push r10
                mov r10, 2
                ;Find next free block.
                .write_file_now_find_clusters_loop:
                    mov rax, r9;Sectors per cluster.
                    mov rbx, 512
                    mul rbx
                    mov rcx, rax

                    ;Get cluster number in FAT and offset for Entry.
                    mov rax, r9;Size of Cluster in 512 Sectors.
                    mov rcx, 512/2;Number of entries.
                    mul rcx
                    mov rcx, rax
                    mov rax, r10
                    xor rdx, rdx
                    div rcx

                    mov rbx, rdx;Number of entries.

                    ;RAX = Cluster of FAT.
                    add ax, [rbp-520+14];Resurved sectors.
                    add rax, r8;Add partion offset.
                    mov rcx, r9;Cluster length in 512 byte sectors.
                    mov rdx, [rbp-8];Disk.
                    mov rdi, r12;Location
                    push rax
                    call Disk_io_funtions.read
                    pop rax

                    ;Get location of next cluster.
                    lea rdx, [rbx*2];Entry.
                    add rdx, rdi
                    inc r10

                    cmp word [rdx], 0
                    jne .write_file_now_find_clusters_loop

                    dec r10
                    mov word [rdx], 0xffff;EOC.


                    ;RAX = Cluster of FAT.
                    mov rcx, r9;Cluster length in 512 byte sectors.
                    mov rdx, [rbp-8];Disk.
                    mov rsi, r12;Location
                    call Disk_io_funtions.write

                    mov rdx, r10


                ;Save free Block in chain.
                pop r10
                push rdx

                    mov rax, r9;Sectors per cluster.
                    mov rbx, 512
                    mul rbx
                    mov rcx, rax

                    ;Get cluster number in FAT and offset for Entry.
                    mov rax, r9;Size of Cluster in 512 Sectors.
                    mov rcx, 512/2;Number of entries.
                    mul rcx
                    mov rcx, rax
                    mov rax, r10
                    xor rdx, rdx
                    div rcx

                    mov rbx, rdx;Number of entries.

                    ;RAX = Cluster of FAT.
                    add ax, [rbp-520+14];Resurved sectors.
                    add rax, r8;Add partion offset.
                    mov rcx, r9;Cluster length in 512 byte sectors.
                    mov rdx, [rbp-8];Disk.
                    mov rdi, r12;Location
                    push rax
                    call Disk_io_funtions.read
                    pop rax

                    ;Get location of next cluster.
                    lea rdx, [rbx*2];Entry.
                    add rdx, rdi
                    pop r10
                    mov [rdx], r10w;EOC.

                    ;RAX = Cluster of FAT.
                    mov rcx, r9;Cluster length in 512 byte sectors.
                    mov rdx, [rbp-8];Disk.
                    mov rsi, r12;Location
                    call Disk_io_funtions.write

                dec r11
                jmp .write_file_now_find_clusters

            .write_file_now_find_clusters_done:

            pop r10

            mov r11, [rbp+8]

            .write_file_now_loop:
                mov rax, r9
                lea rbx, [r10-2]
                mul rbx
                add rax, r13

                mov rcx, r9;Cluster length in 512 byte sectors.
                mov rdx, [rbp-8];Disk.
                mov rsi, r11;Location
                call Disk_io_funtions.write

                ;next cluster.
                mov rax, 512
                mul r9
                add r11, rax


                ;Get cluster number in FAT and offset for Entry.
                mov rax, r9;Size of Cluster in 512 Sectors.
                mov rcx, 512/2;Number of entries.
                mul rcx
                mov rcx, rax
                mov rax, r10
                xor rdx, rdx
                div rcx

                mov rbx, rdx;Number of entries.

                ;RAX = Cluster of FAT.
                add ax, [rbp-520+14];Resurved sectors.
                add rax, r8;Add partion offset.
                mov rcx, r9;Cluster length in 512 byte sectors.
                mov rdx, [rbp-8];Disk.
                mov rdi, r12;Location
                call Disk_io_funtions.read


                ;Get location of next cluster.
                lea rdx, [rbx*2];Entry.
                add rdx, rdi
                xor rax, rax
                mov ax, [rdx]
                mov r10w, ax

                cmp ax, 0xffff;Check for EOC.
                jne .read_file_now_loop

            mov rsi, r12
            call kfree


        .write_done_no_err:
            ;Restore Regs
            mov rsp, rbp
            ;This is for the function.
            pop rsi
            pop rdi
            ;This is for the user.
            pop r10
            pop r9
            pop r8
            pop rbp
            pop rdx
            pop rcx
            pop rbx
            pop r11
            pop r12
            pop r13
            pop r14
            pop rsi
            pop rdi
            popfq

            ;RAX = 0 means passed.
            xor rax, rax
            ret

        .write_err_DNF:
            mov rax, 0x2
            jmp .write_err

        .write_err_disk:
            mov rax, 0x7
            jmp .write_err

        .write_err_not_FAT16:
            ;RAX = 1 means not FAT16.
            mov rax, 1

        .write_err:
            mov rsi, r12
            call kfree
            ;Add free.
            ;Restore Regs
            mov rsp, rbp
            ;This is for the function.
            pop rsi
            pop rdi
            ;This is for the user.
            pop r10
            pop r9
            pop r8
            pop rbp
            pop rdx
            pop rcx
            pop rbx
            pop r11
            pop r12
            pop r13
            pop r14
            pop rsi
            pop rdi
            popfq

            ret




    ;Read file from FAT16 Filesystem
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
        push rdi
        push rsi
        push r14
        push r13
        push r12
        push r11
        push rbx
        push rcx
        push rdx
        push rbp
        push r8
        push r9
        push r10
        ;This is for the function.
        push rdi
        push rsi
        mov rbp, rsp

        xor r12, r12;This is a flag for the program.


        ;Pick disk
        sub rsp, 8;Reserve
        mov [rbp-8], rdx

        ;Sector of disk
        mov rax, r8;Backup for Partion offset

        ;Read BPB+EBPB.
        sub rsp, 512;Reserve
        lea rdi, [rbp-520]
        mov rcx, 1
        mov rdx, [rbp-8]
        call Disk_io_funtions.read

        cmp rax, 'ND'
        je .read_err_disk


        ;Check if FAT 16


        ;RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec – 1)) / BPB_BytsPerSec
        xor rax, rax
        mov rcx, 32
        mov ax, [rbp-520+0x11];BPB_RootEntCnt
        cmp ax, 0;Error, can't have 0 Root entry count.
        je .read_err_not_FAT16
        mul rcx
        xor rcx, rcx
        mov cx, [rbp-520+0xb];BPB_BytsPerSec
        cmp cx, 0;Error, can't have 0 bytes per sector.
        je .read_err_not_FAT16
        dec cx
        add rax, rcx
        xor rdx, rdx
        inc cx
        div rcx
        ;Save RAX
        push rax

        ;If(BPB_FATSz16 != 0)
        ;FATSz = BPB_FATSz16;
        ;Else
        ;FATSz = BPB_FATSz32;
        xor rax, rax
        mov ax, [rbp-520+22];BPB_FATSz16
        cmp ax, 0
        jne .read_after_BPB_FATSz
        .read_else_BPB_FATSz32:
        mov eax, [rbp-520+36];BPB_FATSz32
        .read_after_BPB_FATSz:
        mov rbx, rax

        ;If(BPB_TotSec16 != 0)
        ;TotSec = BPB_TotSec16;
        ;Else
        ;TotSec = BPB_TotSec32;
        xor rax, rax
        mov ax, [rbp-520+19];BPB_TotSec16
        cmp ax, 0
        jne .read_after_BPB_TotSec
        .read_else_BPB_TotSec32:
        mov eax, [rbp-520+32];BPB_TotSec32
        .read_after_BPB_TotSec:
        mov rcx, rax

        ;DataSec = TotSec – (BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors);
        xor rax, rax
        mov al, [rbp-520+16];BPB_NumFATs
        mul rbx
        xor rbx, rbx
        mov bx, [rbp-520+14];BPB_ResvdSecCnt
        add rax, rbx
        ;Get saved value.
        pop rbx
        add rax, rbx
        sub rcx, rax
        mov rax, rcx

        ;CountofClusters = DataSec / BPB_SecPerClus;
        xor rcx, rcx
        mov cl, [rbp-520+13];BPB_SecPerClus
        xor rdx, rdx
        div rcx

        cmp rax, 4085;FAT12
        jl .read_err_not_FAT16
        cmp rax, 65525;FAT32
        jg .read_err_not_FAT16




        ;Find and Read file.

        xor rax, rax
        mov al, [rbp-520+16];Num of FATs.
        xor rcx, rcx
        mov cx, [rbp-520+22];FAT16 size.

        mul rcx

        xor rcx, rcx
        mov cx, [rbp-520+14];Resurved sectors.
        add rax, rcx



        ;Get sector offset in 512 sectores.
        xor rbx, rbx
        mov bx, [rbp-520+0x0B];Bytes per sector.
        mul rbx
        xor rdx, rdx
        mov rcx, 512
        div rcx

        add rax, r8;Partion offset.
        mov r11, rax;Root offset.



        push rax
        ;Get offset of data section.
        xor rax, rax
        mov ax, [rbp-520+0x11]
        mov rcx, 32;Bytes per entry.
        mul rcx
        ;Get sector offset in 512 sectores.
        xor rdx, rdx
        mov rcx, 512
        div rcx

        ;Save data section offset.
        mov r13, r11
        add r13, rax



        ;Number of sectors.
        xor rax, rax
        mov ax, [rbp-520+0x0B];Bytes per sector.
        mov rbx, 512
        xor rdx, rdx
        div rbx

        xor rcx, rcx
        mov cl, [rbp-520+0x0D];Sectors per cluster.
        mul rcx
        mov r9, rax;Backup length of Cluster, in 512 byte sectors.

        ;Get length of Root.
        xor eax, eax
        mov ax, [rbp-520+17];Entries.
        ;Number of bytes.
        mov rcx, 32
        mul rcx
        ;512 byte sectors.
        mov rcx, 512
        div rcx
        mov rcx, rax

        ;Get location for block to be loaded to and size.
        push rcx
        mov rax, rcx
        mov rcx, 512
        mul rcx

        ;Allacate.
        call kmalloc
        mov rdi, rsi
        mov r12, rsi;Save Block in memory so it can be freed.
        ;Restore regs.
        pop rcx
        pop rax

        mov rdx, [rbp-8]
        call Disk_io_funtions.read

        cmp rax, 'ND'
        je .read_err_disk


        mov r10, 1;Root cluter number, number to add to resurved clusters to find cluster.


        xor ecx, ecx
        mov cx, [rbp-520+17];Entries for Root.

        mov r14, 0x1;Search for Dir Flag.
        sub rsi, 32;Back 1.

        .read_find_next_dir:
            mov rdi, [rbp]
            push rdi
            mov al, [rbp+56]
            .read_find_dir_pram_loop:
                cmp [rdi], al
                je .read_find_dir_pram_loop_end
                cmp byte [rdi], 0
                je .read_at_file
                inc rdi
                jmp .read_find_dir_pram_loop

            .read_find_dir_pram_loop_end:
                inc rdi
                mov [rbp], rdi
                pop rdi


            .read_loop_find_dir:
                add rsi, 32;Next entry.

                cmp rcx, 0;Check if we need to get next Block. It may have been missed last time
                je .read_read_loop_find_dir_get_next_Block

                ;Test for ATTR_VOLUME_ID.
                lea rax, [rsi+32]
                lea rbx, [rcx-1]
                test byte [rsi+11], 0x8;If any others are set this is VFAT
                cmovnz rsi, rax
                cmovnz rcx, rbx

                cmp rcx, 0;Check if we need to get next Block. It may have been missed last time
                je .read_read_loop_find_dir_get_next_Block

                ;Number of blocks.
                dec rcx


                ;Check if we are loking for a file or a directory.
                cmp r14, 0x0
                je .read_loop_find_dir_file

                ;Test for ATTR_DIRECTORY.
                test byte [rsi+11], 0x10
                jz .read_loop_find_dir
                jmp .read_loop_find_dir_file_done

                .read_loop_find_dir_file:
                    ;Test for ATTR_DIRECTORY.
                    test byte [rsi+11], 0x10
                    jnz .read_loop_find_dir

                .read_loop_find_dir_file_done:


                ;Check for end.
                cmp byte [rsi], 0
                je .read_err_DNF



                ;Save regs before name check.
                push rcx
                push rsi
                push rdi
                mov rcx, 11;Max length.
                cld
                .read_loop_find_dir_check:
                    cmpsb;Compare bytes
                    jne .read_loop_find_dir_check_no_end

                    dec rcx;Max 11 bytes.
                    jnz .read_loop_find_dir_check

                    mov al, [rbp+56]
                    cmp [rdi], al;If next byte is dir_sep_char done, we have found it.
                    je .read_loop_find_dir_check_end
                    cmp byte [rdi+1], 0;If next byte is 0 done, we have found it.
                    je .read_file_now

                ;If did not pass...
                .read_loop_find_dir_check_no_end:
                    mov al, [rbp+56]
                    cmp [rdi-1], al;If it was the end of RDI maby it is ' ' padded in RSI.
                    jne .read_loop_find_dir_check_no_end_end

                    ;...Check for ' ' padding.
                    dec rsi
                    .read_loop_find_dir_check_no_end_loop:
                        cmp byte[rsi], ' ';Check for ' ' padding.
                        jne .read_loop_find_dir_check_no_end_end;It was not ' ' padded.

                        dec rcx;Check for 11 byte limit.
                        jnz .read_loop_find_dir_check_no_end_loop

                    ;It was ' ' padded.
                    jmp .read_loop_find_dir_check_end

                .read_loop_find_dir_check_no_end_end:
                ;Restore and continue search.
                pop rdi
                pop rsi
                pop rcx

                cmp rcx, 0;Check if we need to get next Block.
                je .read_read_loop_find_dir_get_next_Block

                jmp .read_loop_find_dir


            ;Get next Cluster.
            .read_read_loop_find_dir_get_next_Block:
                cmp r10, 1
                je .read_err_DNF
                ;Get cluster number in FAT and offset for Entry.
                mov rax, r9;Size of Cluster in 512 Sectors.
                mov rcx, 512/2;Number of entries.
                mul rcx
                mov rcx, rax
                mov rax, r10
                xor rdx, rdx
                div rcx

                mov rbx, rdx;Number of entries.

                ;RAX = Cluster of FAT.
                add ax, [rbp-520+14];Resurved sectors.
                add rax, r8;Add partion offset.
                mov rcx, r9;Cluster length in 512 byte sectors.
                mov rdx, [rbp-8];Disk.
                mov rdi, r12;Location
                call Disk_io_funtions.read

                ;Get location of next cluster.
                lea rdx, [rbx*2];Entry.
                add rdx, rdi
                xor rax, rax
                mov ax, [rdx]

                ;Check for EOC.
                cmp ax, 0xffff
                je .read_err_DNF

                dec rax;Back one.

                push rax;Save RAX.

                mov rax, r13;Location of data section.
                ;Get sector offset of Next Cluster.
                push rax
                mov rax, r9;Sectors per cluster.
                mov rbx, r10;Get Cluster number.
                dec rbx;Go back one to get true Cluster number.
                mul rbx;SPC * Cluster = Sector Location.
                mov rcx, rax
                pop rax

                add rax, rcx;Add
                mov rcx, r9;Cluster length in 512 byte sectors.
                mov rdx, [rbp-8];Disk.
                mov rdi, r12;Location
                call Disk_io_funtions.read

                ;Prepare for next search.
                mov rdi, rsi
                sub rsi, 32;Go back one.
                ;Number of entries per block.
                mov rax, r9;Get length of Block.
                mov rcx, 512
                mul rcx;Get byte value.
                mov rcx, 32;Bytes per entry.
                div rcx
                mov rcx, rax;Entries per Block.

                ;Save this clusters number.
                pop rax;Restore RAX.
                mov r10, rax

                jmp .read_loop_find_dir



            ;Found Dir move to next.
            .read_loop_find_dir_check_end:
                add rsp, 8;Pass the old RDI.
                pop rsi
                pop rcx
                push rdi


                xor rax, rax
                mov ax, [rsi+26];Cluster number.
                dec rax
                mov r10, rax

                mov rax, r13;Location of data section.
                ;Get sector offset of Next Cluster.
                push rax
                mov rax, r9;Sectors per cluster.
                mov rbx, r10;Get Cluster number.
                dec rbx;Go back one to get true Cluster number.
                mul rbx;SPC * Cluster = Sector Location.
                mov rcx, rax
                pop rax
                add rax, rcx;Add
                mov rcx, r9;Cluster length in 512 byte sectors.
                mov rdx, [rbp-8];Disk.
                mov rdi, r12;Location
                call Disk_io_funtions.read

                ;Prepare for next search.
                mov rsi, r12
                sub rsi, 32;Go back one.
                ;Number of entries per block.
                mov rax, r9;Get length of Block.
                mov rcx, 512
                mul rcx;Get byte value.
                mov rcx, 64;Bytes per entry*2 since we are one the first one it will be skiped.
                div rcx
                mov rcx, rax;32 bit blocks per entry.

                pop rdi

                jmp .read_find_next_dir





        .read_at_file:
            ;When VFAT is added, Here is where I should check if paramater is LFN and if it is go to LFN function.

            pop rdi

            mov [rsp+1], rax
            sub rsp, 16;Make space on Stack to store File name in 8.3 format, VFAT not supported.
            push rsi
            lea rdi, [rsp+8]
            mov rsi, [rbp]
            mov rcx, 11

            .read_at_file_make_8_3_name_loop:
                mov al, [rsi]
                cmp al, 0
                je .read_at_file_make_8_3_name_loop_pad
                cmp al, '.'
                je .read_at_file_make_8_3_name_loop_pad_dot
                mov [rdi], al
                inc rsi
                inc rdi
                dec rcx
                jnz .read_at_file_make_8_3_name_loop

            .read_at_file_make_8_3_name_loop_pad_dot:
                inc rsi
                cmp rcx, 3
                jle .read_at_file_make_8_3_name_loop_pad_dot_ext
                mov byte [rdi], ' '
                inc rdi
                dec rsi
                dec rcx
                jmp .read_at_file_make_8_3_name_loop_pad_dot

            .read_at_file_make_8_3_name_loop_pad_dot_ext:
                mov al, [rsi]
                cmp al, 0
                je .read_at_file_make_8_3_name_loop_pad
                cmp rcx, 0
                je .read_at_file_make_8_3_name_loop_pad
                mov byte [rdi], al
                inc rdi
                inc rsi
                dec rcx
                jmp .read_at_file_make_8_3_name_loop_pad_dot_ext


            .read_at_file_make_8_3_name_loop_pad:
                cmp rcx, 0
                je .read_at_file_make_8_3_name_loop_pad_done
                mov byte [rdi], ' '
                inc rdi
                dec rcx
                jmp .read_at_file_make_8_3_name_loop_pad

            .read_at_file_make_8_3_name_loop_pad_done:
                mov byte [rdi], 0

                pop rsi

                ;Number of entries per block.
                mov rax, 512
                mul r9;Get byte value.
                mov rcx, 32;Bytes per entry.
                div rcx
                mov rcx, rax;32 bit blocks per entry.


                ;Prepare for next search.
                lea rdi, [rsp]
                xor r14, r14;Search for File flag.


                jmp .read_loop_find_dir




        .read_file_now:
            pop rdi
            pop rsi
            pop rcx

            xor r10, r10
            xor r14, r14
            mov r10w, [rsi+26];Cluster number.
            mov r14d, [rsi+28];Size in bytes.

            .read_file_now_loop:
                mov rax, r9
                lea rbx, [r10-2]
                mul rbx
                add rax, r13

                mov rcx, r9;Cluster length in 512 byte sectors.
                mov rdx, [rbp-8];Disk.
                mov rdi, r12;Location
                call Disk_io_funtions.read

                mov rax, r9;Sectors per cluster.
                mov rbx, 512
                mul rbx
                mov rcx, rax

                mov rdi, [rbp+8];Where to load to.
                mov rsi, r12
                ;Move file to buffer.
                .read_file_now_loop_move_loop:
                    cmp r14, 0;Check if end of file.
                    je .read_file_now_loop_move_loop_end

                    movsb;Move

                    dec r14
                    ;Check if end of Cluster.
                    dec rcx
                    jnz .read_file_now_loop_move_loop

                .read_file_now_loop_move_loop_end:
                    mov [rbp+8], rdi


                ;Get cluster number in FAT and offset for Entry.
                mov rax, r9;Size of Cluster in 512 Sectors.
                mov rcx, 512/2;Number of entries.
                mul rcx
                mov rcx, rax
                mov rax, r10
                xor rdx, rdx
                div rcx

                mov rbx, rdx;Number of entries.

                ;RAX = Cluster of FAT.
                add ax, [rbp-520+14];Resurved sectors.
                add rax, r8;Add partion offset.
                mov rcx, r9;Cluster length in 512 byte sectors.
                mov rdx, [rbp-8];Disk.
                mov rdi, r12;Location
                call Disk_io_funtions.read


                ;Get location of next cluster.
                lea rdx, [rbx*2];Entry.
                add rdx, rdi
                xor rax, rax
                mov ax, [rdx]
                mov r10w, ax

                cmp ax, 0xffff;Check for EOC.
                jne .read_file_now_loop



            mov rsi, r12
            call kfree


        .read_done_no_err:
            ;Restore Regs
            mov rsp, rbp
            ;This is for the function.
            pop rsi
            pop rdi
            ;This is for the user.
            pop r10
            pop r9
            pop r8
            pop rbp
            pop rdx
            pop rcx
            pop rbx
            pop r11
            pop r12
            pop r13
            pop r14
            pop rsi
            pop rdi
            popfq

            ;RAX = 0 means passed.
            xor rax, rax
            ret

        .read_err_DNF:
            mov rax, 0x2
            jmp .read_err

        .read_err_disk:
            mov rax, 0x7
            jmp .read_err

        .read_err_not_FAT16:
            ;RAX = 1 means not FAT16.
            mov rax, 1

        .read_err:
            mov rsi, r12
            call kfree
            ;Add free.
            ;Restore Regs
            mov rsp, rbp
            ;This is for the function.
            pop rsi
            pop rdi
            ;This is for the user.
            pop r10
            pop r9
            pop r8
            pop rbp
            pop rdx
            pop rcx
            pop rbx
            pop r11
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
        lea rsi, [FAT16_DATA.FAT16_ID]
        ret



    ;Ping
    ;Prams:
    ;None.
    ;Returns
    ;RAX = 0x6(Ping recived.)
    .Ping:
        mov rax, 0x6
        ret




    ;Prams:
    ;R12 = Output form (i.e. 0x0 for printf, 0x1 for Memory location (RDI).).
    ;R8 = Sector start of partion.
    ;R11 = Partion Length.
    ;RDX = Disk num.
    ;RBX = Partion.
    ;RSI = Pointer to file path+name.
    ;RCX = Directory seperator char.
    ;RDI = Memory location to return list to each seporated by 0x0a.
    ;Returns:
    ;RAX = Return code.
    .list:
        ;Save Regs
        pushfq
        push rdi
        push rsi
        push r14
        push r13
        push r12
        push r11
        push rbx
        push rcx
        push rdx
        push rbp
        push r8
        push r9
        push r10
        ;This is for the function.
        push rdi
        push rsi
        mov rbp, rsp

        xor r12, r12;This is a flag for the program.


        ;Pick disk
        sub rsp, 8;Reserve
        mov [rbp-8], rdx

        ;Sector of disk
        mov rax, r8;Backup for Partion offset

        ;Read BPB+EBPB.
        sub rsp, 512;Reserve
        lea rdi, [rbp-520]
        mov rcx, 1
        mov rdx, [rbp-8]
        call Disk_io_funtions.read

        cmp rax, 'ND'
        je .list_err_disk


        ;Check if FAT 16


        ;RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec – 1)) / BPB_BytsPerSec
        xor rax, rax
        mov rcx, 32
        mov ax, [rbp-520+0x11];BPB_RootEntCnt
        mul rcx
        xor rcx, rcx
        mov cx, [rbp-520+0xb];BPB_BytsPerSec
        dec cx
        add rax, rcx
        xor rdx, rdx
        xor rcx, rcx
        mov cx, [rbp-520+0xb];BPB_BytsPerSec
        div rcx
        ;Save RAX
        push rax

        ;If(BPB_FATSz16 != 0)
        ;FATSz = BPB_FATSz16;
        ;Else
        ;FATSz = BPB_FATSz32;
        xor rax, rax
        mov ax, [rbp-520+22];BPB_FATSz16
        cmp ax, 0
        jne .list_after_BPB_FATSz
        .list_else_BPB_FATSz32:
        mov eax, [rbp-520+36];BPB_FATSz32
        .list_after_BPB_FATSz:
        mov rbx, rax

        ;If(BPB_TotSec16 != 0)
        ;TotSec = BPB_TotSec16;
        ;Else
        ;TotSec = BPB_TotSec32;
        xor rax, rax
        mov ax, [rbp-520+19];BPB_TotSec16
        cmp ax, 0
        jne .list_after_BPB_TotSec
        .list_else_BPB_TotSec32:
        mov eax, [rbp-520+32];BPB_TotSec32
        .list_after_BPB_TotSec:
        mov rcx, rax

        ;DataSec = TotSec – (BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors);
        xor rax, rax
        mov al, [rbp-520+16];BPB_NumFATs
        mul rbx
        xor rbx, rbx
        mov bx, [rbp-520+14];BPB_ResvdSecCnt
        add rax, rbx
        ;Get saved value.
        pop rbx
        add rax, rbx
        sub rcx, rax
        mov rax, rcx

        ;CountofClusters = DataSec / BPB_SecPerClus;
        xor rcx, rcx
        mov cl, [rbp-520+13];BPB_SecPerClus
        xor rdx, rdx
        div rcx

        cmp rax, 4085;FAT12
        jl .list_err_not_FAT16
        cmp rax, 65525;FAT32
        jg .list_err_not_FAT16




        ;Find and Read file.

        xor rax, rax
        mov al, [rbp-520+16];Num of FATs.
        xor rcx, rcx
        mov cx, [rbp-520+22];FAT16 size.

        mul rcx

        xor rcx, rcx
        mov cx, [rbp-520+14];Resurved sectors.
        add rax, rcx



        ;Get sector offset in 512 sectores.
        xor rbx, rbx
        mov bx, [rbp-520+0x0B];Bytes per sector.
        mul rbx
        xor rdx, rdx
        mov rcx, 512
        div rcx

        add rax, r8;Partion offset.
        mov r11, rax;Root offset.



        push rax
        ;Get offset of data section.
        xor rax, rax
        mov ax, [rbp-520+0x11]
        mov rcx, 32;Bytes per entry.
        mul rcx
        ;Get sector offset in 512 sectores.
        xor rdx, rdx
        mov rcx, 512
        div rcx

        ;Save data section offset.
        mov r13, r11
        add r13, rax



        ;Number of sectors.
        xor rax, rax
        mov ax, [rbp-520+0x0B];Bytes per sector.
        mov rbx, 512
        xor rdx, rdx
        div rbx

        xor rcx, rcx
        mov cl, [rbp-520+0x0D];Sectors per cluster.
        mul rcx
        mov r9, rax;Backup length of Cluster, in 512 byte sectors.

        ;Get length of Root.
        xor eax, eax
        mov ax, [rbp-520+17];Entries.
        ;Number of bytes.
        mov rcx, 32
        mul rcx
        ;512 byte sectors.
        mov rcx, 512
        div rcx
        mov rcx, rax

        ;Get location for block to be loaded to and size.
        push rcx
        mov rax, rcx
        mov rcx, 512
        mul rcx

        ;Allacate.
        call kmalloc
        mov rdi, rsi
        mov r12, rsi;Save Block in memory so it can be freed.
        ;Restore regs.
        pop rcx
        pop rax

        mov rdx, [rbp-8]
        call Disk_io_funtions.read

        cmp rax, 'ND'
        je .list_err_disk


        mov r10, 1;Root cluter number, number to add to resurved clusters to find cluster.


        xor ecx, ecx
        mov cx, [rbp-520+17];Entries for Root.

        mov r14, 0x1;Search for Dir Flag.
        sub rsi, 32;Back 1.

        .list_find_next_dir:
            mov rdi, [rbp]
            push rdi
            mov al, [rbp+56]
            .list_find_dir_pram_loop:
                cmp [rdi], al
                je .list_find_dir_pram_loop_end
                cmp byte [rdi], 0
                je .list_at_file
                inc rdi
                jmp .list_find_dir_pram_loop

            .list_find_dir_pram_loop_end:
                inc rdi
                mov [rbp], rdi
                pop rdi


            .list_loop_find_dir:
                add rsi, 32;Next entry.

                cmp rcx, 0;Check if we need to get next Block. It may have been missed last time
                je .list_list_loop_find_dir_get_next_Block

                ;Test for ATTR_VOLUME_ID.
                test byte [rsi+11], 0x8
                jnz .list_loop_find_dir

                cmp rcx, 0;Check if we need to get next Block. It may have been missed last time
                je .list_list_loop_find_dir_get_next_Block

                ;Number of blocks.
                dec rcx


                ;Check for end.
                cmp byte [rsi], 0
                je .list_err_DNF

                ;List or not
                cmp r14, 0
                jne .list_no_list_yet

                ;Test for HIDDEN_ATTR.
                test byte [rsi+11], 0x02
                jnz .list_loop_find_dir

                ;Check for empty.
                cmp byte [rsi], 0xe5
                je .list_loop_find_dir

                ;Print it.
                mov al, [rsi+11]
                call .puts_8_3_entry
                jmp .list_loop_find_dir

                .list_no_list_yet:


                ;Save regs before name check.
                push rcx
                push rsi
                push rdi
                mov rcx, 11;Max length.
                cld
                .list_loop_find_dir_check:
                    cmpsb;Compare bytes
                    jne .list_loop_find_dir_check_no_end

                    dec rcx;Max 11 bytes.
                    jnz .list_loop_find_dir_check

                    mov al, [rbp+56]
                    cmp [rdi], al;If next byte is dir_sep_char done, we have found it.
                    je .list_loop_find_dir_check_end
                    cmp byte [rdi+1], 0;If next byte is 0 done, we have Not found it.
                    je .list_err_DNF

                ;If did not pass...
                .list_loop_find_dir_check_no_end:
                    mov al, [rbp+56]
                    cmp [rdi-1], al;If it was the end of RDI maby it is ' ' padded in RSI.
                    jne .list_loop_find_dir_check_no_end_end

                    ;...Check for ' ' padding.
                    dec rsi
                    .list_loop_find_dir_check_no_end_loop:
                        cmp byte[rsi], ' ';Check for ' ' padding.
                        jne .list_loop_find_dir_check_no_end_end;It was not ' ' padded.

                        dec rcx;Check for 11 byte limit.
                        jnz .list_loop_find_dir_check_no_end_loop

                    ;It was ' ' padded.
                    jmp .list_loop_find_dir_check_end

                .list_loop_find_dir_check_no_end_end:
                ;Restore and continue search.
                pop rdi
                pop rsi
                pop rcx

                cmp rcx, 0;Check if we need to get next Block.
                je .list_list_loop_find_dir_get_next_Block

                jmp .list_loop_find_dir


            ;Get next Cluster.
            .list_list_loop_find_dir_get_next_Block:
                cmp r10, 1
                je .list_err_DNF
                ;Get cluster number in FAT and offset for Entry.
                mov rax, r9;Size of Cluster in 512 Sectors.
                mov rcx, 512/2;Number of entries.
                mul rcx
                mov rcx, rax
                mov rax, r10
                xor rdx, rdx
                div rcx

                mov rbx, rdx;Number of entries.

                ;RAX = Cluster of FAT.
                add ax, [rbp-520+14];Resurved sectors.
                add rax, r8;Add partion offset.
                mov rcx, r9;Cluster length in 512 byte sectors.
                mov rdx, [rbp-8];Disk.
                mov rdi, r12;Location
                call Disk_io_funtions.read

                ;Get location of next cluster.
                lea rdx, [rbx*2];Entry.
                add rdx, rdi
                xor rax, rax
                mov ax, [rdx]

                ;Check for EOC.
                cmp ax, 0xffff
                je .list_err_DNF

                dec rax;Back one.

                push rax;Save RAX.

                mov rax, r13;Location of data section.
                ;Get sector offset of Next Cluster.
                push rax
                mov rax, r9;Sectors per cluster.
                mov rbx, r10;Get Cluster number.
                dec rbx;Go back one to get true Cluster number.
                mul rbx;SPC * Cluster = Sector Location.
                mov rcx, rax
                pop rax

                add rax, rcx;Add
                mov rcx, r9;Cluster length in 512 byte sectors.
                mov rdx, [rbp-8];Disk.
                mov rdi, r12;Location
                call Disk_io_funtions.read

                ;Prepare for next search.
                mov rdi, rsi
                sub rsi, 32;Go back one.
                ;Number of entries per block.
                mov rax, r9;Get length of Block.
                mov rcx, 512
                mul rcx;Get byte value.
                mov rcx, 32;Bytes per entry.
                div rcx
                mov rcx, rax;Entries per Block.

                ;Save this clusters number.
                pop rax;Restore RAX.
                mov r10, rax

                jmp .list_loop_find_dir



            ;Found Dir move to next.
            .list_loop_find_dir_check_end:
                add rsp, 8;Pass the old RDI.
                pop rsi
                pop rcx
                push rdi


                xor rax, rax
                mov ax, [rsi+26];Cluster number.
                dec rax
                mov r10, rax

                mov rax, r13;Location of data section.
                ;Get sector offset of Next Cluster.
                push rax
                mov rax, r9;Sectors per cluster.
                mov rbx, r10;Get Cluster number.
                dec rbx;Go back one to get true Cluster number.
                mul rbx;SPC * Cluster = Sector Location.
                mov rcx, rax
                pop rax
                add rax, rcx;Add
                mov rcx, r9;Cluster length in 512 byte sectors.
                mov rdx, [rbp-8];Disk.
                mov rdi, r12;Location
                call Disk_io_funtions.read

                ;Prepare for next search.
                mov rsi, r12
                sub rsi, 32;Go back one.
                ;Number of entries per block.
                mov rax, r9;Get length of Block.
                mov rcx, 512
                mul rcx;Get byte value.
                mov rcx, 64;Bytes per entry*2 since we are one the first one it will be skiped.
                div rcx
                mov rcx, rax;32 bit blocks per entry.

                pop rdi

                jmp .list_find_next_dir





        .list_at_file:
            pop rdi
            xor r14, r14;List now Flag.
            jmp .list_loop_find_dir




        .list_done_no_err:
            mov rsi, r12
            call kfree
            ;Restore Regs
            mov rsp, rbp
            ;This is for the function.
            pop rsi
            pop rdi
            ;This is for the user.
            pop r10
            pop r9
            pop r8
            pop rbp
            pop rdx
            pop rcx
            pop rbx
            pop r11
            pop r12
            pop r13
            pop r14
            pop rsi
            pop rdi
            popfq

            ;RAX = 0 means passed.
            xor rax, rax
            ret

        .list_err_DNF:
            ;Check for End of list.
            cmp r14, 0
            je .list_done_no_err
            mov rax, 0x2
            jmp .list_err

        .list_err_disk:
            mov rax, 0x7
            jmp .list_err

        .list_err_not_FAT16:
            ;RAX = 1 means not FAT16.
            mov rax, 1

        .list_err:
            mov rsi, r12
            call kfree
            ;Add free.
            ;Restore Regs
            mov rsp, rbp
            ;This is for the function.
            pop rsi
            pop rdi
            ;This is for the user.
            pop r10
            pop r9
            pop r8
            pop rbp
            pop rdx
            pop rcx
            pop rbx
            pop r11
            pop r12
            pop r13
            pop r14
            pop rsi
            pop rdi
            popfq

            ret


    ;For list only.
    ;RSI points to unformatted string.
    .puts_8_3_entry:
        push rcx
        push rax
        push rdx
        push rdi
        push rsi
        mov rcx, 11
        sub rsp, 16
        mov rdi, rsp

        cmp byte [rsi], ' '
        je .puts_8_3_entry_dir;Somthing is wrong do NOT let it touch the stack.

        .puts_8_3_entry_loop_0:
            movsb
            dec rcx
            jnz .puts_8_3_entry_loop_0

        mov byte[rdi], 0

        test al, 0x10
        jnz .puts_8_3_entry_dir

        mov edx, [rdi-3]
        sub rdi, 4

        inc rdi
        .puts_8_3_entry_loop_1:
            dec rdi
            cmp byte[rdi], ' '
            je .puts_8_3_entry_loop_1

        inc rdi

        cmp edx, 0x00202020
        je .puts_8_3_entry_dir

        mov byte[rdi], '.'
        mov [rdi+1], edx

        .puts_8_3_entry_dir:
        mov byte[rdi+4], 0

        mov rsi, rsp
        call kernel.puts

        mov al, 0x0a
        call kernel.putc;NL (\n).

        add rsp, 16
        pop rsi
        pop rdi
        pop rdx
        pop rax
        pop rcx
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
        push rbx
        push rcx
        push rdx
        push rbp
        mov rbp, rsp

        ;Pick disk
        sub rsp, 8;Reserve
        mov [rbp-8], rdx

        ;start of disk
        mov rax, rbx

        ;Read BPB+EBPB
        sub rsp, 512
        lea rdi, [rbp-520]
        mov rcx, 1
        mov rdx, [rbp-8]
        call Disk_io_funtions.read

        cmp word[rdi+11], 512;Bytes per sector.
        je .check_right_continue1
        cmp word[rdi+11], 1024;Bytes per sector.
        je .check_right_continue1
        cmp word[rdi+11], 2048;Bytes per sector.
        je .check_right_continue1
        cmp word[rdi+11], 4096;Bytes per sector.
        je .check_right_continue1

        jmp .check_right_FS_err;Wrong FS.

        .check_right_continue1:

        cmp word[rdi+14], 0;Number of Resurved sectors, MUST not be 0.
        je .check_right_FS_err;Wrong FS.


        ;RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec – 1)) / BPB_BytsPerSec

        xor rax, rax
        mov rcx, 32
        mov ax, [rbp-520+0x11];BPB_RootEntCnt
        mul rcx

        xor rcx, rcx
        mov cx, [rbp-520+0xb];BPB_BytsPerSec
        dec cx

        add rax, rcx

        xor rdx, rdx
        xor rcx, rcx
        mov cx, [rbp-520+0xb];BPB_BytsPerSec
        cmp cx, 0
        je .check_right_FS_err
        div rcx


        ;Save RAX
        push rax

        ;If(BPB_FATSz16 != 0)
        ;FATSz = BPB_FATSz16;
        ;Else
        ;FATSz = BPB_FATSz32;
        xor rax, rax
        mov ax, [rbp-520+22]
        cmp ax, 0
        jne .check_right_FS_after_BPB_FATSz

        .check_right_FS_else_BPB_FATSz32:
        mov eax, [rbp-520+36]

        .check_right_FS_after_BPB_FATSz:
        mov rbx, rax


        ;If(BPB_TotSec16 != 0)
        ;TotSec = BPB_TotSec16;
        ;Else
        ;TotSec = BPB_TotSec32;
        xor rax, rax
        mov ax, [rbp-520+19]
        cmp ax, 0
        jne .check_right_FS_after_BPB_TotSec

        .check_right_FS_else_BPB_TotSec32:
        mov eax, [rbp-520+32]

        .check_right_FS_after_BPB_TotSec:
        mov rcx, rax


        ;DataSec = TotSec – (BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors);
        xor rax, rax
        mov al, [rbp-520+16]
        mul rbx

        xor rbx, rbx
        mov bx, [rbp-520+14]
        add rax, rbx

        pop rbx
        add rax, rbx

        sub rcx, rax
        mov rax, rcx

        ;CountofClusters = DataSec / BPB_SecPerClus;
        xor rcx, rcx
        mov cl, [rbp-520+13]
        cmp cl, 0
        je .check_right_FS_err

        xor rdx, rdx
        div rcx

        cmp rax, 4085;FAT12
        jl .check_right_FS_err

        cmp rax, 65525;FAT32
        jg .check_right_FS_err



        .check_right_FS_right:
            mov rsp, rbp
            pop rbp
            pop rdx
            pop rcx
            pop rbx
            xor rax, rax
            ret

        .check_right_FS_err:
            mov rsp, rbp
            pop rbp
            pop rdx
            pop rcx
            pop rbx
            mov rax, 1
            ret




FAT16_DATA:
    .MBR_FS_ID: dq 0x06
    .FAT16_ID: db 'FAT16   ', 0



