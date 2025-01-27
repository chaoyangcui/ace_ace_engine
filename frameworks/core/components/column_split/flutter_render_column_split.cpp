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

#include "core/components/column_split/flutter_render_column_split.h"

#include "flutter/lib/ui/painting/paint.h"

#include "core/pipeline/base/constants.h"
#include "core/pipeline/base/flutter_render_context.h"
#include "core/pipeline/base/scoped_canvas_state.h"

namespace OHOS::Ace {

namespace {

constexpr uint32_t COLUMN_SPLIT_COLOR = 0xFF707B7C;

} // namespace

void FlutterRenderColumnSplit::Paint(RenderContext& context, const Offset& offset)
{
    auto children = GetChildren();
    double divderWidth = GetPaintRect().Width();
    for (const auto& item : children) {
        if (!item->GetVisible()) {
            continue;
        }
        context.PaintChild(item, offset);
        PaintDivider(context, item->GetPosition(), divderWidth);
    }
}

void FlutterRenderColumnSplit::PaintDivider(RenderContext& context, const Offset& offset, double width)
{
    double startPointX = offset.GetX();
    double startPointY = offset.GetY();
    double endPointX = startPointX + width;
    double endPointY = startPointY;
    flutter::Paint paint;
    flutter::PaintData paintData;
    auto canvas = ScopedCanvas::Create(context);
    if (!canvas) {
        LOGE("Paint canvas is null");
        return;
    }
    paint.paint()->setColor(COLUMN_SPLIT_COLOR);
    paint.paint()->setStrokeWidth(DEFAULT_SPLIT_HEIGHT);
    canvas->drawLine(startPointX, startPointY, endPointX, endPointY, paint, paintData);
}

} // namespace OHOS::Ace