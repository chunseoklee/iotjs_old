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


namespace iotjs {


#define JVAL_IS_NULL(val_p) ((val_p)->type == JERRY_API_DATA_TYPE_NULL)
#define JVAL_IS_STRING(val_p) ((val_p)->type == JERRY_API_DATA_TYPE_STRING)
#define JVAL_IS_OBJECT(val_p) ((val_p)->type == JERRY_API_DATA_TYPE_OBJECT)
#define JVAL_IS_FUNCTION(val_p) \
  (JVAL_IS_OBJECT(val_p) && jerry_api_is_function((val_p)->v_object))

#define JVAL_IS_NUMBER(val_p) \
  (((val_p)->type == JERRY_API_DATA_TYPE_FLOAT32) || \
   ((val_p)->type == JERRY_API_DATA_TYPE_FLOAT64) || \
   ((val_p)->type == JERRY_API_DATA_TYPE_UINT32))

#define JVAL_TO_INT32(val_p) \
   ((val_p)->type == JERRY_API_DATA_TYPE_FLOAT32 ? \
      static_cast<int32_t>((val_p)->v_float32) : \
    (val_p)->type == JERRY_API_DATA_TYPE_FLOAT64 ? \
      static_cast<int32_t>((val_p)->v_float64) : \
    static_cast<int32_t>((val_p)->v_uint32))

#define JVAL_TO_INT64(val_p) \
   ((val_p)->type == JERRY_API_DATA_TYPE_FLOAT32 ? \
      static_cast<int64_t>((val_p)->v_float32) : \
    (val_p)->type == JERRY_API_DATA_TYPE_FLOAT64 ? \
      static_cast<int64_t>((val_p)->v_float64) : \
    static_cast<int64_t>((val_p)->v_uint32))


JRawObjectType* GetGlobal() {
  return jerry_api_get_global();
}


JRawValueType JerryValFromObject(const JRawObjectType* obj) {
  JRawValueType res;
  res.type = JERRY_API_DATA_TYPE_OBJECT;
  res.v_object = const_cast<JRawObjectType*>(obj);
  return res;
}


void SetObjectField(JRawObjectType* obj,
                    const char* name,
                    JRawValueType* val) {
  bool is_ok = jerry_api_set_object_field_value(obj, name, val);
  assert(is_ok);
}


JRawValueType GetObjectField(JRawObjectType* obj, const char* name) {
  JRawValueType res;
  bool is_ok = jerry_api_get_object_field_value(obj, name, &res);
  assert(is_ok);
  return res;
}


void SetObjectNative(JRawObjectType* obj, uintptr_t ptr) {
  jerry_api_set_object_native_handle(obj, ptr);
}


uintptr_t GetObjectNative(JRawObjectType* obj) {
  uintptr_t res;
  bool is_ok = jerry_api_get_object_native_handle(obj, &res);
  assert(is_ok);
  return res;
}


size_t GetJerryStringLength(const JRawValueType* val) {
  assert(JVAL_IS_STRING(val));
  return jerry_api_string_to_char_buffer (val->v_string, NULL, 0);
}


char* DupJerryString(const JRawValueType* val) {
  assert(JVAL_IS_STRING(val));
  size_t size = -GetJerryStringLength(val);
  char* buffer = AllocCharBuffer(size);
  size_t check = jerry_api_string_to_char_buffer(val->v_string, buffer, size);
  assert(check == size);
  return buffer;
}


void ReleaseJerryString(char* str) {
  ReleaseCharBuffer(str);
}


JObject::JObject() {
  JRawObjectType* new_obj = jerry_api_create_object();
  _obj_val = JerryValFromObject(new_obj);
  _unref_at_close = true;
}


JObject::JObject(const JObject& other) {
  _obj_val = other._obj_val;
  _unref_at_close = true;
  Ref();
}


JObject::JObject(bool v) {
  _obj_val = JVal::Bool(v);
}


JObject::JObject(int32_t v) {
  _obj_val = JVal::Int(v);
}


JObject::JObject(float v) {
  _obj_val = JVal::Float(v);
}


JObject::JObject(double v) {
  _obj_val = JVal::Double(v);
}

JObject::JObject(const char* v) {
  _obj_val.type = JERRY_API_DATA_TYPE_STRING;
  _obj_val.v_string = jerry_api_create_string(v);
  _unref_at_close = true;
}


JObject::JObject(const JRawObjectType* obj, bool need_unref) {
  _obj_val = JerryValFromObject(obj);
  _unref_at_close = need_unref;
}


JObject::JObject(const JRawValueType* val, bool need_unref) {
  _obj_val = *val;
  _unref_at_close = need_unref;
}


JObject::JObject(JHandlerType handler) {
  _obj_val.v_object = jerry_api_create_external_function(handler);
  assert(jerry_api_is_constructor(_obj_val.v_object));
  _obj_val.type = JERRY_API_DATA_TYPE_OBJECT;
  _unref_at_close = true;
}


JObject::~JObject() {
  if (_unref_at_close) {
    Unref();
  }
}


static JRawValueType jnull_val = JVal::Null();
static JObject jnull(&jnull_val);
JObject& JObject::Null() {
  return jnull;
}


JObject JObject::Global() {
  return JObject(GetGlobal());
}


void JObject::SetMethod(const char* name, JHandlerType handler) {
  assert(IsObject());
  JObject method(jerry_api_create_external_function(handler));
  SetProperty(name, method);
}


void JObject::SetProperty(const char* name, JObject& val) {
  assert(IsObject());
  JRawValueType v = val.raw_value();
  SetObjectField(_obj_val.v_object, name, &v);
}


JObject JObject::GetProperty(const char* name) {
  assert(IsObject());
  JRawValueType val = GetObjectField(_obj_val.v_object, name);
  return JObject(&val);
}


void JObject::Ref() {
  if (JVAL_IS_STRING(&_obj_val)) {
    jerry_api_acquire_string(_obj_val.v_string);
  } else if (JVAL_IS_OBJECT(&_obj_val)) {
    jerry_api_acquire_object(_obj_val.v_object);
  }
}


void JObject::Unref() {
  if (JVAL_IS_STRING(&_obj_val)) {
    jerry_api_release_string(_obj_val.v_string);
  } else if (JVAL_IS_OBJECT(&_obj_val)) {
    jerry_api_release_object(_obj_val.v_object);
  }
}


bool JObject::IsNull() {
  return JVAL_IS_NULL(&_obj_val);
}


bool JObject::IsNumber() {
  return JVAL_IS_NUMBER(&_obj_val);
}


bool JObject::IsString() {
  return JVAL_IS_STRING(&_obj_val);
}


bool JObject::IsObject() {
  return JVAL_IS_OBJECT(&_obj_val);
}


bool JObject::IsFunction() {
  return JVAL_IS_FUNCTION(&_obj_val);
}


void JObject::SetNative(uintptr_t ptr) {
  assert(IsObject());
  jerry_api_set_object_native_handle(_obj_val.v_object, ptr);
}


uintptr_t JObject::GetNative() {
  assert(IsObject());
  uintptr_t ptr;
  jerry_api_get_object_native_handle(_obj_val.v_object, &ptr);
  return ptr;
}


void JObject::SetFreeCallback(jerry_object_free_callback_t freecb) {
  assert(IsObject());
  jerry_api_set_object_free_callback(_obj_val.v_object, freecb);
}


JObject JObject::Call(JObject& this_, JArgList& arg) {
  assert(IsFunction());

  JRawObjectType* this_obj_p = this_.IsNull() ? NULL
                                              : this_.raw_value().v_object;
  JRawValueType res;
  JRawValueType* val_args = NULL;
  uint16_t val_argv = 0;

  if (arg.GetLength() > 0) {
    val_argv = arg.GetLength();
    val_args = new JRawValueType[val_argv];
    for (int i = 0; i < val_argv; ++i) {
      val_args[i] = arg.Get(i)->raw_value();
    }
  }

  bool is_ok = jerry_api_call_function(_obj_val.v_object,
                                       this_obj_p,
                                       &res,
                                       val_args,
                                       val_argv);
  assert(is_ok);

  if (val_args) {
    delete val_args;
  }

  return JObject(&res);
}

int32_t JObject::GetInt32() {
  assert(IsNumber());
  return JVAL_TO_INT32(&_obj_val);
}


int64_t JObject::GetInt64() {
  assert(IsNumber());
  return JVAL_TO_INT64(&_obj_val);
}


char* JObject::GetCString() {
  assert(IsString());
  return DupJerryString(&_obj_val);
}


void JObject::ReleaseCString(char* str) {
  ReleaseJerryString(str);
}


size_t JObject::GetCStringLength() {
  assert(IsString());
  return GetJerryStringLength(&_obj_val);
}


JRawValueType JVal::Void() {
  JRawValueType val;
  val.type = JERRY_API_DATA_TYPE_VOID;
  return val;
}


JRawValueType JVal::Undefined() {
  JRawValueType val;
  val.type = JERRY_API_DATA_TYPE_UNDEFINED;
  return val;
}


JRawValueType JVal::Null() {
  JRawValueType val;
  val.type = JERRY_API_DATA_TYPE_NULL;
  return val;
}


JRawValueType JVal::Bool(bool v) {
  JRawValueType val;
  val.type = JERRY_API_DATA_TYPE_BOOLEAN;
  val.v_bool = v;
  return val;
}


JRawValueType JVal::Int(int32_t v) {
  JRawValueType val;
  val.type = JERRY_API_DATA_TYPE_UINT32;
  val.v_uint32 = v;
  return val;
}


JRawValueType JVal::Float(float v) {
  JRawValueType val;
  val.type = JERRY_API_DATA_TYPE_FLOAT32;
  val.v_uint32 = v;
  return val;
}


JRawValueType JVal::Double(double v) {
  JRawValueType val;
  val.type = JERRY_API_DATA_TYPE_FLOAT64;
  val.v_uint32 = v;
  return val;
}


JArgList::JArgList(uint16_t capacity)
    : _capacity(capacity)
    , _argc(0)
    , _argv(NULL) {
  if (_capacity > 0) {
    _argv = new JObject*[_capacity];
    for (int i = 0; i < _capacity; ++i) {
      _argv[i] = NULL;
    }
  }
}


JArgList::~JArgList() {
  assert(_argv == NULL || _argc > 0);
  assert(_argc <= _capacity);
  for (int i = 0; i < _argc; ++i) {
    delete _argv[i];
  }
  delete _argv;
}


static JArgList jarg_empty(0);
JArgList& JArgList::Empty() {
  return jarg_empty;
}

uint16_t JArgList::GetLength() {
  return _argc;
}


void JArgList::Add(JObject& x) {
  assert(_argc < _capacity);
  _argv[_argc++] = new JObject(x);
}


void JArgList::Set(uint16_t i, JObject& x) {
  assert(i < _argc);
  if (_argv[i] != NULL) {
    delete _argv[i];
  }
  _argv[i] = new JObject(x);
}


JObject* JArgList::Get(uint16_t i) {
  assert(i < _argc);
  return _argv[i];
}


JHandlerInfo::JHandlerInfo(const JRawObjectType* function_obj_p,
                           const JRawValueType* this_p,
                           JRawValueType* ret_val_p,
                           const JRawValueType args_p[],
                           const uint16_t args_cnt)
    : _function(function_obj_p, false)
    , _this(this_p, false)
    , _arg_list(args_cnt)
    , _ret_val_p(ret_val_p) {
  if (args_cnt > 0) {
    for (int i = 0; i < args_cnt; ++i) {
      JObject arg(&args_p[i], false);
      _arg_list.Add(arg);
    }
  }
}


JHandlerInfo::~JHandlerInfo() {
}


JObject* JHandlerInfo::GetFunction() {
  return &_function;
}


JObject* JHandlerInfo::GetThis() {
  return &_this;
}


JObject* JHandlerInfo::GetArg(uint16_t i) {
  return _arg_list.Get(i);
}


uint16_t JHandlerInfo::GetArgLength() {
  return _arg_list.GetLength();
}


void JHandlerInfo::Return(JObject& ret) {
  ret.Ref();
  *_ret_val_p = ret.raw_value();
}


JLocalScope::JLocalScope() {
}


JLocalScope::~JLocalScope() {
  _object_list.Clear();
}


JObject* JLocalScope::CreateJObject() {
  JObject* new_object = new JObject();
  _object_list.InsertTail(new_object);
  return new_object;
}


} // namespace iotjs
