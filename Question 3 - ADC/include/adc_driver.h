/*
 * ECNG3006 - Lab 3, Question 2 (reused for Question 3)
 * Student: Karisa Kanchan
 * ID: 816043952
 * Peripheral: SAMD21 built-in ADC (A0 / PA02)
 */
#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include <sam.h>      /* SAMD21 device register definitions */
#include <stdbool.h>
#include <stdint.h>

/* Initialize ADC AIN0, 12-bit, VDDANA input range. */
bool init_adc(void);

/* Write sampling time (0-63 ADC clock cycles). */
bool set_adc_sample_length(uint8_t sample_length);

/* Begin one conversion. */
bool start_adc(void);

/* True when a conversion result is available. */
bool is_done_adc(void);

/* Read the completed 12-bit conversion result (0-4095). */
uint16_t singleread_adc(void);

#endif /* ADC_DRIVER_H */
