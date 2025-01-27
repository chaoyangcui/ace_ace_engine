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

#include "core/components/track/flutter_render_arc_track.h"

#include "flutter/lib/ui/painting/path.h"
#include "third_party/skia/include/core/SkClipOp.h"

#include "core/pipeline/base/scoped_canvas_state.h"

namespace OHOS::Ace {
namespace {

constexpr double EDGE = 15.0;
constexpr double HEIGHT_OFFSET = 5.0;
constexpr double INDICATOR_STROKE_WIDTH = 5.0;
constexpr int8_t UNSELECT_ALPHA = 125;

} // namespace

RefPtr<RenderNode> RenderArcTrack::Create()
{
    return AceType::MakeRefPtr<FlutterRenderArcTrack>();
}

void DrawGauge(RenderContext& context, const RenderRingInfo& trackInfo)
{
    auto canvas = ScopedCanvas::Create(context);
    if (!canvas) {
        LOGE("Paint canvas is null");
        return;
    }
    double thickness = trackInfo.thickness;
    flutter::Paint paint;
    flutter::PaintData paintData;
    paint.paint()->setAntiAlias(true);
    paint.paint()->setColor(trackInfo.color.GetValue());
    paint.paint()->setStyle(SkPaint::Style::kStroke_Style);
    paint.paint()->setStrokeWidth(thickness);
    paint.paint()->setStrokeCap(SkPaint::kRound_Cap);

    canvas->drawArc(trackInfo.center.GetX() + (thickness / 2) - trackInfo.radius,
        trackInfo.center.GetY() + (thickness / 2) - trackInfo.radius,
        trackInfo.center.GetX() + trackInfo.radius - (thickness / 2),
        trackInfo.center.GetY() + trackInfo.radius - (thickness / 2), M_PI * ((trackInfo.startDegree / 180) - 0.5),
        M_PI * 2 * (trackInfo.sweepDegree / 360), false, paint, paintData);
}

bool ShouldHighLight(const double start, const double interval, const double percent)
{
    if (LessOrEqual(percent, start + interval) && GreatOrEqual(percent, start)) {
        return true;
    }
    return false;
}

void DrawIndicator(RenderContext& context, const RenderRingInfo& trackInfo)
{
    auto canvas = ScopedCanvas::Create(context);
    if (!canvas) {
        LOGE("Paint canvas is null");
        return;
    }
    canvas->save();

    auto path = flutter::CanvasPath::Create();
    double pathStartVertexX = trackInfo.center.GetX();
    double pathStartVertexY = trackInfo.center.GetY() - trackInfo.radius + (trackInfo.thickness / 2);
    path->moveTo(pathStartVertexX, pathStartVertexY);
    path->lineTo(pathStartVertexX - EDGE, pathStartVertexY + EDGE);
    path->lineTo(pathStartVertexX - EDGE, pathStartVertexY + EDGE + HEIGHT_OFFSET);
    path->lineTo(pathStartVertexX + EDGE, pathStartVertexY + EDGE + HEIGHT_OFFSET);
    path->lineTo(pathStartVertexX + EDGE, pathStartVertexY + EDGE);
    path->lineTo(pathStartVertexX, pathStartVertexY);

    canvas->canvas()->rotate(
        trackInfo.startDegree + trackInfo.sweepDegree, trackInfo.center.GetX(), trackInfo.center.GetY());
    flutter::Paint paint;
    flutter::PaintData paintData;
    paint.paint()->setStyle(SkPaint::kFill_Style);
    paint.paint()->setColor(Color::WHITE.GetValue());
    paint.paint()->setStrokeCap(SkPaint::kSquare_Cap);
    paint.paint()->setStrokeWidth(INDICATOR_STROKE_WIDTH);
    canvas->drawPath(path.get(), paint, paintData);

    paint.paint()->setStyle(SkPaint::kStroke_Style);
    paint.paint()->setColor(Color::BLACK.GetValue());
    canvas->drawPath(path.get(), paint, paintData);
    canvas->restore();
}

void FlutterRenderArcTrack::Paint(RenderContext& context, const Offset& offset)
{
    RenderRingInfo data = paintData_;
    // now depend on box to clip
    if (data.center.GetX() < 0.0 || data.center.GetY() < 0.0) {
        data.center = Offset(GetLayoutSize().Width() / 2, GetLayoutSize().Height() / 2);
    }
    if (data.radius < 0.0) {
        data.radius = GetLayoutSize().Width() / 2;
    }
    data.center += offset;
    if (!showIndicator_) {
        data.startDegree =
            leftToRight_ ? paintData_.startDegree : paintData_.startDegree + paintData_.sweepDegree;
        data.sweepDegree = leftToRight_ ? paintData_.sweepDegree : -1 * paintData_.sweepDegree;
        // draw background
        data.color = GetBackgroundColor();
        DrawGauge(context, data);
        data.color = GetSelectColor();
        data.sweepDegree = data.sweepDegree * GetTotalRatio();
        // draw content
        DrawGauge(context, data);
    } else {
        if (colors_.size() == 0 || colors_.size() != weights_.size()) {
            return;
        }
        double totalWeight = 0.0;
        for (const auto& weight : weights_) {
            totalWeight += weight;
        }
        if (NearEqual(totalWeight, 0.0)) {
            LOGE("total weight is 0.0");
            return;
        }
        double currentStart = 0.0;
        double highLightStart = 0.0;
        size_t highLightIndex = 0;
        double ratio = leftToRight_ ? GetTotalRatio() : 1 - GetTotalRatio();
        for (int32_t index = colors_.size() - 1; index >= 0; --index) {
            data.color = colors_[index];
            data.color.ChangeAlpha(UNSELECT_ALPHA);
            currentStart += weights_[index];
            if (ShouldHighLight(totalWeight - currentStart, weights_[index], ratio * totalWeight)) {
                highLightIndex = index;
                highLightStart = totalWeight - currentStart;
            }
            data.startDegree = paintData_.startDegree + (1 - currentStart / totalWeight) * paintData_.sweepDegree;
            data.sweepDegree = (weights_[index] / totalWeight) * paintData_.sweepDegree;
            DrawGauge(context, data);
        }
        // draw highlight part
        data.color = colors_[highLightIndex];
        data.startDegree = paintData_.startDegree + (highLightStart / totalWeight) * paintData_.sweepDegree;
        data.sweepDegree = (weights_[highLightIndex] / totalWeight) * paintData_.sweepDegree;
        DrawGauge(context, data);
        data.startDegree = paintData_.startDegree;
        data.sweepDegree = paintData_.sweepDegree * ratio;
        DrawIndicator(context, data);
    }
}

} // namespace OHOS::Ace
