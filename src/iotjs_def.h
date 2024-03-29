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


#ifndef IOTJS_DEF_H
#define IOTJS_DEF_H


#ifndef IOTJS_MAX_READ_BUFFER_SIZE
 #ifdef __NUTTX__
  #define IOTJS_MAX_READ_BUFFER_SIZE 1024
 #else
  #define IOTJS_MAX_READ_BUFFER_SIZE 65535
 #endif
#endif


#ifndef IOTJS_ASSERT
 #ifdef NDEBUG
  #define IOTJS_ASSERT(x) ((void)(x))
 #else
  #define IOTJS_ASSERT(x) assert(x)
 #endif
#endif


// commonly used header files
#include "iotjs_binding.h"
#include "iotjs_env.h"
#include "iotjs_debuglog.h"
#include "iotjs_module.h"
#include "iotjs_module_process.h"
#include "iotjs_util.h"

#include <uv.h>
#include <assert.h>


#endif /* IOTJS_DEF_H */
