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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SVG_FLUTTER_RENDER_SVG_TSPAN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SVG_FLUTTER_RENDER_SVG_TSPAN_H

#include "frameworks/core/components/svg/flutter_render_svg_text.h"

namespace OHOS::Ace {

struct PathOffset {
    Offset svg;
    double start = 0.0;
    double current = 0.0;
    std::string path;
};

class FlutterRenderSvgTspan : public FlutterRenderSvgText {
    DECLARE_ACE_TYPE(FlutterRenderSvgTspan, RenderSvgText);

public:
    static RefPtr<RenderNode> Create();

    void DrawTextPath(RenderContext& context, PathOffset& pathOffset);

protected:
    void OnNotifyRender() override;

private:
    void UpdateDrawOffset(DrawOffset& drawOffset) override;
    void UpdatePathOffset(PathOffset& pathOffset);
    double OnDrawTextPath(RenderContext& context, const PathOffset& pathOffset);
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SVG_FLUTTER_RENDER_SVG_TSPAN_H
