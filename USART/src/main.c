#include "stm32f10x.h"
#include "lcd.h"
#include <stdio.h>
#include <string.h>

char duty_level = 10;

void RCC_Config_HSE_PLL_Max() {
	RCC_HSEConfig(RCC_HSE_ON);
	ErrorStatus HSEStartUpStatus;
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus == SUCCESS) {
		FLASH_SetLatency(FLASH_Latency_2);
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		RCC_PCLK1Config(RCC_HCLK_Div2); // PCLK1 Max 36MHz
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);
		RCC_PLLCmd(ENABLE);
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	} else while(1);
	while (RCC_GetSYSCLKSource() != 0x08);
}

void USART2_SendMessage(char *message) {
	uint16_t aux=0;
    while(aux != strlen(message)) {
        USART_SendData(USART2, (uint8_t) message[aux]);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
        aux++;
    }
}

void USART2_ChangeBaud(int baud) {
    USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
}

void USART2_ReadData() {
	uint8_t RxData;
	RxData = USART_ReceiveData(USART2);

	// R - Mudar o estado do LED GPIOB0
	if (RxData == 'R') {
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 0) {
			GPIO_WriteBit(GPIOB, GPIO_Pin_0, SET);
		} else GPIO_WriteBit(GPIOB, GPIO_Pin_0, RESET);
	}

	// M - LCD Menu com Baud Rates
	int bauds[] = {9600, 19200, 115200};
	if (RxData == 'M') {
	    char buffer_lcd[32];
	    int i;
	    for (i=1;i<=3;i++) {
		    sprintf(buffer_lcd, "%d - %d", i, bauds[i-1]);
		    lcd_draw_string(10, (i*10) , (unsigned char *) buffer_lcd, 0x07F0 , 1);
	    }
	}
	// C ou c - Clear do LCD
	if (RxData == 'C' || RxData == 'c') {
		char buffer_lcd[32];
		int i;
		for (i=1;i<=10;i++) {
		    sprintf(buffer_lcd, "                   ");
		    lcd_draw_string(10, (i*10), (unsigned char *) buffer_lcd, 0x07F0 , 1);
		}
	}
	// 1 2 3 - Seleção do Baud Rate
	if (RxData == 1) USART2_ChangeBaud(bauds[0]);
	if (RxData == 2) USART2_ChangeBaud(bauds[1]);
	if (RxData == 3) USART2_ChangeBaud(bauds[2]);

	// S - Nivel Lógico do Pino GPIOC11 (SW3)
	if (RxData == 'S') {
		char buffer[32];
		sprintf(buffer, "Estado do Pino: %d\n", GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_11));
		USART2_SendMessage(buffer);
	}

	// + - +10% Duty Cycle
	if (RxData == '+') {
		if (duty_level != 10) duty_level++;
		TIM_SetCompare2(TIM4, duty_level*200);
	}
	// - - -10% Duty Cycle
	if (RxData == '-') {
		if (duty_level != 0) duty_level--;
		TIM_SetCompare2(TIM4, duty_level*200);
	}
	// P - Get Duty Level
	if (RxData == 'P') {
		char buffer[32];
		sprintf(buffer, "Duty Cycle: %d%%\n", duty_level*10);
		USART2_SendMessage(buffer);
	}
}

int main(void) {
	// HSE PLL Max 72MHz
	RCC_DeInit();
	RCC_Config_HSE_PLL_Max();
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

	// LCD
	lcd_init();

	GPIO_InitTypeDef GPIO_InitStructure;
    // GPIOB Config - LED 0
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // GPIO Config - LED 1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // GPIOC Config - SW5 (11)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Configuração da USART
    USART_InitTypeDef USART_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);

    // Configuração GPIO USART
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    // RX
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configuração Interrupção USART
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    // USART2
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    TIM_TimeBaseStructure.TIM_Period = 2000;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_Prescaler = 35;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OC2Init(TIM4, &TIM_OCInitStructure);

    TIM_Cmd(TIM4, ENABLE);
    TIM_SetCompare2(TIM4, duty_level*200);

    while(1) {
    	//
    }
}
