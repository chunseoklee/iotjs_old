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


var util = require('util');
var stream = require('stream');

function IncomingMessage(socket) {
  stream.ReadableStream.call(this);

  this.socket = socket;
  this.connection = socket;

  this.readable = true;

  this.headers = {};

  this.upgrade = null;

  // request (server) only
  this.url = '';
  this.method = null;

  // response (client) only
  this.statusCode = null;
  this.statusMessage = null;
  this.client = this.socket;

}

util.inherits(IncomingMessage, stream.ReadableStream);

exports.IncomingMessage = IncomingMessage;

IncomingMessage.prototype.read = function(n) {
  this.read = stream.ReadableStream.prototype.read;
  return this.read(n);
};

/*IncomingMessage.prototype.end = function(data, callback) {
  this.socket.end(data,callback);
};*/
