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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_FLUTTER_RENDER_TEXT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_FLUTTER_RENDER_TEXT_H

#include "flutter/lib/ui/text/paragraph_builder.h"

#include "core/components/text/render_text.h"
#include "core/components/text_span/render_text_span.h"

namespace OHOS::Ace {

class FlutterRenderText final : public RenderText {
    DECLARE_ACE_TYPE(FlutterRenderText, RenderText);

public:
    FlutterRenderText() = default;
    ~FlutterRenderText() override = default;

    void Paint(RenderContext& context, const Offset& offset) override;
    double GetBaselineDistance(TextBaseline textBaseline) override;
    double GetTextWidth() override;

protected:
    virtual bool MaybeRelease() override;
    virtual void ClearRenderObject() override;

private:
    Size Measure() override;
    bool UpdateParagraph();
    bool UpdateParagraphAndLayout(double paragraphMaxWidth);
    Size GetSize();
    bool NeedPaint();
    bool AdaptTextSize(double paragraphMaxWidth);
    bool AdaptMinTextSize(double paragraphMaxWidth);
    bool AdaptPreferTextSize(double paragraphMaxWidth);
    bool AdaptPreferTextSizeGroup(double paragraphMaxWidth);
    bool CheckMeasureFlag();
    uint32_t GetTextLines() override;
    bool DidExceedMaxLines();

    std::unique_ptr<txt::Paragraph> paragraph_;

    double paragraphNewWidth_ = 0.0;
    double lastLayoutMaxWidth_ = 0.0;
    double lastLayoutMinWidth_ = 0.0;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_TEXT_FLUTTER_RENDER_TEXT_H
