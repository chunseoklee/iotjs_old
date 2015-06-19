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
#include "iotjs_module_httpparser.h"
#include "http_parser.h"
#include "iotjs_objectwrap.h"
#include "iotjs_module_buffer.h"
#include <string.h>
#include <stdlib.h>
#include <cstdio>


namespace iotjs {


#define JSETPROPERTY( container_, fname_, vname_ )                  \
  do {                                                              \
    JObject jobj(vname_ );                                          \
    container_.SetProperty( fname_, jobj);                          \
  } while(0)


#define HEADER_MAX 10


struct StringHelper {
  StringHelper() {
    saved_heap = false;
    Reset();
  }

  ~StringHelper() {
    Reset();
  }

  void Reset() {
    if (saved_heap) {
      saved_heap = false;
      delete str;
    }
    str = NULL;
    len = 0;
  }

  void SaveToHeap() {
    if (!saved_heap && len > 0) {
      char* heap_str = new char[len];
      memcpy(heap_str, str, len);
      str = heap_str;
      saved_heap = true;
    }
  }

  void UpdateStr(const char* newstr, int length) {
    if (str == NULL) {
      str = newstr;
    }
    else if (saved_heap || str + len != newstr) {
       char* heap_str = new char[len+length];
       memcpy(heap_str, str, len);
       memcpy(heap_str+len, newstr, length);
       if (saved_heap)
         delete str;
       else
         saved_heap = true;

       str = heap_str;

    }
    len = len + length;
  }

  char* ToCString() {
    char* newstr = new char[len+1];
    memcpy(newstr, str, len);
    newstr[len] = 0;
    return newstr;
  }

  bool saved_heap;
  const char* str;
  int len;
};

class HTTPParserWrap : public JObjectWrap {
public:
  explicit HTTPParserWrap(JObject& parser_, http_parser_type type)
    : JObjectWrap(parser_) {
    //parser_.SetNative((uintptr_t)this, NULL);
    Initialize(type);
    parser.data = this;
  }

  void Initialize(http_parser_type type);

  // http-parser callbacks
  int on_message_begin_() {
    url.Reset();
    status_msg.Reset();
    return 0;
  }

  int on_url_(const char* at, size_t length) {
    url.UpdateStr(at, length);
    return 0;
  }

  int on_status_(const char* at, size_t length) {
    status_msg.UpdateStr(at, length);
    return 0;
  }

  int on_header_field_(const char* at, size_t length) {

    if (n_fields == n_values) {
      n_fields++;
      if (n_fields == HEADER_MAX) {
        Flush(); // to JS world
        n_fields = 1;
        n_values = 0;
      }
      fields[n_fields-1].Reset();
    }
    assert(n_fields == n_values + 1);
    fields[n_fields-1].UpdateStr(at, length);

    return 0;
  }

  int on_header_value_(const char* at, size_t length) {
    if (n_fields != n_values) {
      n_values++;
      values[n_values-1].Reset();
    }

    assert(n_fields == n_values);

    values[n_values-1].UpdateStr(at, length);

    return 0;
  }

  JObject makeHeader(){
    // FIXME: This must be impl. as JS Array
    JObject jheader;
    JSETPROPERTY(jheader, "length", (n_fields-1)*2);
    for (int i=0;i<n_fields;i++) {
      char index_string0 [5];
      char index_string1 [5];
      sprintf(index_string0,"%d",i*2);
      sprintf(index_string1,"%d",i*2+1);
      JObject v(values[i].ToCString());
      JObject f(fields[i].ToCString());
      jheader.SetProperty(index_string0, f);
      jheader.SetProperty(index_string1, v);

    }
    return jheader;
  }


  int on_headers_complete_() {

    JObject jobj = jobject();
    JObject func = jobj.GetProperty("OnHeadersComplete");
    assert(func.IsFunction());

    // URL
    JArgList argv(1);
    JObject info;

    // HEADERS
    if (flushed) {
      Flush();
    }
    else {
      JSETPROPERTY(info, "headers", makeHeader());
      JSETPROPERTY(info, "url", url.ToCString());
    }
    n_fields = n_values = 0;

    // Method
    if (parser.type == HTTP_REQUEST) {
      JSETPROPERTY(info, "method", (int32_t)parser.method);
    }

    // Status
    if (parser.type == HTTP_RESPONSE) {
      JSETPROPERTY(info, "status", (int32_t)parser.status_code);
      JSETPROPERTY(info, "status_msg", status_msg.ToCString());
    }

    // upgrade
    JSETPROPERTY(info, "upgrade", parser.upgrade ? true : false);
    // shouldkeepalive
    JSETPROPERTY(info, "shouldkeepalive",
                 http_should_keep_alive(&parser) ? true : false);


    argv.Add(info);

    JResult jres(func.Call(jobj, argv));

    return jres.value().GetBoolean() ? 1 : 0;
  }
  int on_body_(const char* at, size_t length) {
    JObject jobj = jobject();
    JObject func = jobj.GetProperty("OnBody");
    assert(func.IsFunction());

    JArgList argv(3);
    argv.Add(*cur_jbuf);
    int offset = (int)(at-cur_buf);
    JObject start(offset);
    argv.Add(start);
    JObject leng((int)length);
    argv.Add(leng);

    JResult jres(func.Call(jobj, argv));


    if(jres.value().IsNull()){
      had_exception = true;
      return false;
    }

    return 0;
  }
  int on_message_complete_() {
    JObject jobj = jobject();
    JObject func = jobj.GetProperty("OnMessageComplete");
    assert(func.IsFunction());


    JResult jres(func.Call(jobj, JArgList::Empty()));

    if(jres.value().IsNull()){
      had_exception = true;
      return false;
    }

    return 0;
  }

  void Flush() {
    JObject jobj = jobject();
    JObject func = jobj.GetProperty("OnHeaders");
    assert(func.IsFunction());


    JArgList argv(2);
    JObject jheader(makeHeader());
    argv.Add(jheader);
    JObject jurl(url.ToCString());
    argv.Add(jurl);


    JResult jres(func.Call(jobj, argv));

    if(jres.value().IsNull()){
      had_exception = true;
    }

    url.Reset();
    flushed = true;

  }

  void Save() {
    url.SaveToHeap();
    status_msg.SaveToHeap();
    for (int i = 0; i < n_fields; i++) {
      fields[i].SaveToHeap();
    }

    for (int i = 0; i < n_values; i++) {
      values[i].SaveToHeap();
    }
  }

  http_parser parser;
  StringHelper url;
  StringHelper status_msg;
  StringHelper fields[HEADER_MAX];
  StringHelper values[HEADER_MAX];
  int n_fields;
  int n_values;
  JObject* cur_jbuf;
  char* cur_buf;
  int cur_buf_len;
  bool flushed;
  bool had_exception;
};

#define HTTP_CB(name)                                                   \
  int name(http_parser* parser) {                                       \
    HTTPParserWrap* container = (HTTPParserWrap*)(parser->data);        \
    return container->name##_();                                        \
  }                                                                     \


#define HTTP_DATA_CB(name)                                              \
  int name(http_parser* parser, const char* at, size_t length) {        \
    HTTPParserWrap* container = (HTTPParserWrap*)(parser->data);        \
    return container->name##_(at, length);                              \
  }                                                                     \



//http-parser callbacks
HTTP_CB( on_message_begin );
HTTP_DATA_CB( on_url );
HTTP_DATA_CB( on_status );
HTTP_DATA_CB( on_header_field );
HTTP_DATA_CB( on_header_value );
HTTP_CB( on_headers_complete );
HTTP_DATA_CB( on_body );
HTTP_CB(on_message_complete);

const struct http_parser_settings settings = {
  iotjs::on_message_begin,
  iotjs::on_url,
  iotjs::on_status,
  iotjs::on_header_field,
  iotjs::on_header_value,
  iotjs::on_headers_complete,
  iotjs::on_body,
  iotjs::on_message_complete,
};

void HTTPParserWrap::Initialize(http_parser_type type) {
  http_parser_init(&parser, type);
  url.Reset();
  status_msg.Reset();
  n_fields = 0;
  n_values = 0;
  flushed = false;
  had_exception = false;
}

JHANDLER_FUNCTION(Reinitialize, handler) {
  assert(handler.GetThis()->IsObject());

  JObject* jparser = handler.GetThis();

  http_parser_type httpparser_type =
    static_cast<http_parser_type>(handler.GetArg(0)->GetInt32());
  assert(httpparser_type == HTTP_REQUEST || httpparser_type == HTTP_RESPONSE);

  HTTPParserWrap* parser =
    reinterpret_cast<HTTPParserWrap*>(jparser->GetNative());
  parser->Initialize(httpparser_type);

  return true;
}


JHANDLER_FUNCTION(Finish, handler) {
  assert(handler.GetThis()->IsObject());
  JObject* jparser = handler.GetThis();
  HTTPParserWrap* parser =
    reinterpret_cast<HTTPParserWrap*>(jparser->GetNative());

  int rv = http_parser_execute(&(parser->parser), &settings, NULL, 0);

  if (rv != 0) {
    enum http_errno err = HTTP_PARSER_ERRNO(&parser->parser);

    JObject eobj(JObject::Error("Parse Error"));
    JSETPROPERTY(eobj, "byteParsed", 0);
    JSETPROPERTY(eobj, "code", http_errno_name(err));
    handler.Return(eobj);
  }

  return true;
}


JHANDLER_FUNCTION(Execute, handler) {
  assert(handler.GetThis()->IsObject());
  JObject* jparser = handler.GetThis();
  HTTPParserWrap* parser =
    reinterpret_cast<HTTPParserWrap*>(jparser->GetNative());


  JObject* jbuffer = handler.GetArg(0);
  Buffer* buffer = Buffer::FromJBuffer(*jbuffer);
  char* buf_data = buffer->buffer();
  int buf_len = buffer->length();


  parser->cur_jbuf = jbuffer;
  parser->cur_buf = buf_data;
  parser->cur_buf_len = buf_len;
  parser->had_exception = false;

  int nparsed = http_parser_execute(&(parser->parser), &settings,
                                    buf_data, buf_len);

  parser->Save();

  parser->cur_jbuf = NULL;
  parser->cur_buf = NULL;
  parser->cur_buf_len = 0;


  if(parser->had_exception){
    return false; // ???
  }

  if(nparsed != buf_len){
    enum http_errno err = HTTP_PARSER_ERRNO(&parser->parser);
    JObject eobj(JObject::Error("Parse Error"));
    JSETPROPERTY(eobj, "byteParsed", 0);
    JSETPROPERTY(eobj, "code", http_errno_name(err));
    handler.Return(eobj);
  }
  else{
    JObject ret(nparsed);
    handler.Return(ret);
  }

  return true;
}

JHANDLER_FUNCTION(Pause, handler) {
  assert(handler.GetThis()->IsObject());
  JObject* jparser = handler.GetThis();
  HTTPParserWrap* parser =
    reinterpret_cast<HTTPParserWrap*>(jparser->GetNative());
  http_parser_pause(&parser->parser, 1);
  return true;
}

JHANDLER_FUNCTION(Resume, handler) {
  assert(handler.GetThis()->IsObject());
  JObject* jparser = handler.GetThis();
  HTTPParserWrap* parser =
    reinterpret_cast<HTTPParserWrap*>(jparser->GetNative());
  http_parser_pause(&parser->parser, 0);
  return true;
}


JHANDLER_FUNCTION(Close, handler) {
  assert(handler.GetThis()->IsObject());

  JObject* jparser = handler.GetThis();
  HTTPParserWrap* parser =
    reinterpret_cast<HTTPParserWrap*>(jparser->GetNative());

  delete parser;  // FIXME: used in FreeParser.
                  // sure that this impl is ok.

  return true;
}


JHANDLER_FUNCTION(HTTPParserCons, handler) {
  assert(handler.GetThis()->IsObject());

  JObject* jparser = handler.GetThis();

  http_parser_type httpparser_type =
    static_cast<http_parser_type>(handler.GetArg(0)->GetInt32());
  assert(httpparser_type == HTTP_REQUEST || httpparser_type == HTTP_RESPONSE);
  HTTPParserWrap* httpparser_wrap = new HTTPParserWrap(*jparser,
                                                       httpparser_type);
  assert(httpparser_wrap->jobject().IsObject());
  assert(reinterpret_cast<HTTPParserWrap*>(jparser->GetNative())
         == httpparser_wrap);
  return true;
}


JObject* InitHttpparser() {

  Module* module = GetBuiltinModule(MODULE_HTTPPARSER);
  JObject* httpparser = module->module;

  if (httpparser == NULL) {
    httpparser = new JObject();

    JObject HTTPParserConstructor(HTTPParserCons);
    httpparser->SetProperty("HTTPParser", HTTPParserConstructor);

    JObject request(HTTP_REQUEST);
    HTTPParserConstructor.SetProperty("REQUEST", request);
    JObject response(HTTP_RESPONSE);
    HTTPParserConstructor.SetProperty("RESPONSE", response);


    JObject prototype;
    HTTPParserConstructor.SetProperty("prototype", prototype);
    // prototype ...
    prototype.SetMethod("close", Close);
    prototype.SetMethod("execute", Execute);
    prototype.SetMethod("reinitialize", Reinitialize);
    prototype.SetMethod("finish", Finish);
    prototype.SetMethod("pause", Pause);
    prototype.SetMethod("resume", Resume);

    module->module = httpparser;
  }

  return httpparser;

}


} // namespace iotjs
