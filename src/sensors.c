#include "sensors.h"


/**
 * @brief Initialize the temperature sensor
 */
void temperature_init(void) {
    // Reset ADC first
    resets_hw->reset = (1u << RESET_ADC);
    // Wait for the reset to take effect
    while (!(resets_hw->reset & (1u << RESET_ADC))) {
        tight_loop_contents();
    }
    resets_hw->reset &= ~ (1u << RESET_ADC);

    // Enable ADC and wait for it to be ready
    adc_hw->cs = ADC_CS_EN_BITS;
    while (!(adc_hw->cs & ADC_CS_READY_BITS)) {
        tight_loop_contents();
    }

    // Enable temperature sensor
    adc_hw->cs |= ADC_CS_TS_EN_BITS;
}

/**
 * @brief Read the temperature from the internal sensor
 * @return Temperature in degrees Celsius
 */
float temperature_read_celsius() {
    // Select ADC CH4 for temperature sensor
    adc_hw->cs |= (4u << ADC_CS_AINSEL_LSB) & ADC_CS_AINSEL_BITS;
    sleep_ms(10);

    // Do multiple readings
    uint32_t sum = 0;
    for (int i = 0; i < TEMP_SAMPLE_COUNT; i++) {
        // Start a conversion
        adc_hw->cs |= ADC_CS_START_ONCE_BITS;
        while (!(adc_hw->cs & ADC_CS_READY_BITS)) {
            tight_loop_contents();
        }
        sum += (uint16_t) adc_hw->result;
        sleep_us(50);
    }
    
    // Convert to volts
    const float adc_voltage = (sum / (float)TEMP_SAMPLE_COUNT) * 3.3f / 4096.0f;
    
    // Convert volts to temperature using the formula from RP2040 datasheet
    return 27.0f - (adc_voltage - 0.706f) / 0.001721f;
}