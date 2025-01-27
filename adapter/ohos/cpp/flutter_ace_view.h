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

#ifndef FOUNDATION_ACE_ADAPTER_OHOS_CPP_FLUTTER_ACE_VIEW_H
#define FOUNDATION_ACE_ADAPTER_OHOS_CPP_FLUTTER_ACE_VIEW_H

#include <memory>

#include "flutter/common/settings.h"
#include "flutter/lib/ui/compositing/scene.h"
#include "flutter/lib/ui/compositing/scene_builder.h"
#include "flutter/shell/platform/ohos/ohos_shell_holder.h"

#include "base/utils/noncopyable.h"
#include "core/common/ace_view.h"
#include "core/common/platform_res_register.h"
#include "core/event/key_event_recognizer.h"

#include "mouse_event.h"
#include "touch_event.h"
#include "window_manager.h"

namespace OHOS::Ace::Platform {

using ReleaseCallback = std::function<void()>;

class FlutterAceView : public AceView, public Referenced {
public:
    FlutterAceView() = default;
    explicit FlutterAceView(int32_t id) : instanceId_(id) {}
    ~FlutterAceView() override = default;
    static FlutterAceView* CreateView(int32_t instanceId);
    static void SurfaceCreated(FlutterAceView* view, const OHOS::sptr<OHOS::Window> &window);
    static void SurfaceChanged(FlutterAceView* view, int32_t width, int32_t height, int32_t orientation);
    static void SetViewportMetrics(FlutterAceView* view, const flutter::ViewportMetrics& metrics);

    static bool DispatchTouchEvent(FlutterAceView* view, OHOS::TouchEvent& touchEvent);

    static bool DispatchKeyEvent(FlutterAceView* view, int32_t keyCode, int32_t action, int32_t repeatTime,
        int64_t timeStamp, int64_t timeStampStart);
    static bool DispatchRotationEvent(FlutterAceView* view, float rotationValue);
    static void InitCacheFilePath(const std::string& path);
    static uint32_t GetBackgroundColor();

    void RegisterTouchEventCallback(TouchEventCallback&& callback) override;
    void RegisterKeyEventCallback(KeyEventCallback&& callback) override;
    void RegisterMouseEventCallback(MouseEventCallback&& callback) override;
    void RegisterRotationEventCallback(RotationEventCallBack&& callback) override;
    void RegisterCardViewPositionCallback(CardViewPositionCallBack&& callback) override {}
    void RegisterCardViewAccessibilityParamsCallback(CardViewAccessibilityParamsCallback&& callback) override {}

    void Launch() override;
    void SetShellHolder(std::unique_ptr<flutter::OhosShellHolder> holder);
    flutter::OhosShellHolder* GetShellHolder() const
    {
        return shell_holder_.get();
    }

    bool ProcessTouchEvent(OHOS::TouchEvent& touchEvent);

    void ProcessMouseEvent(OHOS::MouseEvent& mouseEvent);

    bool ProcessKeyEvent(
        int32_t keyCode, int32_t keyAction, int32_t repeatTime, int64_t timeStamp = 0, int64_t timeStampStart = 0);

    void ProcessIdleEvent(int64_t deadline);

    bool ProcessRotationEvent(float rotationValue);

    int32_t GetInstanceId() const override
    {
        return instanceId_;
    }

    void RegisterViewChangeCallback(ViewChangeCallback&& callback) override
    {
        viewChangeCallback_ = std::move(callback);
    }

    void RegisterDensityChangeCallback(DensityChangeCallback&& callback) override
    {
        densityChangeCallback_ = std::move(callback);
    }

    void RegisterSystemBarHeightChangeCallback(SystemBarHeightChangeCallbak&& callback) override
    {
        systemBarHeightChangeCallback_ = std::move(callback);
    }

    void RegisterSurfaceDestroyCallback(SurfaceDestroyCallback&& callback) override
    {
        surfaceDestroyCallback_ = std::move(callback);
    }

    void RegisterIdleCallback(IdleCallback&& callback) override
    {
        idleCallback_ = std::move(callback);
    }

    void SetPlatformResRegister(const RefPtr<PlatformResRegister>& resRegister)
    {
        resRegister_ = resRegister;
    }

    void RegisterViewDestroyCallback(ViewDestoryCallback&& callback) override {}

    const RefPtr<PlatformResRegister>& GetPlatformResRegister() const override
    {
        return resRegister_;
    }

    ViewType GetViewType() const override
    {
        return AceView::ViewType::SURFACE_VIEW;
    }

    std::unique_ptr<DrawDelegate> GetDrawDelegate() override;
    std::unique_ptr<PlatformWindow> GetPlatformWindow() override;
    bool Dump(const std::vector<std::string>& params) override;

private:
    void NotifySurfaceChanged(int width, int height) const
    {
        if (viewChangeCallback_) {
            viewChangeCallback_(width, height);
        }
    }

    void NotifyDensityChanged(double density) const
    {
        if (densityChangeCallback_) {
            densityChangeCallback_(density);
        }
    }

    void NotifySystemBarHeightChanged(double statusBar, double navigationBar) const
    {
        if (systemBarHeightChangeCallback_) {
            systemBarHeightChangeCallback_(statusBar, navigationBar);
        }
    }

    void NotifySurfaceDestroyed() const
    {
        if (surfaceDestroyCallback_) {
            surfaceDestroyCallback_();
        }
    }

    bool IsLastPage() const;
    bool IsNeedForbidToPlatform(TouchPoint point);

    std::unique_ptr<flutter::OhosShellHolder> shell_holder_;
    TouchEventCallback touchEventCallback_;
    MouseEventCallback mouseEventCallback_;
    RotationEventCallBack rotationEventCallBack_;
    ViewChangeCallback viewChangeCallback_;
    DensityChangeCallback densityChangeCallback_;
    SystemBarHeightChangeCallbak systemBarHeightChangeCallback_;
    SurfaceDestroyCallback surfaceDestroyCallback_;
    IdleCallback idleCallback_;
    int32_t instanceId_ = 0;
    bool viewLaunched_ = false;
    RefPtr<PlatformResRegister> resRegister_;
    KeyEventCallback keyEventCallback_;
    KeyEventRecognizer keyEventRecognizer_;
    // mark the touch event's state, HORIZONTAL_STATE: the event should send to platform, VERTICAL_STATE: should not
    enum class EventState { INITIAL_STATE, HORIZONTAL_STATE, VERTICAL_STATE };

    struct TouchPointInfo {
        Offset offset_;
        EventState eventState_ = EventState::INITIAL_STATE;

        TouchPointInfo() = default;
        explicit TouchPointInfo(const Offset& offset) : offset_(offset) {}
        ~TouchPointInfo() = default;
    };
    std::unordered_map<int32_t, TouchPointInfo> touchPointInfoMap_;

    ACE_DISALLOW_COPY_AND_MOVE(FlutterAceView);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_OHOS_CPP_FLUTTER_ACE_VIEW_H
