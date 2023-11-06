;ATA.inc
;Part of Disk.inc
;
;Handels ATA divices.





IDE_primary_bus_base = 0x1f0  ;Primar
IDE_secondary_bus_base = 0x170;Secondar
IDE_third_bus_base = 0x1E8  ;third
IDE_fourth_bus_base = 0x168;forth

ATAPI_MAX_BYTE_CNT = 2048




ATA:

    ;rdx = disk
    ;returns rcx = size of sector in bytes.
    .get_sector_size:
        mov rcx, 512
        ret

    ;Pram RDX Disk num
    .check_for_disk:
            push rbx
            push rcx
            push rdx
            push r8
            push r9
            push r10
            push r11
            mov r11, rdi


            ;find drive
            xor r8, r8;primary or secondary
            xor r9, r9;slave of not slave

            ;primary
            mov rbx, IDE_primary_bus_base
            xor rax, rax
            cmp rdx, 0
            cmove r8, rbx
            cmove r9, rax
            mov rax, 00010000b;slave
            cmp rdx, 1
            cmove r8, rbx
            cmove r9, rax

            ;secondary
            mov rbx, IDE_secondary_bus_base
            xor rax, rax
            cmp rdx, 2
            cmove r8, rbx
            cmove r9, rax
            mov rax, 00010000b;slave
            cmp rdx, 3
            cmove r8, rbx
            cmove r9, rax

            ;third
            mov rbx, IDE_third_bus_base
            xor rax, rax
            cmp rdx, 4
            cmove r8, rbx
            cmove r9, rax
            mov rax, 00010000b;slave
            cmp rdx, 5
            cmove r8, rbx
            cmove r9, rax

            ;forth
            mov rbx, IDE_fourth_bus_base
            xor rax, rax
            cmp rdx, 6
            cmove r8, rbx
            cmove r9, rax
            mov rax, 00010000b;slave
            cmp rdx, 7
            cmove r8, rbx
            cmove r9, rax

            cmp r8, 0
            je .Error_no_disk



            mov rax, 0xa0;Master.
            mov rbx, 0xb0;Slave.
            cmp r9, 00010000b
            cmove rax, rbx

            lea rdx, [r8+6];Drive select.
            out dx, al

            xor rax, rax
            lea rdx, [r8+2]
            out dx, al
            lea rdx, [r8+3]
            out dx, al
            lea rdx, [r8+4]
            out dx, al
            lea rdx, [r8+5]
            out dx, al

            mov rax, 0xEC
            lea rdx, [r8+7]
            out dx, al

            lea rdx, [r8+7]
            in al, dx
            cmp al, 0
            je .Error_no_disk_found

            .check_for_disk_pool:
                in al, dx
                test al, 0x1
                jz .check_for_disk_pool_end
                test al, 0x8
                jz .check_for_disk_pool_end
                test al, 0x80
                jnz .check_for_disk_pool

            .check_for_disk_pool_end:
                lea rdx, [r8]
                mov rcx, 256

                push rdi
                push rbp
                mov rbp, rsp
                sub rsp, 512
                lea rdi, [rbp-512]

                test r10, 1b;Test for save Ident return to address in r11 (rdi).
                cmovnz rdi, r11

                .check_for_disk_identification_loop:
                    in ax, dx
                    stosw
                    dec rcx
                    jnz .check_for_disk_identification_loop

                mov rsp, rbp
                pop rbp
                pop rdi


            ;Check disk type.
            xor rax, rax
            lea rdx, [r8+2]
            in al, dx
            rol rax, 8
            lea rdx, [r8+3]
            in al, dx
            rol rax, 8
            lea rdx, [r8+4]
            in al, dx
            rol rax, 8
            lea rdx, [r8+5]
            in al, dx

            xor r10, r10;ATA
            mov rbx, 1;ATAPI
            cmp rax, 0x10114eb
            cmove r10, rbx


            pop r11
            pop r10
            pop r9
            pop r8
            pop rdx
            pop rcx
            pop rbx
            xor rax, rax
            ret


            .Error_no_disk_found:
                    pop r11
                    pop r10
                    pop r9
                    pop r8
                    pop rdx
                    pop rcx
                    pop rbx
                    mov rax, 'ND'
                    ret




    .Error_no_disk:
            pop r11
            pop r10
            pop r9
            pop r8
            pop rdi
            pop rbx
            pop rcx
            pop rdx
            pop rax
            mov rax, 'ND'
            ret



        ; ATA read sectors
        ;
        ; @param RAX Logical Block Address of sector
        ; @param RCX  Number of sectors to read
        ; @param RDI The address of buffer to put data obtained from disk
        ; @param RDX Disk num
        ;
        ; @return RAX

        ;r10 weird thing with ATAPI
        .read:
            push rax
            push rdx
            push rcx
            push rbx
            push rdi
            push r8
            push r9
            push r10
            push r11

            push rax
            push r10
            xor r10, r10
            call .check_for_disk
            cmp rax, 'ND'
            pop r10
            pop rax
            je .Error_no_disk

            mov r11, .read0
            mov rbx, .read1
            cmp r10, 0x1
            cmove r11, rbx

            ;find drive
            xor r8, r8;primary or secondary
            xor r9, r9;slave of not slave
            push rax

            ;primary
            mov rbx, IDE_primary_bus_base
            xor rax, rax
            cmp rdx, 0
            cmove r8, rbx
            cmove r9, rax
            mov rax, 00010000b;slave
            cmp rdx, 1
            cmove r8, rbx
            cmove r9, rax

            ;secondary
            mov rbx, IDE_secondary_bus_base
            xor rax, rax
            cmp rdx, 2
            cmove r8, rbx
            cmove r9, rax
            mov rax, 00010000b;slave
            cmp rdx, 3
            cmove r8, rbx
            cmove r9, rax

            ;third
            mov rbx, IDE_third_bus_base
            xor rax, rax
            cmp rdx, 4
            cmove r8, rbx
            cmove r9, rax
            mov rax, 00010000b;slave
            cmp rdx, 5
            cmove r8, rbx
            cmove r9, rax

            ;forth
            mov rbx, IDE_fourth_bus_base
            xor rax, rax
            cmp rdx, 6
            cmove r8, rbx
            cmove r9, rax
            mov rax, 00010000b;slave
            cmp rdx, 7
            cmove r8, rbx
            cmove r9, rax

            pop rax
            cmp r8, 0
            je .Error_no_disk

            ;start read
            xor rbx, rbx
            mov rdx, rcx
            .read_l:
                call r11
                add rdi, 512
                inc rax
                dec rdx
                jnz .read_l

            pop r11
            pop r10
            pop r9
            pop r8
            pop rdi
            pop rbx
            pop rcx
            pop rdx
            pop rax
            xor rax, rax;Passed.
            ret




            ;ATAPI
            .read1:
                pushfq
                push rax
                push rbx
                push rcx
                push rdx
                push rdi

                mov rbx, rax         ; Save LBA in RBX

                pop rdi
                pop rdx
                pop rcx
                pop rbx
                pop rax
                popfq
                ret
                ret



        ; ATA read sectors (LBA48 mode)
        ;
        ; @param RAX Logical Block Address of sector
        ; @param CL  Number of sectors to read
        ; @param RDI The address of buffer to put data obtained from disk
        ; @param RDX Disk num
        ;
        ; @return RAX

            ;ATA PIO
            .read0:
                pushfq
                push rax
                push rbx
                push rcx
                push rdx
                push rdi

                mov rbx, rax         ; Save LBA in RBX

                push rax
                push rcx
                push rdx
                ;prepare
                push rcx
                mov rcx, 14
                .read0_init_wait:
                    lea rdx, [r8+7]      ; Command port
                    in al, dx
                    dec rcx
                    jne .read0_init_wait
                pop rcx
                .read0_init:
                    in al, dx
                    bt rax, 7
                    jc .read0_init
                pop rdx
                pop rcx
                pop rax



                lea rdx, [r8+6]      ; Port to send drive and bit 24 - 27 of LBA
                shr rax, 24          ; Get bit 24 - 27 in al
                or al, 01000000b     ; Set bit 6 in al for LBA mode and bit 4 for slave
                or al, r9b;slave or master
                out dx, al

                lea rdx, [r8+2]      ; Port to send number of sectors
                mov al, ch           ; Get high number of sectors from CH
                out dx, al

                lea rdx, [r8+3]      ; Port to send bit 0 - 7 of LBA
                mov rax, rbx         ; Get LBA from RBX
                shr rax, 24          ; Get bit 24 - 31 in AL
                out dx, al

                lea rdx, [r8+4]      ; Port to send bit 8 - 15 of LBA
                mov rax, rbx         ; Get LBA from RBX
                shr rax, 32          ; Get bit 32 - 29 in AL
                out dx, al

                lea rdx, [r8+5]      ; Port to send bit 16 - 23 of LBA
                mov rax, rbx         ; Get LBA from RBX
                shr rax, 40          ; Get bit 40 - 48 in AL
                out dx, al


                lea rdx, [r8+2]      ; Port to send number of sectors
                mov al, cl           ; Get low number of sectors from CL
                out dx, al

                lea rdx, [r8+3]      ; Port to send bit 0 - 7 of LBA
                mov rax, rbx         ; Get LBA from RBX
                out dx, al

                lea rdx, [r8+4]      ; Port to send bit 8 - 15 of LBA
                mov rax, rbx         ; Get LBA from RBX
                shr rax, 8           ; Get bit 8 - 15 in AL
                out dx, al

                lea rdx, [r8+5]      ; Port to send bit 16 - 23 of LBA
                mov rax, rbx         ; Get LBA from RBX
                shr rax, 16          ; Get bit 16 - 23 in AL
                out dx, al

                lea rdx, [r8+7]      ; Command port
                mov al, 0x20         ; Read with retry.
                out dx, al


            .read0_still_going:
                in al, dx
                test al, 8           ; the sector buffer requires servicing.
                jz .read0_still_going      ; until the sector buffer is ready.

                mov ecx, 256         ; RCX is counter for INSW
                mov rdx, r8          ; Data port, in and out
                rep insw             ; in to [RDI]

                ;end
                push rcx
                mov rcx, 14
                .read0_end_wait:
                    lea rdx, [r8+7]      ; Command port
                    in al, dx
                    dec rcx
                    jne .read0_end_wait
                pop rcx
                .read0_wait_for_bsy_to_clear:
                    in al, dx
                    bt rax, 7
                    jc .read0_wait_for_bsy_to_clear

                pop rdi
                pop rdx
                pop rcx
                pop rbx
                pop rax
                popfq
                ret







    ; ATA write sectors (LBA48 mode)
    ;
    ; @param RAX Logical Block Address of sector
    ; @param CL  Number of sectors to write
    ; @param RSI The address of data to write to the disk
    ; @param RDX Disk num
    ;
    ; @return RAX

    .write:
        push rax
        push rdx
        push rcx
        push rbx
        push rdi
        push rsi
        push r8
        push r9
        push r10
        push r11


        push rax
        push r10
        xor r10, r10
        call .check_for_disk
        cmp rax, 'ND'
        pop r10
        pop rax
        je .Error_no_disk

        ;Disk type.
        mov r11, .write0
        mov rbx, .write1
        cmp r10, 0x1
        cmove r11, rbx


        ;find drive
        xor r8, r8;primary or secondary
        xor r9, r9;slave of not slave
        push rax

        ;primary
        mov rbx, IDE_primary_bus_base
        xor rax, rax
        cmp rdx, 0
        cmove r8, rbx
        cmove r9, rax
        mov rax, 00010000b;slave
        cmp rdx, 1
        cmove r8, rbx
        cmove r9, rax

        ;secondary
        mov rbx, IDE_secondary_bus_base
        xor rax, rax
        cmp rdx, 2
        cmove r8, rbx
        cmove r9, rax
        mov rax, 00010000b;slave
        cmp rdx, 3
        cmove r8, rbx
        cmove r9, rax

        ;third
        mov rbx, IDE_third_bus_base
        xor rax, rax
        cmp rdx, 4
        cmove r8, rbx
        cmove r9, rax
        mov rax, 00010000b;slave
        cmp rdx, 5
        cmove r8, rbx
        cmove r9, rax

        ;forth
        mov rbx, IDE_fourth_bus_base
        xor rax, rax
        cmp rdx, 6
        cmove r8, rbx
        cmove r9, rax
        mov rax, 00010000b;slave
        cmp rdx, 7
        cmove r8, rbx
        cmove r9, rax

        pop rax
        cmp r8, 0
        je .Error_no_disk


        xor rbx, rbx
        mov rdx, rcx
        .write_l:
        inc rbx

        mov cl, 1
        call r11

        add rsi, 512

        inc rax

        cmp rbx, rdx
        jl .write_l


        pop r11
        pop r10
        pop r9
        pop r8
        pop rsi
        pop rdi
        pop rbx
        pop rcx
        pop rdx
        pop rax
        xor rax, rax

        ret




        ;ATAPI.
        .write1:
            ret



        ;ATA PIO.
            .write0:
                pushfq
                push rax
                push rbx
                push rcx
                push rdx
                push rdi
                push rsi

                mov rbx, rax         ; Save LBA in RBX

                push rax
                push rcx
                push rdx
                ;prepare
                push rcx
                mov rcx, 14
                .write0_init_wait:
                    lea rdx, [r8+7]      ; Command port
                    in al, dx
                    dec rcx
                    jne .write0_init_wait
                pop rcx
                .write0_init:
                    in al, dx
                    bt rax, 7
                    jc .write0_init
                pop rdx
                pop rcx
                pop rax



                lea rdx, [r8+6]      ; Port to send drive and bit 24 - 27 of LBA
                shr rax, 24          ; Get bit 24 - 27 in al
                or al, 01000000b     ; Set bit 6 in al for LBA mode and bit 4 for slave
                or al, r9b;slave or master
                out dx, al

                lea rdx, [r8+2]      ; Port to send number of sectors
                mov al, ch           ; Get high number of sectors from CH
                out dx, al

                lea rdx, [r8+3]      ; Port to send bit 0 - 7 of LBA
                mov rax, rbx         ; Get LBA from RBX
                shr rax, 24          ; Get bit 24 - 31 in AL
                out dx, al

                lea rdx, [r8+4]      ; Port to send bit 8 - 15 of LBA
                mov rax, rbx         ; Get LBA from RBX
                shr rax, 32          ; Get bit 32 - 29 in AL
                out dx, al

                lea rdx, [r8+5]      ; Port to send bit 16 - 23 of LBA
                mov rax, rbx         ; Get LBA from RBX
                shr rax, 40          ; Get bit 40 - 48 in AL
                out dx, al


                lea rdx, [r8+2]      ; Port to send number of sectors
                mov al, cl           ; Get low number of sectors from CL
                out dx, al

                lea rdx, [r8+3]      ; Port to send bit 0 - 7 of LBA
                mov rax, rbx         ; Get LBA from RBX
                out dx, al

                lea rdx, [r8+4]      ; Port to send bit 8 - 15 of LBA
                mov rax, rbx         ; Get LBA from RBX
                shr rax, 8           ; Get bit 8 - 15 in AL
                out dx, al

                lea rdx, [r8+5]      ; Port to send bit 16 - 23 of LBA
                mov rax, rbx         ; Get LBA from RBX
                shr rax, 16          ; Get bit 16 - 23 in AL
                out dx, al

                lea rdx, [r8+7]      ; Command port
                mov al, 0x30         ; Read with retry.
                out dx, al

            .write0_still_going:
                in al, dx
                test al, 8           ; the sector buffer requires servicing.
                jz .write0_still_going      ; until the sector buffer is ready.

                mov ecx, 256         ; RCX is counter for INSW
                mov rdx, r8          ; Data port, in and out
                rep outsw             ; in to [RDI]

                ;end
                push rcx
                mov rcx, 14
                .write0_end_wait:
                    lea rdx, [r8+7]      ; Command port
                    in al, dx
                    dec rcx
                    jne .write0_end_wait
                pop rcx
                .write0_wait_for_bsy_to_clear:
                    in al, dx
                    bt rax, 7
                    jc .write0_wait_for_bsy_to_clear


            pop rsi
            pop rdi
            pop rdx
            pop rcx
            pop rbx
            pop rax
            popfq
            ret
