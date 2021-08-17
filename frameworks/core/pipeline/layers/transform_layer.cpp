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

#include "core/pipeline/layers/transform_layer.h"

#include "flutter/lib/ui/dart_wrapper.h"
#include "frameworks/core/components/svg/svg_transform.h"
#include "frameworks/core/components/transform/flutter_render_transform.h"

#include "base/log/dump_log.h"

namespace OHOS::Ace::Flutter {

Matrix4 TransformLayer::UpdateTransformAttr(const std::map<std::string, std::vector<float>>& attrs, Offset offset)
{
    auto transformInfo = SvgTransform::CreateMatrix4(attrs);
    if (transformInfo.hasRotateCenter) {
        transformInfo.matrix4 = FlutterRenderTransform::GetTransformByOffset(
            transformInfo.matrix4, transformInfo.rotateCenter);
    }
    transformInfo.matrix4 = FlutterRenderTransform::GetTransformByOffset(
        transformInfo.matrix4, offset);
    return transformInfo.matrix4;
}

void TransformLayer::Update(const Matrix4& matrix)
{
    matrix_ = matrix;
}

void TransformLayer::UpdateTransformProperty(const std::map<std::string, std::vector<float>>& attrs, Offset offset)
{
    if (!attrs.empty()) {
        Update(UpdateTransformAttr(attrs, offset));
    }
}

void TransformLayer::AddToScene(SceneBuilder& builder, double x, double y)
{
    builder.PushTransform(matrix_);
    AddChildToScene(builder, x_ + x, y_ + y);
    builder.Pop();
}

void TransformLayer::Dump()
{
    OffsetLayer::Dump();
    if (DumpLog::GetInstance().GetDumpFile()) {
        DumpLog::GetInstance().AddDesc("Matrix4:");
        DumpLog::GetInstance().AddDesc("[", matrix_[0], matrix_[1], matrix_[2], matrix_[4], "]");
        DumpLog::GetInstance().AddDesc("[", matrix_[4], matrix_[5], matrix_[6], matrix_[7], "]");
        DumpLog::GetInstance().AddDesc("[", matrix_[8], matrix_[9], matrix_[10], matrix_[11], "]");
        DumpLog::GetInstance().AddDesc("[", matrix_[12], matrix_[13], matrix_[14], matrix_[15], "]");
    }
}

} // namespace OHOS::Ace::Flutter
