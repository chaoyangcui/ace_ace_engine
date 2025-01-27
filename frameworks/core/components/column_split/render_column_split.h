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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_COLUMN_SPLIT_RENDER_COLUMN_SPLIT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_COLUMN_SPLIT_RENDER_COLUMN_SPLIT_H

#include "core/gestures/drag_recognizer.h"
#include "core/pipeline/base/render_node.h"

namespace OHOS::Ace {

class RenderColumnSplit : public RenderNode {
    DECLARE_ACE_TYPE(RenderColumnSplit, RenderNode);

public:
    static RefPtr<RenderNode> Create();
    void Update(const RefPtr<Component>& component) override;
    void PerformLayout() override;
    void LayoutChildren();
    void InitializeRecognizer();
    void HandleDragStart(const Offset& startPoint);
    void HandleDragUpdate(const Offset& currentPoint);
    void HandleDragEnd(const Offset& endPoint, double velocity);
    bool TouchTest(const Point& globalPoint, const Point& parentLocalPoint, const TouchRestrict& touchRestrict,
        TouchTestResult& result) override;

protected:
    void OnTouchTestHit(
        const Offset& coordinateOffset, const TouchRestrict& touchRestrict, TouchTestResult& result) override;

private:
    RefPtr<DragRecognizer> dragDetector_;
    Point localPoint_;
    double startY_ = 0.0;
    std::vector<Rect> splitRects_;
    std::vector<double> dragSplitOffset_;
    std::size_t dragedSplitIndex_ = -1;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_COLUMN_SPLIT_RENDER_COLUMN_SPLIT_H