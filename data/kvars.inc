;Filename = SYS_VARS.inc
;Type = part of kernel.
;Description = Kernel KDATA.

;SYS_INT DATA

KDATA:
    .htab cstring "0123456789ABCDEF"
    .main_partion cstring '0/1/'
    .IRQ0x6 cstring 'Floppy IRQ6.\n'
    .Wait_task_switch int_8 0 ;If one do not switch tasks.
    .Root_partion cstring '0/1'
    .Root_mount_point cstring 'Home/'
    .SYS_input_flags int_64 0 ;0 = Keyboard, 1 = COM1.
    .videomode: dq 0 ;1=VGA 2=VESA
    

shell cstring 'Home/BIN/SHELL';Shell file.

;Kernel driver and sys data:
void int_64 0;DO NOT use for storing any data that you don't want DESTROYED!!!
timer int_64 0;Good source of random.
function_key_pause int_8 0

;Paging
;1 = memtest
;0 = normal
PF_handel_methed int_64 0
ammount_of_mem_in_sys int_64 0
;task
curent_task int_64 0xffffffffffffffff
num_of_tasks_running int_64 0;only tasks that are acctev count
TS_first int_64 0
file_name: dq 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

LOGGED_IN int_8 0
STARTED int_8 0

;SYS_info instruct support simaler
supported_extensions int_64 0;1 for yes 0 for no, bit 0 = rdrand support.





;Exeption KDATA.
Error_type int_64 0
Error_code_str cstring '\n\nError code: %x\n'
Stack_Frame_str cstring '\n\nYou system has experensted an error!\n\nHere is the error type: %s.\nHere is the Stack Frame:\n\n      Instruction Pointer(RIP): 0x%x\n      Code Segment(CS): 0x%x\n      64 bit CPU Flags(RFLAGS): %b\n      Stack Pointer(RSP): 0x%x\n      Stack Segment(SS): 0x%x\n\n'







;Sys KDATA.

Started cstring '%s has started!', 0x0a
start_str1 cstring '%s: Installing MVFS drivers (Kernel drivers only) and Mounting 0/1/ as Root...'
start_str2 cstring '%s: Setting up COM1...'
start_str3 cstring '%s: Setting up IDT and PIC...'
start_str5 cstring '%s: Starting kernel...'
start_str6 cstring '%s: Starting mem_test...'
start_str7 cstring '%s: Mapping all memory...'
start_str8 cstring '%s: Making Page Manager Allacotion Table...'
start_str9 cstring '%s: Enabeling SSE...'
start_str10 cstring '%s: Making page tabel for Kernel, and checking for 32 MiB min...'
;start_str11 cstring '%s: Enabeling RTC at IRQ8 and Enabling NMI...'
start_str12 cstring '%s: Setting up GDT...'
start_str13 cstring '%s: Setting up floppy DMA...'
start_str14 cstring '%s: Initiating kernel heap...'
start_str15 cstring '%s: Press 1 if you are using COM1 as your terminal or wait if using keyboard and screen.'
start_str_memory_found cstring 'Found: %d MiB of memory.', 0x0a
start_str_DONE cstring 'Done!', 0x0a
start_str_failed cstring 'Failed!', 0x0a

;settings cstring 'Home/SYS/SETTINGS.S'

OS_kernel cstring 'MOS Kernel 64bit v0.02'
emblem_flounder cstring '         __________ ', 0x0a, '        /          \ ', 0x0a, '\      /            \ ', 0x0a, ' \    /              \ ', 0x0a, '  \--/               O \ ', 0x0a, '                       | ', 0x0a, '  /--\               O / ', 0x0a, ' /    \              / ', 0x0a, '/      \            / ', 0x0a, '        \__________/ ', 0x0a
OS_help_str4 cstring 'MOS 0.2 (Flounder)'

;Errors
;Startup.
not_enugh_memory_for_os_str cstring '\n\nThere is not enughe memory in this system to continue.\n\n'

;Logging error.
;loggin_error_str cstring 'Your settings.s file has been corrupted.\nPress any key to login.'


;Loggin
;pswd cstring 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
;pswd_end cstring 0
welcome cstring 'Welcome to MOS 64 bit v0.2 (Flounder).\n'
;epswd: db 'Enter you password: ', 0

