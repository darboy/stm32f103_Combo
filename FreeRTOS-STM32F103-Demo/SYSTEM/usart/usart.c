#include "sys.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "portable.h"
#include "FreeRTOSConfig.h"
#include "tools.h"
  /**************************************************************************
作者：平衡小车之家
我的淘宝小店：http://shop114407458.taobao.com/
**************************************************************************/
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif
//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);
	USART1->DR = (u8) ch;      
	return ch;
}
#endif 
void uart_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
		
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

	USART_Init(USART1, &USART_InitStructure); //初始化串口1
	USART_Cmd(USART1, ENABLE);                    //使能串口1 
	USART_ClearFlag(USART1, USART_FLAG_TC);
}

extern xQueueHandle xQueueRx;
void USART1_IRQHandler(void)      //串口1 中断服务程序
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	uint8_t Clear = 0;
	u8 rx_dat;
	/*如果接收到一字节数据*/
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)	    //判断读寄存器是否非空
	{	
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);	        //清除中断标志
		rx_dat = USART_ReceiveData(USART1);     //接收串口1数据到buff缓冲区
		xQueueSendToBackFromISR(xQueueRx, &rx_dat, &xHigherPriorityTaskWoken); /*有更高优先级的TASK被解除阻塞,ze xHigherPriorityTaskWoken为true*/
        portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
		
    /*如果接收到一帧数据*/
	}else if(USART_GetITStatus(USART1,USART_IT_IDLE) != RESET){		
		Clear = USART1->SR;
		Clear = USART1->DR;		//清除IDLE标志
	}
	if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)        //这段是为了避免STM32 USART 第一个字节发不出去的BUG 
	{ 
		USART_ITConfig(USART1, USART_IT_TXE, DISABLE);			//禁止发缓冲器空中断， 
	}	
}
