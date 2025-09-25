/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "i2s.h"
#include "spi.h"
#include "usart.h"
#include "usb_host.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_MESSAGE_LENGTH 100
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile char received_message[MAX_MESSAGE_LENGTH];
volatile uint16_t message_index = 0;
volatile uint8_t message_received = 0;

const char* morse_codes[36] = {
    "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", // 0-9
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", // A-J
    "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", // K-T
    "..-", "...-", ".--", "-..-", "-.--", "--.."// U-Z
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
void send_morse_char(char c);
void send_morse_string(const char* str);
void morse_dot(void);
void morse_dash(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_I2C1_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_USB_HOST_Init();

  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart2, (uint8_t*)&received_message[0], 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (message_received) {
      __disable_irq();

      char local_message[MAX_MESSAGE_LENGTH];
      strcpy(local_message, (char*)received_message);

      message_index = 0;
      message_received = 0;

      __enable_irq();

      send_morse_string(local_message);
      morse_dot();
      HAL_Delay(100);
      morse_dash();
      HAL_Delay(100);
      morse_dot();
      HAL_Delay(100);
      morse_dash();
      HAL_Delay(100);
      morse_dot();
      HAL_Delay(100);
      morse_dash();
      HAL_Delay(100);

      HAL_UART_Receive_IT(&huart2, (uint8_t*)&received_message[0], 1);
    }

    /* USER CODE END WHILE */
    MX_USB_HOST_Process();
    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART2) {
    if (received_message[message_index] == '.') {
      received_message[message_index] = '\0';
      message_received = 1;
      return;
    }

    char c = received_message[message_index];
    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')) {
      message_index++;

      if (message_index >= MAX_MESSAGE_LENGTH - 1) {
        received_message[MAX_MESSAGE_LENGTH - 1] = '\0';
        message_index = 0;
        message_received = 1;
        return;
      }
    }

    HAL_UART_Receive_IT(&huart2, (uint8_t*)&received_message[message_index], 1);
  }
}

void send_morse_string(const char* str) {
  for (int i = 0; str[i] != '\0'; i++) {
    send_morse_char(str[i]);
    HAL_Delay(300);
  }
}

void send_morse_char(char c) {
  int index = -1;

  if (c >= '0' && c <= '9') {
    index = c - '0';
  } else if (c >= 'A' && c <= 'Z') {
    index = c - 'A' + 10;
  } else {
    return;
  }

  const char* morse_code = morse_codes[index];

  for (int i = 0; morse_code[i] != '\0'; i++) {
    if (morse_code[i] == '.') {
      morse_dot();
    } else if (morse_code[i] == '-') {
      morse_dash();
    }

    HAL_Delay(100);
  }
}

void morse_dot(void) {
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
  HAL_Delay(100); // Длительность точки
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
}

void morse_dash(void) {
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
  HAL_Delay(300); // Длительность тире (3 точки)
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
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
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */