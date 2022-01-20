#include <node.h>
#include <iostream>
#include <cmath>     // NAN
#include <algorithm> // std::reverse

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
    double val = 0;
    Maybe<double> mbVal = args[0]->NumberValue(context);
    if (mbVal.IsJust()) {
        val = mbVal.FromJust();
    }

    //std::cout << val << std::endl;
    args.GetReturnValue().Set(val + 42);
}

void PassInt32(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    Maybe<int32_t> mbVal = args[0]->Int32Value(context);
    if (mbVal.IsJust()) {
        args.GetReturnValue().Set(mbVal.FromJust() + 42);
        return;
    }

    args.GetReturnValue().Set(NAN);
}
    
void PassBoolean(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    bool val = args[0]->BooleanValue(isolate);
    //std::cout << val << std::endl;

    args.GetReturnValue().Set(val);
}

void PassString(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    if (args[0]->IsNull()) {
        args.GetReturnValue().Set(Null(isolate));
        return;
    }

    if (args[0]->IsUndefined()) {
        // Will return undefined
        return;
    }

    // Catches Object, Array, Function, Regex
    if (args[0]->IsObject()) {
        args.GetReturnValue().Set(args[0]);
        return;
    }

    String::Utf8Value s(isolate, args[0]);

    std::string str(*s);
    std::reverse(str.begin(), str.end());

    Local<String> ret = String::NewFromUtf8(
        isolate, str.c_str(), NewStringType::kNormal).ToLocalChecked();

    args.GetReturnValue().Set(ret);
}

void PassObject(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    Local<Object> obj = args[0].As<Object>();

    Local<String> prop_y = String::NewFromUtf8(
        isolate, "y", NewStringType::kNormal).ToLocalChecked();

    Local<String> prop_z = String::NewFromUtf8(
        isolate, "z", NewStringType::kNormal).ToLocalChecked();

    double y;
    if (obj->Get(context, prop_y).IsEmpty()) {
        //printf("Is empty\n");
        y = 0;
    } else {
        //y = obj->Get(context, prop_y).ToLocalChecked().As<Number>()->Value();
        Local<Value> val = obj->Get(context, prop_y).ToLocalChecked();
        if (val->IsUndefined()) {
            // Key doesn't exit
            //printf("Is undefined\n");
            y = 0;
        } else {
            // Still converts string -> NAN
            Maybe<double> mbDbl = val->NumberValue(context);
            if (mbDbl.IsNothing()) {
                //printf("Is nothing\n");
                y = 0;
            } else {
                y = mbDbl.FromJust();
                //printf("y = %f\n", y);
            }
        }
    }

    obj->Set(context, prop_y, Number::New(isolate, y + 42));
    obj->Set(context, prop_z, Number::New(isolate, -1));
    args.GetReturnValue().Set(obj);
}

// All addons must export an initialization function
void Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "passNumber", PassNumber);
    NODE_SET_METHOD(exports, "passInt32", PassInt32);
    NODE_SET_METHOD(exports, "passBoolean", PassBoolean);
    NODE_SET_METHOD(exports, "passString", PassString);
    NODE_SET_METHOD(exports, "passObject", PassObject);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init) // no semi-colon here!

} // namespace demo
