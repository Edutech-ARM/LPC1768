/**********************************************************************
* $Id$		lpc_global.c
*//**
* @file		lpc_global.c
* @brief	Contains Global Variables of LPC17xx
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
/** @addtogroup GLOBAL
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "LPC17xx.h"
#include "lpc_types.h"

/* If this source file built with example, the LPC17xx FW library configuration
 * file in each example directory ("lpc17xx_libcfg.h") must be included,
 * otherwise the default FW library configuration file must be included instead
 */

/** @addtogroup GLOBAL_Variables
 * @{
 */

/**
 *   Systick
 */
uint32_t led_delay;
uchar status = 0;

/**
 *  MCPWM
 */
// capture flag
FlagStatus CapFlag;
// Latest capture value
uint32_t CapVal;


/******************************** EMAC Block ******************************/
/* Tx, Rx Counters */
__IO uint32_t RXOverrunCount = 0;
__IO uint32_t RXErrorCount = 0;
__IO uint32_t TXUnderrunCount = 0;
__IO uint32_t TXErrorCount = 0;
__IO uint32_t RxFinishedCount = 0;
__IO uint32_t TxFinishedCount = 0;
__IO uint32_t TxDoneCount = 0;
__IO uint32_t RxDoneCount = 0;
__IO uint32_t ReceiveLength = 0;
__IO Bool PacketReceived = FALSE;

__IO FlagStatus Pressed = RESET;



/**
 * @}
 */



/* End of Public Functions ---------------------------------------------------- */

/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */

