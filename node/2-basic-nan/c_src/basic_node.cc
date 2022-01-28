#include <nan.h>

using namespace v8;

#define NEW_FUN(fun) Nan::GetFunction(Nan::New<v8::FunctionTemplate>(fun)).ToLocalChecked()
#define NEW_NUM(num) Nan::New<v8::Number>(num)
#define NEW_STR(str) Nan::New<v8::String>(str).ToLocalChecked()

NAN_METHOD(Add) {
    if (info.Length() < 2) {
        Nan::ThrowError(
            Nan::TypeError(NEW_STR("Wrong number of arguments")));
        return;
    }

    if (!info[0]->IsNumber() || !info[1]->IsNumber()) {
        Nan::ThrowError(
            Nan::TypeError(NEW_STR("Wrong arguments")));
        return;
    }

    double sum = info[0].As<Number>()->Value()
               + info[1].As<Number>()->Value();

    Local<Number> res = NEW_NUM(sum);
    info.GetReturnValue().Set(res);
}

NAN_METHOD(Sum) {
    Isolate* isolate = info.GetIsolate();

    double sum = 0.0;
    size_t len = info.Length();
    for (size_t i = 0; i< len; ++i) {
        if (info[i]->IsNumber()) {
            sum += info[i].As<Number>()->Value();
        } else {
            Local<String> argType = info[i]->TypeOf(isolate);

            const unsigned typeBufSz = 256;
            char typeBuf[typeBufSz];
            argType->WriteUtf8(isolate, typeBuf, typeBufSz);

            const unsigned msgBufSz = 1024;
            char msgBuf[msgBufSz];
            snprintf(msgBuf, msgBufSz, "Expected %s, but saw %s", "number", typeBuf);

            Nan::ThrowError(
                Nan::TypeError(NEW_STR(msgBuf)));
            return;
        }
    }

    // Without Local<Number> handler
    info.GetReturnValue().Set(sum);
}

NAN_METHOD(CallFun0) {
    Nan::Callback cb(info[0].As<Function>());
    Nan::Call(cb, 0, nullptr);

    // Will return undefined
    return;
}

NAN_METHOD(CallFun1) {
    Nan::Callback cb(info[0].As<Function>());
    const unsigned argc = 1;
    Local<Value> argv[argc] = { info[1] };
    Local<Value> res = Nan::Call(cb, argc, argv).ToLocalChecked();

    // Return what callback returned
    info.GetReturnValue().Set(res);
}

NAN_MODULE_INIT(Init) {
    Nan::Set(target, NEW_STR("add"), NEW_FUN(Add));
    Nan::Set(target, NEW_STR("sum"), NEW_FUN(Sum));
    Nan::Set(target, NEW_STR("callFun0"), NEW_FUN(CallFun0));
    Nan::Set(target, NEW_STR("callFun1"), NEW_FUN(CallFun1));
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
