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

var ar = [ 1, 2, 3];
console.log(''+ar.length);

var server = http.createServer(function (req, res) {
  // req is an http.IncomingMessage, which is a Readable Stream
  // res is an http.ServerResponse, which is a Writable Stream

  var body = '';
  var url = req.url;

  console.log("req.method: " + req.method);
  req.on('data', function (chunk) {
    console.log("req.on data called :" + chunk.toString()  +"\n....");
    body += chunk;
  });

  req.on('end', function () {
    console.log("req.end is called");
    //res.write("body is: ");
    res.writeHead(200,{ "Connection" : "close"});
    res.write("<html>");
    res.write("<head>");
    res.write("<title>"+body+"</title>");
    res.write("</head>");
    res.write("<body>");
    res.write(url);
    res.write("</body>");
    res.write("</html>");
    //res.write(body);
    res.end();
  });

});

server.listen(3001,1,function cb(){
  console.log("listening....");
});
