/*
 * hk_adc.h
 *
 *  Created on: 05-Mar-2021
 *      Author: Sonal Shrivastava
 */

#ifndef HK_ADC_HK_ADC_H_
#define HK_ADC_HK_ADC_H_


/* HK ADC 7-bit (unshifted) Address options */
#define HK_ADC_ADDRESS_GND                  0x48        /* used */
#define HK_ADC_ADDRESS_VDD                  0x49        /* not used */
#define HK_ADC_ADDRESS_SDA                  0x4A        /* not used */
#define HK_ADC_ADDRESS_SCL                  0x4B        /* not used */

//#define OS_BIT_MASK 0x8000

/* HK ADC register address pointer */
#define HK_ADC_REG_PTR_CONVERSION           (0x00)      /* HK ADC Conversion register - Register Pointer bits P[1:0] in Address Pointer Register */
#define HK_ADC_REG_PTR_CONFIG               (0x01)      /* HK ADC Configuration register - Register Pointer bits P[1:0] in Address Pointer Register */
#define HK_ADC_REG_PTR_LOW_THRESH           (0x02)      /* HK ADC Low Threshold register - Register Pointer bits P[1:0] in Address Pointer Register */
#define HK_ADC_REG_PTR_HI_THRESH            (0x03)      /* HK ADC High Threshold register - Register Pointer bits P[1:0] in Address Pointer Register */


/* HK ADC operational status or single-shot conversion start - CHECK THIS */
#define HK_ADC_CONFIG_OS_NO_EFFECT          (0x0000)    /* HK ADC Config register - OS_BIT (Bit 15) writing '0' has no effect */
#define HK_ADC_CONFIG_OS_SINGLE_CONV        (0x8000)    /* HK ADC Config register - OS_BIT (Bit 15) writing '1' starts a single conversion */
#define HK_ADC_CONFIG_OS_NOT_READY          (0x0000)    /* HK ADC Config register - OS_BIT (Bit 15) reading '0' means device is currently performing a conversion */
#define HK_ADC_CONFIG_OS_READY              (0x8000)    /* HK ADC Config register - OS_BIT (Bit 15) reading '1' means device is not performing a conversion */


/* HK ADC device operating mode */
#define HK_ADC_CONFIG_MODE_CONTINUOUS       (0x0000)    /* HK ADC Config register - MODE_BIT (Bit 8) '0' - Continuous conversion mode */
#define HK_ADC_CONFIG_MODE_SINGLE_SHOT      (0x0100)    /* HK ADC Config register - MODE_BIT (Bit 8) '1' - Single Shot conversion mode */


/* HK ADC input MUX configuration - HK ADC Config register - MUX[2:0] (Bits 14:12) */
#define HK_ADC_CONFIG_MUX_DIFF_P0_N1        (0x0000)    /* AIN_P = AIN0 and AIN_N = AIN1 (not used) */
#define HK_ADC_CONFIG_MUX_DIFF_P0_N3        (0x1000)    /* AIN_P = AIN0 and AIN_N = AIN3 (not used) */
#define HK_ADC_CONFIG_MUX_DIFF_P1_N3        (0x2000)    /* AIN_P = AIN1 and AIN_N = AIN3 (not used) */
#define HK_ADC_CONFIG_MUX_DIFF_P2_N3        (0x3000)    /* AIN_P = AIN2 and AIN_N = AIN3 (not used) */
#define HK_ADC_CONFIG_MUX_SINGLE_0          (0x4000)    /* AIN_P = AIN0 and AIN_N = GND (VBAT_I) */
#define HK_ADC_CONFIG_MUX_SINGLE_1          (0x5000)    /* AIN_P = AIN1 and AIN_N = GND (VBAT_V) */
#define HK_ADC_CONFIG_MUX_SINGLE_2          (0x6000)    /* AIN_P = AIN2 and AIN_N = GND (CITI_TEMP) */
#define HK_ADC_CONFIG_MUX_SINGLE_3          (0x7000)    /* AIN_P = AIN3 and AIN_N = GND (CITI_DAC_PROBE) */


/* HK ADC Data Rate configuration (SPS stands for samples per second) - HK ADC Config register - DR[2:0] (Bits 7:5) */
#define HK_ADC_CONFIG_DR_128SPS             (0x0000)    /* Data rate = 128 SPS */
#define HK_ADC_CONFIG_DR_250SPS             (0x0020)    /* Data rate = 250 SPS */
#define HK_ADC_CONFIG_DR_490SPS             (0x0040)    /* Data rate = 490 SPS */
#define HK_ADC_CONFIG_DR_920SPS             (0x0060)    /* Data rate = 920 SPS */
#define HK_ADC_CONFIG_DR_1600SPS            (0x0080)    /* Data rate = 1600 SPS */
#define HK_ADC_CONFIG_DR_2400SPS            (0x00A0)    /* Data rate = 2400 SPS */
#define HK_ADC_CONFIG_DR_3300SPS            (0x00C0)    /* Data rate = 3300 SPS */


/* HK ADC programmable gain amplifier  (Full Scale Voltage (FSR) Range) configuration - HK ADC Config register - PGA[2:0] (Bits 11:9) */
#define HK_ADC_CONFIG_FSR_1                 (0X0000)    /* FSR = -6.144 <= V <= +6.144 */
#define HK_ADC_CONFIG_FSR_2                 (0X0200)    /* FSR = -4.096 <= V <= +4.096 */
#define HK_ADC_CONFIG_FSR_3                 (0X0400)    /* FSR = -2.048 <= V <= +2.048 */
#define HK_ADC_CONFIG_FSR_4                 (0X0600)    /* FSR = -1.024 <= V <= +1.024 */
#define HK_ADC_CONFIG_FSR_5                 (0X0800)    /* FSR = -0.512 <= V <= +0.512 */
#define HK_ADC_CONFIG_FSR_6                 (0X0A00)    /* FSR = -0.256 <= V <= +0.256 */


/* HK ADC Comparator mode configuration - HK ADC Config register - COMP_MODE (Bit 4) */
#define HK_ADC_CONFIG_CMODE_TRAD            (0x0000)    /* Traditional comparator (default) */
#define HK_ADC_CONFIG_CMODE_WINDOW          (0x0010)    /* Window comparator */


/* HK ADC Comparator polarity configuration - HK ADC Config register - COMP_POL (Bit 3) */
#define HK_ADC_CONFIG_CPOL_ACTIVE_LOW       (0x0000)    /* ALERT/RDY pin is low when active (default) */
#define HK_ADC_CONFIG_CPOL_ACTIVE_HI        (0x0008)    /* ALERT/RDY pin is high when active */


/* HK ADC Comparator latching configuration - HK ADC Config register - COMP_LAT (Bit 2) */
#define HK_ADC_CONFIG_CLAT_NON_LATCH        (0x0000)    /* Non-latching comparator (default) - ALRT/RDY pin does not latch when asserted */
#define HK_ADC_CONFIG_CLAT_LATCH            (0x0004)    /* Latching comparator - ALRT/RDY pin remains latched when asserted until conversion data is read by Master */


/* HK ADC Comparator queue and disable configuration - HK ADC Config register - COMP_QUE[1:0] (Bits 1:0) */
#define HK_ADC_CONFIG_CQUE_1CONV            (0x0000)    /* Assert ALERT/RDY after one conversions */
#define HK_ADC_CONFIG_CQUE_2CONV            (0x0001)    /* Assert ALERT/RDY after two conversions */
#define HK_ADC_CONFIG_CQUE_4CONV            (0x0002)    /* Assert ALERT/RDY after four conversions */
#define HK_ADC_CONFIG_CQUE_DISABLE          (0x0003)    /* Disable the comparator and put ALERT/RDY in high state (default) */



/* HK ADC Return Type Definitions */
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



/* HK ADC Register Type Definitions */
typedef enum {
    HK_ADC_REG_CONVERSION = 0,                  /* HK ADC Conversion register - Register Pointer bits P[1:0] in Address Pointer Register */
    HK_ADC_REG_CONFIG,                          /* HK ADC Configuration register - Register Pointer bits P[1:0] in Address Pointer Register */
    HK_ADC_REG_LO_THRESH,                       /* HK ADC Low Threshold register - Register Pointer bits P[1:0] in Address Pointer Register */
    HK_ADC_REG_HI_THRESH                        /* HK ADC High Threshold register - Register Pointer bits P[1:0] in Address Pointer Register */
} hk_adc_register_t;



/* HK ADC Conversion Type Definitions */
//typedef enum {
//    HK_ADC_READ_VBAT_I = 0x40,                 /* Read VBAT Current value stored in HK ADC Conversion register */
//    HK_ADC_READ_VBAT_V = 0x50,                 /* Read VBAT Voltage value stored in HK ADC Conversion register */
//    HK_ADC_READ_CITI_TEMP = 0x60,              /* Read CITI Temperature value stored in HK ADC Conversion register */
//    HK_ADC_READ_CITI_DAC_PROBE = 0x70          /* Read CITI DAC Probe value stored in HK ADC Conversion register */
//} hk_adc_mux_conversion_t;



/* HK ADC Full Scale Voltage Range definitions for programmable amplifier */
//typedef enum {
//    HK_ADC_FSR_0 = 0x0,                 /* FSR = -6.144 <= V <= +6.144 */
//    HK_ADC_FSR_1,                       /* FSR = -4.096 <= V <= +4.096 */
//    HK_ADC_FSR_2,                       /* FSR = -2.048 <= V <= +2.048 */
//    HK_ADC_FSR_3,                       /* FSR = -1.024 <= V <= +1.024 */
//    HK_ADC_FSR_4,                       /* FSR = -0.512 <= V <= +0.512 */
//    HK_ADC_FSR_5,                       /* FSR = -0.256 <= V <= +0.256 */
//    HK_ADC_FSR_6,                       /* FSR = -0.256 <= V <= +0.256 */
//    HK_ADC_FSR_7                        /* FSR = -0.256 <= V <= +0.256 */
//} hk_adc_full_scale_range_t;



/* HK ADC output data rate definitions. SPS stands for samples per second */
//typedef enum {
//    HK_ADC_DR_128_SPS = 0x0,            /* Output data rate = 128 SPS */
//    HK_ADC_DR_250_SPS,                  /* Output data rate = 250 SPS */
//    HK_ADC_DR_490_SPS,                  /* Output data rate = 490 SPS */
//    HK_ADC_DR_920_SPS,                  /* Output data rate = 920 SPS */
//    HK_ADC_DR_1600_SPS,                 /* Output data rate = 1600 SPS */
//    HK_ADC_DR_2400_SPS,                 /* Output data rate = 2400 SPS */
//    HK_ADC_DR_3300_SPS                  /* Output data rate = 3300 SPS */
//    HK_ADC_DR_3300_SPS                  /* Output data rate = 3300 SPS. Since, the SPS is same, use the previous enum value */
//} hk_adc_data_rate_t;



/* HK ADC operating mode definitions */
//typedef enum {
//    HK_ADC_OP_CONTINUOUS_CONV = 0x0,    /* ADC Operation mode - continuous conversion */
//    HK_ADC_OP_SINGLE_SHOT_CONV          /* ADC Operation mode - single-shot conversion */
//} hk_adc_operation_mode_t;







int hk_adc_init(void);
int hk_adc_initialisation(void);
int hk_adc_reg_read(hk_adc_register_t reg, uint16_t *read_buffer);
int hk_adc_reg_write(hk_adc_register_t reg, uint8_t *write_buffer);
int hk_adc_reg_read_conv(uint16_t *read_buffer);
//int hk_adc_conv_read_type(hk_adc_mux_conversion_t read_type);
//void hk_adc_start_conv(void);
int hk_adc_conv_read_volt(float * batt_volt);
int hk_adc_conv_read_curr(float * batt_curr);
int hk_adc_conv_read_citi_temp(uint16_t * citi_temp);




#endif /* HK_ADC_HK_ADC_H_ */
