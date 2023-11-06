;Disk.inc
;Kernel's disk drivers.
;
;LBA 48 bit

;TODO
;Add Flopy.
;Add DMA.
;Add error handeling.
;Add AHCI.
;Add SATA.



;Add check for disk.



Disk_io_funtions:
    cmp r8, 0
    je .read
    cmp r8, 1
    je .write
    cmp r8, 2
    je .check_for_disk
    cmp r8, 3
    je .get_sector_size
    ret




    ;=============================================================================
    ; Read sectors
    ;
    ; param RAX Logical Block Address of sector
    ; param CL  Number of sectors to read
    ; param RDI The address of buffer to put data obtained from disk
    ; param RDX Disk num
    ;
    ; return RAX
    ;=============================================================================
    .read:
        ;Floppy
        cmp rdx, 8
        je floppy.read
        ;ATA
        cmp rdx, 7
        jle ATA.read

        ret


    ;=============================================================================
    ; Write sectors
    ;
    ; param RAX Logical Block Address of sector
    ; param CL  Number of sectors to write
    ; param RDI The address of data to write to the disk
    ; param RDX Disk num
    ;
    ; return RAX
    ;=============================================================================

    .write:
        ;Floppy
        cmp rdx, 8
        je floppy.write
        ;ATA
        cmp rdx, 7
        jle ATA.write

        ret

    ;=============================================================================
    ; Read/write blocks size in bytes. Note this is to make drivers simpeler so that the disk driver does the sector conversion.
    ;
    ; param RAX Logical Block Address of sector
    ; param RCX  Number of bytes to read, Must be divisable by sector number or it will round down.
    ; param RDI The address of buffer to put data obtained from disk
    ; param RDX Disk num
    ;
    ; return RAX
    ;=============================================================================

    .read_block_size_in_bytes:
        ;Conbvert size.
        push rdx
        push rax
        mov rax, rcx
        xor rdx, rdx
        mov rcx, 512
        div rcx
        mov rcx, rax
        pop rax
        pop rdx

        ;Floppy
        cmp rdx, 8
        je floppy.read
        ;ATA
        cmp rdx, 7
        jle ATA.read

        ret

    .write_block_size_in_bytes:
        ;Conbvert size.
        push rdx
        push rax
        mov rax, rcx
        xor rdx, rdx
        mov rcx, 512
        div rcx
        mov rcx, rax
        pop rax
        pop rdx

        ;Floppy
        cmp rdx, 8
        je floppy.write
        ;ATA
        cmp rdx, 7
        jle ATA.write

        ret


    .check_for_disk:
        ;Floppy
        cmp rdx, 8
        je floppy.check_for_disk
        ;ATA
        cmp rdx, 7
        jle ATA.check_for_disk

        ret

    .get_sector_size:
        ;Floppy
        cmp rdx, 8
        je floppy.get_sector_size
        ;ATA
        cmp rdx, 7
        jle ATA.get_sector_size

        ret






