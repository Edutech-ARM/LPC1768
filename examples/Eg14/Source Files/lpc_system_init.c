/**********************************************************************
* $Id$		lpc_system_init.c
*//**
* @file		lpc_system_init.c
* @brief	Contains all functions support for SYSTEM INIT library
* 			on LPC17xx
* @version	1.0
* @date		24. July. 2013
* @author	Dwijay.Edutech Learning Solutions
***********************************************************************
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.
**********************************************************************/

/* Peripheral group ----------------------------------------------------------- */
/** @addtogroup SYSTEM_INIT
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "lpc_system_init.h"

/* If this source file built with example, the LPC17xx FW library configuration
 * file in each example directory ("lpc17xx_libcfg.h") must be included,
 * otherwise the default FW library configuration file must be included instead
 */

/** @addtogroup SYSTEM_INIT_Public_Functions
 * @{
 */

/* Public Functions ----------------------------------------------------------- */
/*********************************************************************//**
 * @brief	Initial System Init using Port and Peripheral
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void System_Init(void)
{
	LPC_WDT->WDMOD &= ~WDT_WDMOD_WDEN;  // Disable Watchdog
	SystemInit();						// Initialize system and update core clock
	Port_Init();                        // Port Initialization
	SYSTICK_Config();                   // Systick Initialization
	led_delay = 1000;                   // Heart Beat rate of 1Sec toggle
	NVIC_SetPriority(SysTick_IRQn, 0);  // Set SysTick as Highest Priority

	UART_Config(LPC_UART0, 115200);       // Uart0 Initialize at 9600 Baud Rate
	SSP_Config (LPC_SSP1);              // Initialize SPI
	I2C_Config (LPC_I2C0);              // Initialize I2C0
	GLCD_Init();                        // Initialize GLCD
	ADC_Config ();                      // Initialize ADC
}

/*********************************************************************//**
 * @brief 		General Port Initialization
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void Port_Init(void)
{
	GPIO_SetDir(3, _BIT(25), 1);        // Set HeartBeat Led P3.25 to Output
}

/**
 * @}
 */

/* End of Public Functions ---------------------------------------------------- */

/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */

