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

void Add(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    if (args.Length() < 2) {
        isolate->ThrowException(
            Exception::TypeError(
                String::NewFromUtf8(
                    isolate,
                    "Wrong number of arguments",
                    NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
        isolate->ThrowException(
            Exception::TypeError(
                String::NewFromUtf8(
                    isolate,
                    "Wrong arguments",
                    NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    double sum = args[0].As<Number>()->Value()
               + args[1].As<Number>()->Value();

    Local<Number> res = Number::New(isolate, sum);
    args.GetReturnValue().Set(res);
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

    // Without Local<Number> handler
    args.GetReturnValue().Set(sum);
}

void CallFun0(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    Local<Function> cb = Local<Function>::Cast(args[0]);
    cb->Call(context, Null(isolate), 0, nullptr).ToLocalChecked();

    // Will return undefined
    return;
}

void CallFun1(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    Local<Function> cb = Local<Function>::Cast(args[0]);
    const unsigned argc = 1;
    Local<Value> argv[argc] = { args[1] };
    Local<Value> res = cb->Call(
        context, Null(isolate), argc, argv).ToLocalChecked();

    // Return what callback returned
    args.GetReturnValue().Set(res);
}

// All addons must export an initialization function
void Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "hello", Hello);
    NODE_SET_METHOD(exports, "add", Add);
    NODE_SET_METHOD(exports, "sum", Sum);
    NODE_SET_METHOD(exports, "callFun0", CallFun0);
    NODE_SET_METHOD(exports, "callFun1", CallFun1);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init) // no semi-colon here!

} // namespace demo
