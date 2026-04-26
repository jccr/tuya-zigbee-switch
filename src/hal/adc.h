#ifndef _HAL_ADC_H_
#define _HAL_ADC_H_

#include "hal/gpio.h"
#include <stdint.h>

typedef enum {
    HAL_ADC_INPUT_PIN,  // ADC from external input
    HAL_ADC_INPUT_VBAT, // ADC from internal voltage bus
} hal_adc_input_t;

void hal_adc_init(hal_adc_input_t input, hal_gpio_pin_t pin);

uint16_t hal_adc_read_mv();

/**
 * Read the voltage (in mV) of a specific GPIO pin using ADC.
 * This function is useful for reading switch inputs where digital GPIO
 * reads may be unreliable due to electrical noise.
 *
 * @param pin GPIO pin to read
 * @return Voltage in millivolts, or 0 if reading failed
 */
uint16_t hal_adc_read_pin_mv(hal_gpio_pin_t pin);

#endif
