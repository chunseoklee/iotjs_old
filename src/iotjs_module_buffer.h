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

#ifndef IOTJS_MODULE_BUFFER_H
#define IOTJS_MODULE_BUFFER_H


#include "iotjs_objectwrap.h"


namespace iotjs {


JObject* InitBuffer();


// Crate buffer object.
JObject CreateBuffer(size_t len);


class Buffer : public JObjectWrap {
 public:
  Buffer(JObject& jbuffer, size_t length);

  virtual ~Buffer();

  static Buffer* FromJBuffer(JObject& jbuffer);

  JObject& jbuffer();
  char* buffer();
  size_t length();

  size_t Copy(char* src, size_t len);
  size_t Copy(char* src, size_t len, size_t src_from , size_t dst_from);

 protected:
  char* _buffer;
  size_t _length;
};



} // namespace iotjs


#endif /* IOTJS_MODULE_BUFFER_H */
