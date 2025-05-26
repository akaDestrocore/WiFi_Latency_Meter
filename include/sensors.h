#ifndef SENSORS_H
#define SENSORS_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/resets.h"

// Number of ADC samples to average for temperature reading
#define TEMP_SAMPLE_COUNT 512

// function prototypes for ADC initialization and temperature reading
void temperature_init(void);
float temperature_read_celsius(void);

#endif /* SENSORS_H */