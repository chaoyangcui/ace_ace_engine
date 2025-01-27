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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_INSPECTOR_INSPECT_NODE_H
#define FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_INSPECTOR_INSPECT_NODE_H

#include <string>
#include <unordered_map>

#include "base/json/json_util.h"
#include "base/memory/ace_type.h"
#include "base/utils/device_type.h"
#include "base/utils/macros.h"
#include "base/utils/system_properties.h"
#include "frameworks/bridge/common/dom/dom_type.h"

namespace OHOS::Ace::Framework {
class InspectNode : public AceType {
    DECLARE_ACE_TYPE(InspectNode, AceType);

public:
    InspectNode(NodeId nodeId, const std::string& nodeName);
    ~InspectNode() override;

    void SetAllAttr(std::unique_ptr<JsonValue>& jsonNode);
    void SetAllStyle(std::unique_ptr<JsonValue>& jsonNode);
    void InitCommonStyles();
    virtual void PackAttrAndStyle();
protected:
    NodeId nodeId_ = -1;
    std::string tag_;
    std::unordered_map<std::string, std::string> attrs_;
    std::unordered_map<std::string, std::string> styles_;
};
} // namespace OHOS::Ace::Framework

#endif // FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_INSPECTOR_INSPECT_NODE_H
