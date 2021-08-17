/**
  ******************************************************************************
  * File   : ERTC/ERTC_Calendar/main.c 
  * Version: V1.2.3
  * Date   : 2020-08-15
  * Brief  : Main program body
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "at32f4xx.h"
#include "at32_board.h"
#include <stdio.h>

/** @addtogroup AT32F415_StdPeriph_Examples
  * @{
  */

/** @addtogroup ERTC_Calendar
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Uncomment the corresponding line to select the ERTC Clock source */\
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ERTC_TimeType  ERTC_TimeStructure;
ERTC_DateType  ERTC_DateStructure;
ERTC_InitType  ERTC_InitStructure;

/* Private function prototypes -----------------------------------------------*/
static void ERTC_Config(void);
static void ERTC_TimeShow(void);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  NVIC_InitType  NVIC_InitStructure;
  EXTI_InitType  EXTI_InitStructure;
  int temp=0;
  
  /* AT Board Initial */
  AT32_Board_Init();
  
  /* initialize UART1  */
  UART_Print_Init(115200);
  
  /* ERTC configuration  */
  ERTC_Config();

  while (1)
  {
    ERTC_GetTimeValue(ERTC_Format_BIN, &ERTC_TimeStructure);  
    ERTC_GetDateValue(ERTC_Format_BIN, &ERTC_DateStructure);
          
    if(temp != ERTC_TimeStructure.ERTC_Seconds)
    {
      temp = ERTC_TimeStructure.ERTC_Seconds;
      /* Display time Format : hh:mm:ss */
      printf("20%02d-%02d-%02d ", ERTC_DateStructure.ERTC_Year, ERTC_DateStructure.ERTC_Month, ERTC_DateStructure.ERTC_Date);       
      printf("%0.2d:%0.2d:%0.2d ", ERTC_TimeStructure.ERTC_Hours, ERTC_TimeStructure.ERTC_Minutes, ERTC_TimeStructure.ERTC_Seconds);
      printf("week %d\r\n", ERTC_DateStructure.ERTC_WeekDay);      
    }
  }
}

/**
  * @brief  Configure the ERTC wakeup timer.
  * @param  wksel: ERTC_WakeUpClockSelect_RTCCLK_Div16:   ERTC Wakeup Counter Clock = RTCCLK/16
 *                 ERTC_WakeUpClockSelect_RTCCLK_Div8:    ERTC Wakeup Counter Clock = RTCCLK/8
 *                 ERTC_WakeUpClockSelect_RTCCLK_Div4:    ERTC Wakeup Counter Clock = RTCCLK/4
 *                 ERTC_WakeUpClockSelect_RTCCLK_Div2:    ERTC Wakeup Counter Clock = RTCCLK/2
 *                 ERTC_WakeUpClockSelect_CK_SPRE_16bits: ERTC Wakeup Counter Clock = CK_SPRE
 *                 ERTC_WakeUpClockSelect_CK_SPRE_17bits: ERTC Wakeup Counter Clock = CK_SPRE
 *          cnt: specifies the WakeUp counter. This parameter can be a value from 0x0000 to 0xFFFF.   
  * @retval None
  */
void ERTC_Set_WakeUp(uint32_t wksel, uint32_t cnt)
{ 
  NVIC_InitType NVIC_InitStruct;
  EXTI_InitType EXTI_InitStruct;
  
  ERTC_ClearFlag(ERTC_FLAG_WATWF);
  
  /* Set the WakeUp clock source */
  ERTC_WakeUpClockConfig(wksel);
  
  /* Enable the Wakeup counter */  
  ERTC_SetWakeUpCounter(cnt);

  /* ERTC Wakeup Interrupt Configuration */
  NVIC_InitStruct.NVIC_IRQChannel                   = ERTC_WKUP_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelCmd                = ENABLE;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority        = 0;
  NVIC_Init(&NVIC_InitStruct);

  /* EXTI configuration */  
  EXTI_InitStruct.EXTI_Line       = EXTI_Line22;
  EXTI_InitStruct.EXTI_LineEnable = ENABLE;
  EXTI_InitStruct.EXTI_Mode       = EXTI_Mode_Interrupt;
  EXTI_InitStruct.EXTI_Trigger    = EXTI_Trigger_Rising;
  EXTI_Init(&EXTI_InitStruct);
  
  /* Enable the WakeUp interrupt */
  ERTC_INTConfig(ERTC_INT_WAT, ENABLE);  
  
  /* Enable the WakeUp timer */
  ERTC_WakeUpCmd(ENABLE);
}

/**
  * @brief  Configure the ERTC peripheral by selecting the clock source.
  * @param  None
  * @retval None
  */
static void ERTC_Config(void)
{
  ERTC_DateType ERTC_DateStructure;
  
  /* Enable the PWR clock */
  RCC_APB1PeriphClockCmd(RCC_APB1PERIPH_PWR, ENABLE);

  /* Allow access to ERTC */
  PWR_BackupAccessCtrl(ENABLE);
   
  /* Reset ERTC Domain */
  RCC_BackupResetCmd(ENABLE);
  RCC_BackupResetCmd(DISABLE);
 
  /* Enable the LSE OSC */
  RCC_LSEConfig(RCC_LSE_ENABLE);

  /* Wait till LSE is ready */  
  while(RCC_GetFlagStatus(RCC_FLAG_LSESTBL) == RESET);

  /* Select the ERTC Clock Source */
  RCC_ERTCCLKConfig(RCC_ERTCCLKSelection_LSE);
  /* ck_spre(1Hz) = ERTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/

  /* Enable the ERTC Clock */
  RCC_ERTCCLKCmd(ENABLE);  
  
  /* Wait for ERTC APB registers synchronisation */
  ERTC_WaitForSynchro();
  
  /* Configure the ERTC data register and ERTC prescaler */
  ERTC_InitStructure.ERTC_AsynchPrediv = 127;
  ERTC_InitStructure.ERTC_SynchPrediv = 255;
  ERTC_InitStructure.ERTC_HourFormat = ERTC_HourFormat_24;
  ERTC_Init(&ERTC_InitStructure);
  
  /* Set the date: Friday January 11th 2013 */
  ERTC_DateStructure.ERTC_Year    = 13;
  ERTC_DateStructure.ERTC_Month   = 1;
  ERTC_DateStructure.ERTC_Date    = 11;
  ERTC_DateStructure.ERTC_WeekDay = 6;
  ERTC_SetDateValue(ERTC_Format_BIN, &ERTC_DateStructure);
  
  /* Set the time to 05h 20mn 00s AM */
  ERTC_TimeStructure.ERTC_AMPM    = ERTC_H12_AM;
  ERTC_TimeStructure.ERTC_Hours   = 5;
  ERTC_TimeStructure.ERTC_Minutes = 20;
  ERTC_TimeStructure.ERTC_Seconds = 0; 
  ERTC_SetTimeValue(ERTC_Format_BIN, &ERTC_TimeStructure);   
  
  ERTC_TimeShow();
  
  /* Set the wake-up time to 3s*/  
  ERTC_Set_WakeUp(ERTC_WakeUpClockSelect_CK_SPRE_16bits, 3 - 1); 
    
  printf("Enter in Sleep mode ...\r\n");
  PWR_EnterSleepMode(PWR_SLEEPEntry_WFI);
  printf("Wakeup from sleep mode ...\r\n");  
}

/**
  * @brief  Display the current time.
  * @param  None
  * @retval None
  */
static void ERTC_TimeShow(void)
{
  /* Get the current Time/Date */
  ERTC_GetTimeValue(ERTC_Format_BIN, &ERTC_TimeStructure);  
  ERTC_GetDateValue(ERTC_Format_BIN, &ERTC_DateStructure);
          
  /* Display time Format : hh:mm:ss */
  printf("20%02d-%02d-%02d ", ERTC_DateStructure.ERTC_Year, ERTC_DateStructure.ERTC_Month, ERTC_DateStructure.ERTC_Date);       
  printf("%0.2d:%0.2d:%0.2d ", ERTC_TimeStructure.ERTC_Hours, ERTC_TimeStructure.ERTC_Minutes, ERTC_TimeStructure.ERTC_Seconds);
  printf("week %d\r\n", ERTC_DateStructure.ERTC_WeekDay);      
}


/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2018 ArteryTek *****END OF FILE****/ 
