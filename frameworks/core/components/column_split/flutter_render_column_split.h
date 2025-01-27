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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_COLUMN_SPLIT_FLUTTER_RENDER_COLUMN_SPLIT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_COLUMN_SPLIT_FLUTTER_RENDER_COLUMN_SPLIT_H

#include "base/memory/ace_type.h"
#include "core/components/column_split/render_column_split.h"
#include "core/pipeline/layers/offset_layer.h"

namespace OHOS::Ace {

class FlutterRenderColumnSplit : public RenderColumnSplit {
    DECLARE_ACE_TYPE(FlutterRenderColumnSplit, RenderColumnSplit);

public:
    FlutterRenderColumnSplit() = default;
    ~FlutterRenderColumnSplit() override = default;

    void Paint(RenderContext& context, const Offset& offset) override;

private:
    void PaintDivider(RenderContext& context, const Offset& offset, double width);
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_COLUMN_SPLIT_FLUTTER_RENDER_COLUMN_SPLIT_H