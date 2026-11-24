ENTRY(_start)

/* The Milestone 1 loader maps every PT_LOAD page into a page of its own, so two
 * segments must never share a page. Without PHDRS, ld.lld splits code and
 * read-only data into separate segments inside the first page and the loader
 * rejects the image (EXEC-0x0120). Emit one RWX segment instead. */
PHDRS
{
    image PT_LOAD FLAGS(7);
}

SECTIONS
{
    . = 0x400000;
    .text : {
        *(.text*)
    } :image
    .rodata : {
        *(.rodata*)
    } :image
    .data : {
        *(.data*)
    } :image
    .bss : {
        *(.bss*)
        *(COMMON)
    } :image
    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame*)
    }
}
