#ifndef _ADC_H_
#define _ADC_H_
#include "stm32f10x.h"
#define ADC1_DR_Address    ((uint32_t)0x4001244C)
extern volatile uint16_t ADCConvertedValue[2];
/******************************************
*	���ܣ�ADC1��ʼ����ʹ��PC04���Ųɼ�
******************************************/
void AdcInit(void);

/****************************************************
*	���ܣ���ȡ�ɼ�����ADֵ
*	����ֵ���ɼ�����ֵ
****************************************************/
uint16_t AdcGetVal(void);


#endif