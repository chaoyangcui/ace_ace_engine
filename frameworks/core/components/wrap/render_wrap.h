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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WRAP_RENDER_WRAP_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WRAP_RENDER_WRAP_H

#include <list>
#include <vector>

#include "core/components/common/layout/constants.h"
#include "core/pipeline/base/render_node.h"

namespace OHOS::Ace {

struct ContentInfo {
    ContentInfo(double mainLength, double crossLength, int32_t count, const std::list<RefPtr<RenderNode>>& itemList)
        : mainLength_(mainLength), crossLength_(crossLength), count_(count), itemList_(itemList)
    {}

    double mainLength_ = 0.0;
    double crossLength_ = 0.0;
    int32_t count_ = 0;
    std::list<RefPtr<RenderNode>> itemList_;
};

class ACE_EXPORT RenderWrap : public RenderNode {
    DECLARE_ACE_TYPE(RenderWrap, RenderNode);

public:
    static RefPtr<RenderNode> Create();

    void Update(const RefPtr<Component>& component) override;

    void PerformLayout() override;

    Size GetLeftSize(double crossLength, double mainLeftLength, double crossLeftLength) const;

    void LayoutWholeWrap();

    Offset GetContentOffset(double totalCrossLength) const;

    void TraverseContent(const Offset& startPosition) const;

    void PositionedItem(
        double betweenSpace, const ContentInfo& content, const Offset& position, double crossSpace) const;

    Offset GetItemMainOffset(double mainSpace) const;

    void SetWrapLayoutSize(double mainLength, double crossLength)
    {
        if (direction_ == WrapDirection::HORIZONTAL) {
            SetLayoutSize(GetLayoutParam().Constrain(Size(mainLength, crossLength)));
        } else {
            SetLayoutSize(GetLayoutParam().Constrain(Size(crossLength, mainLength)));
        }
        LOGD("wrap::wrap layout size width:%lf, height:%lf", GetLayoutSize().Width(), GetLayoutSize().Height());
    }

    void Paint(RenderContext& context, const Offset& offset) override
    {
        for (const auto& item : GetChildren()) {
            if (item->GetLayoutParam().GetMaxSize().IsValid()) {
                context.PaintChild(item, offset);
            }
        }
    }

    WrapDirection GetDialogDirection() const
    {
        return dialogDirection_;
    }

protected:
    void PerformLayoutInitialize();
    void PlaceItemAndLog(const RefPtr<RenderNode>& node, const Offset& position, const std::string& align) const;
    void HandleCenterAlignment(double totalCrossSpace, const RefPtr<RenderNode>& node, const Offset& position,
        double betweenSpace, Offset& itemPositionOffset) const;
    void HandleEndAlignment(double totalCrossSpace, const RefPtr<RenderNode>& node, const Offset& position,
        double betweenSpace, Offset& itemPositionOffset) const;
    void HandleStartAlignment(
        const RefPtr<RenderNode>& item, const Offset& position, double betweenSpace, Offset& itemPositionOffset) const;
    double GetMainItemLength(const RefPtr<RenderNode>& item) const;
    double GetCrossItemLength(const RefPtr<RenderNode>& item) const;

    void HandleDialogStretch(const LayoutParam& layoutParam);
    std::list<ContentInfo> contentList_;

    virtual void ClearRenderObject() override;
    virtual bool MaybeRelease() override;

private:
    WrapDirection direction_ = WrapDirection::VERTICAL;
    WrapAlignment alignment_ = WrapAlignment::START;
    WrapAlignment mainAlignment_ = WrapAlignment::START;
    WrapAlignment crossAlignment_ = WrapAlignment::START;
    Dimension spacing_;
    Dimension contentSpace_;
    double mainLengthLimit_ = 0.0;
    double crossLengthLimit_ = 0.0;
    double totalMainLength_ = 0.0;
    double totalCrossLength_ = 0.0;

    WrapDirection dialogDirection_ = WrapDirection::HORIZONTAL;
    bool dialogStretch_ = false;

    bool isLeftToRight_ = true;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WRAP_RENDER_WRAP_H
