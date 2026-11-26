
/*
 * ECNG3006 - Lab 3, Question 2
 * Student: Karisa Kanchan
 * ID: 816043952
 *
 * ADC driver for the Arduino MKR Zero (ATSAMD21G18A).
 * This code configures the built-in ADC to read analog
 * values from A0 and convert them into 12-bit values.
 */

#include "adc_driver.h"
#include <sam.h>

/*
 * Wait for ADC synchronization before continuing.
 * This is needed because the CPU and ADC use different clocks.
 */
static void wait_adc_sync(void)
{
    while (ADC->STATUS.bit.SYNCBUSY != 0u) {
        /* Wait until ADC is ready. */
    }
}

/*
 * Initialize the ADC by setting up the clocks, input pin,
 * calibration values, and conversion settings.
 */
bool init_adc(void)
{
    uint32_t bias;
    uint32_t linearity;

    /* Enable the bus clock and generic clock for the ADC. */
    PM->APBCMASK.reg |= PM_APBCMASK_ADC;

    while ((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) != 0u) {
    }

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_ADC |
                    GCLK_CLKCTRL_GEN_GCLK0 |
                    GCLK_CLKCTRL_CLKEN;

    while ((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) != 0u) {
    }

    /* Configure A0 (PA02) as the analog input using ADC AIN0. */
    PORT->Group[0].PINCFG[2].bit.PMUXEN = 1u;
    PORT->Group[0].PMUX[1].bit.PMUXE = 1u;

    /* Reset the ADC so it does not use previous settings. */
    wait_adc_sync();
    ADC->CTRLA.bit.ENABLE = 0u;
    wait_adc_sync();

    ADC->CTRLA.bit.SWRST = 1u;

    while ((ADC->STATUS.bit.SYNCBUSY != 0u) ||
           (ADC->CTRLA.bit.SWRST != 0u)) {
    }

    /* Read the factory calibration values for ADC accuracy. */
    bias = (*((const uint32_t *)ADC_FUSES_BIASCAL_ADDR) &
            ADC_FUSES_BIASCAL_Msk) >> ADC_FUSES_BIASCAL_Pos;

    linearity = (*((const uint32_t *)ADC_FUSES_LINEARITY_0_ADDR) &
                 ADC_FUSES_LINEARITY_0_Msk) >> ADC_FUSES_LINEARITY_0_Pos;

    linearity |= (((*((const uint32_t *)ADC_FUSES_LINEARITY_1_ADDR) &
                    ADC_FUSES_LINEARITY_1_Msk) >> ADC_FUSES_LINEARITY_1_Pos)
                  << 5u);

    ADC->CALIB.reg = ADC_CALIB_BIAS_CAL(bias) |
                     ADC_CALIB_LINEARITY_CAL(linearity);

    /*
     * Configure the ADC for 12-bit resolution (0-4095).
     * Use the internal reference and gain of 1/2 for an
     * input range of approximately 0-3.3 V.
     */
    ADC->REFCTRL.reg = ADC_REFCTRL_REFSEL_INTVCC1;
    ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1;
    ADC->SAMPCTRL.reg = ADC_SAMPCTRL_SAMPLEN(5u);

    ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV64 |
                     ADC_CTRLB_RESSEL_12BIT;

    wait_adc_sync();

    /* Select AIN0 as the positive input and ground as negative. */
    ADC->INPUTCTRL.reg = ADC_INPUTCTRL_GAIN_DIV2 |
                         ADC_INPUTCTRL_MUXNEG_GND |
                         ADC_INPUTCTRL_MUXPOS_PIN0;

    wait_adc_sync();

    /* Clear the previous result flag and enable the ADC. */
    ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;
    ADC->CTRLA.bit.ENABLE = 1u;
    wait_adc_sync();

    /* Perform an initial conversion and discard the result. */
    ADC->SWTRIG.bit.START = 1u;
    wait_adc_sync();

    while (ADC->INTFLAG.bit.RESRDY == 0u) {
    }

    (void)ADC->RESULT.reg;
    ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;

    /* Return true if the ADC was enabled successfully. */
    return (ADC->CTRLA.bit.ENABLE != 0u);
}

/*
 * Change the ADC sampling length.
 * Valid values are between 0 and 63.
 */
bool set_adc_sample_length(uint8_t sample_length)
{
    /* Check that the sampling length is within range. */
    if (sample_length > 63u) {
        return false;
    }

    /* Disable the ADC before changing the sampling setting. */
    wait_adc_sync();
    ADC->CTRLA.bit.ENABLE = 0u;
    wait_adc_sync();

    ADC->SAMPCTRL.reg = ADC_SAMPCTRL_SAMPLEN(sample_length);

    /* Enable the ADC again after updating the setting. */
    ADC->CTRLA.bit.ENABLE = 1u;
    wait_adc_sync();

    /* Check that the new sampling length was stored correctly. */
    return (ADC->SAMPCTRL.bit.SAMPLEN == sample_length);
}

/*
 * Start a new ADC conversion.
 * Returns false if the ADC has not been enabled.
 */
bool start_adc(void)
{
    if (ADC->CTRLA.bit.ENABLE == 0u) {
        return false;
    }

    wait_adc_sync();

    /* Clear the previous result flag before starting. */
    ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;

    ADC->SWTRIG.bit.START = 1u;
    wait_adc_sync();

    return true;
}

/*
 * Check whether the ADC conversion has finished.
 * Returns true when a new result is available.
 */
bool is_done_adc(void)
{
    return (ADC->INTFLAG.bit.RESRDY != 0u);
}

/*
 * Read the converted ADC value.
 * The result is a 12-bit value between 0 and 4095.
 */
uint16_t singleread_adc(void)
{
    /* Read the result and clear the flag for the next conversion. */
    uint16_t result = ADC->RESULT.reg;
    ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;

    return (uint16_t)(result & 0x0FFFu);
}
