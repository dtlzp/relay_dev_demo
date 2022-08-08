 
/**
 * Copyright (c) 2022 Shenzhen Dingtian Technologies Co.,Ltd All rights reserved.
 * website: www.dingtian-tech.com
 * author: lzp<lzp@dingtian-tech.com>
 * sales: stephen liu<stephen@dingtian-tech.com>
 * date: 2022/6/20
 * file: dt_gpio.c
 * function: Input,Relay,Factory button/led gpio demo source code
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "sdkconfig.h"

#include "../../common/dt_device_tree.h"

#include "dt_mqtt.h"
#include "dt_gpio.h"

#define TAG "DT-GPIO"

extern ddt_t g_dt;

dt_io_t g_io;

void factory_led_init(void)
{
    gpio_config_t io_conf = {0};

    DT_XLOGE("gpio_factory_run_led:%d .........factory run led", g_dt.gpio_factory_run_led);
    if( GPIO_NULL == g_dt.gpio_factory_run_led )
    {
        DT_XLOGW("gpio_factory_run_led:%d NULL", g_dt.gpio_factory_run_led);
        return;
    }

    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ull<<g_dt.gpio_factory_run_led);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    gpio_set_level(g_dt.gpio_factory_run_led, !g_dt.factory_run_led_level);
}

void factory_led_action(b8_t on)
{
    DT_XLOGE("gpio_factory_run_led:%d", g_dt.gpio_factory_run_led);
    if( GPIO_NULL == g_dt.gpio_factory_run_led )
    {
        DT_XLOGW("gpio_factory_run_led:%d NULL", g_dt.gpio_factory_run_led);
        return;
    }

    gpio_set_level(g_dt.gpio_factory_run_led, (0 == g_dt.factory_run_led_level) ? (!on):(on));
}

u32_t time_dec(u32_t now, u32_t before)
{
    u32_t tmp_now = now;
    u32_t tmp_before = before;
    u32_t result;

    if( tmp_now < tmp_before )
    {
        tmp_now += (U32MAX - tmp_before);
        tmp_before = 0;
    }
    result = (tmp_now - tmp_before);

    return result;
}

#define cur_time_ms()   (esp_timer_get_time()/MS2US)

void vpio_tx_byte(u8_t byte)
{
    s32_t i;

    for(i=0; i<8; i++)
    {
        HC595165_SDI( (byte & 0x01) ? DT_HIGH:DT_LOW );

        HC595165_CLK(DT_LOW);
        HC595165_CLK(DT_HIGH);

        byte >>= 1;
    }
}

u8_t vpio_rx_byte(void)
{
    s32_t i;
    u8_t byte = 0;

    for(i=0; i<8; i++)
    {
        byte >>= 1;
        if( HC595165_Q7() )
            byte |= 0x80;

        HC595165_CLK(DT_LOW);
        HC595165_CLK(DT_HIGH);
    }

    return byte;
}

u8_t vpio_tx_rx_byte(u8_t w_byte)
{
    s32_t i;
    u8_t byte = 0;

    for(i=0; i<8; i++)
    {
        HC595165_SDI( (w_byte & 0x01) ? DT_HIGH:DT_LOW );
        w_byte >>= 1;

        byte >>= 1;
        if( HC595165_Q7() )
            byte |= 0x80;

        HC595165_CLK(DT_LOW);
        HC595165_CLK(DT_HIGH);
    }

    return byte;
}

void vpio_refresh(u8_t *igpio, u8_t *ogpio, u8_t i_bytes, u8_t o_bytes)
{
    s32_t i;

    HC595165_RCK(DT_LOW);
    HC595165_PL(DT_HIGH);
    if( i_bytes > 0 )
    {
        for(i=0; i<o_bytes; i++)
        {
            igpio[i] = vpio_tx_rx_byte(ogpio[i]);
        }
    }else
    {
        for(i=0; i<o_bytes; i++)
        {
            vpio_tx_byte(ogpio[i]);
        }
    }
    HC595165_RCK(DT_HIGH);
    HC595165_SDI(DT_HIGH);
}


void vpio_init(dt_io_t *iox)
{
    u8 *igpio;
    u8 *ogpio;
    u8 i_bytes;
    u8 o_bytes;

//    esp_err_t err;
    gpio_config_t io_conf = {0};
    s32_t i;
    uint64_t sp_i_pin_bit_mask = 0;
    uint64_t sp_o_pin_bit_mask = 0;
//    uint64_t i_irq_pin_bit_mask = 0;
//    uint64_t o_pin_bit_mask = 0;
//    u8_t gpio, bit;

    switch(DT_R_CNT)
    {
    default:
        return;
        break;

    case 8:
        i_bytes = VPIO_IB8;
        igpio = &(iox->t_xpio_bits[VPIO_I_BYTE_START]);
        ogpio = &(iox->xpio_bits[VPIO_O_BYTE_START]);
        o_bytes = VPIO_OB8;
        break;

    case 16:
        igpio = &(iox->t_xpio_bits[VPIO_I_BYTE_START]);
        ogpio = &(iox->xpio_bits[VPIO_O_BYTE_START]);
        i_bytes = VPIO_IB16;
        o_bytes = VPIO_OB16;
        break;
    case 24:
        igpio = &(iox->t_xpio_bits[VPIO_I_BYTE_START]);
        ogpio = &(iox->xpio_bits[VPIO_O_BYTE_START]);
        i_bytes = VPIO_IB24;
        o_bytes = VPIO_OB24;
        break;
    case 32:
        igpio = &(iox->t_xpio_bits[VPIO_I_BYTE_START]);
        ogpio = &(iox->xpio_bits[VPIO_O_BYTE_START]);
        i_bytes = VPIO_IB32;
        o_bytes = VPIO_OB32;
        break;
    }

    sp_i_pin_bit_mask = 0;
    sp_o_pin_bit_mask = 0;
    if( DT_I_CNT > 0 )
    {
        DT_XLOGI(" gpio input pin");
        for(i=0; i<DT_I_CNT; i++)
        {
            iox->xpio_map[GPIO_I_START+i] = QUEUE_GPIO_INPUT_START+i;
        }
        sp_o_pin_bit_mask |= (1ull<<g_dt.gpio_595165_rck);
        sp_o_pin_bit_mask |= (1ull<<g_dt.gpio_595165_clk);

        sp_i_pin_bit_mask |= (1ull<<g_dt.gpio_595165_q7);

        io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
        io_conf.pin_bit_mask = sp_i_pin_bit_mask;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_down_en = 0;
        io_conf.pull_up_en = 0;
        gpio_config(&io_conf);
    }
    if( DT_R_CNT > 0 )
    {
        DT_XLOGI(" gpio output pin");
        for(i=0; i<DT_R_CNT; i++)
        {
            iox->xpio_map[GPIO_O_START+i] = QUEUE_VPIO_OUTPUT_START+i;
        }
        sp_o_pin_bit_mask |= (1ull<<g_dt.gpio_595165_rck);
        sp_o_pin_bit_mask |= (1ull<<g_dt.gpio_595165_sdi);
        sp_o_pin_bit_mask |= (1ull<<g_dt.gpio_595165_clk);

        io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = sp_o_pin_bit_mask;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_config(&io_conf);
        if( DT_I_CNT > 0 )
        {
            HC595165_RCK(DT_HIGH);
            HC595165_CLK(DT_HIGH);
            HC595165_SDI(DT_HIGH);

            DT_XLOGI(" gpio HC595 init");
        }
        if( DT_R_CNT > 0 )
        {
            DT_XLOGI(" gpio HC165 init");
            HC595165_RCK(DT_HIGH);
            HC595165_CLK(DT_HIGH);
        }

        vpio_refresh(igpio, ogpio, i_bytes, o_bytes);

        io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ull<<g_dt.gpio_595165_pl);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_config(&io_conf);
    }
}


void vpio_gpio_test(u8_t channel_cnt)
{
    u8 data_ff[VPIO_I_BIT2BYTE*2] = {0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff};
    u8 data_00[VPIO_I_BIT2BYTE*2] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    u8 data_55[VPIO_I_BIT2BYTE*2] = {0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0x55};
    u8 data_aa[VPIO_I_BIT2BYTE*2] = {0x00, 0x00, 0x00, 0x00, 0xaa, 0xaa, 0xaa, 0xaa};

    u8 data_b0[VPIO_I_BIT2BYTE*2] = {0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00};
    u8 data_b1[VPIO_I_BIT2BYTE*2] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00};
    u8 data_b2[VPIO_I_BIT2BYTE*2] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00};
    u8 data_b3[VPIO_I_BIT2BYTE*2] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff};

    u8 data_x[VPIO_I_BIT2BYTE*2] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    s32_t i;

    switch(channel_cnt)
    {
    case 2:
    case 4:
        return;
        break;
    case 8:
        VGPIO_REFRESH(data_ff, &data_ff[VPIO_I_BIT2BYTE], VPIO_IB8, VPIO_OB8);
        VGPIO_REFRESH(data_00, &data_00[VPIO_I_BIT2BYTE], VPIO_IB8, VPIO_OB8);
        VGPIO_REFRESH(data_55, &data_55[VPIO_I_BIT2BYTE], VPIO_IB8, VPIO_OB8);
        VGPIO_REFRESH(data_aa, &data_aa[VPIO_I_BIT2BYTE], VPIO_IB8, VPIO_OB8);

        for(i=0; i<8; i++)
        {
            setbit(data_x, 32+i);
            VGPIO_REFRESH(data_x, &data_x[VPIO_I_BIT2BYTE], VPIO_IB8, VPIO_OB8);
            clrbit(data_x, 32+i);
        }
        for(i=7; i>=0; i--)
        {
            setbit(data_x, 32+i);
            VGPIO_REFRESH(data_x, &data_x[VPIO_I_BIT2BYTE], VPIO_IB8, VPIO_OB8);
            clrbit(data_x, 32+i);
        }
        break;
    case 16:
        VGPIO_REFRESH(data_ff, &data_ff[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);
        VGPIO_REFRESH(data_00, &data_00[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);
        VGPIO_REFRESH(data_55, &data_55[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);
        VGPIO_REFRESH(data_aa, &data_aa[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);

        VGPIO_REFRESH(data_b0, &data_b0[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);
        VGPIO_REFRESH(data_b1, &data_b1[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);
        VGPIO_REFRESH(data_00, &data_00[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);

        for(i=0; i<16; i++)
        {
            setbit(data_x, 32+i);
            VGPIO_REFRESH(data_x, &data_x[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);
            clrbit(data_x, 32+i);
        }
        for(i=15; i>=0; i--)
        {
            setbit(data_x, 32+i);
            VGPIO_REFRESH(data_x, &data_x[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);
            clrbit(data_x, 32+i);
        }
        break;
    case 24:
        VGPIO_REFRESH(data_ff, &data_ff[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);
        VGPIO_REFRESH(data_00, &data_00[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);
        VGPIO_REFRESH(data_55, &data_55[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);
        VGPIO_REFRESH(data_aa, &data_aa[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);

        VGPIO_REFRESH(data_b0, &data_b0[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);
        VGPIO_REFRESH(data_b1, &data_b1[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);
        VGPIO_REFRESH(data_00, &data_00[VPIO_I_BIT2BYTE], VPIO_IB16, VPIO_OB16);

        for(i=0; i<24; i++)
        {
            setbit(data_x, 32+i);
            VGPIO_REFRESH(data_x, &data_x[VPIO_I_BIT2BYTE], VPIO_IB24, VPIO_OB24);
            clrbit(data_x, 32+i);
        }
        for(i=23; i>=0; i--)
        {
            setbit(data_x, 32+i);
            VGPIO_REFRESH(data_x, &data_x[VPIO_I_BIT2BYTE], VPIO_IB24, VPIO_OB24);
            clrbit(data_x, 32+i);
        }
        break;
    case 32:
        VGPIO_REFRESH(data_ff, &data_ff[VPIO_I_BIT2BYTE], VPIO_IB32, VPIO_OB16);
        VGPIO_REFRESH(data_00, &data_00[VPIO_I_BIT2BYTE], VPIO_IB32, VPIO_OB16);
        VGPIO_REFRESH(data_55, &data_55[VPIO_I_BIT2BYTE], VPIO_IB32, VPIO_OB16);
        VGPIO_REFRESH(data_aa, &data_aa[VPIO_I_BIT2BYTE], VPIO_IB32, VPIO_OB16);

        VGPIO_REFRESH(data_b0, &data_b0[VPIO_I_BIT2BYTE], VPIO_IB32, VPIO_OB16);
        VGPIO_REFRESH(data_b1, &data_b1[VPIO_I_BIT2BYTE], VPIO_IB32, VPIO_OB16);
        VGPIO_REFRESH(data_b2, &data_b2[VPIO_I_BIT2BYTE], VPIO_IB32, VPIO_OB16);
        VGPIO_REFRESH(data_b3, &data_b3[VPIO_I_BIT2BYTE], VPIO_IB32, VPIO_OB16);
        VGPIO_REFRESH(data_00, &data_00[VPIO_I_BIT2BYTE], VPIO_IB32, VPIO_OB16);

        for(i=0; i<32; i++)
        {
            setbit(data_x, 32+i);
            VGPIO_REFRESH(data_x, &data_x[VPIO_I_BIT2BYTE], VPIO_IB32, VPIO_OB16);
            clrbit(data_x, 32+i);
        }
        for(i=31; i>=0; i--)
        {
            setbit(data_x, 32+i);
            VGPIO_REFRESH(data_x, &data_x[VPIO_I_BIT2BYTE], VPIO_IB32, VPIO_OB16);
            clrbit(data_x, 32+i);
        }
        break;
    default:
        break;
    }
}

b8 xpio_r(u8 gpio)
{
    dt_io_t *iox = &g_io;

    if( gpio >= GPIO_END )
    {
        return U8MAX;
    }
    return (!!isset(iox->xpio_bits, gpio));
}

void xpio_w(u8 gpio, b8 on)
{
    dt_io_t *iox = &g_io;

    if( gpio >= GPIO_END )
    {
        return;
    }
    mkbit(iox->xpio_bits, gpio, on);
}

void xpio_sync(dt_io_t *iox)
{
    s32_t i;
    u8_t gpio, bit, vgpio_i = 0, vgpio_o = 0;
    u8_t xpio_i;

    DT_XLOGE(" %d", DT_R_CNT);

    if( FALSE == g_dt.is_vpio )
    {
        DT_XLOGE(" gpio %d:%d", GPIO_O_START, DT_R_CNT);
        DT_LOGI_BUFFER_HEX(&iox->xpio_map[VPIO_O_BYTE_START], GPIO_O_BIT2BYTE);
        for(i=0; i<DT_R_CNT; i++)
        {
            xpio_i = i + GPIO_O_START;

            gpio= iox->xpio_map[xpio_i];
            bit = (!!isset(iox->xpio_bits, xpio_i));
            gpio_set_level(gpio, bit);
        }
    }else
    {
        DT_XLOGE(" DT_R_CNT %d", DT_R_CNT);
        switch(DT_R_CNT)
        {
        case 2:
        case 4:
        default:
            DT_XLOGD(" DT_R_CNT:%d no spio", DT_R_CNT);
            return;
            break;

        case 8:
            vgpio_i = VPIO_IB8;
            vgpio_o = VPIO_OB8;
            break;
        case 16:
            vgpio_i = VPIO_IB16;
            vgpio_o = VPIO_OB16;
            break;
        case 24:
            vgpio_i = VPIO_IB24;
            vgpio_o = VPIO_OB24;
            break;
        case 32:
            vgpio_i = VPIO_IB32;
            vgpio_o = VPIO_OB32;
            break;
        }
        VGPIO_REFRESH(&(iox->t_xpio_bits[VPIO_I_BYTE_START]), &(iox->xpio_bits[VPIO_O_BYTE_START]), 
            vgpio_i, vgpio_o);
        DT_XLOGW("t I:%02x%02x%02x%02x O:%02x%02x%02x%02x", 
            iox->t_xpio_bits[VPIO_I_BYTE_START+0], 
            iox->t_xpio_bits[VPIO_I_BYTE_START+1], 
            iox->t_xpio_bits[VPIO_I_BYTE_START+2], 
            iox->t_xpio_bits[VPIO_I_BYTE_START+3],
            iox->xpio_bits[VPIO_I_BYTE_START+0],
            iox->xpio_bits[VPIO_I_BYTE_START+1],
            iox->xpio_bits[VPIO_I_BYTE_START+2],
            iox->xpio_bits[VPIO_I_BYTE_START+3]);
    }
}

void gpio_capture_factory(dt_io_t *iox, u32_t io_num)
{
    u8_t bit, bitx;
//    u8_t i_index;
    u32 cur_time;
    u32_t t_ms = FACTORY_TIMEOUT;

    bit = gpio_get_level(io_num);
    bitx = isset(iox->xpio_bits, GPIO_I_FACTORY);
    DT_XLOGI(" factory_pin %d -> %d ", bitx, bit);
    if( bit == bitx )
    {
        return;
    }
    mkbit(iox->xpio_bits, GPIO_I_FACTORY, bit);
    cur_time = cur_time_ms();
    if( DT_HIGH == bit )
    {
        factory_led_action(FALSE);
        DT_XLOGE(" factory pin:%d %d -> %d release ----------", io_num, bitx, bit);
        if( U32MAX == iox->factory_press_ms )
        {
            DT_XLOGE(" factory pin:%d %d -> %d release factory_press_ms==U32MAX return ----------", io_num, bitx, bit);
            return;
        }
        cur_time = time_dec(cur_time, iox->factory_press_ms);
        iox->factory_press_ms = U32MAX;
        if( cur_time >= t_ms )
        {
            DT_XLOGE(" factory button pressed <<<<<<<<<<<<<<<<");
        }
    }else
    {
        iox->factory_press_ms = cur_time;
        factory_led_action(TRUE);
        DT_XLOGE(" factory pin:%d %d -> %d press ----------", io_num, bitx, bit);
    }
}

void gpio_check_factory(dt_io_t *iox)
{
    u32 cur_time;
    u32_t t_ms = FACTORY_TIMEOUT;

    if( U32MAX == iox->factory_press_ms )
    {
        return;
    }
    cur_time = cur_time_ms();

    cur_time = time_dec(cur_time, iox->factory_press_ms);
    if( cur_time >= t_ms )
    {
        DT_XLOGE(" factory button pressed <<<<<<<<<<<<<<<<");
        iox->factory_press_ms = U32MAX;
    }
}

void gpio_i_change(dt_io_t *iox, u32_t io_num)
{
    s32_t i;
    u8_t gpio, bit, bitx;
    u8_t ux_i, xpio_i;
//    u32 cur_time;

    if( io_num == g_dt.gpio_factory )
    {
        gpio_capture_factory(iox, io_num);
        return;
    }

    if( FALSE == g_dt.is_vpio )
    {
        DT_XLOGI(" io_num:%d", io_num);
        for(i=0; i<DT_I_CNT; i++)
        {
            xpio_i = i + GPIO_I_START;
            ux_i = i;

            gpio= iox->xpio_map[xpio_i];

            bit = gpio_get_level(gpio);
            DT_XLOGD(" gpio:%d new-bit:%d", gpio, bit);
            bitx= (!!isset(iox->xpio_bits, xpio_i));
            if( bitx == bit )
            {
                DT_XLOGE(" gpio:%d %d -> %d same continue", gpio, bitx, bit);
                continue;
            }
            mkbit(iox->xpio_bits, xpio_i, bit);
            if( isset(iox->at_debounce, ux_i) )
            {
                clrbit(iox->at_debounce, ux_i);
                iox->input_debounce_ms[ux_i] = U32MAX;
                DT_XLOGI(" gpio:%d %d -> %d debounce ----------", gpio, bitx, bit);
            }else
            {
                setbit(iox->at_debounce, ux_i);
                iox->input_debounce_ms[ux_i] = cur_time_ms();
                DT_XLOGE(" gpio:%d %d -> %d new ux_i:%d xpio_i:%d >>>>>>>>>>>>>>>", gpio, bitx, bit, ux_i, xpio_i);
            }
        }
    }
}

void vpio_i_change(dt_io_t *iox, u32_t io_num)
{
    s32_t i;
    u8_t bit, bitx;
    u8_t ux_i, xpio_i;

    DT_XLOGD(" vpio check change t I,I");
    DT_LOGN_BUFFER_HEX(&(iox->t_xpio_bits[VPIO_I_BYTE_START]), GPIO_I_BIT2BYTE);
    DT_LOGN_BUFFER_HEX(&(iox->xpio_bits[VPIO_I_BYTE_START]), GPIO_I_BIT2BYTE);
    for(i=0; i<DT_I_CNT; i++)
    {
        xpio_i = i + GPIO_I_START;
        ux_i = i;

        bitx= (!!isset(iox->xpio_bits, xpio_i));
        bit = (!!isset(iox->t_xpio_bits, xpio_i));
        if( bit == bitx )
        {
            DT_XLOGD(" vpio:%d %d -> %d same continue", ux_i, bitx, bit);
            continue;
        }
        mkbit(iox->xpio_bits, xpio_i, bit);
        if( isset(iox->at_debounce, ux_i) )
        {
            clrbit(iox->at_debounce, ux_i);
            iox->input_debounce_ms[ux_i] = U32MAX;
            DT_XLOGI(" vgpio:%d %d -> %d debounce ----------", ux_i, bitx, bit);
        }else
        {
            setbit(iox->at_debounce, ux_i);
            iox->input_debounce_ms[ux_i] = cur_time_ms();
            DT_XLOGE(" vpio:%d %d -> %d new >>>>>>>>>>>>>>>", ux_i, bitx, bit);
        }
    }

    return;
}

u32_t vpio_check_input(dt_io_t *iox)
{
    switch(DT_R_CNT)
    {
    case 2:
    case 4:
    default:
        return U32MAX;
        break;

    case 8:
        VGPIO_REFRESH(&(iox->t_xpio_bits[VPIO_I_BYTE_START]), &(iox->xpio_bits[VPIO_O_BYTE_START]), 
            VPIO_IB8, VPIO_OB8);
        break;

    case 16:
        VGPIO_REFRESH(&(iox->t_xpio_bits[VPIO_I_BYTE_START]), &(iox->xpio_bits[VPIO_O_BYTE_START]), 
            VPIO_IB16, VPIO_OB16);
        break;
    case 24:
        VGPIO_REFRESH(&(iox->t_xpio_bits[VPIO_I_BYTE_START]), &(iox->xpio_bits[VPIO_O_BYTE_START]), 
            VPIO_IB24, VPIO_OB24);
        break;
    case 32:
        VGPIO_REFRESH(&(iox->t_xpio_bits[VPIO_I_BYTE_START]), &(iox->xpio_bits[VPIO_O_BYTE_START]), 
            VPIO_IB32, VPIO_OB32);
        break;
    }
    if( 0 == memcmp(&(iox->t_xpio_bits[VPIO_I_BYTE_START]), 
                    &(iox->xpio_bits[VPIO_I_BYTE_START]), 
                    VPIO_I_BIT2BYTE) )
    {
        return U32MAX;
    }
    DT_XLOGW("t I:%02x%02x%02x%02x I:%02x%02x%02x%02x ", 
        iox->t_xpio_bits[VPIO_I_BYTE_START+0], 
        iox->t_xpio_bits[VPIO_I_BYTE_START+1], 
        iox->t_xpio_bits[VPIO_I_BYTE_START+2], 
        iox->t_xpio_bits[VPIO_I_BYTE_START+3],
        iox->xpio_bits[VPIO_I_BYTE_START+0], 
        iox->xpio_bits[VPIO_I_BYTE_START+1], 
        iox->xpio_bits[VPIO_I_BYTE_START+2], 
        iox->xpio_bits[VPIO_I_BYTE_START+3]);
    DT_XLOGD(" diff QUEUE_VPIO_INPUT_START");
    return QUEUE_VPIO_INPUT_START;
}

void xpio_input_debounce(dt_io_t *iox)
{
    s32_t i, ux_i, xpio_i;
    u32_t cur_ms = cur_time_ms();
    b8 b_find = FALSE;
    u8_t mask[VPIO_I_BIT2BYTE] = {0};
    u8_t bits[VPIO_I_BIT2BYTE] = {0};
    u8_t bit;

    s32_t log_off;
    char log_buf[((VGPIO_I_MAX)*2)+8] = {0};

    for(i=0; i<DT_I_CNT; i++)
    {
        xpio_i = i + GPIO_I_START;
        ux_i = i;
        if( isclr(iox->at_debounce, ux_i) )
        {
            continue;
        }
        if( time_dec(cur_ms, iox->input_debounce_ms[ux_i]) < INPUT_DEBOUNCE_MS )
        {
            continue;
        }
        bit = (!!isset(iox->xpio_bits, xpio_i));
        DT_XLOGW("vpio ux_i: %d xpio_i:%d bit=%d", ux_i, xpio_i, bit);
        clrbit(iox->at_debounce, ux_i);
        iox->input_debounce_ms[ux_i] = U32MAX;
        setbit(mask, ux_i);
        mkbit(bits, ux_i, bit);
        b_find = TRUE;
    }

    if( FALSE == iox->started )
    {
        return;
    }

    if( TRUE == b_find )
    {
        DT_XLOGW(" print changes <<<<<<<<<<<<<<<<");
        log_off = 0;
        log_buf[log_off] = 'M';log_off++;
        log_buf[log_off] = ':';log_off++;
        for(i=0; i<DT_I_CNT; i++)
        {
            ux_i = i;
            log_buf[log_off] = isset(mask, ux_i) ? '1':'0';log_off++;
            if( isset(mask, ux_i) )
            {
                dt_mqtt_set_hw_i(i, !!isset(bits, ux_i));
            }
        }
        log_buf[log_off] = ' ';log_off++;
        log_buf[log_off] = 'V';log_off++;
        log_buf[log_off] = ':';log_off++;
        for(i=0; i<DT_I_CNT; i++)
        {
            ux_i = i;
            log_buf[log_off] = isset(bits, ux_i) ? '1':'0';log_off++;
        }
        DT_XLOGW("%s", log_buf);

        mqtt_notify();
    }
}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    dt_io_t *iox = &g_io;
    u32_t gpio_num = (u32_t) arg;

    if( FALSE == iox->started )
    {
        return;
    }
    xQueueSendFromISR(iox->gpio_queue, &gpio_num, NULL);
}

static void gpio_task(void* arg)
{
    dt_io_t *iox = (dt_io_t*)arg;
    u32_t io_num;
    u32_t ret;

    DT_XLOGW(" start ");

    do{
        io_num = U32MAX;
        ret = xQueueReceive(iox->gpio_queue, &io_num, HW_MS(20));
        if( TRUE == ret )
        {
            DT_XLOGW(" ret:%d io_num:%d", ret, io_num);
            ret = io_num;
        }else
        {
            ret = vpio_check_input(iox);
        }
        switch(ret)
        {
        case QUEUE_GPIO_SYNC:
            DT_XLOGE(" QUEUE_GPIO_SYNC ret:%d QUEUE_GPIO_SYNC:%d", ret, QUEUE_GPIO_SYNC);
            xpio_sync(iox);
            break;
        case QUEUE_GPIO_INPUT_START ... QUEUE_GPIO_INPUT_END:
            DT_XLOGW(" ret:%d io_num:%d", ret, io_num);
            gpio_i_change(iox, ret);
            break;
        case QUEUE_VPIO_INPUT_START ... QUEUE_VPIO_INPUT_END:
            vpio_i_change(iox, ret);
            break;
        case QUEUE_GPIO_START:
            DT_XLOGE(" QUEUE_GPIO_START");
            iox->started = TRUE;
            mqtt_notify();
            break;
        case QUEUE_GPIO_EXIT:
            DT_XLOGE(" QUEUE_GPIO_EXIT");
            goto exit;
            break;

        case U32MAX:
        default:
            break;
        }
        xpio_input_debounce(iox);
        gpio_check_factory(iox);
    }while(1);

exit:
    DT_XLOGW(" end<<<<<<<<<<<<<<<<");

    vTaskDelete(NULL);
}


b8_t relay_r(u8_t i)
{
//    dt_io_t *iox = &g_io;
//    u8_t offset;

    i += GPIO_O_START;

    return xpio_r(i);
}

void relay_w(u8_t i, b8_t on)
{
//    dt_io_t *iox = &g_io;

    i += GPIO_O_START;

    xpio_w(i, on);
}

void iox_sync(void)
{
    dt_io_t *iox = &g_io;
    int32_t io = QUEUE_GPIO_SYNC;
    xQueueSend(iox->gpio_queue, &io, 0);
}

void iox_start(void)
{
    dt_io_t *iox = &g_io;
    int32_t io = QUEUE_GPIO_START;

    xQueueSend(iox->gpio_queue, &io, 0);
}

void iox_exit(void)
{
    dt_io_t *iox = &g_io;
    int32_t io = QUEUE_GPIO_EXIT;

    gpio_uninstall_isr_service();

    xQueueSend(iox->gpio_queue, &io, 0);
}

void dt_gpio_init(void)
{
    dt_io_t *iox = &g_io;
//    esp_err_t err;
    gpio_config_t io_conf = {0};
    int32_t i;
    uint64_t i_irq_pin_bit_mask = 0;
    uint64_t o_pin_bit_mask = 0;
//    s32_t end;
    u32_t gpio, bit, xpio_i;

    factory_led_init();

    memset(iox, 0, sizeof(dt_io_t));
    memset(iox->xpio_map, U8MAX, GPIO_CNT);
    memset(iox->input_debounce_ms, U32MAX, VGPIO_I_MAX*sizeof(u32_t));

    iox->gpio_spinlock = (portMUX_TYPE)portMUX_INITIALIZER_UNLOCKED;

/////////////////////////////////////////////////////
    DT_XLOGI(" gpio input pins %d", DT_I_CNT);
    DT_LOGI_BUFFER_HEX(&iox->xpio_map[GPIO_I_START], DT_I_CNT);
    DT_XLOGI(" gpio output pins %d", DT_R_CNT);
    DT_LOGI_BUFFER_HEX(&iox->xpio_map[GPIO_O_START], DT_R_CNT);

    i_irq_pin_bit_mask = 0;
    /* hw input factory init */
    DT_XLOGI(" factory pin");
    iox->xpio_map[GPIO_I_FACTORY] = QUEUE_GPIO_INPUT_START+g_dt.gpio_factory;
    i_irq_pin_bit_mask |= (1ull<<g_dt.gpio_factory);

    /* hw input init */
    if( FALSE == g_dt.is_vpio ){
        DT_XLOGI(" hw input pin");
        for(i=0; i<DT_I_CNT; i++)
        {
            iox->xpio_map[GPIO_I_START+i] = QUEUE_GPIO_INPUT_START+g_dt.gpio_input[i];
            if( GPIO_NULL != g_dt.gpio_input[i] )
            {
                i_irq_pin_bit_mask |= (1ull<<iox->xpio_map[GPIO_I_START+i]);
            }
        }
        DT_XLOGI(" irq pin config");
        io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
        io_conf.pin_bit_mask = i_irq_pin_bit_mask;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        gpio_config(&io_conf);
        gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
        for(i=0; i<DT_I_CNT; i++)
        {
            gpio = iox->xpio_map[GPIO_I_START+i];
            if( GPIO_NULL != gpio )
            {
                gpio_isr_handler_add(gpio, gpio_isr_handler, (void*)gpio);
            }
        }

        /* gpio output init */
        DT_XLOGI(" gpio output pin");
        o_pin_bit_mask = 0;
        for(i=0; i<DT_R_CNT; i++)
        {
            iox->xpio_map[GPIO_O_START+i] = g_dt.gpio_output[i];
            if( GPIO_NULL != g_dt.gpio_output[i] )
            {
                o_pin_bit_mask |= (1ull<<iox->xpio_map[GPIO_O_START+i]);
            }
        }
        if( 0 != o_pin_bit_mask )
        {
            io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
            io_conf.mode = GPIO_MODE_OUTPUT;
            io_conf.pin_bit_mask = o_pin_bit_mask;
            io_conf.pull_down_en = 0;
            io_conf.pull_up_en = 0;
            gpio_config(&io_conf);
        }
    }else{
        /* spio */
        DT_XLOGI(" spio gpio pin");
        vpio_init(iox);

        DT_XLOGI(" irq pin config");
        io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
        io_conf.pin_bit_mask = i_irq_pin_bit_mask;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        gpio_config(&io_conf);
        gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    }

    DT_XLOGI(" all DEF relay init");
    for(i=0; i<DT_R_CNT; i++)
    {
        xpio_w(GPIO_O_START+i, FALSE);
    }

//////////////////////////////////////////////////////////////
    DT_XLOGI(" add GPIO_I_FACTORY IRQ %d gpio:%d", iox->xpio_map[GPIO_I_FACTORY], g_dt.gpio_factory);
    gpio = g_dt.gpio_factory;
    gpio_isr_handler_add(gpio, gpio_isr_handler, (void*)gpio);

    DT_XLOGI(" xpio_sync 1");
    xpio_sync(iox);
    DT_XLOGI(" xpio_sync 2");
    memcpy(&(iox->xpio_bits[VPIO_I_BYTE_START]), &(iox->t_xpio_bits[VPIO_I_BYTE_START]), VPIO_I_BIT2BYTE);
    for(i=0; i<=DT_I_CNT; i++)
    {
        xpio_i = GPIO_I_START+i;
        gpio = iox->xpio_map[xpio_i];
        if( GPIO_NULL != gpio )
        {
            bit = gpio_get_level(gpio);
            mkbit(iox->xpio_bits, xpio_i, bit);
        }
    }

//////////////////////////////////////////////////////////////
    bit = gpio_get_level(g_dt.gpio_factory);
    DT_XLOGI(" factory_pin:%d ", bit);
    mkbit(iox->xpio_bits, GPIO_I_FACTORY, bit);
    iox->factory_press_ms = U32MAX;

    iox->started = FALSE;
    iox->gpio_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(gpio_task, "gpio", 4096, (void*)iox, 10, NULL);

#if 0
    for(i=0;i<DT_R_CNT; i++){
        relay_w(i, DT_HIGH);
        iox_sync();
        vTaskDelay(HW_MS(S2MS/2));
    }
    vTaskDelay(HW_MS(5*S2MS));
    for(i=0;i<DT_R_CNT; i++){
        relay_w(i, DT_LOW);
        iox_sync();
        vTaskDelay(HW_MS(S2MS/2));
    }
#endif
}

