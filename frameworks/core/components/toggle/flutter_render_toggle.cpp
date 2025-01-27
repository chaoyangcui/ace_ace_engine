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

#include "core/components/toggle/flutter_render_toggle.h"

#include "core/components/transform/flutter_render_transform.h"
#include "core/pipeline/base/flutter_render_context.h"

namespace OHOS::Ace {

RefPtr<RenderNode> RenderToggle::Create()
{
    return AceType::MakeRefPtr<FlutterRenderToggle>();
}

RenderLayer FlutterRenderToggle::GetRenderLayer()
{
    if (!transformLayer_) {
        transformLayer_ = AceType::MakeRefPtr<Flutter::TransformLayer>(Matrix4::CreateIdentity(), 0.0, 0.0);
    }
    return AceType::RawPtr(transformLayer_);
}

void FlutterRenderToggle::UpdateLayer()
{
    float translateX = GetLayoutSize().Width() / 2.0 * (1.0 - scale_);
    float translateY = GetLayoutSize().Height() / 2.0 * (1.0 - scale_);
    Matrix4 translateMatrix = Matrix4::CreateTranslate(translateX, translateY, 0.0);
    Matrix4 scaleMatrix = Matrix4::CreateScale(scale_, scale_, 1.0);
    Matrix4 transformMatrix = translateMatrix * scaleMatrix;
    transformMatrix = FlutterRenderTransform::GetTransformByOffset(transformMatrix, GetGlobalOffset());
    if (transformLayer_) {
        transformLayer_->Update(transformMatrix);
    }
}

void FlutterRenderToggle::Paint(RenderContext& context, const Offset& offset)
{
    flutter::Canvas* canvas = static_cast<FlutterRenderContext&>(context).GetCanvas();
    if (canvas == nullptr) {
        LOGE("Paint canvas is null");
        return;
    }
    UpdateLayer();
    DrawToggle(*canvas, offset);
    RenderNode::Paint(context, offset);
}

Size FlutterRenderToggle::Measure()
{
    toggleSize_ = GetLayoutParam().GetMaxSize();
    if (NearEqual(GetLayoutParam().GetMaxSize().Width(), Size::INFINITE_SIZE) || (!widthDefined_)) {
        toggleSize_.SetWidth(0.0);
    }
    return toggleSize_;
}

void FlutterRenderToggle::DrawToggle(flutter::Canvas& canvas, const Offset& offset) const
{
    flutter::Paint paint;
    paint.paint()->setColor(GetStatusColor().GetValue());
    paint.paint()->setStyle(SkPaint::Style::kFill_Style);
    paint.paint()->setAntiAlias(true);

    flutter::RRect rRect;
    flutter::PaintData paintData;
    double radius = toggleSize_.Height() / 2;
    rRect.sk_rrect.setRectXY(SkRect::MakeIWH(toggleSize_.Width(), toggleSize_.Height()), radius, radius);
    rRect.sk_rrect.offset(offset.GetX(), offset.GetY());
    canvas.drawRRect(rRect, paint, paintData);
}

Color FlutterRenderToggle::GetStatusColor() const
{
    auto toggle = toggleComponent_.Upgrade();
    if (!toggle) {
        LOGE("fail to get status color due to toggle is null");
        return Color::BLACK;
    }
    auto checkColor = toggle->GetCheckedColor();
    auto pressedColor = toggle->GetPressedBlendColor();
    auto backgroundColor = toggle->GetBackgroundColor();
    if (toggle->GetCheckedState()) {
        if (isPressed_) {
            return checkColor.BlendColor(pressedColor);
        }
        return checkColor;
    }
    if (isPressed_) {
        return backgroundColor.BlendColor(pressedColor);
    }
    return backgroundColor;
}

bool FlutterRenderToggle::HasEffectiveTransform() const
{
    if (!transformLayer_) {
        return false;
    }
    return !transformLayer_->GetMatrix4().IsIdentityMatrix();
}

} // namespace OHOS::Ace
