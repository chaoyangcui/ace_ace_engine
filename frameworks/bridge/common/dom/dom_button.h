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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_DOM_DOM_BUTTON_H
#define FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_DOM_DOM_BUTTON_H

#include "core/components/button/button_component.h"
#include "core/components/button/button_theme.h"
#include "core/components/image/image_component.h"
#include "core/components/padding/padding_component.h"
#include "core/components/text/text_component.h"
#include "frameworks/bridge/common/dom/dom_node.h"
#include "frameworks/bridge/common/dom/dom_type.h"

namespace OHOS::Ace::Framework {

class DOMButton final : public DOMNode {
    DECLARE_ACE_TYPE(DOMButton, DOMNode);

public:
    DOMButton(NodeId nodeId, const std::string& nodeName);
    ~DOMButton() override = default;

    void InitializeStyle() override;

    void CallSpecializedMethod(const std::string& method, const std::string& args) override;

    RefPtr<Component> GetSpecializedComponent() override
    {
        return buttonChild_;
    }

protected:
    bool SetSpecializedAttr(const std::pair<std::string, std::string>& attr) override;
    bool SetSpecializedStyle(const std::pair<std::string, std::string>& style) override;
    bool AddSpecializedEvent(int32_t pageId, const std::string& event) override;
    void PrepareSpecializedComponent() override;
    void ResetInitializedStyle() override;

private:
    void InitButtonStyle();
    void InitTextStyle();
    void PrepareBoxSize();
    void PreparePseudoStyle();
    void PrepareUniversalButton();
    void PrepareDefaultButton();
    void PrepareIconButton();
    void PrepareCapsuleButton();
    void PrepareTextButton();
    void PrepareCircleButton();
    void PrepareDownloadButton();
    void PrepareArcButton();
    void PrepareButtonState();
    void PrepareDisabledBackgroundColor();
    void PrepareDisabledChildStyle();
    void PrepareClickedColor();
    void PrepareWaiting();
    void PrepareWaitingWithText(const RefPtr<LoadingProgressComponent>& progressComponent);
    void PrepareBorderStyle();
    void PrepareBackDecorationStyle();
    void PrepareChildren();
    void PrepareChildrenLayout();
    void AddPadding();
    void ResetBoxHeight(double height, DimensionUnit unit = DimensionUnit::PX);
    void UpdateCustomizedColorFlag();

    RefPtr<ButtonComponent> buttonChild_;
    RefPtr<TextComponent> textChild_;
    RefPtr<PaddingComponent> paddingChild_;
    RefPtr<ImageComponent> imageChild_;
    RefPtr<ButtonTheme> buttonTheme_;
    RefPtr<Decoration> backDecoration_;
    RefPtr<PaddingComponent> innerPaddingChild_;

    std::string buttonType_;
    std::string placement_;
    BorderEdge edge_;
    TextStyle textStyle_;
    Color focusColor_;
    Color progressColor_;
    Color disabledColor_;
    Color textColor_;
    Color disabledTextColor_;
    Dimension innerLeftPadding_;
    Dimension diameter_;
    double blendOpacity_ = 0.0;
    bool isWaiting_ = false;
    bool focusColorChanged_ = false;
    bool textColorChanged_ = false;
    bool isWatch_ = false;
    bool isTv_ = false;
    bool isCustomizedColor_ = false;
    bool fontSizeDefined_ = false;
    bool heightDefined_ = false;
    bool disabledColorEffected_ = false;
    bool textColorDefined_ = false;
    bool bgColorDefined_ = false;
    bool radiusDefined_ = false;
};

} // namespace OHOS::Ace::Framework

#endif // FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_DOM_DOM_BUTTON_H
