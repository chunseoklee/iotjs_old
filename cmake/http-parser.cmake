# Copyright 2015 Samsung Electronics Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8)

set(HTTPPARSER_ROOT ${DEP_ROOT}/http-parser)
set(HTTPPARSER_INCDIR ${HTTPPARSER_ROOT})
set(HTTPPARSER_LIB ${LIB_ROOT}/libhttpparser.a)

add_custom_command(OUTPUT ${HTTPPARSER_LIB}
                   COMMAND touch ${HTTPPARSER_LIB})

add_custom_target(targetHttpparser DEPENDS ${HTTPPARSER_LIB})
