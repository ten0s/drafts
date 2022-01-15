#include <node.h>

namespace demo {

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::NewStringType;
using v8::Object;
using v8::String;
using v8::Value;

void Method(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  v8::MaybeLocal<v8::String> str =
      String::NewFromUtf8(isolate, "Hello World!", NewStringType::kNormal);
  v8::Local<v8::String> checkedStr = str.ToLocalChecked();
  v8::ReturnValue<v8::Value> retVal = args.GetReturnValue();
  retVal.Set(checkedStr);
}

// Add addons must export an initialization function
void Initialize(Local<Object> exports) {
  NODE_SET_METHOD(exports, "hello", Method);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize) // no semi-colon here!

} // namespace demo
