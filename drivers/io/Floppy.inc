;Floppy.inc
;Part of Disk.inc
;


floppy_base = 0x3F0
floppy_retry_MAX = 5
floppy_DMA_MEM = 0x1000


floppy_DATA:
    .IRQ_recived: db 0
    .implied_seek_enable = 1
    .fifo_disable = 0
    .drive_polling_mode_disable = 1
    .threshold = 8
    .precomp_val = 0


macro outb arg0, arg1 {
    mov al, arg1
    out arg0, al
}


floppy:

    .get_sector_size:
        mov rcx, 512
        ret


    .check_for_disk:
        ret


    .loop_RQM1_DIO0:
        mov dx, floppy_base+4
        in al, dx
        test al, 0x80
        jz .loop_RQM1_DIO0
        test al, 0x40
        jnz .loop_RQM1_DIO0
        ret


    ;Waits number of PIT ticks in RAX
    .wait:
        push rax
        add rax, [timer]
        sti
        .wait_loop:
            cmp rax, [timer]
            jne .wait_loop
        cli
        pop rax
        ret



    ;IRQ6 handeler.
    .IRQ:
        mov byte [floppy_DATA.IRQ_recived], 1
        push rsi
        mov rsi, KDATA.IRQ0x6
        call kernel.puts
        pop rsi
        ret



    ;Init Floppy + Floppy DMA.
    .initialize_floppy_DMA:
        mov byte [KDATA.Wait_task_switch], 1
        push rdx

        ; set DMA channel 2 to transfer data from 0x1000 - 0x33ff in memory
        ; paging must map this _physical_ memory elsewhere and _pin_ it from paging to disk!
        ; set the counter to 0x23ff, the length of a track on a 1.44 MiB floppy - 1 (assuming 512 byte sectors)
        ; transfer length = counter + 1
        outb 0x0a, 0x06      ; mask DMA channel 2 and 0 (assuming 0 is already masked)
        outb 0x0c, 0xFF      ; reset the master flip-flop
        outb 0x04, floppy_DMA_MEM and 0xff         ; address to 0 (low byte)
        outb 0x04, floppy_DMA_MEM shr 8      ; address to 0x10 (high byte)
        outb 0x0c, 0xFF      ; reset the master flip-flop (again!!!)
        outb 0x05, 0xFF      ; count to 0x23ff (low byte)
        outb 0x05, 0x23      ; count to 0x23ff (high byte),
        outb 0x81, 0         ; external page register to 0 for total address of 00 10 00
        outb 0x0a, 0x02      ; unmask DMA channel 2


        ;Init floppy controller.
        call .reset

        mov al, 16;Version.
        mov dx, floppy_base+5
        out dx, al

        xor rax, rax
        in al, dx
        cmp al, 0x90
        jne .initialize_floppy_DMA_err


        mov al, 19;Configure.
        mov dx, floppy_base+5
        out dx, al
        xor rax, rax
        out dx, al
        mov al, (floppy_DATA.implied_seek_enable shl 6) or (floppy_DATA.fifo_disable shl 5) or (floppy_DATA.drive_polling_mode_disable shl 4) or (floppy_DATA.threshold - 1)
        out dx, al
        mov al, floppy_DATA.precomp_val
        out dx, al

        mov al, 94;Lock.
        mov dx, floppy_base+5
        out dx, al
        in al, dx

        call .reset


        ;Recalabrates disks in revers order.
        mov bl, 4
        .initialize_floppy_DMA_IRQ_recal_loop:
            mov al, 7;Recalabraite.
            mov dx, floppy_base+5
            out dx, al
            mov al, bl
            out dx, al

            mov byte [floppy_DATA.IRQ_recived], 0
            sti
            .initialize_floppy_DMA_IRQ_loop:
                cmp byte [floppy_DATA.IRQ_recived], 1
                jne .initialize_floppy_DMA_IRQ_loop

            cli
            mov byte [floppy_DATA.IRQ_recived], 0

            ;Get result.
            in al, dx

            ;Int recived.
            mov dx, floppy_base+5
            mov al, 0x08
            out dx, al
            ;Wait.
            mov rax, 2
            call .wait
            ;Get result.
            in al, dx
            in al, dx

            dec bl
            jnz .initialize_floppy_DMA_IRQ_recal_loop





        xor rax, rax
        jmp .initialize_floppy_DMA_done

        .initialize_floppy_DMA_err:
            mov rax, 1;Do not suport floppy.
            jmp .initialize_floppy_DMA_done

        .initialize_floppy_DMA_done:
            pop rdx
            mov byte [KDATA.Wait_task_switch], 0
            ret




    ;Reaset floppy controler.
    .reset:
        mov byte [KDATA.Wait_task_switch], 1
        pushfq
        push rdx
        push rbx

        mov byte [floppy_DATA.IRQ_recived], 0

        call .loop_RQM1_DIO0

        ;Reset.
        cli
        mov dx, floppy_base+2
        in al, dx
        bts rax, 2
        out dx, al
        push rax
        xor al, al
        out dx, al
        ;wait.
        mov rax, 2
        call .wait
        pop rax
        out dx, al


        sti
        .reset_IRQ_loop:
            cmp byte [floppy_DATA.IRQ_recived], 1
            jne .reset_IRQ_loop

        cli
        mov byte [floppy_DATA.IRQ_recived], 0


        ;Int recived.
        mov dx, floppy_base+5
        mov al, 0x08
        out dx, al
        ;Wait.
        mov rax, 2
        call .wait
        ;Get result.
        in al, dx
        in al, dx




        xor rax, rax
        jmp .reset_done

        .reset_done:
            pop rbx
            pop rdx
            popfq
            mov byte [KDATA.Wait_task_switch], 0
            ret





    .read:
        mov byte [KDATA.Wait_task_switch], 1
        pushfq
        push rcx
        push rdx
        push r8

        mov r8, rdx

        ;Add LBA to CHS here

        call .reset

        ;Select drive.
        mov dx, floppy_base+7
        mov al, 0
        out dx, al

        mov dx, floppy_base+2
        in al, dx
        or al, 00010011b
        out dx, al

        ;Wait to send cammand.
        call .loop_RQM1_DIO0


        mov byte [floppy_DATA.IRQ_recived], 0


        outb 0x0a, 0x06     ; mask DMA channel 2 and 0 (assuming 0 is already masked)
        outb 0x0b, 0x5A     ; 01011010
                            ; single transfer, address increment, autoinit, write, channel2)
        outb 0x0a, 0x02     ; unmask DMA channel 2


        mov dx, floppy_base+5
        mov al, 0x40 or 0x80 or 0x06;Read.
        out dx, al
        call .loop_RQM1_DIO0

        mov al, 0;Head << 2 | Disk.
        out dx, al
        call .loop_RQM1_DIO0

        mov al, 0;Cylinder.
        out dx, al
        call .loop_RQM1_DIO0

        mov al, 0;Head.
        out dx, al
        call .loop_RQM1_DIO0

        mov al, 1;Sector.
        out dx, al
        call .loop_RQM1_DIO0

        mov al, 2;?
        out dx, al
        call .loop_RQM1_DIO0

        mov al, 63;EOT
        out dx, al
        call .loop_RQM1_DIO0

        mov al, 0x1b;?
        out dx, al
        call .loop_RQM1_DIO0

        mov al, 0xff;?
        out dx, al




        sti
        .read_IRQ_loop:
            cmp byte [floppy_DATA.IRQ_recived], 1
            jne .read_IRQ_loop

        cli
        mov byte [floppy_DATA.IRQ_recived], 0

        in al, dx
        call kernel.puth
        mov al, 0x0a
        call kernel.putc
        in al, dx
        call kernel.puth
        mov al, 0x0a
        call kernel.putc
        in al, dx
        call kernel.puth
        mov al, 0x0a
        call kernel.putc
        in al, dx
        call kernel.puth
        mov al, 0x0a
        call kernel.putc
        in al, dx
        call kernel.puth
        mov al, 0x0a
        call kernel.putc
        in al, dx
        call kernel.puth
        mov al, 0x0a
        call kernel.putc
        in al, dx
        call kernel.puth
        mov al, 0x0a
        call kernel.putc

        mov rax, [floppy_DMA_MEM]
        call kernel.puth


        pop r8
        pop rdx
        pop rcx
        popfq
        mov byte [KDATA.Wait_task_switch], 0
        ret




    .write:
        mov byte [KDATA.Wait_task_switch], 1

        mov byte [KDATA.Wait_task_switch], 0
        ret


















