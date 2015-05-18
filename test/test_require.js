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


var fs = require('buffer');
var util = require('util');

var x = require("require_add.js");
print("require_add: " + x.add(1,4));
process.nextTick(function(){print("nextTick works!");});

var str = process.readSource("package.json");
var json = process.JSONParse(str);
print("package.json src:");
print(str);
print("package.json main: " + json.main);
