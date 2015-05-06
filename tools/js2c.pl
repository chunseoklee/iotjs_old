#!/usr/bin/perl

#  Copyright 2015 Samsung Electronics Co., Ltd.
# 
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
# 
#      http://www.apache.org/licenses/LICENSE-2.0
# 
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file converts src/js/iotjs.js to a C-array in include/iotjs_js.h file  

use strict;

my $lisence = "/* Copyright 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an \"AS IS\" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */\n";
my $header = "#ifndef IOTJS_JS_H\n#define IOTJS_JS_H\nnamespace iotjs{\n";
my $start_variable = "  const char mainjs[] = {\n";
my $end_variable = "0 };\n";
my $foot = "}\n#endif\n";

open(my $in, "<../src/js/iotjs.js") || die "Cannot open iotjs.js\n";
open(my $out, ">../src/iotjs_js.h") || die "Cannot open iotjs_js.h\n";

print $out $lisence;
print $out $header;
print $out $start_variable;

my $data = do { local $/; <$in> };
my @chars = split //, $data;

my $count = 0;
foreach (@chars) {
    $count++;
    my $char = ord($_);
    print $out "$char," ;
    if($count % 10 == 0){
        print $out "\n" ;
    }
}




print $out $end_variable;
print $out "\nconst int mainjs_length \= $count\;\n"; 

print $out $foot;

