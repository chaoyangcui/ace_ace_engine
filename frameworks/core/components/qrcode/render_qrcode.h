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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_QRCODE_RENDER_QRCODE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_QRCODE_RENDER_QRCODE_H

#include "base/utils/system_properties.h"
#include "core/components/common/properties/color.h"
#include "core/components/image/image_component.h"
#include "core/components/image/render_image.h"
#include "core/components/qrcode/qrcode_component.h"
#include "core/pipeline/base/render_node.h"

namespace OHOS::Ace {

class RenderQrcode : public RenderNode {
    DECLARE_ACE_TYPE(RenderQrcode, RenderNode);

public:
    static RefPtr<RenderNode> Create();

    void Update(const RefPtr<Component>& component) override;
    void PerformLayout() override;

protected:
    std::string value_;
    Color backgroundColor_;
    Color qrcodeColor_;
    QrcodeType qrcodeType_ { QrcodeType::RECT };
    double width_ = 0.0;
    double height_ = 0.0;
    Dimension qrcodeWidth_;
    Dimension qrcodeHeight_;
    Size qrcodeSize_;
    RefPtr<QrcodeComponent> qrcode_;
    RefPtr<ImageComponent> qrcodeImageComponent_;
    RefPtr<RenderImage> qrcodeRenderImage_;
    bool isWidthDefined_ = false;
    bool isHeightDefined_ = false;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_QRCODE_RENDER_QRCODE_H
