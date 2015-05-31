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
#include "uv.h"
#include <cstdlib>
#include <string.h>

#ifdef __LINUX__
#define PLATFORM "linux"
#endif

#ifdef __NUTTX__
#define PLATFORM "nuttx"
#endif


namespace iotjs {


// Calls next tick callbacks registered via `process.nextTick()`.
void OnNextTick() {
  Module* module = GetBuiltinModule(MODULE_PROCESS);
  assert(module != NULL);

  JObject* process = module->module;
  assert(process != NULL);
  assert(process->IsObject());

  JObject jon_next_tick = process->GetProperty("_onNextTick");
  assert(jon_next_tick.IsFunction());

  jon_next_tick.Call(JObject::Null(), JArgList::Empty());
}


// Make a callback for the given `function` with `this_` binding and `args`
// arguments. The next tick callbacks registered via `process.nextTick()`
// will be called after the callback function `function` returns.
JObject MakeCallback(JObject& function, JObject& this_, JArgList& args) {
  // Calls back the function.
  JObject res = function.Call(this_, args);

  // Calls the next tick callbacks.
  OnNextTick();

  // Return value.
  return res;
}


JHANDLER_FUNCTION(Binding, handler) {
  assert(handler.GetArgLength() == 1);
  assert(handler.GetArg(0)->IsNumber());

  int module_kind = handler.GetArg(0)->GetInt32();

  Module* module = GetBuiltinModule(static_cast<ModuleKind>(module_kind));

  if (module->module == NULL) {
    module->module = module->fn_register();
  }

  handler.Return(*module->module);

  return true;
}

JHANDLER_FUNCTION(Compile, handler){
  assert(handler.GetArgLength() == 1);
  assert(handler.GetArg(0)->IsString());

  char* code = handler.GetArg(0)->GetCString();

  JObject ret(JObject::Eval(code));
  handler.Return(ret);

  JObject::ReleaseCString(code);

  return true;
}



JHANDLER_FUNCTION(CompileNativePtr, handler){
  assert(handler.GetArgLength() == 1);
  assert(handler.GetArg(0)->IsObject());

  char* source = (char*)(handler.GetArg(0)->GetNative());

  const char* wrapper[2] = {
    "(function (a, b, c) { function wwwwrap(exports,require, module) {",
    " }; wwwwrap(a, b, c); });" };

  int len = strlen(source)+ strlen(wrapper[0]) + strlen(wrapper[1]);
  char* code = AllocCharBuffer(len+1);

  strcpy(code,wrapper[0]);
  strcat(code,source);
  strcat(code,wrapper[1]);

  JObject ret(JObject::Eval(code));
  handler.Return(ret);

  JObject::ReleaseCString(code);

  return true;
}


JHANDLER_FUNCTION(ReadSource, handler){
  assert(handler.GetArgLength() == 1);
  assert(handler.GetArg(0)->IsString());

  char* code = ReadFile(handler.GetArg(0)->GetCString());
  JObject ret(code);
  handler.Return(ret);

  JObject::ReleaseCString(code);
  return true;
}

JHANDLER_FUNCTION(Cwd, handler){
  assert(handler.GetArgLength() == 0);

  char path[120];
  size_t size_path = sizeof(path);
  int err = uv_cwd(path, &size_path);
  if (err) {
    JHANDLER_THROW_RETURN(handler, TypeError, "cwd error");
  }
  JObject ret(path);
  handler.Return(ret);

  return true;
}

void SetNativeSources(JObject* native_sources) {
  for (int i = 0; natives[i].name; i++) {
    JObject native_source;
    native_source.SetNative((uintptr_t)(natives[i].source), NULL);
    native_sources->SetProperty(natives[i].name, native_source);
  }
}


void SetProcessEnv(JObject* process){
  const char *homedir;
  homedir = getenv("HOME");
  if(homedir == NULL) {
    homedir = "";
  }
  JObject home(homedir);
  JObject env;
  env.SetProperty("HOME",home);
  process->SetProperty("env",env);

}

JObject* InitProcess() {
  Module* module = GetBuiltinModule(MODULE_PROCESS);
  JObject* process = module->module;

  if (process == NULL) {
    process = new JObject();
    process->SetMethod("binding", Binding);
    process->SetMethod("compile", Compile);
    process->SetMethod("compileNativePtr", CompileNativePtr);
    process->SetMethod("readSource", ReadSource);
    process->SetMethod("cwd", Cwd);
    SetProcessEnv(process);

    // process.native_sources
    JObject native_sources;
    SetNativeSources(&native_sources);
    process->SetProperty("native_sources", native_sources);

    // process.platform
    JObject platform(PLATFORM);
    process->SetProperty("platform", platform);

    // Binding module id.
    JObject jbinding = process->GetProperty("binding");

#define ENUMDEF_MODULE_LIST(upper, Camel, lower) \
    jbinding.SetProperty(# lower, JVal::Int(MODULE_ ## upper));

    MAP_MODULE_LIST(ENUMDEF_MODULE_LIST)

#undef ENUMDEF_MODULE_LIST

    module->module = process;
  }

  return process;
}


} // namespace iotjs
