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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_VIEW_REGISTER_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_VIEW_REGISTER_H

#include "frameworks/bridge/declarative_frontend/engine/bindings_defines.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_view_abstract.h"

#ifdef USE_QUICKJS_ENGINE
#ifdef __cplusplus
extern "C" {
#endif
#include "third_party/quickjs/quickjs.h"
#ifdef __cplusplus
}
#endif

#include "frameworks/bridge/declarative_frontend/engine/js_object_template.h"

#endif

namespace OHOS::Ace::Framework {

void RemoveViewById(ViewId viewId);

/**
 * register C++ classes that are to be exposed to JS
 */
void JsRegisterViews(BindingTarget globalObj);

} // namespace OHOS::Ace::Framework

#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_VIEW_REGISTER_H
