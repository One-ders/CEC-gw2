/* $FrameWorks: , v1.1 2014/04/07 21:44:00 anders Exp $ */

/*
 * Copyright (c) 2014, Anders Franzen.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @(#)led_drv.c
 */
#include "stm32/stm32f407.h"
#include "stm32/devices.h"
#include "sys.h"
#include "led_drv.h"

static struct device_handle my_dh;

/*************************  Led driver ***************************/

static int led_control(struct device_handle *dh, int cmd, void *arg1, int arg2) {
        switch(cmd) {
                case LED_CTRL_STAT:
			if (arg2<4) return -1;
			*((unsigned int *)arg1)=GPIOD->ODR&0xf000;
                        return 0;
                case LED_CTRL_ACTIVATE: {
			if (arg2<4) return -1;
			GPIOD->ODR|=((*((unsigned int *)arg1))&0xf000);
			break;
		}
                case LED_CTRL_DEACTIVATE: {
			if (arg2<4) return -1;
			GPIOD->ODR&=~((*((unsigned int *)arg1))&0xf000);
			break;
		}
                default:
                        return -1;
        }
        return 0;
}

static int led_close(struct device_handle *dh) {
        return 0;
}

static struct device_handle *led_open(void *instance, DRV_CBH cb_handler, void *dum) {
        return &my_dh;
}


static int led_init(void *inst) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; // enable the clock to GPIOD
	GPIOD->MODER |= (1 << 24); // set pin 12 to be general purpose output
	GPIOD->MODER |= (1 << 26); // set pin 13 to be general purpose output
	GPIOD->MODER |= (1 << 28); // set pin 14 to be general purpose output
	GPIOD->MODER |= (1 << 30); // set pin 15 to be general purpose output
	return 0;
};

static int led_start(void *inst) {
	return 0;
}

static struct driver_ops led_drv_ops = {
        led_open,
        led_close,
        led_control,
	led_init,
	led_start,
};

static struct driver led_drv = {
	LED_DRV,
	0,
	&led_drv_ops,
};

void init_led_drv(void) {
	driver_publish(&led_drv);
}

INIT_FUNC(init_led_drv);
