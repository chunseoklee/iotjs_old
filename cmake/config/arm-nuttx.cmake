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

include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME EXTERNAL)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_SYSTEM_VERSION NUTTX)

set(EXTERNAL_CMAKE_C_COMPILER arm-none-eabi-gcc)
set(EXTERNAL_CMAKE_CXX_COMPILER arm-none-eabi-g++)

CMAKE_FORCE_C_COMPILER(${EXTERNAL_CMAKE_C_COMPILER} GNU)
CMAKE_FORCE_CXX_COMPILER(${EXTERNAL_CMAKE_CXX_COMPILER} GNU)

set(NO_PTHREAD YES)
set(BUILD_TO_LIB YES)

set(FLAGS_COMMON -mcpu=cortex-m4
                 -mthumb
                 -march=armv7e-m
                 -mfpu=fpv4-sp-d16
                 -mfloat-abi=hard
                 -D__NUTTX__
                 -D__ARM__
                 -Os
                 -fno-strict-aliasing
                 -fno-strength-reduce
                 -fomit-frame-pointer)

foreach(FLAG ${FLAGS_COMMON})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
endforeach()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

set(TARGET_INC ${TARGET_INC} "${NUTTX_HOME}/include")
set(TARGET_INC ${TARGET_INC} "${NUTTX_HOME}/include/cxx")

