.set MULTIBOOT_MAGIC,         0x1badb002
.set MULTIBOOT_PAGE_ALIGN,    0x1
.set MULTIBOOT_MEMORY_INFO,   0x2
.set MULTIBOOT_VIDEO_MODE,    0x0  /* 4 for graphics */
.set multiboot_flags,         MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO /*MULTIBOOT_VIDEO_MODE*/
.set multiboot_checksum,      -(MULTIBOOT_MAGIC + multiboot_flags)

.section .multiboot
    .align 4
    .long MULTIBOOT_MAGIC
    .long multiboot_flags
    .long multiboot_checksum

    .long 0x00000000    /* header_addr */
    .long 0x00000000    /* load_addr */
    .long 0x00000000    /* load_end_addr */
    .long 0x00000000    /* bss_end_addr */
    .long 0x00000000    /* entry_addr */

    ;.long 0x00000000    /* mode_type */
    ;.long 1440          /* width */
    ;.long 900           /* height */
    ;.long 32            /* depth */


.section .text
.extern kernelMain
.extern callConstructors
.global loader
.global shutdown
.type shutdown, @function

.extern multiboot_info_ptr
.type multiboot_info_ptr, @object

shutdown:
    mov $0x5301, %ax
    xor %bx, %bx
    int $0x15

    /* Try to set apm version (to 1.2). */
    mov $0x530e, %ax
    xor %bx, %bx
    mov $0x0102, %cx
    int $0x15

    /* Turn off the system. */
    mov $0x5307, %ax
    mov $0x0001, %bx
    mov $0x0003, %cx
    int $0x15

    ret

loader:
    cli
    cld

    pushl %eax
    pushl %ebx
    movl %ebx, multiboot_info_ptr

    mov $kernel_stack, %esp
    call callConstructors
    call kernelMain


_stop:
    cli
    hlt
    jmp _stop


.section .bss
.space 2*1024*1024; # 2 MiB
kernel_stack:
