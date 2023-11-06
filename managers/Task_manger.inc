;Manegas tasks
;Task info
;bit 0 = task bit, 1 = yes
;bit 1 = state bit, 1 = killed, 0 = running
;bit 2 = background bit, 1 = background, 0 = forground
;bit 3 = paused bit, 1 = wating for calle to return, 0 = running
;bit 4 = Type bit, 1 = Callable Modual (Extension), 0 = Normal



;Todo:
;Add forground tasks.
;Shift off old task manger.
;Add call task.



;Fix needed!
;When proces exits and a new proces loads it doesent go to it.




    ;Add new task to system task maneger
    ;RSI = file name.
    New_task:
        pushf
        push rbp
        push rbx
        push rcx
        push rdx
        push r8
        push r9
        push r10
        push r11
        push r12
        push r13
        push r14
        push r15
        push rsi
        push rdi
        cli

        mov rdi, file_name
        call string.str_copy
        call .init

        pop rdi
        pop rsi
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        pop r10
        pop r9
        pop r8
        pop rdx
        pop rcx
        pop rbx
        pop rbp
        popf
        ret

        .init:
            cli
            mov ax, 0x0005
            call New_Page_manager_entry
            jc .error0
            mov rdi, rax

            lea rbx, [rdi+0x200000-16-(8*21)];stack
            mov rcx, rdi;Adress
            mov rax, 1;info
            mov rdx, [curent_task];parent
            call New_Task_entry;make
            jc .error0;if err

            mov rsi, file_name
            mov rax, 1
            call MVFS_call


            cmp rax, 0
            jne .error2

            cmp dword[rdi], 'ME64'
            jne .error1
            cmp word[rdi+4], 'ra'
            jne .error1
            cmp byte[rdi+6], 'e'
            jne .error1

            ;stack
            mov rbp, rsp
            lea rsp, [rdi+0x200000-16]

            xor rax, rax
            mov ax, ss
            push rax
            lea rax, [rdi+0x200000-16]
            push rax;RSP

            pushf

            xor rax, rax
            mov ax, cs
            push rax

            mov r14, rdi;offset

            add rdi, 512
            push rdi

            pushf
            push rax
            push rbx
            push rcx
            push rdx
            push r8
            push r9
            push r10
            push r11
            push r12
            push r13
            push r14
            push r15
            push rsi
            push rdi
            push rbp

            mov rsp, rbp

            inc qword[num_of_tasks_running]

            xor rax, rax
            ret

        ;No memory.
        .error0:
            mov rax, 1;Failed.
            ret

        ;Disk error.
        .error2:
            mov rax, 3;Failed.
            ret

        ;Not runable.
        .error1:
            mov rax, rdi
            xor rdx, rdx
            mov rbx, 0x200000
            div rbx
            mov rcx, rax
            call Kill
            mov rax, 2;Failed.
            ret









    ;input rax = info, rcx = adress, rbx = stack, rdx = parent PID
    ;carry cleared if sucsess, set if faild
    New_Task_entry:
        pushf
        push rdi
        push r8
        mov rdi, TASK_MAP-32
        xor r8, r8
        .find_loop:
            cmp r8, 0xffffffffffffffff;Kernel
            je .no_room
            inc r8
            add rdi, 32
            cmp rdi, TASK_MAP+(32*(max_task_num))
            je .no_room
            cmp qword [rdi], 2;killed
            je .find_loop_end
            cmp qword [rdi], 0
            jne .find_loop

        .find_loop_end:
            mov [rdi], rax;Info
            mov [rdi+8], rcx;Adress
            mov [rdi+16], rbx;Stack
            mov [rdi+24], rdx;Parent ID
            sub rdi, TASK_MAP
            pop r8
            pop rdi
            popf
            clc
            ret

        .no_room:
            pop r8
            pop rdi
            popf
            stc
            ret





    ;kill task PID stored in RCX
    Kill:
        pushf
        push rsi

        cmp qword[num_of_tasks_running], 0
        je .Kernel_return
        cmp rcx, 0xffffffffffffffff;Kernel
        je .pass


        lea rsi, [rcx*8]
        lea rsi, [rsi*4]
        lea rsi, [rsi+TASK_MAP]

        ;Remove Page_manager entry.
        mov rax, [rsi+8]
        xor rdx, rdx
        mov rbx, 0x200000
        div rbx
        mov rcx, rax
        call Page_manager_delete

        mov qword[rsi], 0x2
        mov qword[rsi+8], 0x0
        mov qword[rsi+16], 0x0
        mov qword[rsi+24], 0x0

        dec qword[num_of_tasks_running]

        cmp qword[num_of_tasks_running], 0
        je .Kernel_return

        .pass:
            pop rsi
            popf
            ret

        .Kernel_return:
            mov qword [curent_task], 0xffffffffffffffff;Kernel
            mov rsp, kstack
            jmp UI


    ;Update task, PID stored in R9
    ;rax = info, rcx = adress, rbx = stack, rdx = parent PID
    Update:
        pushf
        push rsi

        lea rsi, [r8*8]
        lea rsi, [(rsi*4)+TASK_MAP]

        mov [rsi], rax
        mov [rsi+8], rcx
        mov [rsi+16], rbx
        mov [rsi+24], rdx

        .pass:
            pop rsi
            popf
            ret

    ;Read task, PID stored in R9
    ;rax = info, rcx = adress, rbx = stack, rdx = parent PID
    Read:
        pushf
        push rsi

        lea rsi, [r8*8]
        lea rsi, [(rsi*4)+TASK_MAP]

        mov rax, [rsi]
        mov rcx, [rsi+8]
        mov rbx, [rsi+16]
        mov rdx, [rsi+24]

        .pass:
            pop rsi
            popf
            ret




    ;fixed
    TS:
        pushf
        push rax
        push rbx
        push rcx
        push rdx
        push r8
        push r9
        push r10
        push r11
        push r12
        push r13
        push r14
        push r15
        push rsi
        push rdi
        push rbp


        ;Aling for fxasve
        mov rax, rsp
        push rax
        xor rdx, rdx
        mov rbx, 16
        div rbx
        pop rax
        cmp rdx, 0
        je .no_need_to_aling
            mov rbx, 16
            sub rbx, rdx
            add rax, rbx
        .no_need_to_aling:
        fxsave [rax-512]


        cmp qword [num_of_tasks_running], 0
        je .done

        ;Init check
        cmp qword [TS_first], 0
        je .pass

        mov rsi, [curent_task]
        lea rsi, [(rsi)*8]
        lea rsi, [rsi*4]
        lea rsi, [rsi+TASK_MAP]
        mov [rsi+16], rsp
        jmp .no_pass

        .pass:
            inc qword[TS_first]

        .no_pass:


        ;time for next task
        inc qword[curent_task]
        mov rcx, [curent_task]

        .find_loop:
            lea rsi, [rcx*8]
            lea rsi, [rsi*4]
            lea rsi, [rsi+TASK_MAP]

            cmp rcx, 0xffffffffffffffff;Kernel.
            je .find_loop_end_reached

            inc rcx
            inc qword[curent_task]

            bt qword[rsi], 0
            jc .find_loop_end
            bt qword[rsi], 1;killed
            jc .find_loop


        .find_loop_end_reached:
            xor rcx, rcx
            mov [curent_task], rcx
            jmp .find_loop

        .find_loop_end:
            bt qword[rsi], 3;Waiting.
            jc .find_loop
            dec qword[curent_task]
            mov rsp, [rsi+16]



        .done:
            ;Restore mmx fpu SSE
            mov rax, rsp
            push rax
            xor rdx, rdx
            mov rbx, 16
            div rbx
            pop rax
            cmp rdx, 0
            je .done_no_need_to_aling
                mov rbx, 16
                sub rbx, rdx
                add rax, rbx
            .done_no_need_to_aling:
            fxrstor [rax-512]

            pop rbp
            pop rdi
            pop rsi
            pop r15
            pop r14
            pop r13
            pop r12
            pop r11
            pop r10
            pop r9
            pop r8
            pop rdx
            pop rcx
            pop rbx
            pop rax
            popf
            iretq


