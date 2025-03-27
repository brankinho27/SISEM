#include "stm32f10x.h"

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

int main(void) {
	// HSE PLL Max 72MHz
	RCC_DeInit();
	RCC_Config_HSE_PLL_Max();
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
	rcc_lcd_info();

	GPIO_InitTypeDef GPIO_InitStructure;
    // GPIOB Config - LED 0 1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

	// GPIOA Config - SW5 (A1)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Timer 3 - Up Count 1Hz
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    TIM_TimeBaseStructure.TIM_Period = 2000;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_Prescaler = 8995;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_Cmd(TIM3, ENABLE);

    // Configuração das Prioridades Group 1
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    // TIM3
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
    // EXTI1
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);

    // Ativar a interrpução do Update Event do TIM3
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    // Confiuração da interrupção EXTI1
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    while(1) {
    	// DO SOMETHING
    }
}

/* RESPOSTA ÀS PERGUNTAS:
 *
 * 3) Sendo a interrupção associada ao botão a de maior prioridade, o LED GPIOB0 pára de piscar
 * 		pois a interrupção do timer é interrompida pela do botão.
 * 4) Sendo a interrupção associada ao botão a de menor prioridade, o LED GPIOB1 acende/apaga mas
 * 		o LED GPIOB0 continua a piscar (pois tem maior prioridade e interrompe a interrupção do botão).
 * 5) Acontece o mesmo que acontece na pergunta 3, porque a subprioridade nunca interrompe uma interrupção
 * 		que esteja a acontecer (neste caso, a interrupção do timer nunca vai interromper a do botão que está
 * 		"presa" quando se deixa o botão pressionado).
 */
