
/*
 * ECNG3006 - Lab 3, Question 3
 * Student: Karisa Kanchan 
 * ID: 816043952
 *
 * This program measures the execution time of two ADC routines:
 * set_adc_sample_length() and start_adc().
 *
 * It records the average and maximum execution times
 * using the SysTick timer on the Arduino MKR Zero.
 */

#include "adc_driver.h"
#include <sam.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#define TOTAL_TESTS 200u

/* These functions are implemented in main.cpp. */
extern void adc_test_report(uint32_t test_number,
                            uint16_t adc_value,
                            uint8_t sample_length,
                            bool passed);

extern void adc_timing_report(const char *routine,
                              const char *condition,
                              uint32_t count,
                              uint32_t average_ns,
                              uint32_t maximum_ns);

/* Store the timing results for each test condition. */
typedef struct {
    uint32_t count;
    uint64_t total_ticks;
    uint32_t maximum_ticks;
} TimingStats;

static TimingStats write_5 = {0};
static TimingStats write_20 = {0};
static TimingStats write_invalid = {0};
static TimingStats start_5 = {0};
static TimingStats start_20 = {0};

static uint32_t test_number = 0u;
static uint32_t timer_overhead = 0u;
static bool initialized = false;
static bool finished = false;

/*
 * Calculate elapsed clock ticks.
 * SysTick counts down and resets every millisecond.
 */
static uint32_t elapsed_ticks(uint32_t before,
                              uint32_t after)
{
    uint32_t period = SysTick->LOAD + 1u;

    if (before >= after) {
        return before - after;
    }

    return before + period - after;
}

/* Measure the overhead caused by reading the timer. */
static uint32_t measure_timer_overhead(void)
{
    uint32_t minimum = UINT32_MAX;

    for (uint32_t i = 0u; i < 50u; i++) {
        uint32_t before = SysTick->VAL;
        uint32_t after = SysTick->VAL;

        uint32_t ticks = elapsed_ticks(before, after);

        if (ticks < minimum) {
            minimum = ticks;
        }
    }

    return minimum;
}

/* Subtract the timer overhead from the measured time. */
static uint32_t corrected_ticks(uint32_t before,
                                 uint32_t after)
{
    uint32_t ticks = elapsed_ticks(before, after);

    if (ticks > timer_overhead) {
        return ticks - timer_overhead;
    }

    return 0u;
}

/* Update the number of readings, total time and maximum time. */
static void record_timing(TimingStats *stats, uint32_t ticks)
{
    stats->count++;
    stats->total_ticks += ticks;

    if (ticks > stats->maximum_ticks) {
        stats->maximum_ticks = ticks;
    }
}

/* Convert CPU clock ticks into nanoseconds. */
static uint32_t ticks_to_ns(uint64_t ticks)
{
    return (uint32_t)((ticks * 1000000000ULL) /
                      SystemCoreClock);
}

/* Calculate and display the average and maximum times. */
static void report_stats(const char *routine,
                         const char *condition,
                         const TimingStats *stats)
{
    if (stats->count == 0u) {
        return;
    }

    /* Calculate the average without losing fractional clock cycles. */
    uint32_t average_ns =
        (uint32_t)((stats->total_ticks * 1000000000ULL) /
               ((uint64_t)stats->count * SystemCoreClock));

    uint32_t maximum_ns =
        ticks_to_ns(stats->maximum_ticks);

    adc_timing_report(routine, condition, stats->count,
                      average_ns, maximum_ns);
}

/* Initialize the ADC and prepare the timer measurements. */
bool adc_test_initialize(void)
{
    initialized = init_adc();

    if (initialized) {
        timer_overhead = measure_timer_overhead();
    }

    return initialized;
}

/*
 * Run one ADC timing test.
 * Sampling lengths of 5 and 20 are alternated.
 * An invalid value of 64 is also tested.
 */
void adc_test_cycle(void)
{
    uint8_t sample_length;
    uint32_t timeout = 1000000u;
    uint32_t before;
    uint32_t after;
    uint16_t value = 0u;
    bool passed;
    bool invalid_result;

    if (!initialized || finished) {
        return;
    }

    test_number++;

    /* Alternate between sampling lengths of 5 and 20. */
    sample_length = ((test_number & 1u) != 0u) ? 5u : 20u;

    /*
     * Measure the time required to update
     * the ADC sampling length.
     */
    before = SysTick->VAL;
    passed = set_adc_sample_length(sample_length);
    after = SysTick->VAL;

    if (sample_length == 5u) {
        record_timing(&write_5,
                      corrected_ticks(before, after));
    } else {
        record_timing(&write_20,
                      corrected_ticks(before, after));
    }

    /*
     * Test an invalid sampling length.
     * The function should return false without
     * changing the previous ADC configuration.
     */
    before = SysTick->VAL;
    invalid_result = set_adc_sample_length(64u);
    after = SysTick->VAL;

    record_timing(&write_invalid,
                  corrected_ticks(before, after));

    if (invalid_result) {
        passed = false;
    }

    /* Measure how long start_adc() takes to execute. */
    if (passed) {
        before = SysTick->VAL;
        passed = start_adc();
        after = SysTick->VAL;

        if (sample_length == 5u) {
            record_timing(&start_5,
                          corrected_ticks(before, after));
        } else {
            record_timing(&start_20,
                          corrected_ticks(before, after));
        }
    }

    /* Wait for conversion to finish, then read the ADC value. */
    if (passed) {
        while (!is_done_adc() && (timeout > 0u)) {
            timeout--;
        }

        passed = is_done_adc();
    }

    if (passed) {
        value = singleread_adc();
        passed = (value <= 4095u);
    }

    /* Display progress every 20 tests or when a test fails. */
    if ((test_number % 20u == 0u) || !passed) {
        adc_test_report(test_number, value,
                        sample_length, passed);
    }

    /* Display the final results after 200 tests. */
    if (test_number >= TOTAL_TESTS) {
        report_stats("set_adc_sample_length", "length 5",
                     &write_5);

        report_stats("set_adc_sample_length", "length 20",
                     &write_20);

        report_stats("set_adc_sample_length", "invalid 64",
                     &write_invalid);

        report_stats("start_adc", "length 5",
                     &start_5);

        report_stats("start_adc", "length 20",
                     &start_20);

        finished = true;
    }
}
