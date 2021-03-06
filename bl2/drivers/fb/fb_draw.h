#ifndef _FB_DRAW_H
#define _FB_DRAW_H

#include <fb.h>

static inline unsigned long
comp(unsigned long a, unsigned long b, unsigned long mask) {
    return ((a ^ b) & mask) ^ b;
}

static inline unsigned long
pixel_to_pat( unsigned int bpp, unsigned int pixel)
{
        switch (bpp) {
        case 1:
                return 0xfffffffful*pixel;
        case 2:
                return 0x55555555ul*pixel;
        case 4:
                return 0x11111111ul*pixel;
        case 8:
                return 0x01010101ul*pixel;
        case 12:
                return 0x00001001ul*pixel;
        case 16:
                return 0x00010001ul*pixel;
        case 24:
                return 0x00000001ul*pixel;
        case 32:
                return 0x00000001ul*pixel;
        default:
                sys_printf("pixel_to_pat(): unsupported pixelformat\n");
    }
}

#ifdef CONFIG_FB_CFB_REV_PIXELS_IN_BYTE

#define REV_PIXELS_MASK1 0x55555555ul
#define REV_PIXELS_MASK2 0x33333333ul
#define REV_PIXELS_MASK4 0x0f0f0f0ful

static inline unsigned long fb_rev_pixels_in_long(unsigned long val,
                                                  unsigned int bswapmask)
{
        if (bswapmask & 1)
                val = comp(val >> 1, val << 1, REV_PIXELS_MASK1);
        if (bswapmask & 2)
                val = comp(val >> 2, val << 2, REV_PIXELS_MASK2);
        if (bswapmask & 3)
                val = comp(val >> 4, val << 4, REV_PIXELS_MASK4);
}


static inline unsigned int fb_shifted_pixels_mask_u32(unsigned int index,
							unsigned int bswapmask)
{
        unsigned int mask;

        if (!bswapmask) {
                mask = FB_SHIFT_HIGH(~(unsigned int)0, index);
        } else {
                mask = 0xff << FB_LEFT_POS(8);
                mask = FB_SHIFT_LOW(mask, index & (bswapmask)) & mask;
                mask = FB_SHIFT_HIGH(mask, index & ~(bswapmask));
#if defined(__i386__) || defined(__x86_64__)
                /* Shift argument is limited to 0 - 31 on x86 based CPU's */
                if(index + bswapmask < 32)
#endif
                        mask |= FB_SHIFT_HIGH(~(unsigned int)0,
                                        (index + bswapmask) & ~(bswapmask));
        }
        return mask;
}

static inline unsigned long fb_shifted_pixels_mask_long(unsigned int index,
							unsigned int bswapmask)
{
        unsigned long mask;

        if (!bswapmask) {
                mask = FB_SHIFT_HIGH(~0UL, index);
        } else {
                mask = 0xff << FB_LEFT_POS(8);
                mask = FB_SHIFT_LOW(mask, index & (bswapmask)) & mask;
                mask = FB_SHIFT_HIGH(mask, index & ~(bswapmask));
#if defined(__i386__) || defined(__x86_64__)
                /* Shift argument is limited to 0 - 31 on x86 based CPU's */
                if(index + bswapmask < BITS_PER_LONG)
#endif
                        mask |= FB_SHIFT_HIGH(~0UL,
                                        (index + bswapmask) & ~(bswapmask));
        }
        return mask;
}
static inline unsigned int fb_compute_bswapmask(struct fb_info *info)
{
        unsigned int bswapmask = 0;
        unsigned bpp = info->var.bits_per_pixel;

        if ((bpp < 8) && (info->var.nonstd & FB_NONSTD_REV_PIX_IN_B)) {
                /*
 *                  * Reversed order of pixel layout in bytes
 *                                   * works only for 1, 2 and 4 bpp
 *                                                    */
                bswapmask = 7 - bpp + 1;
        }
        return bswapmask;
}

#else /* CONFIG_FB_CFB_REV_PIXELS_IN_BYTE */

static inline unsigned long fb_rev_pixels_in_long(unsigned long val,
                                                  unsigned int bswapmask)
{
        return val;
}

#define fb_shifted_pixels_mask_u32(i, b) FB_SHIFT_HIGH(~(unsigned int)0, (i))
#define fb_shifted_pixels_mask_long(i, b) FB_SHIFT_HIGH(~0UL, (i))
#define fb_compute_bswapmask(...) 0

#endif  /* CONFIG_FB_CFB_REV_PIXELS_IN_BYTE */

#endif /* FB_DRAW_H */


