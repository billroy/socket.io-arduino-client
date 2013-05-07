/***

	bitlashsocketio.ino:	Bitlash-Commander Socket.IO Client for Arduino/Bitlash

	Copyright (C) 2013 Bill Roy

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

***/
#include "SocketIOClient.h"
#include "Ethernet.h"
#include "SPI.h"
#include "bitlash.h"

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char hostname[] = "192.168.0.2";
char path[] = "/socket.io/1";
int port = 3000;

SocketIOClient client;

// bitlash output handler: forward output to websocket
void senddata(byte c) {
	char buf[] = {(char)c, 0};
	Serial.print((char) c);
	if (client.connected()) client.send(buf);
}

// bitlash command input over the websocket
void ondata(SocketIOClient client, char *data) {
	Serial.print(data);
	doCommand(data);
}

void setup() {
	initBitlash(57600);
	setOutputHandler(&senddata);

	Ethernet.begin(mac);

	client.setDataArrivedDelegate(ondata);
	client.connect(hostname, path, port);
	if (!client.connected()) Serial.println(F("Not connected."));

	client.send("{cmd: \"Hello, world!\"}");
}

void loop() {
  client.monitor();
  runBitlash();
}
