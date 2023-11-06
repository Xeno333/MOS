;Paging.inc
;Handels paging


;Memory guide:
;page 0 hardware
;page 1 Kernel
;page 2 PLM4+PLM4
;page 3 PDT11
;page 4 PT1-512
;page 5 Page_manager_AT
;page 6 Task table
;page 7 Hardware free vitual points to task running
;Page 8 First Heap allacotion sectoion.
;page 9+ other







page_size = 0x200000

PLM4 = 0x400000
PDPT1 = PLM4+0x1000
PDT1 = PLM4+0x200000
PT1 = PLM4+0x400000

Page_manager_AT = 0xa00000
TASK_MAP = 0xc00000
Running_TASK = 0xe00000




PAGING:
    ;remeber enable ints
    ;pushes 48 bytes
    Page_Fault:
        pushf
        cmp qword [PF_handel_methed], 1
        je .mem_test
        cmp qword [PF_handel_methed], 2
        je .mem_test2
        popf

        ;Add handeler here

        pop rbx;Error Code
        mov qword[Error_type], 'PF'
        push Error_type
        mov rax, rsp
        mov rsi, Stack_Frame_str
        call kernel.printf
        pop rax
        push rbx
        mov rax, rsp
        mov rsi, Error_code_str
        call kernel.printf
        pop rax

        mov rcx, [curent_task]
        call Kill

        ;check for tasks
        cmp qword [num_of_tasks_running], 0
        je UI.loggin

        sti
        jmp $


        .mem_test:
            popf
            add rsp, 48
            jmp mem_test.page_fault_mem_test
        .mem_test2:
            popf
            add rsp, 48
            jmp remap_paging.page_fault_mem_test







    Map_page_to_other_page:
        push rdi
        push rax
        pushf
        lea rdi, [PDT1+(rax*8)]
        mov rax, [PDT1+(rbx*8)]
        mov [rdi], rax
        popf
        pop rax
        pop rdi
        ret

    Page_update:
        push rdi
        pushf
        lea rdi, [PDT1+(rax*8)]
        mov [rdi], rbx
        popf
        pop rdi
        ret

    Page_inv:
        push rdi
        pushf
        lea rdi, [PDT1+(rax*8)]
        and qword[rdi], 0xffffffffffffffff
        popf
        pop rdi
        ret

    Page_val:
        push rdi
        pushf
        lea rdi, [PDT1+(rax*8)]
        or qword[rdi], 1
        popf
        pop rdi
        ret





    ;Page manager.
    ;1 page is used as a Page Alacation Table
    ;It can address up to 256 GiB
    ;Each entry is 16 bytes [Owner(ID)][infomation(permisions)(state)]
    ;information
    ;INFO = bit 0 = type if 0 = kernel, 1 = user
    ;ID = 0 means Free.
    ;ID = 1 means Hardware.
    ;ID = 2 means Kernel.
    ;ID = 3 means Paging.
    ;ID = 4 means Page_manager.
    ;ID = 5 means Task.
    ;ID = 6 means Task switch space.
    ;ID = 7 means Task MAP.
    ;ID = 8 means Hardware Kernel memory.
    ;ID = 9 means Heap allacation section.
    creat_Page_manager:
        mov rdi, Page_manager_AT
        mov word[rdi], 0x0001;Hardware
        add rdi, 2
        mov word[rdi], 0x0002;Kernel
        add rdi, 2
        mov word[rdi], 0x0003;PLM4+PLM4
        add rdi, 2
        mov word[rdi], 0x0003;PDT
        add rdi, 2
        mov word[rdi], 0x0003;PT
        add rdi, 2
        mov word[rdi], 0x0004;Page_manager_AT
        add rdi, 2
        mov word[rdi], 0x0007;TASK_MAP
        ret



    ;input rcx = ofset starting at 0
    ;output ax = value
    Page_manager_look_up:
        pushf
        xor rax, rax
        mov ax, [Page_manager_AT+(rcx*2)]
        popf
        ret


    ;input rcx = ofset starting at 0
    ;output ax = value
    ;Free page entry (2MiB).
    Page_manager_delete:
        pushf
        push rax
        xor rax, rax
        mov [Page_manager_AT+(rcx*2)], ax
        pop rax
        popf
        ret




    ;input al = Type, ah = infomation
    ;output rax = memLocation, rcx = page_num
    ;carry cleared if sucsess, set if faild
    ;Resurve page entry (2MiB).
    New_Page_manager_entry:
        pushf
        push rdi
        mov rdi, Page_manager_AT-2
        .find_loop:
            add rdi, 2
            cmp rdi, Page_manager_AT+page_size
            je .no_room
            cmp word [rdi], 0x0000
            jne .find_loop

        .find_loop_end:
            mov [rdi], ax
            sub rdi, Page_manager_AT
            mov rax, rdi
            mov rdi, 2
            div rdi
            mov rcx, rax
            mov rdi, page_size
            mul rdi
            pop rdi
            popf
            clc
            ret

        .no_room:
            pop rdi
            popf
            stc
            ret




















    ;Kernel init



    mem_test:
        mov rdi, PDT1
        mov rbx, PT1+0000000000000000000000000000000000000000000000000000000000000111b
        mov rcx, 512*512
        .mem_test_initPT:
            mov [rdi], rbx
            add rbx, 0x1000
            add rdi, 8
            dec rcx
            jnz .mem_test_initPT

        mov rdi, PT1
        mov rbx, 0000000000000000000000000000000000000000000000000000000000000111b
        mov rcx, 512*512
        .mem_test_initPDT:
            mov [rdi], rbx
            add rbx, 0x1000
            add rdi, 8
            dec rcx
            jnz .mem_test_initPDT

        mov qword [PF_handel_methed], 1
        mov rdi, 0
        mov rcx, 512
        xor rax, rax
        .mem_test:
            ;if not 0 means working
            mov rbx, [rdi]
            cmp rbx, 0
            jne .pass
            mov word [rdi], 'mt'
            cmp word [rdi], 'mt'
            jne .page_fault_mem_test
            mov [rdi], rbx
            .pass:
            add rdi, 0x200000
            inc rax
            dec rcx
            jnz .mem_test

        .page_fault_mem_test:
            mov qword [PF_handel_methed], 0
            mov rcx, 0x200000
            mul rcx
            mov [ammount_of_mem_in_sys], rax
            ret


        .PT_0x1000:





    init_paging:
        ;PDT 1
        ;1GiB
        ;PT
        mov rdi, PT1
        mov rbx, 3;r/w p
        mov rax, [ammount_of_mem_in_sys]
        mov rcx, 0x1000
        div rcx
        mov rcx, rax
        .PT_loop:
            mov [rdi], rbx
            add rbx, 0x1000
            add rdi, 8
            dec rcx
            jnz .PT_loop
        ;PDT
        mov rdi, PDT1
        mov rbx, PT1+3;r/w p
        mov rax, [ammount_of_mem_in_sys]
        mov rcx, 0x200000
        div rcx
        mov rcx, rax
        .PDT_loop:
            mov [rdi], rbx
            add rbx, 0x1000
            add rdi, 8
            dec rcx
            jnz .PDT_loop

        ;PLM4+PDPT
        mov qword[PLM4], PDPT1+3;r/w p
        mov qword[PDPT1], PDT1+3;r/w p
        ;clear the rest
        mov rdi, PLM4+8
        mov rcx, 3
        xor rax, rax
        rep stosq
        mov rdi, PDPT1+8
        mov rcx, 0xff8
        xor rax, rax
        rep stosq
        ret








    remap_paging:
        ;new PLM4 at 0x400000
        mov rdi, PLM4
        mov rcx, 0x10000
        xor rax, rax
        rep stosd

        ;set up PLM4 and temp PDT1 PT
        mov rdi, PLM4
        mov qword[rdi], PLM4+0x1003
        add rdi, 0x1000
        mov qword[rdi], PLM4+0x2003
        add rdi, 0x1000
        mov rcx, 512
        mov rax, PLM4+0x3003
        .PDT1:
            mov [rdi], rax
            add rax, 0x1000
            add rdi, 8
            dec rcx
            jne .PDT1
        mov rdi, PLM4+0x3000
        mov rbx, 3;r/w p
        mov rcx, 512*sys_min_memory;map 32 MiB
        .PT_loop:
            mov [rdi], rbx
            add rbx, 0x1000
            add rdi, 8
            dec rcx
            jnz .PT_loop
        ;update PLM4 pointer
        mov rdi, PLM4
        mov dword[rdi], PLM4+0x1003
        mov cr3, rdi


        ;remap PDT1 to 0x600000
        mov rdi, PDT1
        mov rcx, sys_min_memory;32 MiB at PDT
        mov rax, PT1+3
        .PDT1n:
            mov [rdi], rax
            add rax, 0x1000
            add rdi, 8
            dec rcx
            jne .PDT1n
        ;remap PT1 to 0x800000
        mov rdi, PT1
        mov rbx, 3;r/w p
        mov rcx, 512*(sys_min_memory/2);32 MiB at PT
        .PT_loopn:
            mov [rdi], rbx
            add rbx, 0x1000
            add rdi, 8
            dec rcx
            jnz .PT_loopn

        mov rdi, 0
        mov rcx, (sys_min_memory/2)
        xor rax, rax
        mov qword [PF_handel_methed], 2
        .mem_test:
            ;if not 0 means working
            mov rbx, [rdi]
            cmp rbx, 0
            jne .pass
            mov word [rdi], 'mt'
            cmp word [rdi], 'mt'
            jne .page_fault_mem_test
            mov [rdi], rbx
            .pass:
            add rdi, 0x200000
            inc rax
            dec rcx
            jnz .mem_test

        .page_fault_mem_test:
            mov qword [PF_handel_methed], 0
            mov rcx, 0x2
            mul rcx
            cmp rax, sys_min_memory
            jl .not_enugh_memory_for_os


        mov qword[PLM4], PDPT1+3
        mov qword[PDPT1], PDT1+3
        ret

        .not_enugh_memory_for_os:
            mov rsi, not_enugh_memory_for_os_str
            call kernel.puts
            hlt
