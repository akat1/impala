# 
#  ImpalaOS
#   http://trzask.int.pl/impala/trac/
# 
#  $Id$
#  

.global kernel_entrypoint

.set ALIGN,    1<<0       
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.set STACKSIZE, 0x4000
.comm stack, STACKSIZE, 32

kernel_entrypoint:
    movl $(stack + STACKSIZE), %esp
    push %eax
    push %ebx
    call init_x86
    call kmain
    hlt

