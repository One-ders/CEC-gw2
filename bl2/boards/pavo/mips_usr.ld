
OUTPUT_FORMAT("elf32-tradlittlemips", "elf32-tradlittlemips", "elf32-tradlittlemips")

OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
        . = 0x0040000;

        . = ALIGN(4);
        .text       :
        {
          *(.text .text.*)
        }

        . = 0x10000000;
        . = ALIGN(4);
        .rodata  : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
	.rodata1 : { *(.rodata1) }
	/* thread local storage */
	.tdata	: { *(.tdata .tdata.* .gnu.linkonce.td.*) }
	.tbss	: { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }

	.sdata2	: {
		*(.sdata2 .sdata2.* .gnu.linkonce.s2.*)
	}

	.sbbs2 : { *(.sbss2 .sbss2.* .gnu.linkonce.sb2.*) }

        . = ALIGN(4);
        .data  : { *(.data .data.*) }
        .data1  : { *(.data1) }

        . = .;

        _gp = ALIGN(16);

        .got  : { *(.got.plt) *(.got) }

        .sdata  : { *(.sdata .sdata.* .gnu.linkonce.s.*) }

	.lit8           : { *(.lit8) }
	.lit4           : { *(.lit4) }
	_edata = .; PROVIDE (edata = .);

        . = ALIGN(4);

	__bss_start__ = .;
        .sbss  : { 
		*(.dynsbss) 
		*(.sbss .sbss.* .gnu.linkonce.sb.*)
        	*(.scommon)
	}
        .bss  : { 
		*(.dynbss)
		*(.bss .bss.* .gnu.linkonce.b.*) 
		*(COMMON)
		. = ALIGN(. != 0 ? 32 / 8 : 1);
	}

	. = ALIGN(32 / 8);
	. = ALIGN(32 / 8);
	__bss_end__ = .;
	_end = .; PROVIDE (end = .);
}

