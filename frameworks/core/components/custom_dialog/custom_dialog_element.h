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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_CUSTOM_DIALOG_CUSTOM_DIALOG_ELEMENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_CUSTOM_DIALOG_CUSTOM_DIALOG_ELEMENT_H

#include "core/animation/animator.h"
#include "core/components/custom_dialog/custom_dialog_component.h"
#include "core/pipeline/base/composed_element.h"

namespace OHOS::Ace {

class CustomDialogElement : public ComposedElement {
    DECLARE_ACE_TYPE(CustomDialogElement, ComposedElement);

public:
    explicit CustomDialogElement(const ComposeId& id) : ComposedElement(id) {};
    ~CustomDialogElement() override = default;

    void PerformBuild() override;

private:
    void ShowDialog();
    void CloseDialog();

    RefPtr<CustomDialogComponent> dialog_;
    WeakPtr<Animator> animator_;
    // used for inspector node in PC preview
    bool isPopDialog_ = false;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_CUSTOM_DIALOG_CUSTOM_DIALOG_ELEMENT_H
