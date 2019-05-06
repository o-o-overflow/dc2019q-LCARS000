PHDRS
{
    code_seg PT_LOAD;
    data_seg PT_LOAD;
}

SECTIONS
{
    .text 0x10000000 : { *(.app_start*) *(.text* .rodata*) *(.got*) } : code_seg
    .data 0x20000000(NOLOAD) :  { *(.data*) *(.bss); } : data_seg
}
