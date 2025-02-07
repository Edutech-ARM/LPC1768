/**********************************************************************
* $Id$		lpc_ssp_glcd.h
*//**
* @file		lpc_ssp_glcd.h
* @brief	Contains all SSP based GLCD Initialization code
* @version	1.0
* @date		18. Dec. 2013
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
/** @defgroup GLCD
 * @ingroup LPC1700CMSIS_FwLib_Drivers
 * @{
 */

#ifndef LPC_SSP_GLCD_H_
#define LPC_SSP_GLCD_H_

/* Includes ------------------------------------------------------------------- */
#include "LPC17xx.h"
#include "lpc_system_init.h"
#include "lpc17xx_ssp.h"

#ifdef __cplusplus
extern "C"
{
#endif


/* Public Macros -------------------------------------------------------------- */
/** @defgroup GLCD_Public_Macros
 * @{
 */
#define LCD_RS 		(1<<0)  //port2
#define	LCD_RST		(1<<5)	//port0
#define LCD_BK		(1<<8)	//port2


/*------------------------------------------------------------------------------
  Color coding
  GLCD is coded:   15..11 red, 10..5 green, 4..0 blue  (unsigned short)  GLCD_R5, GLCD_G6, GLCD_B5
  original coding: 17..12 red, 11..6 green, 5..0 blue                    ORG_R6,  ORG_G6,  ORG_B6

  ORG_R1..5 = GLCD_R0..4,  ORG_R0 = GLCD_R4
  ORG_G0..5 = GLCD_G0..5,
  ORG_B1..5 = GLCD_B0..4,  ORG_B0 = GLCD_B4
 *----------------------------------------------------------------------------*/

/* GLCD RGB color definitions                                                 */
#define Black           0x0000		/*   0,   0,   0 */
#define Navy            0x000F      /*   0,   0, 128 */
#define DarkGreen       0x03E0      /*   0, 128,   0 */
#define DarkCyan        0x03EF      /*   0, 128, 128 */
#define Maroon          0x7800      /* 128,   0,   0 */
#define Purple          0x780F      /* 128,   0, 128 */
#define Olive           0x7BE0      /* 128, 128,   0 */
#define LightGrey       0xC618      /* 192, 192, 192 */
#define DarkGrey        0x7BEF      /* 128, 128, 128 */
#define Blue            0x001F      /*   0,   0, 255 */
#define Green           0x07E0      /*   0, 255,   0 */
#define Cyan            0x07FF      /*   0, 255, 255 */
#define Red             0xF800      /* 255,   0,   0 */
#define Magenta         0xF81F      /* 255,   0, 255 */
#define Yellow          0xFFE0      /* 255, 255, 0   */
#define White           0xFFFF      /* 255, 255, 255 */

/*---------------------- Graphic LCD size definitions ------------------------*/

#define WIDTH       320                 /* Screen Width (in pixels)           */
#define HEIGHT      240                 /* Screen Hight (in pixels)           */
#define BPP         16                  /* Bits per pixel                     */
#define BYPP        ((BPP+7)/8)         /* Bytes per pixel                    */

/**
 * @brief GLCD Driver Output Type definitions
 */
typedef enum {
	TOP_LEFT = 0,	/**< Polling transfer */
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT
} DRIVER_OUT_Type;


/* Coordinate Type */
typedef struct
{
	int16_t x;
	int16_t y;
}COORDINATE_Type;

/**
 * @}
 */


/* Public Functions ----------------------------------------------------------- */
/** @defgroup GLCD_Public_Functions GLCD Public Functions
 * @{
 */

void GLCD_Backlight (FunctionalState NewState);
void GLCD_Reset(void);
void GLCD_Driver_OutCtrl(DRIVER_OUT_Type drv);
void GLCD_Set_Loc (uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void GLCD_Display_Home (void);
void GLCD_Init (void);
void GLCD_Window (uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void GLCD_PutPixel (uint16_t x, uint16_t y, uint16_t color);
void GLCD_SetTextColor (uint16_t color);
void GLCD_SetBackColor (uint16_t color);
void GLCD_Clear (uint16_t color);
void GLCD_Draw_Char (uint16_t x, uint16_t y, uint16_t *c);
void GLCD_Display_Char (uint16_t ln, uint16_t col, uchar  c);
void GLCD_Display_String (uint16_t ln, uint16_t col, uchar *s);
void GLCD_ClearLn (uint16_t ln);
void GLCD_Bitmap (uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *bitmap);
void GLCD_Window_Fill (uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

uchar Write_Command_Glcd (uint8_t Command);
uchar Write_Data_Glcd (uint16_t data);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif /* LPC_SSP_GLCD_H_ */

/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */
