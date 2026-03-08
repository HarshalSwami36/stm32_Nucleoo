/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : PCA9685 Servo Control with STM32F446
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"

/* USER CODE BEGIN Includes */

#define PCA9685_ADDRESS 0x40
#define PCA9685_MODE1 0x00
#define PCA9685_PRE_SCALE 0xFE
#define PCA9685_LED0_ON_L 0x06

#define PCA9685_MODE1_SLEEP_BIT 4
#define PCA9685_MODE1_AI_BIT 5
#define PCA9685_MODE1_RESTART_BIT 7

/* USER CODE END Includes */

I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim1;

/* Function prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);

/* USER CODE BEGIN 0 */

void PCA9685_SetBit(uint8_t reg, uint8_t bit, uint8_t val)
{
    uint8_t data;

    HAL_I2C_Mem_Read(&hi2c1, PCA9685_ADDRESS<<1, reg, 1, &data, 1, 100);

    if(val)
        data |= (1<<bit);
    else
        data &= ~(1<<bit);

    HAL_I2C_Mem_Write(&hi2c1, PCA9685_ADDRESS<<1, reg, 1, &data, 1, 100);
}

void PCA9685_SetPWMFrequency(uint16_t freq)
{
    uint8_t prescale;

    prescale = 25000000/(4096*freq) - 1;

    PCA9685_SetBit(PCA9685_MODE1, PCA9685_MODE1_SLEEP_BIT, 1);

    HAL_I2C_Mem_Write(&hi2c1, PCA9685_ADDRESS<<1, PCA9685_PRE_SCALE, 1, &prescale, 1, 100);

    PCA9685_SetBit(PCA9685_MODE1, PCA9685_MODE1_SLEEP_BIT, 0);

    PCA9685_SetBit(PCA9685_MODE1, PCA9685_MODE1_RESTART_BIT, 1);
}

void PCA9685_Init(uint16_t freq)
{
    PCA9685_SetPWMFrequency(freq);
    PCA9685_SetBit(PCA9685_MODE1, PCA9685_MODE1_AI_BIT, 1);
}

void PCA9685_SetPWM(uint8_t channel, uint16_t on, uint16_t off)
{
    uint8_t reg = PCA9685_LED0_ON_L + 4*channel;

    uint8_t data[4];

    data[0] = on & 0xFF;
    data[1] = on >> 8;
    data[2] = off & 0xFF;
    data[3] = off >> 8;

    HAL_I2C_Mem_Write(&hi2c1, PCA9685_ADDRESS<<1, reg, 1, data, 4, 100);
}

void PCA9685_SetServoAngle(uint8_t channel, float angle)
{
    float pwm;

    pwm = 102 + (angle*(512-102)/180);

    PCA9685_SetPWM(channel,0,(uint16_t)pwm);
}

/* USER CODE END 0 */

int main(void)
{

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();

  /* USER CODE BEGIN 2 */

  PCA9685_Init(50);

  /* USER CODE END 2 */

  while (1)
  {

    PCA9685_SetServoAngle(0,0);
    HAL_Delay(1000);

    PCA9685_SetServoAngle(0,90);
    HAL_Delay(1000);

    PCA9685_SetServoAngle(0,180);
    HAL_Delay(1000);

  }
}

/* ---------------- CLOCK CONFIG ---------------- */

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct={0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct={0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  RCC_OscInitStruct.OscillatorType=RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState=RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState=RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource=RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM=4;
  RCC_OscInitStruct.PLL.PLLN=50;
  RCC_OscInitStruct.PLL.PLLP=RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ=7;
  RCC_OscInitStruct.PLL.PLLR=2;

  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType=RCC_CLOCKTYPE_HCLK|
                              RCC_CLOCKTYPE_SYSCLK|
                              RCC_CLOCKTYPE_PCLK1|
                              RCC_CLOCKTYPE_PCLK2;

  RCC_ClkInitStruct.SYSCLKSource=RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider=RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider=RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider=RCC_HCLK_DIV2;

  HAL_RCC_ClockConfig(&RCC_ClkInitStruct,FLASH_LATENCY_1);
}

/* ---------------- I2C INIT ---------------- */

static void MX_I2C1_Init(void)
{

  hi2c1.Instance=I2C1;
  hi2c1.Init.ClockSpeed=100000;
  hi2c1.Init.DutyCycle=I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1=0;
  hi2c1.Init.AddressingMode=I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode=I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2=0;
  hi2c1.Init.GeneralCallMode=I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode=I2C_NOSTRETCH_DISABLE;

  HAL_I2C_Init(&hi2c1);
}

/* ---------------- TIM1 INIT ---------------- */

static void MX_TIM1_Init(void)
{

  htim1.Instance=TIM1;
  htim1.Init.Prescaler=0;
  htim1.Init.CounterMode=TIM_COUNTERMODE_UP;
  htim1.Init.Period=65535;
  htim1.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;

  HAL_TIM_Base_Init(&htim1);
}

/* ---------------- GPIO INIT ---------------- */

static void MX_GPIO_Init(void)
{

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

}

/* ---------------- ERROR HANDLER ---------------- */

void Error_Handler(void)
{
  while(1){}
}
