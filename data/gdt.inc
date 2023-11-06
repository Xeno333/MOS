;GDT.inc
;GDT



struc GDT_64bit_entry GDT_64bit_entry_limit, GDT_64bit_entry_base, GDT_64bit_entry_access, GDT_64bit_entry_flags {
    dw GDT_64bit_entry_limit and 0xffff;Limit
    dw GDT_64bit_entry_base and 0xffff;Base
    db GDT_64bit_entry_base shr 16;Base
    db GDT_64bit_entry_access;Access
    db (GDT_64bit_entry_flags shl 4)+((GDT_64bit_entry_limit shl 16) and 0xF);Limit and Flags. 4 = Flags.
    db GDT_64bit_entry_base shr 24;Base
}


GDT:
.null GDT_64bit_entry 0, 0, 0, 0
.kernel_code GDT_64bit_entry 0xffffffffff, 0, 10011110b, 1010b
.kernel_data GDT_64bit_entry 0xffffffffff, 0, 10010010b, 1000b

gdt_end:

gdt_pointer:
    dw gdt_end - GDT - 1
    dq GDT

