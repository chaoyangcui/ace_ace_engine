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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_SPAN_TEXT_SPAN_COMPONENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_SPAN_TEXT_SPAN_COMPONENT_H

#include "core/components/common/properties/text_style.h"
#include "core/components/text_span/text_span_element.h"
#include "core/pipeline/base/component_group.h"

namespace OHOS::Ace {

class TextSpanComponent : public ComponentGroup {
    DECLARE_ACE_TYPE(TextSpanComponent, ComponentGroup);

public:
    explicit TextSpanComponent(const std::string& spanData);
    ~TextSpanComponent() override = default;

    RefPtr<RenderNode> CreateRenderNode() override;
    RefPtr<Element> CreateElement() override;

    const std::string& GetSpanData() const
    {
        return spanData_;
    }

    void SetSpanData(const std::string& data)
    {
        spanData_ = data;
    }

    const TextStyle& GetTextStyle() const
    {
        return spanStyle_;
    }

    void SetTextStyle(const TextStyle& spanStyle)
    {
        hasNewStyle_ = true;
        spanStyle_ = spanStyle;
    }

    bool HasNewStyle() const
    {
        return hasNewStyle_;
    }

    bool IsShow() const
    {
        return isShow_;
    }

    void SetIsShow(bool isShow)
    {
        isShow_ = isShow;
    }

private:
    std::string spanData_;
    TextStyle spanStyle_;
    bool hasNewStyle_ = false;
    bool isShow_ = true;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_SPAN_TEXT_SPAN_COMPONENT_H
