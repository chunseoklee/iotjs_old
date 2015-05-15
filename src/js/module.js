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


var Native = require('native');
var fs = Native.require('fs');

function Module(id) {
  this.id = id;
  this.exports = {};
  this.filename = id + ".js";
};

module.exports = Module;


Module.wrapper = Native.wrapper;
Module.wrap = Native.wrap;


Module.resolveModPath = function(id) {

  // 1. 'id'
  var filepath = Module.tryPath(id);

  if(filepath){
    return filepath;
  }

  // 2. 'id.js'
  filepath = Module.tryPath(id+'.js');

  if(filepath){
    return filepath;
  }

  return false;
};


Module.tryPath = function(path) {
  var stats = Module.statPath(path);
  if(stats && !stats.isDirectory()) {
    return path;
  }
  else {
    return false;
  }
};


Module.statPath = function(path) {
  try {
    return fs.statSync(path);
  } catch (ex) {}
  return false;
};


Module.load = function(id,isMain) {
  if(process.native_sources[id]){
    return Native.require(id);
  }
  var module = new Module(id);

  var modPath = Module.resolveModPath(module.id);

  // FIXME: handle the case of no module named 'id' or 'id.js'.
  if(modPath) {
    var source = process.readSource(modPath);
    source = Module.wrap(source);
    var fn = process.compile(source);
    fn(module.exports, module.require, module);
  }

  return module.exports;
};


Module.runMain = function(){
  Module.load(process.argv[1],true);
  process._onNextTick();
};


Module.prototype.require = function(id) {
  return Module.load(id);
};
