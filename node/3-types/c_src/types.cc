#include <node.h>
#include <iostream>
#include <cmath> // NAN

namespace demo {

using namespace v8;

void PassNumber(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    if (args.Length() < 1) {
        args.GetReturnValue().Set(0);
        return;
    }

    //double num = args[0].As<Number>()->Value();
    double num = 0;
    Maybe<double> mbNum = args[0]->NumberValue(context);
    if (mbNum.IsJust()) {
        num = mbNum.FromJust();
    }

    //std::cout << num << std::endl;
    args.GetReturnValue().Set(num + 42);
}

void PassInt32(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    Maybe<int32_t> mbI32 = args[0]->Int32Value(context);
    if (mbI32.IsJust()) {
        args.GetReturnValue().Set(mbI32.FromJust() + 42);
        return;
    }

    args.GetReturnValue().Set(NAN);
}
    
void PassBoolean(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    bool bul = args[0]->BooleanValue(isolate);
    //std::cout << bul << std::endl;

    args.GetReturnValue().Set(bul);
}

void Mutate(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    Local<Object> target = args[0].As<Object>();
    target->Set(context,
                String::NewFromUtf8(
                    isolate, "y", NewStringType::kNormal).ToLocalChecked(),
                Number::New(isolate, 42));

    args.GetReturnValue().Set(target);
}

// All addons must export an initialization function
void Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "passNumber", PassNumber);
    NODE_SET_METHOD(exports, "passInt32", PassInt32);
    NODE_SET_METHOD(exports, "passBoolean", PassBoolean);
    NODE_SET_METHOD(exports, "mutate", Mutate);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init) // no semi-colon here!

} // namespace demo
