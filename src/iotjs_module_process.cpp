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

#include "iotjs_binding.h"
#include "iotjs_module.h"
#include "iotjs_module_process.h"
#include "iotjs_js.h"


namespace iotjs {

static bool Binding(const jerry_api_object_t *function_obj_p,
                    const jerry_api_value_t *this_p,
                    jerry_api_value_t *ret_val_p,
                    const jerry_api_value_t args_p [],
                    const uint16_t args_cnt) {
  assert(args_cnt == 1);
  assert(JVAL_IS_NUMBER(&args_p[0]));

  int module_kind = JVAL_TO_INT32(&args_p[0]);
  Module* module = GetBuiltinModule(static_cast<ModuleKind>(module_kind));

  if (module->module == NULL) {
    module->module = module->fn_register();
  }

  module->module->Ref();

  *ret_val_p = module->module->val();

  return true;
}

static bool Compile(const jerry_api_object_t *function_obj_p,
                     const jerry_api_value_t *this_p,
                     jerry_api_value_t *ret_val_p,
                     const jerry_api_value_t args_p [],
                     const uint16_t args_cnt) {
  assert(args_cnt == 1);
  assert(JVAL_IS_STRING(&args_p[0]));
  int len = -GetJerryStringLength(&args_p[0]);
  char* code = AllocCharBuffer(len+1);
  jerry_api_string_to_char_buffer(args_p[0].v_string, code, len+1);

  jerry_api_eval(code,sizeof(code),true,false,ret_val_p);
  ReleaseCharBuffer(code);
  return true;
}

static bool ReadSource(const jerry_api_object_t *function_obj_p,
                        const jerry_api_value_t *this_p,
                        jerry_api_value_t *ret_val_p,
                        const jerry_api_value_t args_p [],
                        const uint16_t args_cnt) {
  assert(args_cnt == 1);
  assert(JVAL_IS_STRING(&args_p[0]));

  char buf[128];
  int len = -GetJerryStringLength(&args_p[0]);
  jerry_api_string_to_char_buffer(args_p[0].v_string, buf, len);

  char* code = ReadFile(buf);
  JObject ret_obj(code,false);
  *ret_val_p = ret_obj.val();
  ReleaseCharBuffer(code);
  return true;
}

void SetNativeSources(JObject* native_sources) {
  for (int i = 0; natives[i].name; i++) {
    JObject native_source(natives[i].source);
    native_sources->SetProperty(natives[i].name, &native_source);
  }
}

JObject* InitProcess() {
  Module* module = GetBuiltinModule(MODULE_PROCESS);
  JObject* process = module->module;

  if (process == NULL) {
    process = new JObject();
    process->CreateMethod("binding", Binding);
    process->CreateMethod("compile", Compile);
    process->CreateMethod("readSource", ReadSource);

    // process.native_sources
    JObject native_sources;
    SetNativeSources(&native_sources);
    process->SetProperty("native_sources", &native_sources);

    module->module = process;

    JObject global(GetGlobal());
    global.SetProperty("process", process);
  }

  return process;
}

} // namespace iotjs
