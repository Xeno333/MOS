;MVFS.inc
;VFS manager.
;Handels all file reqests.
;State = Under development.


;Only printf output suported.


MVFS_const:
    .Directory_seporation_char = '/'
    .Disk_Partion_sep_char = '/'


;If a MVFS driver is removed from the Table set MBR FS ID to 0xffffffffffffffff to indacate it is empty but not the End.


;All values are set to 0xffffffffffffffff when removed.
struc MVFS_FS_driver_table_entry arg0, arg1 {
    dq arg0;Pointer to MVFS driver modual memory offset.
    dq arg1;Pointer to ID string, has to be presant.
}

;Set Ponter to 0xffffffffffffffff when removed, it also can NOT be 0 or it will be treated as end of list and free.
struc MVFS_FS_Mount_table_entry arg0, arg1, arg2, arg3 {
    .Mount_point: db arg0;32 byte ASCII string giving mount point padded with '\0's, must Contain at least 1 ASCII char or it will be marked empty Last byte MUST be 0 giving you only 31 bytes.
    .Ponter: dq arg1;Pointer to MVFS driver modual used to mount, memory offset.
    .Flags dq arg2;Flags.
    .Partion_start dq arg3;First LBA sector of partion.
    .Disk dq 0;Disk number.
    .Size dq 0;Size in sectors.
}


MVFS_FS_driver_table_entry_len = 16
MVFS_FS_driver_table_entry_Driver_location = 0;Bytes
MVFS_FS_driver_table_entry_ID_str_offset = 8;Bytes.


MVFS_FS_Mount_table_entry_len = 72
MVFS_FS_Mount_table_entry_Mount_Point = 0
MVFS_FS_Mount_table_entry_Pointer = 32
MVFS_FS_Mount_table_entry_Flags = 32+8
MVFS_FS_Mount_table_entry_Start = 32+16
MVFS_FS_Mount_table_entry_Disk = 32+24
MVFS_FS_Mount_table_entry_Size = 32+32


;MVFS entry point.
;Parms
;RAX = Function.
;Others
MVFS_call:
    cmp rax, 0
    je MVFS.mount

    cmp rax, 1
    je MVFS.read

    cmp rax, 2
    je MVFS.write

    cmp rax, 3
    je MVFS.Install_MVFSDRV

    cmp rax, 4
    je MVFS.unmount

    cmp rax, 5
    je MVFS.list

    cmp rax, 6
    je MVFS.list_installed_FSs

    cmp rax, 7
    je MVFS.list_mounted_FSs



    mov rax, 0xffffffffffffffff;No such Function.
    ret




MVFS:

    ;In: al = head, bl = sector (6bit), cx = cylinder(10bit).
    .CHS2LBA:
        ret

    ;r9 = partion, r8 = disk.
    .Get_Partion_info:
        push rbp
        push rdi
        push r11
        mov rbp, rsp

        sub rsp, 512;Reserve
        lea rdi, [rbp-512]
        mov rax, 1
        mov rcx, 1
        mov rdx, r8
        call Disk_io_funtions.read

        mov rax, 'EFI PART'
        cmp [rdi], rax;GPT
        je .Get_Partion_info_GPT

        mov rax, 0
        mov rcx, 1
        mov rdx, r8
        call Disk_io_funtions.read

        cmp r9, 3
        jg .Get_Partion_info_EBR

        mov eax, 16
        mul r9
        add rdi, rax

        xor eax, eax
        mov al, [rdi+0x1BE+4];MBR FS type ID.
        cmp al, 0
        je .Get_Partion_info_error
        mov ebx, [rdi+0x1BE+8];First LBA sector.
        mov ecx, [rdi+0x1BE+0xC];Length in sectors.

        ;Partion type GUID R9:R10 = 0 for MBR.
        xor r9, r9
        xor r10, r10

        xor edx, edx
        jmp .Get_Partion_info_end

        .Get_Partion_info_EBR:
            mov cl, 3
            .Get_Partion_info_EBR_Find_EBR_loop:
                cmp byte[rdi+0x1BE+4], 0x05
                je .Get_Partion_info_EBR_found_first
                cmp byte[rdi+0x1BE+4], 0x0f
                je .Get_Partion_info_EBR_found_first
                add rdi, 16
                dec cl
                jz .Get_Partion_info_error
                cmp byte [rdi+0x1BE+4], 0
                jz .Get_Partion_info_error
                jmp .Get_Partion_info_EBR_Find_EBR_loop

            .Get_Partion_info_EBR_found_first:
                ;Load first EBR
                mov eax, [rdi+0x1BE+8]
                lea rdi, [rbp-512]
                mov rcx, 1
                mov rdx, r8
                call Disk_io_funtions.read

                mov eax, 4
                .Get_Partion_info_EBR_found_first_find_loop:
                    cmp rax, r9
                    je .Get_Partion_info_EBR_found_first_find_loop_done

                    cmp byte[rdi+16+0x1BE+4], 0
                    je .Get_Partion_info_error

                    push rax
                    ;Load next EBR.
                    mov eax, [rdi+16+0x1BE+16+8];Second entry/Next EBR.
                    lea rdi, [rbp-512]
                    mov rcx, 1
                    mov rdx, r8
                    call Disk_io_funtions.read
                    pop rax

                    inc rax
                    jmp .Get_Partion_info_EBR_found_first_find_loop


                .Get_Partion_info_EBR_found_first_find_loop_done:
                    xor eax, eax
                    mov al, [rdi+0x1BE+4];MBR FS type ID.
                    cmp al, 0
                    je .Get_Partion_info_error
                    mov ebx, [rdi+0x1BE+8];First LBA sector.
                    mov ecx, [rdi+0x1BE+0xC];Length in sectors.

                    ;Partion type GUID R9:R10 = 0 for MBR.
                    xor r9, r9
                    xor r10, r10

                    xor edx, edx
                    jmp .Get_Partion_info_end



        .Get_Partion_info_GPT:
            mov eax, [rdi+0x54];Size of entry.
            mul r9;Get byte offset.

            mov rdx, r8
            call Disk_io_funtions.get_sector_size

            xor rdx, rdx
            div rcx
            mov rcx, rax

            mov r11, rdx;Remander.

            mov rax, [rdi+0x48];Start of Partion Table.
            add rax, rcx

            lea rdi, [rbp-512]
            mov rcx, 1
            mov rdx, r8
            call Disk_io_funtions.read

            ;r9:r10 GUID Type
            mov r9, [rdi+r11]
            mov r10, [rdi+r11+8]





            ;Make GUID understandible.
            mov rax, r9
            ror rax, 32
            mov rbx, rax
            ror rbx, 16
            xchg ax, bx
            rol rbx, 16
            ror rax, 16
            xchg ax, bx
            rol rax, 16
            mov r9, rax

            mov rax, r10
            mov rbx, rax
            ror rbx, 8
            rol rax, 8
            mov al, bl
            rol rax, 8
            ror rbx, 8
            mov al, bl
            rol rax, 8
            ror rbx, 8
            mov al, bl
            ror rax, 32+16
            ror rbx, 16
            mov al, bl
            rol rax, 16
            ror ebx, 8
            ror bx, 8
            mov ax, bx
            mov r10, rax


            ;Check to make sure it is not empty.
            cmp r9, 0
            je .Get_Partion_info_error_GPT_Empty

            .Get_Partion_info_error_GPT_not_Empty:
                mov rbx, [rdi+r11+0x20];First LBA.
                mov rcx, [rdi+r11+0x28];End LBA.
                sub rcx, rbx;Size.

                xor eax, eax

                xor edx, edx
                jmp .Get_Partion_info_end

            .Get_Partion_info_error_GPT_Empty:
                cmp r10, 0
                je .Get_Partion_info_error
                jmp .Get_Partion_info_error_GPT_not_Empty


        .Get_Partion_info_error:
            mov edx, 1

        .Get_Partion_info_end:

            mov rsp, rbp
            pop r11
            pop rdi
            pop rbp
            ret





    ;Prams:
    ;RSI = Memory Offset of Driver.
    .Install_MVFSDRV:
        pushfq
        push rbx
        push rcx
        push rdx
        push rbp
        mov rbp, rsp

        ;Ping modual.
        mov rax, 4
        call rsi
        cmp rax, 6;Check for Ping recived respons.
        jne .Install_MVFSDRV_done_Ping_err_Failed

        ;Get FS type ID.
        mov rax, 3
        push rsi
        push r14
        mov r14, rsi
        call rsi
        pop r14

        ;Find empty entry.
        mov rcx, MVFS_FS_driver_table
        .Install_MVFSDRV_loop:
            cmp qword[rcx], 0;Empty.
            je .Install_MVFSDRV_Add
            cmp qword[rcx], 0xffffffffffffffff;Removed.
            je .Install_MVFSDRV_Add
            add rcx, MVFS_FS_driver_table_entry_len
            cmp rcx, MVFS_FS_driver_table_end
            jl .Install_MVFSDRV_loop

        ;Err out of room.
        mov rax, 9
        jmp .Install_MVFSDRV_done


        ;Add entry.
        .Install_MVFSDRV_Add:
            mov [rcx+MVFS_FS_driver_table_entry_ID_str_offset], rsi;Location of FS ID string.
            pop rsi;Location of Driver.
            mov [rcx+MVFS_FS_driver_table_entry_Driver_location], rsi;Driver location in memory.
            xor rax, rax
            jmp .Install_MVFSDRV_done


        .Install_MVFSDRV_done_Ping_err_Failed:
            mov rax, 8
            jmp .Install_MVFSDRV_done

        .Install_MVFSDRV_done:
            mov rsp, rbp
            pop rbp
            pop rdx
            pop rcx
            pop rbx
            popfq
            ret


    .list_installed_FSs:
        pushfq
        push rsi
        push rax
        push rbx

        mov rax, MVFS_FS_driver_table

        .list_installed_FSs_loop:
            mov rsi, [rax+MVFS_FS_driver_table_entry_ID_str_offset]
            add rax, MVFS_FS_driver_table_entry_len
            call kernel.puts
            xchg al, bl
            mov al, 0x0a
            call kernel.putc
            xchg al, bl
            cmp qword [rax], 0xffffffffffffffff;Since 0xffffffffffffffff not a true MBR FS ID we must be on a removed one.
            je .list_installed_FSs_loop
            cmp byte [rax], 0;Since 0 not a true UBMPT FS ID End of list.
            jne .list_installed_FSs_loop

        .list_installed_FSs_loop_end:

        pop rbx
        pop rax
        pop rsi
        popfq
        xor rax, rax
        ret




    .list_mounted_FSs:
        pushfq
        push rsi
        push rax
        push rbx

        mov rax, MVFS_FS_Mount_table

        .list_mounted_FSs_loop:
            cmp rax, MVFS_FS_Mount_table_end
            je .list_mounted_FSs_loop_end

            cmp qword [rax+MVFS_FS_Mount_table_entry_Pointer], 0xffffffffffffffff;Since 0xffffffffffffffff not a true MBR FS ID we must be on a removed one.
            je .list_mounted_FSs_loop

            push rax

            push r14
            mov r14, [rax+MVFS_FS_Mount_table_entry_Pointer]
            push rax
            mov eax, 3
            call r14
            pop rax
            pop r14
            push rsi

            mov rbx, [rax+MVFS_FS_Mount_table_entry_Size]
            push rbx

            lea rax, [rax+MVFS_FS_Mount_table_entry_Mount_Point]
            push rax

            mov rsi, MVFS_DATA.list_mounted_FSs_string
            mov rax, rsp
            call kernel.printf

            add rsp, 8*3
            pop rax

            add rax, MVFS_FS_Mount_table_entry_len
            cmp qword [rax+MVFS_FS_Mount_table_entry_Pointer], 0
            jne .list_mounted_FSs_loop

        .list_mounted_FSs_loop_end:

        pop rbx
        pop rax
        pop rsi
        popfq
        xor rax, rax
        ret


    ;Mount
    ;Prams:
    ;RSI = PATH
    ;local vars
    ;rbp-8 pathlen len = 8
    ;rbp-16 path len = ?
    .mount:
        pushfq
        push rbx
        push rcx
        push rdx
        push rsi
        push rdi
        push rbp
        push r8
        push r9
        push r10
        push r14
        push r11
        push r12
        mov rbp, rsp

        mov r12, rdi


        ;Find Disk.
        lea rbx, [rbp-8]
        xor rcx, rcx
        .mount_pars_prams_loop_0:
            dec rsp;Reserve
            mov al, [rsi]
            mov [rbx], al
            inc rsi
            dec rbx
            inc rcx
            cmp byte [rsi], MVFS_const.Disk_Partion_sep_char
            jne .mount_pars_prams_loop_0


        ;Save rsi.
        mov r10, rsi

        ;Get value.
        xor r8, r8;Disk num stored here.
        mov rbx, 1;Place in number.
        lea rdi, [rbp-7];start
        sub rdi, rcx
        lea rcx, [rbp-7];end
        .mount_pars_prams_loop_1:
            xor rax, rax
            mov al, [rdi]
            sub al, 0x30
            mul rbx
            add r8, rax

            ;Move to nex place.
            mov rax, rbx
            mov rbx, 10
            mul rbx
            mov rbx, rax

            inc rdi
            cmp rdi, rcx
            jne .mount_pars_prams_loop_1

        mov rsp, rbp;Free local vars.

        ;End of find Disk.




        mov rsi, r10;Restore RSI.
        inc rsi;Pass '/' char.

        ;Find partion.
        lea rbx, [rbp-8]
        xor rcx, rcx
        .mount_pars_prams_loop_2:
            dec rsp;Reserve
            mov al, [rsi]
            mov [rbx], al
            inc rsi
            dec rbx
            inc rcx
            cmp byte [rsi], 0
            jne .mount_pars_prams_loop_2

        ;Get value.
        xor r9, r9;Partion num stored here.
        mov rbx, 1;Place in number.
        lea rdi, [rbp-7];start
        sub rdi, rcx
        lea rcx, [rbp-7];end
        .mount_pars_prams_loop_3:
            xor rax, rax
            mov al, [rdi]
            sub al, 0x30
            mul rbx
            add r9, rax

            ;Move to nex place.
            mov rax, rbx
            mov rbx, 10
            mul rbx
            mov rbx, rax

            inc rdi
            cmp rdi, rcx
            jne .mount_pars_prams_loop_3

        mov rsp, rbp;Free local vars.


        ;R8 = Disk number.
        ;R9 = Partion number.

        call .Get_Partion_info
        cmp rdx, 0
        jne .mount_done_err_could_not_mount

        ;rax = MBR FS type ID.
        ;RBX = Start of Partion, First LBA.
        ;RCX = Length or partion Sectors.
        ;r9:r10 = Partion type GUID 0 if not GPT.
        mov rdx, r8;Move Disk number to RDX.
        mov r8, rax;Move MBR FS type ID to R8.

        mov rsi, MVFS_FS_driver_table
        mov rdi, MVFS_FS_driver_table_end
        .mount_find_driver_loop:
            cmp rsi, rdi
            je .mount_done_err_no_driver
            cmp qword[rsi], 0
            je .mount_done_err_no_driver

            mov rax, 7;Check FS type agent driver.

            ;Pass args through stack also.
            push rdx
            push rbx
            push rcx
            push r8
            push r9
            push r10

            mov r14, [rsi];Driver.
            call r14

            add rsp, 8*6

            cmp rax, 0
            je .mount_found_driver

            add rsi, MVFS_FS_driver_table_entry_len
            jmp .mount_find_driver_loop

        .mount_found_driver:
            ;Mount.
            mov r11, rcx
            mov r8, rbx
            ;RDX already set.

            ;Pass args through stack also.
            push rdx
            push r8
            push r11

            mov rax, 0
            call r14
            add rsp, 8*3
            cmp rax, 7
            je .mount_done_err_disk
            cmp rax, 0
            jne .mount_done_err_could_not_mount

            ;Make entry,

            mov rsi, MVFS_FS_Mount_table
            mov rdi, MVFS_FS_Mount_table_end
            .mount_find_Mount_entry_loop:
                cmp rsi, rdi
                je .mount_done_err_no_driver

                cmp qword[rsi+32], 0;Free.
                je .mount_found_entry
                cmp qword[rsi+32], 0xffffffffffffffff;Free.
                je .mount_found_entry

                add rsi, MVFS_FS_Mount_table_entry_len
                jmp .mount_find_Mount_entry_loop

        .mount_found_entry:
            mov [rsi+MVFS_FS_Mount_table_entry_Pointer], r14
            mov qword[rsi+MVFS_FS_Mount_table_entry_Flags], 0;Flags.
            mov [rsi+MVFS_FS_Mount_table_entry_Start], rbx
            mov [rsi+MVFS_FS_Mount_table_entry_Disk], rdx
            mov [rsi+MVFS_FS_Mount_table_entry_Size], rcx

            mov cl, 31
            .mount_found_entry_loop_mount_point:
                mov al, [r12]
                mov [rsi], al
                cmp al, 0
                je .mount_found_entry_loop_mount_point_done
                inc rsi
                inc r12
                dec cl
                jnz .mount_found_entry_loop_mount_point

            .mount_found_entry_loop_mount_point_done:
                mov byte[rsi], 0
                jmp .mount_done_no_err



        .mount_done_err_could_not_mount:
            mov eax, 1
            jmp .mount_done

        .mount_done_err_no_driver:
            mov eax, 7
            jmp .mount_done

        .mount_done_err_disk:
            mov eax, 0xA
            jmp .mount_done

        .mount_done_no_err:
            xor eax, eax

        .mount_done:
            mov rsp, rbp
            pop r12
            pop r11
            pop r14
            pop r10
            pop r9
            pop r8
            pop rbp
            pop rdi
            pop rsi
            pop rdx
            pop rcx
            pop rbx
            popfq
            ret



    ;Unmount.
    ;Prams:
    ;RSI = Mount point.
    .unmount:
        pushfq
        push rbx
        push rcx
        push rdx
        push rsi
        push rdi
        push rbp
        push r8
        push r9
        push r10
        push r11
        push r14
        mov rbp, rsp

        mov r14, rsi

        mov rsi, MVFS_FS_Mount_table
        mov rdi, MVFS_FS_Mount_table_end
        .unmount_find_mount_point_loop:
            xchg r14, rdi
            call string.strcmp
            xchg r14, rdi
            cmp rax, 0
            je .unmount_find_mount_point_loop_done
            add rsi, MVFS_FS_Mount_table_entry_len
            cmp rsi, rdi
            jne .unmount_find_mount_point_loop
            jmp .unmount_done_err_FS_not_mounted

        .unmount_find_mount_point_loop_done:
            mov r14, [rsi+MVFS_FS_Mount_table_entry_Pointer]
            mov r8, [rsi+MVFS_FS_Mount_table_entry_Start]
            mov rdx, [rsi+MVFS_FS_Mount_table_entry_Disk]
            mov r11, [rsi+MVFS_FS_Mount_table_entry_Size]

            ;Pass args through stack also.
            push rdx
            push r8
            push r11

            mov rax, 5
            call r14

            add rsp, 8*3

            cmp rax, 0x07
            je .unmount_ND_err

            ;Else.
            mov rax, 0;Could Not Mount.
            jne .unmount_done


            mov rcx, MVFS_FS_Mount_table_entry_len
            xor eax, eax
            repnz stosb

            jmp .unmount_done_no_err


        .unmount_done_err_FS_not_mounted:
            mov rax, 1
            jmp .unmount_done

        .unmount_ND_err:
            mov rax, 0x0a;No disk.
            jmp .unmount_done

        .unmount_done_no_err:
            xor rax, rax

        .unmount_done:
            mov rsp, rbp
            pop r14
            pop r11
            pop r10
            pop r9
            pop r8
            pop rbp
            pop rdi
            pop rsi
            pop rdx
            pop rcx
            pop rbx
            popfq
            ret



    ;Read.
    ;Prams:
    ;RSI = PATH
    ;loval vars
    ;rbp+(5*8) = RDI
    ;rbp-8 pathlen len = 8
    ;rbp-16 path len = ?
    .read:
        pushfq
        push rbx
        push rcx
        push rdx
        push rsi
        push rbp
        push r8
        push r9
        push r10
        push r11
        push rdi
        mov rbp, rsp

        sub rsp, 32
        lea rdi, [rbp-32]
        .read_loop:
            mov al, [rsi]
            mov [rdi], al
            inc rdi
            inc rsi
            cmp al, MVFS_const.Directory_seporation_char
            jne .read_loop

        mov r10, rsi


        mov rsi, MVFS_FS_Mount_table
        mov rdi, MVFS_FS_Mount_table_end
        .read_find_mount_point_loop:
            push rdi
            lea rdi, [rbp-32]
            call string.strcmp
            pop rdi
            cmp rax, 0
            je .read_find_mount_point_loop_done
            add rsi, MVFS_FS_Mount_table_entry_len
            cmp rsi, rdi
            jne .read_find_mount_point_loop
            jmp .read_done_err_FS_not_mounted

        .read_find_mount_point_loop_done:
            mov r14, [rsi+MVFS_FS_Mount_table_entry_Pointer]
            mov r8, [rsi+MVFS_FS_Mount_table_entry_Start]
            mov rdx, [rsi+MVFS_FS_Mount_table_entry_Disk]
            mov r11, [rsi+MVFS_FS_Mount_table_entry_Size]
            mov rcx, MVFS_const.Directory_seporation_char
            mov rdi, [rbp]
            mov rsi, r10

            ;Pass args through stack also.
            push rdx
            push r8
            push r11
            push rcx
            push rsi
            push rdi

            mov rax, 1
            call r14

            add rsp, 8*6

            cmp rax, 0
            je .read_done_no_err

            cmp rax, 0x07
            je .read_ND_err

            cmp rax, 0x02
            je .read_DNF

            ;Else.
            mov rax, 1;Could Not Mount.
            jmp .read_done


        .read_done_err_FS_not_mounted:
            mov rax, 1
            jmp .read_done

        .read_ND_err:
            mov rax, 0x0a;No disk.
            jmp .read_done

        .read_DNF:
            mov rax, 0x02
            jmp .read_done

        .read_done_no_err:
            xor rax, rax

        .read_done:
            mov rsp, rbp
            pop rdi
            pop r11
            pop r10
            pop r9
            pop r8
            pop rbp
            pop rsi
            pop rdx
            pop rcx
            pop rbx
            popfq
            ret

    ;Write
    ;Prams:
    ;RSI = PATH
    ;loval vars
    ;rbp+(5*8) = RDI
    ;rbp-8 pathlen len = 8
    ;rbp-16 path len = ?
    ;r12 used to back up size(RCX).
    .write:
        pushfq
        push r12
        push rbx
        push rcx
        push rdx
        push rsi
        push rbp
        push r8
        push r9
        push r10
        push r11
        push rdi
        mov rbp, rsp

        mov r12, rcx

        sub rsp, 32
        lea rdi, [rbp-32]
        .write_loop:
            mov al, [rsi]
            mov [rdi], al
            inc rdi
            inc rsi
            cmp al, MVFS_const.Directory_seporation_char
            jne .write_loop

        mov r10, rsi


        mov rsi, MVFS_FS_Mount_table
        mov rdi, MVFS_FS_Mount_table_end
        .write_find_mount_point_loop:
            push rdi
            lea rdi, [rbp-32]
            call string.strcmp
            pop rdi
            cmp rax, 0
            je .write_find_mount_point_loop_done
            add rsi, MVFS_FS_Mount_table_entry_len
            cmp rsi, rdi
            jne .write_find_mount_point_loop
            jmp .write_done_err_FS_not_mounted

        .write_find_mount_point_loop_done:
            mov r14, [rsi+MVFS_FS_Mount_table_entry_Pointer]
            mov r8, [rsi+MVFS_FS_Mount_table_entry_Start]
            mov rdx, [rsi+MVFS_FS_Mount_table_entry_Disk]
            mov r11, [rsi+MVFS_FS_Mount_table_entry_Size]
            mov rcx, MVFS_const.Directory_seporation_char
            mov rdi, [rbp]
            mov rsi, r10
            mov rbx, r12;Size.

            ;Pass args through stack also.
            push rdx
            push r8
            push r11
            push rcx
            push rsi
            push rdi
            push rbx

            mov rax, 2
            call r14

            add rsp, 8*6

            cmp rax, 0
            je .write_done_no_err

            cmp rax, 0x07
            je .write_ND_err

            cmp rax, 0x02
            je .write_DNF

            ;Else.
            mov rax, 1;Could Not Mount.
            jmp .write_done


        .write_done_err_FS_not_mounted:
            mov rax, 1
            jmp .write_done

        .write_ND_err:
            mov rax, 0x0a;No disk.
            jmp .write_done

        .write_DNF:
            mov rax, 0x02
            jmp .write_done

        .write_done_no_err:
            xor rax, rax

        .write_done:
            mov rsp, rbp
            pop rdi
            pop r11
            pop r10
            pop r9
            pop r8
            pop rbp
            pop rsi
            pop rdx
            pop rcx
            pop rbx
            pop r12
            popfq
            ret






    ;list.
    ;Prams:
    ;R12 = Output form (i.e. 0x0 for printf, 0x1 for Memory location (RDI).).
    ;RDI = Memory location to return list to each seporated by 0x0a.
    ;RSI = Path.
    ;Returns:
    ;RAX = Return code.
    ;loval vars
    ;rbp+(5*8) = RDI
    ;rbp-8 path = 8
    ;rbp-16 path len = ?
    .list:
        pushfq
        push rbx
        push rcx
        push rdx
        push rsi
        push rbp
        push r8
        push r9
        push r10
        push r11
        push r12
        push rdi
        mov rbp, rsp

        sub rsp, 32
        lea rdi, [rbp-32]
        .list_loop:
            mov al, [rsi]
            mov [rdi], al
            inc rdi
            inc rsi
            cmp al, MVFS_const.Directory_seporation_char
            jne .list_loop

        mov r10, rsi


        mov rsi, MVFS_FS_Mount_table
        mov rdi, MVFS_FS_Mount_table_end
        .list_find_mount_point_loop:
            push rdi
            lea rdi, [rbp-32]
            call string.strcmp
            pop rdi
            cmp rax, 0
            je .list_find_mount_point_loop_done
            add rsi, MVFS_FS_Mount_table_entry_len
            cmp rsi, rdi
            jne .list_find_mount_point_loop
            jmp .list_done_err_FS_not_mounted

        .list_find_mount_point_loop_done:
            mov r14, [rsi+MVFS_FS_Mount_table_entry_Pointer]
            mov r8, [rsi+MVFS_FS_Mount_table_entry_Start]
            mov rdx, [rsi+MVFS_FS_Mount_table_entry_Disk]
            mov r11, [rsi+MVFS_FS_Mount_table_entry_Size]
            mov rcx, MVFS_const.Directory_seporation_char
            mov rdi, [rbp]
            mov r12, [rbp+8]
            mov rsi, r10


            ;Pass args through stack also.
            push rdx
            push r8
            push r11
            push rcx
            push rsi
            push rdi
            push r12

            mov rax, 6
            call r14

            add rsp, 8*7

            cmp rax, 0
            je .list_done_no_err

            cmp rax, 0x02
            je .list_DNF

            ;Disk err.
            cmp rax, 0x07
            je .list_ND_err

            ;Else.
            mov rax, 1;Could Not Mount.
            jmp .list_done


        .list_done_err_FS_not_mounted:
            mov rax, 1
            jmp .list_done

        .list_ND_err:
            mov rax, 0x0a;No disk.
            jmp .list_done

        .list_DNF:
            mov rax, 0x02
            jmp .list_done

        .list_done_no_err:
            xor rax, rax

        .list_done:
            mov rsp, rbp
            pop rdi
            pop r12
            pop r11
            pop r10
            pop r9
            pop r8
            pop rbp
            pop rsi
            pop rdx
            pop rcx
            pop rbx
            popfq
            ret



MVFS_DATA:
    .list_mounted_FSs_string cstring 'Mount point: %s     Size: %d B     FS type: %s \n'


MVFS_FS_driver_table:
    times 1024 dq 0, 0;Up to 1024 FS moduals allowed at one time.
MVFS_FS_driver_table_end:


MVFS_FS_Mount_table:
    times 1024 dq 0, 0, 0, 0, 0, 0, 0, 0, 0;Up to 1024 FSs allowed to be mounted at one time.
MVFS_FS_Mount_table_end:








