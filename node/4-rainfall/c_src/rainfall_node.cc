#include <node.h>
#include <uv.h>

#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>

#include "rainfall.h"

using v8::Array;
using v8::Context;
using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::HandleScope;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::NewStringType;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;

#define NEW_STR(str) String::NewFromUtf8(isolate, str, NewStringType::kNormal).ToLocalChecked()

Sample unpack_sample(Isolate* isolate, const Local<Context>& context, Local<Object>& Obj) {
    Sample s;

    Local<Value> Date = Obj->Get(context, NEW_STR("date")).ToLocalChecked();
    Local<Value> Rainfall = Obj->Get(context, NEW_STR("rainfall")).ToLocalChecked();

    String::Utf8Value str(isolate, Date);
    s.date = std::string(*str);
    s.rainfall = Rainfall.As<Number>()->Value();

    return s;
}

Location unpack_location(Isolate* isolate, const Local<Context>& context, const Local<Object>& Obj) {
    Location loc;

    Local<Value> Lat = Obj->Get(context, NEW_STR("latitude")).ToLocalChecked();
    Local<Value> Lon = Obj->Get(context, NEW_STR("longitude")).ToLocalChecked();
    loc.latitude = Lat.As<Number>()->Value();
    loc.longitude = Lon.As<Number>()->Value();

    Local<Array> Arr = Local<Array>::Cast(Obj->Get(context, NEW_STR("samples")).ToLocalChecked());
    size_t len = Arr->Length();
    for (size_t i = 0; i < len; ++i) {
        Local<Object> Item = Arr->Get(context, i).ToLocalChecked().As<Object>();
        Sample s = unpack_sample(isolate, context, Item);
        loc.samples.push_back(s);
    }

    return loc;
}

void calc_results(std::vector<Location>& locations, std::vector<RainResult>& results) {
    results.resize(locations.size());
    std::transform(
        locations.begin(),
        locations.end(),
        results.begin(),
        calc_rain_stats);
}

Local<Object> pack_rain_result(Isolate* isolate, const Local<Context>& context, const RainResult& result) {
    Local<Object> Obj = Object::New(isolate);

    Obj->Set(context, NEW_STR("mean"), Number::New(isolate, result.mean));
    Obj->Set(context, NEW_STR("median"), Number::New(isolate, result.median));
    Obj->Set(context, NEW_STR("stdev"), Number::New(isolate, result.stdev));
    Obj->Set(context, NEW_STR("n"), Integer::New(isolate, result.n));

    return Obj;
}

void AvgRainfall(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    Local<Object> Obj = Local<Object>::Cast(args[0]);
    Location loc = unpack_location(isolate, context, Obj);
    double avg = avg_rainfall(loc);

    args.GetReturnValue().Set(avg);
}

void RainfallData(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    Local<Object> Obj = Local<Object>::Cast(args[0]);
    Location loc = unpack_location(isolate, context, Obj);
    RainResult result = calc_rain_stats(loc);

    Local<Object> Result = pack_rain_result(isolate, context, result);
    args.GetReturnValue().Set(Result);
}

void CalcResults(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    std::vector<Location> locations;

    Local<Array> Input = Local<Array>::Cast(args[0]);
    size_t len = Input->Length();
    for (size_t i = 0; i < len; ++i) {
        Local<Object> Item = Input->Get(context, i).ToLocalChecked().As<Object>();
        locations.push_back(unpack_location(isolate, context, Item));
    }

    std::vector<RainResult> results;
    calc_results(locations, /*out*/results);

    Local<Array> Results = Array::New(isolate);
    for (unsigned i = 0; i < len; ++i) {
        Local<Object> Result = pack_rain_result(isolate, context, results[i]);
        Results->Set(context, i, Result);
    }

    args.GetReturnValue().Set(Results);
}

void CalcResultsSync(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    if (args.Length() < 2) {
        isolate->ThrowException(
            Exception::TypeError(
                String::NewFromUtf8(
                    isolate,
                    "Wrong number of arguments",
                    NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    // Unpack locations
    std::vector<Location> locations;

    Local<Array> Input = Local<Array>::Cast(args[0]);
    size_t len = Input->Length();
    for (size_t i = 0; i < len; ++i) {
        Local<Object> Item = Input->Get(context, i).ToLocalChecked().As<Object>();
        locations.push_back(unpack_location(isolate, context, Item));
    }

    // Callback
    Local<Function> callback = Local<Function>::Cast(args[1]);

    // Calc results
    std::vector<RainResult> results;
    calc_results(locations, /*out*/results);

    // Pack results
    Local<Array> Results = Array::New(isolate);
    for (unsigned i = 0; i < len; ++i) {
        Local<Object> Result = pack_rain_result(isolate, context, results[i]);
        Results->Set(context, i, Result);
    }

    // Call callback
    Local<Value> argv[] = { Results };
    const unsigned argc = sizeof(argv) / sizeof(argv[0]);
    callback->Call(context, Null(isolate), argc, argv).ToLocalChecked();

    std::cerr << "Returning from C++ now" << std::endl;
    args.GetReturnValue().Set(Undefined(isolate));
}

struct Work {
    uv_work_t request;
    Persistent<Function> callback;

    std::vector<Location> locations;
    std::vector<RainResult> results;
};

static void WorkAsync(uv_work_t* req) {
    Work* work = static_cast<Work *>(req->data);
    std::vector<Location>& locations = work->locations;
    std::vector<RainResult>& results = work->results;

    // Calc results
    calc_results(locations, /*out*/results);

    // Pretend a lot of work
    std::this_thread::sleep_for(chrono::seconds(1));
}

static void WorkAsyncDone(uv_work_t* req, int status) {
    HandleScope handleScope(Isolate::GetCurrent());
    Isolate* isolate = handleScope.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    Work* work = static_cast<Work *>(req->data);
    std::vector<RainResult>& results = work->results;
    Local<Function> callback = Local<Function>::New(isolate, work->callback);

    // Pack results
    Local<Array> Results = Array::New(isolate);
    size_t len = results.size();
    for (unsigned i = 0; i < len; ++i) {
        Local<Object> Result = pack_rain_result(isolate, context, results[i]);
        Results->Set(context, i, Result);
    }

    // Call callback
    Local<Value> argv[] = { Null(isolate), Results };
    const unsigned argc = sizeof(argv) / sizeof(argv[0]);
    callback->Call(context, Null(isolate), argc, argv).ToLocalChecked();

    // Cleanup
    work->callback.Reset();
    delete work;
}

void CalcResultsAsync(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    if (args.Length() < 2) {
        isolate->ThrowException(
            Exception::TypeError(
                String::NewFromUtf8(
                    isolate,
                    "Wrong number of arguments",
                    NewStringType::kNormal).ToLocalChecked()));
        return;
    }

    // Unpack locations
    std::vector<Location> locations;

    Local<Array> Input = Local<Array>::Cast(args[0]);
    size_t len = Input->Length();
    for (size_t i = 0; i < len; ++i) {
        Local<Object> Item = Input->Get(context, i).ToLocalChecked().As<Object>();
        locations.push_back(unpack_location(isolate, context, Item));
    }

    // Callback
    Local<Function> callback = Local<Function>::Cast(args[1]);

    // Spawn worker thread
    Work* work = new Work();
    work->request.data = work;
    work->locations = locations;
    work->callback.Reset(isolate, callback);

    uv_queue_work(uv_default_loop(), &work->request, WorkAsync, WorkAsyncDone);

    std::cerr << "Returning from C++ now" << std::endl;
    args.GetReturnValue().Set(Undefined(isolate));
}

void Init(Local<Object> exports, Local<Value> module, void* context) {
    NODE_SET_METHOD(exports, "avgRainfall", AvgRainfall);
    NODE_SET_METHOD(exports, "dataRainfall", RainfallData);
    NODE_SET_METHOD(exports, "calcResults", CalcResults);
    NODE_SET_METHOD(exports, "calcResultsSync", CalcResultsSync);
    NODE_SET_METHOD(exports, "calcResultsAsync", CalcResultsAsync);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
