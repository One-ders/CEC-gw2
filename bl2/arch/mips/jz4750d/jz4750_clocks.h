#ifndef JZ_EXTAL
#define JZ_EXTAL                24000000   /*  */
#endif
#ifndef JZ_EXTAL2
#define JZ_EXTAL2               32768     /* 32.768 KHz */
#endif


/*
 * JZ4750D clocks structure
 */
typedef struct {
	unsigned int cclk;      /* CPU clock */
	unsigned int hclk;      /* System bus clock */
	unsigned int pclk;      /* Peripheral bus clock */
	unsigned int mclk;      /* Flash/SRAM/SDRAM clock */
	unsigned int h1clk;     /*  AHB1 clock */
	unsigned int pixclk;    /* LCD pixel clock */
	unsigned int i2sclk;    /* AIC module clock */
	unsigned int usbclk;    /* USB module clock */
	unsigned int mscclk;    /* MSC module clock */
	unsigned int extalclk;  /* EXTAL clock for UART,I2C,SSI,TCU,USB-PHY */
	unsigned int rtcclk;    /* RTC clock for CPM,INTC,RTC,TCU,WDT */
} jz_clocks_t;

extern jz_clocks_t jz_clocks;

/* PLL output frequency */
static __inline__ unsigned int __cpm_get_pllout(void) {
#if defined(CONFIG_FPGA)
	return JZ_EXTAL/CFG_DIV;
#else
	unsigned long m, n, no, pllout;
	unsigned long cppcr = REG_CPM_CPPCR;
	unsigned long od[4] = {1, 2, 2, 4};
	if ((cppcr & CPM_CPPCR_PLLEN) && !(cppcr & CPM_CPPCR_PLLBP)) {
		m = __cpm_get_pllm() + 2;
		n = __cpm_get_plln() + 2;
		no = od[__cpm_get_pllod()];
		pllout = ((JZ_EXTAL) / (n * no)) * m;
	} else {
		pllout = JZ_EXTAL;
	}
	return pllout;
#endif
}

/* PLL output frequency for MSC/I2S/LCD/USB */
static __inline__ unsigned int __cpm_get_pllout2(void) {
#if defined(CONFIG_FPGA)
	return JZ_EXTAL/CFG_DIV;
#else
	if (REG_CPM_CPCCR & CPM_CPCCR_PCS)
		return __cpm_get_pllout();
	else
		return __cpm_get_pllout()/2;
#endif
}

/* CPU core clock */
static __inline__ unsigned int __cpm_get_cclk(void) {

#if defined(CONFIG_FGPA)
	return JZ_EXTAL;
#else
	int div[] = {1, 2, 3, 4, 6, 8, 12, 16, 24, 32};
	return __cpm_get_pllout() / div[__cpm_get_cdiv()];
#endif
}

/* AHB system bus clock */
static __inline__ unsigned int __cpm_get_hclk(void) {
#if defined(CONFIG_FPGA)
	return JZ_EXTAL/CFG_DIV;
#else
	int div[] = {1, 2, 3, 4, 6, 8, 12, 16, 24, 32};

	return __cpm_get_pllout() / div[__cpm_get_hdiv()];
#endif
}

/* Memory bus clock */
static __inline__ unsigned int __cpm_get_mclk(void) {
#if defined(CONFIG_FPGA)
	return JZ_EXTAL/CFG_DIV;
#else
	int div[] = {1, 2, 3, 4, 6, 8, 12, 16, 24, 32};

	return __cpm_get_pllout() / div[__cpm_get_mdiv()];
#endif
}

/* APB peripheral bus clock */
static __inline__ unsigned int __cpm_get_pclk(void) {
#if defined(CONFIG_FPGA)
	return JZ_EXTAL/CFG_DIV;
#else
	int div[] = {1, 2, 3, 4, 6, 8, 12, 16, 24, 32};

	return __cpm_get_pllout() / div[__cpm_get_pdiv()];
#endif
}

/* LCDC module clock */
static __inline__ unsigned int __cpm_get_h1clk(void) {
	return __cpm_get_pllout2() / (__cpm_get_h1div() + 1);
}

/* LCD pixel clock */
static __inline__ unsigned int __cpm_get_pixclk(void) {
	return __cpm_get_pllout2() / (__cpm_get_pixdiv() + 1);
}

/* I2S clock */
static __inline__ unsigned int __cpm_get_i2sclk(void) {
	if (REG_CPM_CPCCR & CPM_CPCCR_I2CS) {
		return __cpm_get_pllout2() / (__cpm_get_i2sdiv() + 1);
	} else {
		return JZ_EXTAL;
	}
}

/* USB clock */
static __inline__ unsigned int __cpm_get_usbclk(void) {
	if (REG_CPM_CPCCR & CPM_CPCCR_UCS) {
		return __cpm_get_pllout2() / (__cpm_get_udiv() + 1);
	} else {
		return JZ_EXTAL;
	}
}

/* 
 * MSC clock
 * @n: the index of MMC/SD controller 
 */
static __inline__ unsigned int __cpm_get_mscclk(int n) {
	return __cpm_get_pllout2() / (__cpm_get_mscdiv(n) + 1);
}

/* EXTAL clock */
static __inline__ unsigned int __cpm_get_extalclk0(void) {
	return JZ_EXTAL;
}

/* EXTAL clock for UART,I2C,SSI,TCU,USB-PHY */
static __inline__ unsigned int __cpm_get_extalclk(void) {
#if defined(CONFIG_FPGA)
	return __cpm_get_pllout();
#else
	if (REG_CPM_CPCCR & CPM_CPCCR_ECS)
		return __cpm_get_extalclk0()/2;
	else
		return __cpm_get_extalclk0();
#endif

}

/* RTC clock for CPM,INTC,RTC,TCU,WDT */
static __inline__ unsigned int __cpm_get_rtcclk(void) {
	return JZ_EXTAL2;
}

/*
 * Output 24MHz for SD and 16MHz for MMC.
 * @n: the index of MMC/SD controller
 */
static inline void __cpm_select_msc_clk(int n, int sd) {
	unsigned int pllout2 = __cpm_get_pllout2();
	unsigned int div = 0;

	if (sd) {
		div = pllout2 / 24000000;
	} else {
		div = pllout2 / 16000000;
	}

	REG_CPM_MSCCDR(n) = div - 1;
	REG_CPM_CPCCR |= CPM_CPCCR_CE;
}


/*
 * Output 48MHz for high speed card.
 */
static inline void __cpm_select_msc_clk_high(int n, int sd) {
	unsigned int pllout2 = __cpm_get_pllout2();
	unsigned int div = 0;

	div = pllout2 / 48000000;

	REG_CPM_MSCCDR(n) = div - 1;
	REG_CPM_CPCCR |= CPM_CPCCR_CE;
}
