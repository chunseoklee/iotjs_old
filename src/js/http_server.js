/* Copyright 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

var EventEmitter = require('events');
var util = require('util');
var net = require('net');
var HTTPParser = process.binding(process.binding.httpparser).HTTPParser;
var IncomingMessage = require('http_incoming').IncomingMessage;
var OutgoingMessage = require('http_outgoing').OutgoingMessage;


var STATUS_CODES = exports.STATUS_CODES = {
  100 : 'Continue',
  101 : 'Switching Protocols',
  102 : 'Processing',                 // RFC 2518, obsoleted by RFC 4918
  200 : 'OK',
  201 : 'Created',
  202 : 'Accepted',
  203 : 'Non-Authoritative Information',
  204 : 'No Content',
  205 : 'Reset Content',
  206 : 'Partial Content',
  207 : 'Multi-Status',               // RFC 4918
  300 : 'Multiple Choices',
  301 : 'Moved Permanently',
  302 : 'Moved Temporarily',
  303 : 'See Other',
  304 : 'Not Modified',
  305 : 'Use Proxy',
  307 : 'Temporary Redirect',
  308 : 'Permanent Redirect',         // RFC 7238
  400 : 'Bad Request',
  401 : 'Unauthorized',
  402 : 'Payment Required',
  403 : 'Forbidden',
  404 : 'Not Found',
  405 : 'Method Not Allowed',
  406 : 'Not Acceptable',
  407 : 'Proxy Authentication Required',
  408 : 'Request Time-out',
  409 : 'Conflict',
  410 : 'Gone',
  411 : 'Length Required',
  412 : 'Precondition Failed',
  413 : 'Request Entity Too Large',
  414 : 'Request-URI Too Large',
  415 : 'Unsupported Media Type',
  416 : 'Requested Range Not Satisfiable',
  417 : 'Expectation Failed',
  418 : 'I\'m a teapot',              // RFC 2324
  422 : 'Unprocessable Entity',       // RFC 4918
  423 : 'Locked',                     // RFC 4918
  424 : 'Failed Dependency',          // RFC 4918
  425 : 'Unordered Collection',       // RFC 4918
  426 : 'Upgrade Required',           // RFC 2817
  428 : 'Precondition Required',      // RFC 6585
  429 : 'Too Many Requests',          // RFC 6585
  431 : 'Request Header Fields Too Large',// RFC 6585
  500 : 'Internal Server Error',
  501 : 'Not Implemented',
  502 : 'Bad Gateway',
  503 : 'Service Unavailable',
  504 : 'Gateway Time-out',
  505 : 'HTTP Version Not Supported',
  506 : 'Variant Also Negotiates',    // RFC 2295
  507 : 'Insufficient Storage',       // RFC 4918
  509 : 'Bandwidth Limit Exceeded',
  510 : 'Not Extended',               // RFC 2774
  511 : 'Network Authentication Required' // RFC 6585
};



function Server(requestListener) {
  if (!(this instanceof Server)) {
    return new Server(requestListener);
  }

  net.Server.call(this);

  if (util.isFunction(requestListener)) {
    this.addListener('request', requestListener);
  }

  this.addListener('connection', connectionListener);

}

util.inherits(Server, net.Server);

Server.prototype.setTimeout = function(msecs, callback) {
  this.timeout = msecs;
  if (callback)
    this.on('timeout', callback);
};

exports.Server = Server;

function connectionListener(socket) {
  var self = this;
  var incoming = [];

  // parser initialize
  var parser = new HTTPParser(HTTPParser.REQUEST);
  // FIXME: This should be impl. with Array
  parser._headers = {};
  parser._url = '';
  // cb during  http_parsering from C side
  parser.OnHeaders = parserOnHeaders;
  parser.OnHeadersComplete = parserOnHeadersComplete;
  parser.OnBody = parserOnBody;
  parser.OnMessageComplete = parserOnMessageComplete;
  parser.onIncoming = parserOnIncoming;

  socket.on("data", socketOnData);
  socket.on("end", socketOnEnd);
  socket.on("error", socketOnError);

  function socketOnData(d) {
    console.log("http_server on socket data");
    console.log("data :" + d.toString());

    var ret = parser.execute(d);

    if (ret instanceof Error) {
      socket.destroy();
    }
    else if (parser.incoming && parser.incoming.upgrade) {
      // Upgrade or CONNECT
      // TODO: fill this part
    }
  }

  function socketOnError(e) {
    self.emit("client error on connection", e, this);
  }

  function socketOnEnd() {
    var socket = this;
    var ret = parser.finish();

    if (ret instanceof Error) {
      socket.destroy();
      return;
    }

    socket.end();

  }

  function parserOnIncoming(req, shouldKeepAlive) {
    incoming.push(req);
    console.log("time to response");
    if (shouldKeepAlive) {
      console.log("wow must keep alive");
    }
    return false;
  }


}


function parserOnMessageComplete() {
}

function parserOnHeadersComplete(info) {
  var parser = this;
  var headers = info.headers;
  var url = info.url;
  if(!url){
    url = parser._url;
    parser.url = "";
  }

  if(!headers){
    headers = parser._headers;
    // FIXME: This should be impl. with Array
    parser._headers = {};
  }

  console.log("=== ===js header complete == \n");
  /*console.log("url = " + url + "\n");
  for(var i=0;i<headers.length;i++){
    console.log(headers[i]);
  }*/

  parser.incoming = new IncomingMessage(parser.socket);
  parser.incoming.url = url;
  parser.incoming.upgrade = info.upgrade;

  var flag_skipbody = false;

  if (!info.upgrade) {
    flag_skipbody = parser.onIncoming(parser.incoming,
                                      info.shouldkeepalive);
  }

  return flag_skipbody;

}

function parserOnBody() {

}

function AddHeader(dest, src) {
  for(var i=0;i<src.length;i++){
    dest[dest.length+i] = src[i];
    }
  dest.length = dest.length + src.length;
}

function parserOnHeaders(headers, url) {
  // FIXME: This should be impl. with Array.concat
  AddHeader(this._headers, headers);
  this._url += url;
}
