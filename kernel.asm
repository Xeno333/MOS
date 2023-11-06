;Filename = kernel.inc
;Type = Center of kernel.
;Description = Includes all other parts of the kernel.
;optamized Nov 15 2021


;Notes:
;Only COM1 is init
;r15 for sys controll bit 0 = keyboard waiting or not bit 1 = COM1 waiting or not


use64
align 8
org base
format binary


jmp Kstart
;Defined Data for kernel (Types, Stack size, Kernel size ...):
include './defined/kdefs.inc'

;Kernel "Header"
include "./data/kheader.inc"; at start so can tell if scanned

;Kinit:
    include './init/kinit.inc'

;Core:
    include "./core/core_functions.inc"
    include "./core/sys_interupts.inc"

;Data:
    include "./data/gdt.inc"
    include "./data/idt.inc"
    include "./data/kvars.inc"

;Manegers:
    ;Memory and Paging managers.
    include "./managers/Paging.inc"
    include "./managers/Mem_Allacotion_manager.inc"
    include "./managers/Task_manger.inc"


;APIs.
    include "./apis/MVFS.inc"
    include './apis/API_ints.inc'
    include "./apis/Disk.inc"

;Drivers:
    include "./drivers/ps2/PS2_input.inc"
    include "./drivers/power/power.inc"
    include "./drivers/io/COM.inc"
    ;In kernel disk read/write drivers.
    include "./drivers/io/Floppy.inc"
    include "./drivers/io/ATA.inc"
    ;FS drivers:
    include "./drivers/fs/UFS.inc"
    include "./drivers/fs/FAT16.inc"
    include "./drivers/fs/USTAR.inc"
    include "./drivers/fs/ext2.inc"
    ;video drivers
    include "./drivers/video/VESA/VESA.inc"
    include "./drivers/video/VESA/Char_set_and_bitmaps.inc"
    include "./drivers/video/VGA.inc"


;stack and padding
times file_length-(kernel_stack_size)-($-$$) db 0;stack
times file_length-($-$$)-8 db 0
kstack: dq 0











;other
;disable NMI
    ;in al, 0x70
    ;or al, 0x80
    ;out 0x70, al
    ;in al, 0x71




    ;mov al, 0x02
    ;out 0x70, al
    ;xor rax, rax
    ;in al, 0x71
    ;call kernel.puth

    ;jmp $

    ;mov rax, int_0x2f_picirq_c
    ;call kernel.putb

    ;putc nl

    ;mov rax, (base+int_0x2f_picirq_c-$$) & 0xffff
    ;call kernel.putb

    ;jmp $


;PC speaker

;    push rax
;    push rbx

;    mov rbx, 1193180
;    div rbx

;    xchg rbx, rax
;    mov al, 0xb6
;    out 0x43, al
;    xchg rbx, rax

;    out 0x42, al
;    xchg ah, al
;    out 0x42, al

;    in al, 0x61
;    or al, 0x03
;    out 0x61, al

;    in al, 0x61
;    and al, 0xfc
;    out 0x61, al

;    pop rax
;    pop rbx

;jmp $
