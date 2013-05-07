/***
	bitlashsocketio.ino: Bitlash-Commander Socket.IO Client for Arduino/Bitlash

	Copyright (C) 2013 Bill Roy
	MIT license: see LICENSE file.

	This sketch listens for Bitlash commands from the companion socket.io
	server you will find in the file index.js in the same directory.
	
	Run the server, then boot up the Arduino with this sketch on it.	
	Commands you type on the server console are executed on the Arduino, 
	and the resulting Bitlash output will be displayed on the server console.

***/
#include "SocketIOClient.h"
#include "Ethernet.h"
#include "SPI.h"
#include "bitlash.h"
SocketIOClient client;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char hostname[] = "192.168.0.2";
int port = 3000;

// bitlash serial output handler: forward output to websocket
void sendchar(byte c) {
	Serial.print((char) c);
	char buf[] = {(char)c, 0};
	if (client.connected()) client.send(buf);
}

// websocket message handler: do something with command from server
void ondata(SocketIOClient client, char *data) {
	Serial.print(data);
	doCommand(data);	// ask Bitlash to execute the command
}

void setup() {
	Ethernet.begin(mac);

	client.setDataArrivedDelegate(ondata);
	if (!client.connect(hostname, port)) Serial.println(F("Not connected."));

	setOutputHandler(&sendchar);
	initBitlash(57600);

	client.send("{cmd: \"Hello, world!\"}");
}

void loop() {
  client.monitor();
  runBitlash();
}
