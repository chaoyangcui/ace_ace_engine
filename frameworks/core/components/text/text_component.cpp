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

#include "core/components/text/text_component.h"

#include "core/components/text/render_text.h"
#include "core/components/text/text_element.h"

namespace OHOS::Ace {

TextComponent::TextComponent(const std::string& data) : ComponentGroup(std::list<RefPtr<Component>>()), data_(data) {}

RefPtr<RenderNode> TextComponent::CreateRenderNode()
{
    return RenderText::Create();
}

RefPtr<Element> TextComponent::CreateElement()
{
    return AceType::MakeRefPtr<TextElement>();
}

} // namespace OHOS::Ace
