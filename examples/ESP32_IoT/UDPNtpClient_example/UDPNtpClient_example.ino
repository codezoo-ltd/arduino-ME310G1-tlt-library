/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
  - UDPNtpClient_example.ino

  @brief
  Udp NTP Client

  @details
  In this example sketch,it gets the time from a Network Time Protocol (NTP) server.\n
  The use of UDP write and parsePacket is demonstrated.\n
  For the sketch to work it is necessary to install the external TimeLib library, available in:
https://www.arduinolibraries.info/libraries/time

@version
1.0.0

@note

@author
Cristina Desogus

@date
25/08/2021

@ported by rooney.jang (rooney.jang@codezoo.co.kr)	
 */


#include <TLTMDM.h>
#include <TimeLib.h>

#include <U8x8lib.h>
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE);
#define U8LOG_WIDTH 16
#define U8LOG_HEIGHT 8
uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];
U8X8LOG u8x8log;

#define ON_OFF 18 /*Select the GPIO to control ON_OFF*/
#define PWR_PIN 5 /*Select the GPIO to control LDO*/
HardwareSerial MDMSerial(2); //use ESP32 UART2

unsigned int localPort = 2500;      // local port to listen for UDP packets

const char timeServer[] = "0.it.pool.ntp.org";
unsigned short hostPort = 123;

const int timeZone = 9;     // Seoul Time (KOR)
							//const int timeZone = 1;   // Central European Time
							//const int timeZone = -5;  // Eastern Standard Time (USA)
							//const int timeZone = -4;  // Eastern Daylight Time (USA)
							//const int timeZone = -8;  // Pacific Standard Time (USA)
							//const int timeZone = -7;  // Pacific Daylight Time (USA)

time_t prevDisplay = 0; // when the digital clock was displayed

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

uint8_t packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// initialize the library instance
ME310 *myME310;
GPRS *gprs;
TLT *tltAccess;
TLTUDP *Udp;

char APN[] = "simplio.apn";

void setup()
{
	u8x8.begin();
	u8x8.setFont(u8x8_font_chroma48medium8_r);

	u8x8log.begin(u8x8, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
	u8x8log.setRedrawMode(1); // 0: Update screen with newline, 1: Update screen for every char

	// Open serial communications and wait for port to open:
	Serial.begin(115200);
	MDMSerial.begin(115200,SERIAL_8N1,16,17); //RXD2:16, TXD2:17
	pinMode(PWR_PIN,OUTPUT);
	digitalWrite(PWR_PIN,HIGH);
	delay(100);

	myME310 = new ME310(MDMSerial);
	myME310->debugMode(false);
	myME310->powerOn(ON_OFF);

	gprs = new GPRS(myME310);
	tltAccess = new TLT(myME310);
	Udp = new TLTUDP(myME310);

	Serial.println(F("Starting Arduino UDP NTP client."));
	u8x8log.print("Starting Arduino UDP NTP client.\n");	
	// connection state
	boolean connected = false;

	// attach the shield to the GPRS network with the APN, login and password
	Serial.print("Begin...");
	u8x8log.print("Begin...");
	while (!connected)
	{
		if ((tltAccess->begin(0, APN, true) == READY) && (gprs->attachGPRS() == GPRS_READY))
		{
			connected = true;
		}
		else
		{
			Serial.print(F("."));
			u8x8log.print(".");
			delay(1000);
		}
	}

	Serial.println(F("Starting connection to server..."));
	u8x8log.print("Starting connection to server...\n");
	Udp->begin(localPort);
	Serial.println(F("Waiting for sync..."));
	u8x8log.print("Waiting for sync...\n");
	setSyncProvider(getNtpTime);
}

void loop()
{
	if (timeStatus() != timeNotSet)
	{
		if (now() != prevDisplay)
		{ //update the display only if time has changed
			prevDisplay = now();
			digitalClockDisplay();
		}
	}

	while(1); //end process
}

void digitalClockDisplay()
{
	String ret;
	// digital clock display of the time
	ret = year();
	Serial.print(ret);
	u8x8log.print(ret);
	Serial.print("/");
	u8x8log.print("/");
	ret = month();
	Serial.print(ret);
	u8x8log.print(ret);
	Serial.print("/");
	u8x8log.print("/");
	ret = day();
	Serial.print(ret);
	u8x8log.print(ret);
	Serial.print(" ");
	u8x8log.print(" ");
	ret = hour();
	Serial.print(ret);
	u8x8log.print(ret);
	printDigits(minute());
	printDigits(second());
	Serial.println();
	u8x8log.print("\n");
}

void printDigits(int digits)
{
	// utility for digital clock display: prints preceding colon and leading 0
	Serial.print(":");
	u8x8log.print(":");
	if(digits < 10)
	{
		Serial.print('0');
		u8x8log.print('0');
	}
	Serial.print(digits);
	u8x8log.print(digits);
}

time_t getNtpTime()
{
	while (Udp->parsePacket() > 0) ; // discard any previously received packets
	Serial.println("Transmit NTP Request.");
	u8x8log.print("Transmit NTP Request.\n");
	sendNTPpacket(timeServer);
	uint32_t beginWait = millis();
	while (millis() - beginWait < 1500) {
		int size = Udp->parsePacket();
		if (size >= NTP_PACKET_SIZE) {
			Serial.println("Receive NTP Response.");
			u8x8log.print("Receive NTP Response.\n");
			Udp->read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];
			return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
		}
	}
	Serial.println("No NTP Response");
	u8x8log.print("No NTP Response\n");
	return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(const char* address)
{

	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
							 // 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;

	//memcpy(packetBuffer, "Hello World", 11);

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	Udp->beginPacket(address, hostPort); //NTP requests are to port 123
	Udp->write(packetBuffer, NTP_PACKET_SIZE);
	return Udp->endPacket();
}
