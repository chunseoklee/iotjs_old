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

function OutgoingMessage() {
  stream.Stream.call(this);

  this.output = [];
  this.outputEncodings = [];
  this.outputCallbacks = [];

  this.writable = true;

  this._last = false;
  this.chunkedEncoding = false;
  this.shouldKeepAlive = true;

  this._hasBody = true;

  this.finished = false;
  this._sentHeader = false;

  this.socket = null;
  this.connection = null;
  this._header = null;
  this._headers = {};
  this._headerNames = {};
}
util.inherits(OutgoingMessage, stream.Stream);


exports.OutgoingMessage = OutgoingMessage;


OutgoingMessage.prototype.end = function(data, encoding, callback) {
  this._finish();
  return true;
};


OutgoingMessage.prototype._finish = function() {
  this.emit('prefinish');
};


OutgoingMessage.prototype._send = function(chunk, encoding, callback) {
  if (!this._sentHeader) {
    if (util.isBuffer(chunk)) {
      chunk = chunk.toString();
      console.log("buffer chunk"+chunk);
    }
    chunk = this._header + "\r\n" + chunk;
    console.log("header sent");
    this._sentHeader = true;
  }
  if (util.isBuffer(chunk)) {
    chunk = chunk.toString();

  }
  console.log("socket write chunk"+chunk);
  this.connection.write(chunk, encoding, callback);
};


OutgoingMessage.prototype.write = function(chunk, encoding, callback) {
  if (!this._header) {
    console.log("_implicitHeader called");
    this._implicitHeader();
  }

  var ret = this._send(chunk, encoding, callback);

  return ret;

};


OutgoingMessage.prototype._storeHeader = function(statusLine) {
  console.log("_storiHeader called");
  var headerStr = '';

  var keys;
  if (this._headers) {
    keys = Object.keys(this._headers);
    for (var i=0;i<keys.length;i++) {
      var key = keys[i];
      headerStr += key + ": " + this._headers[key] + '\r\n';
    }
  }

  this._header = statusLine + headerStr;
  console.log(this._header);
};
