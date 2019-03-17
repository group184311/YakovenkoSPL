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
	// включаем тактирование портов
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; // тактирование порта С
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // тактирование порта B
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; // тактирование порта A

	//кнопка 1 GPIOC, pin15
	//сбрасываем все в ноль. Т.к. pin15, то CRH - он старший конфигурационный регистр
	GPIOC->CRH &= ~(GPIO_CRH_MODE15_1 | GPIO_CRH_MODE15_0 | GPIO_CRH_CNF15_1 | GPIO_CRH_CNF15_0);
	//выставляем MODE [1:0] = 00; и CNF [1:0] = 10; т.к. это pull-down
	GPIOC->CRH |= GPIO_CRH_CNF15_1;
	//подтяжка к земле
	GPIOC->ODR &= ~ GPIO_ODR_ODR15;

	//кнопка 2, GPIOB, pin12
	//сбрасываем все в ноль. Т.к. pin12, то CRH - он старший конфигурационный регистр
	GPIOB->CRH &= ~(GPIO_CRH_MODE12_1 | GPIO_CRH_MODE12_0 | GPIO_CRH_CNF12_1 | GPIO_CRH_CNF12_0);
	//выставляем MODE [1:0] = 00; и CNF [1:0] = 10; т.к. это pull-down
	GPIOB->CRH |= GPIO_CRH_CNF12_1;
	//подтяжка к питанию
	GPIOB->ODR |= GPIO_ODR_ODR12;

	//кнопка 3, GPIOB, pin 15
	//сбрасываем все в ноль. Т.к. pin15, то CRH - он старший конфигурационный регистр
	GPIOB->CRH  &= ~(GPIO_CRH_MODE15_1 | GPIO_CRH_MODE15_0 | GPIO_CRH_CNF15_1 | GPIO_CRH_CNF15_0);
	//выставляем MODE [1:0] = 00; и CNF [1:0] = 10; т.к. это pull-down
	GPIOB->CRH |= GPIO_CRH_CNF15_1;
	//подтяжка к земле
	GPIOB->ODR &= ~ GPIO_ODR_ODR15;

	//кнопка 4, GPIOA, pin7
	//сбраываем все в ноль. Т.к. pin7, то CRL - он младший конфигурационный регистр
	GPIOA->CRL &= ~(GPIO_CRL_MODE7_1 | GPIO_CRL_MODE7_0 | GPIO_CRL_CNF7_1 | GPIO_CRL_CNF7_0);
	//выставляем MODE [1:0] = 00; и CNF [1:0] = 10; т.к. это pull-down
	GPIOA->CRL |= GPIO_CRL_CNF7_1;
	//подтяжка к питанию
	GPIOA->ODR |= GPIO_ODR_ODR7;

	//выставляем в ноль биты светодиода, pin13
	//сбрасыываем все в ноль.  Т.к. pin13, то CRH - он старший конфигурационный регистр
	GPIOC->CRH &= ~(GPIO_CRH_MODE13_1 | GPIO_CRH_MODE13_0 | GPIO_CRH_CNF13_1 | GPIO_CRH_CNF13_0);
	//выставляем MODE [1:0] = 10; и CNF [1:0] = 00; и берем MODE [1:0]  с минимальной выходной скоростью 2MHz
	GPIOC->CRH |= GPIO_CRH_MODE13_1;

	//включаем тактирование таймера
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

	// Запускаем таймер на тактовой частоте в 1000 Hz
	TIM3->PSC = 36000 - 1;
	// Период - 500мс тактов => 500/1000 = 0,5 Hz (начинаем с ожидания, поэтому delay)
	TIM3->ARR = d - 1;
	// Разрешаем прерывания по переполнению таймера
	TIM3->DIER |= TIM_DIER_UIE;

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
