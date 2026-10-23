/* EEE3096S Practical 3: complete the four TODOs in this file.
 * See ../../../docs/prac_03.md and ST RM0091, ADC and DAC chapters.
 * Use CMSIS register names; clock, GPIO and startup helpers are supplied.
 */
#include "voltage_mirror.h"
#include "board_io.h"
#include "main.h"

#define CODE_MAX        4095u
#define ADC_WAIT_LIMIT  100000u

bool voltage_mirror_init(void)
{
    if (!board_converters_prepare()) return false;

    /* TODO 1: ADC1: 12-bit, right-aligned, software-started single conversions,
     * channel 5 only, sampling time 239.5 ADC cycles.
     * DAC channel 1: buffer on, trigger off, initial command 2048, enable.
     * Set ADC1->CFGR1, ADC1->CHSELR, ADC1->SMPR, DAC->DHR12R1 and DAC->CR.
     * Finish by returning board_adc_enable(). Remove the false return below.
     */
    ADC1->CFGR1 &= ~(ADC_CFGR1_RES | ADC_CFGR1_ALIGN | ADC_CFGR1_CONT | ADC_CFGR1_EXTEN | ADC_CFGR1_DISCEN);

    ADC1->CHSELR = ADC_CHSELR_CHSEL5;
    ADC1->SMPR   = ADC_SMPR_SMP_0 | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_2;

    DAC->DHR12R1 = 2048u;
    DAC->CR &= ~(DAC_CR_BOFF1 | DAC_CR_TEN1 | DAC_CR_TSEL1);

    DAC->CR |= DAC_CR_EN1;

    return board_adc_enable(); /* The uncompleted starter intentionally does not acquire. */
}

bool voltage_mirror_read(uint16_t *code)
{
    /* TODO 2: reject a null pointer; clear EOC/EOS/OVR flags with W1C writes;
     * start one conversion, wait for EOC with a bounded loop, read ADC1->DR.
     * Return true only when *code contains a real acquired 12-bit result.
     * On timeout request ADSTP, then return false. Never wait forever.
     */
    if (code == 0) return false;

        ADC1->ISR = ADC_ISR_EOC | ADC_ISR_EOSEQ | ADC_ISR_OVR;

        ADC1->CR |= ADC_CR_ADSTART;

        uint32_t guard = ADC_WAIT_LIMIT;
        while (((ADC1->ISR & ADC_ISR_EOC) == 0u) && (guard > 0u)) {
            guard--;
        }

        if ((ADC1->ISR & ADC_ISR_EOC) == 0u) {

            ADC1->CR |= ADC_CR_ADSTP;
            guard = ADC_WAIT_LIMIT;
            while (((ADC1->CR & ADC_CR_ADSTP) != 0u) && (guard > 0u)) {
                guard--;
            }
            ADC1->ISR = ADC_ISR_EOC | ADC_ISR_EOSEQ | ADC_ISR_OVR;
            return false;
        }

        uint16_t result = (uint16_t)ADC1->DR;
        ADC1->ISR = ADC_ISR_EOC | ADC_ISR_EOSEQ | ADC_ISR_OVR;

        if (result > CODE_MAX) result = CODE_MAX;
        *code = result;
        return true;
}

void voltage_mirror_write(uint16_t code)
{
    /* TODO 3: limit code to 0..4095 and write DAC->DHR12R1. */
	if (code > CODE_MAX) code = CODE_MAX;
	    DAC->DHR12R1 = code;
}

uint16_t voltage_mirror_map(uint16_t code, bool coarse)
{
    /* TODO 4: limit code to 0..4095. FULL passes it through; COARSE keeps
     * the six most significant bits and clears the six least significant.
     * Retain the 12-bit DAC scale: do not send a 0..63 code directly.
     */
	if (code > CODE_MAX) code = CODE_MAX;
	    if (coarse) {
	        return (uint16_t)(code & 0xFFC0u);
	    }
    return code;
}
