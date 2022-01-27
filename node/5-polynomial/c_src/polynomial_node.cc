#include <node.h>
#include <node_object_wrap.h>

#include <cmath>

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
    explicit WrappedPoly(double a = 0, double b = 0, double c = 0) : a_(a), b_(b), c_(c) {}
    ~WrappedPoly() {}

    static void New(const FunctionCallbackInfo<Value>& info);

    static void GetProp(Local<String> prop, const PropertyCallbackInfo<Value>& info);
    static void SetProp(Local<String> prop, Local<Value> value, const PropertyCallbackInfo<void>& info);

    static void At(const FunctionCallbackInfo<Value>& info);
    static void Roots(const FunctionCallbackInfo<Value>& info);

    static Persistent<Function> constructor;
    double a_;
    double b_;
    double c_;
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
    exports->Set(context, NEW_STR("Polynomial"), cons);
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
    WrappedPoly* poly = ObjectWrap::Unwrap<WrappedPoly>(info.This());

    String::Utf8Value s(isolate, prop);
    std::string str(*s);

    if (str == "a") {
        info.GetReturnValue().Set(poly->a_);
    } else if (str == "b") {
        info.GetReturnValue().Set(poly->b_);
    } else if (str == "c") {
        info.GetReturnValue().Set(poly->c_);
    }
}

void WrappedPoly::SetProp(Local<String> prop,
                       Local<Value> value,
                       const PropertyCallbackInfo<void>& info) {
    Isolate* isolate = info.GetIsolate();
    WrappedPoly* poly = ObjectWrap::Unwrap<WrappedPoly>(info.This());

    String::Utf8Value s(isolate, prop);
    std::string str(*s);

    if (str == "a") {
        poly->a_ = value.As<Number>()->Value();
    } else if (str == "b") {
        poly->b_ = value.As<Number>()->Value();
    } else if (str == "c") {
        poly->c_ = value.As<Number>()->Value();
    }
}

void WrappedPoly::At(const FunctionCallbackInfo<Value>& info) {
    double x = info[0]->IsUndefined() ? 0 : info[0].As<Number>()->Value();
    WrappedPoly* poly = ObjectWrap::Unwrap<WrappedPoly>(info.Holder());

    double results = x * x * poly->a_ + x * poly->b_ + poly->c_;

    info.GetReturnValue().Set(results);
}

void WrappedPoly::Roots(const FunctionCallbackInfo<Value>& info) {
    Isolate* isolate = info.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    WrappedPoly* poly = ObjectWrap::Unwrap<WrappedPoly>(info.Holder());

    Local<Array> roots = Array::New(isolate);

    double desc = poly->b_ * poly->b_ - (4 * poly->a_ * poly->c_);
    if (desc >= 0) {
        double r = (-poly->b_ + sqrt(desc)) / (2 * poly->a_);
        roots->Set(context, 0, NEW_NUM(r));
        if ( desc > 0) {
            r = (-poly->b_ - sqrt(desc)) / (2 * poly->a_);
            roots->Set(context, 1, NEW_NUM(r));
        }
    }

    info.GetReturnValue().Set(roots);
}

void WrappedPoly::Add(const FunctionCallbackInfo<Value>& info) {
    Isolate* isolate = info.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    WrappedPoly* obj1 = ObjectWrap::Unwrap<WrappedPoly>(info[0].As<Object>());
    WrappedPoly* obj2 = ObjectWrap::Unwrap<WrappedPoly>(info[1].As<Object>());

    double a = obj1->a_ + obj2->a_;
    double b = obj1->b_ + obj2->b_;
    double c = obj1->c_ + obj2->c_;

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
