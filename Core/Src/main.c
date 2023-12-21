/* USER CODE BEGIN Header */
/**
ddsa
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
//ceshi1
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "delay.h"
#include "tmc5130.h"
#include "hx711.h"
#include "bq25895.h"
#include "apply.h"
#include "led.h"
#include "bq27441.h"
#include "screen.h"
#include "tmp114.h"
#include "heat.h"
#include "pid.h"
#include "24cxx.h"
#include "ws2812b.h"
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

/* USER CODE BEGIN PV */
u8 PWM_SCALE[4],GCONF[4],XACTUAL[4];
u16 ADC_VALUE[3];
u8 tempbq[21];
u8 tempbq1[1],tempbq2[1];
u8 temptmp[2];
u16 tempadc[9];
s32 tempdata,tempdata1,temp2;
float Force;
u8 Flag_100ms_Toggle;

extern u32 RGB_DATA;

extern s32 ForceRawOffset;
extern s32 ForceRawActual;//应变片实际值
extern float EyeTmp;
extern u8 PWR_STATE;
extern u8 WorkMode;
extern u8 PowerState,LastPowerState;
/*

1:正在充电
2:充电已满
3:未充电量充足
*/

extern u8 Flag_3200ms,Flag_800ms,Flag_400ms,Flag_200ms,Flag_100ms;

extern u8 BQ25895Reg[21];

extern BQ27441_typedef BQ27441;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  MX_TIM6_Init();
  MX_TIM14_Init();
  MX_TIM16_Init();
  MX_TIM17_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
	delay_init(64);
//	LEDCtrl(LED3,LEDOn);
	AT24CXX_Init();
	BQ27441_Init();
	BQ25895_Init();
	HeatPIDInit();
	HeatPower(ON);
//	HeatPWMSet(50);
	PWM_WS2812B_Init();
	PWM_WS2812B_Show(5);
	ScreenUpdateSOC(BQ27441.SOC,PowerState);//电量上传
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		
		if(PWR_STATE==0)
		{
			if(Flag_800ms)
			{
				BQ25895_MultiRead(BQ25895Reg);//获取充电芯片状态
				PowerStateUpdate();//电源状态更新
//				LEDUpdate();//LED状态更新
				BQ27441_MultiRead(&BQ27441);//获取电量计数值
				BATCheckDIS();
				Flag_800ms=0;
			}
			LEDUpdate();
			if(PWR_SW==1)
			{
				PWM_WS2812B_Write_24Bits(5,0x0000ff);
				PWR_STATE=1;
				TMC5130_Init();
				MotorChecking();
				HX711_Init();
				HAL_ADC_Start_DMA(&hadc1,(uint32_t *)&ADC_VALUE,3);
			}
		}
		else if(PWR_STATE==1)
		{
			LEDUpdate();
			SW_CMDHandler();
			TaskProcessing();
			if((USART1_RX_STA&0x8000)==0x8000)
			{
				UART1_CMDHandler(USART1_RX_BUF);
				USART1_RX_STA=0;
			}
			if(Flag_3200ms)
			{
				ScreenUpdateSOC(BQ27441.SOC,PowerState);//电量上传
				Flag_3200ms=0;
			}
			if(Flag_800ms)
			{
				BQ25895_MultiRead(BQ25895Reg);//获取充电芯片状态
				PowerStateUpdate();//电源状态更新
//				LEDUpdate();//LED状态更新
				BQ27441_MultiRead(&BQ27441);//获取电量计数值
				BATCheckDIS();
				Flag_800ms=0;
			}
			if(Flag_100ms)
			{
				if((WorkMode&0x05)==0x05 && Flag_100ms_Toggle==0)
				{
					ScreenUpdateTemperature(EyeTmp);
				}
				if((WorkMode&0x06)==0x06 && Flag_100ms_Toggle==1)
				{
					ScreenUpdateForce((ForceRawActual - ForceRawOffset < 0) ? 0 : (ForceRawActual - ForceRawOffset));
				}
				Flag_100ms_Toggle=!Flag_100ms_Toggle;
				Flag_100ms=0;
			}
			if(PWR_SW==0)
			{
				PWR_STATE=0;
				HAL_ADC_Stop_DMA(&hadc1);
				PWM_WS2812B_Write_24Bits(5,0x000000);
				SystemPowerDown();
			}
		}
//		PWM_WS2812B(5,RGB_DATA);
//		PWM_WS2812B_Show(5);
//		WS2812B_Reset();
//		WS2812B_Reset();
//		PWM_WS2812B_Red(2);		//???????
//		PWM_WS2812B_Green(3);
//		PWM_WS2812B_Blue(1);
		
//		Force=HX711_GetForce();
//		tempdata=HX711_Read();
//		tempdata1=tempdata-ForceRawOffset;
//		temp2=MotorCompare(ForceRawOffset+84534,tempdata);
//		TMC5130_Read(0x21,XACTUAL);

		
//		BQ25895_MultiRead(tempbq);
//		delay_ms(500);
		
//		delay_ms(500);
//		BQ27441_WriteWord(0x00,0x0008);
//		delay_ms(1);
//		BQ27441_Read(0x00,tempbq1);
//		delay_ms(1);
//		BQ27441_Read(0x01,tempbq2);
		
//		TMP114_Read(0x00,temptmp);
//		delay_ms(500);
		
//		HAL_ADC_Start_DMA(&hadc1,(uint32_t *)&tempadc,9);
//		delay_ms(5000);
		
//		MotorCtrl(200000);
//		for(u16 i=0;i<500;i++)
//		{
//			delay_ms(10);
//			TMC5130_Read(0x71,PWM_SCALE);
//			TMC5130_Read(0x00,GCONF);
//			Force=HX711_GetForce();
//		}
//		MotorCtrl(0);
//		for(u16 i=0;i<500;i++)
//		{
//			delay_ms(10);
//			TMC5130_Read(0x71,PWM_SCALE);
//			TMC5130_Read(0x00,GCONF);
//			Force=HX711_GetForce();
//		}
		
    /* USER CODE END WHILE */

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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
