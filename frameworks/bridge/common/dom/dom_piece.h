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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_DOM_DOM_PIECE_H
#define FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_DOM_DOM_PIECE_H

#include "core/components/image/image_component.h"
#include "core/components/piece/piece_component.h"
#include "core/components/text/text_component.h"
#include "frameworks/bridge/common/dom/dom_node.h"
#include "frameworks/bridge/common/utils/utils.h"

namespace OHOS::Ace::Framework {

class DOMPiece final : public DOMNode {
    DECLARE_ACE_TYPE(DOMPiece, DOMNode);

public:
    DOMPiece(NodeId id, const std::string& nodeName);
    ~DOMPiece() override = default;

    RefPtr<Component> GetSpecializedComponent() override;

    void InitializeStyle() override;

protected:
    bool SetSpecializedAttr(const std::pair<std::string, std::string>& attr) override;
    bool SetSpecializedStyle(const std::pair<std::string, std::string>& style) override;
    void PrepareSpecializedComponent() override;
    bool AddSpecializedEvent(int32_t pageId, const std::string& event) override;
    void ResetInitializedStyle() override;

private:
    bool hasContent_ = false;
    bool hasBackGround_ = false;
    RefPtr<PieceComponent> pieceChild_;
};

} // namespace OHOS::Ace::Framework

#endif // FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_DOM_DOM_PIECE_H
