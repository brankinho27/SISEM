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

void SetRGBColor(uint8_t red, uint8_t green, uint8_t blue) {
	TIM_SetCompare1(TIM4, red*2000/255);
	TIM_SetCompare2(TIM4, green*2000/255);
	TIM_SetCompare3(TIM4, blue*2000/255);
}

int main(void) {
	// HSE PLL Max 72MHz
	RCC_DeInit();
	RCC_Config_HSE_PLL_Max();
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
	rcc_lcd_info();

	GPIO_InitTypeDef GPIO_InitStructure;
    // GPIOB Config - LED 0
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // GPIOB Config - LED 1, RGB
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // Timer 3 - Up Count 1Hz
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    TIM_TimeBaseStructure.TIM_Period = 2000;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_Prescaler = 17990;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    // Timer 3 - Output Compare Toggle CH4
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Toggle;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 2000;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC4Init(TIM3, &TIM_OCInitStructure);

    TIM_Cmd(TIM3, ENABLE);

    // Timer 4 - PWM 1kHz CH1 CH2 CH3
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    TIM_TimeBaseStructure.TIM_Period = 2000;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_Prescaler = 35;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
    TIM_OC2Init(TIM4, &TIM_OCInitStructure);
    TIM_OC3Init(TIM4, &TIM_OCInitStructure);

    TIM_Cmd(TIM4, ENABLE);

    // Set RGB Magenta
    SetRGBColor(255, 0, 255);

    // Brilho Gradual %
    uint8_t brilho[] = {0, 0.25*255, 0.5*255, 0.75*255, 1*255, 0.75*255, 0.5*255, 0.25*255};
    uint8_t estado;

    while(1) {
    	while (TIM_GetFlagStatus(TIM3,TIM_FLAG_Update) == RESET);
    	GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);
    	SetRGBColor(brilho[estado], 0, 0);
    	estado++;
    	if (estado==8) estado=0;
    	TIM_ClearFlag(TIM3, TIM_FLAG_Update);

    	while (TIM_GetFlagStatus(TIM3,TIM_FLAG_Update) == RESET);
    	GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);
    	SetRGBColor(brilho[estado], 0, 0);
    	estado++;
    	if (estado==8) estado=0;
    	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
    }
}
