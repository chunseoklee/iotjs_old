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

#ifndef IOTJS_ENV_H
#define IOTJS_ENV_H

#include "uv.h"

#include "iotjs_module.h"
#include "iotjs_util.h"


namespace iotjs {

class ReqWrap;

class Environment {
 public:
  Environment(uv_loop_t* loop);
  ~Environment() { delete _statConstructor; }
  static Environment* GetEnv();

  uv_loop_t* loop() { return _loop; }
  JObject* statConstructor() { return _statConstructor; }
  void SetStatConstructor(JObject* cons) { _statConstructor = cons; }

 private:
  uv_loop_t* _loop;
  JObject* _statConstructor;
}; // class Environment

} // namespace iotjs

#endif /* IOTJS_ENV_H */
