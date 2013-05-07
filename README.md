# SocketIO Arduino Client, an Arduino client for connecting and messaging with Socket.io servers

Based on Kevin Rohling's arduino websocket client modified to work with socket.io servers.  Along the way, all uses of the String class were replaced with fixed char buffers so as to use less memory.

The bitlashsocketio.ino example provides an integration with Bitlash on the Arduino and a node.js example server that can be used to send Bitlash commands over the Websocket fabric to the Arduino, and see its output in reply.

Kevin's documentation is reproduced hereinafter, with changes as needed.


## Caveats

This library doesn't support every inch of the Websocket spec, most notably the use of a Sec-Websocket-Key.  Also, because Arduino doesn't support SSL, this library also doesn't support the use of Websockets over https.  If you're interested in learning more about the Websocket spec I recommend checking out the [Wikipedia Page](http://en.wikipedia.org/wiki/WebSocket).  Now that I've got that out of the way, I've been able to successfully use this to connect to several hosted Websocket services, including: [echo.websocket.org](http://websocket.org/echo.html) and [pusherapp.com](http://pusherapp.com).

## Installation instructions

Clone this repo into your Arduino Sketchbook directory under libraries, then restart the Arduino IDE so that it notices the new library.  Now, under File\Examples you should see SocketIOClient.  

## How To Use This Library

```
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "echo.websocket.org";
WebSocketClient client;

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac);
  client.connect(server);
  client.setDataArrivedDelegate(dataArrived);
  client.send("Hello World!");
}

void loop() {
  client.monitor();
}

void dataArrived(WebSocketClient client, char *data) {
  Serial.println("Data Arrived: " + data);
}
```

## Examples

There example included with this library, EchoExample, will connect to echo.websocket.org, which hosts a service that simply echos any messages that you send it via Websocket.  This example sends the message "Hello World!".  If the example runs correctly, the Arduino will receive this same message back over the Websocket and print it via Serial.println.