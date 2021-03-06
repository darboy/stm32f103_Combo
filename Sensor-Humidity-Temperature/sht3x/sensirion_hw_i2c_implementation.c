/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stm32f10x.h>
//#include "stm32f1xx_hal_i2c.h"

#include "sensirion_arch_config.h"
#include "sensirion_i2c.h"
#include "stdio.h"
#include"RS232.H"
#include "stdio.h"

/**
 * Create new I2C instance. You may also use a different interface, e.g. hI2C1,
 * depending on your CubeMX configuration
 */
 /*----------------------------------------------------------------------------------------------------------------------------------------------------*/
 static DMA_InitTypeDef dma;
 
 void start(void)
{
	if (I2C1->SR1 || I2C1->SR2)
	printf("error!\n");

restart:
	I2C_GenerateSTART(I2C1, ENABLE);
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) == ERROR); // ??????????
	I2C_SendData(I2C1, 0x44); 
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR) // ??????
	{
		if (I2C_GetFlagStatus(I2C1, I2C_FLAG_AF) == SET)
		{
			I2C_ClearFlag(I2C1, I2C_FLAG_AF);
		//printf("NACK!\n");
			goto restart;
		}
	}
}
 
s8 read(uint8_t addr, u8 *data, uint16_t len)
{
	dma.DMA_BufferSize = len;
	dma.DMA_DIR = DMA_DIR_PeripheralSRC;
	dma.DMA_MemoryBaseAddr = (uint32_t)data;
	DMA_Init(DMA1_Channel7, &dma);
	DMA_Cmd(DMA1_Channel7, ENABLE);
	start();
	I2C_SendData(I2C1, addr);
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR);
	I2C_GenerateSTART(I2C1, ENABLE);
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);
	I2C_SendData(I2C1, addr);
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) == ERROR);
	I2C_DMACmd(I2C1, ENABLE);
	I2C_DMALastTransferCmd(I2C1, ENABLE);
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	while (DMA_GetFlagStatus(DMA1_FLAG_TC7) == RESET);
	DMA_ClearFlag(DMA1_FLAG_TC7);
	DMA_Cmd(DMA1_Channel7, DISABLE);
	I2C_GenerateSTOP(I2C1, ENABLE);
	I2C_DMACmd(I2C1, DISABLE);
	I2C_DMALastTransferCmd(I2C1, DISABLE);
	I2C_AcknowledgeConfig(I2C1, DISABLE);
	while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) == SET);
	return 0;
}

 s8 write(uint8_t addr, const u8 *data, uint8_t len) 
{
	dma.DMA_BufferSize = len;
	dma.DMA_DIR = DMA_DIR_PeripheralDST;
	dma.DMA_MemoryBaseAddr = (uint32_t)data;
	DMA_Init(DMA1_Channel6, &dma);
	DMA_Cmd(DMA1_Channel6, ENABLE);
	start();
	I2C_SendData(I2C1, addr);
	I2C_DMACmd(I2C1, ENABLE);
	while (DMA_GetFlagStatus(DMA1_FLAG_TC6) == RESET);
	DMA_ClearFlag(DMA1_FLAG_TC6);
	DMA_Cmd(DMA1_Channel6, DISABLE);
	while (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) == ERROR); // ??????????
	I2C_GenerateSTOP(I2C1, ENABLE);
	I2C_DMACmd(I2C1, DISABLE);
	while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) == SET);
	return 0;
}

 /*----------------------------------------------------------------------------------------------------------------------------------------------------*/
 
 //��д����
 /*----------------------------------------------------------------------------------------------------------------------------------------------------*/
 void I2C_WriteByte(uint8_t id, uint8_t w_addr, uint8_t* p_data)
{
	I2C_GenerateSTART(I2C1, ENABLE);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	I2C_Send7bitAddress(I2C1, id, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	I2C_SendData(I2C1, w_addr);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	I2C_SendData(I2C1, *p_data);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	I2C_GenerateSTOP(I2C1, ENABLE);
}

s8 I2C_WriteBlock(uint8_t w_addr, const uint8_t* p_data, uint16_t len)
{
	//__disable_irq();
	
	uint16_t i = 0;
	I2C_GenerateSTART(I2C1, ENABLE);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	
	I2C_Send7bitAddress(I2C1,(w_addr << 1), I2C_Direction_Transmitter);
	//printf("address:%x",w_addr);
	//delay_us(9);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	
	while (i < len) {
		I2C_SendData(I2C1, *p_data);
		while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED | I2C_EVENT_MASTER_BYTE_TRANSMITTING));
		i++; p_data++;
	}
	I2C_GenerateSTOP(I2C1, ENABLE);
	//__enable_irq();
	return 0;
}

s8 I2C_ReadBlock(uint8_t r_addr, uint8_t* p_data, uint16_t len)
{
	while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	I2C_GenerateSTART(I2C1, ENABLE);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	I2C_Send7bitAddress(I2C1, (r_addr << 1), I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	I2C_Cmd(I2C1, ENABLE);
	I2C_SendData(I2C1,r_addr);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	I2C_GenerateSTART(I2C1, ENABLE);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	I2C_Send7bitAddress(I2C1,(r_addr << 1), I2C_Direction_Receiver);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	while (len)
	{
		if (len == 1)
		{
			I2C_AcknowledgeConfig(I2C1, DISABLE);
			I2C_GenerateSTOP(I2C1, ENABLE);
		}
		if (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))
		{
			*p_data = I2C_ReceiveData(I2C1);
			p_data++;
			len--;
		}
	}
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	return 0;
}

void I2C_ReadByte(uint8_t id , uint8_t r_addr, uint8_t* p_data)
{
	I2C_ReadBlock(r_addr, p_data, 1);
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------------*/

static I2C_InitTypeDef hI2C1;
/**
 * Initialize all hard- and software components that are needed for the I2C
 * communication.
 */
void sensirion_i2c_init()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	GPIO_InitTypeDef GPIO_InitS;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	//GPIO_PinRemapConfig(GPIO_Remap_I2C1,ENABLE);
	
	GPIO_InitS.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitS.GPIO_Speed = GPIO_Speed_2MHz;
	//GPIO_InitS.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitS.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(GPIOB, &GPIO_InitS);
	
	
	
    
	I2C_Cmd(I2C1, DISABLE);
	hI2C1.I2C_Mode = I2C_Mode_I2C;
	hI2C1.I2C_DutyCycle = I2C_DutyCycle_2;
	hI2C1.I2C_OwnAddress1 = 0;
	hI2C1.I2C_Ack = I2C_Ack_Enable;
	hI2C1.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    hI2C1.I2C_ClockSpeed = 100000;
	I2C_Cmd(I2C1, ENABLE);
	I2C_Init(I2C1, &hI2C1);
    /* Enable the remapping of Pins 6/7 to 8/9 and the I2C clock before the
     * initialization of the GPIO Pins in HAL_I2C_Init(). This is a fix of the
     * code generated by CubeMX v4.16.0 */
	
}

/**
 * Execute one read transaction on the I2C bus, reading a given number of bytes.
 * If the device does not acknowledge the read command, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to read from
 * @param data    pointer to the buffer where the data is to be stored
 * @param count   number of bytes to read from I2C and store in the buffer
 * @returns 0 on success, error code otherwise
 */
s8 sensirion_i2c_read(u8 address, u8 *data, u16 count)
{
    return (s8) I2C_ReadBlock(address, data, count);
}

/**
 * Execute one write transaction on the I2C bus, sending a given number of bytes.
 * The bytes in the supplied buffer must be sent to the given address. If the
 * slave device does not acknowledge any of the bytes, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to write to
 * @param data    pointer to the buffer containing the data to write
 * @param count   number of bytes to read from the buffer and send over I2C
 * @returns 0 on success, error code otherwise
 */
s8 sensirion_i2c_write(u8 address, const u8 *data, u16 count)
{
    return (s8) I2C_WriteBlock(address, data, count);
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * @param useconds the sleep time in microseconds
 */
void sensirion_sleep_usec(u32 useconds) {
//    u32 msec = useconds / 1000;
//    if (useconds % 1000 > 0) {
//        msec++;
//    }

//    /*
//     * Increment by 1 if STM32F1 driver version less than 1.1.1
//     * Old firmwares of STM32F1 sleep 1ms shorter than specified in HAL_Delay.
//     * This was fixed with firmware 1.6 (driver version 1.1.1), so we have to
//     * fix it ourselves for older firmwares
//     */
//    if (HAL_GetHalVersion() < 0x01010100) {
//        msec++;
//    }

//    HAL_Delay(msec);
}









