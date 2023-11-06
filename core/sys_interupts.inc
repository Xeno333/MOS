

;Error
error_int0:
    cli
    cld;Clear drirection flag.

    mov qword[Error_type], 'DE'
    push Error_type
    mov rax, rsp
    mov rsi, Stack_Frame_str
    call kernel.printf

    mov rcx, [curent_task]
    call Kill

    ;check for tasks
    cmp qword [num_of_tasks_running], 0
    je UI.loggin

    sti
    jmp $

error_int1:
    cli
    cld;Clear drirection flag.

    mov qword[Error_type], 'DB'
    push Error_type
    mov rax, rsp
    mov rsi, Stack_Frame_str
    call kernel.printf

    mov rcx, [curent_task]
    call Kill

    ;check for tasks
    cmp qword [num_of_tasks_running], 0
    je UI.loggin

    sti
    jmp $

error_int2:
    cli
    cld;Clear drirection flag.

    mov qword[Error_type], 'NMI'
    push Error_type
    mov rax, rsp
    mov rsi, Stack_Frame_str
    call kernel.printf

    mov rcx, [curent_task]
    call Kill

    ;check for tasks
    cmp qword [num_of_tasks_running], 0
    je UI.loggin

    sti
    jmp $

error_int3:
    cli
    cld;Clear drirection flag.

    mov qword[Error_type], 'BP'
    push Error_type
    mov rax, rsp
    mov rsi, Stack_Frame_str
    call kernel.printf

    mov rcx, [curent_task]
    call Kill

    ;check for tasks
    cmp qword [num_of_tasks_running], 0
    je UI.loggin

    sti
    jmp $

error_int4:
    cli
    cld;Clear drirection flag.

    mov qword[Error_type], 'OF'
    push Error_type
    mov rax, rsp
    mov rsi, Stack_Frame_str
    call kernel.printf

    mov rcx, [curent_task]
    call Kill

    ;check for tasks
    cmp qword [num_of_tasks_running], 0
    je UI.loggin

    sti
    jmp $

error_int5:
    cli
    cld;Clear drirection flag.

    mov qword[Error_type], 'BR'
    push Error_type
    mov rax, rsp
    mov rsi, Stack_Frame_str
    call kernel.printf

    mov rcx, [curent_task]
    call Kill

    ;check for tasks
    cmp qword [num_of_tasks_running], 0
    je UI.loggin

    sti
    jmp $

;UD
error_int6:
    cli
    cld;Clear drirection flag.

    mov qword[Error_type], 'UD'
    push Error_type
    mov rax, rsp
    mov rsi, Stack_Frame_str
    call kernel.printf
    pop rax


    mov rcx, [curent_task]
    call Kill

    ;check for tasks
    cmp qword [num_of_tasks_running], 0
    je UI.loggin

    sti
    jmp $

error_int7:
    cli
    cld;Clear drirection flag.

    mov qword[Error_type], 'NM'
    push Error_type
    mov rax, rsp
    mov rsi, Stack_Frame_str
    call kernel.printf

    mov rcx, [curent_task]
    call Kill

    ;check for tasks
    cmp qword [num_of_tasks_running], 0
    je UI.loggin

    sti
    jmp $

;DF add code here
;Add crash code
error_int8:
    cli
    cld;Clear drirection flag.
    pop rbx;Error Code

    mov qword[Error_type], 'DF'
    push Error_type

    mov rax, rsp
    mov rsi, Stack_Frame_str
    call kernel.printf
    pop rax

    push rbx
    mov rax, rsp
    mov rsi, Error_code_str
    call kernel.printf

    cli
    hlt


error_int9:
    cli
    cld;Clear drirection flag.

    mov qword[Error_type], 'CSO'
    push Error_type
    mov rax, rsp
    mov rsi, Stack_Frame_str
    call kernel.printf

    mov rcx, [curent_task]
    call Kill

    ;check for tasks
    cmp qword [num_of_tasks_running], 0
    je UI.loggin

    sti
    jmp $

error_int10:
    cli
    cld;Clear drirection flag.

    pop rbx;Error Code
    mov qword[Error_type], 'TS'
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

error_int11:
    cli
    cld;Clear drirection flag.

    pop rbx;Error Code
    mov qword[Error_type], 'NP'
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

error_int12:
    cli
    cld;Clear drirection flag.

    pop rbx;Error Code
    mov qword[Error_type], 'SS'
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

;GP
error_int13:
    cli
    cld;Clear drirection flag.

    pop rbx;Error Code
    mov qword[Error_type], 'GP'
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

;PF
error_int14:
    cli
    jmp Page_Fault

;Reserved
error_int15:
    iretq


error_int16:
    cli
    cld;Clear drirection flag.

    mov qword[Error_type], 'MF'
    push Error_type
    mov rax, rsp
    mov rsi, Stack_Frame_str
    call kernel.printf

    mov rcx, [curent_task]
    call Kill

    ;check for tasks
    cmp qword [num_of_tasks_running], 0
    je UI.loggin

    sti
    jmp $

error_int17:
    cli
    cld;Clear drirection flag.

    pop rbx;Error Code
    mov qword[Error_type], 'AC'
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

;MC
error_int18:
    cli
    cld;Clear drirection flag.

    mov qword[Error_type], 'MC'
    push Error_type
    mov rax, rsp
    mov rsi, Stack_Frame_str
    call kernel.printf

    cli
    hlt

error_int19:
    cli
    cld;Clear drirection flag.

    mov qword[Error_type], 'XF'
    push Error_type
    mov rax, rsp
    mov rsi, Stack_Frame_str
    call kernel.printf

    mov rcx, [curent_task]
    call Kill

    ;check for tasks
    cmp qword [num_of_tasks_running], 0
    je UI.loggin

    sti
    jmp $

error_int20:
    cli
    cld;Clear drirection flag.

    mov qword[Error_type], 'VE'
    push Error_type
    mov rax, rsp
    mov rsi, Stack_Frame_str
    call kernel.printf

    mov rcx, [curent_task]
    call Kill

    ;check for tasks
    cmp qword [num_of_tasks_running], 0
    je UI.loggin

    sti
    jmp $


;Resurved
error_int21:
    iretq

error_int22:
    iretq

error_int23:
    iretq

error_int24:
    iretq

error_int25:
    iretq

error_int26:
    iretq

error_int27:
    iretq

error_int28:
    iretq

error_int29:
    iretq


error_int30:
    cli
    cld;Clear drirection flag.

    pop rbx;Error Code
    mov qword[Error_type], 'SX'
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


;Reserved
error_int31:
    iretq







;PIC1


int_0x20_picirq_c:;timer will add multi tasking here
    cli
    cld

    inc qword [timer]

    push rax
    mov al, 0x20
    out 0x20, al
    mov al, 0
    out 0x60, al
    pop rax

    cmp byte [KDATA.Wait_task_switch], 1
    je int_0x20_picirq_c.int_0x20_picirq_c_pass_Task_switch

    jmp TS

    .int_0x20_picirq_c_pass_Task_switch:
        iretq





int_0x21_picirq_c:;keyboard
    cli
    cld
    write_port 0x20, 0x20
    call ps2.key_pressed
    iretq






;Other PIC


int_0x22_picirq_c:
    cli
    cld
    write_port 0x20, 0x20
    iretq


int_0x23_picirq_c:
    cli
    write_port 0x20, 0x20
    iretq

;COM1
;Need to get input
int_0x24_picirq_c:
    cli
    cld

    write_port 0x20, 0x20
    call serial.COM1_data_in
    iretq

int_0x25_picirq_c:
    cli
    cld
    write_port 0x20, 0x20
    iretq


;?
    ;push rax
    ;push rdx
    ;mov al, 0x20
    ;out 0x20, al
    ;;Enable FIFO, clear them, with 14-byte threshold
    ;mov al, 0xC7
    ;mov dx, 0x3FA
    ;out dx, al
    ;pop rdx
    ;pop rax



;Floppy
int_0x26_picirq_c:
    cli
    cld
    write_port 0x20, 0x20
    call floppy.IRQ
    iretq


int_0x27_picirq_c:
    cli
    cld
    write_port 0x20, 0x20
    iretq

;PIC2

int_0x28_picirq_c:;rtc
    cli
    cld;Clear drirection flag.

    ;read C
    push rax
    mov al, 0x0c
    out 0x70, al
    in al, 0x71
    pop rax


    write_port 0x20, 0x20
    write_port 0x0a, 0x20
    iretq


int_0x29_picirq_c:
    cli
    cld
    write_port 0x20, 0x20
    write_port 0x0a, 0x20
    iretq
int_0x2a_picirq_c:
    cli
    cld
    write_port 0x20, 0x20
    write_port 0x0a, 0x20
    iretq
int_0x2b_picirq_c:
    cli
    cld
    write_port 0x20, 0x20
    write_port 0x0a, 0x20
    iretq
int_0x2c_picirq_c:
    cli
    cld
    write_port 0x20, 0x20
    write_port 0x0a, 0x20
    iretq
int_0x2d_picirq_c:
    cli
    cld
    write_port 0x20, 0x20
    write_port 0x0a, 0x20
    iretq
int_0x2e_picirq_c:
    cli
    cld
    write_port 0x20, 0x20
    write_port 0x0a, 0x20
    iretq
int_0x2f_picirq_c:
    cli
    cld
    write_port 0x20, 0x20
    write_port 0x0a, 0x20
    iretq





