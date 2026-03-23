/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
#define DATA_SIZE 8   // 4 joystick + 4 buttons

uint8_t slave_addr = 0x05;
uint8_t data[DATA_SIZE];

int l_x = 0, l_y = 0, r_x = 0, r_y = 0;
/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_I2C1_Init(void);

/* USER CODE BEGIN PFP */
long map(long x, long in_min, long in_max, long out_min, long out_max);
/* USER CODE END PFP */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_I2C1_Init();

  char buffer[128];

  sprintf(buffer, "\r\n--- System Started ---\r\nReading Slave 0x%02X\r\n", slave_addr);
  HAL_UART_Transmit(&huart3, (uint8_t*)buffer, strlen(buffer), 100);

  while (1)
  {
    HAL_StatusTypeDef status = HAL_I2C_Master_Receive(
        &hi2c1,
        (uint16_t)(slave_addr << 1),
        data,
        DATA_SIZE,
        100
    );

    if (status == HAL_OK)
    {
      /* Map joystick values */
      l_x = map(data[0], 0, 255, -127, 127);
      l_y = map(data[1], 0, 255, -127, 127);
      r_x = map(data[2], 0, 255, -127, 127);
      r_y = map(data[3], 0, 255, -127, 127);

      /* Buttons */
      uint8_t L1 = data[4];
      uint8_t L2 = (data[5] > 50) ? 1 : 0;
      uint8_t R1 = data[6];
      uint8_t R2 = (data[7] > 50) ? 1 : 0;

      sprintf(buffer,
              "LX:%4d | LY:%4d | RX:%4d | RY:%4d | L1:%d L2:%d R1:%d R2:%d\r\n",
              l_x, l_y, r_x, r_y, L1, L2, R1, R2);
    }
    else
    {
      sprintf(buffer, "I2C Error! Code: %d\r\n", status);
    }

    HAL_UART_Transmit(&huart3, (uint8_t*)buffer, strlen(buffer), 100);

    HAL_Delay(1000);  // 100ms delay
  }
}

/* ================= CLOCK CONFIG (FIXED ERROR) ================= */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    Error_Handler();

  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    Error_Handler();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLRCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    Error_Handler();
}

/* ================= I2C ================= */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0x05;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    Error_Handler();
}

/* ================= UART ================= */
static void MX_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart3) != HAL_OK)
    Error_Handler();
}

/* ================= GPIO ================= */
static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
}

/* ================= MAP FUNCTION ================= */
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) /
         (in_max - in_min) + out_min;
}

/* ================= ERROR HANDLER ================= */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
