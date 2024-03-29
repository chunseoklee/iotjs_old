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

#include "iotjs_def.h"
#include "iotjs_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


namespace iotjs {


char* ReadFile(const char* path) {
  FILE* file = fopen(path, "rb");
  IOTJS_ASSERT(file != NULL);

  fseek(file, 0, SEEK_END);
  size_t len = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* buff = AllocBuffer(len + 1);

  size_t read = fread(buff, 1, len, file);
  IOTJS_ASSERT(read == len);

  *(buff+len) = 0;

  fclose(file);

  return buff;
}


char* AllocBuffer(size_t size) {
  char* buff = static_cast<char*>(malloc(size));
  memset(buff, 0, size);
  return buff;
}


char* ReallocBuffer(char* buffer, size_t size) {
  return static_cast<char*>(realloc(buffer, size));
}


void ReleaseBuffer(char* buffer) {
  free(buffer);
}


LocalString::LocalString(size_t len)
    : _strp(AllocBuffer(len)) {
  IOTJS_ASSERT(_strp != NULL);
}

LocalString::LocalString(char* strp)
    : _strp(strp) {
  IOTJS_ASSERT(_strp != NULL);
}


LocalString::~LocalString() {
  IOTJS_ASSERT(_strp != NULL);
  ReleaseBuffer(const_cast<char*>(_strp));
}


LocalString::operator char* () const {
  return _strp;
}


} // namespace iotjs
