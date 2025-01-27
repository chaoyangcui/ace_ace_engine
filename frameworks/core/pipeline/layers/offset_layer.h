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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_LAYERS_OFFSET_LAYER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_LAYERS_OFFSET_LAYER_H

#include "core/pipeline/layers/container_layer.h"
#include "core/pipeline/layers/scene_builder.h"

namespace OHOS::Ace::Flutter {

class OffsetLayer : public ContainerLayer {
    DECLARE_ACE_TYPE(OffsetLayer, ContainerLayer)
public:
    OffsetLayer() = default;
    OffsetLayer(double x, double y) : x_(x), y_(y) {}
    ~OffsetLayer() override = default;

    void SetOffset(double x, double y)
    {
        x_ = x;
        y_ = y;
    }

    void AddToScene(SceneBuilder& builder, double x, double y) override;

    void Dump() override;

protected:
    double x_ = 0.0;
    double y_ = 0.0;
};

} // namespace OHOS::Ace::Flutter

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_LAYERS_OFFSET_LAYER_H
