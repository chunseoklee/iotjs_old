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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "jerry.h"
#include "jerry-api.h"

#include "uv.h"

#include "iotjs_binding.h"
#include "iotjs_env.h"
#include "iotjs_util.h"
#include "iotjs_module.h"
#include "iotjs_module_process.h"

#include "iotjs.h"
#include "iotjs_js.h"

namespace iotjs {


static bool InitJerry(char* src) {
  //char* src = ReadFile(src_path);

  jerry_init(JERRY_FLAG_EMPTY);

  if (!jerry_parse(src, strlen(src))) {
    fprintf(stderr, "jerry_parse() failed\n");
    return false;
  }

  if (jerry_run() != JERRY_COMPLETION_CODE_OK) {
    fprintf(stderr, "jerry_run() failed\n");
    return false;
  }

  //ReleaseCharBuffer(src);

  return true;
}


static void ReleaseJerry() {
  jerry_cleanup();
}


static void InitModules() {
  InitModuleList();
  InitProcess();
}

static void CleanupModules() {
  CleanupModuleList();
}

static bool InitIoTjs() {

  char *iotjs_main_src = iotjs::ReadFile("../../../../src/js/iotjs.js");
  jerry_api_value_t retval_p;
  jerry_api_eval(mainjs,mainjs_length,
                 false, false, &retval_p);
  JObject global(GetGlobal());
  JObject process = global.GetProperty("process");
  JObject iotjs_fun(&retval_p, true);
  JObject *args = new JObject[1];
  args[0] = process;
  iotjs_fun.Call(&global, &args, 1);

  return 1;
}

static bool StartIoTjs() {
  bool is_ok;

  // Get jerry global object.
  JObject global(GetGlobal());

  // Create environtment.
  Environment env(uv_default_loop());

  // Bind environment to global object.
  global.SetNative((uintptr_t)(&env));


  // Call the entry.
  // load and call iotjs.js
  InitIoTjs();

  bool more;
  do {
    more = uv_run(env.loop(), UV_RUN_ONCE);
  } while (more);

  return true;
}


int Start(char* src) {
  if (!InitJerry(NULL)) {
    fprintf(stderr, "InitJerry failed\n");
    return 1;
  }

  InitModules();

  if (!StartIoTjs()) {
    fprintf(stderr, "StartIoTJs failed\n");
    return 1;
  }

  CleanupModules();

  ReleaseJerry();

  return 0;
}

} // namespace iotjs


extern "C" int iotjs_entry(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: iotjs <js>\n");
    return 1;
  }

  char* src = iotjs::ReadFile(argv[1]);

  int res = iotjs::Start(src);

  iotjs::ReleaseCharBuffer(src);

  return res;
}
