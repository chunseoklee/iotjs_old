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


var fs = exports;
var buffer = require('buffer').Buffer;
var constants = require('constants');
var util = require('util');
var fsBuiltin = process.binding(process.binding.fs);

var S_IFMT = 61440;  // octat 170000
var S_IFDIR = 16384; // octat  40000


fs.statSync = function(path) {
  return fsBuiltin.stat(path);
};


fs.Stats = function(dev,
                    mode,
                    size) {
  this.dev = dev;
  this.mode = mode;
  this.size = size;

};

fs.Stats.prototype.isDirectory = function() {
  return ((this.mode & S_IFMT) === S_IFDIR);
};

fs.createStat = function(dev, mode, size) {
  var statobj = new fs.Stats(dev, mode, size);
  return statobj;
};

fsBuiltin.setStatConstructor(fs.createStat);



var O_APPEND = constants.O_APPEND;
var O_CREAT = constants.O_CREAT;
var O_EXCL = constants.O_EXCL;
var O_RDONLY = constants.O_RDONLY;
var O_RDWR = constants.O_RDWR;
var O_SYNC = constants.O_SYNC;
var O_TRUNC = constants.O_TRUNC;
var O_WRONLY = constants.O_WRONLY;


fs.open = function(path, flags, mode, callback) {
  if (!util.isNumber(mode)) {
    mode = 438; // 0666;
  }
  fsBuiltin.open(path,
                 convertFlags(flags),
                 mode,
                 callback);
};


fs.read = function(fd, buffer, offset, length, position, callback) {
  if (!util.isBuffer(buffer)) {
    throw new TypeError('Bad arguments');
  }

  var cb_wrap = function(err, bytesRead) {
    callback && callback(err, bytesRead || 0, buffer);
  };

  fsBuiltin.read(fd, buffer, offset, length, position, cb_wrap);
};


function convertFlags(flag) {
  if (!util.isString(flag)) {
    return flag;
  }

  switch (flag) {
    case 'r': return O_RDONLY;
    case 'rs':
    case 'sr': return O_RDONLY | O_SYNC;

    case 'r+': return O_RDWR;
    case 'rs+':
    case 'sr+': return O_RDWR | O_SYNC;

    case 'w': return O_TRUNC | O_CREAT | O_WRONLY;
    case 'wx':
    case 'xw': return O_TRUNC | O_CREAT | O_WRONLY | O_EXCL;

    case 'w+': return O_TRUNC | O_CREAT | O_RDWR;
    case 'wx+':
    case 'xw+': return O_TRUNC | O_CREAT | O_RDWR | O_EXCL;

    case 'a': return O_APPEND | O_CREAT | O_WRONLY;
    case 'ax':
    case 'xa': return O_APPEND | O_CREAT | O_WRONLY | O_EXCL;

    case 'a+': return O_APPEND | O_CREAT | O_RDWR;
    case 'ax+':
    case 'xa+': return O_APPEND | O_CREAT | O_RDWR | O_EXCL;
  }
  return new Error('unknown file open flag: ' + flag);
}
