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

#define MDMSerial Serial1
#define ON_OFF 2
#define APN "simplio.apn"

ME310* _me310 = new ME310();
ME310::return_t rc;  //Enum of return value  methods

int cID = 1;           //PDP Context Identifier
int connID = 1;        //Socket connection identifier.
char ipProt[] = "IP";  //Packet Data Protocol type
char* resp;

char server[] = "api.thingspeak.com";
int port = 80;

String WApiKey = "****************";  // Thing Speak Write API Key 16Character
String fieldLati = "field1";
String fieldLongi = "field2";
char recvBuffer[700];

/*When NMEA_DEBUG is false Unsolicited NMEA is disabled*/
/*NMEA is true*/
TLTGNSS gnss(_me310, true);

void setup() {
	Serial.begin(115200);
	MDMSerial.begin(115200);
	delay(100);
	Serial.println("TLT GNSS example, enabling ME310 module");
	_me310->debugMode(false);
	_me310->powerOn(ON_OFF);
	_me310->module_reboot();

	_me310->read_enter_pin();  //issue command AT+pin? in read mode, check that the SIM is inserted and the module is not waiting for the PIN
	resp = (char*)_me310->buffer_cstr(2);

	if (resp != NULL) {
		if (strcmp(resp, "OK") == 0)  // read response in 2 array position
		{
			Serial.println("Define PDP Context");
			rc = _me310->define_pdp_context(cID, ipProt, APN);  //issue command AT+CGDCONT=cid,PDP_type,APN
			if (rc == ME310::RETURN_VALID) {
				_me310->read_define_pdp_context();  //issue command AT+CGDCONT=? (read mode)
				Serial.print("pdp context read: ");
				Serial.println(_me310->buffer_cstr(1));  //print second line of modem answer
			}
		}
	}
}

void loop() {

	bool isFix = false;
	float lat = 0.0;
	float lng = 0.0;


	Serial.print("Initializing GNSS");
	while (!gnss.setGNSSConfiguration()) {
		Serial.print(".");
	}
	Serial.println(" is completed successfully");

	while (!isFix) {
		GNSSInfo gnssInfo = gnss.getGNSSData();
		/*gnssInfo fields will have latitude, longitude and the other details in string format*/

		/*Fix 1.2 or 1.3 means valid fix*/
		if (gnssInfo.fix.toFloat() > 1.0) {
			isFix = true;
			Serial.println("");
			Serial.print("Fix valid, converting...");
			if (gnss.convertNMEA2Decimal(gnssInfo.latitude, gnssInfo.longitude, &lat, &lng)) {

				Serial.println("Conversion done!");
				Serial.println(lat, 6);
				Serial.println(lng, 6);
			} else {
				Serial.println("Conversion failed!");
			}
		} else {
			Serial.println("Fix not valid yet.");
			Serial.print("Fix value: ");
			Serial.println(gnssInfo.fix.toFloat());
		}
		delay(1000);
	}
	isFix = false;
	Serial.print("GNSS Off");
	while (!gnss.unsetGNSSConfiguration()) {
		Serial.print(".");
	}
	Serial.println(" OK!");
	Serial.print("gprs network registration status: ");

	rc = _me310->read_gprs_network_registration_status();  //issue command AT+CGREG=? (read mode)
	if (rc == ME310::RETURN_VALID) {
		resp = (char*)_me310->buffer_cstr(1);
		Serial.println(resp);
		while (resp != NULL) {
			if ((strcmp(resp, "+CGREG: 0,1") != 0) && (strcmp(resp, "+CGREG: 0,5") != 0)) {
				delay(3000);
				rc = _me310->read_gprs_network_registration_status();
				if (rc != ME310::RETURN_VALID) {
					Serial.println("ERROR");
					Serial.println(_me310->return_string(rc));
					break;
				}
				Serial.println(_me310->buffer_cstr(1));
				resp = (char*)_me310->buffer_cstr(1);
			} else {
				break;
			}
		}
	}
	Serial.println("Activate context");
	_me310->context_activation(cID, 1);  //issue command AT#SGACT=cid,state and wait for answer or timeout
	Serial.print("Socket configuration: ");
	rc = _me310->socket_configuration(connID, cID);  //issue command AT#SCFG=connID,cID and wait for answer or timeout
	Serial.println(_me310->return_string(rc));       //returns a string with return_t codes
	if (rc == ME310::RETURN_VALID) {
		Serial.print("Socket dial: ");
		rc = _me310->socket_dial(connID, 0, port, server, 0, 0, 1, 0, 0, ME310::TOUT_1MIN);  // issue commandAT#SD=connID,protocol,port,IPAddrServer,timeout
		Serial.println(_me310->return_string(rc));                                           //returns a string with return_t codes
		if (rc == ME310::RETURN_VALID) {
			delay(100);
			Serial.print("Socket Status: ");
			rc = _me310->socket_status(connID, ME310::TOUT_10SEC);  //issue command AT#SS=connID and wait for answer or timeout
			delay(100);
			Serial.println(_me310->return_string(rc));

			char latBuff[10];
			char longBuff[10];

			dtostrf(lat, 4, 6, latBuff);
			dtostrf(lng, 4, 6, longBuff);

			String data = "GET /update?api_key=" + WApiKey + "&" + fieldLati + "=" + String(latBuff) + "&" + fieldLongi + "=" + String(longBuff) + " HTTP/1.1\r\n";
			data += "Host: api.thingspeak.com\r\n";
			data += "Connection: close\r\n\r\n";

			Serial.print("SEND: ");
			rc = _me310->socket_send_data_command_mode_extended(connID, (int)strlen(data.c_str()) + 1, (char*)data.c_str(), 1, ME310::TOUT_30SEC);  //include the NULL character
			Serial.println(_me310->return_string(rc));
			if (rc != ME310::RETURN_VALID) {
				Serial.println("Send is failed");
			} else {
				Serial.print("Socket Listen: ");
				rc = _me310->socket_listen(connID, 0, port);  //issue command AT#SL=connID,listenState(0 close socket listening),port and wait for answer or timeout
				Serial.println(_me310->return_string(rc));
				delay(5000);
				if (rc == ME310::RETURN_VALID) {
					Serial.print("READ: ");
					rc = _me310->socket_receive_data_command_mode(connID, (int)sizeof(recvBuffer), 0, ME310::TOUT_10SEC);  //issue command AT#SRECV=connID,size and wait for answer or timeout
					Serial.println(_me310->return_string(rc));
					if (rc == ME310::RETURN_VALID) {
						Serial.print("Payload: <");
						Serial.print((String)_me310->buffer_cstr_raw());  //print modem answer in raw mode
						Serial.println(">");
					} else {
						Serial.println(_me310->return_string(rc));
					}
				} else {
					Serial.println(_me310->return_string(rc));
				}
				Serial.println("socket disconnect");
				_me310->socket_shutdown(connID, ME310::TOUT_1MIN);
			}
		} else {
			Serial.println(_me310->return_string(rc));
		}
	}
	Serial.println("DeActivate context");
	_me310->context_activation(cID, 0);  //issue command AT#SGACT=cid,state and wait for answer or timeout
	delay(60000);                        /* every 1min */
}
