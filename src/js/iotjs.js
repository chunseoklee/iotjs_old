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

(function(process) {
  this.global = this;

  function start_iotjs() {
    var module = Native.require('module');
    module.runMain();
  };

  function Native(id) {
    this.id = id;
    this.filename = id + '.js';
    this.exports = {};
  }

  Native.require = function(id) {
    if (id == 'native') {
      return Native;
    }

    var nativeMod = new Native(id);
    nativeMod.compile();
    return nativeMod.exports;
  };

  Native.wrap = function(script) {
    var temp1 = Native.wrapper[0] + script;
    temp1 = temp1 + Native.wrapper[1];
    return temp1;
  };

  Native.wrapper = [
    '(function (exports, require, module) { ',
    ' });'
  ];

  Native.prototype.compile = function() {
    var source = process.native_sources[this.id];
    source = Native.wrap(source);
    var fn = process.compile(source);
    fn(this.exports, Native.require, this);
  };

  // temp impl. before JSON.parse is done
  process.JSONParse = function(text) {
      return process.compile("(" + text + ");");
  };

  start_iotjs();
});
