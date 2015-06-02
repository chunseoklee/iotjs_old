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

var fs = require('fs');


var onRead = function(err, bytesRead, buffer) {
  if (err) {
    console.log(err.message);
  } else {
    console.log(buffer.toString());
  }
};


var onOpen = function(err, fd) {
  if (err) {
    console.log(err.message);
  } else {
    buffer = new Buffer(64);
    fs.read(fd, buffer, 0, 64, 0, onRead);
  }
};


fs.open("greeting.txt", "r", 438, onOpen);


try {
  var fd = fs.openSync("greeting.txt", "r", 438);
  var buffer = new Buffer(64);
  fs.read(fd, buffer, 0, 64, 0, onRead);
  console.log("openSync->readAsync succeed");
}
catch(err) {
   console.log(err.message);
}

try {
  var fd = fs.openSync("greeting.txt", "r", 438);
  var buffer = new Buffer(64);
  var str = fs.read(fd, buffer, 0, 64, 0);
  console.log("openSync->readSync succeed");
}
catch(err) {
   console.log(err.message);
}

// error test
try {
  var fd = fs.openSync("non-greeting.txt", "r", 438);
  var buffer = new Buffer(64);
  var str = fs.read(fd, buffer, 0, 64, 0);
  console.log("openSync->readSync succeed");
}
catch(err) {
   console.log(err.message);
}

try {
  var fd = fs.openSync("output.txt", "w", 438);
  var buffer = new Buffer(17);
  buffer.write("writeBuffer test1");
  var str = fs.writeSync(fd, buffer, 0, 17, 0);
  console.log("openSync->writeSync succeed");
}
catch(err) {
   console.log(err.message);
}


var onWrite = function(err, fd, buffer) {
  if (err) {
    console.log(err.message);
  } else {
    console.log(buffer.toString());
  }
};

var onWrite2 = function(err, fd, buffer) {
  if (err) {
    console.log(err.message);
  } else {
    console.log(buffer.toString());
  }
};

try {
  var fd = fs.openSync("output.txt", "w", 438);
  var buffer = new Buffer(17);
  buffer.write("writeBuffer test2");
  var str = fs.write(fd, buffer, 0, 17, 0, onWrite);
  console.log("openSync->writeAsync succeed");
}
catch(err) {
   console.log(err.message);
}

try {
  var fd = fs.openSync("output.txt", "w", 438);
  var buffer = "writeString Sync test";
  var str = fs.writeSync(fd, buffer, 0, 'ascii');
  console.log("openSync->writeSync(writeString) succeed");
}
catch(err) {
   console.log(err.message);
}

try {
  var fd = fs.openSync("output.txt", "w", 438);
  var buffer = "writeString Async test";
  var str = fs.write(fd, buffer, 0, 'ascii', onWrite2);
  console.log("openSync->writeASync(writeString) succeed");
}
catch(err) {
   console.log(err.message);
}
