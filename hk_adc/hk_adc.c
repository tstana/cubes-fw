/*
 * HK-ADC functions
 *
 * Copyright © 2020 Theodor Stana, Sonal Shrivastava
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "../firmware/drivers/mss_gpio/mss_gpio.h"
#include "../firmware/drivers/mss_i2c/mss_i2c.h"
#include "hk_adc.h"


#define TX_LENGTH                       (1u)
#define RX_LENGTH                       (2u)
#define DUMMY_SERIAL_ADDR               (0xFF)
#define NUM_SAMPLES_TO_AVG              (10u)


/* Local Function prototypes */
static int hk_adc_reg_write(hk_adc_register_t reg, uint8_t *write_buffer);
static int hk_adc_reg_read(hk_adc_register_t reg, uint16_t *read_buffer);




/**
 * @brief HK ADC Init function
 *
 * This function is called once to initialize ADC with a certain configuration
 * - FSR - +-4.096, Single-shot conversion, DR = 1600SPS, MUX input CITI_TEMP and
 * disabled comparator.
 *
 *
 * @param none.
 * @return HK_ADC_ERR_INIT_FAILED if any error occurs otherwise HK_ADC_NO_ERR.
 */
int hk_adc_init(void)
{
    int err = HK_ADC_ERR_INIT_FAILED;

    // By default, ADC is configured to point towards CITI_TEMP as MUX input
    const uint16_t CONFIG = HK_ADC_CONFIG_MUX_SINGLE_2 | HK_ADC_CONFIG_FSR_2 |
                      HK_ADC_CONFIG_MODE_SINGLE_SHOT | HK_ADC_CONFIG_DR_1600SPS
                      | HK_ADC_CONFIG_CMODE_TRAD | HK_ADC_CONFIG_CPOL_ACTIVE_LOW
                      | HK_ADC_CONFIG_CLAT_NON_LATCH | HK_ADC_CONFIG_CQUE_DISABLE;

    uint8_t send_buffer[2] = {CONFIG>>8, CONFIG};

    // Set the clock for i2c0 instance; Clock div:10^8/256 = 390,625Hz =~ 390KHz
    MSS_I2C_init(&g_mss_i2c0, DUMMY_SERIAL_ADDR, MSS_I2C_PCLK_DIV_256);

    // Write to the CONFIG register
    err = hk_adc_reg_write(HK_ADC_REG_CONFIG, &send_buffer[0]);

    if (err == HK_ADC_NO_ERR)
    {
        return err;
    }

    err = HK_ADC_ERR_INIT_FAILED;
    return err;
}




/**
 * @brief HK ADC read Battery Voltage function
 *
 * This function reads the raw battery voltage value as read by the ADC and is
 * returned in the unsigned 16-bit variable pointer passed to the function as an
 * argument. ADC measures raw Vout value where Vout = V_Batt/8. V_Batt is found
 * by performing appropriate conversion on the ground software (CITIROC UI tool)
 * side.
 *
 * @param batt_volt[out]     pointer to unsigned 16-bit variable to store the result in.
 * @return HK_ADC_ERR_VOLT_READ_FAILED if any error occurs otherwise HK_ADC_NO_ERR.
 *
 */
int hk_adc_conv_read_volt(uint16_t * batt_volt)
{
    int err = HK_ADC_ERR_VOLT_READ_FAILED;

    uint8_t os_bit = 0;
    uint16_t read_value;

    uint8_t send_buffer[2] = {0};

    err = hk_adc_reg_read(HK_ADC_REG_CONFIG, &read_value);

    if (err == HK_ADC_NO_ERR)
    {
        // Let the rest of the configurations remain intact, just change MUX
        // input config and start single-shot conversion
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
                // Right shift the received result by 4 bits. Conversion value
                // are 12 most significant bits
                read_value = read_value >> 4;
                *batt_volt = read_value;
                return err;
            }
        }
    }

    err = HK_ADC_ERR_VOLT_READ_FAILED;
    return err;
}




/**
 * @brief HK ADC read Battery Current function
 *
 * This function reads the raw battery current value as read by the ADC and is
 * returned in the unsigned 16-bit variable pointer passed to the function as an
 * argument. ADC measures raw Vout value where Vout = I_Batt[Amp] * 2.5. I_Batt
 * is found by performing appropriate conversion on the ground software (CITIROC
 * UI tool) side.
 *
 * @param batt_curr[out]     pointer to unsigned 16-bit variable to store the result in.
 * @return HK_ADC_ERR_CURR_READ_FAILED if any error occurs otherwise HK_ADC_NO_ERR.
 *
 */
int hk_adc_conv_read_curr(uint16_t * batt_curr)
{
    uint8_t os_bit = 0;
    uint16_t read_value;
    uint8_t err = HK_ADC_ERR_CURR_READ_FAILED;

    uint8_t send_buffer[2] = {0};

    err = hk_adc_reg_read(HK_ADC_REG_CONFIG, &read_value);

    if (err == HK_ADC_NO_ERR)
    {
        // Let the rest of the configurations remain intact, just change MUX
        // input config and start single-shot conversion
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
                // Right shift the received result by 4 bits. Conversion value
                // are 12 most significant bits
                read_value = read_value >> 4;
                *batt_curr = read_value;
                return err;
            }
        }
    }

    err = HK_ADC_ERR_CURR_READ_FAILED;
    return err;
}




/**
 * @brief HK ADC read CITIROC temperature (deg. C) function
 *
 * This function reads the raw voltage value (Vtemp) as read by the ADC and is
 * returned in the unsigned 16-bit variable pointer passed to the function as an
 * argument. ADC measures raw Vtemp value where Vtemp is equivalent to CITIROC
 * ASIC temperature value. Conversion to actual temperature value is performed
 * by the ground software (CITIROC UI tool).
 *
 * @param citi_temp[out]     pointer to unsigned 16-bit variable to store the result in.
 * @return HK_ADC_ERR_TEMP_READ_FAILED if any error occurs otherwise HK_ADC_NO_ERR.
 *
 */

int hk_adc_conv_read_citi_temp(uint16_t * citi_temp)
{
    int err = HK_ADC_ERR_TEMP_READ_FAILED;

    uint8_t os_bit = 0;
    uint16_t read_value;

    uint8_t send_buffer[2] = {0};

    err = hk_adc_reg_read(HK_ADC_REG_CONFIG, &read_value);

    if (err == HK_ADC_NO_ERR)
    {
        // Let the rest of the configurations remain intact, just change MUX
        // input config and start single-shot conversion
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
                // Right shift the received result by 4 bits. Conversion value
                // are 12 most significant bits
                read_value = read_value >> 4;
                *citi_temp = read_value;
                return err;
            }
        }
    }

    err = HK_ADC_ERR_TEMP_READ_FAILED;
    return err;
}




/**
 * @brief HK ADC register write function
 *
 * This function performs register write operation. Usually, the value to be
 * written is a 16-bit value as all the registers accessible for a write
 * operation in HK ADC are 16-bit registers.
 *
 * @param reg[in]              Variable of enumeration type #hk_adc_register_t
 *                             (defined in hk_adc.h) indicating which register
 *                             is to be written.
 * @param write_buffer[in]     pointer to an unsigned byte array containing data
 *                             to be written.
 * @return HK_ADC_ERR_WRITE_FAILED if any error occurs otherwise HK_ADC_NO_ERR.
 *
 */
static int hk_adc_reg_write(hk_adc_register_t reg, uint8_t *write_buffer)
{
    int err = HK_ADC_ERR_WRITE_FAILED;

    mss_i2c_status_t status;
    uint8_t send_buffer[3] = {0xff};

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
    MSS_I2C_write(&g_mss_i2c0, HK_ADC_ADDRESS_GND, &send_buffer[0],
            sizeof(send_buffer)/sizeof(send_buffer[0]), MSS_I2C_RELEASE_BUS);

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




/**
 * @brief HK ADC register read function
 *
 * This function performs register read operation. Usually, the value to be
 * read is a 16-bit value as all the registers accessible for a read
 * operation in HK ADC are 16-bit registers.
 *
 * @param reg[in]              Variable of enumeration type #hk_adc_register_t
 *                             (defined in hk_adc.h) indicating which register
 *                             is to be read.
 * @param read_buffer[in]      pointer to a uint16_t variable to store the result
 *                             in.
 * @return HK_ADC_ERR_READ_FAILED if any error occurs otherwise HK_ADC_NO_ERR.
 *
 */
static int hk_adc_reg_read(hk_adc_register_t reg, uint16_t *read_buffer)
{
    int err = HK_ADC_ERR_READ_FAILED;

    mss_i2c_status_t status;
    uint8_t  rx_buffer[RX_LENGTH];
    uint16_t read_value;
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
    MSS_I2C_write(&g_mss_i2c0, HK_ADC_ADDRESS_GND, &send_buffer, TX_LENGTH,
                    MSS_I2C_RELEASE_BUS);

    // Wait for completion and record the outcome
    status = MSS_I2C_wait_complete(&g_mss_i2c0, MSS_I2C_NO_TIMEOUT);

    // Proceed only if it is a success
    if (status == MSS_I2C_SUCCESS)
    {
        // Read data from target slave (we always read 2B)
        MSS_I2C_read(&g_mss_i2c0, HK_ADC_ADDRESS_GND, &rx_buffer[0], RX_LENGTH,
                        MSS_I2C_RELEASE_BUS);

        status = MSS_I2C_wait_complete(&g_mss_i2c0, MSS_I2C_NO_TIMEOUT);

        if(status == MSS_I2C_SUCCESS)
        {
            read_value = rx_buffer[0] << 8 | rx_buffer[1];
            *read_buffer = read_value;
            err = HK_ADC_NO_ERR;
            return err;
        }
    }

    err = HK_ADC_ERR_READ_FAILED;
    return err;
}




/**
 * @brief HK ADC average voltage calculation function
 *
 * This function calculates the average battery voltage from raw ADC voltage
 * values. Number of samples to be averaged is 8.
 *
 * @param None.
 *
 * @return Average battery voltage raw value.
 *
 */
uint16_t hk_adc_calc_avg_voltage(void)
{
    uint16_t sum = 0;
    uint16_t volt_array[8];

    for (int i = 0; i < 8; i ++)
    {
        hk_adc_conv_read_volt(&volt_array[i]);
        sum += volt_array[i];
    }

    return (sum >> 3);
}




/**
 * @brief HK ADC average current calculation function
 *
 * This function calculates the average battery current from raw ADC voltage
 * values. Number of samples to be averaged is 8.
 *
 * @param None.
 *
 * @return Average battery current raw value.
 *
 */
uint16_t hk_adc_calc_avg_current(void)
{
    uint16_t sum = 0;
    uint16_t curr_array[8];

    for (int i = 0; i < 8; i ++)
    {
        hk_adc_conv_read_curr(&curr_array[i]);
        sum += curr_array[i];
    }

    return (sum >> 3);
}




/**
 * @brief HK ADC average CITIROC temperature calculation function
 *
 * This function calculates the average CITIROC temperature from raw ADC voltage
 * values. Number of samples to be averaged is 8.
 *
 * @param None.
 *
 * @return Average CITIROC temperature raw value.
 *
 */
uint16_t hk_adc_calc_avg_citi_temp(void)
{
    uint16_t sum = 0;
    uint16_t citi_temp_array[8];

    for (int i = 0; i < 8; i ++)
    {
        hk_adc_conv_read_citi_temp(&citi_temp_array[i]);
        sum += citi_temp_array[i];
    }

    return (sum >> 3);
}


