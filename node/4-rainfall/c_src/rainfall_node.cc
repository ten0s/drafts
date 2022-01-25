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

Sample unpack_sample(Isolate* isolate,
                     const Local<Context>& context,
                     const Local<Object>& obj) {
    Sample s;

    Local<Value> date = obj->Get(context, NEW_STR("date")).ToLocalChecked();
    Local<Value> rainfall = obj->Get(context, NEW_STR("rainfall")).ToLocalChecked();

    String::Utf8Value str(isolate, date);
    s.date = std::string(*str);
    s.rainfall = rainfall.As<Number>()->Value();

    return s;
}

Location unpack_location(Isolate* isolate,
                         const Local<Context>& context,
                         const Local<Object>& obj) {
    Location loc;

    Local<Value> lat = obj->Get(context, NEW_STR("latitude")).ToLocalChecked();
    Local<Value> lon = obj->Get(context, NEW_STR("longitude")).ToLocalChecked();
    loc.latitude = lat.As<Number>()->Value();
    loc.longitude = lon.As<Number>()->Value();

    Local<Array> arr = Local<Array>::Cast(obj->Get(context, NEW_STR("samples")).ToLocalChecked());
    size_t len = arr->Length();
    for (size_t i = 0; i < len; ++i) {
        Local<Object> item = arr->Get(context, i).ToLocalChecked().As<Object>();
        Sample s = unpack_sample(isolate, context, item);
        loc.samples.push_back(s);
    }

    return loc;
}

void unpack_locations(Isolate* isolate,
                      const Local<Context>& context,
                      const Local<Array>& input,
                      std::vector<Location>& locations) {
    size_t len = input->Length();
    for (size_t i = 0; i < len; ++i) {
        Local<Object> item = input->Get(context, i).ToLocalChecked().As<Object>();
        locations.push_back(unpack_location(isolate, context, item));
    }
}

void calc_results(std::vector<Location>& locations,
                  std::vector<RainResult>& results) {
    results.resize(locations.size());
    std::transform(
        locations.begin(),
        locations.end(),
        results.begin(),
        calc_rain_stats);
}

Local<Object> pack_rain_result(Isolate* isolate,
                               const Local<Context>& context,
                               const RainResult& result) {
    Local<Object> obj = Object::New(isolate);

    obj->Set(context, NEW_STR("mean"), Number::New(isolate, result.mean));
    obj->Set(context, NEW_STR("median"), Number::New(isolate, result.median));
    obj->Set(context, NEW_STR("stdev"), Number::New(isolate, result.stdev));
    obj->Set(context, NEW_STR("n"), Integer::New(isolate, result.n));

    return obj;
}

Local<Array> pack_results(Isolate* isolate,
                          const Local<Context>& context,
                          const std::vector<RainResult>& results) {
    Local<Array> arr = Array::New(isolate);

    size_t len = results.size();
    for (unsigned i = 0; i < len; ++i) {
        Local<Object> obj = pack_rain_result(isolate, context, results[i]);
        arr->Set(context, i, obj);
    }

    return arr;
}

void AvgRainfall(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    // Args
    Local<Object> obj = Local<Object>::Cast(args[0]);

    // Unpack location
    Location loc = unpack_location(isolate, context, obj);

    // Calc
    double avg = avg_rainfall(loc);

    // Return
    args.GetReturnValue().Set(avg);
}

void RainfallData(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    // Args
    Local<Object> obj = Local<Object>::Cast(args[0]);

    // Unpack location
    Location loc = unpack_location(isolate, context, obj);

    // Calc result
    RainResult result = calc_rain_stats(loc);

    // Pack result
    Local<Object> output = pack_rain_result(isolate, context, result);

    // Return
    args.GetReturnValue().Set(output);
}

void CalcResults(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    // Args
    Local<Array> input = Local<Array>::Cast(args[0]);

    // Unpack locations
    std::vector<Location> locations;
    unpack_locations(isolate, context, input, locations);

    // Calc results
    std::vector<RainResult> results;
    calc_results(locations, /*out*/results);

    // Pack results
    Local<Array> output = pack_results(isolate, context, results);

    // Return
    args.GetReturnValue().Set(output);
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

    // Args
    Local<Array> input = Local<Array>::Cast(args[0]);
    Local<Function> callback = Local<Function>::Cast(args[1]);

    // Unpack locations
    std::vector<Location> locations;
    unpack_locations(isolate, context, input, locations);

    // Calc results
    std::vector<RainResult> results;
    calc_results(locations, /*out*/results);

    // Pack results
    Local<Array> output = pack_results(isolate, context, results);

    // Call callback
    Local<Value> argv[] = { output };
    const unsigned argc = sizeof(argv) / sizeof(argv[0]);
    callback->Call(context, Null(isolate), argc, argv).ToLocalChecked();

    // Return `undefined`
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
    Local<Array> output = pack_results(isolate, context, results);

    // Call callback
    Local<Value> argv[] = { Null(isolate), output };
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

    // Args
    Local<Array> input = Local<Array>::Cast(args[0]);
    Local<Function> callback = Local<Function>::Cast(args[1]);

    // Unpack locations
    std::vector<Location> locations;
    unpack_locations(isolate, context, input, locations);

    // Spawn worker thread
    Work* work = new Work();
    work->request.data = work;
    work->locations = locations;
    work->callback.Reset(isolate, callback);

    uv_queue_work(uv_default_loop(), &work->request, WorkAsync, WorkAsyncDone);

    // Return `undefined` immediately
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
