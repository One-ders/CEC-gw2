
#include <config.h>
#include "jz4750d.h"
#include <mtd_nand.h>


/*
 * NAND flash definitions
 */

#define NAND_DATAPORT   0xb8000000
#define NAND_ADDRPORT   0xb8010000
#define NAND_COMMPORT   0xb8008000

#define ECC_BLOCK       512
#define ECC_POS         6
#define PAR_SIZE        9

#define __nand_enable()         (REG_NFCSR |= NFCSR_NFE1 | NFCSR_NFCE1)
#define __nand_disable()        (REG_NFCSR &= ~(NFCSR_NFCE1))
#define __nand_ecc_rs_encoding() \
        (REG_NFECCR = NFECCR_ECCE | NFECCR_ERST | NFECCR_RSE | NFECCR_ENCE)
#define __nand_ecc_rs_decoding() \
        (REG_NFECCR = NFECCR_ECCE | NFECCR_ERST | NFECCR_RSE)
#define __nand_ecc_disable()    (REG_NFECCR &= ~NFECCR_ECCE)
#define __nand_ecc_encode_sync() while (!(REG_NFINTS & NFINTS_ENCF))
#define __nand_ecc_decode_sync() while (!(REG_NFINTS & NFINTS_DECF))

#define PORT_A 0
#define PORT_B 1
#define PORT_C 2
#define PORT_D 3
#define PORT_E 4
#define PORT_F 5

static inline void __nand_dev_ready(void)
{
        unsigned int timeout = 10000;
        while ((REG_GPIO_PXPIN(PORT_C) & 0x40000000) && timeout--);
        while (!(REG_GPIO_PXPIN(PORT_C) & 0x40000000));
}

#define __nand_cmd(n)           (REG8(NAND_COMMPORT) = (n))
#define __nand_addr(n)          (REG8(NAND_ADDRPORT) = (n))
#define __nand_data8()          REG8(NAND_DATAPORT)
#define __nand_data16()         REG16(NAND_DATAPORT)

/*
 * NAND flash parameters
 */
static int bus_width = 8;
static int page_size = 2048;
static int oob_size = 64;
static int ecc_count = 4;
static int row_cycle = 3;
static int page_per_block = 64;
static int bad_block_pos = 0;
static int block_size = 131072;

static unsigned char oob_buf[128] = {0};

/*
 * External routines
 */
extern void serial_puts(const char *s);

/*
 * NAND flash routines
 */

static inline void nand_read_buf16(void *buf, int count) {
	int i;
	unsigned short int *p = (unsigned short int *)buf;

	for (i = 0; i < count; i += 2) {
		*p++ = __nand_data16();
	}
}

static inline void nand_read_buf8(void *buf, int count) {
	int i;
	unsigned char *p = (unsigned char *)buf;

	for (i = 0; i < count; i++) {
		*p++ = __nand_data8();
	}
}

static inline void nand_read_buf(void *buf, int count, int bw) {
	if (bw == 8) {
		nand_read_buf8(buf, count);
	} else {
		nand_read_buf16(buf, count);
	}
}

/* Correct 1~9-bit errors in 512-bytes data */
static void rs_correct(unsigned char *dat, int idx, int mask) {
	int i;

	idx--;

	i = idx + (idx >> 3);
	if (i >= 512) return;

	mask <<= (idx & 0x7);

	dat[i] ^= mask & 0xff;
	if (i < 511) dat[i+1] ^= (mask >> 8) & 0xff;
}

static int nand_read_oob(int page_addr, unsigned char *buf, int size) {
	int col_addr;

	if (page_size == 2048) col_addr = 2048;
	else col_addr = 0;

	if (page_size == 2048) {
		/* Send READ0 command */
		__nand_cmd(NAND_CMD_READ0);
	} else {
		/* Send READOOB command */
		__nand_cmd(NAND_CMD_READOOB);
	}

	/* Send column address */
	__nand_addr(col_addr & 0xff);
	if (page_size == 2048) {
		__nand_addr((col_addr >> 8) & 0xff);
	}

	/* Send page address */
	__nand_addr(page_addr & 0xff);
	__nand_addr((page_addr >> 8) & 0xff);
	if (row_cycle == 3) {
		__nand_addr((page_addr >> 16) & 0xff);
	}

	/* Send READSTART command for 2048 ps NAND */
	if (page_size == 2048) {
		__nand_cmd(NAND_CMD_READSTART);
	}

	/* Wait for device ready */
	__nand_dev_ready();

	/* Read oob data */
	nand_read_buf(buf, size, bus_width);

	return 0;
}

int nand_read_page(int block, int page, unsigned char *dst, unsigned char *oobbuf) {
	int page_addr = page + block * page_per_block;
	unsigned char *databuf = dst, *tmpbuf;
	int i, j;

	/*
	 * Read oob data
	 */
	nand_read_oob(page_addr, oobbuf, oob_size);

	/*
	 * Read page data
	 */

	/* Send READ0 command */
	__nand_cmd(NAND_CMD_READ0);

	/* Send column address */
	__nand_addr(0);
	if (page_size == 2048) __nand_addr(0);

	/* Send page address */
	__nand_addr(page_addr & 0xff);
	__nand_addr((page_addr >> 8) & 0xff);
	if (row_cycle == 3) __nand_addr((page_addr >> 16) & 0xff);

	/* Send READSTART command for 2048 ps NAND */
	if (page_size == 2048) __nand_cmd(NAND_CMD_READSTART);

	/* Wait for device ready */
	__nand_dev_ready();

	/* Read page data */
	tmpbuf = databuf;

	for (i = 0; i < ecc_count; i++) {
		volatile unsigned char *paraddr = 
				(volatile unsigned char *)&(REG_NFPAR0);
		unsigned int stat;

		/* Enable RS decoding */
		REG_NFINTS = 0x0;
		__nand_ecc_rs_decoding();

		/* Read data */
		nand_read_buf((void *)tmpbuf, ECC_BLOCK, bus_width);

		/* Set PAR values */
		for (j = 0; j < PAR_SIZE; j++) {
			*paraddr++ = oobbuf[ECC_POS + i*PAR_SIZE + j];
		}

		/* Set PRDY */
		REG_NFECCR |= NFECCR_PRDY;

		/* Wait for completion */
		__nand_ecc_decode_sync();

		/* Disable decoding */
                __nand_ecc_disable();

		/* Check result of decoding */
		stat = REG_NFINTS;
		if (stat & NFINTS_ERR) {
			/* Error occurred */
			if (stat & NFINTS_UNCOR) {
				/* Uncorrectable error occurred */
			} else {
				unsigned int errcnt, index, mask;

				errcnt = (stat & NFINTS_ERRC_MASK) >> NFINTS_ERRC_SHIFT;
				switch (errcnt) {
					case 4:
						index = (REG_NFERR3 & NFERR_INDEX_MASK) >> NFERR_INDEX_SHIFT;
						mask = (REG_NFERR3 & NFERR_MASK_MASK) >> NFERR_MASK_SHIFT;
						rs_correct(tmpbuf, index, mask);
						/* FALL-THROUGH */
					case 3:
						index = (REG_NFERR2 & NFERR_INDEX_MASK) >> NFERR_INDEX_SHIFT;
						mask = (REG_NFERR2 & NFERR_MASK_MASK) >> NFERR_MASK_SHIFT;
						rs_correct(tmpbuf, index, mask);
						/* FALL-THROUGH */
					case 2:
						index = (REG_NFERR1 & NFERR_INDEX_MASK) >> NFERR_INDEX_SHIFT;
						mask = (REG_NFERR1 & NFERR_MASK_MASK) >> NFERR_MASK_SHIFT;
						rs_correct(tmpbuf, index, mask);
						/* FALL-THROUGH */
					case 1:
						index = (REG_NFERR0 & NFERR_INDEX_MASK) >> NFERR_INDEX_SHIFT;
						mask = (REG_NFERR0 & NFERR_MASK_MASK) >> NFERR_MASK_SHIFT;
						rs_correct(tmpbuf, index, mask);
						break;
					default:
						break;
				}
			}
		}

		tmpbuf += ECC_BLOCK;
	}

	return 0;
}

#ifndef CFG_NAND_BADBLOCK_PAGE
#define CFG_NAND_BADBLOCK_PAGE 0 /* NAND bad block was marked at this page in a block, starting from 0 */
#endif

void nand_load(int offs, int size, unsigned char *dst) {
	int block;
	int pagecopy_count;
	int page_inblock;
	int pages;

	__nand_enable();

	/*
	 * offs has to be aligned to a block address!
	 */
	block = offs / block_size;
	pagecopy_count = 0;
	pages=((size-1)/page_size)+1;
	page_inblock=(offs/page_size)%page_per_block;

	while (pagecopy_count < pages) {

		/* New block is required to check the bad block flag */
		nand_read_oob(block * page_per_block + CFG_NAND_BADBLOCK_PAGE, oob_buf, oob_size);

		if (oob_buf[bad_block_pos] != 0xff) {
			block++;

			/* Skip bad block */
			continue;
		}

		for (;page_inblock < page_per_block; page_inblock++) {

			/* Load this page to dst, do the ECC */
			nand_read_page(block, page_inblock, dst, oob_buf);

			dst += page_size;
			pagecopy_count++;
			if (pagecopy_count>=pages) break;
		}

		page_inblock=0;
		block++;
	}

	__nand_disable();
}
