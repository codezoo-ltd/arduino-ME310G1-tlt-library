/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
  - TLTMDM.h

  @brief
  Scan Networks.

  @details
  This example sketch prints the IMEI number of the modem, then check if it is connected to an operator.\n
  It then scans nearby networks and prints their signal strength.
  @version
  1.0.0

  @note

  @author
  Cristina Desogus

  @date
  09/23/2021

  @ported by rooney.jang (rooney.jang@codezoo.co.kr)	
 */
// libraries

// initialize the library instance
#include "TLTMDM.h"

#include <U8x8lib.h>
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE);
#define U8LOG_WIDTH 16
#define U8LOG_HEIGHT 8
uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];
U8X8LOG u8x8log;

#define ON_OFF 18 /*Select the GPIO to control ON_OFF*/
#define PWR_PIN 5 /*Select the GPIO to control LDO*/
HardwareSerial MDMSerial(2); //use ESP32 UART2

ME310 *myME310;
TLT *TLTAccess;
TLTScanner *scannerNetworks;

// Save data variables
String IMEI = "";

char APN[]= "simplio.apn";

void setup() {
	u8x8.begin();
	u8x8.setFont(u8x8_font_chroma48medium8_r);

	u8x8log.begin(u8x8, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
	u8x8log.setRedrawMode(1); // 0: Update screen with newline, 1: Update screen for every char

	// initialize serial communications and wait for port to open:
	Serial.begin(115200);
	MDMSerial.begin(115200,SERIAL_8N1,16,17); //RXD2:16, TXD2:17
	pinMode(PWR_PIN,OUTPUT);
	digitalWrite(PWR_PIN,HIGH);
	delay(100);

	myME310 = new ME310(MDMSerial);
	myME310->debugMode(false);
	myME310->powerOn(ON_OFF);

	TLTAccess = new TLT(myME310, true);     // include a 'true' parameter to enable debugging
	scannerNetworks = new TLTScanner(myME310);
	Serial.println("LTE Cat.M1 networks scanner");
	u8x8log.print("LTE Cat.M1 networks scanner\n");
	scannerNetworks->begin();

	// connection state
	boolean connected = false;

	// Start module
	// If your SIM has PIN, pass it as a parameter of begin() in quotes
	while (!connected)
	{
		if (TLTAccess->begin(NULL, APN, true) == READY)
		{
			connected = true;
		}
		else
		{
			Serial.println("Not connected");
			u8x8log.print("Not connected\n");
			delay(1000);
		}
	}

	// get modem parameters
	// IMEI, modem unique identifier
	Serial.print("Modem IMEI: ");
	u8x8log.print("Modem IMEI: ");
	IMEI = TLTAccess->getIMEI();
	Serial.println(IMEI.c_str());
	u8x8log.print(IMEI.c_str());
	u8x8log.print("\n");
}

void loop() {
	String ret;

	// currently connected carrier
	Serial.print("Current carrier: ");
	u8x8log.print("Current carrier: ");
	ret = scannerNetworks->getCurrentCarrier();
	Serial.println(ret);
	u8x8log.print(ret);
	u8x8log.print("\n");

	// returns strength and BER
	// signal strength in 0-31 scale. 31 means power > 51dBm
	// BER is the Bit Error Rate. 0-7 scale. 99=not detectable
	Serial.print("Signal Strength: ");
	u8x8log.print("Signal Strength: ");
	ret = scannerNetworks->getSignalStrength();
	Serial.print(ret);
	u8x8log.print(ret);
	Serial.println(" [0-31]");
	u8x8log.print(" [0-31]\n");

	// scan for existing networks, displays a list of networks
	Serial.println("Scanning available networks. This may take some seconds.");
	u8x8log.print("Scanning available networks. This may take some seconds.\n");
	ret = scannerNetworks->readNetworks();
	Serial.println(ret);
	u8x8log.print(ret);
	u8x8log.print("\n");
	// wait ten seconds before scanning again
	delay(10000);
}
