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


var http = require('http');

var server = http.createServer(function (req, res) {
  // req is an http.IncomingMessage, which is a Readable Stream
  // res is an http.ServerResponse, which is a Writable Stream

  var body = '';

  console.log("request handler is called");
  req.on('data', function (chunk) {
    body += chunk;
    res.write("on data's chunk : " + body.toString());

  });

  req.on('end', function () {
    // write back something interesting to the user:
    res.write("we are done:"+ body + "\n");
    res.end();
  });


});

server.listen(3001,1,cb);

function cb(){
  console.log("listening....");
};
