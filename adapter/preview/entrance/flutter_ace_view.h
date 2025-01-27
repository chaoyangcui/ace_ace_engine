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

#ifndef FOUNDATION_ACE_ADAPTER_PREVIEW_FLUTTER_ACE_VIEW_H
#define FOUNDATION_ACE_ADAPTER_PREVIEW_FLUTTER_ACE_VIEW_H

#include <memory>

#include "adapter/preview/entrance/ace_resource_register.h"
#include "base/utils/noncopyable.h"
#include "core/common/ace_view.h"
#include "core/event/key_event_recognizer.h"

namespace OHOS::Ace::Platform {

using ReleaseCallback = std::function<void()>;

class FlutterAceView : public AceView, public Referenced {
public:
    explicit FlutterAceView(int32_t instanceId) : instanceId_(instanceId) {}
    ~FlutterAceView() override = default;

    void RegisterTouchEventCallback(TouchEventCallback&& callback) override;
    void RegisterKeyEventCallback(KeyEventCallback&& callback) override;
    void RegisterMouseEventCallback(MouseEventCallback&& callback) override;
    void RegisterRotationEventCallback(RotationEventCallBack&& callback) override;

    void Launch() override;

    int32_t GetInstanceId() const override
    {
        return instanceId_;
    }

    void RegisterCardViewPositionCallback(CardViewPositionCallBack&& callback) override
    {
        if (callback) {
            cardViewPositionCallBack_ = std::move(callback);
        }
    }

    void RegisterCardViewAccessibilityParamsCallback(CardViewAccessibilityParamsCallback&& callback) override
    {
        if (callback) {
            cardViewAccessibilityParamsCallback_ = std::move(callback);
        }
    }

    void RegisterViewChangeCallback(ViewChangeCallback&& callback) override
    {
        if (callback) {
            viewChangeCallback_ = std::move(callback);
        }
    }

    void RegisterDensityChangeCallback(DensityChangeCallback&& callback) override
    {
        if (callback) {
            densityChangeCallback_ = std::move(callback);
        }
    }

    void RegisterSystemBarHeightChangeCallback(SystemBarHeightChangeCallbak&& callback) override
    {
        if (callback) {
            systemBarHeightChangeCallbak_ = std::move(callback);
        }
    }

    void RegisterSurfaceDestroyCallback(SurfaceDestroyCallback&& callback) override
    {
        if (callback) {
            surfaceDestroyCallback_ = std::move(callback);
        }
    }

    void RegisterIdleCallback(IdleCallback&& callback) override
    {
        if (callback) {
            idleCallback_ = std::move(callback);
        }
    }

    const RefPtr<PlatformResRegister>& GetPlatformResRegister() const override
    {
        return resRegister_;
    }

    bool Dump(const std::vector<std::string>& params) override;

    void ProcessIdleEvent(int64_t deadline);

    void NotifySurfaceChanged(int32_t width, int32_t height) const
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
        if (systemBarHeightChangeCallbak_) {
            systemBarHeightChangeCallbak_(statusBar, navigationBar);
        }
    }

    void NotifySurfaceDestroyed() const
    {
        if (surfaceDestroyCallback_) {
            surfaceDestroyCallback_();
        }
    }

    void NotifyViewDestroyed(ViewReleaseCallback&& callback)
    {
        if (viewDestoryCallback_) {
            viewDestoryCallback_(std::move(callback));
        }
    }

    void RegisterViewDestroyCallback(ViewDestoryCallback&& callback) override
    {
        viewDestoryCallback_ = std::move(callback);
    }

    // Use to receive event from glfw window
    bool HandleTouchEvent(std::unique_ptr<flutter::PointerDataPacket> packet) override;

    // Use to receive event from pc previewer
    bool HandleTouchEvent(const TouchPoint& touchEvent) override;

    ViewType GetViewType() const override
    {
        return AceView::ViewType::SURFACE_VIEW;
    }

    std::unique_ptr<DrawDelegate> GetDrawDelegate() override;
    std::unique_ptr<PlatformWindow> GetPlatformWindow() override;

private:
    int32_t instanceId_ = 0;
    RefPtr<PlatformResRegister> resRegister_ = Referenced::MakeRefPtr<AceResourceRegister>();

    TouchEventCallback touchEventCallback_;
    MouseEventCallback mouseEventCallback_;
    RotationEventCallBack rotationEventCallBack_;
    CardViewPositionCallBack cardViewPositionCallBack_;
    CardViewAccessibilityParamsCallback cardViewAccessibilityParamsCallback_;
    ViewChangeCallback viewChangeCallback_;
    DensityChangeCallback densityChangeCallback_;
    SystemBarHeightChangeCallbak systemBarHeightChangeCallbak_;
    SurfaceDestroyCallback surfaceDestroyCallback_;
    IdleCallback idleCallback_;
    KeyEventCallback keyEventCallback_;
    KeyEventRecognizer keyEventRecognizer_;
    ViewDestoryCallback viewDestoryCallback_;

    ACE_DISALLOW_COPY_AND_MOVE(FlutterAceView);
};

} // namespace OHOS::Ace::Platform

#endif // FOUNDATION_ACE_ADAPTER_PREVIEW_FLUTTER_ACE_VIEW_H
