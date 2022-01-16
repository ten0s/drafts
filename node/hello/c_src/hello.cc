#include <node.h>

namespace demo {

using namespace v8;

void Hello(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    MaybeLocal<String> str =
        String::NewFromUtf8(isolate, "Hello World!", NewStringType::kNormal);
    Local<String> checkedStr = str.ToLocalChecked();
    ReturnValue<Value> retVal = args.GetReturnValue();
    retVal.Set(checkedStr);
}

// Add addons must export an initialization function
void Initialize(Local<Object> exports) {
    NODE_SET_METHOD(exports, "hello", Hello);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize) // no semi-colon here!

} // namespace demo
