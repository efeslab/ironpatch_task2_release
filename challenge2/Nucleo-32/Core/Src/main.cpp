/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.cpp
  * @brief          : Main program body
  ******************************************************************************
  * This is the STM32 AMP Challenge 02 main program file. It houses the logic
  * for receiving and decoding CAN frames, setting the appropriate states in
  * the Bumper class, handling timer interrupts, and setting pin states.
  *
  * Program flow:
  *   - Initialization of peripherals such as hardware timers and GPIO
  *   - Main program loop:
  *     - Check CAN FiFo via polling
  *       - If frame(s) exist, enter recv handling routines
  *     - Update Bumper class via Bumper brake and signal routines
  *     - Update physical LEDs based on Bumper state
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

// Used for determining current LED states
#include "Bumper.h"

// Common definition file
#include "local_defs.h"

// sprintf
#include <stdio.h>

// Used for verifying functionality w/o CAN network
// Main while loop must be adjusted to use respective recv, polling, and
// PGN checking
// #include "UnitTesting.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM14_Init(void);
static void MX_CAN_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
// Routine for handling incoming brake CAN frames, vulnerability exists here
void rx_brake_routine( unsigned char buff[], Bumper *bumper );
// Routine for handling incoming turn signal CAN frames
inline void rx_signal_routine( unsigned char buff[], Bumper *bumper );
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// global used for handling the 1 second hardware timer (htim3)
volatile uint32_t second_timer = 0;

// global used for handling the high precision performance metrics timer
volatile uint32_t perf_time = 0;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM14_Init();
  MX_CAN_Init();
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  // Start up PWM Channels for bumper LEDS
  HAL_TIM_PWM_Start( &htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start( &htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start( &htim14, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start( &htim2, TIM_CHANNEL_4);

  // Start hardware timers
  HAL_TIM_Base_Start_IT(&htim1);
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim3);
  HAL_TIM_Base_Start_IT(&htim14);

  // Initialize bumper
  Bumper bumper( &htim3, &second_timer ); // htim3; 1 second timer with TicksPerSecond as the counter period (manually updated localdefs.h)

  // For CAN Recv function
  CAN_RxHeaderTypeDef RxHeader;
  uint8_t RxData[8];
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t perf_local_time = 0;  // variable to keep track of relative time (performance)
  char text[100] = "";           // Used in serial printing of time elapsed
  // Under an oscilloscope, the rx_functions take a minimium of 2 - ~3.5 micro seconds
  // depending on configuration (minus 1 micro second for GPIO toggle). This was measured
  // by toggling a pin High and then low before and after function execution. A timer
  // was later used for ease of use in case an oscilloscope was not available.
  while (1)
  {
	if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) > 0){  // Check CAN FiFo mailbox
		// get the first frame in the queue
		HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &RxHeader, RxData);
		// Toggle RED LED on board
		HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_7);

		// Check PGN value in EXT ID portion of header
		if (((RxHeader.ExtId >> 8) & 0x7FFFF) == PGN_CruiseControlVehicleSpeed1 ){
			perf_local_time = perf_time + __HAL_TIM_GET_COUNTER(&htim1);

			rx_brake_routine(RxData, &bumper);

			perf_local_time = (perf_time + __HAL_TIM_GET_COUNTER(&htim1)) - perf_local_time;
			sprintf(text, "rx_brake_routine=%lu\r\n", (unsigned long)perf_local_time);
			HAL_UART_Transmit(&huart2, (uint8_t*)text, sizeof(text), 20);
		}

		if (((RxHeader.ExtId >> 8) & 0x7FFFF) == PGN_OperatorsExternalLightControls){
			perf_local_time = perf_time + __HAL_TIM_GET_COUNTER(&htim1);

			rx_signal_routine(RxData, &bumper);

			perf_local_time = (perf_time + __HAL_TIM_GET_COUNTER(&htim1)) - perf_local_time;
			sprintf(text, "rx_signal_routine=%lu\r\n", (unsigned long)perf_local_time);
			HAL_UART_Transmit(&huart2, (uint8_t*)text, sizeof(text), 20);
		}
	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  // run bumper update routines. These update LED states based on bumper state.
	  bumper.turn_signal_routine();
	  bumper.brake_routine();

	  // Change PWM compare registers in relation to bumper state
	  htim2.Instance->CCR1 = (bumper.outer_left) ? PWM_HIGH : PWM_LOW;
	  htim2.Instance->CCR2 = (bumper.inner_left) ? PWM_HIGH : PWM_LOW;
	  htim2.Instance->CCR4 = (bumper.inner_right) ? PWM_HIGH : PWM_LOW;
	  htim14.Instance->CCR1 = (bumper.outer_right) ? PWM_HIGH : PWM_LOW;
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN;
  hcan.Init.Prescaler = 12;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_2TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  // Create and start CAN filter, start up CAN
  CAN_FilterTypeDef sFilterConfig = {0};
  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK){
	  Error_Handler();
  }

  if (HAL_CAN_Start(&hcan) != HAL_OK) {
	  Error_Handler();
  }

  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

	/*
	 * This timer is used for the high precision performance metrics
	 * There is no prescaling done, and the period is the max allowed
	 */
  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */
	/*
	 * PWM Channel timer for the first 3 LEDs
	 */
  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 100;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */
	/*
	 * Whole second timer. Current config has step resolution of
	 * 1/1000 of a second (millisecond). The period is 999, which
	 * gives 1000 steps (0 indexed) before an interrupt is triggered.
	 * This timer is used so brake and turn signal flashing flash
	 * at and for the appropriate length of time.
	 */
  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 48000;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */
	/*
	 * Timer channel used for the 4th LED,
	 * configured the same as TIM2.
	 */
  /* USER CODE END TIM14_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 0;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 100;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim14, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */
  HAL_TIM_MspPostInit(&htim14);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */
	/*
	 * This UART channel is used to transmit serial data
	 * over the USB ST-Link line. It is being used to send
	 * performance metrics.
	 */
  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 38400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
/**
  * @brief  Handles a CAN data frame associated with PGN 65265, containing SPN 84, 597 and updates the corresponding Bumper
  * 		class. This function is structured slightly different than the turn signal function. More of it's update
  * 		logic is done here within the main file, to avoid calling a setter function directly. This limits the
  * 		vulnerability strictly to this function, so the fix will only be the type def here.
  *
  * @param  buff: The buffer containing the CAN data
  * @param  bumper: The bumper class to be updated
  * @retval None
  */
void rx_brake_routine( unsigned char buff[], Bumper *bumper ){
	int16_t speed_value;  // vulnerability here
	uint8_t brake_switch;
	// Extract Speed and brake switch from frame
	speed_value  = (buff[3] << 8) + buff[2];  // buf[3] = speed integer, buf[2] = speed decimal
	brake_switch = (buff[4] & 0b00001100) >> 2;
	// update related bumper members
	bumper->brake_state = (brake_switch) ? true : false;

	// This segment would ideally be moved to bumper method
	if (bumper->brake_state) {
		if ((speed_value > 0) && (bumper->prev_brake_state != bumper->brake_state)){  // speed > 0 and brakes were off last
			bumper->flash_lock = true;
			bumper->flash_timer = 0;
		}
	}
	else {
		bumper->flash_lock = false;
	}
	bumper->prev_brake_state = bumper->brake_state;
}


/**
  * @brief  Handles a CAN data frame associated with PGN 64972, containing SPN 2876 and updates the corresponding Bumper
  * 		class. Inlined due to size.
  *
  * @param  buff: The buffer containing the CAN data
  * @param  bumper: The bumper class to be updated
  * @retval None
  */
inline void rx_signal_routine( unsigned char buff[], Bumper *bumper ) {
	bumper->signal = (buff[1] >>4);
}


/**
  * @brief  Interrupt callback for the hardware timers. The timer that threw an interrupt
  * with be the passed parameter. It is checked if it is the second or precision timer
  * and updated accordingly.
  *
  * @param  *htim: The hardware timer responsible for calling this interrupt
  * @retval None
  *
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim == &htim3){
		second_timer += TicksPerSecond;  // Current timer (prescale 480, period of 100,000 gives ~12 hours before int overflow)
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
	}
	else if (htim == &htim1){  // Performance timer, used for high precision
		perf_time += 65535;  // will overflow after the 2^16 interrupt, ~6 minutes until overflow
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
