/***
	hello.ino: Hello World Socket.IO Client for Arduino/Bitlash

	Copyright (C) 2013 Bill Roy
	MIT license: see LICENSE file.

	This sketch listens for a Bitlash command from a socket.io server and
	executes the command, returning its output to the server over the websocket.
	
	For testing, you will find a companion socket.io server in the file 
	index.js in the same directory.

	Run the server ("node index.js"), then boot up the Arduino with this sketch on it.	
	Commands you type on the server console will be executed on the Arduino, 
	and the resulting Bitlash output will be displayed on the server console.

	You will need to adjust the hostname and port below to match your network.
	By default the server runs on port 3000.

***/
#include "SocketIOClient.h"
#include "Ethernet.h"
#include "SPI.h"
#include "bitlash.h"
SocketIOClient client;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char hostname[] = "192.168.0.2";
int port = 3000;


// websocket message handler: do something with command from server
void ondata(SocketIOClient client, char *data) {
	Serial.print(data);
}

void setup() {
	Serial.begin(57600);

	Ethernet.begin(mac);

	client.setDataArrivedDelegate(ondata);
	if (!client.connect(hostname, port)) Serial.println(F("Not connected."));

	if (client.connected()) client.send("Client here!");
}

#define HELLO_INTERVAL 3000UL
unsigned long lasthello;

void loop() {
	client.monitor();

	unsigned long now = millis();
	if ((now - lasthello) >= HELLO_INTERVAL) {
		lasthello = now;
		if (client.connected()) client.send("Hello, world!\n");
	}
}
