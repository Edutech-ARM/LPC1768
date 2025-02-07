/**********************************************************************
* $Id$		lpc17xx_uart.c
*//**
* @file		lpc17xx_uart.c
* @brief	Contains all functions support for UART firmware library on LPC17xx
* @version	1.0
* @date		25. Nov. 2013
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
/** @addtogroup UART
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "lpc17xx_uart.h"
#include "lpc17xx_clkpwr.h"

/* Private Functions ---------------------------------------------------------- */
static Status uart_set_divisors(LPC_UART_TypeDef *UARTx, uint32_t baudrate);
void UART_IntTransmit(LPC_UART_TypeDef *UARTx);
void UART_IntReceive(LPC_UART_TypeDef *UARTx);

#ifdef INTERRUPT_MODE
/*********************************************************************//**
 * @brief	UART0 interrupt handler sub-routine
 * @param	None
 * @return	None
 **********************************************************************/
void UART0_IRQHandler(void)
{
	// Call Standard UART 0 interrupt handler
	uint32_t intsrc, tmp, tmp1;

	// Determine the interrupt source
	intsrc = UART_GetIntId(LPC_UART0);
	tmp = intsrc & UART_IIR_INTID_MASK;

	// Receive Line Status
	if (tmp == UART_IIR_INTID_RLS)
	{
		// Check line status
		tmp1 = UART_GetLineStatus(LPC_UART0);
		// Mask out the Receive Ready and Transmit Holding empty status
		tmp1 &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_BI | UART_LSR_RXFE);
		// If any error exist
		if (tmp1)
		{
			while(tmp1)
			{
				; //implement error handling here
			}
		}
	}

	// Receive Data Available or Character time-out
	if ((tmp == UART_IIR_INTID_RDA) || (tmp == UART_IIR_INTID_CTI))
	{
		UART_IntReceive(LPC_UART0);
	}

	// Transmit Holding Empty
	if (tmp == UART_IIR_INTID_THRE)
	{
		UART_IntTransmit(LPC_UART0);
	}
}


/*********************************************************************//**
 * @brief	UART2 interrupt handler sub-routine
 * @param	None
 * @return	None
 **********************************************************************/
void UART2_IRQHandler(void)
{
	// Call Standard UART 2 interrupt handler
	uint32_t intsrc, tmp, tmp1;

	/* Determine the interrupt source */
	intsrc = UART_GetIntId(LPC_UART2);
	tmp = intsrc & UART_IIR_INTID_MASK;

	// Receive Line Status
	if (tmp == UART_IIR_INTID_RLS)
	{
		// Check line status
		tmp1 = UART_GetLineStatus(LPC_UART2);
		// Mask out the Receive Ready and Transmit Holding empty status
		tmp1 &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_BI | UART_LSR_RXFE);
		// If any error exist
		if (tmp1)
		{
			while(tmp1)
			{
				; //implement error handling here
			}
		}
	}

	// Receive Data Available or Character time-out
	if ((tmp == UART_IIR_INTID_RDA) || (tmp == UART_IIR_INTID_CTI))
	{
		UART_IntReceive(LPC_UART2);
	}

	// Transmit Holding Empty
	if (tmp == UART_IIR_INTID_THRE)
	{
		UART_IntTransmit(LPC_UART2);
	}
}


#endif

/*********************************************************************//**
 * @brief		Determines best dividers to get a target clock rate
 * @param[in]	UARTx	Pointer to selected UART peripheral, should be:
 * 				- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @param[in]	baudrate Desired UART baud rate.
 * @return 		Error status, could be:
 * 				- SUCCESS
 * 				- ERROR
 **********************************************************************/
static Status uart_set_divisors(LPC_UART_TypeDef *UARTx, uint32_t baudrate)
{
	Status errorStatus = ERROR;

	uint32_t uClk;
	uint32_t d, m, bestd, bestm, tmp;
	uint64_t best_divisor, divisor;
	uint32_t current_error, best_error;
	uint32_t recalcbaud;

	/* get UART block clock */
	if (UARTx == (LPC_UART_TypeDef *)LPC_UART0)
	{
		uClk = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_UART0);
	}
	else if (UARTx == (LPC_UART_TypeDef *)LPC_UART1)
	{
		uClk = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_UART1);
	}
	else if (UARTx == LPC_UART2)
	{
		uClk = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_UART2);
	}
	else if (UARTx == LPC_UART3)
	{
		uClk = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_UART3);
	}


	/* In the Uart IP block, baud rate is calculated using FDR and DLL-DLM registers
	* The formula is :
	* BaudRate= uClk * (mulFracDiv/(mulFracDiv+dividerAddFracDiv) / (16 * (DLL)
	* It involves floating point calculations. That's the reason the formulae are adjusted with
	* Multiply and divide method.*/
	/* The value of mulFracDiv and dividerAddFracDiv should comply to the following expressions:
	* 0 < mulFracDiv <= 15, 0 <= dividerAddFracDiv <= 15 */
	best_error = 0xFFFFFFFF; /* Worst case */
	bestd = 0;
	bestm = 0;
	best_divisor = 0;
	for (m = 1 ; m <= 15 ;m++)
	{
		for (d = 0 ; d < m ; d++)
		{
		  divisor = ((uint64_t)uClk<<28)*m/(baudrate*(m+d));
		  current_error = divisor & 0xFFFFFFFF;

		  tmp = divisor>>32;

		  /* Adjust error */
		  if(current_error > ((uint32_t)1<<31)){
			current_error = -current_error;
			tmp++;
			}

		  if(tmp<1 || tmp>65536) /* Out of range */
		  continue;

		  if( current_error < best_error){
			best_error = current_error;
			best_divisor = tmp;
			bestd = d;
			bestm = m;
			if(best_error == 0) break;
			}
		} /* end of inner for loop */

		if (best_error == 0)
		  break;
	} /* end of outer for loop  */

	if(best_divisor == 0) return ERROR; /* can not find best match */

	recalcbaud = (uClk>>4) * bestm/(best_divisor * (bestm + bestd));

	/* reuse best_error to evaluate baud error*/
	if(baudrate>recalcbaud) best_error = baudrate - recalcbaud;
	else best_error = recalcbaud -baudrate;

	best_error = best_error * 100 / baudrate;

	if (best_error < UART_ACCEPTED_BAUDRATE_ERROR)
		{
			if (((LPC_UART1_TypeDef *)UARTx) == LPC_UART1)
			{
				((LPC_UART1_TypeDef *)UARTx)->LCR |= UART_LCR_DLAB_EN;
				((LPC_UART1_TypeDef *)UARTx)->/*DLIER.*/DLM = UART_LOAD_DLM(best_divisor);
				((LPC_UART1_TypeDef *)UARTx)->/*RBTHDLR.*/DLL = UART_LOAD_DLL(best_divisor);
				/* Then reset DLAB bit */
				((LPC_UART1_TypeDef *)UARTx)->LCR &= (~UART_LCR_DLAB_EN) & UART_LCR_BITMASK;
				((LPC_UART1_TypeDef *)UARTx)->FDR = (UART_FDR_MULVAL(bestm) \
						| UART_FDR_DIVADDVAL(bestd)) & UART_FDR_BITMASK;
			}
			else
			{
				UARTx->LCR |= UART_LCR_DLAB_EN;
				UARTx->/*DLIER.*/DLM = UART_LOAD_DLM(best_divisor);
				UARTx->/*RBTHDLR.*/DLL = UART_LOAD_DLL(best_divisor);
				/* Then reset DLAB bit */
				UARTx->LCR &= (~UART_LCR_DLAB_EN) & UART_LCR_BITMASK;
				UARTx->FDR = (UART_FDR_MULVAL(bestm) \
						| UART_FDR_DIVADDVAL(bestd)) & UART_FDR_BITMASK;
			}
			errorStatus = SUCCESS;
		}

		return errorStatus;
}

/* End of Private Functions ---------------------------------------------------- */

/************************** PUBLIC FUNCTIONS *************************/
/* Public Functions ----------------------------------------------------------- */
/** @addtogroup UART_Public_Functions
 * @{
 */
 
/*-------------------------PUBLIC FUNCTIONS------------------------------*/
 /* UART Initialization Config function ---------------------------------*/
/********************************************************************//**
 * @brief		Initializes the UARTx peripheral according to the specified
 *               parameters.
 * @param[in]	UARTx	UART peripheral selected, should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * @param[in]	baud define the baudrate for UARTx
 * @return 		None
 *********************************************************************/
void UART_Config(LPC_UART_TypeDef *UARTx, long int baud)
{
	// UART Configuration structure variable
	UART_CFG_Type UARTConfigStruct;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	// Pin configuration for UART
	PINSEL_CFG_Type PinCfg;

	// DeInit NVIC and SCBNVIC
//	NVIC_DeInit();
//	NVIC_SCBDeInit();

	/* Configure the NVIC Preemption Priority Bits:
	 * two (2) bits of preemption priority, six (6) bits of sub-priority.
	 * Since the Number of Bits used for Priority Levels is five (5), so the
	 * actual bit number of sub-priority is three (3)
	 */
//	NVIC_SetPriorityGrouping(0x05);
//
//	//  Set Vector table offset value
//#if (__RAM_MODE__==1)
//	NVIC_SetVTOR(0x10000000);
//#else
//	NVIC_SetVTOR(0x00000000);
//#endif

	if(UARTx == LPC_UART0)
	{
		/*
		 * Initialize UART0 pin connect
		 */
		PinCfg.Funcnum = 1;
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = 0;
		PinCfg.Pinnum = 2;
		PinCfg.Portnum = 0;
		PINSEL_ConfigPin(&PinCfg);
		PinCfg.Pinnum = 3;
		PINSEL_ConfigPin(&PinCfg);
	}

	else if((LPC_UART1_TypeDef *)UARTx == LPC_UART1)
	{
		/*
		 * Initialize UART1 pin connect
		 */
		PinCfg.Funcnum = 2;
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = 0;
		PinCfg.Pinnum = 0;
		PinCfg.Portnum = 2;
		PINSEL_ConfigPin(&PinCfg);
		PinCfg.Pinnum = 1;
		PINSEL_ConfigPin(&PinCfg);
	}

	else if(UARTx == LPC_UART2)
	{
		/*
		 * Initialize UART2 pin connect
		 */
		PinCfg.Funcnum = 1;
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = 0;
		PinCfg.Pinnum = 10;
		PinCfg.Portnum = 0;
		PINSEL_ConfigPin(&PinCfg);
		PinCfg.Pinnum = 11;
		PINSEL_ConfigPin(&PinCfg);
	}

	/* Initialize UART Configuration parameter structure to default state:
	 * Baudrate = 9600bps
	 * 8 data bit
	 * 1 Stop bit
	 * None parity
	 */

	UART_ConfigStructInit(&UARTConfigStruct);
	UARTConfigStruct.Baud_rate = baud;

	// Initialize UARTx peripheral with given to corresponding parameter
	UART_Init(UARTx, &UARTConfigStruct);

	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UARTx peripheral
	UART_FIFOConfig(UARTx, &UARTFIFOConfigStruct);

	// Enable UARTx Transmit
	UART_TxCmd(UARTx, ENABLE);

#ifdef INTERRUPT_MODE
	UART_IntConfig(UARTx, UART_INTCFG_RBR, ENABLE);
	/* Enable UART line status interrupt */
	UART_IntConfig(UARTx, UART_INTCFG_RLS, ENABLE);

	/**
	 * Do not enable transmit interrupt here, since it is handled by
	 * UART_Send() function, just to reset Tx Interrupt state for the
	 * first time
	 */
	TxIntStat = RESET;

	// Reset ring buf head and tail idx
	__BUF_RESET(rb.rx_head);
	__BUF_RESET(rb.rx_tail);
	__BUF_RESET(rb.tx_head);
	__BUF_RESET(rb.tx_tail);

	if(UARTx == LPC_UART0)
	{
		/* preemption = 1, sub-priority = 1 */
		NVIC_SetPriority(UART0_IRQn, ((0x01<<3)|0x01));
		/* Enable Interrupt for UART0 channel */
		NVIC_EnableIRQ(UART0_IRQn);
	}
	else if(UARTx == LPC_UART2)
	{
		/* preemption = 1, sub-priority = 1 */
		NVIC_SetPriority(UART2_IRQn, 2);
		/* Enable Interrupt for UART2 channel */
		NVIC_EnableIRQ(UART2_IRQn);
	}

#endif
}

/* UART Init/DeInit functions -------------------------------------------------*/
/********************************************************************//**
 * @brief		Initializes the UARTx peripheral according to the specified
 *               parameters in the UART_ConfigStruct.
 * @param[in]	UARTx	UART peripheral selected, should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @param[in]	UART_ConfigStruct Pointer to a UART_CFG_Type structure
*                    that contains the configuration information for the
*                    specified UART peripheral.
 * @return 		None
 *********************************************************************/
void UART_Init(LPC_UART_TypeDef *UARTx, UART_CFG_Type *UART_ConfigStruct)
{
	uint32_t tmp;

	// For debug mode
	CHECK_PARAM(PARAM_UARTx(UARTx));
	CHECK_PARAM(PARAM_UART_DATABIT(UART_ConfigStruct->Databits));
	CHECK_PARAM(PARAM_UART_STOPBIT(UART_ConfigStruct->Stopbits));
	CHECK_PARAM(PARAM_UART_PARITY(UART_ConfigStruct->Parity));


	if(UARTx == (LPC_UART_TypeDef *)LPC_UART0)
	{
		/* Set up clock and power for UART module */
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCUART0, ENABLE);
	}



	if(((LPC_UART1_TypeDef *)UARTx) == LPC_UART1)
	{
		/* Set up clock and power for UART module */
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCUART1, ENABLE);
	}



	if(UARTx == LPC_UART2)
	{
		/* Set up clock and power for UART module */
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCUART2, ENABLE);
	}



	if(UARTx == LPC_UART3)
	{
		/* Set up clock and power for UART module */
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCUART3, ENABLE);
	}


	if (((LPC_UART1_TypeDef *)UARTx) == LPC_UART1)
	{
		/* FIFOs are empty */
		((LPC_UART1_TypeDef *)UARTx)->/*IIFCR.*/FCR = ( UART_FCR_FIFO_EN \
				| UART_FCR_RX_RS | UART_FCR_TX_RS);
		// Disable FIFO
		((LPC_UART1_TypeDef *)UARTx)->/*IIFCR.*/FCR = 0;

		// Dummy reading
		while (((LPC_UART1_TypeDef *)UARTx)->LSR & UART_LSR_RDR)
		{
			tmp = ((LPC_UART1_TypeDef *)UARTx)->/*RBTHDLR.*/RBR;
		}

		((LPC_UART1_TypeDef *)UARTx)->TER = UART_TER_TXEN;
		// Wait for current transmit complete
		while (!(((LPC_UART1_TypeDef *)UARTx)->LSR & UART_LSR_THRE));
		// Disable Tx
		((LPC_UART1_TypeDef *)UARTx)->TER = 0;

		// Disable interrupt
		((LPC_UART1_TypeDef *)UARTx)->/*DLIER.*/IER = 0;
		// Set LCR to default state
		((LPC_UART1_TypeDef *)UARTx)->LCR = 0;
		// Set ACR to default state
		((LPC_UART1_TypeDef *)UARTx)->ACR = 0;
		// Set Modem Control to default state
		((LPC_UART1_TypeDef *)UARTx)->MCR = 0;
		// Set RS485 control to default state
		((LPC_UART1_TypeDef *)UARTx)->RS485CTRL = 0;
		// Set RS485 delay timer to default state
		((LPC_UART1_TypeDef *)UARTx)->RS485DLY = 0;
		// Set RS485 addr match to default state
		((LPC_UART1_TypeDef *)UARTx)->ADRMATCH = 0;
		//Dummy Reading to Clear Status
		tmp = ((LPC_UART1_TypeDef *)UARTx)->MSR;
		tmp = ((LPC_UART1_TypeDef *)UARTx)->LSR;
	}
	else
	{
		/* FIFOs are empty */
		UARTx->/*IIFCR.*/FCR = ( UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS);
		// Disable FIFO
		UARTx->/*IIFCR.*/FCR = 0;

		// Dummy reading
		while (UARTx->LSR & UART_LSR_RDR)
		{
			tmp = UARTx->/*RBTHDLR.*/RBR;
		}

		UARTx->TER = UART_TER_TXEN;
		// Wait for current transmit complete
		while (!(UARTx->LSR & UART_LSR_THRE));
		// Disable Tx
		UARTx->TER = 0;

		// Disable interrupt
		UARTx->/*DLIER.*/IER = 0;
		// Set LCR to default state
		UARTx->LCR = 0;
		// Set ACR to default state
		UARTx->ACR = 0;
		// Dummy reading
		tmp = UARTx->LSR;
	}

	if (UARTx == LPC_UART3)
	{
		// Set IrDA to default state
		UARTx->ICR = 0;
	}

	// Set Line Control register ----------------------------

	uart_set_divisors(UARTx, (UART_ConfigStruct->Baud_rate));

	if (((LPC_UART1_TypeDef *)UARTx) == LPC_UART1)
	{
		tmp = (((LPC_UART1_TypeDef *)UARTx)->LCR & (UART_LCR_DLAB_EN | UART_LCR_BREAK_EN)) \
				& UART_LCR_BITMASK;
	}
	else
	{
		tmp = (UARTx->LCR & (UART_LCR_DLAB_EN | UART_LCR_BREAK_EN)) & UART_LCR_BITMASK;
	}

	switch (UART_ConfigStruct->Databits){
	case UART_DATABIT_5:
		tmp |= UART_LCR_WLEN5;
		break;
	case UART_DATABIT_6:
		tmp |= UART_LCR_WLEN6;
		break;
	case UART_DATABIT_7:
		tmp |= UART_LCR_WLEN7;
		break;
	case UART_DATABIT_8:
	default:
		tmp |= UART_LCR_WLEN8;
		break;
	}

	if (UART_ConfigStruct->Parity == UART_PARITY_NONE)
	{
		// Do nothing...
	}
	else
	{
		tmp |= UART_LCR_PARITY_EN;
		switch (UART_ConfigStruct->Parity)
		{
		case UART_PARITY_ODD:
			tmp |= UART_LCR_PARITY_ODD;
			break;

		case UART_PARITY_EVEN:
			tmp |= UART_LCR_PARITY_EVEN;
			break;

		case UART_PARITY_SP_1:
			tmp |= UART_LCR_PARITY_F_1;
			break;

		case UART_PARITY_SP_0:
			tmp |= UART_LCR_PARITY_F_0;
			break;
		default:
			break;
		}
	}

	switch (UART_ConfigStruct->Stopbits){
	case UART_STOPBIT_2:
		tmp |= UART_LCR_STOPBIT_SEL;
		break;
	case UART_STOPBIT_1:
	default:
		// Do no thing
		break;
	}


	// Write back to LCR, configure FIFO and Disable Tx
	if (((LPC_UART1_TypeDef *)UARTx) ==  LPC_UART1)
	{
		((LPC_UART1_TypeDef *)UARTx)->LCR = (uint8_t)(tmp & UART_LCR_BITMASK);
	}
	else
	{
		UARTx->LCR = (uint8_t)(tmp & UART_LCR_BITMASK);
	}
}

/*********************************************************************//**
 * @brief		De-initializes the UARTx peripheral registers to their
 *                  default reset values.
 * @param[in]	UARTx	UART peripheral selected, should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @return 		None
 **********************************************************************/
void UART_DeInit(LPC_UART_TypeDef* UARTx)
{
	// For debug mode
	CHECK_PARAM(PARAM_UARTx(UARTx));

	UART_TxCmd(UARTx, DISABLE);

#ifdef _UART0
	if (UARTx == (LPC_UART_TypeDef *)LPC_UART0)
	{
		/* Set up clock and power for UART module */
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCUART0, DISABLE);
	}
#endif

#ifdef _UART1
	if (((LPC_UART1_TypeDef *)UARTx) == LPC_UART1)
	{
		/* Set up clock and power for UART module */
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCUART1, DISABLE);
	}
#endif

#ifdef _UART2
	if (UARTx == LPC_UART2)
	{
		/* Set up clock and power for UART module */
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCUART2, DISABLE);
	}
#endif

#ifdef _UART3
	if (UARTx == LPC_UART3)
	{
		/* Set up clock and power for UART module */
		CLKPWR_ConfigPPWR (CLKPWR_PCONP_PCUART3, DISABLE);
	}
#endif
}

/*****************************************************************************//**
* @brief		Fills each UART_InitStruct member with its default value:
* 				- 9600 bps
* 				- 8-bit data
* 				- 1 Stopbit
* 				- None Parity
* @param[in]	UART_InitStruct Pointer to a UART_CFG_Type structure
*                    which will be initialized.
* @return		None
*******************************************************************************/
void UART_ConfigStructInit(UART_CFG_Type *UART_InitStruct)
{
	UART_InitStruct->Baud_rate = 9600;
	UART_InitStruct->Databits = UART_DATABIT_8;
	UART_InitStruct->Parity = UART_PARITY_NONE;
	UART_InitStruct->Stopbits = UART_STOPBIT_1;
}

/* UART Send/Recieve functions -------------------------------------------------*/
/*********************************************************************//**
 * @brief		Transmit a single data through UART peripheral
 * @param[in]	UARTx	UART peripheral selected, should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @param[in]	Data	Data to transmit (must be 8-bit long)
 * @return 		None
 **********************************************************************/
void UART_SendByte(LPC_UART_TypeDef* UARTx, uint8_t Data)
{
	CHECK_PARAM(PARAM_UARTx(UARTx));

	if (((LPC_UART1_TypeDef *)UARTx) == LPC_UART1)
	{
		((LPC_UART1_TypeDef *)UARTx)->/*RBTHDLR.*/THR = Data & UART_THR_MASKBIT;
	}
	else
	{
		UARTx->/*RBTHDLR.*/THR = Data & UART_THR_MASKBIT;
	}

}


/*********************************************************************//**
 * @brief		Receive a single data from UART peripheral
 * @param[in]	UARTx	UART peripheral selected, should be:
 *  			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @return 		Data received
 **********************************************************************/
uint8_t UART_ReceiveByte(LPC_UART_TypeDef* UARTx)
{
	CHECK_PARAM(PARAM_UARTx(UARTx));

	if (((LPC_UART1_TypeDef *)UARTx) == LPC_UART1)
	{
		EscFlag = 0;                    /* clear EscapeFlag */
		return (((LPC_UART1_TypeDef *)UARTx)->/*RBTHDLR.*/RBR & UART_RBR_MASKBIT);
	}
	else
	{
		EscFlag = 0;                    /* clear EscapeFlag */
		return (UARTx->/*RBTHDLR.*/RBR & UART_RBR_MASKBIT);
	}
}


#ifdef POLLING_MODE
/*********************************************************************//**
 * @brief		Send a block of data via UART peripheral
 * @param[in]	UARTx	Selected UART peripheral used to send data, should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @param[in]	txbuf 	Pointer to Transmit buffer
 * @param[in]	buflen 	Length of Transmit buffer
 * @param[in] 	flag 	Flag used in  UART transfer, should be
 * 						NONE_BLOCKING or BLOCKING
 * @return 		Number of bytes sent.
 *
 * Note: when using UART in BLOCKING mode, a time-out condition is used
 * via defined symbol UART_BLOCKING_TIMEOUT.
 **********************************************************************/
uint32_t UART_Send(LPC_UART_TypeDef *UARTx, uint8_t *txbuf,
		uint32_t buflen, TRANSFER_BLOCK_Type flag)
{
	uint32_t bToSend, bSent, timeOut, fifo_cnt;
	uint8_t *pChar = txbuf;

	bToSend = buflen;

	// blocking mode
	if (flag == BLOCKING) {
		bSent = 0;
		while (bToSend){
			timeOut = UART_BLOCKING_TIMEOUT;
			// Wait for THR empty with timeout
			while (!(UARTx->LSR & UART_LSR_THRE)) {
				if (timeOut == 0) break;
				timeOut--;
			}
			// Time out!
			if(timeOut == 0) break;
			fifo_cnt = UART_TX_FIFO_SIZE;
			while (fifo_cnt && bToSend){
				UART_SendByte(UARTx, (*pChar++));
				fifo_cnt--;
				bToSend--;
				bSent++;
			}
		}
	}
	// None blocking mode
	else {
		bSent = 0;
		while (bToSend) {
			if (!(UARTx->LSR & UART_LSR_THRE)){
				break;
			}
			fifo_cnt = UART_TX_FIFO_SIZE;
			while (fifo_cnt && bToSend) {
				UART_SendByte(UARTx, (*pChar++));
				bToSend--;
				fifo_cnt--;
				bSent++;
			}
		}
	}
	return bSent;
}

/*********************************************************************//**
 * @brief		Receive a block of data via UART peripheral
 * @param[in]	UARTx	Selected UART peripheral used to send data,
 * 				should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @param[out]	rxbuf 	Pointer to Received buffer
 * @param[in]	buflen 	Length of Received buffer
 * @param[in] 	flag 	Flag mode, should be NONE_BLOCKING or BLOCKING

 * @return 		Number of bytes received
 *
 * Note: when using UART in BLOCKING mode, a time-out condition is used
 * via defined symbol UART_BLOCKING_TIMEOUT.
 **********************************************************************/
uint32_t UART_Receive(LPC_UART_TypeDef *UARTx, uint8_t *rxbuf, \
		uint32_t buflen, TRANSFER_BLOCK_Type flag)
{
	uint32_t bToRecv, bRecv, timeOut;
	uint8_t *pChar = rxbuf;

	bToRecv = buflen;

	// Blocking mode
	if (flag == BLOCKING)
	{
		bRecv = 0;
		while (bToRecv)
		{
			while (!(UARTx->LSR & UART_LSR_RDR))
			{
				//Infinite Loop
			}
			// Get data from the buffer
			(*pChar++) = UART_ReceiveByte(UARTx);
			bToRecv--;
			bRecv++;
		}
	}
	else if (flag == TIME_BLOCKING)
	{
		bRecv = 0;
		while (bToRecv)
		{
			timeOut = UART_BLOCKING_TIMEOUT;
			while (!(UARTx->LSR & UART_LSR_RDR))
			{
				if (timeOut == 0) break;
				timeOut--;
			}
			// Time out!
			if(timeOut == 0) break;
			// Get data from the buffer
			(*pChar++) = UART_ReceiveByte(UARTx);
			bToRecv--;
			bRecv++;
		}
	}
	// None blocking mode
	else
	{
		bRecv = 0;
		while (bToRecv)
		{
			if (!(UARTx->LSR & UART_LSR_RDR))
			{
				break;
			}
			else
			{
				(*pChar++) = UART_ReceiveByte(UARTx);
				bRecv++;
				bToRecv--;
			}
		}
	}
	return bRecv;
}
#endif


/*********************************************************************//**
 * @brief	 The getche() function returns the next character read from the
             console and echoes that character to the screen.Characters from
             space(20hex) to (7E) are echo to the screen.The getche() function
             is not define by the ANSI C standard.
 * @param[in]	UARTx	UART peripheral selected, should be:
 *  			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @return 		return with valid character or nothing
 **********************************************************************/
int16 getche(LPC_UART_TypeDef *UARTx)
{
	uint8_t key[1];
	uint32_t idx, len;
	while(1)
	{
	    if(UARTx == LPC_UART0)
	    {
		    len = UART_Receive(UARTx, key, 1, BLOCKING);
		    /* Got some data */
		    idx = 0;
		    while (idx < len)
		    {
		        if ( key[idx] == In_CR )
	            {
			        return(key[idx]);
	            }
	            else if ( key[idx] == In_DELETE || key[idx] == In_BACKSPACE )
	            {
	    	        return(key[idx]);
	            }
	            else if ( key[idx] == In_ESC )
	            {
	    	        EscFlag = 1;
	    	        return ( In_ESC );
	            }
	            else if ( key[idx] >= ' ' )
	            {
	    	        return (key[idx]);
	            }
	            else
	            {
	    	        UART_Send(UARTx,&key[idx],1,BLOCKING);
	            }
		        idx++;
		    }
	    }
	    else if(UARTx == LPC_UART2)
	    {
		    len = UART_Receive(UARTx, key, 1, BLOCKING);
		    /* Got some data */
		    idx = 0;
		    while (idx < len)
		    {
		    	return(key[idx]);
		    }
	    }
	}
    return(0);
}

/*********************************************************************//**
 * @brief	 One line editor
 * @param[in]	UARTx	UART peripheral selected, should be:
 *  			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @param[out]	s[] Pointer to Received buffer
 * @param[in]   lim Length of Received buffer
 * @return 		return character count
 **********************************************************************/
uchar get_line(LPC_UART_TypeDef *UARTx, schar s[], uchar lim)
{
	schar kb;                 /* input character */

	uchar pointer=0;          /* Pointer in buffer */
	uchar count=0;            /* character count */
    uint8_t temp;

	while(1)
	{
		kb = getche(UARTx);

		if (kb == In_ESC)                /* if ESCAPE pressed then exit */
		{
			return (kb);
		}

		else if (kb == In_CR)            /* CARRIAGE return ? */
		{
			s[pointer] = '\0';             /* put null char on last position */
			break;                         /* yes, exit from this loop */
		}

		else if(kb == In_DELETE || kb == In_BACKSPACE)
		{
			if(pointer==0)                 /* any characters entered */
			{
				continue;                    /* no, so get another character */
			}

			temp = Out_BACKSPACE;
			UART_Send(UARTx,&temp,1,BLOCKING);   /* go back one position */
			temp = Out_SPACE;
			UART_Send(UARTx,&temp,1,BLOCKING);   /* erase char */
			temp = Out_BACKSPACE;
			UART_Send(UARTx,&temp,1,BLOCKING);   /* go back one more position */

			pointer--;                     /* decrement pointer */
			count--;                       /* decrement character count */

			continue;
		}

		else if(pointer < lim)
		{
			s[pointer] = kb;               /* save character and increment pointer */
			pointer++;
			count++;                       /* increment count */
			UART_Send(UARTx,&kb,1,BLOCKING);       /* echo character */

			continue;                      /* and get some more */
		}
		else
		{
			temp = In_BELL;
			UART_Send(UARTx,&temp,1,BLOCKING);   /* ring the bell */
		}
	}
	return(count);
}


/*********************************************************************//**
 * @brief		Modified version of Standard Printf statement
 *
 * @par			Supports standard formats "%c %s %d %x"
 * 				"%d" and "%x" requires non-standard qualifiers,"%dfn, %xfn":-
 *		        f supplies a fill character
 *		        n supplies a field width
 *
 *		        ENABLE RTC_SUPPORT in lpc17xx_uart.h for RTC Features
 *
 *				Supports custom formats  "%b  %u %t %y %a"
 *				"%b"	prints a 2 digit BCD value with leading zero
 *				"%u"	prints the 16 bit unsigned integer in hex format
 *				"%t"    prints current time
 *				"%y"    prints current date
 *				"%a"    prints alarm time and date
 * @param[in]	UARTx	Selected UART peripheral used to send data,
 * 				should be:
 *  			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @param[in] 	*format Character format
 * @param[in]   ...  <multiple argument>
 *
 * @return 		return with valid character or nothing
 **********************************************************************/
int16 printf(LPC_UART_TypeDef *UARTx, const char *format, ...)
{
	uchar hex[]= "0123456789ABCDEF";
	unsigned int width_dec[10] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000};
	unsigned int width_hex[10] = { 0x1, 0x10, 0x100, 0x1000, 0x10000, 0x100000, 0x1000000,0x10000000};
	unsigned int temp;

	schar format_flag, fill_char;
	ulong32 u_val, div_val;
	uint16 base;

	schar *ptr;
#ifdef RTC_MODE
	RTC_TIME_Type FullTime;
#endif
	va_list ap;
	va_start(ap, format);

	for(;;)
	{
		while((format_flag = *format++) != '%')      /* until full format string read */
		{
			if(!format_flag)
			{                        /* until '%' or '\0' */
				return (0);
			}
			UART_Send(UARTx,&format_flag,1,BLOCKING);
		}

		switch(format_flag = *format++)
		{
			case 'c':
				format_flag = va_arg(ap, int);
				UART_Send(UARTx,&format_flag,1,BLOCKING);

				continue;

			default:
				UART_Send(UARTx,&format_flag,1,BLOCKING);

        		continue;

			case 'b':
				format_flag = va_arg(ap,int);
				UART_Send(UARTx,&(hex[(uint16)format_flag >> 4]),1,BLOCKING);
				UART_Send(UARTx,&(hex[(uint16)format_flag & 0x0F]),1,BLOCKING);

				continue;

			case 's':
				ptr = va_arg(ap, schar *);
				while(*ptr)
				{
					UART_Send(UARTx,&(*ptr++),1,BLOCKING);
				}

				continue;
#ifdef RTC_MODE
			case 't':
				RTC_GetFullTime (LPC_RTC, &FullTime);
			    printf(UARTx, "%d02:%d02:%d02",FullTime.HOUR,FullTime.MIN,FullTime.SEC);

				continue;

			case 'y':
				RTC_GetFullTime (LPC_RTC, &FullTime);
			    printf(UARTx, "%d02/%d02/%d04",FullTime.DOM,FullTime.MONTH,FullTime.YEAR);

				continue;

			case 'a':
				RTC_GetFullAlarmTime (LPC_RTC, &FullTime);
				printf(UARTx, "Time: %d02:%d02:%d02",FullTime.HOUR,FullTime.MIN,FullTime.SEC);
				printf(UARTx, "  Date: %d02/%d02/%d04",FullTime.DOM,FullTime.MONTH,FullTime.YEAR);

				continue;
#endif
			case 'u':
				base = 16;
				div_val = 0x100000;
				u_val = va_arg(ap, uint32_t);
				do
				{
					UART_Send(UARTx,&(hex[u_val/div_val]),1,BLOCKING);
					u_val %= div_val;
					div_val /= base;
				}while(div_val);

				continue;

			case 'd':
				base = 10;
				fill_char = *format++;
				format_flag = ( *format++ ) - '1';
				div_val = width_dec[format_flag];
				u_val = va_arg(ap,int);

				if(((int)u_val) < 0)
				{
					u_val = - u_val;    /* applied to unsigned type, result still unsigned */
					temp = '-';
				    UART_Send(UARTx,&temp,1,BLOCKING);
				}

				goto  CONVERSION_LOOP;

			case 'x':
				base = 16;
				fill_char = *format++;
				format_flag = (*format++) - '1';
				div_val = width_hex[format_flag];
				u_val = va_arg(ap, int);

				CONVERSION_LOOP:
				while(div_val > 1 && div_val > u_val)
				{
					div_val /= base;
					UART_Send(UARTx,&fill_char,1,BLOCKING);
				}

				do
				{
					UART_Send(UARTx,&(hex[u_val/div_val]),1,BLOCKING);
					u_val %= div_val;
					div_val /= base;
				}while(div_val);
		}/* end of switch statement */
	}
	return(0);
}


/*********************************************************************//**
 * @brief		Force BREAK character on UART line, output pin UARTx TXD is
				forced to logic 0.
 * @param[in]	UARTx	UART peripheral selected, should be:
 *  			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @return 		None
 **********************************************************************/
void UART_ForceBreak(LPC_UART_TypeDef* UARTx)
{
	CHECK_PARAM(PARAM_UARTx(UARTx));

	if (((LPC_UART1_TypeDef *)UARTx) == LPC_UART1)
	{
		((LPC_UART1_TypeDef *)UARTx)->LCR |= UART_LCR_BREAK_EN;
	}
	else
	{
		UARTx->LCR |= UART_LCR_BREAK_EN;
	}
}


/********************************************************************//**
 * @brief 		Enable or disable specified UART interrupt.
 * @param[in]	UARTx	UART peripheral selected, should be
 *  			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @param[in]	UARTIntCfg	Specifies the interrupt flag,
 * 				should be one of the following:
				- UART_INTCFG_RBR 	:  RBR Interrupt enable
				- UART_INTCFG_THRE 	:  THR Interrupt enable
				- UART_INTCFG_RLS 	:  RX line status interrupt enable
				- UART1_INTCFG_MS	:  Modem status interrupt enable (UART1 only)
				- UART1_INTCFG_CTS	:  CTS1 signal transition interrupt enable (UART1 only)
				- UART_INTCFG_ABEO 	:  Enables the end of auto-baud interrupt
				- UART_INTCFG_ABTO 	:  Enables the auto-baud time-out interrupt
 * @param[in]	NewState New state of specified UART interrupt type,
 * 				should be:
 * 				- ENALBE: Enable this UART interrupt type.
* 				- DISALBE: Disable this UART interrupt type.
 * @return 		None
 *********************************************************************/
void UART_IntConfig(LPC_UART_TypeDef *UARTx, UART_INT_Type UARTIntCfg, FunctionalState NewState)
{
	uint32_t tmp;

	CHECK_PARAM(PARAM_UARTx(UARTx));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(NewState));

	switch(UARTIntCfg){
		case UART_INTCFG_RBR:
			tmp = UART_IER_RBRINT_EN;
			break;
		case UART_INTCFG_THRE:
			tmp = UART_IER_THREINT_EN;
			break;
		case UART_INTCFG_RLS:
			tmp = UART_IER_RLSINT_EN;
			break;
		case UART1_INTCFG_MS:
			tmp = UART1_IER_MSINT_EN;
			break;
		case UART1_INTCFG_CTS:
			tmp = UART1_IER_CTSINT_EN;
			break;
		case UART_INTCFG_ABEO:
			tmp = UART_IER_ABEOINT_EN;
			break;
		case UART_INTCFG_ABTO:
			tmp = UART_IER_ABTOINT_EN;
			break;
	}

	if ((LPC_UART1_TypeDef *) UARTx == LPC_UART1)
	{
		CHECK_PARAM((PARAM_UART_INTCFG(UARTIntCfg)) || (PARAM_UART1_INTCFG(UARTIntCfg)));
	}
	else
	{
		CHECK_PARAM(PARAM_UART_INTCFG(UARTIntCfg));
	}

	if (NewState == ENABLE)
	{
		if ((LPC_UART1_TypeDef *) UARTx == LPC_UART1)
		{
			((LPC_UART1_TypeDef *)UARTx)->/*DLIER.*/IER |= tmp;
		}
		else
		{
			UARTx->/*DLIER.*/IER |= tmp;
		}
	}
	else
	{
		if ((LPC_UART1_TypeDef *) UARTx == LPC_UART1)
		{
			((LPC_UART1_TypeDef *)UARTx)->/*DLIER.*/IER &= (~tmp) & UART1_IER_BITMASK;
		}
		else
		{
			UARTx->/*DLIER.*/IER &= (~tmp) & UART_IER_BITMASK;
		}
	}
}


/********************************************************************//**
 * @brief 		Get current value of Line Status register in UART peripheral.
 * @param[in]	UARTx	UART peripheral selected, should be:
 *  			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @return		Current value of Line Status register in UART peripheral.
 * Note:	The return value of this function must be ANDed with each member in
 * 			UART_LS_Type enumeration to determine current flag status
 * 			corresponding to each Line status type. Because some flags in
 * 			Line Status register will be cleared after reading, the next reading
 * 			Line Status register could not be correct. So this function used to
 * 			read Line status register in one time only, then the return value
 * 			used to check all flags.
 *********************************************************************/
uint8_t UART_GetLineStatus(LPC_UART_TypeDef* UARTx)
{
	CHECK_PARAM(PARAM_UARTx(UARTx));

	if (((LPC_UART1_TypeDef *)UARTx) == LPC_UART1)
	{
		return ((((LPC_UART1_TypeDef *)LPC_UART1)->LSR) & UART_LSR_BITMASK);
	}
	else
	{
		return ((UARTx->LSR) & UART_LSR_BITMASK);
	}
}

/********************************************************************//**
 * @brief 		Get Interrupt Identification value
 * @param[in]	UARTx	UART peripheral selected, should be:
 *  			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @return		Current value of UART UIIR register in UART peripheral.
 *********************************************************************/
uint32_t UART_GetIntId(LPC_UART_TypeDef* UARTx)
{
	CHECK_PARAM(PARAM_UARTx(UARTx));
	return (UARTx->IIR & 0x03CF);
}

/*********************************************************************//**
 * @brief		Check whether if UART is busy or not
 * @param[in]	UARTx	UART peripheral selected, should be:
 *  			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @return		RESET if UART is not busy, otherwise return SET.
 **********************************************************************/
FlagStatus UART_CheckBusy(LPC_UART_TypeDef *UARTx)
{
	if (UARTx->LSR & UART_LSR_TEMT){
		return RESET;
	} else {
		return SET;
	}
}


/*********************************************************************//**
 * @brief		Configure FIFO function on selected UART peripheral
 * @param[in]	UARTx	UART peripheral selected, should be:
 *  			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @param[in]	FIFOCfg	Pointer to a UART_FIFO_CFG_Type Structure that
 * 						contains specified information about FIFO configuration
 * @return 		none
 **********************************************************************/
void UART_FIFOConfig(LPC_UART_TypeDef *UARTx, UART_FIFO_CFG_Type *FIFOCfg)
{
	uint8_t tmp = 0;

	CHECK_PARAM(PARAM_UARTx(UARTx));
	CHECK_PARAM(PARAM_UART_FIFO_LEVEL(FIFOCfg->FIFO_Level));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(FIFOCfg->FIFO_DMAMode));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(FIFOCfg->FIFO_ResetRxBuf));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(FIFOCfg->FIFO_ResetTxBuf));

	tmp |= UART_FCR_FIFO_EN;
	switch (FIFOCfg->FIFO_Level){
	case UART_FIFO_TRGLEV0:
		tmp |= UART_FCR_TRG_LEV0;
		break;
	case UART_FIFO_TRGLEV1:
		tmp |= UART_FCR_TRG_LEV1;
		break;
	case UART_FIFO_TRGLEV2:
		tmp |= UART_FCR_TRG_LEV2;
		break;
	case UART_FIFO_TRGLEV3:
	default:
		tmp |= UART_FCR_TRG_LEV3;
		break;
	}

	if (FIFOCfg->FIFO_ResetTxBuf == ENABLE)
	{
		tmp |= UART_FCR_TX_RS;
	}
	if (FIFOCfg->FIFO_ResetRxBuf == ENABLE)
	{
		tmp |= UART_FCR_RX_RS;
	}
	if (FIFOCfg->FIFO_DMAMode == ENABLE)
	{
		tmp |= UART_FCR_DMAMODE_SEL;
	}


	//write to FIFO control register
	if (((LPC_UART1_TypeDef *)UARTx) == LPC_UART1)
	{
		((LPC_UART1_TypeDef *)UARTx)->/*IIFCR.*/FCR = tmp & UART_FCR_BITMASK;
	}
	else
	{
		UARTx->/*IIFCR.*/FCR = tmp & UART_FCR_BITMASK;
	}
}

/*****************************************************************************//**
* @brief		Fills each UART_FIFOInitStruct member with its default value:
* 				- FIFO_DMAMode = DISABLE
* 				- FIFO_Level = UART_FIFO_TRGLEV0
* 				- FIFO_ResetRxBuf = ENABLE
* 				- FIFO_ResetTxBuf = ENABLE
* 				- FIFO_State = ENABLE

* @param[in]	UART_FIFOInitStruct Pointer to a UART_FIFO_CFG_Type structure
*                    which will be initialized.
* @return		None
*******************************************************************************/
void UART_FIFOConfigStructInit(UART_FIFO_CFG_Type *UART_FIFOInitStruct)
{
	UART_FIFOInitStruct->FIFO_DMAMode = DISABLE;
	UART_FIFOInitStruct->FIFO_Level = UART_FIFO_TRGLEV0;
	UART_FIFOInitStruct->FIFO_ResetRxBuf = ENABLE;
	UART_FIFOInitStruct->FIFO_ResetTxBuf = ENABLE;
}


/*********************************************************************//**
 * @brief		Start/Stop Auto Baudrate activity
 * @param[in]	UARTx	UART peripheral selected, should be
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @param[in]	ABConfigStruct	A pointer to UART_AB_CFG_Type structure that
 * 								contains specified information about UART
 * 								auto baudrate configuration
 * @param[in]	NewState New State of Auto baudrate activity, should be:
 * 				- ENABLE: Start this activity
 *				- DISABLE: Stop this activity
 * Note:		Auto-baudrate mode enable bit will be cleared once this mode
 * 				completed.
 * @return 		none
 **********************************************************************/
void UART_ABCmd(LPC_UART_TypeDef *UARTx, UART_AB_CFG_Type *ABConfigStruct, \
				FunctionalState NewState)
{
	uint32_t tmp;

	CHECK_PARAM(PARAM_UARTx(UARTx));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(NewState));

	tmp = 0;
	if (NewState == ENABLE) {
		if (ABConfigStruct->ABMode == UART_AUTOBAUD_MODE1){
			tmp |= UART_ACR_MODE;
		}
		if (ABConfigStruct->AutoRestart == ENABLE){
			tmp |= UART_ACR_AUTO_RESTART;
		}
	}

	if (((LPC_UART1_TypeDef *)UARTx) == LPC_UART1)
	{
		if (NewState == ENABLE)
		{
			// Clear DLL and DLM value
			((LPC_UART1_TypeDef *)UARTx)->LCR |= UART_LCR_DLAB_EN;
			((LPC_UART1_TypeDef *)UARTx)->DLL = 0;
			((LPC_UART1_TypeDef *)UARTx)->DLM = 0;
			((LPC_UART1_TypeDef *)UARTx)->LCR &= ~UART_LCR_DLAB_EN;
			// FDR value must be reset to default value
			((LPC_UART1_TypeDef *)UARTx)->FDR = 0x10;
			((LPC_UART1_TypeDef *)UARTx)->ACR = UART_ACR_START | tmp;
		}
		else
		{
			((LPC_UART1_TypeDef *)UARTx)->ACR = 0;
		}
	}
	else
	{
		if (NewState == ENABLE)
		{
			// Clear DLL and DLM value
			UARTx->LCR |= UART_LCR_DLAB_EN;
			UARTx->DLL = 0;
			UARTx->DLM = 0;
			UARTx->LCR &= ~UART_LCR_DLAB_EN;
			// FDR value must be reset to default value
			UARTx->FDR = 0x10;
			UARTx->ACR = UART_ACR_START | tmp;
		}
		else
		{
			UARTx->ACR = 0;
		}
	}
}

/*********************************************************************//**
 * @brief		Clear Autobaud Interrupt Pending
 * @param[in]	UARTx	UART peripheral selected, should be
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @param[in]	ABIntType	type of auto-baud interrupt, should be:
 * 				- UART_AUTOBAUD_INTSTAT_ABEO: End of Auto-baud interrupt
 * 				- UART_AUTOBAUD_INTSTAT_ABTO: Auto-baud time out interrupt
 * @return 		none
 **********************************************************************/
void UART_ABClearIntPending(LPC_UART_TypeDef *UARTx, UART_ABEO_Type ABIntType)
{
	CHECK_PARAM(PARAM_UARTx(UARTx));
	if (((LPC_UART1_TypeDef *)UARTx) == LPC_UART1)
	{
		UARTx->ACR |= ABIntType;
	}
	else
		UARTx->ACR |= ABIntType;
}

/*********************************************************************//**
 * @brief		Enable/Disable transmission on UART TxD pin
 * @param[in]	UARTx	UART peripheral selected, should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @param[in]	NewState New State of Tx transmission function, should be:
 * 				- ENABLE: Enable this function
				- DISABLE: Disable this function
 * @return none
 **********************************************************************/
void UART_TxCmd(LPC_UART_TypeDef *UARTx, FunctionalState NewState)
{
	CHECK_PARAM(PARAM_UARTx(UARTx));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(NewState));

	if (NewState == ENABLE)
	{
		if (((LPC_UART1_TypeDef *)UARTx) == LPC_UART1)
		{
			((LPC_UART1_TypeDef *)UARTx)->TER |= UART_TER_TXEN;
		}
		else
		{
			UARTx->TER |= UART_TER_TXEN;
		}
	}
	else
	{
		if (((LPC_UART1_TypeDef *)UARTx) == LPC_UART1)
		{
			((LPC_UART1_TypeDef *)UARTx)->TER &= (~UART_TER_TXEN) & UART_TER_BITMASK;
		}
		else
		{
			UARTx->TER &= (~UART_TER_TXEN) & UART_TER_BITMASK;
		}
	}
}

/* UART IrDA functions ---------------------------------------------------*/

/*********************************************************************//**
 * @brief		Enable or disable inverting serial input function of IrDA
 * 				on UART peripheral.
 * @param[in]	UARTx UART peripheral selected, should be LPC_UART3 (only)
 * @param[in]	NewState New state of inverting serial input, should be:
 * 				- ENABLE: Enable this function.
 * 				- DISABLE: Disable this function.
 * @return none
 **********************************************************************/
void UART_IrDAInvtInputCmd(LPC_UART_TypeDef* UARTx, FunctionalState NewState)
{
	CHECK_PARAM(PARAM_UART_IrDA(UARTx));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(NewState));

	if (NewState == ENABLE)
	{
		UARTx->ICR |= UART_ICR_IRDAINV;
	}
	else if (NewState == DISABLE)
	{
		UARTx->ICR &= (~UART_ICR_IRDAINV) & UART_ICR_BITMASK;
	}
}


/*********************************************************************//**
 * @brief		Enable or disable IrDA function on UART peripheral.
 * @param[in]	UARTx UART peripheral selected, should be LPC_UART3 (only)
 * @param[in]	NewState New state of IrDA function, should be:
 * 				- ENABLE: Enable this function.
 * 				- DISABLE: Disable this function.
 * @return none
 **********************************************************************/
void UART_IrDACmd(LPC_UART_TypeDef* UARTx, FunctionalState NewState)
{
	CHECK_PARAM(PARAM_UART_IrDA(UARTx));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(NewState));

	if (NewState == ENABLE)
	{
		UARTx->ICR |= UART_ICR_IRDAEN;
	}
	else
	{
		UARTx->ICR &= (~UART_ICR_IRDAEN) & UART_ICR_BITMASK;
	}
}


/*********************************************************************//**
 * @brief		Configure Pulse divider for IrDA function on UART peripheral.
 * @param[in]	UARTx UART peripheral selected, should be LPC_UART3 (only)
 * @param[in]	PulseDiv Pulse Divider value from Peripheral clock,
 * 				should be one of the following:
				- UART_IrDA_PULSEDIV2 	: Pulse width = 2 * Tpclk
				- UART_IrDA_PULSEDIV4 	: Pulse width = 4 * Tpclk
				- UART_IrDA_PULSEDIV8 	: Pulse width = 8 * Tpclk
				- UART_IrDA_PULSEDIV16 	: Pulse width = 16 * Tpclk
				- UART_IrDA_PULSEDIV32 	: Pulse width = 32 * Tpclk
				- UART_IrDA_PULSEDIV64 	: Pulse width = 64 * Tpclk
				- UART_IrDA_PULSEDIV128 : Pulse width = 128 * Tpclk
				- UART_IrDA_PULSEDIV256 : Pulse width = 256 * Tpclk

 * @return none
 **********************************************************************/
void UART_IrDAPulseDivConfig(LPC_UART_TypeDef *UARTx, UART_IrDA_PULSE_Type PulseDiv)
{
	uint32_t tmp, tmp1;
	CHECK_PARAM(PARAM_UART_IrDA(UARTx));
	CHECK_PARAM(PARAM_UART_IrDA_PULSEDIV(PulseDiv));

	tmp1 = UART_ICR_PULSEDIV(PulseDiv);
	tmp = UARTx->ICR & (~UART_ICR_PULSEDIV(7));
	tmp |= tmp1 | UART_ICR_FIXPULSE_EN;
	UARTx->ICR = tmp & UART_ICR_BITMASK;
}



/* UART1 FullModem function ---------------------------------------------*/

/*********************************************************************//**
 * @brief		Force pin DTR/RTS corresponding to given state (Full modem mode)
 * @param[in]	UARTx	LPC_UART1 (only)
 * @param[in]	Pin	Pin that NewState will be applied to, should be:
 * 				- UART1_MODEM_PIN_DTR: DTR pin.
 * 				- UART1_MODEM_PIN_RTS: RTS pin.
 * @param[in]	NewState New State of DTR/RTS pin, should be:
 * 				- INACTIVE: Force the pin to inactive signal.
				- ACTIVE: Force the pin to active signal.
 * @return none
 **********************************************************************/
void UART_FullModemForcePinState(LPC_UART1_TypeDef *UARTx, UART_MODEM_PIN_Type Pin, \
							UART1_SignalState NewState)
{
	uint8_t tmp = 0;

	CHECK_PARAM(PARAM_UART1_MODEM(UARTx));
	CHECK_PARAM(PARAM_UART1_MODEM_PIN(Pin));
	CHECK_PARAM(PARAM_UART1_SIGNALSTATE(NewState));

	switch (Pin){
	case UART1_MODEM_PIN_DTR:
		tmp = UART1_MCR_DTR_CTRL;
		break;
	case UART1_MODEM_PIN_RTS:
		tmp = UART1_MCR_RTS_CTRL;
		break;
	default:
		break;
	}

	if (NewState == ACTIVE){
		UARTx->MCR |= tmp;
	} else {
		UARTx->MCR &= (~tmp) & UART1_MCR_BITMASK;
	}
}


/*********************************************************************//**
 * @brief		Configure Full Modem mode for UART peripheral
 * @param[in]	UARTx	LPC_UART1 (only)
 * @param[in]	Mode Full Modem mode, should be:
 * 				- UART1_MODEM_MODE_LOOPBACK: Loop back mode.
 * 				- UART1_MODEM_MODE_AUTO_RTS: Auto-RTS mode.
 * 				- UART1_MODEM_MODE_AUTO_CTS: Auto-CTS mode.
 * @param[in]	NewState New State of this mode, should be:
 * 				- ENABLE: Enable this mode.
				- DISABLE: Disable this mode.
 * @return none
 **********************************************************************/
void UART_FullModemConfigMode(LPC_UART1_TypeDef *UARTx, UART_MODEM_MODE_Type Mode, \
							FunctionalState NewState)
{
	uint8_t tmp;

	CHECK_PARAM(PARAM_UART1_MODEM(UARTx));
	CHECK_PARAM(PARAM_UART1_MODEM_MODE(Mode));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(NewState));

	switch(Mode){
	case UART1_MODEM_MODE_LOOPBACK:
		tmp = UART1_MCR_LOOPB_EN;
		break;
	case UART1_MODEM_MODE_AUTO_RTS:
		tmp = UART1_MCR_AUTO_RTS_EN;
		break;
	case UART1_MODEM_MODE_AUTO_CTS:
		tmp = UART1_MCR_AUTO_CTS_EN;
		break;
	default:
		break;
	}

	if (NewState == ENABLE)
	{
		UARTx->MCR |= tmp;
	}
	else
	{
		UARTx->MCR &= (~tmp) & UART1_MCR_BITMASK;
	}
}


/*********************************************************************//**
 * @brief		Get current status of modem status register
 * @param[in]	UARTx	LPC_UART1 (only)
 * @return 		Current value of modem status register
 * Note:	The return value of this function must be ANDed with each member
 * 			UART_MODEM_STAT_type enumeration to determine current flag status
 * 			corresponding to each modem flag status. Because some flags in
 * 			modem status register will be cleared after reading, the next reading
 * 			modem register could not be correct. So this function used to
 * 			read modem status register in one time only, then the return value
 * 			used to check all flags.
 **********************************************************************/
uint8_t UART_FullModemGetStatus(LPC_UART1_TypeDef *UARTx)
{
	CHECK_PARAM(PARAM_UART1_MODEM(UARTx));
	return ((UARTx->MSR) & UART1_MSR_BITMASK);
}


/* UART RS485 functions --------------------------------------------------------------*/

/*********************************************************************//**
 * @brief		Configure UART peripheral in RS485 mode according to the specified
*               parameters in the RS485ConfigStruct.
 * @param[in]	UARTx	LPC_UART1 (only)
 * @param[in]	RS485ConfigStruct Pointer to a UART1_RS485_CTRLCFG_Type structure
*                    that contains the configuration information for specified UART
*                    in RS485 mode.
 * @return		None
 **********************************************************************/
void UART_RS485Config(LPC_UART1_TypeDef *UARTx, UART1_RS485_CTRLCFG_Type *RS485ConfigStruct)
{
	uint32_t tmp;

	CHECK_PARAM(PARAM_UART1_MODEM(UARTx));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(RS485ConfigStruct->AutoAddrDetect_State));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(RS485ConfigStruct->AutoDirCtrl_State));
	CHECK_PARAM(PARAM_UART1_RS485_CFG_DELAYVALUE(RS485ConfigStruct->DelayValue));
	CHECK_PARAM(PARAM_SETSTATE(RS485ConfigStruct->DirCtrlPol_Level));
	CHECK_PARAM(PARAM_UART_RS485_DIRCTRL_PIN(RS485ConfigStruct->DirCtrlPin));
	CHECK_PARAM(PARAM_UART1_RS485_CFG_MATCHADDRVALUE(RS485ConfigStruct->MatchAddrValue));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(RS485ConfigStruct->NormalMultiDropMode_State));
	CHECK_PARAM(PARAM_FUNCTIONALSTATE(RS485ConfigStruct->Rx_State));

	tmp = 0;
	// If Auto Direction Control is enabled -  This function is used in Master mode
	if (RS485ConfigStruct->AutoDirCtrl_State == ENABLE)
	{
		tmp |= UART1_RS485CTRL_DCTRL_EN;

		// Set polar
		if (RS485ConfigStruct->DirCtrlPol_Level == SET)
		{
			tmp |= UART1_RS485CTRL_OINV_1;
		}

		// Set pin according to
		if (RS485ConfigStruct->DirCtrlPin == UART1_RS485_DIRCTRL_DTR)
		{
			tmp |= UART1_RS485CTRL_SEL_DTR;
		}

		// Fill delay time
		UARTx->RS485DLY = RS485ConfigStruct->DelayValue & UART1_RS485DLY_BITMASK;
	}

	// MultiDrop mode is enable
	if (RS485ConfigStruct->NormalMultiDropMode_State == ENABLE)
	{
		tmp |= UART1_RS485CTRL_NMM_EN;
	}

	// Auto Address Detect function
	if (RS485ConfigStruct->AutoAddrDetect_State == ENABLE)
	{
		tmp |= UART1_RS485CTRL_AADEN;
		// Fill Match Address
		UARTx->ADRMATCH = RS485ConfigStruct->MatchAddrValue & UART1_RS485ADRMATCH_BITMASK;
	}


	// Receiver is disable
	if (RS485ConfigStruct->Rx_State == DISABLE)
	{
		tmp |= UART1_RS485CTRL_RX_DIS;
	}

	// write back to RS485 control register
	UARTx->RS485CTRL = tmp & UART1_RS485CTRL_BITMASK;

	// Enable Parity function and leave parity in stick '0' parity as default
	UARTx->LCR |= (UART_LCR_PARITY_F_0 | UART_LCR_PARITY_EN);
}

/*********************************************************************//**
 * @brief		Enable/Disable receiver in RS485 module in UART1
 * @param[in]	UARTx	LPC_UART1 (only)
 * @param[in]	NewState	New State of command, should be:
 * 							- ENABLE: Enable this function.
 * 							- DISABLE: Disable this function.
 * @return		None
 **********************************************************************/
void UART_RS485ReceiverCmd(LPC_UART1_TypeDef *UARTx, FunctionalState NewState)
{
	if (NewState == ENABLE){
		UARTx->RS485CTRL &= ~UART1_RS485CTRL_RX_DIS;
	} else {
		UARTx->RS485CTRL |= UART1_RS485CTRL_RX_DIS;
	}
}

/*********************************************************************//**
 * @brief		Send data on RS485 bus with specified parity stick value (9-bit mode).
 * @param[in]	UARTx	LPC_UART1 (only)
 * @param[in]	pDatFrm 	Pointer to data frame.
 * @param[in]	size		Size of data.
 * @param[in]	ParityStick	Parity Stick value, should be 0 or 1.
 * @return		None
 **********************************************************************/
uint32_t UART_RS485Send(LPC_UART1_TypeDef *UARTx, uint8_t *pDatFrm, \
					uint32_t size, uint8_t ParityStick)
{
	uint8_t tmp, save;
	uint32_t cnt;

	if (ParityStick){
		save = tmp = UARTx->LCR & UART_LCR_BITMASK;
		tmp &= ~(UART_LCR_PARITY_EVEN);
		UARTx->LCR = tmp;
		cnt = UART_Send((LPC_UART_TypeDef *)UARTx, pDatFrm, size, BLOCKING);
		while (!(UARTx->LSR & UART_LSR_TEMT));
		UARTx->LCR = save;
	} else {
		cnt = UART_Send((LPC_UART_TypeDef *)UARTx, pDatFrm, size, BLOCKING);
		while (!(UARTx->LSR & UART_LSR_TEMT));
	}
	return cnt;
}

/*********************************************************************//**
 * @brief		Send Slave address frames on RS485 bus.
 * @param[in]	UARTx	LPC_UART1 (only)
 * @param[in]	SlvAddr Slave Address.
 * @return		None
 **********************************************************************/
void UART_RS485SendSlvAddr(LPC_UART1_TypeDef *UARTx, uint8_t SlvAddr)
{
	UART_RS485Send(UARTx, &SlvAddr, 1, 1);
}

/*********************************************************************//**
 * @brief		Send Data frames on RS485 bus.
 * @param[in]	UARTx	LPC_UART1 (only)
 * @param[in]	pData Pointer to data to be sent.
 * @param[in]	size Size of data frame to be sent.
 * @return		None
 **********************************************************************/
uint32_t UART_RS485SendData(LPC_UART1_TypeDef *UARTx, uint8_t *pData, uint32_t size)
{
	return (UART_RS485Send(UARTx, pData, size, 0));
}


/* UART INTERRUPT functions ---------------------------------------------------------*/

#ifdef INTERRUPT_MODE

/********************************************************************//**
 * @brief 		UART receive function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void UART_IntReceive(LPC_UART_TypeDef *UARTx)
{
	uint8_t tmpc;

	while(1)
	{
		// Call UART read function in UART driver
		tmpc = UART_ReceiveByte(UARTx);
		if (tmpc)
		{
			/* Check if buffer is more space
			 * If no more space, remaining character will be trimmed out
			 */
			if (!__BUF_IS_FULL(rb.rx_head,rb.rx_tail))
			{
				rb.rx[rb.rx_head] = tmpc;
				__BUF_INCR(rb.rx_head);
			}
		}
		// no more data
		else
		{
			break;
		}
	}
}

/********************************************************************//**
 * @brief 		UART transmit function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void UART_IntTransmit(LPC_UART_TypeDef *UARTx)
{
	uint32_t bToSend, bSent, timeOut, fifo_cnt;
	uint8_t *pChar;

    // Disable THRE interrupt
    UART_IntConfig(UARTx, UART_INTCFG_THRE, DISABLE);

	/* Wait for FIFO buffer empty, transfer UART_TX_FIFO_SIZE bytes
	 * of data or break whenever ring buffers are empty */
	/* Wait until THR empty */
    while (UART_CheckBusy(UARTx) == SET);

	while (!__BUF_IS_EMPTY(rb.tx_head,rb.tx_tail))
    {
        /* Move a piece of data into the transmit FIFO */
		// None blocking mode
		pChar = &rb.tx[rb.tx_tail];
		bToSend = 1;
		bSent = 0;
		while (bToSend)
		{
			timeOut = UART_BLOCKING_TIMEOUT;
			// Wait for THR empty with timeout
			while (!(UARTx->LSR & UART_LSR_THRE))
			{
				if (timeOut == 0) break;
				timeOut--;
			}
			// Time out!
			if(timeOut == 0) break;
			fifo_cnt = UART_TX_FIFO_SIZE;
			while (fifo_cnt && bToSend)
			{
				UART_SendByte(UARTx, (*pChar++));
				fifo_cnt--;
				bToSend--;
				bSent++;
			}
		}

    	if(bSent)
    	{
    		/* Update transmit ring FIFO tail pointer */
    		__BUF_INCR(rb.tx_tail);
     	}
    	else
    	{
    		break;
    	}
    }

    /* If there is no more data to send, disable the transmit
       interrupt - else enable it or keep it enabled */
	if (__BUF_IS_EMPTY(rb.tx_head, rb.tx_tail))
	{
    	UART_IntConfig(UARTx, UART_INTCFG_THRE, DISABLE);
    	// Reset Tx Interrupt state
    	TxIntStat = RESET;
    }
    else
    {
      	// Set Tx Interrupt state
		TxIntStat = SET;
    	UART_IntConfig(UARTx, UART_INTCFG_THRE, ENABLE);
    }
}


/*********************************************************************//**
 * @brief		Send a block of data via UART peripheral
 * @param[in]	UARTx	Selected UART peripheral used to send data, should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @param[in]	txbuf 	Pointer to Transmit buffer
 * @param[in]	buflen 	Length of Transmit buffer
 * @param[in] 	flag 	Flag used in  UART transfer, should be
 * 						NONE_BLOCKING or BLOCKING
 * @return 		Number of bytes sent.
 *
 * Note: when using UART in BLOCKING mode, a time-out condition is used
 * via defined symbol UART_BLOCKING_TIMEOUT.
 **********************************************************************/
uint32_t UART_Send(LPC_UART_TypeDef *UARTx, uint8_t *txbuf, uint32_t buflen, TRANSFER_BLOCK_Type flag)
{
	uint8_t *data = (uint8_t *) txbuf;
	uint32_t bytes = 0;

	/* Temporarily lock out UART transmit interrupts during this
	   read so the UART transmit interrupt won't cause problems
	   with the index values */
	UART_IntConfig(UARTx, UART_INTCFG_THRE, DISABLE);

	/* Loop until transmit run buffer is full or until n_bytes
	   expires */
	while ((buflen > 0) && (!__BUF_IS_FULL(rb.tx_head, rb.tx_tail)))
	{
		/* Write data from buffer into ring buffer */
		rb.tx[rb.tx_head] = *data;
		data++;

		/* Increment head pointer */
		__BUF_INCR(rb.tx_head);

		/* Increment data count and decrement buffer size count */
		bytes++;
		buflen--;
	}

	/**
	 * Check if current Tx interrupt enable is reset,
	 * that means the Tx interrupt must be re-enabled
	 * due to call UART_IntTransmit() function to trigger
	 * this interrupt type
	 */
	if (TxIntStat == RESET)
	{
		UART_IntTransmit(UARTx);
	}
	/**
	 * Otherwise, re-enables Tx Interrupt
	 */
	else
	{
		UART_IntConfig(UARTx, UART_INTCFG_THRE, ENABLE);
	}

	return bytes;
}

/*********************************************************************//**
 * @brief		Receive a block of data via UART peripheral
 * @param[in]	UARTx	Selected UART peripheral used to send data,
 * 				should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @param[out]	rxbuf 	Pointer to Received buffer
 * @param[in]	buflen 	Length of Received buffer
 * @param[in] 	flag 	Flag mode, should be NONE_BLOCKING or BLOCKING

 * @return 		Number of bytes received
 *
 * Note: when using UART in BLOCKING mode, a time-out condition is used
 * via defined symbol UART_BLOCKING_TIMEOUT.
 **********************************************************************/
uint32_t UART_Receive(LPC_UART_TypeDef *UARTx, uint8_t *rxbuf, uint32_t buflen, TRANSFER_BLOCK_Type flag)
{
	uint8_t *data = (uint8_t *) rxbuf;
	uint32_t bytes = 0;

	/* Temporarily lock out UART receive interrupts during this
	   read so the UART receive interrupt won't cause problems
	   with the index values */
	UART_IntConfig(UARTx, UART_INTCFG_RBR, DISABLE);

	/* Loop until receive buffer ring is empty or
		until max_bytes expires */
	while ((buflen > 0) && (!(__BUF_IS_EMPTY(rb.rx_head, rb.rx_tail))))
	{
		/* Read data from ring buffer into user buffer */
		*data = rb.rx[rb.rx_tail];
		data++;

		/* Update tail pointer */
		__BUF_INCR(rb.rx_tail);

		/* Increment data count and decrement buffer size count */
		bytes++;
		buflen--;
	}

	/* Re-enable UART interrupts */
	UART_IntConfig(UARTx, UART_INTCFG_RBR, ENABLE);

    return bytes;
}
#endif


/********************************************************************//**
 * @brief		VT100- code to set cursor to Home
 * @param[in]	UARTx	Selected UART peripheral used to send data,
 * 				should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @return 		None
 *********************************************************************/
void reset_cursor (LPC_UART_TypeDef *UARTx)
{
	printf(UARTx,"\x1b[H");   /* escape sequence for vt220 ^[H sets cursor to Home */
}


/********************************************************************//**
 * @brief		VT100- code to Clear Screen
 * @param[in]	UARTx	Selected UART peripheral used to send data,
 * 				should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @return 		None
 *********************************************************************/
void clear_screen (LPC_UART_TypeDef *UARTx)
{
	printf(UARTx,"\x1b[2J");   /* escape sequence for vt220 ESC[2J clears screen */
}


/********************************************************************//**
 * @brief		VT100- code to Clear Screen and Reset Cursor
 * @param[in]	UARTx	Selected UART peripheral used to send data,
 * 				should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @return 		None
 *********************************************************************/
void clr_scr_rst_cur (LPC_UART_TypeDef *UARTx)
{
	clear_screen(UARTx);
	reset_cursor(UARTx);
}


/********************************************************************//**
 * @brief		Erase Character
 * @param[in]	UARTx	Selected UART peripheral used to send data,
 * 				should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @return 		None
 *********************************************************************/
void Erase_Char (LPC_UART_TypeDef *UARTx)
{
	printf(UARTx, "%c", Out_BACKSPACE);
	printf(UARTx, "%c", Out_SPACE);
	printf(UARTx, "%c", Out_BACKSPACE);
}


/********************************************************************//**
 * @brief		Erase Character with Underscore '_'
 * @param[in]	UARTx	Selected UART peripheral used to send data,
 * 				should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @return 		None
 *********************************************************************/
void Erase_Char_With_UnderScore (LPC_UART_TypeDef *UARTx)
{
	printf(UARTx, "%c", Out_BACKSPACE);       /* erase character on the screen */
	printf(UARTx, "_");                   /* and write '_' on the screen */
	printf(UARTx, "%c", Out_BACKSPACE);       /* erase character on the screen */
}


/********************************************************************//**
 * @brief		Erase BackLash
 * @param[in]	UARTx	Selected UART peripheral used to send data,
 * 				should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @return 		None
 *********************************************************************/
void Erase_BackLash (LPC_UART_TypeDef *UARTx)
{
	printf(UARTx, "%c", Out_BACKSPACE);       /* erase character on the screen */
	printf(UARTx, "/");                   /* and write '/' on the screen */
	printf(UARTx, "%c", Out_BACKSPACE);       /* erase character on the screen */
	printf(UARTx, "%c", Out_BACKSPACE);       /* erase character on the screen */
	printf(UARTx, "_");                   /* and write '_' on the screen */
	printf(UARTx, "%c", Out_BACKSPACE);       /* erase character on the screen */
}


/********************************************************************//**
 * @brief		Erase Semicolon
 * @param[in]	UARTx	Selected UART peripheral used to send data,
 * 				should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @return 		None
 *********************************************************************/
void Erase_SemiColon (LPC_UART_TypeDef *UARTx)
{
	printf(UARTx, "%c", Out_BACKSPACE);       /* erase character on the screen */
	printf(UARTx, ":");                   /* and write ':' on the screen */
	printf(UARTx, "%c", Out_BACKSPACE);       /* erase character on the screen */
	printf(UARTx, "%c", Out_BACKSPACE);       /* erase character on the screen */
	printf(UARTx, "_");                   /* and write '_' on the screen */
	printf(UARTx, "%c", Out_BACKSPACE);       /* erase character on the screen */
}


/********************************************************************//**
 * @brief		Erase and Ring the Bell
 * @param[in]	UARTx	Selected UART peripheral used to send data,
 * 				should be:
 *   			- LPC_UART0: UART0 peripheral
 * 				- LPC_UART1: UART1 peripheral
 * 				- LPC_UART2: UART2 peripheral
 * 				- LPC_UART3: UART3 peripheral
 * @return 		None
 *********************************************************************/
void Erase_And_RingTheBell (LPC_UART_TypeDef *UARTx)
{
	printf(UARTx, "%c", Out_BACKSPACE);         /* back space */
	printf(UARTx, "_");                   /* erase 1st char on the screen */
	printf(UARTx, "%c", Out_BACKSPACE);         /* back space */
	printf(UARTx, "_");                   /* erase 2nd char on the screen */
	printf(UARTx, "%c", Out_BACKSPACE);         /* back space */
	printf(UARTx, "\7");                  /* ring the bell */
}


/**
 * @}
 */


/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */

