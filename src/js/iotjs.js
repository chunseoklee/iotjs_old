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
  global.process = process;


  function start_iotjs() {
    init_global();
    init_timers();

    initProcess();

    var module = Native.require('module');

    module.runMain();
  };


  function init_global() {
    global.process = process;
    global.global = global;
    global.GLOBAL = global;
    global.root = global;
    global.console =  process.binding(process.binding.console);
    global.Buffer = Native.require('buffer');
  };


  function init_timers() {
    global.setTimeout = function(callback, delay) {
      var t = Native.require('timers');
      return t.setTimeout.call(this, callback, delay);
    };

    global.setInterval = function(callback, repeat) {
      var t = Native.require('timers');
      return t.setInterval.call(this, callback, repeat);
    };

    global.clearTimeout = function(timeout) {
      var t = Native.require('timers');
      return t.clearTimeout.call(this, timeout);
    };

    global.clearInterval = function(interval) {
      var t = Native.require('timers');
      return t.clearInterval.call(this, interval);
    };
  }


  function initProcess() {
    initProcessEvents();
    initProcessNextTick();
    initProcessUncaughtException();
    initProcessExit();
  }


  function initProcessEvents() {
    var EventEmitter = Native.require('events').EventEmitter;

    EventEmitter.call(process);

    var keys = Object.keys(EventEmitter.prototype);
    for (var i = 0; i < keys.length; ++i) {
      var key = keys[i];
      if (!process[key]) {
        process[key] = EventEmitter.prototype[key];
      }
    }
  }


  function initProcessNextTick() {
    var nextTickQueue = [];

    process.nextTick = nextTick;
    process._onNextTick = _onNextTick;

    function _onNextTick() {
      // clone nextTickQueue to new array object, and calls function
      // iterating the cloned array. This is becuase,
      // during processing nextTick
      // a callback could add another next tick callback using
      // `process.nextTick()`, if we calls back iterating original
      // `nextTickQueue` that could turn into infinify loop.

      var callbacks = nextTickQueue.slice(0);
      nextTickQueue = [];

      for (var i = 0; i < callbacks.length; ++i) {
        try {
          callbacks[i]();
        } catch (e) {
          process._onUncaughtExcecption(e);
        }
      }

      return nextTickQueue.length > 0;
    }

    function nextTick(callback) {
      nextTickQueue.push(callback);
    }
  }


  function initProcessUncaughtException() {
    process._onUncaughtExcecption = _onUncaughtExcecption;
    function _onUncaughtExcecption(error) {
      var event = 'uncaughtException';
      if (process._events[event] && process._events[event].length > 0) {
        try {
          // Emit uncaughtException event.
          process.emit('uncaughtException', error);
        } catch (e) {
          // Even uncaughtException handler thrown, that could not be handled.
          console.error('uncaughtException handler throws: ' + e);
          process.exit(1);
        }
      } else {
        // Exit if there are no handler for uncaught exception.
        console.error('uncaughtException: ' + error);
        process.exit(1);
      }
    }
  }


  function initProcessExit() {
    process.exitCode = 0;
    process._exiting = false;


    process.emitExit = function(code) {
      if (!process._exiting) {
        process._exiting = true;
        if (code || code == 0) {
          process.exitCode = code;
        }
        process.emit('exit', process.exitCode || 0);
      }
    }


    process.exit = function(code) {
      try {
        process.emitExit(code);
      } catch (e) {
        process.exitCode = 1;
        process._onUncaughtExcecption(e);
      } finally {
        process.doExit(process.exitCode || 0);
      }
    };
  }


  function Native(id) {
    this.id = id;
    this.filename = id + '.js';
    this.exports = {};
  };


  Native.cache = {};


  Native.require = function(id) {
    if (id == 'native') {
      return Native;
    }

    if (Native.cache[id]) {
      return Native.cache[id].exports;
    }

    var nativeMod = new Native(id);

    Native.cache[id] = nativeMod;
    nativeMod.compile();

    return nativeMod.exports;
  };


  Native.wrap = function(script) {
    var temp1 = Native.wrapper[0] + script;
    temp1 = temp1 + Native.wrapper[1];
    return temp1;
  };


  Native.wrapper = [
    '(function (a, b, c) { function wwwwrap(exports, require, module) {',
    ' }; wwwwrap(a, b, c); });'
  ];


  Native.prototype.compile = function() {
    // process.native_sources has a list of pointers to
    // the source strings defined in 'iotjs_js.h', not
    // source strings.
    var source = process.native_sources[this.id];
    var fn = process.compileNativePtr(source);
    fn(this.exports, Native.require, this);
  };


  // temp impl. before JSON.parse is done
  process.JSONParse = function(text) {
      return process.compile("(" + text + ");");
  };

  start_iotjs();

});
