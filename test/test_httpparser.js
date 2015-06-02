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

var Buffer = require('buffer');
var httpparser = process.binding(process.binding.httpparser).HTTPParser;

function AddHeader(dest, src){
  for(var i=0;i<src.length;i++){
    dest[dest.length+i] = src[i];
    }
  dest.length = dest.length + src.length;
}

var OnHeaderscb = function(headers, url) {
  console.log("======js on headers ====== \n");
  for(var i=0;i<headers.length;i++){
      console.log(headers[i]);
  }
  console.log("url: " + url);

  // FIXME: This should be impl. with Array.concat
  AddHeader(this._headers, headers);
  this._url += url;

};

var OnHeadersCompletecb = function(info) {
  console.log("=== ===js header complete == \n");
  console.log("url = " + this._url + "\n");
  for(var i=0;i<this._headers.length;i++){
    console.log(this._headers[i]);
  }
};

var OnMessageCompletecb = function() {
  console.log("======js on_msg_complete=====\n");
};

var OnBodycb = function(buf, offset, length) {
  console.log("======js on body == \n");

  console.log("buf: "+ buf.toString());
  console.log("offset: "+ offset);
  console.log("length: "+ length);
};

console.log("httpparser.REQUEST = "+httpparser.REQUEST);
console.log("httpparser.RESPONSE = "+httpparser.RESPONSE);


//console.log("httpparser.kOnHeaders = "+httpparser.kOnHeaders);
//console.log("httpparser.kOnBody = "+httpparser.kOnBody);

var x = new httpparser(httpparser.REQUEST);
x._url = "";
x._headers ={};
x._headers.length = 0;
x.OnHeaders = OnHeaderscb;
x.OnHeadersComplete = OnHeadersCompletecb;
x.OnMessageComplete = OnMessageCompletecb;

var buf_req = new Buffer("GET http://weitz.de/ HTTP/1.1\r\n\
Host: weitz.de\r\n\
User-Agent: Drakma/1.3.11 SBCL 1.1.14.debian Linux\r\n\
Accept: a\r\n\
Connection: close\r\n\
\r\n\
arseiotniarsneitnareionstn\r\n\
\r\n");

x.execute(buf_req);


var parser_res = new httpparser(httpparser.RESPONSE);
parser_res._url = "";
parser_res._headers ={};
parser_res._headers.length = 0;
parser_res.OnHeaders = OnHeaderscb;
parser_res.OnHeadersComplete = OnHeadersCompletecb;
parser_res.OnMessageComplete = OnMessageCompletecb;
parser_res.OnBody = OnBodycb;
var buf_res = new Buffer("HTTP/1.1 200 OK\r\n\
Content-Length: 10\r\n\
Connection: close\r\n\
\r\n\
\"areistss\"\r\n");

parser_res.execute(buf_res);


var ar = ["a", "b", "c"];
console.log(ar.shift());
