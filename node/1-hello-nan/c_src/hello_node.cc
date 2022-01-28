#include <nan.h>

using v8::Local;
using v8::String;

#define NEW_FUN(fun) Nan::GetFunction(Nan::New<v8::FunctionTemplate>(fun)).ToLocalChecked()
#define NEW_STR(str) Nan::New<v8::String>(str).ToLocalChecked()

NAN_METHOD(Hello) {
    Local<String> str = NEW_STR("Hello World!");
    info.GetReturnValue().Set(str);
}

NAN_MODULE_INIT(Init) {
    Nan::Set(target, NEW_STR("hello"), NEW_FUN(Hello));
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
