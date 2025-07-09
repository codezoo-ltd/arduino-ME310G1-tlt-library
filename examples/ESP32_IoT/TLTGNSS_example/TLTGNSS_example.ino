/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
  - TLTMDM.h

  @brief
  Example of test on the use of the GNSS class of the TLT library.

  @details
  In this example sketch, it is shown how to use GNSS management using GNSS class of TLT library.\n
  GNSS configuration, GNSS controller power management, GNSS nmea configuration functions are shown.\n
  GPS positions are acquired and response is printed.
NOTE:\n
For the sketch to work correctly, GNSS should be tested in open sky conditions to allow a fix. The fix may take a few minutes.


@version
1.0.0

@note

@author
Fabio Pintus

@date
10/14/2021

@ported by rooney.jang (rooney.jang@codezoo.co.kr)	
 */


#include <TLTMDM.h>

#include <U8x8lib.h>
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE);
#define U8LOG_WIDTH 16
#define U8LOG_HEIGHT 8
uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];
U8X8LOG u8x8log;

#define ON_OFF 18 /*Select the GPIO to control ON_OFF*/
#define PWR_PIN 5 /*Select the GPIO to control LDO*/
HardwareSerial MDMSerial(2); //use ESP32 UART2

ME310 *_me310;

/*When NMEA_DEBUG is false Unsolicited NMEA is disabled*/
/*NMEA is true*/
TLTGNSS *gnss;

void setup()
{
	u8x8.begin();
	u8x8.setFont(u8x8_font_chroma48medium8_r);

	u8x8log.begin(u8x8, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
	u8x8log.setRedrawMode(1); // 0: Update screen with newline, 1: Update screen for every char

	Serial.begin(115200);
	MDMSerial.begin(115200,SERIAL_8N1,16,17); //RXD2:16, TXD2:17
	pinMode(PWR_PIN,OUTPUT);
	digitalWrite(PWR_PIN,HIGH);   
	delay(100);
	_me310 = new ME310(MDMSerial);
	gnss = new TLTGNSS(_me310, true);
	Serial.println("TLT GNSS example, enabling ME310 module");
	u8x8log.print("TLT GNSS example, enabling ME310 module\n");
	_me310->debugMode(false);
	_me310->powerOn(ON_OFF);
	Serial.print("Initializing GNSS");
	while (!gnss->setGNSSConfiguration())
	{
		Serial.print(".");
	}
	Serial.println(" is completed successfully");
}

void loop()
{
	DMS lat_dms, lng_dms;
	float lat, lng;
	GNSSInfo gnssInfo = gnss->getGNSSData();
	/*gnssInfo fields will have latitude, longitude and the other details in string format*/

	/*Fix 1.2 or 1.3 means valid fix*/
	if (gnssInfo.fix.toFloat() > 1.0)
	{
		Serial.println("");
		Serial.print("Fix valid, converting...");
		if (gnss->convertNMEA2Decimal(gnssInfo.latitude, gnssInfo.longitude, &lat, &lng))
		{

			Serial.println("Conversion done!");
			Serial.println(lat, 6);
			Serial.println(lng, 6);
			u8x8log.print(lat, 6);
			u8x8log.print("\n");
			u8x8log.print(lng, 6);
			u8x8log.print("\n");

			lat_dms = gnss->convertDecimal2DMS(lat);
			lng_dms = gnss->convertDecimal2DMS(lng);

			Serial.println("");
			Serial.println("DMS coordinates: ");
			if (lat > 0)
			{
				Serial.print("N ");
				u8x8log.print("N ");
			}
			else
			{
				Serial.print("S ");
				u8x8log.print("S ");
			}
			Serial.print(lat_dms.degrees);
			u8x8log.print(lat_dms.degrees);
			Serial.print("° ");
			u8x8log.print("* ");
			Serial.print(lat_dms.minutes);
			u8x8log.print(lat_dms.minutes);
			Serial.print("' ");
			u8x8log.print("' ");
			Serial.print(lat_dms.seconds);
			u8x8log.print(lat_dms.seconds);
			Serial.println("\"");
			u8x8log.print("\"\n");

			if (lng > 0)
			{
				Serial.print("E ");
				u8x8log.print("E ");
			}
			else
			{
				Serial.print("W ");
				u8x8log.print("W ");
			}
			Serial.print(lng_dms.degrees);
			u8x8log.print(lng_dms.degrees);
			Serial.print("° ");
			u8x8log.print("* ");
			Serial.print(lng_dms.minutes);
			u8x8log.print(lng_dms.minutes);
			Serial.print("' ");
			u8x8log.print("' ");
			Serial.print(lng_dms.seconds);
			u8x8log.print(lng_dms.seconds);
			Serial.println("\"");
			u8x8log.print("\"\n");
		}
		else
		{
			Serial.println("Conversion failed!");
		}
	}
	else
	{
		Serial.println("Fix not valid yet.");
		Serial.print("Fix value: ");
		Serial.println(gnssInfo.fix.toFloat());
	}
	delay(10000);
}
