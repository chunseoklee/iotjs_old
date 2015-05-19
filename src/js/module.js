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

function Module(id, parent) {
  this.id = id;
  this.exports = {};
  this.filename = null;
  this.parent = parent;
  if(parent && parent.chidren){
    parent.chidren.push(this);
  }
  this.chidren = [];
};

module.exports = Module;


Module.wrapper = Native.wrapper;
Module.wrap = Native.wrap;


var moduledirs = [ process.env.HOME + "/.iotjs_module/", process.cwd()+"/"];

Module.concatdir = function(a, b){
  var rlist = [];
  for(var i = 0; i< a.length ; i++) {
    rlist.push(a[i]);
  }

  for(var i = 0; i< b.length ; i++) {
    rlist.push(b[i]);
  }

  return rlist;
};


Module.resolveDirectories = function(id, parent) {

  if((id[0] != '.' || id[1] != '.') && (id[0] != '.' || id[1] != '/')) {
    var dirs = moduledirs;
    if(parent) {
      if(!parent.dirs){
        parent.dirs = [];
      }
      dirs = Module.concatdir(parent.dirs, dirs);
    }
    return dirs;
  }

  if(!parent){
    var newdirs = Module.concatdir(["./"], moduledirs);
    return newdirs;
  }


  return moduledirs;
};


Module.resolveFilepath = function(id, directories) {

  for(var i = 0; i<directories.length ; i++) {
    var dir = directories[i];
    // 1. 'id'
    var filepath = Module.tryPath(dir+id);

    if(filepath){
      return filepath;
    }

    // 2. 'id.js'
    filepath = Module.tryPath(dir+id+'.js');

    if(filepath){
      return filepath;
    }

    // 3. package path $HOME/.iotjs_module/id
    var packagepath = dir + id;
    var jsonpath = packagepath + "/package.json";
    filepath = Module.tryPath(jsonpath);
    if(filepath){
      var pkgSrc = process.readSource(jsonpath);
      var pkgMainFile = process.JSONParse(pkgSrc).main;
      return packagepath + "/" + pkgMainFile;
    }
  }

  return false;
};


Module.resolveModPath = function(id, parent) {

  // 0. resolve Directory for lookup
  var directories = Module.resolveDirectories(id, parent);

  var filepath = Module.resolveFilepath(id, directories);

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


Module.load = function(id, parent, isMain) {

  if(process.native_sources[id]){
    return Native.require(id);
  }

  var module = new Module(id, parent);

  var modPath = Module.resolveModPath(module.id, module.parent);


  if(modPath) {
    module.filename = modPath;
    module.SetModuleDirs(modPath);
    module.compile();
  }

  return module.exports;
};

Module.prototype.compile = function() {

  var source = process.readSource(this.filename);
  source = Module.wrap(source);
  var fn = process.compile(source);
  fn.call(this, this.exports, this.require, this);
};


Module.runMain = function(){
  Module.load(process.argv[1], null, true);
  process._onNextTick();
};


Module.prototype.SetModuleDirs = function(filepath)
{
  var dir = "";
  var i;
  for(i = filepath.length-1;i>=0 ; i--) {
    if(filepath[i] == '/'){
      break;
    }
  }

  // save filepath[0] to filepath[i]
  // e.g. /home/foo/main.js ->  /home/foo/
  for(;i>=0 ; i--) {
    dir = filepath[i] + dir;
  }
  this.dirs = [dir];
};

Module.prototype.require = function(id) {
  return Module.load(id, this);
};
