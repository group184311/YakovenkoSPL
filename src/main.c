/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f1xx.h"
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

//1."очищаем" буфер
//буфер на прием; fifo1,
#define RX_BUFFER_SIZE 350 //размер буфера
volatile uint8_t fifo1[RX_BUFFER_SIZE];
volatile uint16_t tail1=0, head1=0;//хвост буфера (куда записываем данные) и начало буфера (откуда читаем данные)
volatile uint16_t count1= 0; //количество данных в буфере
volatile uint8_t owerflow1=0; //инфа о переполнении буфера

//буфер на передачу; fifo2
#define TX_BUFFER_SIZE 350 //размер буфера
volatile uint8_t fifo2[TX_BUFFER_SIZE];
volatile uint16_t tail2=0, head2=0;//хвост буфера (куда записываем данные) и начало буфера (откуда читаем данные)
volatile uint16_t count2= 0; //количество данных в буфере


void SystemClock_Config(void);
void GPIO_Init(void);
void GreenLedGPIO_Init(void); // светодиод
//void OutputsPin_Init(void); //вывод ножек для UART
void USART1_UART_Init(void);
void USART2_UART_Init(void);
void Error_Handler(void);


int main(void)
{
	uint8_t str[]="Send info\r\n";

	HAL_Init();
	SystemClock_Config();
	GPIO_Init();
	GreenLedGPIO_Init();
	USART1_UART_Init();
	USART2_UART_Init();
	Error_Handler();

	HAL_UART_Transmit(&huart1, str, 16, 0xFFFF);

	while(1)
	{

		HAL_UART_Receive_IT(&huart1, str, 11, 0xFFFF);
		HAL_UART_Receive_IT(&huart2, str, 11, 0xFFFF);

		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		HAL_Delay(250);
	}
}

void SystemClock_Config()
{
	RCC_OscInitTypeDef osc_RCC;
	RCC_ClkInitTypeDef clk_RCC;

	osc_RCC.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	osc_RCC.HSEState = RCC_HSE_ON;
	osc_RCC.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	osc_RCC.HSIState = RCC_HSI_OFF;
	osc_RCC.LSEState = RCC_LSE_OFF;
	osc_RCC.LSIState = RCC_LSI_OFF;
	osc_RCC.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	osc_RCC.PLL.PLLMUL = RCC_PLL_MUL9;
	osc_RCC.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	osc_RCC.PLL.PLLState = RCC_PLL_NONE;

	if(HAL_RCC_OscConfig(&osc_RCC) != HAL_OK)
	{
		Error_Handler();
	}

	clk_RCC.AHBCLKDivider = RCC_SYSCLK_DIV1;
	clk_RCC.APB1CLKDivider = RCC_HCLK_DIV1;
	clk_RCC.APB2CLKDivider = RCC_HCLK_DIV1;
	clk_RCC.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	clk_RCC.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

	if(HAL_RCC_ClockConfig(&clk_RCC, FLASH_LATENCY_2) !=HAL_OK)
	{
		Error_Handler();
	}



	//HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);//настраиваем время прерывания Systick
	//HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK); //настраиваем Systick
	//HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0); //конфигурация прерываний
}


void USART1_UART_Init()
{
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 9600;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_2;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if(HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
}

void USART2_UART_Init()
{
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 9600;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_EVEN;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if(HAL_UART_Init(&huart2) !=HAL_OK)
	{
		Error_Handler();
	}
}

void GPIO_Init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
}

void GreenLedGPIO_Init()
{
	__HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStructure;

	//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

	GPIO_InitStructure.Pin = GPIO_PIN_13;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
}

//void OutputsPin_Init()
//{
	//__HAL_RCC_USART1_CLK_ENABLE();

	//GPIO_InitTypeDef GPIO_InitStructure;

	//вывод пина pin9, GPIOA - RXD
	//GPIO_InitStructure.Pin = GPIO_PIN_9;
	//GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	//GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	//HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	//вывод пина pin10. GPIOA - TXD
	//GPIO_InitStructure.Pin = GPIO_PIN_10;
	//GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	//GPIO_InitStructure.Pull = GPIO_NOPULL;
	//HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
//}


void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStructUart;
  if(huart->Instance==USART1)
  {
	  //настраиваем USART1
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    //вывод PIN9, GPIOA - RX
    GPIO_InitStructUart.Pin = GPIO_PIN_9;
    GPIO_InitStructUart.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructUart.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructUart);

    //вывод PIN10, GPIOA -TX
    GPIO_InitStructUart.Pin = GPIO_PIN_10;
    GPIO_InitStructUart.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructUart.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructUart);

    HAL_NVIC_Enable(USART1_IRQn); //разрешаем прерывание у первого UART
    HAL_NVIC_SetPriority(USART1_IRQn, 15, 0); //задаем приоритет первому UART (группа приоритетов 4: приоритет - 15, субприоритет-0)
  }

  if(huart->Instance==USART2)
  	{
  		//настраиваем USART2
  	__HAL_RCC_USART2_CLK_ENABLE();
  	//PIN8, GPIOA
  	GPIO_InitStructUart.Pin = GPIO_PIN_8;
  	GPIO_InitStructUart.Mode = GPIO_MODE_AF_PP;
  	GPIO_InitStructUart.Speed = GPIO_SPEED_FREQ_HIGH;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructUart);

  	//PIN7, GPIOA
  	GPIO_InitStructUart.Pin = GPIO_PIN_7;
  	GPIO_InitStructUart.Mode = GPIO_MODE_INPUT;
  	GPIO_InitStructUart.Pull = GPIO_NOPULL;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStructUart);

  	HAL_NVIC_Enable(USART2_IRQn);
  	HAL_NVIC_SetPriority(USART2_IRQn, 15, 0);
  	 }
}


void HAL_MspInit(void)
{
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();


  __HAL_AFIO_REMAP_SWJ_NOJTAG();

}


//обработчик прерывания, ПЕРЕПИСАТЬ!!!!!!!!!
//void USART1_IRQHandler(void)
//{
	//if(USART_GetItStatus(USART1, USART_IT_RXNE) == SET) //перывание по окончанию приема данных
	//{
		//if((USART1->SR & USART_SR_RXNE) == 0) //проверяем нет ли ошибок
		//{
			//fifo1[rx_tail_fifo1++] =  (USART_ReceiveData(USART1) &0xFF); //считываем данные в буфер, инкрементируя хвост буфера
			//if(rx_tail_fifo1 == RX_BUFFER_SIZE) rx_tail_fifo1 = 0; // перебираем данные
			//if(rx_count_fifo1 == RX_BUFFER_SIZE) //переполнение буфера
			//{
				//rx_count_fifo1 = 0; //очищаем все и начинаем сначала
				//rx_overflow_fifo1=1; // сообщаем о переполнении
			//}
		//}
		//else USART_ReceiveData(USART1); // принимаем байт
	//}

	//if(USART_GetITStatus(USART2, USART_IT_TXE) == SET) //прерывание по окончанию передачи
	//{
		//if(tx_count_fifo2) //если есть что передавать
		//{
			//tx_count_fifo2--; //уменьшаем кол-во не переданных данных
			//USART_SendData(USART2, fifo2[tx_head_fifo2]); //передаем данные
			//if(tx_head_fifo2 == TX_BUFFER_SIZE) tx_head_fifo2=0; //перебираем данные
		//}
		//else //если ничего не передаем, запрещаем прерывание по передачи
		//{
			//USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
		//}
	//}
//}

void USART1_IRQHandler(void)
{
	HAL_UART_IRQHandler(&huart1);
}

void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&huart2);
}

//2. берем символ из буфера
uint8_t get_char(void)
{
	uint8_t receive_data_1; // переменная для данных
		while (count1 == 0); // если данных нет - ожидание
		receive_data_1 = fifo1[head1++]; // берем данные из буфера
				if (head1 == RX_BUFFER_SIZE) head1 = 0; // перебираем по кругу
					USART_ITConfig(USART2, USART_IT_RXNE, DISABLE); // запрещаем прерывание
					count--;
					USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // разрешаем прерывание
				return receive_data_1;
}

//3.кладем символ из буфера
void put_char(uint8_t receive_data_2)
{
	while (count2 == TX_BUFFER_SIZE);//если буфер переполнен - ожидание
		USART_ITConfig(USART1, USART_IT_TXE, DISABLE); //запрещаем прерывание
		if (count2 || (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)) //если в буфер уже что-то передается
		{
			fifo2[tail2++] = receive_data_2; //кладем данные в буфер
			if(tail2 == TX_BUFFER_SIZE) tail = 0; //перебираем по кругу
			USART_ITConfig(USART2, USART_IT_TXE, ENABLE);//разрешаем прерывание
		}
		else
			USART_SendData(USART1, receive_data_2); //передаем данные
}


void Error_Handler()
{
	  while(1)
	  {
		  HAL_GPIO_TOGGLE(GPIOC, GPIO_PIN_13);
		  HAL_Delay(100);
	  }
}
