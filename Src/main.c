/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "kb.h"
#include "sdk_uart.h"
#include "pca9538.h"
#include "oled.h"
#include "fonts.h"
#include "tim.h"
#include "buzzer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum { KEY_IDLE, KEY_HELD } key_state_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

char lastKey;
uint32_t lastScanTime;
key_state_t keyState;

char timer[6] = {'\0','\0','\0','\0','\0','\0'};
uint8_t current_digit = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void KB_Test( void );
void OLED_KB( uint8_t OLED_Keys[]);
void oled_Reset( void );

void scan_KB( void );
char read_key( void );

void process_pressed_key( void );

void start_timer( void );

void draw_timer( void );
void draw_started_timer( void );
void update_timer_chars( uint32_t sec );


void play_KB_beep( void );
void play_premature_timer_end( void );
void play_full_timer_end( void );
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
  MX_USART6_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  oled_Init();
  draw_timer();
  oled_UpdateScreen();
  Buzzer_Init();
  /* USER CODE END 2 */
 
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  scan_KB();
	  process_pressed_key();


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
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
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

void drawHappyFace(uint8_t x0, uint8_t y0) {
	// left half of the face, bit 15 = leftmost column
	static const uint16_t rows[32] = {
		0x003F, 0x01C0, 0x0600, 0x0800, 0x1000, 0x2000, 0x2000, 0x4000,
		0x4000, 0x8060, 0x80F0, 0x8198, 0x8000, 0x8000, 0x8000, 0x8000,
		0x83FF, 0x83FF, 0x81FF, 0x81FF, 0x80FF, 0x807F, 0x803F, 0x401F,
		0x400F, 0x2000, 0x2000, 0x1000, 0x0800, 0x0600, 0x01C0, 0x003F
	};

	for (uint8_t y = 0; y < 32; y++) {
		for (uint8_t x = 0; x < 16; x++) {
			OLED_COLOR c = (rows[y] & (0x8000 >> x)) ? White : Black;
			oled_DrawPixel(x0 + x,      y0 + y, c);
			oled_DrawPixel(x0 + 31 - x, y0 + y, c);
		}
	}
}

void drawSadFace(uint8_t x0, uint8_t y0) {
	// left half of the face (tears, outline, eyebrows, eyes, mouth)
	static const uint16_t rows[32] = {
		0x0000, 0x0600, 0x0600, 0x0000, 0x6000, 0x6300, 0x0300, 0x0000,
		0x600F, 0x6070, 0x0080, 0x0100, 0x6200, 0x62E0, 0x0470, 0x0438,
		0x081C, 0x0800, 0x0860, 0x0860, 0x0800, 0x0807, 0x081F, 0x083F,
		0x043F, 0x0478, 0x0270, 0x0200, 0x0100, 0x0080, 0x0070, 0x000F
	};

	for (uint8_t y = 0; y < 32; y++) {
		for (uint8_t x = 0; x < 16; x++) {
			OLED_COLOR c = (rows[y] & (0x8000 >> x)) ? White : Black;
			oled_DrawPixel(x0 + x,      y0 + y, c);
			oled_DrawPixel(x0 + 31 - x, y0 + y, c);
		}
	}
}

void play_KB_beep( void ) {
	Buzzer_Set_Freq(N_C5);
	Buzzer_Set_Volume(BUZZER_VOLUME_MAX);
	HAL_Delay(30);
	Buzzer_Set_Volume(BUZZER_VOLUME_MUTE);
}

void play_premature_timer_end( void ) {
	uint32_t lose_melody[] = {
		N_E5, N_DS5, N_D5, N_CS5,   // chromatic slide down
		0,                          // short rest
		N_C5, N_B4, N_AS4,          // sagging steps
		N_A4,                       // slow drop
		N_E3                        // low final note
	};
	uint32_t lose_delays[] = {
		8, 8, 8, 8,
		16,
		8, 8, 8,
		4,
		2
	};
	Buzzer_Play(lose_melody, lose_delays, sizeof(lose_melody) / sizeof(uint32_t));
}
void play_full_timer_end( void ) {
	uint32_t win_melody[] = {
			N_C5, N_E5, N_G5, N_C6, N_E6, N_G6,   // rising arpeggio
			N_E6, N_G6, 0,                        // lift and a short rest
			N_A5, N_B5, N_C6, N_D6, N_E6, N_C6,   // answer phrase
			N_G5, N_C6, N_E6, N_C7                // final climb, held
		};
		uint32_t win_delays[] = {
			16, 16, 16, 16, 16, 16,
			8, 4, 16,
			16, 16, 16, 16, 8, 8,
			16, 16, 16, 2
		};
	Buzzer_Play(win_melody, win_delays, sizeof(win_melody) / sizeof(uint32_t));
}

void process_pressed_key( void ) {
	char key = lastKey;
	lastKey = '\0';

	if (key != '\0') {
		play_KB_beep();
		if (key == 'D') {
			if (current_digit > 0) {
				timer[--current_digit] = '\0';
			}
			draw_timer();
			oled_UpdateScreen();
		} else if (key == 'E') {
			start_timer();
			for ( uint8_t i = 0; i < 6; i++ ) {
				timer[i] = '\0';
			}
			current_digit = 0;
			oled_Fill(Black);
			draw_timer();
			oled_UpdateScreen();
		} else if (current_digit < 6) {
			timer[current_digit++] = key;
			draw_timer();
			oled_UpdateScreen();
		}
	}
}

void draw_timer( void ) {
	// Draw invitation
	oled_SetCursor(22, 4);
	oled_WriteString("Enter timer:", Font_7x10, White);
	oled_DrawHLine(20, 108, 15, White);

	// Draw timer itself
	oled_SetCursor(20, 18);
	for ( int i = 0; i < 6; i++ ) {

		if (i % 2 == 0 && i != 0) {
			oled_WriteChar(':', Font_11x18, White);
		}

		if (timer[i] == '\0') {
			if (i == current_digit) {
				oled_WriteChar('_', Font_11x18, White);
			} else {
				oled_WriteChar(' ', Font_11x18, White);
			}
		} else {
			oled_WriteChar(timer[i], Font_11x18, White);
		}
	}
	oled_DrawHLine(20, 108, 39, White);
	// Draw explanation
	oled_SetCursor(20, 42);
	oled_WriteString("HH:MM:SS", Font_11x18, White);


}

void update_timer_chars( uint32_t sec ) {
	uint32_t h = sec / 3600;
	uint32_t m = (sec % 3600) / 60;
	uint32_t s = sec % 60;

	timer[0] = '0' + h / 10;
	timer[1] = '0' + h % 10;
	timer[2] = '0' + m / 10;
	timer[3] = '0' + m % 10;
	timer[4] = '0' + s / 10;
	timer[5] = '0' + s % 10;
}

void start_timer( void ) {
	for ( int i = 0; i < 6; i++ ) {
		if (timer[i] == '\0') timer[i] = '0';
	}

	uint32_t h = (timer[0]-'0')*10 + (timer[1]-'0');
	uint32_t m = (timer[2]-'0')*10 + (timer[3]-'0');
	uint32_t s = (timer[4]-'0')*10 + (timer[5]-'0');
	uint32_t time_to_wait = h*3600 + m*60 + s;


	update_timer_chars(time_to_wait);
	oled_Fill(Black);
	draw_started_timer();
	oled_UpdateScreen();

	uint32_t last = HAL_GetTick();

	while ( time_to_wait > 0 ) {
		scan_KB();
		if ( lastKey != '\0' ) {
			lastKey = '\0';
			oled_Fill(Black);
			drawSadFace(48, 16);
			oled_UpdateScreen();
			play_premature_timer_end();
			return;
		}

		if ( HAL_GetTick() - last >= 1000 ) {
			last += 1000;
			time_to_wait--;
			update_timer_chars( time_to_wait );
			draw_started_timer();
			oled_UpdateScreen();
		}
	}
	oled_Fill(Black);
	drawHappyFace(48, 16);
	oled_UpdateScreen();
	play_full_timer_end();
}

void draw_started_timer ( void ) {
	// Draw explanation
	oled_SetCursor(29, 4);
	oled_WriteString("Time left:", Font_7x10, White);
	oled_DrawHLine(20, 108, 15, White);

	// Draw timer itself
	oled_SetCursor(20, 18);
	for ( int i = 0; i < 6; i++ ) {

		if (i % 2 == 0 && i != 0) {
			oled_WriteChar(':', Font_11x18, White);
		}
		oled_WriteChar(timer[i], Font_11x18, White);
	}
	oled_DrawHLine(22, 108, 39, White);
	// Draw explanation
	oled_SetCursor(15, 42);
	oled_WriteString("Any key 2 stop", Font_7x10, White);

}

void scan_KB( void ) {
	  // debouncing, if we scanned in the last 150 ms we dont do anything.
	  if (HAL_GetTick() - lastScanTime > 150) {
	    lastScanTime = HAL_GetTick();
	    // scan keyboard
	    char currentKey = read_key();

	    if (keyState == KEY_IDLE) {
	      if (currentKey != '\0') {
	        lastKey = currentKey;
	        keyState = KEY_HELD;
	      }
	    } else {
	      if (currentKey == '\0') {
	        keyState = KEY_IDLE;
	        lastKey = '\0';
	      }
	    }
	  }
}

char read_key( void ) {
	  const char keymap[4][3] = {
		{'D', '0', 'E'},
	    {'1', '2', '3'},
	    {'4', '5', '6'},
	    {'7', '8', '9'},

	  };
	  uint8_t Key = 0, Row[4] = {ROW4, ROW3, ROW2, ROW1};


	  for ( int i = 0; i < 4; i++ ) {
		  Key = Check_Row(Row[i]);
		  switch( Key ) {
		  	  case 0x01:
		  		  return keymap[i][2];
		  	  case 0x02:
		  		  return keymap[i][1];
		  	  case 0x04:
		  		  return keymap[i][0];
		  }
	  }
	  return '\0';

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
