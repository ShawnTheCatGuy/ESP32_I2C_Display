#include <Wire.h>
#include "bitmaps.h"
//============================================================================
/*Connections:

   ESP32 => OLED
 GPIO 22 / SCL => D0 (add ~5K pullup)
 GPIO 21 / SDA => D1 and D2 (add ~5K pullup)
 GPIO 18 => *RESET
 GND => D/C (Sets I2C address)
   GND => BS0 (Selects I2C)
   3.3v => BS1 (Selects I2C)
  3.3v => VBAT
  3.3v => VDD
*/
//============================================================================
#define OLED_RESET 18
#define CLR_RESET (digitalWrite(OLED_RESET, LOW))
#define SET_RESET (digitalWrite(OLED_RESET, HIGH))
//============================================================================
#define NUMBER_OF_SCREENS (17)
#define GEAR_START_SCREEN (9)
const SCREEN_IMAGE *const screens[NUMBER_OF_SCREENS] PROGMEM=
  {
  &Logo_1_4864,
  &Logo_2_4864,
  &Logo_3_4864,
  &Dimensions_4864,
  &Up_Arrow_4864,
  &Parking_4864,
  &Down_Arrow_4864,
  &FWD_4864,
  &REW_4864,
  &Gear_0_4864,
  &Gear_1_4864,
  &Gear_2_4864,
  &Gear_3_4864,
  &Gear_4_4864,
  &Gear_5_4864,
  &Gear_6_4864,
  &Gear_7_4864
  };
//============================================================================
void I2C_sendCommand(uint8_t payload)
  {
  Wire.beginTransmission(0x3C); //send start & Slave address
  Wire.write(0x00);		        //Control Byte - Command
  Wire.write(payload);	        //payload
  Wire.endTransmission();
  }
//============================================================================
void I2C_sendData(uint8_t payload)
  {
  Wire.beginTransmission(0x3C);	//send start & Slave address
  Wire.write(0xC0);		        //Control Byte - Data (non-continued)
  Wire.write(payload);          //payload
  Wire.endTransmission();
  }
//============================================================================
#define SSD1306B_00_SET_LOWER_COLUMN_ADDRESS_BIT (0x00)
#define SSD1306B_10_SET_UPPER_COLUMN_ADDRESS_BIT (0x10)
#define SSD1306B_B0_SET_PAGE_START_ADDRESS_BIT   (0xB0)
#define SSD1306B_DCDC_CONFIG_PREFIX_8D           (0x8D)
#define SSD1306B_DCDC_CONFIG_7p5v_14             (0x14)
#define SSD1306B_DCDC_CONFIG_6p0v_15             (0x15)
#define SSD1306B_DCDC_CONFIG_8p5v_94             (0x94)
#define SSD1306B_DCDC_CONFIG_9p0v_95             (0x95)
#define SSD1306B_DISPLAY_OFF_YES_SLEEP_AE        (0xAE)
#define SSD1306B_DISPLAY_ON_NO_SLEEP_AF          (0xAF)
#define SSD1306B_CLOCK_DIVIDE_PREFIX_D5          (0xD5)
#define SSD1306B_MULTIPLEX_RATIO_PREFIX_A8       (0xA8)
#define SSD1306B_DISPLAY_OFFSET_PREFIX_D3        (0xD3)
#define SSD1306B_DISPLAY_START_LINE_40           (0x40)
#define SSD1306B_SEG0_IS_COL_0_A0                (0xA0)
#define SSD1306B_SEG0_IS_COL_127_A1              (0xA1)
#define SSD1306B_SCAN_DIR_UP_C0                  (0xC0)
#define SSD1306B_SCAN_DIR_DOWN_C8                (0xC8)
#define SSD1306B_COM_CONFIG_PREFIX_DA            (0xDA)
#define SSD1306B_COM_CONFIG_SEQUENTIAL_LEFT_02   (0x02)
#define SSD1306B_COM_CONFIG_ALTERNATE_LEFT_12    (0x12)
#define SSD1306B_COM_CONFIG_SEQUENTIAL_RIGHT_22  (0x22)
#define SSD1306B_COM_CONFIG_ALTERNATE_RIGHT_32   (0x32)
#define SSD1306B_CONTRAST_PREFIX_81              (0x81)
#define SSD1306B_PRECHARGE_PERIOD_PREFIX_D9      (0xD9)
#define SSD1306B_VCOMH_DESELECT_PREFIX_DB        (0xDB)
#define SSD1306B_VCOMH_DESELECT_0p65xVCC_00      (0x00)
#define SSD1306B_VCOMH_DESELECT_0p71xVCC_10      (0x10)
#define SSD1306B_VCOMH_DESELECT_0p77xVCC_20      (0x20)
#define SSD1306B_VCOMH_DESELECT_0p83xVCC_30      (0x30)
#define SSD1306B_ENTIRE_DISPLAY_FORCE_ON_A5      (0xA5)
#define SSD1306B_ENTIRE_DISPLAY_NORMAL_A4        (0xA4)
#define SSD1306B_INVERSION_NORMAL_A6             (0xA6)
#define SSD1306B_INVERSION_INVERTED_A7           (0xA7)
//============================================================================
void Set_Address(uint8_t column, uint8_t page)
  {
  //Set column-lower nibble
  I2C_sendCommand(SSD1306B_00_SET_LOWER_COLUMN_ADDRESS_BIT|(column&0x0F));
  //Set column-upper nibble
  I2C_sendCommand(SSD1306B_10_SET_UPPER_COLUMN_ADDRESS_BIT|((column>>4)&0x0F));
  //Set page address, limiting from 0 to 7
  I2C_sendCommand(SSD1306B_B0_SET_PAGE_START_ADDRESS_BIT|(page&0x0F));
  }
//============================================================================
void show_48_x_64_bitmap(const SCREEN_IMAGE *OLED_image)
  {
  // ~ 16.26 mS normal I2C_sendData() calls are ~45 mS
  uint8_t
    column;
  uint8_t
    row;

  for(row=0;row<8;row++)
    {
    //Display is offest by 40 columns
    //(only 48 used of a possible 128, 40 discarded each side)
    Set_Address(40,row);
    //Arduino TWI Library is max 32 bytes by default.
    //Break into a right and left half.
    //Left Half
    Wire.beginTransmission(0x3C);  //send start & Slave address
    Wire.write(0x40);   //Control Byte - Data (continued)
    for(column=0;column<24;column++)
      {
      //Read this byte from the program memory / flash
      Wire.write(pgm_read_byte( &(OLED_image  ->bitmap_data[row][column]) ));
      }
    //Right Half
    Wire.endTransmission();
    Wire.beginTransmission(0x3C);  //send start & Slave address
    Wire.write(0x40);   //Control Byte - Data (continued)
    for(column=24;column<48;column++)
      {
      //Read this byte from the program memory / flash
      Wire.write(pgm_read_byte( &(OLED_image  ->bitmap_data[row][column]) ));
      }
    Wire.endTransmission();
    }
  }
//============================================================================
void Initialize_CFAL4864A(void)
  {
  //Thump the reset.  
  delay(1);
  CLR_RESET;
  delay(1);
  SET_RESET;
  delay(1);

  //Set the display to sleep mode for the rest of the init.
  I2C_sendCommand(SSD1306B_DISPLAY_OFF_YES_SLEEP_AE);

  //Set the clock speed, nominal ~105FPS
  //Low nibble is divide ratio
  //High level is oscillator frequency
  I2C_sendCommand(SSD1306B_CLOCK_DIVIDE_PREFIX_D5);
  I2C_sendCommand(0x80);//177Hz measured

  //Set the multiplex ratio to 1/64
  //Default is 0x3F (1/64 Duty), we need 0x3F (1/64 Duty)
  I2C_sendCommand(SSD1306B_MULTIPLEX_RATIO_PREFIX_A8);
  I2C_sendCommand(0x3F);  

  //Set the display offset to 0 (default)
  I2C_sendCommand(SSD1306B_DISPLAY_OFFSET_PREFIX_D3);
  I2C_sendCommand(0x00);

  //Set the display RAM display start line to 0 (default)
  //Bits 0-5 can be set to 0-63 with a bitwise or
  I2C_sendCommand(SSD1306B_DISPLAY_START_LINE_40);

  //Enable DC/DC converter, 9.0v
  I2C_sendCommand(SSD1306B_DCDC_CONFIG_PREFIX_8D);
  I2C_sendCommand(SSD1306B_DCDC_CONFIG_9p0v_95);

  //Map the columns correctly for our OLED glass layout
  I2C_sendCommand(SSD1306B_SEG0_IS_COL_127_A1);

  //Set COM output scan correctly for our OLED glass layout
  I2C_sendCommand(SSD1306B_SCAN_DIR_DOWN_C8);

  //Set COM pins correctly for our OLED glass layout
  I2C_sendCommand(SSD1306B_COM_CONFIG_PREFIX_DA);
  I2C_sendCommand(SSD1306B_COM_CONFIG_ALTERNATE_LEFT_12);

  //Set Contrast Control / SEG Output Current / Iref
  I2C_sendCommand(SSD1306B_CONTRAST_PREFIX_81);
  I2C_sendCommand(0xFF);  //magic # from factory

  //Set precharge (low nibble) / discharge (high nibble) timing
  //precharge = 15 clocks
  //discharge = 15 clocks
  I2C_sendCommand(SSD1306B_PRECHARGE_PERIOD_PREFIX_D9); //Set Pre-Charge period        
  I2C_sendCommand(0xFF); 

  //Set VCOM Deselect Level
  I2C_sendCommand(SSD1306B_VCOMH_DESELECT_PREFIX_DB);
  I2C_sendCommand(SSD1306B_VCOMH_DESELECT_0p65xVCC_00); 

  //Make sure Entire Display On is disabled (default)
  I2C_sendCommand(SSD1306B_ENTIRE_DISPLAY_NORMAL_A4);

  //Make sure display is not inverted (default)
  I2C_sendCommand(SSD1306B_INVERSION_NORMAL_A6);

  //Get out of sleep mode, into normal operation
  I2C_sendCommand(SSD1306B_DISPLAY_ON_NO_SLEEP_AF);
  }
//============================================================================
uint8_t
  gear_dir;
//----------------------------------------------------------------------------
void setup( void )
{
  pinMode(18, OUTPUT);
  pinMode(2, OUTPUT);

  Serial.begin(9600);
  //Get the I2C going

  Wire.begin();
  //Thanks to Limor / Adafruit for the hint ;-)   
  Wire.setClock(400000); // upgrade to 400KHz!

  //Fire up the I2C OLED
  Initialize_CFAL4864A();

  gear_dir=0;
}
//----------------------------------------------------------------------------
void  loop(void)
{
  digitalWrite(2, HIGH);
  delay(200);
  digitalWrite(2, LOW);
  delay(200);

  uint8_t current_screen;
  //Put up some bitmaps from flash
  for(current_screen=0;current_screen<GEAR_START_SCREEN;current_screen++)
    {
    show_48_x_64_bitmap(screens[current_screen]);
    //Wait a bit . . .
    delay(1500);
    if(current_screen < 4)
      {
      //Wait a little extra for the logo & dimensions
      delay(750);
      }
    }
  //Loop the gear animation as fast as the I2C can stuff it out.
  uint8_t repeat;

  if(0==gear_dir)
    {
    for(repeat=0;repeat<25;repeat++)
      {
      for(current_screen=GEAR_START_SCREEN;current_screen<NUMBER_OF_SCREENS;current_screen++)
        {
        show_48_x_64_bitmap(screens[current_screen]);
		//I2C slow enough to not need a delay
        }
      }
    gear_dir=1;  
    }
  else
    {
    for(repeat=0;repeat<25;repeat++)
      {
      for(current_screen=NUMBER_OF_SCREENS-1;GEAR_START_SCREEN<=current_screen;current_screen--)
        {
        show_48_x_64_bitmap(screens[current_screen]);
		//I2C slow enough to not need a delay
        }
      }
    gear_dir=0;  
    }
}
//============================================================================

