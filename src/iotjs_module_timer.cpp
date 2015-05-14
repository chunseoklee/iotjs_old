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

#include <assert.h>

#include "iotjs_binding.h"
#include "iotjs_env.h"
#include "iotjs_handlewrap.h"
#include "iotjs_module.h"
#include "iotjs_module_timer.h"
#include "iotjs_module_process.h"
#include "iotjs_util.h"


namespace iotjs {

class TimerWrap : public HandleWrap {
 public:
  explicit TimerWrap(Environment* env, JObject* jtimer)
      : HandleWrap(jtimer, reinterpret_cast<uv_handle_t*>(&_handle)) {
    uv_timer_init(env->loop(), &_handle);
  }

  virtual ~TimerWrap() {}

  static TimerWrap* FromHandle(uv_timer_t* handle) {
    TimerWrap* timer_wrap = static_cast<TimerWrap*>(handle->data);
    assert(handle == timer_wrap->handle_ptr());
    return timer_wrap;
  }

  uv_timer_t* handle_ptr() {
    return &_handle;
  }

  void set_timeout(int64_t timeout) {
    _timeout = timeout;
  }

  void set_repeat(int64_t repeat) {
    _repeat = repeat;
  }

  void OnTimeout() {
    if (_jcallback != NULL && _jcallback->IsFunction()) {
      assert(object()->IsObject());
      MakeCallback(*_jcallback, *object(), JArgList::Empty());
    }
  }

 protected:
  uv_timer_t _handle;
  int64_t _timeout;
  int64_t _repeat;
};


static void timerHandleTimeout(uv_timer_t* handle) {
  TimerWrap* timer_wrap = TimerWrap::FromHandle(handle);
  assert(timer_wrap->object()->IsObject());
  if (timer_wrap) {
    timer_wrap->OnTimeout();
  }
}


JHANDLER_FUNCTION(Start, handler) {
  assert(handler.GetArgLength() >= 3);
  assert(handler.GetArg(2)->IsFunction());

  JObject* jtimer = handler.GetThis();
  assert(jtimer->IsObject());

  int64_t timeout = handler.GetArg(0)->GetInt64();
  int64_t repeat = handler.GetArg(1)->GetInt64();
  JObject* jcallback = handler.GetArg(2);

  TimerWrap* timer_wrap = reinterpret_cast<TimerWrap*>(jtimer->GetNative());
  assert(timer_wrap != NULL);
  assert(timer_wrap->object()->IsObject());

  timer_wrap->set_timeout(timeout);
  timer_wrap->set_repeat(repeat);
  timer_wrap->set_callback(*jcallback);

  int err;
  err = uv_timer_start(timer_wrap->handle_ptr(),
                       timerHandleTimeout,
                       timeout,
                       repeat);

  JObject ret(err);
  handler.Return(ret);

  return true;
}


JHANDLER_FUNCTION(Stop, handler) {
  JObject* jtimer = handler.GetThis();

  TimerWrap* timer_wrap = reinterpret_cast<TimerWrap*>(jtimer->GetNative());
  assert(timer_wrap != NULL);

  int err = uv_timer_stop(timer_wrap->handle_ptr());

  JObject ret(err);
  handler.Return(ret);

  return true;
}


JHANDLER_FUNCTION(Timer, handler) {
  // `this` should be a object.
  assert(handler.GetThis()->IsObject());

  Environment* env = Environment::GetEnv();
  JObject* jtimer = handler.GetThis();

  TimerWrap* timer_wrap = new TimerWrap(env, jtimer);
  assert(timer_wrap->object()->IsObject());
  assert(jtimer->GetNative() != 0);;

  return true;
}


JObject* InitTimer() {
  Module* module = GetBuiltinModule(MODULE_TIMER);
  JObject* timer = module->module;

  if (timer == NULL) {
    timer = new JObject(Timer);

    JObject prototype;
    timer->SetProperty("prototype", prototype);
    prototype.SetMethod("start", Start);
    prototype.SetMethod("stop", Stop);

    module->module = timer;
  }

  return timer;
}

} // namespace iotjs
