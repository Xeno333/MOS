;Pre defined macros for kernel.



macro write_port arg0, arg1 {
    push rax
    mov al, arg1
    out arg0, al
    pop rax
}

;works for port num larger than 1 byte
macro write_port_nosave arg0, arg1 {
    mov al, arg1
    mov dx, arg0
    out dx, al
}

macro putc arg0 {
    mov al, arg0
    call kernel.putc
}

macro puts arg0 {
    mov rsi, arg0
    call kernel.puts
}

macro puth arg0 {
    mov rax, arg0
    call kernel.puth
}


;Predefined sybols to make kernel easy to maintain.
nl = 0x0a;newline



;DATA TYPES for kernel.
struc char arg0 {
    db arg0
}
struc int_8  arg0 {
    db arg0
}
struc int_16  arg0 {
    dw arg0
}
struc int_32  arg0 {
    dd arg0
}
struc int_64  arg0 {
    dq arg0
}
struc float_64 arg0, arg2 {
    common
    .raw64: dq
    .place: db
}
struc int_80  arg0 {
    dt arg0
}
struc pointer  arg0 {
    dq arg0
}
struc pstring [data] {
    common
    . db data
    .size: dq  $ - .
}
struc cstring [data] {
    common
    . db data, 0
}

chars = 1
int_8s = 1
int_16s = 2
int_32s = 4
int_64s = 8
ints = 8
longs = 8
pointers = 8



;Predefined sybols for kernel.
vid_mem = 0xa0000;Video memory offset
base = 0x200000;Kernel base
file_length = 1024*1024;Length of kernel in bytes 1MiB

size_of_exe = 0x10000;bytes = 64KiB
kernel_stack_size = 1024*4;4 KiB
load_settings_loc = 0x00b8000-1024*128

code_segment = GDT.kernel_code-GDT
data_segment = GDT.kernel_data-GDT

PIC1_on = 00000000b
PIC1_off = 11111111b
PIC2_on = 00000000b
PIC2_off = 11111111b


max_task_num = 32;Max number of tasks on main core

sys_min_memory = 32;MiB

