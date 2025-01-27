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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NAVIGATOR_RENDER_NAVIGATOR_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NAVIGATOR_RENDER_NAVIGATOR_H

#include "core/components/navigator/navigator_component.h"
#include "core/gestures/click_recognizer.h"
#include "core/pipeline/base/render_node.h"

namespace OHOS::Ace {

class RenderNavigator : public RenderNode {
    DECLARE_ACE_TYPE(RenderNavigator, RenderNode);

public:
    RenderNavigator();
    ~RenderNavigator() override = default;

    static RefPtr<RenderNode> Create();

    void Update(const RefPtr<Component>& component) override;

    void PerformLayout() override;

    void NavigatePage();

private:
    void OnTouchTestHit(
        const Offset& coordinateOffset, const TouchRestrict& touchRestrict, TouchTestResult& result) override;
    void Initialize();

    RefPtr<ClickRecognizer> clickRecognizer_;

    std::string uri_;
    NavigatorType type_ = NavigatorType::DEFAULT;
    bool active_ = false;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NAVIGATOR_RENDER_NAVIGATOR_H
