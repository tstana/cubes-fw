/*
 * hk_adc.h
 *
 *  Created on: 05-Mar-2021
 *      Author: Sonal Shrivastava
 */

#ifndef HK_ADC_HK_ADC_H_
#define HK_ADC_HK_ADC_H_


/* HK ADC Return Type Definitions */
typedef enum {
    HK_ADC_NO_ERR = 0,                  /* HK ADC no error occurred */
    HK_ADC_ERR_INIT_FAILED,             /* HK ADC Initialisation failed */
    HK_ADC_ERR_READ_FAILED,             /* HK ADC Read operation failed */
    HK_ADC_ERR_WRITE_FAILED,            /* HK ADC Write operation failed */
    HK_ADC_ERR_CONV_CONFIG_FAILED,      /* HK ADC Configuration operation for conversion register failed */
} hk_adc_return_t;


/* HK ADC Register Type Definitions */
typedef enum {
    HK_ADC_REG_CONVERSION = 0,      /* HK ADC Conversion register - Register Pointer bits P[1:0] in Address Pointer Register */
    HK_ADC_REG_CONFIG,              /* HK ADC Configuration register - Register Pointer bits P[1:0] in Address Pointer Register */
    HK_ADC_REG_LO_THRESH,           /* HK ADC Low Threshold register - Register Pointer bits P[1:0] in Address Pointer Register */
    HK_ADC_REG_HI_THRESH            /* HK ADC High Threshold register - Register Pointer bits P[1:0] in Address Pointer Register */
} hk_adc_reg_t;



/* HK ADC Conversion Type Definitions */
typedef enum {
    HK_ADC_READ_VBAT_I = 0x0100,         /* Read VBAT Current value stored in HK ADC Conversion register */
    HK_ADC_READ_VBAT_V,             /* Read VBAT Voltage value stored in HK ADC Conversion register */
    HK_ADC_READ_CITI_TEMP,          /* Read CITI Temperature value stored in HK ADC Conversion register */
    HK_ADC_READ_CITI_DAC_PROBE      /* Read CITI DAC Probe value stored in HK ADC Conversion register */
} hk_adc_conv_t;




int hk_adc_init(void);
int hk_adc_initialisation(void);
int hk_adc_reg_read(hk_adc_reg_t reg, uint16_t *read_buffer);
int hk_adc_reg_write(hk_adc_reg_t reg, uint8_t *write_buffer);
int hk_adc_reg_read_conv(uint16_t *read_buffer);
int hk_adc_conv_config(hk_adc_conv_t read_type);



#endif /* HK_ADC_HK_ADC_H_ */
