/**
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <stddef.h>
#include <assert.h>
#include "syscfg/syscfg.h"
#include "sysinit/sysinit.h"
#include "mcu/samd21.h"
#include "bsp/bsp.h"
#include <bsp/bsp_sysid.h>
#include <hal/hal_spi.h>
#if MYNEWT_VAL(I2C_5)
#include <hal/hal_i2c.h>
#endif
#include <hal/hal_bsp.h>
#include "hal/hal_flash.h"
#include <mcu/hal_spi.h>
#include <mcu/hal_i2c.h>

/*
 * hw/mcu/atmel/samd21xx/src/sam0/drivers/sercom/usart/usart.h
 */
#include <usart.h>
#include <os/os_dev.h>
#include <uart/uart.h>
#include <uart_hal/uart_hal.h>
#include <mcu/hal_uart.h>

static struct uart_dev hal_uart0;

const struct hal_flash *
hal_bsp_flash_dev(uint8_t id)
{
    /*
     * Internal flash mapped to id 0.
     */
    if (id != 0) {
        return NULL;
    }

    return &samd21_flash_dev;
}

/*
 * What memory to include in coredump.
 */
static const struct hal_bsp_mem_dump dump_cfg[] = {
    [0] = {
        .hbmd_start = &_ram_start,
        .hbmd_size = RAM_SIZE
    }
};

const struct hal_bsp_mem_dump *
hal_bsp_core_dump(int *area_cnt)
{
    *area_cnt = sizeof(dump_cfg) / sizeof(dump_cfg[0]);
    return dump_cfg;
}

#if MYNEWT_VAL(SPI_0)
/* configure the SPI port for arduino external spi */
struct samd21_spi_config icsp_spi_config = {
    .dipo = 0,  /* sends MISO to PAD 0 */
    .dopo = 1,  /* send CLK to PAD 3 and MOSI to PAD 2 */
    .pad0_pinmux = PINMUX_PA12D_SERCOM4_PAD0,       /* MISO */
    .pad3_pinmux = PINMUX_PB11D_SERCOM4_PAD3,       /* SCK */
    .pad2_pinmux = PINMUX_PB10D_SERCOM4_PAD2,       /* MOSI */
};
#endif

#if MYNEWT_VAL(SPI_1)
/* NOTE using this will overwrite the debug interface */
struct samd21_spi_config alt_spi_config = {
    .dipo = 3,  /* sends MISO to PAD 3 */
    .dopo = 0,  /* send CLK to PAD 1 and MOSI to PAD 0 */
    .pad0_pinmux = PINMUX_PA04D_SERCOM0_PAD0,       /* MOSI */
    .pad1_pinmux = PINMUX_PA05D_SERCOM0_PAD1,       /* SCK */
    .pad2_pinmux = PINMUX_PA06D_SERCOM0_PAD2,       /* SCK */
    .pad3_pinmux = PINMUX_PA07D_SERCOM0_PAD3,       /* MISO */
};
#endif

#if MYNEWT_VAL(I2C_5)
struct samd21_i2c_config i2c_config = {
    .pad0_pinmux = PINMUX_PA22D_SERCOM5_PAD0,
    .pad1_pinmux = PINMUX_PA23D_SERCOM5_PAD1,
};
#endif

#if MYNEWT_VAL(UART_0)
static const struct samd21_uart_config uart_cfgs[] = {
    [0] = {
        .suc_sercom = SERCOM2,
        .suc_mux_setting = USART_RX_3_TX_2_XCK_3,
        .suc_generator_source = GCLK_GENERATOR_0,
        .suc_sample_rate = USART_SAMPLE_RATE_16X_ARITHMETIC,
        .suc_sample_adjustment = USART_SAMPLE_ADJUSTMENT_7_8_9,
        .suc_pad0 = 0,
        .suc_pad1 = 0,
        .suc_pad2 = PINMUX_PA10D_SERCOM2_PAD2,
        .suc_pad3 = PINMUX_PA11D_SERCOM2_PAD3
    }
};
#endif


void
hal_bsp_init(void)
{
    int rc;

#if MYNEWT_VAL(UART_0)
    rc = os_dev_create((struct os_dev *) &hal_uart0, CONSOLE_UART,
      OS_DEV_INIT_PRIMARY, 0, uart_hal_init, (void *)&uart_cfgs[0]);
    SYSINIT_PANIC_ASSERT(rc == 0);
#endif

#if MYNEWT_VAL(SPI_0)
    rc = hal_spi_init(ARDUINO_ZERO_SPI_ICSP, &icsp_spi_config,
                      MYNEWT_VAL(SPI_0_TYPE));
    SYSINIT_PANIC_ASSERT(rc == 0);
#endif

#if MYNEWT_VAL(SPI_1)
    rc = hal_spi_init(ARDUINO_ZERO_SPI_ALT, &alt_spi_config,
                      MYNEWT_VAL(SPI_1_TYPE));
    SYSINIT_PANIC_ASSERT(rc == 0);
#endif

#if MYNEWT_VAL(I2C_5)
    rc = hal_i2c_init(5, &i2c_config);
    SYSINIT_PANIC_ASSERT(rc == 0);
#endif
}
