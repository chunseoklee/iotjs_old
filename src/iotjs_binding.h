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

#ifndef IOTJS_BINDING_H
#define IOTJS_BINDING_H

#include "jerry-api.h"
#include "iotjs_util.h"


namespace iotjs {


typedef jerry_external_handler_t JHandlerType;
typedef jerry_object_free_callback_t JFreeHandlerType;
typedef jerry_api_object_t JRawObjectType;
typedef jerry_api_value_t JRawValueType;


class JObject;
class JArgList;
class JHandlerInfo;
class JLocalScope;


/// Wrapper for Javascript objects.
class JObject {
 public:
  // Consturctors.

  // Creates a initail javascript object.
  explicit JObject();

  // Creates a object copied from other object.
  JObject(const JObject& other);

  // Creates a javascript boolean object.
  explicit JObject(bool v);

  // Creates a javascript number object from various c type.
  explicit JObject(int32_t v);
  explicit JObject(float v);
  explicit JObject(double v);

  // Creates a javascirpt number object.
  explicit JObject(const char* v);

  // Creates a object from `JRawObjectType*`.
  // If second argument set true, then ref count for the object will be
  // decreased when this wrapper is being destroyed.
  explicit JObject(const JRawObjectType* obj, bool need_unref = true);

  // Creates a object from `JRawValueType*`.
  // If second argument set true, then ref count for the object will be
  // decreased when this wrapper is being destroyed.
  explicit JObject(const JRawValueType* val, bool need_unref = true);

  // Creates a javascript function object.
  // When the function is called, the handler will be triggered.
  explicit JObject(JHandlerType handler);

  // Create a javascript null object.
  static JObject& Null();

  // Get the javascript global object.
  static JObject Global();

  // Create a javascript error object.
  static JObject Error(const char* message = NULL);
  static JObject EvalError(const char* message = NULL);
  static JObject RangeError(const char* message = NULL);
  static JObject ReferenceError(const char* message = NULL);
  static JObject SyntaxError(const char* message = NULL);
  static JObject TypeError(const char* message = NULL);
  static JObject URIError(const char* message = NULL);

  // Destoyer for this class.
  // When the wrapper is being destroyed, ref count for correspoding javascript
  // object will be decreased unless `need_unref` was set false.
  ~JObject();

  // Increases ref count.
  void Ref();

  // Decreases ref count.
  void Unref();

  // Returns whether the object is specific type.
  bool IsNull();
  bool IsNumber();
  bool IsString();
  bool IsObject();
  bool IsFunction();

  // Sets native handler method for the javascript object.
  void SetMethod(const char* name, JHandlerType handler);

  // Sets & gets property for the javascript object.
  void SetProperty(const char* name, JObject& val);
  void SetProperty(const char* name, JRawValueType val);
  JObject GetProperty(const char* name);

  // Sets & gets native data for the javascript object.
  void SetNative(uintptr_t ptr, JFreeHandlerType free_handler);
  uintptr_t GetNative();

  // Retruns value for 32bit integer contents of number object.
  int32_t GetInt32();

  // Returns value for 64bit integer contents of number object.
  int64_t GetInt64();

  // Returns pontiner to null terminated string contents of string object.
  // Returned pointer should be released using `ReleaseCString()` when it become
  // unnecessary.
  char* GetCString();

  // Release memory retrieved from `GetCString()`.
  static void ReleaseCString(char* str);

  // Returns length of null teminated string contents of string object.
  size_t GetCStringLength();

  // Calls javascript function.
  JObject Call(JObject& this_, JArgList& arg);

  JRawValueType raw_value() { return _obj_val; }

 private:
  JRawValueType _obj_val;
  bool _unref_at_close;
};


class JVal {
 public:
  static JRawValueType Void();
  static JRawValueType Undefined();
  static JRawValueType Null();
  static JRawValueType Bool(bool v);
  static JRawValueType Int(int32_t v);
  static JRawValueType Float(float v);
  static JRawValueType Double(double v);
  static JRawValueType Object(const JRawObjectType* obj);
};


class JArgList {
 public:
  JArgList(uint16_t capacity);
  ~JArgList();

  static JArgList& Empty();

  uint16_t GetLength();

  void Add(JObject& x);
  void Set(uint16_t i, JObject& x);
  JObject* Get(uint16_t i);

 private:
  uint16_t _capacity;
  uint16_t _argc;
  JObject** _argv;
};


class JHandlerInfo {
 public:
  JHandlerInfo(const JRawObjectType* function_obj_p,
               const JRawValueType* this_p,
               JRawValueType* ret_val_p,
               const JRawValueType args_p[],
               const uint16_t args_cnt);
  ~JHandlerInfo();

  JObject* GetFunction();
  JObject* GetThis();
  JObject* GetArg(uint16_t i);
  uint16_t GetArgLength();

  void Return(JObject& ret);
  void Throw(JObject& err);

  bool HasThrown();

 private:
  JObject _function;
  JObject _this;
  JArgList _arg_list;
  JRawValueType* _ret_val_p;
  bool _thrown;
};


#define JHANDLER_THROW(handler_info, error_type, message) \
  JObject error = JObject::error_type(message); \
  handler_info.Throw(error); \


#define JHANDLER_THROW_RETURN(handler_info, error_type, message) \
  JHANDLER_THROW(handler_info, error_type, message); \
  return false;


#define JHANDLER_FUNCTION(handler, arg_name) \
  static bool ___ ## handler ## _wrap(JHandlerInfo& arg_name); \
  static bool handler(const JRawObjectType *function_obj_p, \
                      const JRawValueType *this_p, \
                      JRawValueType *ret_val_p, \
                      const JRawValueType args_p [], \
                      const uint16_t args_cnt) { \
    JHandlerInfo info(function_obj_p, this_p, ret_val_p, args_p, args_cnt); \
    return ___ ## handler ## _wrap(info); \
  } \
  static bool ___ ## handler ## _wrap(JHandlerInfo& arg_name) \


#define JFREE_HANDLER_FUNCTION(handler, arg_name) \
  static void handler(const uintptr_t arg_name) \


class JLocalScope {
 public:
  explicit JLocalScope();
  ~JLocalScope();

  JObject* CreateJObject();

 private:
  LinkedList<JObject*> _object_list;
};


} // namespace iotjs


#endif /* IOTJS_BINDING_H */
