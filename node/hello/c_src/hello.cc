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

void Sum(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    double sum = 0.0;
    for (int i = 0; i< args.Length(); ++i) {
        if (args[i]->IsNumber()) {
            sum += args[i].As<Number>()->Value();
        } else {
            Local<String> argType = args[i]->TypeOf(isolate);

            const unsigned typeBufSz = 256;
            char typeBuf[typeBufSz];
            argType->WriteUtf8(isolate, typeBuf, typeBufSz);

            const unsigned msgBufSz = 1024;
            char msgBuf[msgBufSz];
            snprintf(msgBuf, msgBufSz, "Expected %s, but saw %s", "number", typeBuf);

            Local<String> msg = String::NewFromUtf8(
                isolate, msgBuf, NewStringType::kNormal).ToLocalChecked();
            isolate->ThrowException(Exception::TypeError(msg));
            return;
        }
    }

    Local<Number> res = Number::New(isolate, sum);
    ReturnValue<Value> retVal = args.GetReturnValue();
    retVal.Set(res);
}

// Add addons must export an initialization function
void Initialize(Local<Object> exports) {
    NODE_SET_METHOD(exports, "hello", Hello);
    NODE_SET_METHOD(exports, "sum", Sum);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize) // no semi-colon here!

} // namespace demo
