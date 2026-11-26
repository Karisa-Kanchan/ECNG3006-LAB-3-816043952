
/*
 * ECNG3006 - Lab 3, Question 2
 * Student: Karisa Kanchan
 * ID: 816043952
 *
 * This program tests the ADC by repeatedly changing the
 * sampling length, starting conversions, and reading the
 * converted values to check that the ADC works correctly.
 */

#include "adc_driver.h"
#include <stdbool.h>
#include <stdint.h>

/* Function used to display test results through main.cpp. */
extern void adc_test_report(uint32_t test_number,
                            uint16_t adc_value,
                            uint8_t sample_length,
                            bool passed);

/* Keep track of the number of tests and ADC initialization status. */
static uint32_t test_number = 0u;
static bool initialized = false;

/*
 * Initialize the ADC before running the tests.
 * Returns true if initialization was successful.
 */
bool adc_test_initialize(void)
{
    initialized = init_adc();
    return initialized;
}

/*
 * Run one ADC test cycle.
 * Each cycle changes the sampling length, starts a conversion,
 * reads the result, and checks if the test passed.
 */
void adc_test_cycle(void)
{
    uint8_t sample_length;
    uint32_t timeout = 1000000u;
    bool passed;
    uint16_t value = 0u;

    /* Do not run the test if the ADC was not initialized. */
    if (!initialized) {
        adc_test_report(test_number, value, 0u, false);
        return;
    }

    test_number++;

    /* Alternate between sampling lengths of 5 and 20 for each test. */
    sample_length = ((test_number & 1u) != 0u) ? 5u : 20u;

    /* Write the new sampling length to the ADC. */
    passed = set_adc_sample_length(sample_length);

    /* Start a conversion if the sampling length was set correctly. */
    if (passed) {
        passed = start_adc();
    }

    /*
     * Wait for the conversion to finish.
     * A timeout is used to prevent the program from waiting forever.
     */
    if (passed) {
        while (!is_done_adc() && (timeout > 0u)) {
            timeout--;
        }
        passed = is_done_adc();
    }

    /* Read the ADC result and check that it is within the 12-bit range. */
    if (passed) {
        value = singleread_adc();
        passed = (value <= 4095u);
    }

    /* Send the test number, ADC value, sampling length, and result to main.cpp. */
    adc_test_report(test_number, value, sample_length, passed);
}
