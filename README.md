# TLT Library for Arduino

This library forked the Telit arduino TLT Library([Telit arduino TLT Library](https://github.com/telit/arduino-tlt-library)) and ported it to work with the CodeZoo ME310G1 Modem. 
We modified the library and examples to work with Arduino UNO R4 and excluded examples that don't work due to different hardware environments from the Charlie board. 

It is a loose porting effort of https://github.com/arduino-libraries/MKRNB Arduino library, providing the same interfaces when possible.

## Download and install the ME310 Library

Download the ME310 Arduino library from https://github.com/codezoo-ltd/arduino-ME310G1-library, and place the folder in your Arduino libraries folder, or install the new library from the ZIP file.

## Contents

This Library will simplify the interactions with the ME310G1 Module.

### Classes

The library provides the following classes:

 - **TLT**:  _modem related operations (turn off, check status, enable connectivity, etc. )_
 - **TLTSMS**: _helper for SMS operations_
 - **GPRS**: _GPRS attach utilities_
 - **TLTClient**: _Client to exchange data over TCP/IP_
 - **TLTScanner**: _Utilities to analyze the cellular network such as carrier info, signal strength, etc._
 - **TLTPIN**: _Utilities for the SIM PIN management_
 - **TLTSSLClient**: _TLS/SSL client to exchange data in secure mode_
 - **TLTUDP**: _UDP client utilities_
 - **TLTFileUtils**: _Modem filesystem management_
 - **TLTGNSS**: _GNSS configuration and data management/conversion_


### Examples

The following examples are available:

 - **[ScanNetworks_example](examples/ScanNetworks_example/ScanNetworks_example.ino)** : _Scan nearby network cells and provide info_
 - **[TLTGNSS_example](examples/TLTGNSS_example/TLTGNSS_example.ino)** : _Configure the module in GNSS priority and then waits a fix, printing the retrieved coordinates (in decimal and DMS formats)_
 - **[UDPNtpClient_example](examples/UDPNtpClient_example/UDPNtpClient_example.ino)** : _UDP client used to retrieve NTP time_

## Support

If you need support, please open a ticket to our technical support by sending an email to:

 - rooney.jang@codezoo.co.kr

providing the following information:

 - module type
 - answer to the commands (you can use the ME310 library TrasparentBridge example to communicate with the modem)
   - AT#SWPKGV
   - AT+CPIN?
   - AT+CCID
   - AT+CGSN
   - AT+CGDCONT?
