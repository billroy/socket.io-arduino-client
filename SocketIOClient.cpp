/*
	git@github.com:billroy/ArduinoSocketIOClient.git

	Forked by Bill Roy from the Kevin Rohling version
	Changes released under the MIT license below.

	5/6/13 Hacked to use char* and a fixed data buffer instead of the String class -br
*/

/*
 SocketIOClient, a websocket client for Arduino
 Copyright 2011 Kevin Rohling
 http://kevinrohling.com
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include <SocketIOClient.h>
#include <WString.h>
#include <string.h>
#include <stdlib.h>

bool SocketIOClient::connect(char hostname[], char path[], int port) {
	if (!_client.connect(hostname, port)) return false;
	_hostname = hostname;
	_path = path;
	_port = port;
	sendHandshake(hostname, path);
	return readHandshake();
}

bool SocketIOClient::connected() {
	return _client.connected();
}

void SocketIOClient::disconnect() {
	_client.stop();
}

// find the nth colon starting from _dataptr
void SocketIOClient::findColon(char which) {	
	while (*_dataptr) {
		if (*_dataptr == ':') {
			if (--which <= 0) return;
		}
		++_dataptr;
	}
}

// terminate command at _dataptr at closing double quote
void SocketIOClient::terminateCommand(void) {
	_dataptr[strlen(_dataptr)-3] = 0;
}


void SocketIOClient::monitor() {

	*_databuffer = 0;
	if (!_client.available()) return;

	char which;
	while (_client.available()) {
		_readLine();
		_dataptr = _databuffer;
		switch (_databuffer[0]) {	

		case '1':
			which = 6;
			break;

		case '5':		// 5:::{"name":"ls"}
			which = 4;
			break;

		default: 
			Serial.print("Drop ");
			Serial.println(_dataptr);
			continue;
		}

		findColon(which);
		_dataptr += 2;
		terminateCommand();
Serial.print("[");
Serial.print(_dataptr);
Serial.print("]");

		if (_dataArrivedDelegate != NULL) {
			_dataArrivedDelegate(*this, _dataptr);
		}
	}
}

void SocketIOClient::setDataArrivedDelegate(DataArrivedDelegate dataArrivedDelegate) {
	  _dataArrivedDelegate = dataArrivedDelegate;
}

void SocketIOClient::sendHandshake(char hostname[], char path[]) {
	_client.println(F("GET /socket.io/1/ HTTP/1.1"));
	_client.print(F("Host: "));
	_client.println(hostname);
	_client.println(F("Origin: ArduinoSocketIOClient\r\n"));
}

bool SocketIOClient::waitForInput(void) {
unsigned long now = millis();
	while (!_client.available() && ((millis() - now) < 30000UL)) {;}
	return _client.available();
}

void SocketIOClient::eatHeader(void) {
	while (_client.available()) {	// consume the header
		_readLine();
		if (strlen(_databuffer) == 0) break;
	}
}

bool SocketIOClient::readHandshake() {

	if (!waitForInput()) return false;

	// check for happy "HTTP/1.1 200" response
	_readLine();
	if (atoi(&_databuffer[9]) != 200) {
		while (_client.available()) _readLine();
		_client.stop();
		return false;
	}
	eatHeader();
	_readLine();	// read first line of response
	_readLine();	// read sid : transport : timeout

	char *iptr = _databuffer;
	char *optr = _sid;
	while (*iptr && (*iptr != ':') && (optr < &_sid[SID_LEN-2])) *optr++ = *iptr++;
	*optr = 0;

	Serial.print(F("Connected. SID="));
	Serial.println(_sid);	// sid:transport:timeout 

	while (_client.available()) _readLine();
	_client.stop();
	delay(1000);

	// reconnect on websocket connection
	Serial.print(F("WS Connect..."));
	if (!_client.connect(_hostname, _port)) {
		Serial.print(F("Reconnect failed."));
		return false;
	}
	Serial.println(F("Reconnected."));

	_client.print(F("GET /socket.io/1/websocket/"));
	_client.print(_sid);
	_client.println(F(" HTTP/1.1"));
	_client.print(F("Host: "));
	_client.println(_hostname);
	_client.println(F("Origin: ArduinoSocketIOClient"));
	_client.println(F("Upgrade: WebSocket"));	// must be camelcase ?!
	_client.println(F("Connection: Upgrade\r\n"));

	if (!waitForInput()) return false;

	_readLine();
	if (atoi(&_databuffer[9]) != 101) {
		while (_client.available()) _readLine();
		_client.stop();
		return false;
	}
	eatHeader();
	monitor();		// treat the response as input
	return true;
}


void SocketIOClient::_readLine() {
	_dataptr = _databuffer;
	while (_client.available() && (_dataptr < &_databuffer[DATA_BUFFER_LEN-2])) {
		char c = _client.read();
		//Serial.print(c);
		if (c == 0) Serial.print(F("NULL"));
		else if (c == 255) Serial.print(F("0x255"));
		else if (c == '\r') {;}
		else if (c == '\n') break;
		else *_dataptr++ = c;
	}
	*_dataptr = 0;
}


void SocketIOClient::send (char *data) {
		_client.print((char)0);
	_client.print("3:::");
	_client.print(data);
	_client.print((char)255);
}
