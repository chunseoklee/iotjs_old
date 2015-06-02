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

function Server(requestListener) {
  if (!(this instanceof Server)) {
    return new Server(requestListener);
  }

  net.Server.call(this);

  if (requestListener) {
    this.addListener('request', requestListener);
  }

  this.addListener('connection', connectionListener);

  this.timeout = 2 * 60 * 1000; // ?
}

util.inherits(Server, net.Server);

Server.prototype.setTimeout = function(msecs, callback) {
  this.timeout = msecs;
  if (callback)
    this.on('timeout', callback);
};

exports.Server = Server;

function connectionListener(socket) {

}
