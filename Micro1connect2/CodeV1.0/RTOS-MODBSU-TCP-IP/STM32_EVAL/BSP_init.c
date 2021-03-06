/**
  ******************************************************************************
  * @file    main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    06/19/2009
  * @brief   Main program body
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

#include  <cpu.h>
#include  <lib_def.h>
#include  <lib_mem.h>
#include  <lib_str.h>

#include  <ucos_ii.h>

#include "stm32f10x.h"
#include "stm32_eval.h"
#include "stm32_eth.h"

/* Private macro -------------------------------------------------------------*/
#define DP83848_PHY        /* Ethernet pins mapped on STM3210C-EVAL Board */
#define PHY_ADDRESS       0x01 /* Relative to STM3210C-EVAL Board */

#define MII_MODE          /* MII mode for STM3210C-EVAL Board (MB784) (check jumpers setting) */
//#define RMII_MODE       /* RMII mode for STM3210C-EVAL Board (MB784) (check jumpers setting) */

#define ETH_RXBUFNB       6 
#define ETH_TXBUFNB       3

u8  macadd[] = {0xad, 0x00, 0x00, 0x00, 0x00, 0x01};

/* Private variables ---------------------------------------------------------*/
ETH_InitTypeDef ETH_InitStructure;
/* Ethernet Rx & Tx DMA Descriptors */
ETH_DMADESCTypeDef  DMARxDscrTab[ETH_RXBUFNB], DMATxDscrTab[ETH_TXBUFNB];
/* Ethernet buffers */
u8 Rx_Buff[ETH_RXBUFNB][ETH_MAX_PACKET_SIZE], Tx_Buff[ETH_TXBUFNB][ETH_MAX_PACKET_SIZE]; 
ErrorStatus HSEStartUpStatus;
vu32 Value = 0;
 
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void RCC_Configuration(void);
void GPIO_Configuration(void);
void NVIC_Configuration(void);
void ADC_Configuration(void);
void ETH_Configuration(void);

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void BSP_init(void)
{
  /* Setup STM32 system (clock, PLL and Flash configuration) */
  SystemInit();

  /* Initialize LEDs **********************************************************/
  STM_EVAL_LEDInit(LED1);
  STM_EVAL_LEDInit(LED2);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);

  /* Turn on leds available on STM3210X-EVAL **********************************/
  STM_EVAL_LEDOff(LED1);
  STM_EVAL_LEDOff(LED2);
  STM_EVAL_LEDOff(LED3);
  STM_EVAL_LEDOff(LED4);

  /* System Clocks Configuration */
  RCC_Configuration();
       
  /* NVIC configuration */
  NVIC_Configuration();

  /* ADC configuration */
  ADC_Configuration();

  /* ETHERNET pins remapp in STM3210C-EVAL board: RX_DV and RxD[3:0] */
  GPIO_PinRemapConfig(GPIO_Remap_ETH, ENABLE); 
    
  /* MII/RMII Media interface selection */
#ifdef MII_MODE /* Mode MII with STM3210C-EVAL  */
  GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_MII);
  
  /* Get HSE clock = 25MHz on PA8 pin(MCO) */
  RCC_MCOConfig(RCC_MCO_HSE); 
  
#elif defined RMII_MODE  /* Mode RMII with STM3210C-EVAL */
  GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);
  
  /* Get HSE clock = 25MHz on PA8 pin(MCO) */
  /* set PLL3 clock output to 50MHz (25MHz /5 *10 =50MHz) */
  RCC_PLL3Config(RCC_PLL3Mul_10);
  /* Enable PLL3 */
  RCC_PLL3Cmd(ENABLE);
  /* Wait till PLL3 is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_PLL3RDY) == RESET)
  {}
 
  /* Get clock PLL3 clock on PA8 pin */
  RCC_MCOConfig(RCC_MCO_PLL3CLK);    
#endif
  
  /* Configure the GPIO ports */
  GPIO_Configuration();
  
  /* Configure the ETH */
  ETH_Configuration();
 
}

/**
  * @brief  Configures the clock
  * @param  None
  * @retval None
  */
void RCC_Configuration(void)
{
  RCC_ClocksTypeDef RCC_ClockFreq;

   /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus != ERROR)
  {
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

 /****************************************************************/
 /*      HSE=25MHz, HCLK=72MHz, PCLK2=72MHz, PCLK1=36MHz         */
 /****************************************************************/
    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    /* PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1);
    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div2);
    /*  ADCCLK = PCLK2/4 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    /* Configure PLLs *********************************************************/
    /* PPL2 configuration: PLL2CLK = (HSE / 5) * 8 = 40 MHz */
    RCC_PREDIV2Config(RCC_PREDIV2_Div5);
    RCC_PLL2Config(RCC_PLL2Mul_8);

    /* Enable PLL2 */
    RCC_PLL2Cmd(ENABLE);

    /* Wait till PLL2 is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET)
    {}

    /* PPL1 configuration: PLLCLK = (PLL2 / 5) * 9 = 72 MHz */
    RCC_PREDIV1Config(RCC_PREDIV1_Source_PLL2, RCC_PREDIV1_Div5);
    RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_9);

    /* Enable PLL */
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {}

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while (RCC_GetSYSCLKSource() != 0x08)
    {}
  }

  RCC_GetClocksFreq(&RCC_ClockFreq);

  /* Enable USART2 clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	


  /* Enable ETHERNET clock  */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx |
                        RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);

  /* Enable GPIOs clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |	RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
                         RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE| RCC_APB2Periph_AFIO, ENABLE);
  /* Enable ADC1 clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
}

/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval None
  */
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

/* ETHERNET pins configuration */
/* AF Output Push Pull: 
- ETH_MII_MDIO / ETH_RMII_MDIO: PA2 
- ETH_MII_MDC / ETH_RMII_MDC: PC1
- ETH_MII_TXD2: PC2 
- ETH_MII_TX_EN / ETH_RMII_TX_EN: PB11  
- ETH_MII_TXD0 / ETH_RMII_TXD0: PB12 
- ETH_MII_TXD1 / ETH_RMII_TXD1: PB13  
- ETH_MII_PPS_OUT / ETH_RMII_PPS_OUT: PB5
- ETH_MII_TXD3: PB8 */ 

  /* Configure PA2 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure PC1, PC2 and PC3 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Configure PB5, PB8, PB11, PB12 and PB13 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_11 | 
                                GPIO_Pin_12 | GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
/**************************************************************/
/*               For Remapped Ethernet pins                   */
/*************************************************************/
/* Input (Reset Value):  
- ETH_MII_CRS CRS: PA0 
- ETH_MII_RX_CLK / ETH_RMII_REF_CLK: PA1   
- ETH_MII_COL: PA3  
- ETH_MII_RX_DV / ETH_RMII_CRS_DV: PD8
- ETH_MII_TX_CLK: PC3
- ETH_MII_RXD0 / ETH_RMII_RXD0: PD9 
- ETH_MII_RXD1 / ETH_RMII_RXD1: PD10 
- ETH_MII_RXD2: PD11
- ETH_MII_RXD3: PD12
- ETH_MII_RX_ER: PB10 */
 
  /* Configure PA0, PA1 and PA3 as input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure PB10 as input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* Configure PC3 as input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure); 

  /* Configure PD8, PD9, PD10, PD11 and PD12 as input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOD, &GPIO_InitStructure); /**/    

    /* ADC Channel14 config --------------------------------------------------------*/
  /* Relative to STM3210D-EVAL Board   */
  /* Configure PC.04 (ADC Channel14) as analog input -------------------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* MCO pin configuration------------------------------------------------- */
  /* Configure MCO (PA8) as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
 
}

/**
  * @brief  Configures the ADC.
  * @param  None
  * @retval None
  */
void ADC_Configuration(void)
{
  ADC_InitTypeDef ADC_InitStructure;

  /* ADC1 Configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel14 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_13Cycles5);

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);  
}

/**
  * @brief  Configures the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef   NVIC_InitStructure;
  
  /* Set the Vector Table base location at 0x08000000 */
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
  
  /* 2 bit for pre-emption priority, 2 bits for subpriority */
  //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
  
  /* Enable the Ethernet global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  Configures the ETH.
  * @param  None
  * @retval None
  */
void ETH_Configuration(void)
{ 
  /* Reset ETHERNET on AHB Bus */
  ETH_DeInit();
  
  /* Software reset */ 
  ETH_SoftwareReset();
  
  /* Wait for software reset */
  while(ETH_GetSoftwareResetStatus()==SET);

  /* ETHERNET Configuration ------------------------------------------------------*/
  /* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
  ETH_StructInit(&ETH_InitStructure);
  
  /* Fill ETH_InitStructure parametrs */
  /*------------------------   MAC   -----------------------------------*/
  ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable  ;  
  //ETH_InitStructure.ETH_Speed = ETH_Speed_100M;                                      
  ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;              
  //ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;                                                                                  
  ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;                                                                                  
  ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;                                                                                                                                                                        
  ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Enable;                                                                                                       
  ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Disable;      
  ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;                                                             
  ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;      
  ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;                        
  /* Configure ETHERNET */
  Value = ETH_Init(&ETH_InitStructure, PHY_ADDRESS);
  
  ETH_MACAddressConfig(ETH_MAC_Address0, macadd);
  
  /* Initialize Tx Descriptors list: Chain Mode */
  ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
  /* Initialize Rx Descriptors list: Chain Mode  */
  ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);
  
  /* Rx and Tx interrupts are used. */
  //ETH_DMAITConfig( ETH_DMA_IT_R | ETH_DMA_IT_T | ETH_DMA_IT_NIS, ENABLE );
  ETH_DMAITConfig( ETH_DMA_IT_R | ETH_DMA_IT_NIS, ENABLE );

  /* Enable MAC and DMA transmission and reception */
  ETH_Start();
}


/*
*********************************************************************************************************
*                                       BSP_CPU_ClkFreq()
*
* Description : This function reads CPU registers to determine the CPU clock frequency of the chip in KHz.
*
* Argument(s) : none.
*
* Return(s)   : The CPU clock frequency, in Hz.
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT32U  BSP_CPU_ClkFreq (void)
{
    static  RCC_ClocksTypeDef  rcc_clocks;


    RCC_GetClocksFreq(&rcc_clocks);

    return ((CPU_INT32U)rcc_clocks.HCLK_Frequency);
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                                         OS CORTEX-M3 FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         OS_CPU_SysTickClkFreq()
*
* Description : Get system tick clock frequency.
*
* Argument(s) : none.
*
* Return(s)   : Clock frequency (of system tick).
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

INT32U  OS_CPU_SysTickClkFreq (void)
{
    INT32U  freq;


    freq = BSP_CPU_ClkFreq();
    return (freq);
}



/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
