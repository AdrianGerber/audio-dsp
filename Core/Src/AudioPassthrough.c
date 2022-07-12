/*
 * AudioPassthrough.c
 *
 *  Created on: 12.07.2022
 *      Author: A. Gerber
 */

#include "AudioPasstrough.h"
#include "main.h"
#include "dma.h"
#include "i2s.h"
#include "cmsis_gcc.h"

/*
 * Basic I2S setup with DMA from https://audiodsplab.wordpress.com/ping-pong-buffer-audio-stream
 *
 * DMA continuously receives/transmits one half of the buffers s.t. processing can be done
 * on the other half.
 */

// 8 x uint16_t = 4 samples @ 24 bit (with 4 bytes unused, but needed by the DMA)
// --> processing in 2 blocks, each with left + right ADC value
#define BUFFERSIZE 8
static uint16_t TxBuffer[BUFFERSIZE];
static uint16_t RxBuffer[BUFFERSIZE];

// Default audio processing: output unmodified input stream
static Sample_t DefaultProcessingFunction(Sample_t in){ return in; }
static ProcessingFunction_t ProcessingFunction = DefaultProcessingFunction;

static Sample_t SampleFromBuffer(uint16_t* buffer){
   // 2x 24-bit signed audio sample --> 2x right-aligned int32_t
   Sample_t tmp;
   tmp.left = (((int32_t)buffer[0]<<16)|buffer[1])>>8;
   tmp.right = (((int32_t)buffer[2]<<16)|buffer[3])>>8;
   return tmp;
}

static void SampleToBuffer(Sample_t sample, uint16_t* buffer){
   // 2x right-aligned int32_t --> 2x 24-bit signed audio sample
   buffer[0] = (sample.left>>8)&0xFFFF;
   buffer[1] = sample.left & 0xFFFF;
   buffer[2] = (sample.right>>8)&0xFFFF;
   buffer[3] = sample.right & 0xFFFF;
}

void AudioPassthrough_Init(void){
   // Circular DMA will keep transmitting/receiving
   HAL_I2SEx_TransmitReceive_DMA(&hi2s2, TxBuffer, RxBuffer, BUFFERSIZE/2);
}

void AudioPassthrough_SetProcessingFunction(ProcessingFunction_t f){
   // Critical section
   uint32_t primask = __get_PRIMASK();
   __disable_irq();
   ProcessingFunction = f;
   __set_PRIMASK(primask);
}

// First half of buffer received --> processing can be done
void HAL_I2SEx_TxRxHalfCpltCallback(I2S_HandleTypeDef *hi2s){
   HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);

   Sample_t in = SampleFromBuffer(RxBuffer);
   Sample_t out = ProcessingFunction(in);
   SampleToBuffer(out, TxBuffer);
}

// Second half of buffer received --> processing can be done
void HAL_I2SEx_TxRxCpltCallback(I2S_HandleTypeDef *hi2s){
   HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);

   Sample_t in = SampleFromBuffer(RxBuffer + BUFFERSIZE/2);
   Sample_t out = ProcessingFunction(in);
   SampleToBuffer(out, TxBuffer + BUFFERSIZE/2);
}
