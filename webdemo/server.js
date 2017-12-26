'use strict';

var express = require('express');
var http = require('http');
var fs = require('fs');
const { spawn } = require('child_process');

var expressWebServer = express();

expressWebServer.set('view engine', 'jade');
expressWebServer.set('views', './views');
expressWebServer.use('/static/', express.static('./static'));

http.createServer(expressWebServer);

expressWebServer.listen(3010, function() {
console.log('SchemeToJS Web App listening on port 3010');
});

expressWebServer.get('/', function(req, res) {

res.render('index', {});

});

var server = http.createServer(expressWebServer);
var io = require('socket.io')(server);
server.listen(3100);


// When there is a new connection we open a socket as well.
io.on('connection', function(socket) {

  var initScheme = fs.readFileSync('./example.scm', 'utf8');
  socket.emit('setScheme', initScheme);

  // Handle requests from the browser.
  socket.on('schemeToJS', function(scm) {

    const child = spawn('../bin/scm2js');
    child.on('exit', function (code, signal) {
      console.log('child process exited with ' +
                  `code ${code} and signal ${signal}`);
    });

    child.stdout.on('data', (data) => {
      socket.emit('setJS', data.toString('utf8'));
    });
    child.stdin.write(scm);
    child.stdin.end();
    
  });
});
