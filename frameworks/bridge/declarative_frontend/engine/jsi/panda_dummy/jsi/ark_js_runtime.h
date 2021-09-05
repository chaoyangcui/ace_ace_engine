/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_RUNTIME_ECMASCRIPT_NAPI_JSI_ARK_JS_RUNTIME_H
#define PANDA_RUNTIME_ECMASCRIPT_NAPI_JSI_ARK_JS_RUNTIME_H

#include <memory>

#include "frameworks/bridge/declarative_frontend/engine/jsi/panda_dummy/include/jsnapi.h"
#include "frameworks/bridge/declarative_frontend/engine/jsi/panda_dummy/jsi/include/js_runtime.h"

namespace panda::ecmascript {
class EcmaVM;
}  // namespace panda::ecmascript

// NOLINTNEXTLINE(readability-identifier-naming)
namespace OHOS::Ace::Framework {
using panda::ArrayRef;
using panda::BooleanRef;
using panda::EscapeLocalScope;
using panda::FunctionRef;
using panda::Global;
using panda::IntegerRef;
using panda::JSExecutionScope;
using panda::JSNApi;
using panda::JSON;
using panda::JSValueRef;
using panda::Local;
using panda::LocalScope;
using panda::NumberRef;
using panda::ObjectRef;
using panda::RuntimeOption;
using panda::StringRef;
using panda::ecmascript::EcmaVM;
class PandaFunctionData;

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ArkJSRuntime final : public JsRuntime, public std::enable_shared_from_this<ArkJSRuntime> {
public:
    static shared_ptr<JsRuntime> GetInstance()
    {
        shared_ptr<ArkJSRuntime> runtime = std::make_shared<ArkJSRuntime>();
        new shared_ptr<ArkJSRuntime>(runtime);
        return runtime;
    }
    bool StartDebugger(const char *libraryPath, EcmaVM *vm);
    bool Initialize(const std::string &libraryPath) override;
    void Reset() override;
    void SetLogPrint(LOG_PRINT out) override;
    shared_ptr<JsValue> EvaluateJsCode(const std::string &src) override;
    bool EvaluateJsCode(const uint8_t *buffer, int32_t size) override;
    bool ExecuteJsBin(const std::string &fileName) override;
    shared_ptr<JsValue> GetGlobal() override;
    void RunGC() override;

    shared_ptr<JsValue> NewNumber(double d) override;
    shared_ptr<JsValue> NewInt32(int32_t value) override;
    shared_ptr<JsValue> NewBoolean(bool value) override;
    shared_ptr<JsValue> NewNull() override;
    shared_ptr<JsValue> NewUndefined() override;
    shared_ptr<JsValue> NewString(const std::string &str) override;
    shared_ptr<JsValue> ParseJson(const std::string &str) override;
    shared_ptr<JsValue> NewObject() override;
    shared_ptr<JsValue> NewArray() override;
    shared_ptr<JsValue> NewFunction(RegisterFunctionType func) override;
    void RegisterUncaughtExceptionHandler(UncaughtExceptionCallback callback) override;
    void HandleUncaughtException() override;

    EcmaVM *GetEcmaVm() const override
    {
        return vm_;
    }

private:
    EcmaVM *vm_ = nullptr;
    std::vector<PandaFunctionData *> dataList_;
    LOG_PRINT print_ { nullptr };
    UncaughtExceptionCallback uncaughtErrorHandler_ { nullptr };
    std::string libPath_ {};
    int debuggerOrder_ { 0 };
};

class PandaFunctionData {
public:
    PandaFunctionData(shared_ptr<ArkJSRuntime> runtime, RegisterFunctionType func)
        : runtime_(std::move(runtime)), func_(std::move(func))
    {
    }

    ~PandaFunctionData() = default;

    PandaFunctionData(const PandaFunctionData&) = delete;
    void operator=(const PandaFunctionData&) = delete;
    PandaFunctionData(PandaFunctionData&&) = delete;
    PandaFunctionData& operator=(PandaFunctionData&&) = delete;

private:
    Local<JSValueRef> Callback(const Local<JSValueRef> &thisValue,
                               const Local<JSValueRef> argument[],  // NOLINT(modernize-avoid-c-arrays)
                               int32_t length) const;
    shared_ptr<ArkJSRuntime> runtime_;
    RegisterFunctionType func_;
    friend Local<JSValueRef> FunctionCallback(EcmaVM *vm, Local<JSValueRef> thisValue,
                                              const Local<JSValueRef> argument[],  // NOLINT(modernize-avoid-c-arrays)
                                              int32_t length, void *data);
};
}  // namespace OHOS::Ace::Framework
#endif  // PANDA_RUNTIME_ECMASCRIPT_NAPI_JSI_ARK_JS_RUNTIME_H