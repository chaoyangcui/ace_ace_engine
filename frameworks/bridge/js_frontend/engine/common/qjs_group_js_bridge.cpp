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

#include "frameworks/bridge/js_frontend/engine/quickjs/qjs_group_js_bridge.h"

#include "base/json/json_util.h"
#include "base/log/ace_trace.h"
#include "base/log/event_report.h"
#include "base/log/log.h"
#include "frameworks/bridge/codec/function_call.h"
#include "frameworks/bridge/codec/standard_function_codec.h"
#include "frameworks/bridge/js_frontend/engine/common/js_constants.h"
#include "frameworks/bridge/js_frontend/engine/quickjs/qjs_engine.h"

namespace OHOS::Ace::Framework {
namespace {

const int32_t PLUGIN_REQUEST_MIN_ARGC_NUM = 4;
const int32_t PLUGIN_REQUEST_ARG_RESOLVE_INDEX = 0;
const int32_t PLUGIN_REQUEST_ARG_REJECT_INDEX = 1;
const int32_t PLUGIN_REQUEST_ARG_GROUP_NAME_INDEX = 2;
const int32_t PLUGIN_REQUEST_ARG_FUNCTION_NAME_INDEX = 3;
const int32_t PLUGIN_REQUEST_ARG_APP_PARAMS_INDEX = 4;

} // namespace

int32_t QuickJsGroupJsBridge::InitializeGroupJsBridge(JSContext* jsContext)
{
    if (nullptr == jsContext) {
        LOGE("jsContext is null");
        EventReport::SendAPIChannelException(APIChannelExcepType::JS_BRIDGE_INIT_ERR);
        return JS_CALL_FAIL;
    }

    context_ = jsContext;

    if (JS_CALL_SUCCESS != LoadJsBridgeFunction()) {
        LOGE("Load JSBridge function failed!");
        EventReport::SendAPIChannelException(APIChannelExcepType::JS_BRIDGE_INIT_ERR);
        return JS_CALL_FAIL;
    }

    eventCallBackFuncs_.clear();
    moduleCallBackFuncs_.clear();
    requestIdCallbackIdMap_.clear();
    pendingCallbackId_ = 1;

    return JS_CALL_SUCCESS;
}

void QuickJsGroupJsBridge::Uninitialize()
{
    if (!context_) {
        LOGE("qjs context is null");
        return;
    }

    for (auto& cb : eventCallBackFuncs_) {
        JS_FreeValue(context_, cb.second);
    }
    eventCallBackFuncs_.clear();

    for (auto& cb : moduleCallBackFuncs_) {
        JS_FreeValue(context_, cb.second.resolveCallback);
        JS_FreeValue(context_, cb.second.rejectCallback);
    }
    moduleCallBackFuncs_.clear();
}

int32_t QuickJsGroupJsBridge::LoadJsBridgeFunction()
{
    JSValue group = JS_NewObject(context_);
    // 3 means parameter number
    int32_t ret = JS_SetPropertyStr(
        context_, group, "sendGroupMessage", JS_NewCFunction(context_, ProcessJsRequest, "sendGroupMessage", 3));
    if (ret < 0) {
        LOGE("set sendGroupMessage mapping failed!");
        EventReport::SendAPIChannelException(APIChannelExcepType::SET_FUNCTION_ERR);
        JS_FreeValue(context_, group);
        return JS_CALL_FAIL;
    }

    JSValue globalObj = JS_GetGlobalObject(context_);
    ret = JS_SetPropertyStr(context_, globalObj, "group", group);
    if (ret < 0) {
        LOGE("set globalObj group mapping failed!");
        EventReport::SendAPIChannelException(APIChannelExcepType::SET_FUNCTION_ERR);
        JS_FreeValue(context_, group);
        JS_FreeValue(context_, globalObj);
        return JS_CALL_FAIL;
    }

    JS_FreeValue(context_, globalObj);
    return JS_CALL_SUCCESS;
}

void QuickJsGroupJsBridge::LoadPluginJsCode(std::string&& jsCode)
{
    LOGI("Load Plugin Js Code, code len:%{public}d", static_cast<int32_t>(jsCode.length()));
    if (JS_CALL_SUCCESS != CallEvalBuf(context_, jsCode.c_str(), jsCode.length(), "<input>", JS_EVAL_TYPE_MODULE)) {
        LOGE("CallEvalBuf failed!");
    }
}

void QuickJsGroupJsBridge::LoadPluginJsByteCode(std::vector<uint8_t>&& jsCode, std::vector<int32_t>&& jsCodeLen)
{
    LOGW("Qjs do not support load js bytecode now.");
}

void QuickJsGroupJsBridge::QuickJsPrintException(JSContext* context, int32_t expLine, int32_t line)
{
    std::string expMsg = "Unknown";
    QjsHandleScope handleScope(context);
    JSValue jsExp = JS_GetException(context);
    JSValue jsMsg = QjsUtils::GetPropertyStr(context, jsExp, "message");
    if (JS_IsString(jsMsg)) {
        size_t msgLen = 0;
        const char* msg = JS_ToCStringLen(context, &msgLen, jsMsg);
        if (msg == NULL) {
            JS_FreeValue(context, JS_GetException(context));
        } else {
            expMsg.assign(msg, msgLen);
            JS_FreeCString(context, msg);
        }
    }

    JS_FreeValue(context, jsExp);

    LOGE("Exception at line %{private}d: %{private}s at line %{private}d", line, expMsg.c_str(), expLine);
}

bool QuickJsGroupJsBridge::QuickJsCheckException(JSContext* context, JSValueConst object, int32_t line)
{
    if (JS_IsException(object)) {
        QuickJsPrintException(context, JS_VALUE_GET_INT(object), line);
        return true;
    }
    return false;
}

#define QUICK_JS_PRINT_EXCEPTION(ctx) QuickJsGroupJsBridge::QuickJsPrintException(ctx, 0, __LINE__)
#define QUICK_JS_CHECK_EXCEPTION(ctx, obj) QuickJsGroupJsBridge::QuickJsCheckException(ctx, obj, __LINE__)

int32_t QuickJsGroupJsBridge::QuickJsGetLengthIfObjectIsArray(JSContext* ctx, JSValueConst obj)
{
    int32_t result = JS_IsArray(ctx, obj);
    if (result < 0) {
        QUICK_JS_PRINT_EXCEPTION(ctx);
        return -1;
    }
    if (!result) {
        return -1;
    }
    QjsHandleScope handleScope(ctx);
    JSValue valLen = QjsUtils::GetPropertyStr(ctx, obj, "length");
    int32_t length = -1;
    if ((JS_IsInteger(valLen)) && (JS_ToInt32(ctx, &length, valLen)) < 0) {
        QUICK_JS_PRINT_EXCEPTION(ctx);
        return -1;
    }
    return length;
}

std::string QuickJsGroupJsBridge::SerializationObjectToString(JSContext* ctx, JSValue& val)
{
    JSValue strValue = JS_UNDEFINED;
    JSValue global = JS_UNDEFINED;
    JSValue json = JS_UNDEFINED;
    JSValue func = JS_UNDEFINED;

    QjsHandleScope handleScope(ctx);
    do {
        global = JS_GetGlobalObject(ctx);
        json = QjsUtils::GetPropertyStr(ctx, global, "JSON");
        if (!JS_IsObject(json)) {
            break;
        }
        func = QjsUtils::GetPropertyStr(ctx, json, "stringify");
        if (!JS_IsFunction(ctx, func)) {
            break;
        }
    } while (false);

    JS_FreeValue(ctx, global);

    strValue = QjsUtils::Call(ctx, func, json, 1, &(val));
    if (QUICK_JS_CHECK_EXCEPTION(ctx, strValue)) {
        LOGE("SerializationObjectToString error");
        return "";
    } else {
        JSValue tempString = JS_ToString(ctx, strValue);
        ScopedString para(ctx, tempString);
        std::string functionPara(para.get());
        JS_FreeValue(ctx, tempString);
        return functionPara;
    }
}

ParseJsDataResult QuickJsGroupJsBridge::ParseJsPara(JSContext* ctx, int32_t argc,
    JSValueConst* argv, std::vector<CodecData>& args, int32_t requestId)
{
    if (argc < PLUGIN_REQUEST_ARG_APP_PARAMS_INDEX) {
        return ParseJsDataResult::PARSE_JS_SUCCESS;
    }

    for (int32_t i = PLUGIN_REQUEST_ARG_APP_PARAMS_INDEX; i < argc; i++) {
        JSValue val = argv[i];
        if (JS_IsString(val)) {
            ScopedString para(ctx, val);
            std::string functionPara(para.get());
            CodecData arg(functionPara);
            args.push_back(arg);
        } else if (JS_IsNumber(val)) {
            if (JS_IsInteger(val)) {
                int32_t valInt = 0;
                JS_ToInt32(ctx, &valInt, val);
                CodecData arg(valInt);
                args.push_back(arg);
                LOGD("Process callNative para type:Int");
            } else {
                double valDouble;
                JS_ToFloat64(ctx, &valDouble, val);
                CodecData arg(valDouble);
                args.push_back(arg);
                LOGD("Process callNative para type:Double");
            }
        } else if (JS_IsBool(val)) {
            bool valBool = JS_ToBool(ctx, val);
            CodecData arg(valBool);
            args.push_back(arg);
            LOGD("Process callNative para type:bool");
        } else if (JS_IsNull(val)) {
            CodecData argNull;
            args.push_back(argNull);
            LOGD("Process callNative para type:null");
        } else if (JS_IsFunction(ctx, val)) {
            int32_t functionId = GetPendingCallbackIdAndIncrement();
            CodecData arg(functionId, BufferDataType::TYPE_FUNCTION);
            args.push_back(arg);
            SetEventGroupCallBackFuncs(ctx, val, functionId, requestId);
        } else if (JS_IsArray(ctx, val) || JS_IsObject(val)) {
            std::string objStr = SerializationObjectToString(ctx, val);
            if (objStr.empty()) {
                return ParseJsDataResult::PARSE_JS_ERR_UNSUPPORTED_TYPE;
            }
            CodecData arg(objStr, BufferDataType::TYPE_OBJECT);
            args.push_back(arg);
        } else if (JS_IsUndefined(val)) {
            LOGD("Process callNative para type:undefined");
        } else {
            LOGE("Process callNative para type: unsupported type");
            JS_FreeValue(ctx, val);
            return ParseJsDataResult::PARSE_JS_ERR_UNSUPPORTED_TYPE;
        }
    }
    return ParseJsDataResult::PARSE_JS_SUCCESS;
}

bool QuickJsGroupJsBridge::SetModuleGroupCallbackFuncs(
    JSContext* ctx, const JSValue& resolveCallback, const JSValue& rejectCallback, int32_t callbackId)
{
    if (!JS_IsFunction(ctx, resolveCallback) || JS_IsNull(resolveCallback) || !JS_IsFunction(ctx, rejectCallback) ||
        JS_IsNull(rejectCallback)) {
        LOGE("resolve or reject callback function is invalid");
        return false;
    }

    PromiseCallback promiseCallJsFunc;
    promiseCallJsFunc.resolveCallback = resolveCallback;
    promiseCallJsFunc.rejectCallback = rejectCallback;

    auto result = moduleCallBackFuncs_.try_emplace(callbackId, promiseCallJsFunc);
    if (!result.second) {
        LOGE("module callback function has been existed!");
        return false;
    } else {
        JS_DupValue(ctx, promiseCallJsFunc.resolveCallback);
        JS_DupValue(ctx, promiseCallJsFunc.rejectCallback);
    }

    return true;
}

bool QuickJsGroupJsBridge::SetEventGroupCallBackFuncs(
    JSContext* ctx, const JSValue& eventCallbackFunc, int32_t callbackId, int32_t requestId)
{
    if (JS_IsNull(eventCallbackFunc) || !JS_IsFunction(ctx, eventCallbackFunc)) {
        LOGE("callback function is invalid!");
        return false;
    }

    LOGI("record event callback, requestId:%{private}d, callbackId:%{private}d", requestId, callbackId);
    auto result = eventCallBackFuncs_.try_emplace(callbackId, eventCallbackFunc);
    if (result.second) {
        JS_DupValue(ctx, eventCallbackFunc);
    } else {
        JS_FreeValue(ctx, result.first->second);
        result.first->second = JS_DupValue(ctx, eventCallbackFunc);
    }

    AddRequestIdCallbackIdRelation(callbackId, requestId);

    return true;
}

void QuickJsGroupJsBridge::RemoveEventGroupCallBackFuncs(int32_t callbackId)
{
    LOGI("remove event callback, callbackId:%{private}d", callbackId);
    auto itFunc = eventCallBackFuncs_.find(callbackId);
    if (itFunc != eventCallBackFuncs_.end()) {
        JS_FreeValue(context_, itFunc->second);
        eventCallBackFuncs_.erase(callbackId);
    }
}

void QuickJsGroupJsBridge::AddRequestIdCallbackIdRelation(int32_t eventId, int32_t requestId)
{
    auto result = requestIdCallbackIdMap_.try_emplace(requestId, eventId);
    if (!result.second) {
        result.first->second = eventId;
    }
}

void QuickJsGroupJsBridge::RemoveRequestIdCallbackIdRelation(int32_t requestId, bool removeEventCallback)
{
    auto eventId = requestIdCallbackIdMap_.find(requestId);
    if (eventId != requestIdCallbackIdMap_.end()) {
        if (removeEventCallback) {
            RemoveEventGroupCallBackFuncs(eventId->second);
        }
        requestIdCallbackIdMap_.erase(requestId);
    }
}

JSValue QuickJsGroupJsBridge::ProcessJsRequest(JSContext* ctx, JSValueConst thisVal, int32_t argc, JSValueConst* argv)
{
    ACE_SCOPED_TRACE("ProcessJsRequest");
    QjsEngineInstance* instance = static_cast<QjsEngineInstance*>(JS_GetContextOpaque(ctx));
    if (instance == nullptr) {
        LOGE("invalid args, failed to get QjsEngineInstance from the JSContext");
        return JS_EXCEPTION;
    }

    auto groupJsBridge = AceType::DynamicCast<QuickJsGroupJsBridge>(instance->GetDelegate()->GetGroupJsBridge());
    if (groupJsBridge == nullptr) {
        LOGE("invalid args, failed to get GroupJsBridge from the JSContext");
        return JS_EXCEPTION;
    }

    // Should have at least 4 parameters
    if (argv == nullptr || argc < PLUGIN_REQUEST_MIN_ARGC_NUM) {
        LOGE("invalid args number:%{public}d", argc);
        return JS_EXCEPTION;
    }

    int32_t callbackId = groupJsBridge->GetPendingCallbackIdAndIncrement();
    if (!groupJsBridge->SetModuleGroupCallbackFuncs(ctx, argv[PLUGIN_REQUEST_ARG_RESOLVE_INDEX],
        argv[PLUGIN_REQUEST_ARG_REJECT_INDEX], callbackId)) {
        LOGE("set module callback function failed!");
        return JS_EXCEPTION;
    }
    ScopedString groupName(ctx, argv[PLUGIN_REQUEST_ARG_GROUP_NAME_INDEX]);
    if (groupName.get() == nullptr) {
        LOGE("invalid paras, groupName:%{private}s", groupName.get());
        return JS_EXCEPTION;
    }
    std::string strGroupName(groupName.get());
    LOGI("send message, groupName: %{private}s, callbackId: %{private}d", strGroupName.c_str(), callbackId);

    ScopedString functionName(ctx, argv[PLUGIN_REQUEST_ARG_FUNCTION_NAME_INDEX]);
    std::string strFunctionName(functionName.get());

    std::vector<CodecData> args;
    ParseJsDataResult parseJsResult = groupJsBridge->ParseJsPara(ctx, argc, argv, args, callbackId);
    if (parseJsResult != ParseJsDataResult::PARSE_JS_SUCCESS) {
        ProcessParseJsError(parseJsResult, ctx, callbackId);
        return JS_NULL;
    }

    FunctionCall functionCall(strFunctionName, args);
    StandardFunctionCodec codec;
    std::vector<uint8_t> encodeBuf;
    if (!codec.EncodeFunctionCall(functionCall, encodeBuf)) {
        groupJsBridge->TriggerModulePluginGetErrorCallback(callbackId, PLUGIN_REQUEST_FAIL,
            "encode request message failed");
        return JS_NULL;
    }

    if (!instance->CallPlatformFunction(strGroupName, std::move(encodeBuf), callbackId)) {
        LOGE("CallPlatformFunction fail");
        groupJsBridge->TriggerModulePluginGetErrorCallback(callbackId, PLUGIN_REQUEST_FAIL, "send message failed");
        return JS_NULL;
    }

    return JS_NULL;
}

int32_t QuickJsGroupJsBridge::CallEvalBuf(
    JSContext* ctx, const char* buf, size_t bufLen, const char* filename, int32_t evalFlags)
{
    int32_t ret = JS_CALL_SUCCESS;
    JSValue val = JS_Eval(ctx, buf, bufLen, filename, evalFlags);
    if (QUICK_JS_CHECK_EXCEPTION(ctx, val)) {
        LOGE("EvalBuf failed!");
        ret = JS_CALL_FAIL;
    }
    JS_FreeValue(ctx, val);
    return ret;
}

void QuickJsGroupJsBridge::ProcessParseJsError(ParseJsDataResult errorType, JSContext* ctx, int32_t callbackId)
{
    QjsEngineInstance* instance = static_cast<QjsEngineInstance*>(JS_GetContextOpaque(ctx));
    if (instance == nullptr) {
        LOGE("get quick js engine instance failed!");
        return;
    }

    std::string errMessage;
    switch (errorType) {
        case ParseJsDataResult::PARSE_JS_ERR_UNSUPPORTED_TYPE:
            errMessage = "unsupporteded js parameter type";
            instance->PluginErrorCallback(callbackId,
                static_cast<int32_t>(ParseJsDataResult::PARSE_JS_ERR_UNSUPPORTED_TYPE), std::move(errMessage));
            break;
        case ParseJsDataResult::PARSE_JS_ERR_TOO_MANY_PARAM:
            errMessage = "the number of parameters exceeds 255";
            instance->PluginErrorCallback(callbackId,
                static_cast<int32_t>(ParseJsDataResult::PARSE_JS_ERR_TOO_MANY_PARAM), std::move(errMessage));
            break;
        default:
            break;
    }
}

void QuickJsGroupJsBridge::TriggerModuleJsCallback(int32_t callbackId, int32_t code, std::vector<uint8_t>&& messageData)
{
    JSValue callBackResult = JS_NULL;
    CodecData codecResult;
    StandardFunctionCodec codec;
    if (codec.DecodePlatformMessage(messageData, codecResult)) {
        std::string resultString = codecResult.GetStringValue();
        if (resultString.empty()) {
            callBackResult = JS_NULL;
        } else {
            callBackResult = JS_NewString(context_, resultString.c_str());
        }
    } else {
        LOGE("trigger JS result function error, decode message fail, callbackId:%{private}d",
            callbackId);
        code = PLUGIN_REQUEST_FAIL;
        callBackResult = JS_NewString(context_, std::string("{\"code\":").append(std::to_string(code)).append(",")
            .append("\"data\":\"invalid response data\"}").c_str());
    }

    CallModuleJsCallback(callbackId, code, callBackResult);

    messageData.clear();
}

void QuickJsGroupJsBridge::CallModuleJsCallback(int32_t callbackId, int32_t code, JSValue callBackResult)
{
    RemoveRequestIdCallbackIdRelation(callbackId, code != PLUGIN_REQUEST_SUCCESS);

    auto itFunc = moduleCallBackFuncs_.find(callbackId);
    if (itFunc != moduleCallBackFuncs_.end()) {
        PromiseCallback callback = itFunc->second;
        JSValue jsFunc = (code == PLUGIN_REQUEST_SUCCESS ? callback.resolveCallback : callback.rejectCallback);
        if (!JS_IsFunction(context_, jsFunc) || JS_IsNull(jsFunc)) {
            LOGE("trigger JS result function error, it is not a function, callbackId:%{private}d",
                callbackId);
            return;
        }

        // Pass only 1 parameter, call promise resolve call back.
        JS_Call(context_, jsFunc, JS_UNDEFINED, 1, &callBackResult);
        js_std_loop(context_);
        JS_FreeValue(context_, callback.resolveCallback);
        JS_FreeValue(context_, callback.rejectCallback);
        JS_FreeValue(context_, callBackResult);
        moduleCallBackFuncs_.erase(itFunc);
        LOGI("trigger JS result function success, callbackId:%{private}d, code:%{private}d", callbackId, code);
    } else {
        LOGE("trigger JS result function is not exists, callbackId:%{private}d, code:%{private}d", callbackId, code);
    }
}

void QuickJsGroupJsBridge::TriggerModulePluginGetErrorCallback(
    int32_t callbackId, int32_t errorCode, std::string&& errorMessage)
{
    RemoveRequestIdCallbackIdRelation(callbackId, true);

    auto itFunc = moduleCallBackFuncs_.find(callbackId);
    if (itFunc != moduleCallBackFuncs_.end()) {
        PromiseCallback jsCallbackFunc = itFunc->second;
        if (!JS_IsFunction(context_, jsCallbackFunc.rejectCallback) || JS_IsNull(jsCallbackFunc.rejectCallback)) {
            LOGE("trigger JS result function error, reject is not a function, callbackId:%{private}d",
                callbackId);
            return;
        }

        auto resultJson = JsonUtil::Create(true);
        resultJson->Put(std::string("code").c_str(), errorCode);
        resultJson->Put(std::string("data").c_str(), errorMessage.c_str());

        JSValue emptyReplyCallBack = JS_NewString(context_, resultJson->ToString().c_str());
        // Pass only 1 parameter, call promise reject call back for error get in plugin.
        JS_Call(context_, jsCallbackFunc.rejectCallback, JS_UNDEFINED, 1, &emptyReplyCallBack);
        js_std_loop(context_);
        JS_FreeValue(context_, jsCallbackFunc.resolveCallback);
        JS_FreeValue(context_, jsCallbackFunc.rejectCallback);
        JS_FreeValue(context_, emptyReplyCallBack);
        moduleCallBackFuncs_.erase(itFunc);
    }
}

void QuickJsGroupJsBridge::CallEventJsCallback(int32_t callbackId, std::vector<uint8_t>&& eventData)
{
    JSValue callBackEvent = JS_NULL;
    CodecData codecEvent;
    StandardFunctionCodec codec;
    if (codec.DecodePlatformMessage(eventData, codecEvent)) {
        std::string eventString = codecEvent.GetStringValue();
        if (eventString.empty()) {
            callBackEvent = JS_NULL;
        } else {
            callBackEvent = JS_NewString(context_, eventString.c_str());
        }
    } else {
        LOGE("trigger JS callback function error, decode message fail, callbackId:%{private}d", callbackId);
        callBackEvent = JS_NULL;
    }

    auto itFunc = eventCallBackFuncs_.find(callbackId);
    if (itFunc != eventCallBackFuncs_.end()) {
        JSValue jsEventCallbackFunc = itFunc->second;
        if (!JS_IsFunction(context_, jsEventCallbackFunc) || JS_IsNull(jsEventCallbackFunc)) {
            LOGE("trigger JS callback function error, callback is not a function, "
                "callbackId:%{private}d", callbackId);
            return;
        }

        // Pass only 1 parameter
        JS_Call(context_, jsEventCallbackFunc, JS_UNDEFINED, 1, &callBackEvent);
        js_std_loop(context_);
        JS_FreeValue(context_, callBackEvent);
    } else {
        LOGE("trigger JS callback function error, it is not exists, callbackId:%{private}d", callbackId);
    }
    eventData.clear();
}

void QuickJsGroupJsBridge::TriggerEventJsCallback(int32_t callbackId, int32_t code, std::vector<uint8_t>&& eventData)
{
    if (code == PLUGIN_CALLBACK_DESTROY) {
        RemoveEventGroupCallBackFuncs(callbackId);
    } else {
        CallEventJsCallback(callbackId, std::move(eventData));
    }
}
} // namespace OHOS::Ace::Framework
