#include "stm32f10x.h"
GPIO_InitTypeDef GPIO_InitStructure;

void delayTime(uint32_t time) {
	int i;
	for(i=0;i<=time;i++);
}

int main(void) {
	// GPIOC13 Config - Botão Pressão
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // GPIO5 Config - LED
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    while(1) {
    	GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET);
    	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1) {
    		delayTime(500000);
    	} else delayTime(250000);

    	GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_RESET);
    	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 1) {
    		delayTime(500000);
    	} else delayTime(250000);
    }
}
