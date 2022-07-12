/*
 * AudioPasstrough.h
 *
 *  Created on: 12.07.2022
 *      Author: A. Gerber
 */

#ifndef INC_AUDIOPASSTROUGH_H_
#define INC_AUDIOPASSTROUGH_H_

#include <stdint.h>

/*
 * Hardware:
 * - STM32F401 Nucleo-64 (STM32F401RET6)
 * - Digilent PMOD I2S2 (https://digilent.com/reference/pmod/pmodi2s2/reference-manual)
 *
 * Audio Configuration:
 * - 24 bit stereo @ 48kHz, ADC + DAC
 *
 * Connections:
 * (MCLK, LRCK and SCLK of ADC and DAC are connected in parallel)
 *
 * I2S2        STM32
 * MCLK  <---> I2S2_MCK (PC6)
 * LRCK  <---> I2S2_WS (PB12)
 * SCLK  <---> I2S2_CK (PB10)
 * SDOUT <---> I2S2_ext_SD (PC2)
 * SDIN  <---> I2S2_SD (PC3)
 * GND   <---> GND
 * VCC   <---> 3.3V
 *
 */

typedef struct{
   int32_t left;
   int32_t right;
} Sample_t;

typedef Sample_t (*ProcessingFunction_t)(Sample_t);

// Setup audio hardware and begin passthrough
void AudioPassthrough_Init(void);

// Set function that processes the audio stream (called for each sample, in 48kHz interrupt!)
void AudioPassthrough_SetProcessingFunction(ProcessingFunction_t f);

#endif /* INC_AUDIOPASSTROUGH_H_ */
