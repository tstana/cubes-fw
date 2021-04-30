/*
 * hk_adc.c
 *
 *  Created on: 05-Mar-2021
 *      Author: Sonal Shrivastava
 */

#include "../firmware/drivers/mss_gpio/mss_gpio.h"
#include "../firmware/drivers/mss_i2c/mss_i2c.h"
#include "hk_adc.h"


#define TX_LENGTH           1u
#define RX_LENGTH           2u
#define DUMMY_SERIAL_ADDR   0xFF
#define VBAT_TO_VOUT_RATIO  8u
#define VOUT_TO_IBAT_RATIO  2.5
#define VOLT_TO_MILLIVOLT_RATIO  1000u


/* Local Variables */
static mss_i2c_status_t status;
static uint16_t read_value;
static uint8_t os_bit = 0;
static uint8_t  rx_buffer[RX_LENGTH];
static float multiplier_to_millivolts = 1.0F;
static int fsr_setting = HK_ADC_CONFIG_FSR_2;


/* Local Function prototypes */
static void hk_adc_update_multiplier_volt(void);
static void hk_adc_set_fsr(uint8_t fsr);
static int hk_adc_reg_read(hk_adc_register_t reg, uint16_t *read_buffer);
static int hk_adc_reg_write(hk_adc_register_t reg, uint8_t *write_buffer);




int hk_adc_init(void)
{
    uint8_t err = HK_ADC_ERR_INIT_FAILED;

    // set the full scale voltage range to -4.096 <= V <= +4.096 to support measurement of all ADC inputs
    fsr_setting = HK_ADC_CONFIG_FSR_2;

    // By default, ADC is configured to point towards CITI_TEMP as MUX input
    uint16_t config = HK_ADC_CONFIG_MUX_SINGLE_2 | HK_ADC_CONFIG_FSR_2 | HK_ADC_CONFIG_MODE_SINGLE_SHOT | HK_ADC_CONFIG_DR_128SPS | HK_ADC_CONFIG_CMODE_TRAD | HK_ADC_CONFIG_CPOL_ACTIVE_LOW | HK_ADC_CONFIG_CLAT_NON_LATCH | HK_ADC_CONFIG_CQUE_DISABLE;
    uint8_t send_buffer[2] = {config>>8, config};

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




int hk_adc_conv_read_volt(float * batt_volt)
{
    uint8_t err = HK_ADC_ERR_VOLT_READ_FAILED;

    uint8_t send_buffer[2] = {0};

    err = hk_adc_reg_read(HK_ADC_REG_CONFIG, &read_value);

    if (err == HK_ADC_NO_ERR)
    {
        // let the rest of the configurations remain intact, just change MUX input config and start single-shot conversion
        read_value = read_value & 0x8fff;
        read_value = read_value | HK_ADC_CONFIG_MUX_SINGLE_1;

        send_buffer[0] = (read_value >> 8) | HK_ADC_CONFIG_OS_SINGLE_CONV;
        send_buffer[1] = read_value;

        // Write changed MUX input back to the CONFIG register
        err = hk_adc_reg_write(HK_ADC_REG_CONFIG, &send_buffer[0]);

        if (err == HK_ADC_NO_ERR)
        {
            //INSERT DELAY FN (1sec), do not use WHILE Loops
            //Wait until OS bit turns 1
            do{
                hk_adc_reg_read(HK_ADC_REG_CONFIG, &read_value);
                os_bit = read_value >> 15;
            }while(os_bit == HK_ADC_CONFIG_OS_NOT_READY);

            err = hk_adc_reg_read(HK_ADC_REG_CONVERSION, &read_value);
            if (err == HK_ADC_NO_ERR)
            {
                // Right shift the received result by 4 bits. Conversion value are 12 most significant bits
                read_value = read_value >> 4;
                // make sure we have the most recent multiplier settings based on the currently configured PGA
                hk_adc_update_multiplier_volt();
                *batt_volt = ((float)read_value * multiplier_to_millivolts) * VBAT_TO_VOUT_RATIO/VOLT_TO_MILLIVOLT_RATIO;
                return err;
            }
        }
    }
    return err;
}




int hk_adc_conv_read_curr(float * batt_curr)
{
    uint8_t err = HK_ADC_ERR_CURR_READ_FAILED;

    uint8_t send_buffer[2] = {0};

    err = hk_adc_reg_read(HK_ADC_REG_CONFIG, &read_value);

    if (err == HK_ADC_NO_ERR)
    {
        // let the rest of the configurations remain intact, just change MUX input config and start single-shot conversion
        read_value = read_value & 0x8fff;
        read_value = read_value | HK_ADC_CONFIG_MUX_SINGLE_0;

        send_buffer[0] = (read_value >> 8) | HK_ADC_CONFIG_OS_SINGLE_CONV;
        send_buffer[1] = read_value;

        // Write changed MUX input back to the CONFIG register
        err = hk_adc_reg_write(HK_ADC_REG_CONFIG, &send_buffer[0]);

        if (err == HK_ADC_NO_ERR)
        {
            //INSERT DELAY FN (1sec), do not use WHILE Loops
            //Wait until OS bit turns 1
            do{
                hk_adc_reg_read(HK_ADC_REG_CONFIG, &read_value);
                os_bit = read_value >> 15;
            }while(os_bit == HK_ADC_CONFIG_OS_NOT_READY);

            err = hk_adc_reg_read(HK_ADC_REG_CONVERSION, &read_value);
            if (err == HK_ADC_NO_ERR)
            {
                // Right shift the received result by 4 bits. Conversion value are 12 most significant bits
                read_value = read_value >> 4;
                // make sure we have the most recent multiplier settings based on the currently configured PGA
                hk_adc_update_multiplier_volt();
                *batt_curr = (((float)read_value * multiplier_to_millivolts)/(VOUT_TO_IBAT_RATIO * VOLT_TO_MILLIVOLT_RATIO));     // Current in Amps;
                return err;
            }
        }
    }
    return err;
}



int hk_adc_conv_read_citi_temp(float * citi_temp)
{
    uint8_t err = HK_ADC_ERR_TEMP_READ_FAILED;

    uint8_t send_buffer[2] = {0};

    err = hk_adc_reg_read(HK_ADC_REG_CONFIG, &read_value);

    if (err == HK_ADC_NO_ERR)
    {
        // let the rest of the configurations remain intact, just change MUX input config and start single-shot conversion
        read_value = read_value & 0x8fff;
        read_value = read_value | HK_ADC_CONFIG_MUX_SINGLE_2;

        send_buffer[0] = (read_value >> 8) | HK_ADC_CONFIG_OS_SINGLE_CONV;
        send_buffer[1] = read_value;

        // Write changed MUX input back to the CONFIG register
        err = hk_adc_reg_write(HK_ADC_REG_CONFIG, &send_buffer[0]);

        if (err == HK_ADC_NO_ERR)
        {
            //INSERT DELAY FN (1sec), do not use WHILE Loops
            //Wait until OS bit turns 1
            do{
                hk_adc_reg_read(HK_ADC_REG_CONFIG, &read_value);
                os_bit = read_value >> 15;
            }while(os_bit == HK_ADC_CONFIG_OS_NOT_READY);

            err = hk_adc_reg_read(HK_ADC_REG_CONVERSION, &read_value);
            if (err == HK_ADC_NO_ERR)
            {
                // Right shift the received result by 4 bits. Conversion value are 12 most significant bits
                read_value = read_value >> 4;
                // make sure we have the most recent multiplier settings based on the currently configured PGA
                hk_adc_update_multiplier_volt();
                *citi_temp = ((float)read_value * multiplier_to_millivolts/VOLT_TO_MILLIVOLT_RATIO);    //result passed onto CITIROC UI should be in Volts (Vtemp)
                return err;
            }
        }
    }
    return err;
}




int hk_adc_reg_write(hk_adc_register_t reg, uint8_t *write_buffer)
{
    uint8_t send_buffer[3] = {0xff};
    uint8_t err = HK_ADC_ERR_WRITE_FAILED;

    // assign register address pointer
    switch(reg)
    {
        case HK_ADC_REG_CONVERSION:
            send_buffer[0] = HK_ADC_REG_PTR_CONVERSION;
            break;

        case HK_ADC_REG_CONFIG:
            send_buffer[0] = HK_ADC_REG_PTR_CONFIG;
            break;

        case HK_ADC_REG_LO_THRESH:
            send_buffer[0] = HK_ADC_REG_PTR_LOW_THRESH;
            break;

        case HK_ADC_REG_HI_THRESH:
            send_buffer[0] = HK_ADC_REG_PTR_HI_THRESH;
            break;

//        default:
            //no other register possible
    }

    send_buffer[1] = *write_buffer;         //MSB
    send_buffer[2] = *(write_buffer + 1);   //LSB

    // Write data to slave.
    MSS_I2C_write(&g_mss_i2c0, HK_ADC_ADDRESS_GND, &send_buffer[0], sizeof(send_buffer)/sizeof(send_buffer[0]), MSS_I2C_RELEASE_BUS);

    // Wait for completion and record the outcome
    status = MSS_I2C_wait_complete(&g_mss_i2c0, MSS_I2C_NO_TIMEOUT);

    if (status == MSS_I2C_SUCCESS)
    {
        err = HK_ADC_NO_ERR;
        return err;
    }

    err = HK_ADC_ERR_WRITE_FAILED;
    return err;
}


int hk_adc_reg_read(hk_adc_register_t reg, uint16_t *read_buffer)
{
    uint8_t send_buffer = 0xff;

    // assign register address pointer
    switch(reg)
    {
        case HK_ADC_REG_CONVERSION:
            send_buffer = HK_ADC_REG_PTR_CONVERSION;
            break;

        case HK_ADC_REG_CONFIG:
            send_buffer = HK_ADC_REG_PTR_CONFIG;
            break;

        case HK_ADC_REG_LO_THRESH:
            send_buffer = HK_ADC_REG_PTR_LOW_THRESH;
            break;

        case HK_ADC_REG_HI_THRESH:
            send_buffer = HK_ADC_REG_PTR_HI_THRESH;
            break;

//        default:
            //no other register possible
    }

    // Write to Address Pointer Register - pointing to 'reg'
    MSS_I2C_write(&g_mss_i2c0, HK_ADC_ADDRESS_GND, &send_buffer, TX_LENGTH, MSS_I2C_RELEASE_BUS);

    // Wait for completion and record the outcome
    status = MSS_I2C_wait_complete(&g_mss_i2c0, MSS_I2C_NO_TIMEOUT);

    // Proceed only if it is a success
    if (status == MSS_I2C_SUCCESS)
    {
        // Read data from target slave (we always read 2B)
        MSS_I2C_read(&g_mss_i2c0, HK_ADC_ADDRESS_GND, &rx_buffer[0], RX_LENGTH, MSS_I2C_RELEASE_BUS);

        status = MSS_I2C_wait_complete(&g_mss_i2c0, MSS_I2C_NO_TIMEOUT);

        if(status == MSS_I2C_SUCCESS)
        {
            read_value = rx_buffer[0] << 8 | rx_buffer[1];
            *read_buffer = read_value;
            return HK_ADC_NO_ERR;
        }
    }

    return HK_ADC_ERR_READ_FAILED;
}



void hk_adc_set_fsr(uint8_t fsr)
{
    fsr_setting = fsr;
    hk_adc_update_multiplier_volt();
}




void hk_adc_update_multiplier_volt(void)
{
    switch(fsr_setting)
    {
        case HK_ADC_CONFIG_FSR_1:
            multiplier_to_millivolts = 3.0F;    // corresponds to LSB = 3.0mV
            break;

        case HK_ADC_CONFIG_FSR_2:
            multiplier_to_millivolts = 2.0F;    // corresponds to LSB = 2.0mV
            break;

        case HK_ADC_CONFIG_FSR_3:
            multiplier_to_millivolts = 1.0F;    // corresponds to LSB = 1.0mV
            break;

        case HK_ADC_CONFIG_FSR_4:
            multiplier_to_millivolts = 0.5F;    // corresponds to LSB = 0.5mV
            break;

        case HK_ADC_CONFIG_FSR_5:
            multiplier_to_millivolts = 0.25F;    // corresponds to LSB = 0.25mV
            break;

        case HK_ADC_CONFIG_FSR_6:
            multiplier_to_millivolts = 0.125F;    // corresponds to LSB = 0.125mV
            break;

        default:
            multiplier_to_millivolts = 1.0F;    // corresponds to LSB = 0.1mV
    }
}
