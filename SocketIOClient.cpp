/*
	socket.io-arduino-client: a Socket.IO client for the Arduino

	Based on the Kevin Rohling WebSocketClient

	Copyright 2013 Bill Roy

	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:
	
	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
*/
#include <SocketIOClient.h>

bool SocketIOClient::connect(char thehostname[], int theport) {
	if (!client.connect(thehostname, theport)) return false;
	hostname = thehostname;
	port = theport;
	sendHandshake(hostname);
	return readHandshake();
}

bool SocketIOClient::connected() {
	return client.connected();
}

void SocketIOClient::disconnect() {
	client.stop();
}

// find the nth colon starting from dataptr
void SocketIOClient::findColon(char which) {	
	while (*dataptr) {
		if (*dataptr == ':') {
			if (--which <= 0) return;
		}
		++dataptr;
	}
}

// terminate command at dataptr at closing double quote
void SocketIOClient::terminateCommand(void) {
	dataptr[strlen(dataptr)-3] = 0;
}

void SocketIOClient::monitor() {

	*databuffer = 0;

	if (!client.connected()) {
		if (!client.connect(hostname, port)) return;
	}

	if (!client.available()) return;

	char which;
	while (client.available()) {
		readLine();
		dataptr = databuffer;
		switch (databuffer[0]) {	

		case '1':		// connect: []
			which = 6;
			break;

		case '2':		// heartbeat: [2::]
			client.print((char)0);
			client.print("2::");
			client.print((char)255);
			continue;

		case '5':		// event: [5:::{"name":"ls"}]
			which = 4;
			break;

		default: 
			Serial.print("Drop ");
			Serial.println(dataptr);
			continue;
		}

		findColon(which);
		dataptr += 2;

		// handle backslash-delimited escapes
		char *optr = databuffer;
		while (*dataptr && (*dataptr != '"')) {
			if (*dataptr == '\\') {
				++dataptr;		// todo: this just handles "; handle \r, \n, \t, \xdd
			}
			*optr++ = *dataptr++;
		}
		*optr = 0;

		Serial.print("[");
		Serial.print(databuffer);
		Serial.print("]");

		if (dataArrivedDelegate != NULL) {
			dataArrivedDelegate(*this, databuffer);
		}
	}
}

void SocketIOClient::setDataArrivedDelegate(DataArrivedDelegate newdataArrivedDelegate) {
	  dataArrivedDelegate = newdataArrivedDelegate;
}

void SocketIOClient::sendHandshake(char hostname[]) {
	client.println(F("GET /socket.io/1/ HTTP/1.1"));
	client.print(F("Host: "));
	client.println(hostname);
	client.println(F("Origin: Arduino\r\n"));
}

bool SocketIOClient::waitForInput(void) {
unsigned long now = millis();
	while (!client.available() && ((millis() - now) < 30000UL)) {;}
	return client.available();
}

void SocketIOClient::eatHeader(void) {
	while (client.available()) {	// consume the header
		readLine();
		if (strlen(databuffer) == 0) break;
	}
}

bool SocketIOClient::readHandshake() {

	if (!waitForInput()) return false;

	// check for happy "HTTP/1.1 200" response
	readLine();
	if (atoi(&databuffer[9]) != 200) {
		while (client.available()) readLine();
		client.stop();
		return false;
	}
	eatHeader();
	readLine();	// read first line of response
	readLine();	// read sid : transport : timeout

	char *iptr = databuffer;
	char *optr = sid;
	while (*iptr && (*iptr != ':') && (optr < &sid[SID_LEN-2])) *optr++ = *iptr++;
	*optr = 0;

	Serial.print(F("Connected. SID="));
	Serial.println(sid);	// sid:transport:timeout 

	while (client.available()) readLine();
	client.stop();
	delay(1000);

	// reconnect on websocket connection
	Serial.print(F("WS Connect..."));
	if (!client.connect(hostname, port)) {
		Serial.print(F("Reconnect failed."));
		return false;
	}
	Serial.println(F("Reconnected."));

	client.print(F("GET /socket.io/1/websocket/"));
	client.print(sid);
	client.println(F(" HTTP/1.1"));
	client.print(F("Host: "));
	client.println(hostname);
	client.println(F("Origin: ArduinoSocketIOClient"));
	client.println(F("Upgrade: WebSocket"));	// must be camelcase ?!
	client.println(F("Connection: Upgrade\r\n"));

	if (!waitForInput()) return false;

	readLine();
	if (atoi(&databuffer[9]) != 101) {
		while (client.available()) readLine();
		client.stop();
		return false;
	}
	eatHeader();
	monitor();		// treat the response as input
	return true;
}

void SocketIOClient::readLine() {
	dataptr = databuffer;
	while (client.available() && (dataptr < &databuffer[DATA_BUFFER_LEN-2])) {
		char c = client.read();
		//Serial.print(c);
		if (c == 0) Serial.print(F("NULL"));
		else if (c == 255) Serial.print(F("0x255"));
		else if (c == '\r') {;}
		else if (c == '\n') break;
		else *dataptr++ = c;
	}
	*dataptr = 0;
}

void SocketIOClient::send(char *data) {
	client.print((char)0);
	client.print("3:::");
	client.print(data);
	client.print((char)255);
}
