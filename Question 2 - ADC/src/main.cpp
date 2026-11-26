/*
 * ECNG3006 - Lab 3, Question 2
 * Student: Karisa Kanchan
 * ID: 816043952
 * Arduino adapter only: all ADC routines and the actual test are C files.
 */
#include <Arduino.h>
#include <stdbool.h>
#include <stdint.h>

extern "C" bool adc_test_initialize(void);
extern "C" void adc_test_cycle(void);

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

    /* Recreate the visual ADC threshold behaviour from Lab 0. */
    digitalWrite(LED_BUILTIN, (passed && adc_value >= 2048u) ? HIGH : LOW);
}

void setup(void)
{
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    const unsigned long start_time = millis();
    while (!Serial && (millis() - start_time < 5000u)) {
        /* Allow serial monitor to connect, but do not hang indefinitely. */
    }
    Serial.println("ECNG3006 Lab 3 Q2: SAMD21 ADC C-routine test");
    Serial.println("Rotate potentiometer at A0 (0 V to 3.3 V).");
    Serial.println(adc_test_initialize() ? "ADC initialization: PASS" :
                                            "ADC initialization: FAIL");
}

void loop(void)
{
    adc_test_cycle();
    delay(500);
}
