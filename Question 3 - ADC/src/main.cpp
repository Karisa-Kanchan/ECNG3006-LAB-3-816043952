
/*
 * ECNG3006 - Lab 3, Question 3
 * Student: Karisa Kanchan
 * ID: 816043952
 *
 * This file displays the ADC test results and timing
 * measurements through the Serial Monitor.
 */

#include <Arduino.h>
#include <stdbool.h>
#include <stdint.h>

extern "C" bool adc_test_initialize(void);
extern "C" void adc_test_cycle(void);

/* Display the ADC readings and test results. */
extern "C" void adc_test_report(uint32_t test_number,
                                uint16_t adc_value,
                                uint8_t sample_length,
                                bool passed)
{
    Serial.print("Test ");
    Serial.print(test_number);
    Serial.print(" | sample length: ");
    Serial.print(sample_length);
    Serial.print(" | ADC A0: ");
    Serial.print(adc_value);
    Serial.println(passed ? " | PASS" : " | FAIL");
}

/* Display the average and maximum execution times. */
extern "C" void adc_timing_report(const char *routine,
                                  const char *condition,
                                  uint32_t count,
                                  uint32_t average_ns,
                                  uint32_t maximum_ns)
{
    Serial.println("-----------------------------");

    Serial.print("Routine: ");
    Serial.println(routine);

    Serial.print("Condition: ");
    Serial.println(condition);

    Serial.print("Readings: ");
    Serial.println(count);

    Serial.print("Average time: ");
    Serial.print(average_ns / 1000.0f, 3);
    Serial.println(" us");

    Serial.print("Maximum time: ");
    Serial.print(maximum_ns / 1000.0f, 3);
    Serial.println(" us");
}

void setup(void)
{
    Serial.begin(115200);

    const unsigned long start_time = millis();

    while (!Serial && (millis() - start_time < 5000u)) {
        /* Wait briefly for the Serial Monitor. */
    }

    Serial.println("ECNG3006 Lab 3 - Question 3");
    Serial.println("ADC Execution Time Measurements");
    Serial.println("Using DC power supply connected to A0.");
    Serial.println("Running 200 timing tests...");

    Serial.println(adc_test_initialize() ?
                   "ADC initialization: PASS" :
                   "ADC initialization: FAIL");
}

void loop(void)
{
    adc_test_cycle();

    delay(30);
}
