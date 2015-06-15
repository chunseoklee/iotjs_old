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


var httpparser = process.binding(process.binding.httpparser).HTTPParser;
var http = require('http');

function handleRequest(request, response){
    console.log('handleRequest called');
}



var newserver = new http.createServer(handleRequest);

newserver.setTimeout(1000, function(){
  console.log("server timeouted");
});

newserver.on('connection', function(socket) {
  socket.on('data', function(data) {
    socket.end('I received : \n' + data.toString());
  });
  socket.on('close', function() {
    newserver.close();
  });
});


newserver.listen(3001, 2, function(){
  console.log("listening....");
});
