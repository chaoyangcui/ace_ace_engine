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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_DOM_DOM_POLYGON_H
#define FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_DOM_DOM_POLYGON_H

#include "frameworks/bridge/common/dom/dom_svg_base.h"
#include "frameworks/core/components/svg/svg_polygon_component.h"

namespace OHOS::Ace::Framework {

class ACE_EXPORT DOMSvgPolygon final : public DOMSvgBase {
    DECLARE_ACE_TYPE(DOMSvgPolygon, DOMSvgBase);

public:
    DOMSvgPolygon(NodeId nodeId, const std::string& nodeName);
    ~DOMSvgPolygon() override = default;

protected:
    void OnMounted(const RefPtr<DOMNode>& parentNode) override;
    void OnChildNodeAdded(const RefPtr<DOMNode>& child, int32_t slot) override;
    void PrepareSpecializedComponent() override;
    bool SetSpecializedAttr(const std::pair<std::string, std::string>& attr) override;
    RefPtr<Component> GetSpecializedComponent() override;

private:
    std::string points_;
    RefPtr<SvgPolygonComponent> polygonComponent_;
};

} // namespace OHOS::Ace::Framework

#endif // FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_DOM_DOM_POLYGON_H
