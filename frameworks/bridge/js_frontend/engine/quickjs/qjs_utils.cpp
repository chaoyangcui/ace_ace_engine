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

#include "frameworks/bridge/js_frontend/engine/quickjs/qjs_utils.h"

#include <sys/types.h>
#include <unistd.h>

#include "base/log/event_report.h"
#include "base/log/log.h"
#include "base/utils/macros.h"
#include "core/common/ace_application_info.h"
#include "core/common/ace_engine.h"

namespace OHOS::Ace::Framework {

namespace {

const char JS_CRASH[] = "Js Crash";
const char DESTROY_APP_ERROR[] = "Destroy application failed";
const char DESTROY_PAGE_FAILED[] = "Destroy page instance failed";
const char LOAD_JS_BUNDLE_FAILED[] = "JS framework load js bundle failed";
const char EVAL_BUFFER_FAILED[] = "Eval buffer failed";
const char READ_OBJECT_FAILED[] = "Read object failed";
const char COMPILE_AND_RUN_BUNDLE_FAILED[] = "Js compile and run bundle failed";
const char LOAD_JS_FRAMEWORK_FAILED[] = "Loading JS framework failed";
const char FIRE_EVENT_FAILED[] = "Fire event failed";

std::string GetReason(JsErrorType errorType)
{
    std::string reasonStr;
    switch (errorType) {
        case OHOS::Ace::Framework::JsErrorType::JS_CRASH:
            reasonStr = JS_CRASH;
            break;
        case OHOS::Ace::Framework::JsErrorType::EVAL_BUFFER_ERROR:
            reasonStr = EVAL_BUFFER_FAILED;
            break;
        case OHOS::Ace::Framework::JsErrorType::DESTROY_APP_ERROR:
            reasonStr = DESTROY_APP_ERROR;
            break;
        case OHOS::Ace::Framework::JsErrorType::DESTROY_PAGE_ERROR:
            reasonStr = DESTROY_PAGE_FAILED;
            break;
        case OHOS::Ace::Framework::JsErrorType::LOAD_JS_BUNDLE_ERROR:
            reasonStr = LOAD_JS_BUNDLE_FAILED;
            break;
        case OHOS::Ace::Framework::JsErrorType::READ_OBJECT_ERROR:
            reasonStr = READ_OBJECT_FAILED;
            break;
        case OHOS::Ace::Framework::JsErrorType::COMPILE_AND_RUN_BUNDLE_ERROR:
            reasonStr = COMPILE_AND_RUN_BUNDLE_FAILED;
            break;
        case OHOS::Ace::Framework::JsErrorType::LOAD_JS_FRAMEWORK_ERROR:
            reasonStr = LOAD_JS_FRAMEWORK_FAILED;
            break;
        case OHOS::Ace::Framework::JsErrorType::FIRE_EVENT_ERROR:
            reasonStr = FIRE_EVENT_FAILED;
            break;
        default:
            reasonStr = JS_CRASH;
            break;
    }

    return reasonStr;
}

std::string GenerateSummaryBody(
    const char* exceptionStr, const char* stackTrace, int32_t instanceId, const char* pageUrl)
{
    std::string abilityName;
    if (AceEngine::Get().GetContainer(instanceId) != nullptr) {
        abilityName = AceEngine::Get().GetContainer(instanceId)->GetHostClassName();
    }

    std::string summaryBody;
    summaryBody.append("Lifetime: ")
        .append(std::to_string(OHOS::Ace::AceApplicationInfo::GetInstance().GetLifeTime()))
        .append("s")
        .append("\n");

    if (!abilityName.empty()) {
        summaryBody.append("Ability: ").append(abilityName).append("\n");
    }

    if (pageUrl != nullptr) {
        summaryBody.append("page: ").append(pageUrl).append("\n");
    }

    summaryBody.append("Js-Engine: Quick JS\n");
    if (exceptionStr == nullptr && stackTrace == nullptr) {
        return summaryBody;
    }

    summaryBody.append("Stacktrace: ");
    if (exceptionStr != nullptr) {
        summaryBody.append(exceptionStr).append("\n");
    }
    if (stackTrace != nullptr) {
        summaryBody.append(stackTrace);
    }

    return summaryBody;
}

} // namespace

thread_local std::stack<QjsHandleScope*> QjsHandleScope::qjsHandleScopeStack;

ScopedString::ScopedString(JSContext* ctx, JSValueConst val) : context_(ctx)
{
    ACE_DCHECK(ctx != nullptr);
    stringValue_ = JS_ToCString(ctx, val);
}

ScopedString::ScopedString(JSContext* ctx, JSAtom atom) : context_(ctx)
{
    ACE_DCHECK(ctx != nullptr);
    stringValue_ = JS_AtomToCString(ctx, atom);
}

ScopedString::ScopedString(JSValueConst val) : context_(QJSContext::current())
{
    stringValue_ = JS_ToCString(context_, val);
}

ScopedString::ScopedString(JSAtom atom) : context_(QJSContext::current())
{
    stringValue_ = JS_AtomToCString(context_, atom);
}

ScopedString::~ScopedString()
{
    JS_FreeCString(context_, stringValue_);
}

bool ScopedString::operator==(const char* rhs)
{
    return std::strcmp(get(), rhs) == 0;
}
bool ScopedString::operator==(const std::string& rhs)
{
    return rhs == get();
}
bool ScopedString::operator==(const ScopedString& rhs)
{
    return std::strcmp(get(), rhs.get()) == 0;
}

const char* ScopedString::get() const
{
    return stringValue_;
}

std::string ScopedString::str()
{
    return stringValue_;
}

std::string ScopedString::Stringify(JSValueConst val)
{
    JSContext* ctx = QJSContext::current();
    JSValue globalObj = JS_GetGlobalObject(ctx);
    JSValue thisObj = JS_GetPropertyStr(ctx, globalObj, "JSON");
    JSValue funcObj = JS_GetPropertyStr(ctx, thisObj, "stringify");
    JSValue retVal = JS_Call(ctx, funcObj, thisObj, 1, &val);
    std::string str = ScopedString(retVal).str();
    JS_FreeValue(ctx, retVal);
    JS_FreeValue(ctx, funcObj);
    JS_FreeValue(ctx, thisObj);
    JS_FreeValue(ctx, globalObj);
    return str;
}

ScopedString::operator std::string() const
{
    return stringValue_;
}

JSValue QjsUtils::NewStringLen(JSContext* ctx, const char* str, size_t len)
{
    ACE_DCHECK(!QjsHandleScope::qjsHandleScopeStack.empty());
    QjsHandleScope* scope = QjsHandleScope::qjsHandleScopeStack.top();
    JSValue retVal = JS_NewStringLen(ctx, str, len);
    scope->jsValues_.push_back(retVal);
    return retVal;
}

JSValue QjsUtils::NewString(JSContext* ctx, const char* str)
{
    ACE_DCHECK(!QjsHandleScope::qjsHandleScopeStack.empty());
    QjsHandleScope* scope = QjsHandleScope::qjsHandleScopeStack.top();
    JSValue retVal = JS_NewString(ctx, str);
    scope->jsValues_.push_back(retVal);
    return retVal;
}

JSValue QjsUtils::ParseJSON(JSContext* ctx, const char* buf, size_t bufLen, const char* filename)
{
    ACE_DCHECK(!QjsHandleScope::qjsHandleScopeStack.empty());
    QjsHandleScope* scope = QjsHandleScope::qjsHandleScopeStack.top();
    JSValue retVal = JS_ParseJSON(ctx, buf, bufLen, filename);
    scope->jsValues_.push_back(retVal);
    return retVal;
}

JSValue QjsUtils::NewObject(JSContext* ctx)
{
    ACE_DCHECK(!QjsHandleScope::qjsHandleScopeStack.empty());
    QjsHandleScope* scope = QjsHandleScope::qjsHandleScopeStack.top();
    JSValue retVal = JS_NewObject(ctx);
    scope->jsValues_.push_back(retVal);
    return retVal;
}

JSValue QjsUtils::Call(JSContext* ctx, JSValueConst funcObj, JSValueConst thisObj, int32_t argc, JSValueConst* argv)
{
    ACE_DCHECK(!QjsHandleScope::qjsHandleScopeStack.empty());
    QjsHandleScope* scope = QjsHandleScope::qjsHandleScopeStack.top();
    JSValue retVal = JS_Call(ctx, funcObj, thisObj, argc, argv);
    scope->jsValues_.push_back(retVal);
    return retVal;
}

JSValue QjsUtils::Eval(JSContext* ctx, const char* input, size_t inputLen, const char* filename, int32_t evalFlags)
{
    ACE_DCHECK(!QjsHandleScope::qjsHandleScopeStack.empty());
    QjsHandleScope* scope = QjsHandleScope::qjsHandleScopeStack.top();
    JSValue retVal = JS_Eval(ctx, input, inputLen, filename, evalFlags);
    scope->jsValues_.push_back(retVal);
    return retVal;
}

JSValue QjsUtils::GetPropertyStr(JSContext* ctx, JSValueConst thisObj, const char* prop)
{
    ACE_DCHECK(!QjsHandleScope::qjsHandleScopeStack.empty());
    QjsHandleScope* scope = QjsHandleScope::qjsHandleScopeStack.top();
    JSValue retVal = JS_GetPropertyStr(ctx, thisObj, prop);
    scope->jsValues_.push_back(retVal);
    return retVal;
}

int32_t QjsUtils::JsGetArrayLength(JSContext* ctx, JSValueConst arrayObject)
{
    int32_t result = JS_IsArray(ctx, arrayObject);
    if (result == TRUE) {
        QjsHandleScope handleScope(ctx);
        JSValue propLength = QjsUtils::GetPropertyStr(ctx, arrayObject, "length");
        int32_t length = -1;

        if (JS_ToInt32(ctx, &length, propLength) < 0) {
            length = -1;
        }

        JS_FreeValue(ctx, propLength);
        return length;
    }
    if (result < 0) {
        JsStdDumpErrorAce(ctx);
    }
    return -1;
}

void QjsUtils::JsStdDumpErrorAce(JSContext* ctx, JsErrorType errorType, int32_t instanceId, const char* pageUrl,
    const RefPtr<JsAcePage>& page)
{
    RefPtr<RevSourceMap> pageMap;
    RefPtr<RevSourceMap> appMap;
    if (page != nullptr) {
        pageMap = page->GetPageMap();
        appMap = page->GetAppMap();
    }
    JSValue exceptionVal = JS_GetException(ctx);
    BOOL isError = JS_IsError(ctx, exceptionVal);
    if (!isError) {
        LOGE("Throw: ");
    }
    ScopedString printLog(ctx, exceptionVal);
    LOGE("[DUMP] %{public}s", printLog.get());
    QjsHandleScope handleScope(ctx);
    if (isError) {
        JSValue val = QjsUtils::GetPropertyStr(ctx, exceptionVal, "stack");
        if (!JS_IsUndefined(val)) {
            const char* stack = JS_ToCString(ctx, val);
            if (stack != nullptr) {
                if (pageMap || appMap) {
                    const std::string& standardStack = JsDumpSourceFile(stack, pageMap, appMap);
                    LOGE("%{public}s", standardStack.c_str());
                } else {
                    LOGE("%{public}s", stack);
                }
                std::string reasonStr = GetReason(errorType);
                std::string summaryBody = GenerateSummaryBody(printLog.get(), stack, instanceId, pageUrl);
                EventReport::JsErrReport(
                    AceEngine::Get().GetUid(), AceEngine::Get().GetPackageName(), reasonStr, summaryBody);
                JS_FreeValue(ctx, exceptionVal);
                JS_FreeCString(ctx, stack);
            }
        }
    }
}

std::string QjsUtils::JsDumpSourceFile(const char* stack, const RefPtr<RevSourceMap>& pageMap,
    const RefPtr<RevSourceMap>& appMap)
{
    const std::string closeBrace = ")";
    const std::string openBrace = "(";
    const std::string appFile = "app.js";
    std::string tempStack = stack;
    std::string sourceInfo;
    std::string line = "";

    int32_t appFlag = tempStack.find(appFile);
    int32_t closeBracePos = tempStack.find(closeBrace);
    int32_t openBracePos = tempStack.find(openBrace);

    for (int32_t i = closeBracePos - 1; i > 0; i--) {
        if (tempStack[i] >= '0' && tempStack[i] <= '9') {
            line = tempStack[i] + line;
        } else {
            break;
        }
    }
    if (!line.empty()) {
        if (appFlag > 0 && appMap) {
            sourceInfo = appMap->Find(std::stoi(line), 1).ToString();
        } else {
            sourceInfo = pageMap->Find(std::stoi(line), 1).ToString();
        }
    }
    tempStack.replace(openBracePos, closeBracePos - openBracePos + 1, sourceInfo);
    return tempStack;
}

void QjsUtils::JsDumpMemoryStats(JSContext* ctx)
{
#if ACE_DEBUG
    LOGD("JS Memory ---------------------");
    JSMemoryUsage s;
    JSRuntime* rt = JS_GetRuntime(ctx);
    if (!rt) {
        return;
    }

    JS_ComputeMemoryUsage(rt, &s);

    LOGD("malloc limit: %ld ------------", (int64_t)(ssize_t)s.malloc_limit);

    LOGD("   COUNT      SIZE");

    if (s.malloc_count) {
        LOGD("%8ld  %8ld Bytes  Memory allocated (%0.1f per block)", s.malloc_count, s.malloc_size,
            (double)s.malloc_size / s.malloc_count);
        LOGD("%8ld  %8ld Bytes  Memory used (%d overhead, %0.1f average slack)", s.memory_used_count,
            s.memory_used_size, 8, ((double)(s.malloc_size - s.memory_used_size) / s.memory_used_count));
    }
    if (s.atom_count) {
        LOGD("%8ld  %8ld Bytes Atoms (%0.1f per atom)", s.atom_count, s.atom_size, (double)s.atom_size / s.atom_count);
    }
    if (s.str_count) {
        LOGD("%8ld  %8ld Bytes Strings (%0.1f per string)", s.str_count, s.str_size, (double)s.str_size / s.str_count);
    }
    if (s.obj_count) {
        LOGD("%8ld  %8ld Bytes Objects (%0.1f per object)", s.obj_count, s.obj_size, (double)s.obj_size / s.obj_count);
        LOGD("%8ld  %8ld Bytes Properties (%0.1f per object)", s.prop_count, s.prop_size,
            (double)s.prop_count / s.obj_count);
        LOGD("%8ld  %8ld Bytes Shapes (%0.1f per shape)", s.shape_count, s.shape_size,
            (double)s.shape_size / s.shape_count);
    }
    if (s.js_func_count) {
        LOGD("%8ld  %8ld Bytes Bytecode functions", s.js_func_count, s.js_func_size);
        LOGD("%8ld  %8ld Bytes Bytecode (%0.1f per function) ", s.js_func_count, s.js_func_code_size,
            (double)s.js_func_code_size / s.js_func_count);
        if (s.js_func_pc2line_count) {
            LOGD("%8ld  %8ld Bytes Pc2line (%0.1f per function)", s.js_func_pc2line_count, s.js_func_pc2line_size,
                (double)s.js_func_pc2line_size / s.js_func_pc2line_count);
        }
    }
    if (s.c_func_count) {
        LOGD("%8ld C functions", s.c_func_count);
    }
    if (s.array_count) {
        LOGD("%8ld arrays", s.array_count);
        if (s.fast_array_count) {
            LOGD("%8ld fast arrays", s.fast_array_count);
            LOGD("%8ld  %8ld (%0.1f per fast array) elements", s.fast_array_elements,
                s.fast_array_elements * (int)sizeof(JSValue), (double)s.fast_array_elements / s.fast_array_count);
        }
    }
    if (s.binary_object_count) {
        LOGD("%8ld  %8ld binary objects", s.binary_object_count, s.binary_object_size);
    }
#endif
    LOGD("JS Memory ---------------------");
}

std::vector<std::string> QjsUtils::GetObjectKeys(JSValueConst obj, int flags)
{
    return QjsUtils::GetObjectKeys(QJSContext::current(), obj, flags);
}

bool AlwaysMatch(std::string)
{
    return true;
}

std::vector<std::string> QjsUtils::GetObjectKeys(JSContext* ctx, JSValueConst obj, int flags)
{
    return QjsUtils::GetFilteredObjectKeys(ctx, obj, AlwaysMatch, flags);
}

std::vector<std::string> QjsUtils::GetFilteredObjectKeys(
    JSContext* ctx, JSValueConst obj, bool (*matcherFunc)(std::string), int flags)
{
    std::vector<std::string> result;
    if (JS_IsUndefined(obj)) {
        return result;
    }

    if (!JS_IsObject(obj)) {
        return result;
    }

    JSPropertyEnum* properties = nullptr;
    uint32_t propLength = 0;
    int res = JS_GetOwnPropertyNames(ctx, &properties, &propLength, obj, flags);
    if (res == -1) {
        free(properties);
        return result;
    }

    for (uint32_t i = 0; i < propLength; i++) {
        ScopedString keyStr(ctx, properties[i].atom);
        if (matcherFunc(keyStr.get())) {
            result.push_back(keyStr.get());
        }
        JS_FreeAtom(ctx, properties[i].atom);
    }

    free(properties);
    return result;
}

/**
 * obtain the properties of given object fromMap and return in pTab and len.
 * @param ctx: context
 * @param fromMap: elmt's attributes object
 * @param pTab: result array of properties
 * @param len: length of the result array
 * NOTE: caller needs to free pTab array after use
 */
bool QjsUtils::CheckAndGetJsProperty(JSContext* ctx, JSValueConst jsObj, JSPropertyEnum** pTab, uint32_t* len)
{
    int err = JS_GetOwnPropertyNames(ctx, pTab, len, jsObj, JS_GPN_STRING_MASK);
    if (err < 0) {
        LOGE("the object's Property enum is nullptr");
        return false;
    }
    return true;
}

// get the top -level registered variable types
// this function  is called during the initial variable registration process
std::string QjsUtils::typeAsString(JSValueConst targetValue)
{
    JSContext* ctx = QJSContext::current();
    if (JS_IsNumber(targetValue) || JS_IsInteger(targetValue) || JS_IsBigFloat(targetValue)) {
        return "number";
    } else if (JS_IsBool(targetValue)) {
        return "boolean";
    } else if (JS_IsString(targetValue)) {
        return "string";
    } else if (JS_IsArray(ctx, targetValue)) {
        return "array";
    } else if (JS_IsObject(targetValue)) {
        return "object";
    } else if (JS_IsFunction(ctx, targetValue)) {
        return "function";
    } else if (JS_IsNull(targetValue)) {
        return "null";
    } else if (JS_IsUndefined(targetValue)) {
        return "undefined";
    } else {
        LOGW("typeAsString, cannot guess type ");
    }

    return "undefined";
}

JSValue QjsUtils::GetArgvSafe(int idx, int argc, JSValueConst* argv)
{
    if (idx < 0 || idx >= argc) {
        return JS_UNDEFINED;
    }

    return argv[idx];
}

void QjsUtils::DefineGlobalFunction(JSContext* ctx, JSCFunction cFunc, const char* name, const int paramNum)
{
    JSValue global = JS_GetGlobalObject(ctx);
    JSValue jsFuncVal = JS_NewCFunction2(ctx, cFunc, name, paramNum, JS_CFUNC_generic, 0);
    JS_SetPropertyStr(ctx, global, name, jsFuncVal);
    JS_FreeValue(ctx, global);
}

thread_local std::stack<JSContext*> QJSContext::s_qjsContextStack;

JSContext* QJSContext::current()
{
    ACE_DCHECK(s_qjsContextStack.size() > 0 &&
               "QJSContext::current() called outside of the lifetime of a QJSContext::Scope "
               "object. Did you forget QJSContext::Scope scope(ctx); ?");
    return s_qjsContextStack.top();
}

QJSContext::Scope::Scope(JSContext* ctx)
{
    s_qjsContextStack.push(ctx);
}

QJSContext::Scope::~Scope()
{
    s_qjsContextStack.pop();
}

QjsHandleScope::QjsHandleScope(JSContext* ctx)
{
    ACE_DCHECK(ctx != nullptr);
    static int scopeId = 0;
    scopeId_ = scopeId++;
    context_ = ctx;
    qjsHandleScopeStack.push(this);
}

QjsHandleScope::~QjsHandleScope()
{
    qjsHandleScopeStack.pop();
    for (const auto& val : jsValues_) {
        JS_FreeValue(context_, val);
    }
}

} // namespace OHOS::Ace::Framework
