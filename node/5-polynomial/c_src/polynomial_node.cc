#include <node.h>
#include <node_object_wrap.h>

#include "polynomial.h"

using v8::Array;
using v8::Context;
using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::NewStringType;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::PropertyCallbackInfo;
using v8::String;
using v8::Value;

using node::ObjectWrap;

#define NEW_STR(str) String::NewFromUtf8(isolate, str, NewStringType::kNormal).ToLocalChecked()
#define NEW_NUM(num) Number::New(isolate, num)

class WrappedPoly : public ObjectWrap {
public:
    static void Init(Local<Object> exports);

    static void Add(const FunctionCallbackInfo<Value>& info);

private:
    explicit WrappedPoly(double a = 0, double b = 0, double c = 0) {
        poly_ = new Polynomial(a, b, c);
    }

    ~WrappedPoly() {
        delete poly_;
    }

    static void New(const FunctionCallbackInfo<Value>& info);

    static void GetProp(Local<String> prop, const PropertyCallbackInfo<Value>& info);
    static void SetProp(Local<String> prop, Local<Value> value, const PropertyCallbackInfo<void>& info);

    static void At(const FunctionCallbackInfo<Value>& info);
    static void Roots(const FunctionCallbackInfo<Value>& info);

    static Persistent<Function> constructor;

    Polynomial* poly_;
};

Persistent<Function> WrappedPoly::constructor;

void WrappedPoly::Init(Local<Object> exports) {
    Isolate* isolate = exports->GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(NEW_STR("Polynomial"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add functions to the object prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "at", At);
    NODE_SET_PROTOTYPE_METHOD(tpl, "roots", Roots);

    // Add properties
    tpl->InstanceTemplate()->SetAccessor(NEW_STR("a"), GetProp, SetProp);
    tpl->InstanceTemplate()->SetAccessor(NEW_STR("b"), GetProp, SetProp);
    tpl->InstanceTemplate()->SetAccessor(NEW_STR("c"), GetProp, SetProp);

    // Don't move it up!!!
    Local<Function> cons = tpl->GetFunction(context).ToLocalChecked();
    // Store a reference to the constructor, so it can be called in New
    // if New is called without construct syntax, like
    // const poly = Polynomial(...) instead of
    // const poly = new Polynomial(...)
    constructor.Reset(isolate, cons);

    // Export the constructor
    exports->Set(context, NEW_STR("Polynomial"), cons).IsJust();
}

void WrappedPoly::New(const FunctionCallbackInfo<Value>& info) {
    Isolate* isolate = info.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    if (info.IsConstructCall()) {
        // Invoked as constructor: new Polynomial(...)
        double a = info[0]->IsUndefined() ? 0 : info[0].As<Number>()->Value();
        double b = info[1]->IsUndefined() ? 0 : info[1].As<Number>()->Value();
        double c = info[2]->IsUndefined() ? 0 : info[2].As<Number>()->Value();

        WrappedPoly* obj = new WrappedPoly(a, b, c);
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    } else {
        // Invoked as function: Polynomial(...),
        // turn into construct call
        Local<Value> argv[] = { info[0], info[1], info[2] };
        const size_t argc = sizeof(argv) / sizeof(argv[1]);
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        info.GetReturnValue().Set(cons->NewInstance(context, argc, argv).ToLocalChecked());
    }
}

void WrappedPoly::GetProp(Local<String> prop,
                       const PropertyCallbackInfo<Value>& info) {
    Isolate* isolate = info.GetIsolate();
    WrappedPoly* obj = ObjectWrap::Unwrap<WrappedPoly>(info.This());

    String::Utf8Value s(isolate, prop);
    std::string str(*s);

    if (str == "a") {
        info.GetReturnValue().Set(obj->poly_->getA());
    } else if (str == "b") {
        info.GetReturnValue().Set(obj->poly_->getB());
    } else if (str == "c") {
        info.GetReturnValue().Set(obj->poly_->getC());
    }
}

void WrappedPoly::SetProp(Local<String> prop,
                       Local<Value> value,
                       const PropertyCallbackInfo<void>& info) {
    Isolate* isolate = info.GetIsolate();
    WrappedPoly* obj = ObjectWrap::Unwrap<WrappedPoly>(info.This());

    String::Utf8Value s(isolate, prop);
    std::string str(*s);

    if (str == "a") {
        obj->poly_->setA(value.As<Number>()->Value());
    } else if (str == "b") {
        obj->poly_->setB(value.As<Number>()->Value());
    } else if (str == "c") {
        obj->poly_->setC(value.As<Number>()->Value());
    }
}

void WrappedPoly::At(const FunctionCallbackInfo<Value>& info) {
    WrappedPoly* obj = ObjectWrap::Unwrap<WrappedPoly>(info.Holder());

    double x = info[0]->IsUndefined() ? 0 : info[0].As<Number>()->Value();
    double results = obj->poly_->at(x);

    info.GetReturnValue().Set(results);
}

void WrappedPoly::Roots(const FunctionCallbackInfo<Value>& info) {
    Isolate* isolate = info.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    WrappedPoly* obj = ObjectWrap::Unwrap<WrappedPoly>(info.Holder());
    Local<Array> results = Array::New(isolate);

    std::vector<double> roots = obj->poly_->roots();
    const size_t len = roots.size();
    for (size_t i = 0; i < len; ++i) {
        results->Set(context, i, NEW_NUM(roots[i])).IsJust();
    }

    info.GetReturnValue().Set(results);
}

void WrappedPoly::Add(const FunctionCallbackInfo<Value>& info) {
    Isolate* isolate = info.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    WrappedPoly* obj1 = ObjectWrap::Unwrap<WrappedPoly>(info[0].As<Object>());
    WrappedPoly* obj2 = ObjectWrap::Unwrap<WrappedPoly>(info[1].As<Object>());

    double a = obj1->poly_->getA() + obj2->poly_->getA();
    double b = obj1->poly_->getB() + obj2->poly_->getB();
    double c = obj1->poly_->getC() + obj2->poly_->getC();

    Local<Value> argv[] = { NEW_NUM(a), NEW_NUM(b), NEW_NUM(c) };
    const size_t argc = sizeof(argv) / sizeof(argv[0]);

    Local<Function> cons = Local<Function>::New(isolate, constructor);
    info.GetReturnValue().Set(cons->NewInstance(context, argc, argv).ToLocalChecked());
}

void Init(Local<Object> exports, Local<Value> module, void* context) {
    WrappedPoly::Init(exports);
    NODE_SET_METHOD(exports, "add", WrappedPoly::Add);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
