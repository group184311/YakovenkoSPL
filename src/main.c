/*
 * main.c
 *
 *  Created on: 29 ноября 2018 г.
 *      Author: Асус
 */

#include "stm32f10x.h"


// Выполнено Яковенко Ю.А.

//объявление переменных для работы с таймером. d - задержка, w - работа. (u-беззнаковый)
volatile uint16_t d = 500; // задержка, равна 500мс
volatile uint16_t w = 500; // время работы равное 500мс
uint16_t now; // значение итерации в данный момент времени
uint16_t past; // значение предыдущей итерации

int main (void)
{
	//тактирование порта С
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	//тактирование порта В
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	//тактирование порта А
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	//кнопка 1 GPIOC, pin15
	//объявляем и настраиваем структуру инициализации
	GPIO_InitTypeDef gpio_init;
	gpio_init.GPIO_Pin = GPIO_Pin_15;
	gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
	gpio_init.GPIO_Mode = GPIO_Mode_IPD; // подтяжка к земле
	GPIO_Init(GPIOC, &gpio_init); // инициализация

	//кнопка 2, GPIOB, pin12
	//объявляем и настраиваем структуру инициализации
	GPIO_InitTypeDef gpio_init;
	gpio_init.GPIO_Pin = GPIO_Pin_12;
	gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
	gpio_init.GPIO_Mode = GPIO_Mode_IPU; //подтяжка к питанию
	GPIO_Init(GPIOB, &gpio_init); //инициализация

	//кнопка 3, GPIOB, pin 15
	//объявляем и настраиваем структуру инициализации
	GPIO_InitTypeDef gpio_init;
	gpio_init.GPIO_Pin = GPIO_Pin_15;
	gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
	gpio_init.GPIO_Mode = GPIO_Mode_IPD; // подтяжка к земле
	GPIO_Init(GPIOB, &gpio_init); //инициализация

	//кнопка 4, GPIOA, pin7
	//объявляем и настраиваем структуру инициализации
	GPIO_InitTypeDef gpio_init;
	gpio_init.GPIO_Pin = GPIO_Pin_7;
	gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
	gpio_init.GPIO_Mode = GPIO_Mode_IPU; // подтяжка к питанию
	GPIO_Init(GPIOA, &gpio_init);//инициализация

	//светодиод
	GPIO_InitTypeDef gpio_init;
	gpio_init.GPIO_Pin = GPIO_Pin_13;
	gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
	gpio_init.GPIO_Mode = GPIO_Mode_Out_PP; //выход двух состояний (push-pull)
	GPIO_Init(GPIOC, &gpio_init);

	//таймер
	//включаем тактирование таймера
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInitTypeDef tim; //начало работы с таймером
	tim.TIM_ClockDivision = TIM_CKD_DIV1; //таймер затактирован без деления частоты - делитель равен 1, TIM_CKD_DIV1
	tim.TIM_CounterMode = TIM_CounterMode_Up; //отсчет от нуля до TIM_Period - счет вверх
	tim.TIM_Period = d - 1;
	tim.TIM_Prescaler = 36000 - 1;
	TIM_TimeBaseInit(TIM3, &tim); // инициализация таймера
	TIM_ITConfig(TIM3, TIM_DIER_UIE, ENABLE); // разрешаем прерывания по таймеру


	// Включение прерывания таймера 3
	NVIC_EnableIRQ(TIM3_IRQn);

	//включем таймер
	TIM3->CR1 |= TIM_CR1_CEN;

	past = GPIOC->IDR & GPIO_IDR_IDR15; // значение предыдущей итерации первой кнопки (PIN15, GPIOC)

	for(;;) {

		now = GPIOC->IDR & GPIO_IDR_IDR15; // значение нынешнего состояния первой кнопки (PIN15, GPIOC)

		if (now != past ) // если значение нанышней итерации не равно значению предыдущей итерации, то
		{
			TIM3->CR1 &= ~ TIM_CR1_CEN; // выключаем таймер
			GPIOC->BSRR = GPIO_BSRR_BR13; //выкл

			if (now == GPIO_IDR_IDR15) //если значение нынешней итерации равно единице (т.е. она нажата)
			{
				TIM3->ARR = UINT16_MAX; // назначаем период счета таймера максимальным
			}
			else
			{
				if (TIM3->CNT >= 200) // если счетчик больше минимального значения (200 мс)
				{ d = TIM3->CNT; } // то тогда мы можем присвоить задержку  (то что мы насчитали)
			}
			TIM3->CNT = 0; // обнуление счетчика
			TIM3->CR1 |= TIM_CR1_CEN; // включение таймера
			past = now; // значение предыдущей итерации равно значению нынешней итерации
		}



		if (!(GPIOB->IDR & GPIO_IDR_IDR12)) //если кнопка нажата, то пишем значение переменной отвечащей за работу,
											//равной определенному значению работы (подтяжка к питанию)
		{
			w = 1000; //работа для кнопки с PIN12 GPIOB равная работе 1000 мс
		}
		else if(GPIOB->IDR & GPIO_IDR_IDR15) //(подтяжка к земле)
		{
			w = 1500; //работа для кнопки с PIN15 GPIOB равная работе 1500 мс
		}
		else if(!(GPIOA->IDR & GPIO_IDR_IDR7)) //(подтяжка к питанию)
		{
			w = 2000; //работа для кнопки с PIN7 GPIOА равная работе 2000 мс
		}
		else
		{
			w=500;
		}

	}
}

//Функция обработчика прерывания от таймера
void TIM3_IRQHandler(void)
{
	// Сбрасываем флаг переполнения таймера
	TIM3->SR &= ~TIM_SR_UIF; //Clean UIF Flag

	// Считываем логическое состояние вывода светодиода и инвертируем состояние
	if ( GPIOC->ODR & GPIO_ODR_ODR13 )
	{
		TIM3->ARR = w - 1; //время работы
		GPIOC->BSRR = GPIO_BSRR_BR13; //выкл
	}
	else
	{
		TIM3->ARR = d - 1; //время ожидания
		GPIOC->BSRR = GPIO_BSRR_BS13; //вкл
	}
}
