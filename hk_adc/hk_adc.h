/*
 * HK-ADC functions header
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

#ifndef HK_ADC_HK_ADC_H_
#define HK_ADC_HK_ADC_H_


/* HK ADC 7-bit (unshifted) Address options */
#define HK_ADC_ADDRESS_GND                  0x48        /* used */
#define HK_ADC_ADDRESS_VDD                  0x49        /* not used */
#define HK_ADC_ADDRESS_SDA                  0x4A        /* not used */
#define HK_ADC_ADDRESS_SCL                  0x4B        /* not used */


/******* HK ADC register address pointer *******/

/* HK ADC Conversion reg - Address Pointer Register bits P[1:0] */
#define HK_ADC_REG_PTR_CONVERSION           (0x00)

/* HK ADC Configuration reg - Address Pointer Register bits P[1:0] */
#define HK_ADC_REG_PTR_CONFIG               (0x01)

/* HK ADC Low Threshold reg - Address Pointer Register bits P[1:0] */
#define HK_ADC_REG_PTR_LOW_THRESH           (0x02)

/* HK ADC High Threshold reg - Address Pointer Register bits P[1:0] */
#define HK_ADC_REG_PTR_HI_THRESH            (0x03)


/******* HK ADC operational status or single-shot conversion start *******/

/* HK ADC Config reg - OS_BIT (Bit 15) writing '0' has no effect */
#define HK_ADC_CONFIG_OS_NO_EFFECT          (0x0000)

/* HK ADC Config reg - OS_BIT (Bit 15) writing '1' starts a single conversion */
#define HK_ADC_CONFIG_OS_SINGLE_CONV        (0x8000)

/* HK ADC Config reg - OS_BIT (Bit 15) reading '0' means device is currently
 * performing a conversion */
#define HK_ADC_CONFIG_OS_NOT_READY          (0x0000)

/* HK ADC Config reg - OS_BIT (Bit 15) reading '1' means device is not performing
 *  a conversion */
#define HK_ADC_CONFIG_OS_READY              (0x8000)


/******* HK ADC device operating mode *******/

/* HK ADC Config register - MODE_BIT (Bit 8) '0' - Continuous conversion mode */
#define HK_ADC_CONFIG_MODE_CONTINUOUS       (0x0000)

/* HK ADC Config register - MODE_BIT (Bit 8) '1' - Single Shot conversion mode */
#define HK_ADC_CONFIG_MODE_SINGLE_SHOT      (0x0100)


/******* HK ADC input MUX configuration [Config register] MUX[2:0] (Bits 14:12) *******/

/* AIN_P = AIN0 and AIN_N = AIN1 (not used) */
#define HK_ADC_CONFIG_MUX_DIFF_P0_N1        (0x0000)

/* AIN_P = AIN0 and AIN_N = AIN3 (not used) */
#define HK_ADC_CONFIG_MUX_DIFF_P0_N3        (0x1000)

/* AIN_P = AIN1 and AIN_N = AIN3 (not used) */
#define HK_ADC_CONFIG_MUX_DIFF_P1_N3        (0x2000)

/* AIN_P = AIN2 and AIN_N = AIN3 (not used) */
#define HK_ADC_CONFIG_MUX_DIFF_P2_N3        (0x3000)

/* AIN_P = AIN0 and AIN_N = GND (VBAT_I) */
#define HK_ADC_CONFIG_MUX_SINGLE_0          (0x4000)

/* AIN_P = AIN1 and AIN_N = GND (VBAT_V) */
#define HK_ADC_CONFIG_MUX_SINGLE_1          (0x5000)

/* AIN_P = AIN2 and AIN_N = GND (CITI_TEMP) */
#define HK_ADC_CONFIG_MUX_SINGLE_2          (0x6000)

/* AIN_P = AIN3 and AIN_N = GND (CITI_DAC_PROBE) */
#define HK_ADC_CONFIG_MUX_SINGLE_3          (0x7000)


/******* HK ADC Data Rate configuration (SPS stands for samples per second)
 * [Config register] DR[2:0] (Bits 7:5) *******/

/* Data rate = 128 SPS */
#define HK_ADC_CONFIG_DR_128SPS             (0x0000)

/* Data rate = 250 SPS */
#define HK_ADC_CONFIG_DR_250SPS             (0x0020)

/* Data rate = 490 SPS */
#define HK_ADC_CONFIG_DR_490SPS             (0x0040)

/* Data rate = 920 SPS */
#define HK_ADC_CONFIG_DR_920SPS             (0x0060)

/* Data rate = 1600 SPS */
#define HK_ADC_CONFIG_DR_1600SPS            (0x0080)

/* Data rate = 2400 SPS */
#define HK_ADC_CONFIG_DR_2400SPS            (0x00A0)

/* Data rate = 3300 SPS */
#define HK_ADC_CONFIG_DR_3300SPS            (0x00C0)


/******* HK ADC programmable gain amplifier  (Full Scale Voltage (FSR) Range)
 * configuration [Config register] PGA[2:0] (Bits 11:9) *******/

/* FSR = -6.144 <= V <= +6.144 */
#define HK_ADC_CONFIG_FSR_1                 (0X0000)

/* FSR = -4.096 <= V <= +4.096 */
#define HK_ADC_CONFIG_FSR_2                 (0X0200)

/* FSR = -2.048 <= V <= +2.048 */
#define HK_ADC_CONFIG_FSR_3                 (0X0400)

/* FSR = -1.024 <= V <= +1.024 */
#define HK_ADC_CONFIG_FSR_4                 (0X0600)

/* FSR = -0.512 <= V <= +0.512 */
#define HK_ADC_CONFIG_FSR_5                 (0X0800)

/* FSR = -0.256 <= V <= +0.256 */
#define HK_ADC_CONFIG_FSR_6                 (0X0A00)


/******* HK ADC Comparator mode configuration [Config register] COMP_MODE (Bit 4) *******/

/* Traditional comparator (default) */
#define HK_ADC_CONFIG_CMODE_TRAD            (0x0000)

/* Window comparator */
#define HK_ADC_CONFIG_CMODE_WINDOW          (0x0010)


/******* HK ADC Comparator polarity configuration [Config register] COMP_POL (Bit 3) *******/

/* ALERT/RDY pin is low when active (default) */
#define HK_ADC_CONFIG_CPOL_ACTIVE_LOW       (0x0000)

/* ALERT/RDY pin is high when active */
#define HK_ADC_CONFIG_CPOL_ACTIVE_HI        (0x0008)


/******* HK ADC Comparator latching configuration [Config register] COMP_LAT (Bit 2) *******/

/* Non-latching comparator (default) - ALRT/RDY pin does not latch when asserted */
#define HK_ADC_CONFIG_CLAT_NON_LATCH        (0x0000)

/* Latching comparator - ALRT/RDY pin remains latched when asserted until conversion
 *  data is read by Master */
#define HK_ADC_CONFIG_CLAT_LATCH            (0x0004)


/******* HK ADC Comparator queue and disable configuration [Config register]
 * COMP_QUE[1:0] (Bits 1:0) *******/

/* Assert ALERT/RDY after one conversions */
#define HK_ADC_CONFIG_CQUE_1CONV            (0x0000)

/* Assert ALERT/RDY after two conversions */
#define HK_ADC_CONFIG_CQUE_2CONV            (0x0001)

/* Assert ALERT/RDY after four conversions */
#define HK_ADC_CONFIG_CQUE_4CONV            (0x0002)

/* Disable the comparator and put ALERT/RDY in high state (default) */
#define HK_ADC_CONFIG_CQUE_DISABLE          (0x0003)


/******* HK ADC Low and High Threshold Registers configuration to set ALRT/RDY
 * pin in conversion RDY mode *******/

#define HK_ADC_REG_LOW_THRESH_MSB            (0x00)

/* Lowest 4-bits are read-only (value - 0h) */
#define HK_ADC_REG_LOW_THRESH_LSB            (0x00)

#define HK_ADC_REG_HI_THRESH_MSB             (0x80)

/* Lowest 4-bits are read-only (value - 1h) */
#define HK_ADC_REG_HI_THRESH_LSB             (0x0f)



/******* HK ADC Return Type Definitions *******/
typedef enum {
    HK_ADC_NO_ERR = 0,                  /* HK ADC no error occurred */
    HK_ADC_ERR_INIT_FAILED,             /* HK ADC Initialisation failed */
    HK_ADC_ERR_READ_FAILED,             /* HK ADC read operation failed */
    HK_ADC_ERR_VOLT_READ_FAILED,        /* HK ADC battery voltage read operation failed */
    HK_ADC_ERR_CURR_READ_FAILED,        /* HK ADC battery current read operation failed */
    HK_ADC_ERR_TEMP_READ_FAILED,        /* HK ADC CITIROC temperature read operation failed */
    HK_ADC_ERR_WRITE_FAILED,            /* HK ADC Write operation failed */
    HK_ADC_ERR_CONV_CONFIG_FAILED       /* HK ADC Configuration operation for conversion register failed */
} hk_adc_return_t;



/******* HK ADC Register Definitions *******/
typedef enum {
    HK_ADC_REG_CONVERSION = 0,                  /* HK ADC Conversion register */
    HK_ADC_REG_CONFIG,                          /* HK ADC Configuration register */
    HK_ADC_REG_LO_THRESH,                       /* HK ADC Low Threshold register */
    HK_ADC_REG_HI_THRESH                        /* HK ADC High Threshold register */
} hk_adc_register_t;



/******* Exported Function prototypes *******/
int hk_adc_init(void);
int hk_adc_conv_read_volt(uint16_t * batt_volt);               //instantaneous value
int hk_adc_conv_read_curr(uint16_t * batt_curr);               //instantaneous value
int hk_adc_conv_read_citi_temp(uint16_t * citi_temp);          //instantaneous value
uint16_t hk_adc_calc_avg_voltage(void);                        //average value
uint16_t hk_adc_calc_avg_current(void);                        //average value
uint16_t hk_adc_calc_avg_citi_temp(void);                      //average value


#endif /* HK_ADC_HK_ADC_H_ */
