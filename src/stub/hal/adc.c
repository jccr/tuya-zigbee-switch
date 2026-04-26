#include "hal/adc.h"

// Default high voltage when GPIO is pulled up (3.3V)
#define STUB_ADC_HIGH_MV    3300
// Default low voltage when GPIO is pulled down
#define STUB_ADC_LOW_MV     0

static uint16_t stub_adc_voltage_mv = 3000;

// For simulating pin-specific ADC readings in tests
// Uses the GPIO value to determine voltage: high = 3300mV, low = 0mV
extern uint8_t stub_gpio_get_output(uint16_t gpio_pin);

void hal_adc_init(hal_adc_input_t input, hal_gpio_pin_t pin) {
    (void)input;
    (void)pin;
}

uint16_t hal_adc_read_mv() {
    return stub_adc_voltage_mv;
}

uint16_t hal_adc_read_pin_mv(hal_gpio_pin_t pin) {
    if (pin == HAL_INVALID_PIN) {
        return 0;
    }
    // In stub mode, simulate ADC reading based on GPIO value
    // This assumes tests use stub_gpio_simulate_input to set pin state
    uint8_t gpio_value = stub_gpio_get_output(pin);
    return gpio_value ? STUB_ADC_HIGH_MV : STUB_ADC_LOW_MV;
}

void stub_set_adc_voltage_mv(uint16_t voltage_mv) {
    stub_adc_voltage_mv = voltage_mv;
}

void stub_set_battery_voltage_mv(uint16_t voltage_mv) {
    stub_set_adc_voltage_mv(voltage_mv);
}
