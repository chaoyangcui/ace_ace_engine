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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_LAYERS_TRANSFORM_LAYER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_LAYERS_TRANSFORM_LAYER_H

#include "base/geometry/matrix4.h"
#include "core/pipeline/layers/offset_layer.h"
#include "core/pipeline/layers/scene_builder.h"

namespace OHOS::Ace::Flutter {

class TransformLayer : public OffsetLayer {
    DECLARE_ACE_TYPE(TransformLayer, OffsetLayer)
public:
    TransformLayer(const Matrix4& matrix, double x, double y) : OffsetLayer(x, y), matrix_(matrix) {}
    ~TransformLayer() override = default;
    static Matrix4 UpdateTransformAttr(const std::map<std::string, std::vector<float>>& attrs, Offset offset);

    void Update(const Matrix4& matrix);
    void UpdateTransformProperty(const std::map<std::string, std::vector<float>>& attrs, Offset offset);
    void AddToScene(SceneBuilder& builder, double x, double y) override;
    void Dump() override;

    const Matrix4& GetMatrix4() const
    {
        return matrix_;
    }

private:
    Matrix4 matrix_;
};

} // namespace OHOS::Ace::Flutter

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_LAYERS_TRANSFORM_LAYER_H
