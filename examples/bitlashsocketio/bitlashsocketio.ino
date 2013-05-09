/***
	bitlashsocketio.ino: Bitlash-Commander Socket.IO Client for Arduino/Bitlash

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

// buffer Bitlash's char-based output to cut down on sent packet count
#define OUTPUT_BUFFER_LEN 33
byte output_index = 0;
char output_buffer[OUTPUT_BUFFER_LEN];

// transmit the output buffer
void sendbuffer(void) {
	if (output_index == 0) return;
	output_buffer[output_index] = 0;
	Serial.print(output_buffer);
	if (client.connected()) client.send(output_buffer);
	output_index = 0;
}

// bitlash serial output handler: 
// 	buffer an output character for forwarding to websocket
void sendchar(byte c) {
	output_buffer[output_index++] = c;
	if (output_index >= OUTPUT_BUFFER_LEN-1) sendbuffer();
}

// websocket message handler: do something with command from server
void ondata(SocketIOClient client, char *data) {
	Serial.print(data);
	doCommand(data);	// ask Bitlash to execute the command
	sendbuffer();
}

void setup() {
	setOutputHandler(&sendchar);	// make Bitlash use sendchar() to print
	initBitlash(57600);				// and initialize Bitlash

	Ethernet.begin(mac);

	client.setDataArrivedDelegate(ondata);
	if (!client.connect(hostname, port)) Serial.println(F("Not connected."));

	client.send("Client here!");
}

void loop() {
  client.monitor();
  runBitlash();
  sendbuffer();
}
