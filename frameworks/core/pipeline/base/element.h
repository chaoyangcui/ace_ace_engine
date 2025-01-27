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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_BASE_ELEMENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_BASE_ELEMENT_H

#include <list>

#include "base/utils/macros.h"
#include "core/focus/focus_node.h"
#include "core/pipeline/base/component.h"
#include "core/pipeline/base/render_node.h"

namespace OHOS::Ace {

class PageElement;
class PipelineContext;

// If no insertion location is specified, new child will be added to the end of children list by default.
constexpr int32_t DEFAULT_ELEMENT_SLOT = -1;

// Element is the key class in the UI framework, which presents a basic logic
// unit to construct a view hierarchy.
// There are two types inherited element: RenderElement and ComposedElement.
class ACE_EXPORT Element : public virtual AceType {
    DECLARE_ACE_TYPE(Element, AceType);

public:
    Element() = default;
    ~Element();

    void AddChild(const RefPtr<Element>& child, int32_t slot = DEFAULT_ELEMENT_SLOT);
    void RemoveChild(const RefPtr<Element>& child);
    void InactivateChild(const RefPtr<Element>& child);
    void DeactivateChild(const RefPtr<Element>& child);
    void Rebuild();

    // create a new child element and mount to element tree.
    RefPtr<Element> InflateComponent(const RefPtr<Component>& newComponent, int32_t slot);
    virtual void Mount(const RefPtr<Element>& parent, int32_t slot = DEFAULT_ELEMENT_SLOT);
    void AddToFocus();
    RefPtr<Element> UpdateChild(
        const RefPtr<Element>& child, const RefPtr<Component>& newComponent, int32_t slot = DEFAULT_ELEMENT_SLOT);
    void DetachChild(const RefPtr<Element>&);
    RefPtr<Element> RetakeDeactivateElement(const RefPtr<Component>& newComponent);

    virtual void Detached() {}
    virtual void Deactivate() {}
    virtual void Prepare(const WeakPtr<Element>& parent) {}
    virtual void PerformBuild() {}
    virtual void Update() {}
    virtual void DumpTree(int32_t depth);
    virtual void Dump();
    virtual bool CanUpdate(const RefPtr<Component>& newComponent);

    virtual void MarkDirty();

    virtual bool NeedUpdateWithComponent(const RefPtr<Component>& newComponent)
    {
        return true;
    }

    void SetUpdateComponent(const RefPtr<Component>& newComponent);

    bool NeedUpdate() const
    {
        return component_ != nullptr;
    }

    void SetDepth(int32_t depth)
    {
        depth_ = depth;
    }

    int32_t GetDepth() const
    {
        return depth_;
    }

    void SetPipelineContext(const WeakPtr<PipelineContext>& context);

    enum ElementType {
        BASE_ELEMENT,
        RENDER_ELEMENT,
        COMPOSED_ELEMENT,
        COMPONENT_ELEMENT,
        FOREACH_ELEMENT,
    };

    virtual ElementType GetType() const
    {
        return type_;
    }

    virtual void SetNewComponent(const RefPtr<Component>& newComponent)
    {
        component_ = newComponent;
        if (newComponent) {
            retakeId_ = newComponent->GetRetakeId();
            componentTypeName_ = AceType::TypeName(component_);
        }
    }

    RefPtr<FocusGroup> GetFocusScope();

    RefPtr<Element> GetFirstChild() const;
    RefPtr<Element> GetLastChild() const;

    virtual RefPtr<RenderNode> GetRenderNode() const
    {
        if (children_.empty()) {
            return nullptr;
        } else {
            return children_.front()->GetRenderNode();
        }
    }

    Rect GetRenderRect() const
    {
        auto renderNode = GetRenderNode();
        if (renderNode) {
            return renderNode->GetRectBasedWindowTopLeft();
        }
        return Rect();
    }

    const std::list<RefPtr<Element>>& GetChildren() const;

    const WeakPtr<PipelineContext>& GetContext() const
    {
        return context_;
    }

    void SetSlot(int32_t slot);

    int32_t GetSlot() const
    {
        return slot_;
    }

    const WeakPtr<Element>& GetElementParent() const
    {
        return parent_;
    }

    RefPtr<PageElement> GetPageElement();

    bool IsAutoAccessibility() const
    {
        return autoAccessibility_;
    }

    void SetAutoAccessibility(bool value)
    {
        autoAccessibility_ = value;
    }

    void SetRetakeId(int32_t retakeId)
    {
        retakeId_ = retakeId;
    }

    int32_t GetRetakeId() const
    {
        return retakeId_;
    }

    RefPtr<Component> GetComponent() const
    {
        return component_;
    }

    void RebuildFocusTree();

    RefPtr<FocusNode> RebuildFocusChild();

protected:
    inline RefPtr<Element> DoUpdateChildWithNewComponent(
        const RefPtr<Element>& child, const RefPtr<Component>& newComponent, int32_t slot);

    virtual void Apply(const RefPtr<Element>& child) = 0;

    virtual void OnContextAttached() {}

    void SetParent(const WeakPtr<Element>& parent)
    {
        parent_ = parent;
    }

    RefPtr<ThemeManager> GetThemeManager() const
    {
        auto context = context_.Upgrade();
        if (!context) {
            return nullptr;
        }
        return context->GetThemeManager();
    }

    ElementType type_ = BASE_ELEMENT;
    std::list<RefPtr<Element>> children_;
    RefPtr<Component> component_;
    WeakPtr<PipelineContext> context_;
    const char* componentTypeName_ = nullptr;

private:
    WeakPtr<Element> parent_;
    int32_t depth_ = 0;
    int32_t slot_ = DEFAULT_ELEMENT_SLOT;
    bool autoAccessibility_ = true;
    // One-to-one correspondence with component through retakeId
    int32_t retakeId_ = 0;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_BASE_ELEMENT_H
