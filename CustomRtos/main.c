/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rtos.h"     // our custom RTOS

/* Private define ------------------------------------------------------------*/

// INA219 I2C address (change 0x40 to 0x41 if you used that address)
#define INA_ADDR        (0x40 << 1)

// Relay pin on STM32 (update if your relay is on a different pin)
#define RELAY_GPIO_PORT GPIOA
#define RELAY_PIN       GPIO_PIN_0

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
float   lastBusV     = 0.0f;
uint8_t relay_state  = 0;
/* USER CODE END PV */

/* Function prototypes -------------------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);

/* USER CODE BEGIN PFP */
static uint16_t INA219_ReadReg(uint8_t reg);
static void INA219_WriteConfig(void);
static float INA219_ReadBusVoltage_V(void);

// RTOS task prototypes
void Task_Sense(void);
void Task_Control(void);
void Task_Heartbeat(void);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* Read 16-bit register from INA219 */
static uint16_t INA219_ReadReg(uint8_t reg)
{
    uint8_t buf[2];

    if (HAL_I2C_Master_Transmit(&hi2c1, INA_ADDR, &reg, 1, HAL_MAX_DELAY) != HAL_OK)
        return 0xFFFF;

    if (HAL_I2C_Master_Receive(&hi2c1, INA_ADDR, buf, 2, HAL_MAX_DELAY) != HAL_OK)
        return 0xFFFF;

    return ((uint16_t)buf[0] << 8) | buf[1];
}

/* Simple config: 32V range, 12-bit samples, continuous */
static void INA219_WriteConfig(void)
{
    uint8_t buf[3];

    // Config register = 0x00, value = 0x399F (common default)
    buf[0] = 0x00;      // register address
    buf[1] = 0x39;      // MSB
    buf[2] = 0x9F;      // LSB

    HAL_I2C_Master_Transmit(&hi2c1, INA_ADDR, buf, 3, HAL_MAX_DELAY);
    HAL_Delay(10);
}

/* Read bus voltage in volts */
static float INA219_ReadBusVoltage_V(void)
{
    uint16_t raw = INA219_ReadReg(0x02);  // bus voltage reg

    if (raw == 0xFFFF)
        return -1.0f;

    uint16_t bus_raw = (raw >> 3) & 0x1FFF;  // bits [12:3]
    float bus_mV = bus_raw * 4.0f;           // 4 mV per LSB

    return bus_mV / 1000.0f;
}

/* ===== RTOS TASKS ===== */

/* Task 1: Sense voltage from INA219 periodically */
void Task_Sense(void)
{
    float v = INA219_ReadBusVoltage_V();
    if (v >= 0.0f) {
        lastBusV = v;
    }
}

/* Task 2: Simple automatic control based on voltage */
void Task_Control(void)
{
    // Example threshold: if voltage > 1.0 V, turn relay ON
    const float THRESH_V = 1.0f;

    if (lastBusV > THRESH_V) {
        HAL_GPIO_WritePin(RELAY_GPIO_PORT, RELAY_PIN, GPIO_PIN_SET);
        relay_state = 1;
    } else {
        HAL_GPIO_WritePin(RELAY_GPIO_PORT, RELAY_PIN, GPIO_PIN_RESET);
        relay_state = 0;
    }
}

/* Task 3: Heartbeat (LED blink) to show RTOS is running */
void Task_Heartbeat(void)
{
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();

  // Relay OFF at start
  HAL_GPIO_WritePin(RELAY_GPIO_PORT, RELAY_PIN, GPIO_PIN_RESET);
  relay_state = 0;

  // Configure INA219
  INA219_WriteConfig();

  // ==== RTOS SETUP ====
  // 1 kHz tick rate
  RTOS_Init(1000);

  // Task_Sense: read voltage every 200 ms (5 Hz)
  RTOS_AddTask(Task_Sense, 200, 0);

  // Task_Control: run every 200 ms
  RTOS_AddTask(Task_Control, 200, 50);

  // Task_Heartbeat: blink LED every 500 ms (2 Hz)
  RTOS_AddTask(Task_Heartbeat, 500, 100);

  // Start the scheduler (never returns)
  RTOS_Start();

  while (1)
  {
    // Should never reach here
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|LD2_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
