/*
 * hk_adc.c
 *
 *  Created on: 05-Mar-2021
 *      Author: Sonal Shrivastava
 */

#include "../firmware/drivers/mss_i2c/mss_i2c.h"
#include "hk_adc.h"

// MSS_I2C_read()
// MSS_I2C_write()
// MSS_I2C_write_read()
// MSS_I2C_get_status()
// MSS_I2C_wait_complete()

//uint32_t g_FrequencyPCLK0 = MSS_SYS_APB_0_CLK_FREQ = 100MHz; APB0 cz i2c0 is driven by APB0

#define TX_LENGTH           1u
#define RX_LENGTH           2u
#define HK_ADC_SLAVE_ADDR   0x48
#define DUMMY_SERIAL_ADDR   0xFF

uint8_t  rx_buffer[RX_LENGTH];
uint8_t  read_length = RX_LENGTH;
uint8_t  tx_buffer[TX_LENGTH];
uint8_t  write_length = TX_LENGTH;

//static uint8_t  target_slave_addr =  HK_ADC_SLAVE_ADDR;
static mss_i2c_status_t status;


int hk_adc_initialisation(void)
{
    uint16_t result = 0;
    uint8_t err = HK_ADC_ERR_INIT_FAILED;
//    uint8_t send_buffer[3] = {HK_ADC_REG_CONFIG, 0xE3, 0x03}; //config register, MSB, LSB (Reading CITI TEMP)
    uint8_t send_buffer[2] = {0xE3, 0x03}; //MSB, LSB (Reading CITI TEMP)

    // Set the clock
    MSS_I2C_init(&g_mss_i2c0, DUMMY_SERIAL_ADDR, MSS_I2C_PCLK_DIV_256); //10^8/256 = 390,625Hz =~ 390KHz

//    //Already in fast mode no special action is required
//    // Write data to slave.
//    MSS_I2C_write(&g_mss_i2c0, HK_ADC_SLAVE_ADDR, &send_buffer[0], sizeof(send_buffer)/sizeof(send_buffer[0]), MSS_I2C_RELEASE_BUS);
//
//    // Wait for completion and record the outcome
//    status = MSS_I2C_wait_complete(&g_mss_i2c0, MSS_I2C_NO_TIMEOUT);

    err = hk_adc_reg_write(HK_ADC_REG_CONFIG, &send_buffer[0]);

    if (err == HK_ADC_NO_ERR)
    {
//        tx_buffer[0] = HK_ADC_REG_CONVERSION;
//        // Write to Address Pointer Register - pointing to 'reg'
//        MSS_I2C_write(&g_mss_i2c0, HK_ADC_SLAVE_ADDR, &tx_buffer[0], sizeof(tx_buffer)/sizeof(tx_buffer[0]), MSS_I2C_RELEASE_BUS);
//
//        // Wait for completion and record the outcome
//        status = MSS_I2C_wait_complete(&g_mss_i2c0, MSS_I2C_NO_TIMEOUT);
//
//        // Proceed only if it is a success
//        if (status == MSS_I2C_SUCCESS)
//        {
//            // Read data from target slave using MSS I2C 0 (assuming we always read 2B)
//            MSS_I2C_read(&g_mss_i2c0, HK_ADC_SLAVE_ADDR, &rx_buffer[0], read_length, MSS_I2C_RELEASE_BUS);
//
//            status = MSS_I2C_wait_complete(&g_mss_i2c0, MSS_I2C_NO_TIMEOUT);
//
//            if(status == MSS_I2C_SUCCESS)
//            {
//                result = rx_buffer[1] << 8 | rx_buffer[0];
//                return HK_ADC_NO_ERR;
//            }
//        }
        err = hk_adc_reg_read(HK_ADC_REG_CONVERSION, &result);
        if (err == HK_ADC_NO_ERR)
        {
            return err;
        }
    }

    return HK_ADC_ERR_INIT_FAILED;
}



int hk_adc_init(void)
{
    uint8_t err = HK_ADC_ERR_INIT_FAILED;

    // By default, ADC is configured to point towards CITI_TEMP as MUX input
    uint8_t send_buffer[2] = {0x63, 0x03}; //MSB, LSB (Reading CITI TEMP) 0x63 = OS-bit = 0 - no conversion yet

    // Set the clock for i2c0 instance
    MSS_I2C_init(&g_mss_i2c0, DUMMY_SERIAL_ADDR, MSS_I2C_PCLK_DIV_256); //10^8/256 = 390,625Hz =~ 390KHz

    // Write to the CONFIG register
    err = hk_adc_reg_write(HK_ADC_REG_CONFIG, &send_buffer[0]);

    if (err == HK_ADC_NO_ERR)
    {
        return err;
    }

    return err;
}




int hk_adc_conv_config(hk_adc_conv_t read_type)
{
    uint8_t err = HK_ADC_ERR_CONV_CONFIG_FAILED;

    // By default, ADC is configured to point towards CITI_TEMP as MUX input
    uint8_t send_buffer[2] = {0x00, 0x03}; //MSB, LSB (Reading CITI TEMP)
    send_buffer[0] = (((1u << 3) | read_type) << 4) | 0x03;

    // Write to the CONFIG register
    err = hk_adc_reg_write(HK_ADC_REG_CONFIG, &send_buffer[0]);

    if (err == HK_ADC_NO_ERR)
    {
        return err;
    }

    return err;
}


int hk_adc_reg_read_conv(uint16_t *read_buffer)
{
    uint16_t result = 0;
//    uint8_t send_buffer = HK_ADC_REG_CONVERSION;
    uint8_t err = HK_ADC_ERR_READ_FAILED;

//    // Write to Address Pointer Register - pointing to 'reg'
//    MSS_I2C_write(&g_mss_i2c0, HK_ADC_SLAVE_ADDR, &send_buffer, TX_LENGTH, MSS_I2C_RELEASE_BUS);
//
//    // Wait for completion and record the outcome
//    status = MSS_I2C_wait_complete(&g_mss_i2c0, MSS_I2C_NO_TIMEOUT);
//
//    // Proceed only if it is a success
//    if (status == MSS_I2C_SUCCESS)
//    {
//        // Read data from target slave (we always read 2B)
//        MSS_I2C_read(&g_mss_i2c0, HK_ADC_SLAVE_ADDR, &rx_buffer[0], RX_LENGTH, MSS_I2C_RELEASE_BUS);
//
//        status = MSS_I2C_wait_complete(&g_mss_i2c0, MSS_I2C_NO_TIMEOUT);
//
//        if(status == MSS_I2C_SUCCESS)
//        {
//            // Right shift the read data by 4bits since conversion reg holds 12-bit result D[15:4]
//            result = (rx_buffer[0] << 8 | rx_buffer[1]) >> 4;
//            *read_buffer = result;
//            return HK_ADC_NO_ERR;
//        }
//    }

    // OR
    err = hk_adc_reg_read(HK_ADC_REG_CONVERSION, &result);
    if (err == HK_ADC_NO_ERR)
    {
        // Right shift the received result by 4 bits. Conversion value are 12-bits
        result = result >> 4;
        *read_buffer = result;
        return err;
    }

    return err;
}



int hk_adc_reg_write(hk_adc_reg_t reg, uint8_t *write_buffer)
{
    // Check what size of write buffer is actually needed
    uint8_t send_buffer[3] = {reg, 0, 0};
//    send_buffer[0] = reg;
    send_buffer[1] = *write_buffer;         //MSB
    send_buffer[2] = *(write_buffer + 1);   //LSB
    uint8_t err = HK_ADC_ERR_WRITE_FAILED;
    uint16_t read_back, tmp = 0;
    tmp = send_buffer[1]<<8 | send_buffer[2];

    // Write data to slave.
    MSS_I2C_write(&g_mss_i2c0, HK_ADC_SLAVE_ADDR, &send_buffer[0], sizeof(send_buffer)/sizeof(send_buffer[0]), MSS_I2C_RELEASE_BUS);

    // Wait for completion and record the outcome
    status = MSS_I2C_wait_complete(&g_mss_i2c0, MSS_I2C_NO_TIMEOUT);

    if (status == MSS_I2C_SUCCESS)
    {
        err = hk_adc_reg_read(reg, &read_back);
        if (err == HK_ADC_NO_ERR)
        {
            if (read_back == tmp)
            {
                return err;
            }
        }

        err = HK_ADC_ERR_WRITE_FAILED;
    }

    return err;
}


int hk_adc_reg_read(hk_adc_reg_t reg, uint16_t *read_buffer)
{
    uint16_t result = 0;
    uint8_t send_buffer = reg;

    // Write to Address Pointer Register - pointing to 'reg'
    MSS_I2C_write(&g_mss_i2c0, HK_ADC_SLAVE_ADDR, &send_buffer, TX_LENGTH, MSS_I2C_RELEASE_BUS);

    // Wait for completion and record the outcome
    status = MSS_I2C_wait_complete(&g_mss_i2c0, MSS_I2C_NO_TIMEOUT);

    // Proceed only if it is a success
    if (status == MSS_I2C_SUCCESS)
    {
        // Read data from target slave (we always read 2B)
        MSS_I2C_read(&g_mss_i2c0, HK_ADC_SLAVE_ADDR, &rx_buffer[0], RX_LENGTH, MSS_I2C_RELEASE_BUS);

        status = MSS_I2C_wait_complete(&g_mss_i2c0, MSS_I2C_NO_TIMEOUT);

        if(status == MSS_I2C_SUCCESS)
        {
            result = rx_buffer[0] << 8 | rx_buffer[1];
            *read_buffer = result;
            return HK_ADC_NO_ERR;
        }
    }

    return HK_ADC_ERR_READ_FAILED;
}


