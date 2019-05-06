PHDRS
{
    code_seg PT_LOAD;
    data_seg PT_LOAD;
}

SECTIONS
{
    .text 0x100000: { *(.app_start*) *(.text* .rodata*) *(.got*) } : code_seg
    .data 0x200000(NOLOAD) :  { *(.data*) *(.bss); } : data_seg
}
