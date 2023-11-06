;Kstart entry point for kernel.



;Start Kernel
;set up
;INIT
Kstart:
    cli
    cld

    mov [KDATA.videomode], rax

    push OS_kernel
    mov rax, rsp
    mov rsi, Started
    call kernel.printf

    ;Set stack at kstack
    mov rsp, kstack
    mov byte [LOGGED_IN], 0

    ;A20 should be on but make sure
    ;enable A20 fast
    in al, 0x92
    or al, 2
    out 0x92, al


    ;detect hardware support


    ;test for rdrand
    mov rax, 1
    cpuid
    bt rcx, 30;check for rdrand
    jnc .rdrand_suport_done
    xor qword[supported_extensions], 1
    .rdrand_suport_done:

    .done_with_hardware_check:


    ;for printf
    push OS_kernel

    ;Set up New GDT.
    mov rax, rsp
    mov rsi, start_str12
    call kernel.printf



    lgdt [gdt_pointer]
    mov rbp, rsp
    push data_segment;ss ds
    push rbp
    push 010b;Flags
    push code_segment;cs
    push .flush_segments
    iretq;Flush.
    .flush_segments:
        mov ax, data_segment
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

    mov rsi, start_str_DONE
    call kernel.puts


    ;Set up interupts.
    mov rax, rsp
    mov rsi, start_str3
    call kernel.printf
    ;load idt
    lidt [IDT_x64]
    ;map and init PIC
    mov al, 0x11
    out 0x20, al
    mov al, 0x11
    out 0xa0, al
    mov al, 0x20
    out 0x21, al
    mov al, 0x28
    out 0xa1, al
    mov al, 0x04
    out 0x21, al
    mov al, 0x02
    out 0xa1, al
    mov al, 0x01
    out 0x21, al
    mov al, 0x01
    out 0xa1, al
    ;mask PIC
    mov al, 0xff
    out 0x21, al
    mov al, 0xff
    out 0xa1, al
    mov rsi, start_str_DONE
    call kernel.puts


    ;set up COM1
    mov rax, rsp
    mov rsi, start_str2
    call kernel.printf
    write_port_nosave 0x3f8 + 1, 0x00
    write_port_nosave 0x3f8 + 3, 0x80
    write_port_nosave 0x3f8 + 0, 0x03
    write_port_nosave 0x3f8 + 1, 0x00
    write_port_nosave 0x3f8 + 3, 0x03
    write_port_nosave 0x3f8 + 2, 0xC7
    write_port_nosave 0x3f8 + 4, 0x0B
    write_port_nosave 0x3f8 + 4, 0x1E
    write_port_nosave 0x3f8 + 0, 0xAE
    ;Check if serial is faulty.
    mov dx, 0x3f8
    in al, dx
    cmp al, 0xAE
    jne .COM1_init_failed
    ;on
    write_port_nosave 0x3f8 + 4, 0x0F
    write_port_nosave 0x3f8 + 1, 0x01;Enable IRQs
    ;add error here
    mov rsi, start_str_DONE
    call kernel.puts
    jmp .COM1_init_done
    .COM1_init_failed:
        mov rsi, start_str_failed
        call kernel.puts
    .COM1_init_done:







    mov rax, rsp
    mov rsi, start_str9
    call kernel.printf
    ;add SSE here
    mov rax, cr0
    and ax, 0xfffb
    or ax, 0x02
    mov cr0, rax
    mov rax, cr4
    or ax, 600;3<<9
    mov cr4, rax
    mov rsi, start_str_DONE
    call kernel.puts


    ;does not work in vbox
    ;Set Readonly pages to be enforced in rings 1-3.
    ;mov rax, cr0
    ;or rax, 10000;0xffffffffffffffff&(1<<16)
    ;mov cr0, rax



    ;Mem test.
    mov rax, rsp
    mov rsi, start_str10
    call kernel.printf
    call remap_paging
    mov rsi, start_str_DONE
    call kernel.puts

    mov rax, rsp
    mov rsi, start_str6
    call kernel.printf
    call mem_test
    mov rsi, start_str_DONE
    call kernel.puts

    mov rax, [ammount_of_mem_in_sys]

    mov rcx, 0x100000
    xor rdx, rdx
    div rcx

    push rax
    mov rax, rsp
    mov rsi, start_str_memory_found
    call kernel.printf
    add rsp, 8


    ;Init paging.
    mov rax, rsp
    mov rsi, start_str7
    call kernel.printf
    call init_paging
    mov rsi, start_str_DONE
    call kernel.puts

    ;Make Page_manager
    mov rax, rsp
    mov rsi, start_str8
    call kernel.printf
    call creat_Page_manager
    mov rsi, start_str_DONE
    call kernel.puts

    ;mov rax, rsp
    ;mov rsi, start_str11
    ;call kernel.printf
    ;mov rsi, start_str_DONE
    ;call kernel.puts






    ;Kernel heap init.
    mov rax, rsp
    mov rsi, start_str14
    call kernel.printf
    call kheap_init
    mov rsi, start_str_DONE
    call kernel.puts

    ;Floppy
    mov rax, rsp
    mov rsi, start_str13
    call kernel.printf
    ;call floppy.initialize_floppy_DMA
    mov rsi, start_str_DONE
    call kernel.puts


    mov rax, rsp
    mov rsi, start_str1
    call kernel.printf

    ;Install MVFS drivers.
    ;FAT16
    mov rsi, FAT16_call
    mov rax, 3
    call MVFS_call
    cmp rax, 0
    jne start_err
    ;USTAR
    mov rsi, USTAR_call
    mov rax, 3
    call MVFS_call
    cmp rax, 0
    jne start_err
    ;ext2
    mov rsi, ext2_call
    mov rax, 3
    call MVFS_call
    cmp rax, 0
    jne start_err
    ;UFS
    mov rsi, UFS_call
    mov rax, 3
    call MVFS_call
    cmp rax, 0
    jne start_err





    mov rax, 0
    mov rsi, UFS_drive
    mov rdi, UFS_drive_mount
    call MVFS_call

    mov rax, 1
    mov rdi, UFS_drive_read
    mov rsi, UFS_drive_read
    call MVFS_call


    mov rsi, UFS_drive_read
    call kernel.puts


    jmp $

UFS_drive cstring '2/0'
UFS_drive_mount cstring 'UFS/'

UFS_drive_read cstring 'UFS/MOS/File.txt'


    ;Mount.
    mov rax, 0
    mov rsi, KDATA.Root_partion
    mov rdi, KDATA.Root_mount_point
    call MVFS_call
    cmp rax, 0
    jne start_err

    ;if good print done
    mov rsi, start_str_DONE
    call kernel.puts



    ;unmask PIC
    cli
    mov al, PIC1_on
    out 0x21, al
    mov al, PIC2_on
    out 0xa1, al


    ;Enable IRQ 8
    mov al, 0x8b
    out 0x70, al
    in al, 0x71
    mov ah, al
    or ah, 0x40
    mov al, 0x8b
    out 0x70, al
    mov al, ah
    out 0x71, al

    mov al, 0x8c
    out 0x70, al
    in al, 0x71

    ;NMI enabel
    mov al, 0x00
    out 0x70, al
    in al, 0x71


    ;Set up Kernel task.
    mov qword [curent_task], 0xffffffffffffffff


    ;Kernel init done.
    mov byte [STARTED], 2;Staring but needs keyboard.

    call kernel.clear
    mov rax, rsp
    mov rsi, start_str15
    call kernel.printf

    mov rcx, 10
    sti
    .loop_COM1_input:
        mov r15, 10b
        hlt
        cmp al, '1'
        je .COM1_input_setup
        dec rcx
        jnz .loop_COM1_input

        jmp .done

    .COM1_input_setup:
        cli
        mov qword [KDATA.SYS_input_flags], 1
        jmp .done

    .done:
    xor r15, r15



    ;Setup Stack
    mov rsp, kstack
    call kernel.clear
    mov byte [STARTED], 1
    ;Sart ints.
    sti


    mov rax, 0
    mov rsi, fat16_drive
    mov rdi, fat16_drive_mount
    call MVFS_call
    call kernel.puth
    mov al, 0x0a
    call kernel.putc


    mov al, 0x0a
    call kernel.putc
    mov al, 0x0a
    call kernel.putc
    mov al, 0x0a
    call kernel.putc



    mov rax, 0
    mov rsi, ext2_drive
    mov rdi, ext2_drive_mount
    call MVFS_call
    call kernel.puth
    mov al, 0x0a
    call kernel.putc

    call MVFS.list_mounted_FSs




    ;Write.
    mov rsi, FileToWrite
    mov rdi, WriteToFile
    mov rax, 2
    mov rcx, 2048*4;21
    call MVFS_call
    call kernel.puth
    mov al, 0x0a
    call kernel.putc

    jmp $
    ;Read.
    mov rsi, FileToWrite
    mov rdi, kstack
    mov rax, 1
    call MVFS_call
    call kernel.puth
    mov al, 0x0a
    call kernel.putc

    mov rsi, kstack
    call kernel.puts



    jmp $


    ;FAT16
    mov rsi, ELF_file
    mov rdi, kstack
    xor r12, r12
    mov rax, 5
    call MVFS_call
    call kernel.puth
    mov al, 0x0a
    call kernel.putc

    mov rsi, rdi
    ;call PE_Loader
    call kernel.puth

    jmp $

    ;start UI
    jmp UI



WriteToFile cstring 'Hello this is a test.';Size: 21 bytes.
FileToWrite cstring 'Home/MOS/TESTW.TXT'


fat16_drive cstring '1/9'
fat16_drive_mount cstring 'EBR/'

ext2_drive cstring '0/2'
ext2_drive_mount cstring 'ext2/'


ELF_file cstring 'Home/BIN/'


;Error mountting Part 1.
start_err:
    mov rsi, start_str_failed
    call kernel.puts
    cli
    hlt


;Loging
UI:
    call kernel.clear
    ;Login
    .loggin:
        ;prepare for UI
        ;Read with MVFS
        ;mov rax, 1
        ;mov rsi, settings
        ;mov rdi, load_settings_loc
        ;call MVFS_call

        ;cmp dword [load_settings_loc], 'MOS6'
        ;jne .loggin_error

        ;Print welcome.
        mov rsi, welcome
        call kernel.puts

    .loggin_welcome_pass:
        ;Add password here.


        ;mov byte [LOGGED_IN], 0
        ;promt user for password
        ;mov rsi, epswd
        ;call kernel.puts

        ;Get password
        ;mov rsi, pswd
        ;mov rdi, pswd_end
        ;call input.gets

        ;check
        ;mov rsi, pswd
        ;mov rdi, load_settings_loc+40
        ;call string.strcmp
        ;cmp al, 0
        ;jne .loggin_welcome_pass

    .logged_in:
        ;shell
        mov byte [LOGGED_IN], 1
        ;call kernel.clear

        ;shell
        cli
        mov rsi, shell
        call New_task
        sti
        jmp $


    ;log in.
    ;.loggin_error:
        ;mov rsi, loggin_error_str
        ;call kernel.puts

        ;call input.wait_til_key
        ;jmp .logged_in

