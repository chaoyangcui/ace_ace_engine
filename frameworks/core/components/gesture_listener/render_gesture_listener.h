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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_GESTURE_LISTENER_RENDER_GESTURE_LISTENER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_GESTURE_LISTENER_RENDER_GESTURE_LISTENER_H

#include "core/components/proxy/render_proxy.h"
#include "core/gestures/click_recognizer.h"
#include "core/gestures/drag_recognizer.h"
#include "core/gestures/long_press_recognizer.h"

namespace OHOS::Ace {

class GestureListenerComponent;

class ACE_EXPORT RenderGestureListener final : public RenderProxy {
    DECLARE_ACE_TYPE(RenderGestureListener, RenderProxy);

public:
    static RefPtr<RenderNode> Create();
    void Update(const RefPtr<Component>& component) override;
    void UpdateTouchRect() override;

    bool GetVisible() const override;

    void SetOnClickCallback(const ClickCallback& callback);
    void SetOnLongPressCallback(const OnLongPress& callback);

protected:
    void OnTouchTestHit(
        const Offset& coordinateOffset, const TouchRestrict& touchRestrict, TouchTestResult& result) override;

private:
    void SetOnClickCallback(const RefPtr<GestureListenerComponent>& component);
    void SetOnLongPressCallback(const RefPtr<GestureListenerComponent>& component);

    RefPtr<ClickRecognizer> clickRecognizer_;
    RefPtr<LongPressRecognizer> longPressRecognizer_;
    RefPtr<FreeDragRecognizer> freeDragRecognizer_;
    RefPtr<VerticalDragRecognizer> verticalDragRecognizer_;
    RefPtr<HorizontalDragRecognizer> horizontalDragRecognizer_;
    bool isVisible_ = true;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_GESTURE_LISTENER_RENDER_GESTURE_LISTENER_H
