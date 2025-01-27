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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_DOM_DOM_WEB_H
#define FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_DOM_DOM_WEB_H

#include "core/components/web/web_component.h"
#include "frameworks/bridge/common/dom/dom_node.h"

namespace OHOS::Ace::Framework {

class DOMWeb final : public DOMNode {
    DECLARE_ACE_TYPE(DOMWeb, DOMNode);

public:
    DOMWeb(NodeId nodeId, const std::string& nodeName);
    ~DOMWeb() override = default;

    RefPtr<Component> GetSpecializedComponent() override
    {
        return webChild_;
    }

    void CallSpecializedMethod(const std::string& method, const std::string& args) override;

protected:
    bool SetSpecializedAttr(const std::pair<std::string, std::string>& attr) override;
    bool AddSpecializedEvent(int32_t pageId, const std::string& event) override;

private:
    RefPtr<WebComponent> webChild_;
    EventMarker pageStartEventId_;
    EventMarker pageFinishEventId_;
    EventMarker pageErrorEventId_;
};

} // namespace OHOS::Ace::Framework

#endif // FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_DOM_DOM_WEB_H
