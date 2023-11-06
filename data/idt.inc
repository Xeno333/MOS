;Filename = 64bit_itd.inc
;Type = part of kernel.
;Description = Handels interupts.
;




macro idt_entry idt_entry_base, idt_entry_GDT_selector, idt_entry_type {
        dw (base + idt_entry_base - $$) and 0xffff
        dw idt_entry_GDT_selector;GDT selecter
        dw idt_entry_type shl 8;Type.
        dw (base + idt_entry_base - $$) shr 16
        dd (base + idt_entry_base - $$) shr 32
        dd 0
}

IDT_x64_start:

expthandel:
        idt_entry error_int0, code_segment,  10001111b;0
        idt_entry error_int1, code_segment,  10001111b;1
        idt_entry error_int2, code_segment,  10001111b;2
        idt_entry error_int3, code_segment,  10001111b;3
        idt_entry error_int4, code_segment,  10001111b;4
        idt_entry error_int5, code_segment,  10001111b;5
        idt_entry error_int6, code_segment,  10001111b;6
        idt_entry error_int7, code_segment,  10001111b;7
        idt_entry error_int8, code_segment,  10001111b;8
        idt_entry error_int9, code_segment,  10001111b;9
        idt_entry error_int10, code_segment,  10001111b;10
        idt_entry error_int11, code_segment,  10001111b;11
        idt_entry error_int12, code_segment,  10001111b;12
        idt_entry error_int13, code_segment,  10001111b;13
        idt_entry error_int14, code_segment,  10001111b;14
        idt_entry error_int15, code_segment,  10001111b;15
        idt_entry error_int16, code_segment,  10001111b;16
        idt_entry error_int17, code_segment,  10001111b;17
        idt_entry error_int18, code_segment,  10001111b;18
        idt_entry error_int19, code_segment,  10001111b;19
        idt_entry error_int20, code_segment,  10001111b;20
        idt_entry error_int21, code_segment,  10001111b;21
        idt_entry error_int22, code_segment,  10001111b;22
        idt_entry error_int23, code_segment,  10001111b;23
        idt_entry error_int24, code_segment,  10001111b;24
        idt_entry error_int25, code_segment,  10001111b;25
        idt_entry error_int26, code_segment,  10001111b;26
        idt_entry error_int27, code_segment,  10001111b;27
        idt_entry error_int28, code_segment,  10001111b;28
        idt_entry error_int29, code_segment,  10001111b;29
        idt_entry error_int30, code_segment,  10001111b;30
        idt_entry error_int31, code_segment,  10001111b;31

int_0x20_0x2f_PIC1:
    idt_entry int_0x20_picirq_c, code_segment,  10001110b;32
    idt_entry int_0x21_picirq_c, code_segment,  10001110b;32
    idt_entry int_0x22_picirq_c, code_segment,  10001110b;32
    idt_entry int_0x23_picirq_c, code_segment,  10001110b;32
    idt_entry int_0x24_picirq_c, code_segment,  10001110b;32
    idt_entry int_0x25_picirq_c, code_segment,  10001110b;32
    idt_entry int_0x26_picirq_c, code_segment,  10001110b;32
    idt_entry int_0x27_picirq_c, code_segment,  10001110b;32
    idt_entry int_0x28_picirq_c, code_segment,  10001110b;32
    idt_entry int_0x29_picirq_c, code_segment,  10001110b;32
    idt_entry int_0x2a_picirq_c, code_segment,  10001110b;32
    idt_entry int_0x2b_picirq_c, code_segment,  10001110b;32
    idt_entry int_0x2c_picirq_c, code_segment,  10001110b;32
    idt_entry int_0x2d_picirq_c, code_segment,  10001110b;32
    idt_entry int_0x2e_picirq_c, code_segment,  10001110b;32
    idt_entry int_0x2f_picirq_c, code_segment,  10001110b;32

int_0x30_system:
    idt_entry int_0x30_sys_call_c, code_segment,  10001110b;32
    idt_entry int_0x31c, code_segment,  10001110b;32
    idt_entry int_0x32_diskio_c, code_segment,  10001110b;32
    idt_entry int_0x33_kill_c, code_segment,  10001110b;32


IDT_x64_end:

IDT_x64:
    dw IDT_x64_end - IDT_x64_start - 1
    dq IDT_x64_start

    dd 0


