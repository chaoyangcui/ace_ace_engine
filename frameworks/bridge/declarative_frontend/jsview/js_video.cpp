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

#include "frameworks/bridge/declarative_frontend/jsview/js_video.h"

#include "base/log/ace_trace.h"
#include "core/common/ace_application_info.h"
#include "core/components/video/resource/player.h"
#include "core/components/video/resource/texture.h"
#include "core/components/video/video_component_v2.h"
#include "frameworks/bridge/declarative_frontend/view_stack_processor.h"

namespace OHOS::Ace::Framework {

void JSVideo::Create(const JSCallbackInfo& info)
{
    auto stack = ViewStackProcessor::GetInstance();
    auto videoComponent = AceType::DynamicCast<VideoComponentV2>(stack->GetMainComponent());
    if (!videoComponent) {
        videoComponent = AceType::MakeRefPtr<OHOS::Ace::VideoComponentV2>();
        stack->SetSaveComponentEvent(
            [videoComponent] (std::unordered_map<std::string, RefPtr<Component>> map) {
                videoComponent->SetGuestureComponentMap(map);
            });
        if (AceApplicationInfo::GetInstance().IsRightToLeft()) {
            videoComponent->SetTextDirection(TextDirection::RTL);
        }

        ViewStackProcessor::GetInstance()->Push(videoComponent);
    }

    if (info.Length() <= 0 || !info[0]->IsObject()) {
        LOGE("JSVideo: info is invalid.");
        return;
    }

    JSRef<JSObject> videoObj = JSRef<JSObject>::Cast(info[0]);
    JSRef<JSVal> srcValue = videoObj->GetProperty("src");
    if (srcValue->IsString()) {
        videoComponent->SetSrc(srcValue->ToString());
    }

    JSRef<JSVal> currentProgressRateValue = videoObj->GetProperty("currentProgressRate");
    if (!currentProgressRateValue->IsNull() && !currentProgressRateValue->IsEmpty()) {
        if (currentProgressRateValue->IsNumber()) {
            videoComponent->SetSpeed(currentProgressRateValue->ToNumber<double>());
        } else if (currentProgressRateValue->IsString()) {
            videoComponent->SetSpeed(StringUtils::StringToDouble(currentProgressRateValue->ToString()));
        }
    }

    JSRef<JSVal> previewUriValue = videoObj->GetProperty("previewUri");
    if (previewUriValue->IsString()) {
        videoComponent->SetPoster(previewUriValue->ToString());
    }

    JSRef<JSVal> playModeValue = videoObj->GetProperty("playMode");
    if (playModeValue->IsBoolean()) {
        videoComponent->SetPlayMode(playModeValue->ToBoolean());
    }

    JSRef<JSVal> fullscreenValue = videoObj->GetProperty("fullscreen");
    if (fullscreenValue->IsBoolean()) {
        videoComponent->SetFullscreenMode(fullscreenValue->ToBoolean());
    }

    JSRef<JSVal> currentTimeValue = videoObj->GetProperty("currentTime");
    if (!currentTimeValue->IsNull() && !currentTimeValue->IsEmpty()) {
        if (currentTimeValue->IsNumber()) {
            videoComponent->SetCurrentTime(currentTimeValue->ToNumber<int32_t>());
        } else if (currentTimeValue->IsString()) {
            videoComponent->SetCurrentTime(StringUtils::StringToUint(currentTimeValue->ToString()));
        }
    }
    stack->GetFlexItemComponent();
}

void JSVideo::JsMuted(bool isMuted)
{
    auto stack = ViewStackProcessor::GetInstance();
    auto videoComponent = AceType::DynamicCast<VideoComponentV2>(stack->GetMainComponent());
    if (!videoComponent) {
        LOGE("JSVideo: MainComponent is null.");
        return;
    }

    videoComponent->SetMute(isMuted);
}

void JSVideo::JsAutoPlay(bool autoPlay)
{
    auto stack = ViewStackProcessor::GetInstance();
    auto videoComponent = AceType::DynamicCast<VideoComponentV2>(stack->GetMainComponent());
    if (!videoComponent) {
        LOGE("JSVideo: MainComponent is null.");
        return;
    }

    videoComponent->SetAutoPlay(autoPlay);
}

void JSVideo::JsControls(bool controls)
{
    auto stack = ViewStackProcessor::GetInstance();
    auto videoComponent = AceType::DynamicCast<VideoComponentV2>(stack->GetMainComponent());
    if (!videoComponent) {
        LOGE("JSVideo: MainComponent is null.");
        return;
    }

    videoComponent->SetNeedControls(controls);
}

void JSVideo::JsObjectFit(int32_t objectFit)
{
    auto stack = ViewStackProcessor::GetInstance();
    auto videoComponent = AceType::DynamicCast<VideoComponentV2>(stack->GetMainComponent());
    if (!videoComponent) {
        LOGE("JSVideo: MainComponent is null.");
        return;
    }

    videoComponent->SetFit(static_cast<ImageFit>(objectFit));
}

void JSVideo::JsOnStart(const JSCallbackInfo& args)
{
    auto stack = ViewStackProcessor::GetInstance();
    auto videoComponent = AceType::DynamicCast<VideoComponentV2>(stack->GetMainComponent());
    if (!videoComponent) {
        LOGE("JSVideo: MainComponent is null.");
        return;
    }
    std::vector<std::string> keys = { "start" };
    videoComponent->SetStartEventId(GetEventMarker(args, keys));
}

void JSVideo::JsOnPause(const JSCallbackInfo& args)
{
    auto stack = ViewStackProcessor::GetInstance();
    auto videoComponent = AceType::DynamicCast<VideoComponentV2>(stack->GetMainComponent());
    if (!videoComponent) {
        LOGE("JSVideo: MainComponent is null.");
        return;
    }
    std::vector<std::string> keys = { "pause" };
    videoComponent->SetPauseEventId(GetEventMarker(args, keys));
}

void JSVideo::JsOnFinish(const JSCallbackInfo& args)
{
    auto stack = ViewStackProcessor::GetInstance();
    auto videoComponent = AceType::DynamicCast<VideoComponentV2>(stack->GetMainComponent());
    if (!videoComponent) {
        LOGE("JSVideo: MainComponent is null.");
        return;
    }
    std::vector<std::string> keys = { "finish" };
    videoComponent->SetFinishEventId(GetEventMarker(args, keys));
}

void JSVideo::JsOnFullscreenChange(const JSCallbackInfo& args)
{
    auto stack = ViewStackProcessor::GetInstance();
    auto videoComponent = AceType::DynamicCast<VideoComponentV2>(stack->GetMainComponent());
    if (!videoComponent) {
        LOGE("JSVideo: MainComponent is null.");
        return;
    }
    std::vector<std::string> keys = { "fullscreen" };
    videoComponent->SetFullscreenChangeEventId(GetEventMarker(args, keys));
}

void JSVideo::JsOnPrepared(const JSCallbackInfo& args)
{
    auto stack = ViewStackProcessor::GetInstance();
    auto videoComponent = AceType::DynamicCast<VideoComponentV2>(stack->GetMainComponent());
    if (!videoComponent) {
        LOGE("JSVideo: MainComponent is null.");
        return;
    }
    std::vector<std::string> keys = { "duration" };
    videoComponent->SetPreparedEventId(GetEventMarker(args, keys));
}

void JSVideo::JsOnSeeking(const JSCallbackInfo& args)
{
    auto stack = ViewStackProcessor::GetInstance();
    auto videoComponent = AceType::DynamicCast<VideoComponentV2>(stack->GetMainComponent());
    if (!videoComponent) {
        LOGE("JSVideo: MainComponent is null.");
        return;
    }
    std::vector<std::string> keys = { "time" };
    videoComponent->SetSeekingEventId(GetEventMarker(args, keys));
}

void JSVideo::JsOnSeeked(const JSCallbackInfo& args)
{
    auto stack = ViewStackProcessor::GetInstance();
    auto videoComponent = AceType::DynamicCast<VideoComponentV2>(stack->GetMainComponent());
    if (!videoComponent) {
        LOGE("JSVideo: MainComponent is null.");
        return;
    }
    std::vector<std::string> keys = { "time" };
    videoComponent->SetSeekedEventId(GetEventMarker(args, keys));
}

void JSVideo::JsOnUpdate(const JSCallbackInfo& args)
{
    auto stack = ViewStackProcessor::GetInstance();
    auto videoComponent = AceType::DynamicCast<VideoComponentV2>(stack->GetMainComponent());
    if (!videoComponent) {
        LOGE("JSVideo: MainComponent is null.");
        return;
    }
    std::vector<std::string> keys = { "time" };
    videoComponent->SetTimeUpdateEventId(GetEventMarker(args, keys));
}

void JSVideo::JsOnError(const JSCallbackInfo& args)
{
    auto stack = ViewStackProcessor::GetInstance();
    auto videoComponent = AceType::DynamicCast<VideoComponentV2>(stack->GetMainComponent());
    if (!videoComponent) {
        LOGE("JSVideo: MainComponent is null.");
        return;
    }
    std::vector<std::string> keys = { "error" };
    videoComponent->SetErrorEventId(GetEventMarker(args, keys));
}

EventMarker JSVideo::GetEventMarker(const JSCallbackInfo& info, const std::vector<std::string>& keys)
{
    if (!info[0]->IsFunction()) {
        LOGE("info[0] is not a function.");
        return EventMarker();
    }

    RefPtr<JsFunction> jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto eventMarker = EventMarker(
        [execCtx = info.GetExecutionContext(), func = std::move(jsFunc), keys](const std::string& param) {
            JAVASCRIPT_EXECUTION_SCOPE(execCtx);
            func->Execute(keys, param);
        });
    return eventMarker;
}

void JSVideo::JSBind(BindingTarget globalObj)
{
    JSClass<JSVideo>::Declare("Video");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSVideo>::StaticMethod("create", &JSVideo::Create, opt);
    JSClass<JSVideo>::StaticMethod("muted", &JSVideo::JsMuted, opt);
    JSClass<JSVideo>::StaticMethod("autoPlay", &JSVideo::JsAutoPlay, opt);
    JSClass<JSVideo>::StaticMethod("controls", &JSVideo::JsControls, opt);
    JSClass<JSVideo>::StaticMethod("objectFit", &JSVideo::JsObjectFit, opt);

    JSClass<JSVideo>::StaticMethod("onStart", &JSVideo::JsOnStart);
    JSClass<JSVideo>::StaticMethod("onPause", &JSVideo::JsOnPause);
    JSClass<JSVideo>::StaticMethod("onFinish", &JSVideo::JsOnFinish);
    JSClass<JSVideo>::StaticMethod("onFullscreenChange", &JSVideo::JsOnFullscreenChange);
    JSClass<JSVideo>::StaticMethod("onPrepared", &JSVideo::JsOnPrepared);
    JSClass<JSVideo>::StaticMethod("onSeeking", &JSVideo::JsOnSeeking);
    JSClass<JSVideo>::StaticMethod("onSeeked", &JSVideo::JsOnSeeked);
    JSClass<JSVideo>::StaticMethod("onUpdate", &JSVideo::JsOnUpdate);
    JSClass<JSVideo>::StaticMethod("onError", &JSVideo::JsOnError);

    JSClass<JSVideo>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSVideo>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSVideo>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSVideo>::StaticMethod("onClick", &JSInteractableView::JsOnClick);

    JSClass<JSVideo>::Inherit<JSViewAbstract>();
    JSClass<JSVideo>::Bind<>(globalObj);
}

} // namespace OHOS::Ace::Framework