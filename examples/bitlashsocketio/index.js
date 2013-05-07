#! /usr/bin/env node
//////////
//
//	index.js: Bitlash WebSocket Client test harness
//
//	Copyright 2013 Bill Roy (MIT License; see LICENSE file)
//
//
var opt = require('optimist');
var url = require('url');
var argv = opt.usage('Usage: $0 [flags]')
	.alias('p', 'port')
	.describe('p', 'TCP port for the http server (3000)')
	.alias('d', 'debug')
	.describe('d', 'Enable debug output')
	.argv;

if (argv.help) {
	opt.showHelp();
	process.exit();
} 
var port = argv.port || 3000;

console.log('WebSocket Commander here!', argv);

//////////
//
//	Configure HTTP server
//
var express = require('express');
var app = express();
var http = require('http');
var server = http.createServer(app);
var io = require('socket.io').listen(server);
io.set('transports', ['websocket']);    
io.set('log level', argv.debug ? 3 : 1);

app.configure(function () {
	app.use(express.logger());
	app.use(express.bodyParser());
	app.use(express.static(__dirname + '/public'));
});

app.get('/', function(req, res) {
	res.send("WebSocket Client Test")
});

server.listen(port);
console.log('Listening on port:', port);


//////////
//
//	Socket.io startup
//
var output_socket;
io.sockets.on('connection', function (socket) {
	console.log('Client connected via', socket.transport);
	socket.on('message', function(data) {
		process.stdout.write(data);
	});
	socket.emit("ls");
	output_socket = socket;		// save global ugh
});


//////////
//
//	Keyboard handling
//
var cmd = '';
process.stdin.setRawMode(true);
process.stdin.resume();
process.stdin.setEncoding('utf8');
process.stdin.on('data', function (key) {
	if ((key === '\u0003') || (key == 'q')) process.exit();
	else if ((key === '\r') || (key == '\n')) {
		process.stdout.write('\r\n');
		output_socket.emit(cmd);
		cmd = '';
	}
	else {
		process.stdout.write(key);
		cmd = cmd + key;
	}
});
