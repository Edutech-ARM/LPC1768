/**********************************************************************
* $Id$		lpc_ssp_glcd.c
*//**
* @file		lpc_ssp_glcd.c
* @brief	Contains all functions support for SSP based GLCD
*           library on LPC17xx
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
/** @addtogroup GLCD
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "lpc_ssp_glcd.h"
#include "math.h"
#include "Font_24x16.h"


/* If this source file built with example, the LPC17xx FW library configuration
 * file in each example directory ("lpc17xx_libcfg.h") must be included,
 * otherwise the default FW library configuration file must be included instead
 */

/******************************************************************************/
static volatile uint16_t TextColor = Black, BackColor = White;

// Swap two bytes
#define SWAP(x,y) do { (x)=(x)^(y); (y)=(x)^(y); (x)=(x)^(y); } while(0)

/** @addtogroup GLCD_Public_Functions
 * @{
 */

/* Public Functions ----------------------------------------------------------- */

/*********************************************************************//**
 * @brief	    This function controls Backlight
 * @param[in]	NewState	ENABLE/DISABLE the Backlight
 * @return 		None
 **********************************************************************/
void GLCD_Backlight (FunctionalState NewState)
{
	if(NewState)
	{
		GPIO_SetValue(2, LCD_BK);
	}
	else
	{
		GPIO_ClearValue(2, LCD_BK);
	}
}


/*********************************************************************//**
 * @brief	    This function resets GLCD
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void GLCD_Reset (void)
{
	GPIO_SetValue(0, LCD_RST);
	delay_ms(2);
	GPIO_ClearValue(0, LCD_RST);  //reset low
	delay_ms(4);
	GPIO_SetValue(0, LCD_RST);  //reset low
}


/*********************************************************************//**
 * @brief	    This function controls GLCD Output
 * @param[in]	drv	    Output Format
 * 						- TOP_LEFT
 * 						- TOP_RIGHT
 * 						- BOTTOM_LEFT
 * 						- BOTTOM_RIGHT
 * @return 		None
 **********************************************************************/
void GLCD_Driver_OutCtrl (DRIVER_OUT_Type drv)
{
	Write_Command_Glcd(0x01);    // Driver Output Control

	switch (drv)
	{
	case TOP_LEFT:
		Write_Data_Glcd(0x72EF);       // Page 36-39 of SSD2119 datasheet
		break;

	case TOP_RIGHT:
		Write_Data_Glcd(0x70EF);       // Page 36-39 of SSD2119 datasheet
		break;

	case BOTTOM_LEFT:
		Write_Data_Glcd(0x32EF);       // Page 36-39 of SSD2119 datasheet
		break;

	case BOTTOM_RIGHT:
		Write_Data_Glcd(0x30EF);       // Page 36-39 of SSD2119 datasheet
		break;

	default:
		break;
	}
}


/*********************************************************************//**
 * @brief	    Set draw window region to required width and height
 *              as well as location
 * @param[in]	x        horizontal position
 *              y        vertical position
 *              w        width of bitmap
 *              h        height of bitmap
 * @return 		None
 **********************************************************************/
void GLCD_Window (uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	Write_Command_Glcd(0x45);      /* Horizontal GRAM Start Address      */
	Write_Data_Glcd(x);

	Write_Command_Glcd(0x46);      /* Horizontal GRAM End   Address (-1) */
	Write_Data_Glcd(x+w-1);

	Write_Command_Glcd(0x44);      /* Vertical   GRAM Start Address      */
	Write_Data_Glcd(y);

	Write_Command_Glcd(0x44);      /* Vertical   GRAM End   Address (-1) */
	Write_Data_Glcd((y+h-1)<<8);
}


/*********************************************************************//**
 * @brief	    This function Sets Cursor to to desired location
 * @param[in]	x        horizontal position
 *              y        vertical position
 *              w        width of bitmap
 *              h        height of bitmap
 * @return 		None
 **********************************************************************/
void GLCD_Set_Loc (uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	GLCD_Window (x,y,w,h);

	Write_Command_Glcd(0x4E);    // GDDRAM Horizontal
	Write_Data_Glcd(x);       // Page 58 of SSD2119 datasheet

	Write_Command_Glcd(0x4F);    // GDDRAM Vertical
	Write_Data_Glcd(y);       // Page 58 of SSD2119 datasheet

	Write_Command_Glcd(0x22);    // RAM data write/read
}


/*********************************************************************//**
 * @brief	    This function Sets Cursor to Home
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void GLCD_Display_Home (void)
{
	GLCD_Set_Loc (0,0,320,240);
}


/*********************************************************************//**
 * @brief	    This function Initializes GLCD
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void GLCD_Init (void)
{
	GPIO_SetDir(2, LCD_RS, 1);   // RS as output
	GPIO_SetDir(0, LCD_RST, 1);  // Reset as Output
	GPIO_SetDir(2, LCD_BK, 1);   // Backlight as output

	delay_ms(2);
	GLCD_Reset();                // Reset GLCD
	GLCD_Backlight(ENABLE);
	delay_ms(2);

	Write_Command_Glcd(0x28);    // VCOM OTP
	Write_Data_Glcd(0x0006);     // Page 55-56 of SSD2119 datasheet

	Write_Command_Glcd(0x00);    // start Oscillator
	Write_Data_Glcd(0x0001);     // Page 36 of SSD2119 datasheet

	Write_Command_Glcd(0x10);    // Sleep mode
	Write_Data_Glcd(0x0000);     // Page 49 of SSD2119 datasheet

	GLCD_Driver_OutCtrl (TOP_LEFT);

	Write_Command_Glcd(0x02);    // LCD Driving Waveform Control
	Write_Data_Glcd(0x0600);     // Page 40-42 of SSD2119 datasheet

	Write_Command_Glcd(0x03);    // Power Control 1
	Write_Data_Glcd(0x6A38);     // Page 43-44 of SSD2119 datasheet 6A38

	Write_Command_Glcd(0x11);    // Entry Mode
	Write_Data_Glcd(0x6870);     // Page 50-52 of SSD2119 datasheet

	Write_Command_Glcd(0x0F);    // Gate Scan Position
	Write_Data_Glcd(0x0000);     // Page 49 of SSD2119 datasheet

	Write_Command_Glcd(0x0B);    // Frame Cycle Control
	Write_Data_Glcd(0x5308);     // Page 45 of SSD2119 datasheet

	Write_Command_Glcd(0x0C);    // Power Control 2
	Write_Data_Glcd(0x0003);     // Page 47 of SSD2119 datasheet

	Write_Command_Glcd(0x0D);    // Power Control 3
	Write_Data_Glcd(0x000A);     // Page 48 of SSD2119 datasheet

	Write_Command_Glcd(0x0E);    // Power Control 4
	Write_Data_Glcd(0x2E00);     // Page 48 of SSD2119 datasheet

	Write_Command_Glcd(0x1E);    // Power Control 5
	Write_Data_Glcd(0x00BE);     // Page 53 of SSD2119 datasheet

	Write_Command_Glcd(0x25);    // Frame Frequency Control
	Write_Data_Glcd(0x8000);     // Page 53 of SSD2119 datasheet  8000

	Write_Command_Glcd(0x26);    // Analog setting
	Write_Data_Glcd(0x7800);     // Page 54 of SSD2119 datasheet

	Write_Command_Glcd(0x4E);    // Ram Address Set
	Write_Data_Glcd(0x0000);     // Page 58 of SSD2119 datasheet

	Write_Command_Glcd(0x4F);    // Ram Address Set
	Write_Data_Glcd(0x0000);     // Page 58 of SSD2119 datasheet

	Write_Command_Glcd(0x12);    // Sleep mode
	Write_Data_Glcd(0x08D9);     // Page 49 of SSD2119 datasheet

	// Gamma Control (R30h to R3Bh) -- Page 56 of SSD2119 datasheet
	Write_Command_Glcd(0x30);
	Write_Data_Glcd(0x0000);

	Write_Command_Glcd(0x31);
	Write_Data_Glcd(0x0104);

	Write_Command_Glcd(0x32);
	Write_Data_Glcd(0x0100);

	Write_Command_Glcd(0x33);
	Write_Data_Glcd(0x0305);

	Write_Command_Glcd(0x34);
	Write_Data_Glcd(0x0505);

	Write_Command_Glcd(0x35);
	Write_Data_Glcd(0x0305);

	Write_Command_Glcd(0x36);
	Write_Data_Glcd(0x0707);

	Write_Command_Glcd(0x37);
	Write_Data_Glcd(0x0300);

	Write_Command_Glcd(0x3A);
	Write_Data_Glcd(0x1200);

	Write_Command_Glcd(0x3B);
	Write_Data_Glcd(0x0800);

	Write_Command_Glcd(0x07);      // Display Control
	Write_Data_Glcd(0x0033);       // Page 45 of SSD2119 datasheet

	delay_ms(5);

	Write_Command_Glcd(0x22);    // RAM data write/read
}


/*********************************************************************//**
 * @brief	    Draw a pixel in foreground color
 * @param[in]	x        horizontal position
 *              y        vertical position
 * @return 		None
 **********************************************************************/
void GLCD_PutPixel (uint16_t x, uint16_t y, uint16_t color)
{
	Write_Command_Glcd(0x4E);     /* GDDRAM Horizontal */
	Write_Data_Glcd(x);

	Write_Command_Glcd(0x4F);     /* GDDRAM Vertical */
	Write_Data_Glcd(y);

	Write_Command_Glcd(0x22);      /* RAM data write     */
	Write_Data_Glcd(color);
}


/*********************************************************************//**
 * @brief	    Set foreground color
 * @param[in]	color    foreground color
 * @return 		None
 **********************************************************************/
void GLCD_SetTextColor (uint16_t color)
{
	TextColor = color;
}


/*********************************************************************//**
 * @brief	    Set background color
 * @param[in]	color    background color
 * @return 		None
 **********************************************************************/
void GLCD_SetBackColor (uint16_t color)
{
	BackColor = color;
}


/*********************************************************************//**
 * @brief	    Start of data writing to LCD controller
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static __INLINE void wr_dat_start (void)
{
	CS_Force1 (LPC_SSP1, DISABLE);
	GPIO_SetValue(2, LCD_RS);  // select data mode
}


/*********************************************************************//**
 * @brief	    Stop of data writing to LCD controller
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static __INLINE void wr_dat_stop (void)
{
	CS_Force1 (LPC_SSP1, ENABLE);
}


/*********************************************************************//**
 * @brief	    Data writing to LCD controller
 * @param[in]	c     data to be written
 * @return 		None
 **********************************************************************/
static __INLINE void wr_dat_only (uint16_t c)
{
	SSP_DATA_SETUP_Type xferConfig;

	Tx_Buf1[0] = (uchar)(c>>8);    // 1st byte extract
	Tx_Buf1[1] = (uchar) c;        // 2nd byte extract

	xferConfig.tx_data = Tx_Buf1;               /* Send Instruction Byte    */
	xferConfig.rx_data = NULL;
	xferConfig.length = 2;
	SSP_ReadWrite(LPC_SSP1, &xferConfig, SSP_TRANSFER_POLLING);
}


/*********************************************************************//**
 * @brief	    Clear display
 * @param[in]	color    display clearing color
 * @return 		None
 **********************************************************************/
void GLCD_Clear (uint16_t color)
{
	unsigned int   i;

	GLCD_Window (0,0,320,240);    // Window Max

	Write_Command_Glcd(0x4E);     /* GDDRAM Horizontal */
	Write_Data_Glcd(0);

	Write_Command_Glcd(0x4F);     /* GDDRAM Vertical */
	Write_Data_Glcd(0);

	Write_Command_Glcd(0x22);
	wr_dat_start();
	for(i = 0; i < (WIDTH*HEIGHT); i++)
		wr_dat_only(color);
	wr_dat_stop();
}


/*********************************************************************//**
 * @brief	    Draw character on given position
 * @param[in]	x       horizontal position
 *              y       vertical position
 *              c       pointer to character bitmap
 * @return 		None
 **********************************************************************/
void GLCD_Draw_Char (uint16_t x, uint16_t y, uint16_t *c)
{
	int idx = 0, i, j;

	x = x-CHAR_W;

	Write_Command_Glcd(0x45);      /* Horizontal GRAM Start Address      */
	Write_Data_Glcd(x);

	Write_Command_Glcd(0x46);      /* Horizontal GRAM End   Address (-1) */
	Write_Data_Glcd(x+CHAR_W-1);

	Write_Command_Glcd(0x44);      /* Vertical   GRAM Start Address      */
	Write_Data_Glcd(y);

	Write_Command_Glcd(0x44);      /* Vertical   GRAM End   Address (-1) */
	Write_Data_Glcd((y+CHAR_H-1)<<8);

	Write_Command_Glcd(0x4E);     /* GDDRAM Horizontal */
	Write_Data_Glcd(x);

	Write_Command_Glcd(0x4F);     /* GDDRAM Vertical */
	Write_Data_Glcd(y);

	Write_Command_Glcd(0x22);

	wr_dat_start();
	for (j = 0; j < CHAR_H; j++)
	{
		for (i = 0; i<CHAR_W; i++)
		{
			if((c[idx] & (1 << i)) == 0x00)
			{
				wr_dat_only(BackColor);
			}
			else
			{
				wr_dat_only(TextColor);
			}
		}
		c++;
	}
	wr_dat_stop();
}


/*********************************************************************//**
 * @brief	    Disply character on given line
 * @param[in]	ln       line number
 *              col      column number
 *              c        ascii character
 * @return 		None
 **********************************************************************/
void GLCD_Display_Char (uint16_t ln, uint16_t col, uchar c)
{
	c -= 32;
	GLCD_Draw_Char(col * CHAR_W, ln * CHAR_H, (uint16_t *)&Font_24x16[c * CHAR_H]);
}


/*********************************************************************//**
 * @brief	    Disply string on given line
 * @param[in]	ln       line number
 *              col      column number
 *              s        pointer to string
 * @return 		None
 **********************************************************************/
void GLCD_Display_String (uint16_t ln, uint16_t col, uchar *s)
{
	GLCD_Window(0,0,320,240);  // Window Max
	while (*s)
	{
		GLCD_Display_Char(ln, col++, *s++);
	}
}


/*********************************************************************//**
 * @brief	    Clear given line
 * @param[in]	ln       line number
 * @return 		None
 **********************************************************************/
void GLCD_ClearLn (uint16_t ln)
{
	GLCD_Window(0,0,320,240);  // Window Max
	GLCD_Display_String(ln, 0, "                    ");
}


/*********************************************************************//**
 * @brief	    Draw bargraph
 * @param[in]	x        horizontal position
 *              y        vertical position
 *              w        maximum width of bargraph (in pixels)
 *              val      value of active bargraph (in 1/1024)
 * @return 		None
 **********************************************************************/
void GLCD_Bargraph (uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t val)
{
	int i,j;

	Write_Command_Glcd(0x45);      /* Horizontal GRAM Start Address      */
	Write_Data_Glcd(x);

	Write_Command_Glcd(0x46);      /* Horizontal GRAM End   Address (-1) */
	Write_Data_Glcd(x+w-1);

	Write_Command_Glcd(0x44);      /* Vertical   GRAM Start Address      */
	Write_Data_Glcd(y);

	Write_Command_Glcd(0x44);      /* Vertical   GRAM End   Address (-1) */
	Write_Data_Glcd((y+CHAR_H-1)<<8);

	Write_Command_Glcd(0x4E);     /* GDDRAM Horizontal */
	Write_Data_Glcd(x);

	Write_Command_Glcd(0x4F);     /* GDDRAM Vertical */
	Write_Data_Glcd(y);

	Write_Command_Glcd(0x22);

	val = (val * w) >> 10;                /* Scale value for 24x12 characters   */

	wr_dat_start();
	for (i = h; i >0; i--)
	{
		for (j = 0; j <= w-1; j++)
		{
			if(j <= val)
			{
				wr_dat_only(BackColor);
			}
			else
			{
				wr_dat_only(TextColor);
			}
		}
	}
	wr_dat_stop();
}


/*********************************************************************//**
 * @brief	    Display graphical bitmap image at position x horizontally
 *              and y vertically (This function is optimized for
 *              16 bits per pixel format, it has to be adapted for
 *              any other bits per pixel format)
 * @param[in]	x        horizontal position
 *              y        vertical position
 *              w        width of bitmap
 *              h        height of bitmap
 *              bitmap   address at which the bitmap data resides
 * @return 		None
 **********************************************************************/
void GLCD_Bitmap (uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
	uint32_t i,j,k;

	GLCD_Set_Loc (x,y,w,h);

	wr_dat_start();
	k = 16;
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			wr_dat_only(bitmap[k++]);
		}
	}
	wr_dat_stop();
}


/*********************************************************************//**
 * @brief	    F at position x horizontally
 *              and y vertically (This function is optimized for
 *              16 bits per pixel format, it has to be adapted for
 *              any other bits per pixel format)
 * @param[in]	x        horizontal position
 *              y        vertical position
 *              w        width of bitmap
 *              h        height of bitmap
 *              color    window color
 * @return 		None
 **********************************************************************/
void GLCD_Window_Fill (uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	uint32_t i,j;

	GLCD_Set_Loc (x,y,w,h);

	wr_dat_start();
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			wr_dat_only(color);
		}
	}
	wr_dat_stop();
}


/*********************************************************************//**
 * @brief	    Draw a line on a graphic LCD using Bresenham's
 *              line drawing algorithm
 * @param[in]	(x1, y1)   the start coordinate
 *              (x2, y2)   the end coordinate
 *              color      line color
 * @return 		None
 **********************************************************************/
void GLCD_Line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
	int16_t  x, y, addx, addy, dx, dy;
	int32_t P,i;

	dx = abs((int16_t)(x2 - x1));
	dy = abs((int16_t)(y2 - y1));
	x = x1;
	y = y1;

	if(x1 > x2)
		addx = -1;
	else
		addx = 1;
	if(y1 > y2)
		addy = -1;
	else
		addy = 1;


	if(dx >= dy)
	{
		P = 2*dy - dx;

		for(i=0; i<=dx; ++i)
		{
			GLCD_PutPixel(x, y, color);

			if(P < 0)
			{
				P += 2*dy;
				x += addx;
			}
			else
			{
				P += 2*dy - 2*dx;
				x += addx;
				y += addy;
			}
		}
	}
	else
	{
		P = 2*dx - dy;

		for(i=0; i<=dy; ++i)
		{
			GLCD_PutPixel(x, y, color);

			if(P < 0)
			{
				P += 2*dx;
				y += addy;
			}
			else
			{
				P += 2*dx - 2*dy;
				x += addx;
				y += addy;
			}
		}
	}
}

/*********************************************************************//**
 * @brief	    Draw a Rectangle on a graphic LCD
 * @param[in]	(x1, y1)     the start coordinate
 *              (x2, y2)     the end coordinate
 *              fill         Fill Rectangle TRUE/FALSE or ON/OFF
 *              color        Boundary color
 *              fill_color   fill color
 * @return 		None
 **********************************************************************/
void GLCD_Rect(COORDINATE_Type *p1, COORDINATE_Type *p2, Bool fill, uint16_t color, uint16_t fill_color)
{
	int16_t  width,height;                          // Find the y min and max

	if(fill)
	{
		if(p2->x > p1->x)
		{
			if(p2->y > p1->y)
			{
				width = p2->x - p1->x;
				height = p2->y - p1->y;
				GLCD_Window_Fill(p1->x+1,p1->y+1,width-1,height-1,fill_color);
			}
			else
			{
				width = p2->x - p1->x;
				height = p1->y - p2->y;
				GLCD_Window_Fill(p1->x+1,p2->y+1,width-1,height-1,fill_color);
			}
		}
		else
		{
			if(p2->y > p1->y)
			{
				width = p1->x - p2->x;
				height = p2->y - p1->y;
				GLCD_Window_Fill(p2->x+1,p1->y+1,width-1,height-1,fill_color);
			}
			else
			{
				width = p1->x - p2->x;
				height = p1->y - p2->y;
				GLCD_Window_Fill(p2->x+1,p2->y+1,width-1,height-1,fill_color);
			}
		}
		fill = NO;
	}
	if(!fill)
	{
		GLCD_Line(p1->x, p1->y, p2->x, p1->y, color);      // Draw the outer border 4 sides
		GLCD_Line(p1->x, p2->y, p2->x, p2->y, color);
		GLCD_Line(p1->x, p1->y, p1->x, p2->y, color);
		GLCD_Line(p2->x, p1->y, p2->x, p2->y, color);
	}
}


/*********************************************************************//**
 * @brief	    Draw a frame on a graphic LCD
 * @param[in]	(x1, y1)     the start coordinate
 *              (x2, y2)     the end coordinate
 *              frame_width  Total Width of Frame
 *              color        Boundary color
 *              fill_color   Frame fill color
 * @return 		None
 **********************************************************************/
void GLCD_Frame(COORDINATE_Type *p1, COORDINATE_Type *p2, int16_t frame_width, uint16_t color, uint16_t fill_color)
{
	int16_t fw;                          // Find the y min and max

	fw = frame_width;

	while(fw)
	{
		if(fw < frame_width)
		{
			GLCD_Line(p1->x, p1->y+fw, p2->x, p1->y+fw, fill_color);      // Draw the interior 4 sides
			GLCD_Line(p1->x, p2->y-fw, p2->x, p2->y-fw, fill_color);
			GLCD_Line(p1->x+fw, p1->y, p1->x+fw, p2->y, fill_color);
			GLCD_Line(p2->x-fw, p1->y, p2->x-fw, p2->y, fill_color);
			fw--;
		}
		else
		{
			GLCD_Line(p1->x, p1->y+fw, p2->x, p1->y+fw, color);      // inner border 4 sides
			GLCD_Line(p1->x, p2->y-fw, p2->x, p2->y-fw, color);
			GLCD_Line(p1->x+fw, p1->y, p1->x+fw, p2->y, color);
			GLCD_Line(p2->x-fw, p1->y, p2->x-fw, p2->y, color);
			fw--;
		}
	}

	if(!fw)
	{
		GLCD_Line(p1->x, p1->y, p2->x, p1->y, color);      // Draw the outer border 4 sides
		GLCD_Line(p1->x, p2->y, p2->x, p2->y, color);
		GLCD_Line(p1->x, p1->y, p1->x, p2->y, color);
		GLCD_Line(p2->x, p1->y, p2->x, p2->y, color);
	}
}


/*********************************************************************//**
 * @brief	    Draw a rectangle/frame on a graphic LCD
 * @param[in]	(x1, y1)     the start coordinate
 *              (x2, y2)     the end coordinate
 *              (x3, y3)     the end coordinate
 *              fill         Fill Triangle TRUE/FALSE or ON/OFF
 *              color        Boundary color
 *              fill_color   Triangle Fill Color
 * @return 		None
 **********************************************************************/
void GLCD_Triangle(COORDINATE_Type *p1, COORDINATE_Type *p2, COORDINATE_Type *p3, Bool fill,uint16_t color, uint16_t fill_color)
{
    if(fill)
    {
    	uint16_t t1x,t2x,y,minx,maxx,t1xp,t2xp;
    	Bool changed1 = FALSE;
    	Bool changed2 = FALSE;
    	int16_t signx1,signx2,dx1,dy1,dx2,dy2;
    	uint16_t e1,e2;
        // Sort vertices
    	if (p1->y > p2->y) { SWAP(p1->y,p2->y); SWAP(p1->x,p2->x); }
    	if (p1->y > p3->y) { SWAP(p1->y,p3->y); SWAP(p1->x,p3->x); }
    	if (p2->y > p3->y) { SWAP(p2->y,p3->y); SWAP(p2->x,p3->x); }

    	t1x=t2x=p1->x; y=p1->y;   // Starting points

    	dx1 = (int16_t)(p2->x - p1->x);
    	if(dx1<0) { dx1=-dx1; signx1=-1; } else signx1=1;
    	dy1 = (int16_t)(p2->y - p1->y);

    	dx2 = (int8_t)(p3->x - p1->x);
    	if(dx2<0) { dx2=-dx2; signx2=-1; } else signx2=1;
    	dy2 = (int8_t)(p3->y - p1->y);

    	if (dy1 > dx1)
    	{   // swap values
            SWAP(dx1,dy1);
    		changed1 = TRUE;
    	}
    	if (dy2 > dx2)
    	{   // swap values
            SWAP(dy2,dx2);
    		changed2 = TRUE;
    	}

    	e2 = (uint16_t)(dx2>>1);
        // Flat top, just process the second half
        if(p1->y==p2->y) goto next;
        e1 = (uint16_t)(dx1>>1);

    	for (uint8_t i = 0; i < dx1;)
    	{
    		t1xp=0; t2xp=0;
    		if(t1x<t2x) { minx=t1x; maxx=t2x; }
    		else		{ minx=t2x; maxx=t1x; }
            // process first line until y value is about to change
    		while(i<dx1)
    		{
    			i++;
    			e1 += dy1;
    	   	   	while (e1 >= dx1)
    	   	   	{
    				e1 -= dx1;
       	   	   	   if (changed1) t1xp=signx1;//t1x += signx1;
    				else          goto next1;
    			}
    			if (changed1) break;
    			else t1x += signx1;
    		}
    	// Move line
    	next1:
            // process second line until y value is about to change
    		while (1)
    		{
    			e2 += dy2;
    			while (e2 >= dx2)
    			{
    				e2 -= dx2;
    				if (changed2) t2xp=signx2;//t2x += signx2;
    				else          goto next2;
    			}
    			if (changed2)     break;
    			else              t2x += signx2;
    		}
    	next2:
    		if(minx>t1x) minx=t1x; if(minx>t2x) minx=t2x;
    		if(maxx<t1x) maxx=t1x; if(maxx<t2x) maxx=t2x;
    		GLCD_Line(minx,y,maxx,y,fill_color);    // Draw line from min to max points found on the y

    		// Now increase y
    		if(!changed1) t1x += signx1;
    		t1x+=t1xp;
    		if(!changed2) t2x += signx2;
    		t2x+=t2xp;
        	y += 1;
    		if(y==p2->y) break;
       }
    	next:
    	// Second half
    	dx1 = (int8_t)(p3->x - p2->x); if(dx1<0) { dx1=-dx1; signx1=-1; } else signx1=1;
    	dy1 = (int8_t)(p3->y - p2->y);
    	t1x=p2->x;

    	if (dy1 > dx1)
    	{   // swap values
            SWAP(dy1,dx1);
    		changed1 = TRUE;
    	} else changed1=FALSE;

    	e1 = (uint8_t)(dx1>>1);

    	for (uint8_t i = 0; i<=dx1; i++)
    	{
    		t1xp=0; t2xp=0;
    		if(t1x<t2x) { minx=t1x; maxx=t2x; }
    		else		{ minx=t2x; maxx=t1x; }
    	    // process first line until y value is about to change
    		while(i<dx1)
    		{
        		e1 += dy1;
    	   	   	while (e1 >= dx1)
    	   	   	{
    				e1 -= dx1;
       	   	   	   	if (changed1) { t1xp=signx1; break; }//t1x += signx1;
    				else          goto next3;
    			}
    			if (changed1) break;
    			else   	   	  t1x += signx1;
    			if(i<dx1) i++;
    		}
    	next3:
            // process second line until y value is about to change
    		while (t2x!=p3->x)
    		{
    			e2 += dy2;
    	   	   	while (e2 >= dx2)
    	   	   	{
    				e2 -= dx2;
    				if(changed2) t2xp=signx2;
    				else          goto next4;
    			}
    			if (changed2)     break;
    			else              t2x += signx2;
    		}
    	next4:

    		if(minx>t1x) minx=t1x; if(minx>t2x) minx=t2x;
    		if(maxx<t1x) maxx=t1x; if(maxx<t2x) maxx=t2x;
    		GLCD_Line(minx,y,maxx,y,fill_color);    // Draw line from min to max points found on the y
    		// Now increase y
    		if(!changed1) t1x += signx1;
    		t1x+=t1xp;
    		if(!changed2) t2x += signx2;
    		t2x+=t2xp;
        	y += 1;
    		if(y>p3->y) break;
    	}
    	fill = NO;
    }


	if(!fill)
	{
		GLCD_Line(p1->x, p1->y, p2->x, p2->y, color);
		GLCD_Line(p1->x, p1->y, p3->x, p3->y, color);
		GLCD_Line(p2->x, p2->y, p3->x, p3->y, color);
	}
}


/*********************************************************************//**
 * @brief	    Draw a Circle with given Center and Radius
 * @param[in]	(x, y)     the center of the circle
 *              radius     the radius of the circle
 *              fill       YES or NO
 *              color      Boundary color
 * @return 		None
 **********************************************************************/
void GLCD_Circle(int16_t x, int16_t y, int16_t radius, Bool fill, uint16_t color, uint16_t fill_color)
{
	int16_t a, b, P;
	a = 0;
	b = radius;
	P = 1 - radius;

	do
	{
		if(fill)
		{
			GLCD_Line(x-a, y+b, x+a, y+b, fill_color);
			GLCD_Line(x-a, y-b, x+a, y-b, fill_color);
			GLCD_Line(x-b, y+a, x+b, y+a, fill_color);
			GLCD_Line(x-b, y-a, x+b, y-a, fill_color);
		}

		if(P < 0)
			P+= 3 + 2*a++;
		else
			P+= 5 + 2*(a++ - b--);
	} while(a <= b);

	fill = NO;
	a = 0;
	b = radius;
	P = 1 - radius;
	do
	{
		if(!fill)
		{
			GLCD_PutPixel(a+x, b+y, color);
			GLCD_PutPixel(b+x, a+y, color);
			GLCD_PutPixel(x-a, b+y, color);
			GLCD_PutPixel(x-b, a+y, color);
			GLCD_PutPixel(b+x, y-a, color);
			GLCD_PutPixel(a+x, y-b, color);
			GLCD_PutPixel(x-a, y-b, color);
			GLCD_PutPixel(x-b, y-a, color);
		}

		if(P < 0)
			P+= 3 + 2*a++;
		else
			P+= 5 + 2*(a++ - b--);
	} while(a <= b);
}


/*********************************************************************//**
 * @brief	    This function writes commands to the GLCD
 * @param[in]	Command		command to be written on GLCD
 * @return 		status
 **********************************************************************/
uchar Write_Command_Glcd (uint8_t Command)
{
	SSP_DATA_SETUP_Type xferConfig;
	uint8_t WriteStatus =0;
	__IO uint32_t i;

	GPIO_ClearValue(2, LCD_RS);  //select command mode

	CS_Force1 (LPC_SSP1, DISABLE);                        /* Select device           */
	xferConfig.tx_data = &Command;               /* Send Instruction Byte    */
	xferConfig.rx_data = NULL;
	xferConfig.length = 1;
	WriteStatus = SSP_ReadWrite(LPC_SSP1, &xferConfig, SSP_TRANSFER_POLLING);

	if(WriteStatus)
	{
		CS_Force1 (LPC_SSP1, ENABLE);                          /* CS high inactive        */
		for(i=925; i>0; i--);
		GPIO_SetValue(2, LCD_RS);  // select data mode
		return(1);
	}
	else
		return(0);
}


/*********************************************************************//**
 * @brief	    This function writes data to the GLCD
 * @param[in]	data	data to be written on GLCD
 * @return 		None
 **********************************************************************/
uchar Write_Data_Glcd (uint16_t data)
{
	SSP_DATA_SETUP_Type xferConfig;
	uint8_t WriteStatus =0;

	Tx_Buf1[0] = (uchar)(data>>8);    // 1st byte extract
	Tx_Buf1[1] = (uchar) data;        // 2nd byte extract

	GPIO_SetValue(2, LCD_RS);  // select data mode

	CS_Force1 (LPC_SSP1, DISABLE);                        /* Select device           */
	xferConfig.tx_data = Tx_Buf1;               /* Send Instruction Byte    */
	xferConfig.rx_data = NULL;
	xferConfig.length = 2;
	WriteStatus = SSP_ReadWrite(LPC_SSP1, &xferConfig, SSP_TRANSFER_POLLING);

	if(WriteStatus)
	{
		CS_Force1 (LPC_SSP1, ENABLE);                          /* CS high inactive        */
		return(1);
	}
	else
		return(0);
}


/**
 * @}
 */

/* End of Public Functions ---------------------------------------------------- */

/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */

