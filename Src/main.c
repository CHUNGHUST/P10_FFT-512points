
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "adc.h"
#include "tim.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include <math.h>

#define PI      				 3.1415926535897
#define N_point     		 512
#define N_freq       		 N_point/2 // frequency N_freq = 0 is DC signal  
#define FP_PRECISION 		 8  
#define FP_MASK 			 	 ((1<<FP_PRECISION) - 1)
#define FP__MUL(A, B)    ((int16_t)(((int32_t)(A) * (int32_t)(B)) >> FP_PRECISION))

#define log2N 9


#define OE_DIS     HAL_GPIO_WritePin(OE_GPIO_Port,  OE_Pin,  GPIO_PIN_SET)
#define OE_EN      HAL_GPIO_WritePin(OE_GPIO_Port,  OE_Pin,  GPIO_PIN_RESET)
#define A_SET      HAL_GPIO_WritePin(A_GPIO_Port,   A_Pin,   GPIO_PIN_SET)
#define A_RESET    HAL_GPIO_WritePin(A_GPIO_Port,   A_Pin,   GPIO_PIN_RESET)
#define B_SET      HAL_GPIO_WritePin(B_GPIO_Port,   B_Pin,   GPIO_PIN_SET)
#define B_RESET    HAL_GPIO_WritePin(B_GPIO_Port,   B_Pin,   GPIO_PIN_RESET)
#define CLK_SET    HAL_GPIO_WritePin(CLK_GPIO_Port, CLK_Pin, GPIO_PIN_SET)
#define CLK_RESET  HAL_GPIO_WritePin(CLK_GPIO_Port, CLK_Pin, GPIO_PIN_RESET)
#define LAT_SET    HAL_GPIO_WritePin(LAT_GPIO_Port, LAT_Pin, GPIO_PIN_SET)
#define LAT_RESET  HAL_GPIO_WritePin(LAT_GPIO_Port, LAT_Pin, GPIO_PIN_RESET) 
#define R1_SET     HAL_GPIO_WritePin(R1_GPIO_Port,  R1_Pin,  GPIO_PIN_SET)
#define R1_RESET   HAL_GPIO_WritePin(R1_GPIO_Port,  R1_Pin,  GPIO_PIN_RESET)
#define G1_SET     HAL_GPIO_WritePin(G1_GPIO_Port,  G1_Pin,  GPIO_PIN_SET)
#define G1_RESET   HAL_GPIO_WritePin(G1_GPIO_Port,  G1_Pin,  GPIO_PIN_RESET)

#define chonHang(h)  (0x02&h)?B_SET:B_RESET; \
                     (0x01&h)?A_SET:A_RESET; \
                      OE_EN
	
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint8_t  LED1[16][32];
uint8_t  LED2[16][32];
int16_t xn[N_point]; 
//int16_t buffer[N_point];
uint8_t  sampleDone;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

const uint16_t bitRevTable512[512] = 
{
    0x0000, 0x0100, 0x0080, 0x0180, 0x0040, 0x0140, 0x00C0, 0x01C0,
    0x0020, 0x0120, 0x00A0, 0x01A0, 0x0060, 0x0160, 0x00E0, 0x01E0,
    0x0010, 0x0110, 0x0090, 0x0190, 0x0050, 0x0150, 0x00D0, 0x01D0,
    0x0030, 0x0130, 0x00B0, 0x01B0, 0x0070, 0x0170, 0x00F0, 0x01F0,
    0x0008, 0x0108, 0x0088, 0x0188, 0x0048, 0x0148, 0x00C8, 0x01C8,
    0x0028, 0x0128, 0x00A8, 0x01A8, 0x0068, 0x0168, 0x00E8, 0x01E8,
    0x0018, 0x0118, 0x0098, 0x0198, 0x0058, 0x0158, 0x00D8, 0x01D8,
    0x0038, 0x0138, 0x00B8, 0x01B8, 0x0078, 0x0178, 0x00F8, 0x01F8,
    0x0004, 0x0104, 0x0084, 0x0184, 0x0044, 0x0144, 0x00C4, 0x01C4,
    0x0024, 0x0124, 0x00A4, 0x01A4, 0x0064, 0x0164, 0x00E4, 0x01E4,
    0x0014, 0x0114, 0x0094, 0x0194, 0x0054, 0x0154, 0x00D4, 0x01D4,
    0x0034, 0x0134, 0x00B4, 0x01B4, 0x0074, 0x0174, 0x00F4, 0x01F4,
    0x000C, 0x010C, 0x008C, 0x018C, 0x004C, 0x014C, 0x00CC, 0x01CC,
    0x002C, 0x012C, 0x00AC, 0x01AC, 0x006C, 0x016C, 0x00EC, 0x01EC,
    0x001C, 0x011C, 0x009C, 0x019C, 0x005C, 0x015C, 0x00DC, 0x01DC,
    0x003C, 0x013C, 0x00BC, 0x01BC, 0x007C, 0x017C, 0x00FC, 0x01FC,
    0x0002, 0x0102, 0x0082, 0x0182, 0x0042, 0x0142, 0x00C2, 0x01C2,
    0x0022, 0x0122, 0x00A2, 0x01A2, 0x0062, 0x0162, 0x00E2, 0x01E2,
    0x0012, 0x0112, 0x0092, 0x0192, 0x0052, 0x0152, 0x00D2, 0x01D2,
    0x0032, 0x0132, 0x00B2, 0x01B2, 0x0072, 0x0172, 0x00F2, 0x01F2,
    0x000A, 0x010A, 0x008A, 0x018A, 0x004A, 0x014A, 0x00CA, 0x01CA,
    0x002A, 0x012A, 0x00AA, 0x01AA, 0x006A, 0x016A, 0x00EA, 0x01EA,
    0x001A, 0x011A, 0x009A, 0x019A, 0x005A, 0x015A, 0x00DA, 0x01DA,
    0x003A, 0x013A, 0x00BA, 0x01BA, 0x007A, 0x017A, 0x00FA, 0x01FA,
    0x0006, 0x0106, 0x0086, 0x0186, 0x0046, 0x0146, 0x00C6, 0x01C6,
    0x0026, 0x0126, 0x00A6, 0x01A6, 0x0066, 0x0166, 0x00E6, 0x01E6,
    0x0016, 0x0116, 0x0096, 0x0196, 0x0056, 0x0156, 0x00D6, 0x01D6,
    0x0036, 0x0136, 0x00B6, 0x01B6, 0x0076, 0x0176, 0x00F6, 0x01F6,
    0x000E, 0x010E, 0x008E, 0x018E, 0x004E, 0x014E, 0x00CE, 0x01CE,
    0x002E, 0x012E, 0x00AE, 0x01AE, 0x006E, 0x016E, 0x00EE, 0x01EE,
    0x001E, 0x011E, 0x009E, 0x019E, 0x005E, 0x015E, 0x00DE, 0x01DE,
    0x003E, 0x013E, 0x00BE, 0x01BE, 0x007E, 0x017E, 0x00FE, 0x01FE,
    0x0001, 0x0101, 0x0081, 0x0181, 0x0041, 0x0141, 0x00C1, 0x01C1,
    0x0021, 0x0121, 0x00A1, 0x01A1, 0x0061, 0x0161, 0x00E1, 0x01E1,
    0x0011, 0x0111, 0x0091, 0x0191, 0x0051, 0x0151, 0x00D1, 0x01D1,
    0x0031, 0x0131, 0x00B1, 0x01B1, 0x0071, 0x0171, 0x00F1, 0x01F1,
    0x0009, 0x0109, 0x0089, 0x0189, 0x0049, 0x0149, 0x00C9, 0x01C9,
    0x0029, 0x0129, 0x00A9, 0x01A9, 0x0069, 0x0169, 0x00E9, 0x01E9,
    0x0019, 0x0119, 0x0099, 0x0199, 0x0059, 0x0159, 0x00D9, 0x01D9,
    0x0039, 0x0139, 0x00B9, 0x01B9, 0x0079, 0x0179, 0x00F9, 0x01F9,
    0x0005, 0x0105, 0x0085, 0x0185, 0x0045, 0x0145, 0x00C5, 0x01C5,
    0x0025, 0x0125, 0x00A5, 0x01A5, 0x0065, 0x0165, 0x00E5, 0x01E5,
    0x0015, 0x0115, 0x0095, 0x0195, 0x0055, 0x0155, 0x00D5, 0x01D5,
    0x0035, 0x0135, 0x00B5, 0x01B5, 0x0075, 0x0175, 0x00F5, 0x01F5,
    0x000D, 0x010D, 0x008D, 0x018D, 0x004D, 0x014D, 0x00CD, 0x01CD,
    0x002D, 0x012D, 0x00AD, 0x01AD, 0x006D, 0x016D, 0x00ED, 0x01ED,
    0x001D, 0x011D, 0x009D, 0x019D, 0x005D, 0x015D, 0x00DD, 0x01DD,
    0x003D, 0x013D, 0x00BD, 0x01BD, 0x007D, 0x017D, 0x00FD, 0x01FD,
    0x0003, 0x0103, 0x0083, 0x0183, 0x0043, 0x0143, 0x00C3, 0x01C3,
    0x0023, 0x0123, 0x00A3, 0x01A3, 0x0063, 0x0163, 0x00E3, 0x01E3,
    0x0013, 0x0113, 0x0093, 0x0193, 0x0053, 0x0153, 0x00D3, 0x01D3,
    0x0033, 0x0133, 0x00B3, 0x01B3, 0x0073, 0x0173, 0x00F3, 0x01F3,
    0x000B, 0x010B, 0x008B, 0x018B, 0x004B, 0x014B, 0x00CB, 0x01CB,
    0x002B, 0x012B, 0x00AB, 0x01AB, 0x006B, 0x016B, 0x00EB, 0x01EB,
    0x001B, 0x011B, 0x009B, 0x019B, 0x005B, 0x015B, 0x00DB, 0x01DB,
    0x003B, 0x013B, 0x00BB, 0x01BB, 0x007B, 0x017B, 0x00FB, 0x01FB,
    0x0007, 0x0107, 0x0087, 0x0187, 0x0047, 0x0147, 0x00C7, 0x01C7,
    0x0027, 0x0127, 0x00A7, 0x01A7, 0x0067, 0x0167, 0x00E7, 0x01E7,
    0x0017, 0x0117, 0x0097, 0x0197, 0x0057, 0x0157, 0x00D7, 0x01D7,
    0x0037, 0x0137, 0x00B7, 0x01B7, 0x0077, 0x0177, 0x00F7, 0x01F7,
    0x000F, 0x010F, 0x008F, 0x018F, 0x004F, 0x014F, 0x00CF, 0x01CF,
    0x002F, 0x012F, 0x00AF, 0x01AF, 0x006F, 0x016F, 0x00EF, 0x01EF,
    0x001F, 0x011F, 0x009F, 0x019F, 0x005F, 0x015F, 0x00DF, 0x01DF,
    0x003F, 0x013F, 0x00BF, 0x01BF, 0x007F, 0x017F, 0x00FF, 0x01FF
};

const int16_t cosTable[512] = 
{
		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,	
		0xfe,		0xfe,		0xfe,		0xfd,		0xfd,		0xfc,		0xfc,		0xfb,	
		0xfb,		0xfa,		0xf9,		0xf9,		0xf8,		0xf7,		0xf6,		0xf5,	
		0xf4,		0xf4,		0xf3,		0xf2,		0xf1,		0xef,		0xee,		0xed,	
		0xec,		0xeb,		0xea,		0xe8,		0xe7,		0xe6,		0xe4,		0xe3,	
		0xe1,		0xe0,		0xde,		0xdd,		0xdb,		0xd9,		0xd8,		0xd6,	
		0xd4,		0xd3,		0xd1,		0xcf,		0xcd,		0xcb,		0xc9,		0xc7,	
		0xc5,		0xc3,		0xc1,		0xbf,		0xbd,		0xbb,		0xb9,		0xb7,	
		0xb5,		0xb2,		0xb0,		0xae,		0xab,		0xa9,		0xa7,		0xa4,	
		0xa2,		0x9f,		0x9d,		0x9b,		0x98,		0x95,		0x93,		0x90,	
		0x8e,		0x8b,		0x88,		0x86,		0x83,		0x80,		0x7e,		0x7b,	
		0x78,		0x75,		0x73,		0x70,		0x6d,		0x6a,		0x67,		0x64,	
		0x61,		0x5f,		0x5c,		0x59,		0x56,		0x53,		0x50,		0x4d,	
		0x4a,		0x47,		0x44,		0x41,		0x3e,		0x3b,		0x38,		0x35,	
		0x31,		0x2e,		0x2b,		0x28,		0x25,		0x22,		0x1f,		0x1c,	
		0x19,		0x15,		0x12,		0xf,		0xc,		0x9,		0x6,		0x3,	
		0x0,		0xfffd,	0xfffa,	0xfff7,	0xfff4,	0xfff1,	0xffee,	0xffeb,	
		0xffe7,	0xffe4,	0xffe1,	0xffde,	0xffdb,	0xffd8,	0xffd5,	0xffd2,	
		0xffcf,	0xffcb,	0xffc8,	0xffc5,	0xffc2,	0xffbf,	0xffbc,	0xffb9,	
		0xffb6,	0xffb3,	0xffb0,	0xffad,	0xffaa,	0xffa7,	0xffa4,	0xffa1,	
		0xff9f,	0xff9c,	0xff99,	0xff96,	0xff93,	0xff90,	0xff8d,	0xff8b,	
		0xff88,	0xff85,	0xff82,	0xff80,	0xff7d,	0xff7a,	0xff78,	0xff75,	
		0xff72,	0xff70,	0xff6d,	0xff6b,	0xff68,	0xff65,	0xff63,	0xff61,	
		0xff5e,	0xff5c,	0xff59,	0xff57,	0xff55,	0xff52,	0xff50,	0xff4e,	
		0xff4b,	0xff49,	0xff47,	0xff45,	0xff43,	0xff41,	0xff3f,	0xff3d,	
		0xff3b,	0xff39,	0xff37,	0xff35,	0xff33,	0xff31,	0xff2f,	0xff2d,	
		0xff2c,	0xff2a,	0xff28,	0xff27,	0xff25,	0xff23,	0xff22,	0xff20,	
		0xff1f,	0xff1d,	0xff1c,	0xff1a,	0xff19,	0xff18,	0xff16,	0xff15,	
		0xff14,	0xff13,	0xff12,	0xff11,	0xff0f,	0xff0e,	0xff0d,	0xff0c,	
		0xff0c,	0xff0b,	0xff0a,	0xff09,	0xff08,	0xff07,	0xff07,	0xff06,	
		0xff05,	0xff05,	0xff04,	0xff04,	0xff03,	0xff03,	0xff02,	0xff02,	
		0xff02,	0xff01,	0xff01,	0xff01,	0xff01,	0xff01,	0xff01,	0xff01,	
		0xff00,	0xff01,	0xff01,	0xff01,	0xff01,	0xff01,	0xff01,	0xff01,	
		0xff02,	0xff02,	0xff02,	0xff03,	0xff03,	0xff04,	0xff04,	0xff05,	
		0xff05,	0xff06,	0xff07,	0xff07,	0xff08,	0xff09,	0xff0a,	0xff0b,	
		0xff0c,	0xff0c,	0xff0d,	0xff0e,	0xff0f,	0xff11,	0xff12,	0xff13,	
		0xff14,	0xff15,	0xff16,	0xff18,	0xff19,	0xff1a,	0xff1c,	0xff1d,	
		0xff1f,	0xff20,	0xff22,	0xff23,	0xff25,	0xff27,	0xff28,	0xff2a,	
		0xff2c,	0xff2d,	0xff2f,	0xff31,	0xff33,	0xff35,	0xff37,	0xff39,	
		0xff3b,	0xff3d,	0xff3f,	0xff41,	0xff43,	0xff45,	0xff47,	0xff49,	
		0xff4b,	0xff4e,	0xff50,	0xff52,	0xff55,	0xff57,	0xff59,	0xff5c,	
		0xff5e,	0xff61,	0xff63,	0xff65,	0xff68,	0xff6b,	0xff6d,	0xff70,	
		0xff72,	0xff75,	0xff78,	0xff7a,	0xff7d,	0xff80,	0xff82,	0xff85,	
		0xff88,	0xff8b,	0xff8d,	0xff90,	0xff93,	0xff96,	0xff99,	0xff9c,	
		0xff9f,	0xffa1,	0xffa4,	0xffa7,	0xffaa,	0xffad,	0xffb0,	0xffb3,	
		0xffb6,	0xffb9,	0xffbc,	0xffbf,	0xffc2,	0xffc5,	0xffc8,	0xffcb,	
		0xffcf,	0xffd2,	0xffd5,	0xffd8,	0xffdb,	0xffde,	0xffe1,	0xffe4,	
		0xffe7,	0xffeb,	0xffee,	0xfff1,	0xfff4,	0xfff7,	0xfffa,	0xfffd,	
		0x0,		0x3,		0x6,		0x9,		0xc,		0xf,		0x12,		0x15,	
		0x19,		0x1c,		0x1f,		0x22,		0x25,		0x28,		0x2b,		0x2e,	
		0x31,		0x35,		0x38,		0x3b,		0x3e,		0x41,		0x44,		0x47,	
		0x4a,		0x4d,		0x50,		0x53,		0x56,		0x59,		0x5c,		0x5f,	
		0x61,		0x64,		0x67,		0x6a,		0x6d,		0x70,		0x73,		0x75,	
		0x78,		0x7b,		0x7e,		0x80,		0x83,		0x86,		0x88,		0x8b,	
		0x8e,		0x90,		0x93,		0x95,		0x98,		0x9b,		0x9d,		0x9f,	
		0xa2,		0xa4,		0xa7,		0xa9,		0xab,		0xae,		0xb0,		0xb2,	
		0xb5,		0xb7,		0xb9,		0xbb,		0xbd,		0xbf,		0xc1,		0xc3,	
		0xc5,		0xc7,		0xc9,		0xcb,		0xcd,		0xcf,		0xd1,		0xd3,	
		0xd4,		0xd6,		0xd8,		0xd9,		0xdb,		0xdd,		0xde,		0xe0,	
		0xe1,		0xe3,		0xe4,		0xe6,		0xe7,		0xe8,		0xea,		0xeb,	
		0xec,		0xed,		0xee,		0xef,		0xf1,		0xf2,		0xf3,		0xf4,	
		0xf4,		0xf5,		0xf6,		0xf7,		0xf8,		0xf9,		0xf9,		0xfa,	
		0xfb,		0xfb,		0xfc,		0xfc,		0xfd,		0xfd,		0xfe,		0xfe,	
		0xfe,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff
};

const int16_t sinTable[512] = 
{
		0x0,		0xfffd,	0xfffa,	0xfff7,	0xfff4,	0xfff1,	0xffee,	0xffeb,	
		0xffe7,	0xffe4,	0xffe1,	0xffde,	0xffdb,	0xffd8,	0xffd5,	0xffd2,	
		0xffcf,	0xffcb,	0xffc8,	0xffc5,	0xffc2,	0xffbf,	0xffbc,	0xffb9,	
		0xffb6,	0xffb3,	0xffb0,	0xffad,	0xffaa,	0xffa7,	0xffa4,	0xffa1,	
		0xff9f,	0xff9c,	0xff99,	0xff96,	0xff93,	0xff90,	0xff8d,	0xff8b,	
		0xff88,	0xff85,	0xff82,	0xff80,	0xff7d,	0xff7a,	0xff78,	0xff75,	
		0xff72,	0xff70,	0xff6d,	0xff6b,	0xff68,	0xff65,	0xff63,	0xff61,	
		0xff5e,	0xff5c,	0xff59,	0xff57,	0xff55,	0xff52,	0xff50,	0xff4e,	
		0xff4b,	0xff49,	0xff47,	0xff45,	0xff43,	0xff41,	0xff3f,	0xff3d,	
		0xff3b,	0xff39,	0xff37,	0xff35,	0xff33,	0xff31,	0xff2f,	0xff2d,	
		0xff2c,	0xff2a,	0xff28,	0xff27,	0xff25,	0xff23,	0xff22,	0xff20,	
		0xff1f,	0xff1d,	0xff1c,	0xff1a,	0xff19,	0xff18,	0xff16,	0xff15,	
		0xff14,	0xff13,	0xff12,	0xff11,	0xff0f,	0xff0e,	0xff0d,	0xff0c,	
		0xff0c,	0xff0b,	0xff0a,	0xff09,	0xff08,	0xff07,	0xff07,	0xff06,	
		0xff05,	0xff05,	0xff04,	0xff04,	0xff03,	0xff03,	0xff02,	0xff02,	
		0xff02,	0xff01,	0xff01,	0xff01,	0xff01,	0xff01,	0xff01,	0xff01,	
		0xff00,	0xff01,	0xff01,	0xff01,	0xff01,	0xff01,	0xff01,	0xff01,	
		0xff02,	0xff02,	0xff02,	0xff03,	0xff03,	0xff04,	0xff04,	0xff05,	
		0xff05,	0xff06,	0xff07,	0xff07,	0xff08,	0xff09,	0xff0a,	0xff0b,	
		0xff0c,	0xff0c,	0xff0d,	0xff0e,	0xff0f,	0xff11,	0xff12,	0xff13,	
		0xff14,	0xff15,	0xff16,	0xff18,	0xff19,	0xff1a,	0xff1c,	0xff1d,	
		0xff1f,	0xff20,	0xff22,	0xff23,	0xff25,	0xff27,	0xff28,	0xff2a,	
		0xff2c,	0xff2d,	0xff2f,	0xff31,	0xff33,	0xff35,	0xff37,	0xff39,	
		0xff3b,	0xff3d,	0xff3f,	0xff41,	0xff43,	0xff45,	0xff47,	0xff49,	
		0xff4b,	0xff4e,	0xff50,	0xff52,	0xff55,	0xff57,	0xff59,	0xff5c,	
		0xff5e,	0xff61,	0xff63,	0xff65,	0xff68,	0xff6b,	0xff6d,	0xff70,	
		0xff72,	0xff75,	0xff78,	0xff7a,	0xff7d,	0xff80,	0xff82,	0xff85,	
		0xff88,	0xff8b,	0xff8d,	0xff90,	0xff93,	0xff96,	0xff99,	0xff9c,	
		0xff9f,	0xffa1,	0xffa4,	0xffa7,	0xffaa,	0xffad,	0xffb0,	0xffb3,	
		0xffb6,	0xffb9,	0xffbc,	0xffbf,	0xffc2,	0xffc5,	0xffc8,	0xffcb,	
		0xffcf,	0xffd2,	0xffd5,	0xffd8,	0xffdb,	0xffde,	0xffe1,	0xffe4,	
		0xffe7,	0xffeb,	0xffee,	0xfff1,	0xfff4,	0xfff7,	0xfffa,	0xfffd,	
		0x0,		0x3,		0x6,		0x9,		0xc,		0xf,		0x12,		0x15,	
		0x19,		0x1c,		0x1f,		0x22,		0x25,		0x28,		0x2b,		0x2e,	
		0x31,		0x35,		0x38,		0x3b,		0x3e,		0x41,		0x44,		0x47,	
		0x4a,		0x4d,		0x50,		0x53,		0x56,		0x59,		0x5c,		0x5f,	
		0x61,		0x64,		0x67,		0x6a,		0x6d,		0x70,		0x73,		0x75,	
		0x78,		0x7b,		0x7e,		0x80,		0x83,		0x86,		0x88,		0x8b,	
		0x8e,		0x90,		0x93,		0x95,		0x98,		0x9b,		0x9d,		0x9f,	
		0xa2,		0xa4,		0xa7,		0xa9,		0xab,		0xae,		0xb0,		0xb2,	
		0xb5,		0xb7,		0xb9,		0xbb,		0xbd,		0xbf,		0xc1,		0xc3,	
		0xc5,		0xc7,		0xc9,		0xcb,		0xcd,		0xcf,		0xd1,		0xd3,	
		0xd4,		0xd6,		0xd8,		0xd9,		0xdb,		0xdd,		0xde,		0xe0,	
		0xe1,		0xe3,		0xe4,		0xe6,		0xe7,		0xe8,		0xea,		0xeb,	
		0xec,		0xed,		0xee,		0xef,		0xf1,		0xf2,		0xf3,		0xf4,	
		0xf4,		0xf5,		0xf6,		0xf7,		0xf8,		0xf9,		0xf9,		0xfa,	
		0xfb,		0xfb,		0xfc,		0xfc,		0xfd,		0xfd,		0xfe,		0xfe,	
		0xfe,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,	
		0x100,	0xff,		0xff,		0xff,		0xff,		0xff,		0xff,		0xff,	
		0xfe,		0xfe,		0xfe,		0xfd,		0xfd,		0xfc,		0xfc,		0xfb,	
		0xfb,		0xfa,		0xf9,		0xf9,		0xf8,		0xf7,		0xf6,		0xf5,	
		0xf4,		0xf4,		0xf3,		0xf2,		0xf1,		0xef,		0xee,		0xed,	
		0xec,		0xeb,		0xea,		0xe8,		0xe7,		0xe6,		0xe4,		0xe3,	
		0xe1,		0xe0,		0xde,		0xdd,		0xdb,		0xd9,		0xd8,		0xd6,	
		0xd4,		0xd3,		0xd1,		0xcf,		0xcd,		0xcb,		0xc9,		0xc7,	
		0xc5,		0xc3,		0xc1,		0xbf,		0xbd,		0xbb,		0xb9,		0xb7,	
		0xb5,		0xb2,		0xb0,		0xae,		0xab,		0xa9,		0xa7,		0xa4,	
		0xa2,		0x9f,		0x9d,		0x9b,		0x98,		0x95,		0x93,		0x90,	
		0x8e,		0x8b,		0x88,		0x86,		0x83,		0x80,		0x7e,		0x7b,	
		0x78,		0x75,		0x73,		0x70,		0x6d,		0x6a,		0x67,		0x64,	
		0x61,		0x5f,		0x5c,		0x59,		0x56,		0x53,		0x50,		0x4d,	
		0x4a,		0x47,		0x44,		0x41,		0x3e,		0x3b,		0x38,		0x35,	
		0x31,		0x2e,		0x2b,		0x28,		0x25,		0x22,		0x1f,		0x1c,	
		0x19,		0x15,		0x12,		0xf,		0xc,		0x9,		0x6,		0x3
};	
/* define for FFT caculate */
										
void P10_scan(void) 	
{
	static uint8_t hang = 0;
	static uint8_t	count = 0;
	/* quet led */
	uint8_t x, y, z, temp1, temp2;
	OE_DIS;
	for(z = 0; z <= 16; z += 16)
	{
		for(y = 0; y <= 12; y += 4)
		{
			temp1 = hang+12-y;
			for(x = 0; x < 16; x++)
			{	
				temp2 = x + z;
				LED1[temp1][temp2] > count ? R1_SET : R1_RESET; /* pin Red data   */
				LED2[temp1][temp2] > count ? G1_SET : G1_RESET; /* pin Green data */
				CLK_SET;
				CLK_RESET;
			}
		}
	}
	LAT_SET;
	LAT_RESET;
	chonHang(hang);
	if(++hang == 4) 
	{
			hang = 0;
			if(++count == 15) count = 0;
	}
}

void P10_draw(int16_t *mag)
{
	uint16_t i, x;
	for(i = 1; i <= 32; i++)
	{
		if(mag[i] >= 16)
		{
			for(x = 16; x > 1; x--)
			{
				LED1[16 - x][i-1] = 15;
				LED2[16 - x][i-1] = 0;
			}
		}
		
		if(mag[i] < 16)
		{
			for(x = mag[i]; x > 1; x--) 
			{			
				LED1[16 - x][i-1] = 15;
				LED2[16 - x][i-1] = 0;
			}
			for(x = 16; x > mag[i]; x--) LED1[16 - x][i-1] = LED2[16 - x][i-1] = 0;
		}
		
		if(mag[i] == 0) { LED1[15][i-1] = 0; LED2[15][i-1] = 15; } 
	}
}

uint16_t ADC_read(void)
{
	HAL_ADC_Start(&hadc1);
	while(!__HAL_ADC_GET_FLAG(&hadc1, ADC_FLAG_EOC));
	HAL_ADC_Stop(&hadc1);
	return (uint16_t)HAL_ADC_GetValue(&hadc1);
}

void FFT_calculate(int16_t *R)
{
	int16_t I[N_point];
	uint16_t step, temp3, temp, tem, k, j;
	for(k = 0; k < N_point; k++) I[k] = 0;
	for(step = 1; step <= log2N; step++)
	{
		temp3 = 1<<(step -1);  
		for(k = 0; k < temp3; k++)
		{
			temp = 1<<(step);;
			tem = k*N_point/temp;
			int16_t WR = cosTable[tem];
			int16_t WI = sinTable[tem];			
			for(j = 0; j < N_point; j+=temp)
			{						
				int16_t tempR, tempI;
				tempR = FP__MUL(WR, R[j + temp3 + k]) - FP__MUL(WI, I[j + temp3 + k]);
				tempI = FP__MUL(WR, I[j + temp3 + k]) + FP__MUL(WI, R[j + temp3 + k]);
				
				
				R[j + temp3 + k]   = (R[j + k] - tempR);//WR * R[j + temp3 + k] + WI * I[j + temp3 + k];
				I[j + temp3 + k]   = (I[j + k] - tempI);//WR * I[j + temp3 + k] - WI * temp2;	
				
				R[j + k]           = (R[j + k] + tempR);//WR * R[j + temp3 + k] - WI * I[j + temp3 + k];						
				I[j + k]      	   = (I[j + k] + tempI);//WR * I[j + temp3 + k] + WI * temp2;			
			}
		}
	}
	
	for(k = 1; k <= 32; k++)
	{
		R[k] = (int16_t)sqrt(R[k]*R[k] + I[k]*I[k]);
		R[k] >>= 6;
	}
}

//void FFT_calculate(int16_t *R, int16_t *I, uint16_t N)
//{
//	if(N >= 2)
//	{
//		N /= 2;
//		FFT_calculate(R,     I,     N);
//		FFT_calculate(R + N, I + N, N);

//		for(uint16_t k = 0; k < N; k++)
//		{
//			uint16_t tem = k*256/N;
//			int16_t WR = cosTable[tem];
//			int16_t WI = sinTable[tem];

//			/************************/
//			/*  Ek_re = R[k]        */
//			/*  Ek_im = R[k + N]  */
//			/*  Ok_re = I[k]        */
//			/*  Ok_im = I[k + N]  */
//			/************************/
//			int16_t tempR, tempI;
//			tempR = FP__MUL(WR, R[k + N]) - FP__MUL(WI, I[k + N]);
//			tempI = FP__MUL(WR, I[k + N]) + FP__MUL(WI, R[k + N]);
//			
//			R[k + N] = (R[k] - tempR);
//			I[k + N] = (I[k] - tempI);
//			
//			R[k]     = (R[k] + tempR);
//			I[k]     = (I[k] + tempI);
//			
////			//Tranh overflow			
////			R[k + N] = (R[k] - tempR)>>1;
////			I[k + N] = (I[k] - tempI)>>1;
////			
////			R[k]     = (R[k] + tempR)>>1;
////			I[k]     = (I[k] + tempI)>>1;		
//		}
//	}
//}


//void mag(int16_t *R, int16_t *I)
//{
//	uint16_t k;
//	for(k = 1; k <= 32; k++)
//	{
//		R[k] = (int16_t)sqrt(R[k]*R[k] + I[k]*I[k]);
//		R[k] >>= 6;
//	}
//}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == htim2.Instance)
	{
		P10_scan();
	}
	
	else
	if(htim->Instance == htim3.Instance)  // sample 512 points at 38kHz
	{
		uint16_t adc_value;
		static uint16_t cnt = 0;
		adc_value = ADC_read()>>2;  // >>2 10bit value
	  if(sampleDone == 0)
	  {
		  xn[bitRevTable512[cnt]] = adc_value;
			//buffer[cnt] = 0;
		  cnt++;
		  if(cnt > N_point)
		  {
			  sampleDone = 1;
			  cnt = 0;
		  }
	  }
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
	for(uint8_t i = 0; i < 32; i++)
	{ LED1[15][i] = 0; LED2[15][i] = 15; } 
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
		while(!sampleDone);
		HAL_TIM_Base_Stop_IT(&htim3);
		FFT_calculate(xn);
//		FFT_calculate(xn, buffer, N_point);
//		mag(xn, buffer);
		P10_draw(xn);
		//P10_effect1(g_xn);
		sampleDone = 0;
		HAL_TIM_Base_Start_IT(&htim3);
  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
