/**
  ******************************************************************************
  * @file    main.c
  * @author  Sara Pereira 1170515, Pedro Ferreira 1170739
  * @version V1.0
  * @date    02/12/2021
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f10x.h"
#include "lcd.h"
#include <stdio.h>
#include <string.h>




 //SPI vars
 int16_t x_high, x_low, y_high = 0, y_low = 0, z_high = 0, z_low = 0;
 int16_t SPIx = 0, SPIy = 0, SPIz = 0;


//Posição da bola
 typedef struct pos_bola{
 	uint8_t x;
 	uint8_t y;

 }pos_bola;

 pos_bola bola;

 //Posição dos obstáculos
 typedef struct pos_obs{
	 uint8_t x;
	 uint8_t y;
 }pos_obs;

 pos_obs bola1;
 pos_obs bola2;
 pos_obs bola3;
 pos_obs bola4;
 pos_obs bola5;
 pos_obs bola6;
 pos_obs bola7;
 pos_obs bola8;
 pos_obs bola9;
//Inicialização do número de vidas
uint8_t vidas=3;

//Lcd
 unsigned char buffer_lcd[32];

//Flags
extern int flag_inicio, flag_gameover;


//Flags timer
extern int flag_time;
extern int flag_time1;



//Sentido do movimento dos obstáculos
int sentido_bola1;
int sentido_bola2;
int sentido_bola3;
int sentido_bola4;
int sentido_bola5;
int sentido_bola6;
int sentido_bola7;
int sentido_bola8;
int sentido_bola9;

//Contadores auxiliares- timer
extern int cont_1;


void RCC_Config_HSE_Default()
{
	RCC_SYSCLKConfig(RCC_SYSCLKSource_HSE);
	RCC_HSEConfig(RCC_HSE_ON);


	ErrorStatus HSEStartUpStatus;
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if(HSEStartUpStatus == SUCCESS)
	{

		FLASH_SetLatency(FLASH_Latency_0);
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	}
	else
	while(1);

	while(RCC_GetSYSCLKSource() != 0x04);

}


void RCC_Config_HSE_PLL_Max()
{


		FLASH_SetLatency(FLASH_Latency_2);
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);


		RCC_PLLConfig(RCC_PLLSource_HSE_Div2, RCC_PLLMul_12);
		RCC_PLLCmd(ENABLE);
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		RCC_PCLK1Config(RCC_HCLK_Div2);


		while(RCC_GetSYSCLKSource() != 0x08);
}

void Init_SPI()
{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_DeInit(GPIOB);

		/* SPI */

		// PB5 - RDY
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		// PB13 - SCL (SPI2_SCK)
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		// PB14 - SDO (SPI2_MISO)
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		// PB15 - SDA (SPI2_MOSI)
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		// PD2 - CS/SS (Chip Select/Slave Select)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOD, &GPIO_InitStructure);

		RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, ENABLE); // SPI2 (APB1)

		SPI_InitTypeDef SPI_InitStructure;
		SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // Full Duplex
		SPI_InitStructure.SPI_Mode = SPI_Mode_Master; // Board Master -> Sensor Slave
		SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; // Tamanho 8 bits
		SPI_InitStructure.SPI_CPOL = SPI_CPOL_High; // Ler em alto
		SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; // Por causa da leitura em alto
		SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; // Controlo interno do SS
		SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
		SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	    SPI_InitStructure.SPI_CRCPolynomial = 7;
		SPI_Init(SPI2, &SPI_InitStructure);

		SPI_Cmd(SPI2, ENABLE); // Ativar SPI

		GPIO_WriteBit(GPIOD,GPIO_Pin_2,Bit_SET); // P�r o GPIOD2 (Chip Select) a nivel l�gico 1 (alto)
}

void Init_USART()
{
		// USART2

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//transmission pin
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		USART_InitTypeDef USART_InitStructure;
		USART_InitStructure.USART_BaudRate = 9600;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Tx;
		USART_Init(USART2, &USART_InitStructure);
		USART_Cmd(USART2, ENABLE);
}

void Init_GPIOs()
{
		// LEDS
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		/* Botoes*/
		// PA1 - SW5
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(GPIOA, &GPIO_InitStructure);


}

uint8_t spi_send( uint8_t Data )
{
		int periph = 0;
		int i;
		for(i=0;i<2;i++)
		{
		while(SPI_I2S_GetFlagStatus( SPI2, SPI_I2S_FLAG_TXE ) == RESET ); // esperar pela flag de transmiss�o ficar ativa
		SPI_I2S_SendData( SPI2, Data ); // enviar informa�ao pelo perif�rico
		SPI_I2S_ClearFlag(SPI2, SPI_I2S_FLAG_TXE); // clear flag

		while( SPI_I2S_GetFlagStatus( SPI2, SPI_I2S_FLAG_RXNE ) == RESET );
		periph = SPI_I2S_ReceiveData( SPI2 ); // receber informa��o do periferico
		SPI_I2S_ClearFlag(SPI2, SPI_I2S_FLAG_RXNE); // clear flag
		}

		return periph; // Informa��o recebida do perif�rico
}

// Leitura dos dados do acelerometro
void read_SPI()
{
		GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET); // Ler o bit SS em baixo
		x_low=spi_send(0b10101000);	// ler 8 bits baixos x
		GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET); // Colocar bit alto

		GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET);
		x_high=spi_send(0b10101001);	// ler 8 bits altos x
		GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);

		GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET);
		y_low=spi_send(0b10101010);	// ler 8 bits baixos y
		GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);

		GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET);
		y_high=spi_send(0b10101011);	// ler 8 bits altos y
		GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);

		GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET);
		z_low=spi_send(0b10101100);	// ler 8 bits baixos z
		GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);

		GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_RESET);
		z_high=spi_send(0b10101101);	// ler 8 bits altos z
		GPIO_WriteBit(GPIOD, GPIO_Pin_2, Bit_SET);

		SPIx = (x_high << 8) | x_low;
		SPIy = (y_high << 8) | y_low;
		SPIz = (z_high << 8) | z_low;
}

// Configurçao do registo de controlo
void CONFIG_SREG()
{

		GPIO_WriteBit(GPIOD,GPIO_Pin_2,Bit_RESET); // colocar pino em baixo
		spi_send(0x20);	// Endere�o do registo CTRL_REG1
		spi_send(0b11000111); 	// Sensor ativo - 11; Dividir por 512 - 00 (40Hz); ST desligado - 0; Zen,Yen,Xen (eixos) ativos - 111;
		GPIO_WriteBit(GPIOD,GPIO_Pin_2,Bit_SET); // colocar pino alto
}

 //Enviar dados recebidas por SPI por USART
void USART()
{
	uint8_t i;
	char dados[30];
	sprintf(dados, "x=%d; y=%d; z=%d", SPIx, SPIy, SPIz);

	for(i=0;i<=strlen(dados);i++)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
		USART_SendData(USART2,(uint16_t) dados[i]);
		USART_ClearFlag(USART2,USART_FLAG_TXE);
	}
}

//Configuração do timer3
void Timer3()
{

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = 200;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_Prescaler = 3599;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM3, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;


	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE );


}

//Configuração interrupções externas
void Init_EXTI()
{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);

		EXTI_InitTypeDef EXTI_InitStructure;
		EXTI_InitStructure.EXTI_Line = EXTI_Line1;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);


		NVIC_InitTypeDef NVIC_InitStructure;
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
		NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;

		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);

}

void inicio_jogo()
{
	// Inicio Jogo

	bola.x=60;
	bola.y=10;

	//Sentidos
	 sentido_bola1=0;
	 sentido_bola2=1;
	 sentido_bola3=0;
	 sentido_bola4=1;
	 sentido_bola5=0;
	 sentido_bola6=1;
	 sentido_bola7=0;
	 sentido_bola8=0;
	 sentido_bola9=0;
	 //Desenhar bola
	lcd_draw_circle(bola.x,bola.y, 2,534);


	//Obstaculos posições iniciais
	bola1.x=5;
	bola2.x=120;
	bola3.x=5;
	bola4.x=120;
	bola5.x=5;
	bola6.x=120;
	bola7.x=5;
	bola8.x=5;
	bola9.x=60;
}


void inicializacao_ecra()
{
	//Inicialização do jogo
	lcd_draw_fillrect(0, 0, 128, 135, 0);
	lcd_draw_string(45, 27, (unsigned char *) "INICIO",	875, 2);
	lcd_draw_string(50, 47, (unsigned char *) "DO",	875, 2);
	lcd_draw_string(37, 67, (unsigned char *) "JOGO",875, 2);
	lcd_draw_string(90, 130, (unsigned char *) "SW5",300, 2);


	//Verificação do número de vidas
	if(vidas==3)
	{

		GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_SET);
		GPIO_WriteBit(GPIOB, GPIO_Pin_2, Bit_SET);
		lcd_draw_string(20, 87, (unsigned char *) "3 Vidas",475, 2);

	}

	if(vidas==2)
	{
		lcd_draw_string(20, 87, (unsigned char *) "2 Vidas",475, 2);
	}
	if(vidas==1)
	{
		lcd_draw_string(20, 87, (unsigned char *) "1 Vida",475, 2);
	}
}

void values_screen()
{
	//Leitura dos valores do acelerómetro
	read_SPI();


	//x
	sprintf((char *) buffer_lcd, "x:%d", SPIx);
	lcd_draw_string(0, 0,buffer_lcd, 475, 1);

	//y
	sprintf((char *)buffer_lcd, "y:%d", SPIy);
	lcd_draw_string(43, 0, buffer_lcd,475, 1);

	//z
	sprintf((char *)buffer_lcd, "z:%d", SPIz);
	lcd_draw_string(86, 0,buffer_lcd, 475, 1);



}

void game()
{
	values_screen();
	move_bola();

	//Detetar se a bola toca nos obstáculos

	//Conversão

	if( ((bola.x+2 >= bola1.x-3 && bola.x-2 <= bola1.x+3) && (bola.y+2 >= bola1.y-3 && bola.y-2 <= bola1.y+3))
	||((bola.x+2 >= bola2.x-3 && bola.x-2 <= bola2.x+3) && (bola.y+2 >= bola2.y-3 && bola.y-2 <= bola2.y+3))
	||((bola.x+2 >= bola3.x-3 && bola.x-2 <= bola3.x+3) && (bola.y+2 >= bola3.y-3 && bola.y-2 <= bola3.y+3))
	||((bola.x+2 >= bola4.x-3 && bola.x-2 <= bola4.x+3) && (bola.y+2 >= bola4.y-3 && bola.y-2 <= bola4.y+3))
	||((bola.x+2 >= bola5.x-3 && bola.x-2 <= bola5.x+3) && (bola.y+2 >= bola5.y-3 && bola.y-2 <= bola5.y+3))
	||((bola.x+2 >= bola6.x-3 && bola.x-2 <= bola6.x+3) && (bola.y+2 >= bola6.y-3 && bola.y-2 <= bola6.y+3))
	||((bola.x+2 >= bola7.x-3 && bola.x-2 <= bola7.x+3) && (bola.y+2 >= bola7.y-3 && bola.y-2 <= bola7.y+3))
	||((bola.x+2 >= bola8.x-3 && bola.x-2 <= bola8.x+3) && (bola.y+2 >= bola8.y-3 && bola.y-2 <= bola8.y+3))
	||((bola.x+2 >= bola9.x-3 && bola.x-2 <= bola9.x+3) && (bola.y+2 >= bola9.y-3 && bola.y-2 <= bola9.y+3))
	)
	{
		vidas--;

		if(vidas==0)
		{
			GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);
			lcd_draw_fillrect(0, 0, 128, 135, 0);
			lcd_draw_string(45, 27, (unsigned char *) "GAME",	255, 2);
			lcd_draw_string(50, 47, (unsigned char *) "OVER",	255, 2);
			flag_inicio=0;
			vidas=3;
			flag_gameover = 1;
		}
		else
			if(vidas==2)
			{
				GPIO_WriteBit(GPIOB, GPIO_Pin_2, Bit_RESET);
				lcd_draw_fillrect(0, 0, 128, 135, 0);
				flag_inicio=0;
				inicializacao_ecra();

			}
			else
				if(vidas==1)
				{
					GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_RESET);
					lcd_draw_fillrect(0, 0, 128, 135, 0);
					flag_inicio=0;
					inicializacao_ecra();


				}

	}

	//Se ganhar
	if( ( bola.x>0 || bola.x<120))
	{
		 if( bola.y==150)
		 {

		lcd_draw_fillrect(0, 0, 200, 200, 0);

		lcd_draw_string(30, 67, (unsigned char *) "VITORIA",	255, 2);
		vidas=3;
		flag_inicio=0;
		 }
	}

}

void move_bola()
{
	//Mexer bola conforme valores obtidos

	//Limite do lcd

		lcd_draw_circle(bola.x,bola.y, 2,0); //limpar bola anterior
		if(SPIx >= 150 && SPIx <=700)
		{
			bola.x +=5;
			if(bola.x>115){
				bola.x=115; // limite direito do LCD

			}
		}

		if(SPIy >= 150 && SPIy<=700)
		{
			bola.y +=5;
			if(bola.y>150){
				bola.y=150; // limite inferior do LCD

			}
		}

		if(SPIx <= -150 && SPIx >=-700)
		{
			bola.x-=5;
			if(bola.x<5){
				bola.x=5; // limite esquerdo do LCD

			}
		}

		if(SPIy <= -150 && SPIy >= -700)
		{
			bola.y-=5;
			if(bola.y<15){
				bola.y=15; // limite superior do LCD

			}
		}

		//Desenhar bola
		lcd_draw_circle(bola.x,bola.y, 2,534);


}
void obstaculos()
{
	//Definir posições
	bola1.y=30;
	bola2.y=60;
	bola3.y=90;
	bola4.y=105;
	bola5.y=115;
	bola6.y=130;
	bola7.y=145;
	bola8.y=45;
	bola9.y=75;
	if(flag_time==1)
	{
		flag_time=0;

		if(sentido_bola1==1)
		{
				if(bola1.x<120)
				{

					lcd_draw_circle(bola1.x,bola1.y, 3,0);
					bola1.x=bola1.x+10;
					lcd_draw_circle(bola1.x, bola1.y, 3, 786);
				}
				else
				{
					sentido_bola1=0;
				}

		}
		else
		{
			if(bola1.x>5)
			{
				lcd_draw_circle(bola1.x, bola1.y, 3,0);
				bola1.x=bola1.x-10;
				lcd_draw_circle(bola1.x, bola1.y, 3, 786);
			}
			else
			{
				sentido_bola1=1;
			}
		}

		if(sentido_bola7==1)
				{
						if(bola7.x<120)
						{

							lcd_draw_circle(bola7.x,bola7.y, 3,0);
							bola7.x=bola7.x+5;
							lcd_draw_circle(bola7.x, bola7.y, 3, 786);
						}
						else
						{
							sentido_bola7=0;
						}

				}
				else
				{
					if(bola7.x>5)
					{
						lcd_draw_circle(bola7.x, bola7.y, 3,0);
						bola7.x=bola7.x-5;
						lcd_draw_circle(bola7.x, bola7.y, 3, 786);
					}
					else
					{
						sentido_bola7=1;
					}
				}

		if(sentido_bola3==1)
		{
				if(bola3.x<120)
				{

					lcd_draw_circle(bola3.x,bola3.y, 3,0);
					bola3.x=bola3.x+10;
					lcd_draw_circle(bola3.x, bola3.y, 3, 786);



				}
				else
				{
					sentido_bola3=0;
				}

		}
		else
		{
			if(bola3.x>5)
			{
				lcd_draw_circle(bola3.x, bola3.y, 3,0);
				bola3.x=bola3.x-10;
				lcd_draw_circle(bola3.x, bola3.y, 3, 786);


			}
			else
			{
				sentido_bola3=1;
			}
		}
		if(sentido_bola5==0)
				{
						if(bola5.x<60)
						{

							lcd_draw_circle(bola5.x,bola5.y, 3,0);
							bola5.x=bola5.x+10;
							lcd_draw_circle(bola5.x, bola5.y, 3, 786);


							lcd_draw_circle(bola8.x,bola8.y, 3,0);
							bola8.x=bola8.x+10;
							lcd_draw_circle(bola8.x, bola8.y, 3, 786);

						}
						else
						{
							sentido_bola5=1;
						}

				}
				else
				{
					if(bola5.x>5)
					{
						lcd_draw_circle(bola5.x, bola5.y, 3,0);
						bola5.x=bola5.x-10;
						lcd_draw_circle(bola5.x, bola5.y, 3, 786);

						lcd_draw_circle(bola8.x, bola8.y, 3,0);
						bola8.x=bola8.x-10;
						lcd_draw_circle(bola8.x, bola8.y, 3, 786);



					}
					else
					{
						sentido_bola5=0;
					}
				}




			if(sentido_bola2==1)
				{
						if(bola2.x<120)
						{

							lcd_draw_circle(bola2.x,bola2.y, 3,0);
							bola2.x=bola2.x+10;
							lcd_draw_circle(bola2.x, bola2.y, 3, 786);

						}
						else
						{
							sentido_bola2=0;


						}
				}


				if(sentido_bola2==0)
				{
					if(bola2.x>5)
					{
						lcd_draw_circle(bola2.x, bola2.y, 3,0);
						bola2.x=bola2.x-10;
						lcd_draw_circle(bola2.x, bola2.y, 3, 786);

					}
					else
					{
						sentido_bola2=1;

					}
				}
				if(sentido_bola4==1)
				{
					if(bola4.x<120)
					{

						lcd_draw_circle(bola4.x,bola4.y, 3,0);
						bola4.x=bola4.x+5;
						lcd_draw_circle(bola4.x, bola4.y, 3, 786);

					}
					else
					{
						sentido_bola4=0;


					}
				}


				if(sentido_bola4==0)
				{
					if(bola4.x>5)
					{
						lcd_draw_circle(bola4.x, bola4.y, 3,0);
						bola4.x=bola4.x-5;
						lcd_draw_circle(bola4.x, bola4.y, 3, 786);

					}
					else
					{
						sentido_bola4=1;

					}
				}

				if(sentido_bola6==1)
				{
					if(bola6.x<120)
					{

						lcd_draw_circle(bola6.x,bola6.y, 3,0);
						bola6.x=bola6.x+5;
						lcd_draw_circle(bola6.x, bola6.y, 3, 786);

					}
					else
					{
						sentido_bola6=0;


					}
				}


				if(sentido_bola6==0)
				{
					if(bola6.x>60)
					{
						lcd_draw_circle(bola6.x, bola6.y, 3,0);
						bola6.x=bola6.x-5;
						lcd_draw_circle(bola6.x, bola6.y, 3, 786);

					}
					else
					{
						sentido_bola6=1;

					}

				}
				if(sentido_bola9==1)
				{
					if(bola9.x<60)
					{

						lcd_draw_circle(bola9.x,bola9.y, 3,0);
						bola9.x=bola9.x+5;
						lcd_draw_circle(bola9.x, bola9.y, 3, 786);
					}
					else
					{
						sentido_bola9=0;
					}

				}
				else
					{
					if(bola9.x>5)
					{
						lcd_draw_circle(bola9.x, bola9.y, 3,0);
						bola9.x=bola9.x-5;
						lcd_draw_circle(bola9.x, bola9.y, 3, 786);
					}
					else
					{
						sentido_bola9=1;
					}
					}
	}

}

int main(void)
{



	RCC_Config_HSE_Default();
	RCC_Config_HSE_PLL_Max();

	lcd_init();
	Init_SPI();
	Init_GPIOs();
	Init_USART();
	CONFIG_SREG();
	Init_EXTI();

	Timer3();

	inicializacao_ecra();


while(1)
{
	read_SPI();
	USART();

	//Se o jogo tiver sido iniciado e ter vidas
	if(flag_inicio==1 && vidas>0)
		{
			game();
			obstaculos();
		}

}
}
