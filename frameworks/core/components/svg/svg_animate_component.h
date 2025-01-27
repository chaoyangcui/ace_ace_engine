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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SVG_SVG_ANIMATE_COMPONENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SVG_SVG_ANIMATE_COMPONENT_H

#include <vector>

#include "frameworks/core/animation/svg_animate.h"
#include "frameworks/core/pipeline/base/composed_component.h"

namespace OHOS::Ace {

class SvgAnimateComponent : public ComposedComponent, public SvgAnimate {
    DECLARE_ACE_TYPE(SvgAnimateComponent, ComposedComponent, SvgAnimate);

public:
    SvgAnimateComponent(const ComposeId& id, const std::string& name, SvgAnimateType svgAnimateType)
        : ComposedComponent(id, name)
    {
        svgAnimateType_ = svgAnimateType;
    };

    SvgAnimateComponent(const ComposeId& id, const std::string& name) : ComposedComponent(id, name) {};
    ~SvgAnimateComponent() override = default;

    void SetSvgId(const std::string& svgId)
    {
        svgId_ = svgId;
    }

private:
    std::string svgId_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SVG_SVG_ANIMATE_COMPONENT_H
