#! /usr/bin/env node
//////////
//
//	index.js: Bitlash WebSocket Client test harness
//
//	Copyright 2013-2016 Bill Roy (MIT License; see LICENSE file)
//
//
var opt = require('optimist');
var url = require('url');
var argv = opt.usage('Usage: $0 [flags]')
	.alias('p', 'port')
	.describe('p', 'TCP port for the http server (3000)')
	.alias('d', 'debug')
	.describe('d', 'Enable debug output')
	.alias('l', 'log')
	.describe('l', 'Log arduino output to file')
	.argv;

if (argv.help) {
	opt.showHelp();
	process.exit();
}

var port = argv.port || 3000;
var log_file = "datalog.txt";

console.log('WebSocket Commander here!', argv);


//////////
//
//	Configure HTTP server
//
var express = require('express');
var app = express();
var http = require('http');
var server = http.createServer(app);
var listener = server.listen(port);
var io = require('socket.io').listen(listener);
io.set('transports', ['websocket']);
app.use('/', express.static(__dirname + '/public'));
console.log('Listening on port:', port);


//////////
//
//	Socket.io startup
//
var output_socket;
var fs = require('fs');

io.sockets.on('connection', function (socket) {
    output_socket = socket;		// save global socket handle
	console.log('Client connected...');
    socket.emit('tty', 'Connected to server at ' + new Date().toString() + '\n');
	socket.on('tty', function(data) {
		process.stdout.write(data + '\n');
		//socket.emit('tty', data);
		if (argv.log) {
			fs.appendFile(log_file, data, function (err) {
				if (err) console.log('LOG FILE WRITE ERROR:', data);
				else {}
			});
		}
	});
	//socket.emit('ls');
});

setInterval(function() {
	console.log('Sending tty ping...');
	if (output_socket) output_socket.emit('tty', new Date().toString() + '\n');
}, 30000);


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
		output_socket.emit('tty', cmd + '\n');
		cmd = '';
	}
	else {
		process.stdout.write(key);
		cmd = cmd + key;
	}
});
