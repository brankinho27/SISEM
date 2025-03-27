#include "stm32f10x.h"
GPIO_InitTypeDef GPIO_InitStructure;

void RCC_Config_HSI_default() {
	RCC_HSICmd(ENABLE);
	FLASH_SetLatency(FLASH_Latency_0);
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
	while (RCC_GetSYSCLKSource() != 0x00);
}

void RCC_Config_HSI_PLL_Max() {
	RCC_HSICmd(ENABLE);
	FLASH_SetLatency(FLASH_Latency_2);
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16);
	RCC_PLLCmd(ENABLE);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while (RCC_GetSYSCLKSource() != 0x08);
}

void RCC_Config_HSE_default() {
	RCC_HSEConfig(RCC_HSE_ON);
	ErrorStatus HSEStartUpStatus;
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus == SUCCESS) {
		FLASH_SetLatency(FLASH_Latency_0);
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	} else while(1);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_HSE);
	while (RCC_GetSYSCLKSource() != 0x04);
}

void RCC_Config_HSE_PLL_Max() {
	RCC_HSEConfig(RCC_HSE_ON);
	ErrorStatus HSEStartUpStatus;
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus == SUCCESS) {
		FLASH_SetLatency(FLASH_Latency_2);
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	} else while(1);
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);
	RCC_PLLCmd(ENABLE);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while (RCC_GetSYSCLKSource() != 0x08);
}

void RCC_Config_30MHz() {
	RCC_HSEConfig(RCC_HSE_ON);
	ErrorStatus HSEStartUpStatus;
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus == SUCCESS) {
		FLASH_SetLatency(FLASH_Latency_1);
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	} else while(1);
	RCC_PLLConfig(RCC_PLLSource_HSE_Div2, RCC_PLLMul_5);
	RCC_PLLCmd(ENABLE);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while (RCC_GetSYSCLKSource() != 0x08);
}

void delayTime(uint32_t time) {
	int i;
	for(i=0;i<=time;i++);
}

void pisca_pisca() {
	GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);
	delayTime(1000000);
	GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);
	delayTime(1000000);
}

int main(void) {

    // GPIOB0 Config - LED
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    while(1) {
    	uint8_t i;

    	// HSE PLL 30MHz
    	RCC_DeInit();
    	RCC_Config_30MHz();
    	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
    	rcc_lcd_info();
    	for (i=1;i<=10;i++) pisca_pisca();

    	// HSI PLL Max
    	RCC_DeInit();
    	RCC_Config_HSI_PLL_Max();
   		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
   		rcc_lcd_info();
    	for (i=1;i<=10;i++) pisca_pisca();
    }
}
