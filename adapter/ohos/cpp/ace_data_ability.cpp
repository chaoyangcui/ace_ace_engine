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

#include "adapter/ohos/cpp/ace_data_ability.h"
#include "adapter/ohos/cpp/pa_container.h"
#include "adapter/ohos/cpp/platform_event_callback.h"

#include "base/log/log.h"
#include "core/common/backend.h"
#include "frameworks/bridge/pa_backend/pa_backend.h"

#include "res_config.h"
#include "resource_manager.h"

namespace OHOS {
namespace Ace {
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

using DataPlatformFinish = std::function<void()>;
class DataPlatformEventCallback final : public Platform::PlatformEventCallback {
public:
    explicit DataPlatformEventCallback(DataPlatformFinish onFinish) : onFinish_(onFinish) {}

    ~DataPlatformEventCallback() = default;

    virtual void OnFinish() const
    {
        LOGI("DataPlatformEventCallback OnFinish");
        if (onFinish_) {
            onFinish_();
        }
    }

    virtual void OnStatusBarBgColorChanged(uint32_t color)
    {
        LOGI("DataPlatformEventCallback OnStatusBarBgColorChanged");
    }
private:
    DataPlatformFinish onFinish_;
};

int32_t AceDataAbility::instanceId_ = 0;
const std::string AceDataAbility::START_PARAMS_KEY = "__startParams";
const std::string AceDataAbility::URI = "url";

REGISTER_AA(AceDataAbility)
void AceDataAbility::OnStart(const OHOS::AAFwk::Want &want)
{
    Ability::OnStart(want);
    LOGI("AceDataAbility::OnStart called");

    // get url
    std::string parsedUrl;
    if (want.HasParameter(URI)) {
        parsedUrl = want.GetStringParam(URI);
    } else {
        parsedUrl = "data.js";
    }

    // init data ability
    BackendType backendType = BackendType::DATA;
    Platform::PaContainer::CreateContainer(
        abilityId_, backendType, this,
        std::make_unique<DataPlatformEventCallback>([this]() {
            TerminateAbility();
        }));

    // get asset
    auto packagePathStr = GetBundleCodePath();
    auto moduleInfo = GetHapModuleInfo();
    if (moduleInfo != nullptr) {
        packagePathStr += "/" + moduleInfo->name + "/";
    }
    auto assetBasePathStr = {std::string("assets/js/default/"), std::string("assets/js/share/")};
    Platform::PaContainer::AddAssetPath(abilityId_, packagePathStr, assetBasePathStr);

    // run data ability
    Platform::PaContainer::RunPa(
        abilityId_, parsedUrl, want.GetStringParam(START_PARAMS_KEY));

    LOGI("AceDataAbility::OnStart called End");
}

void AceDataAbility::OnStop()
{
    LOGI("AceDataAbility::OnStop called ");
    Ability::OnStop();
    Platform::PaContainer::DestroyContainer(abilityId_);
    LOGI("AceDataAbility::OnStop called End");
}
}
} // namespace OHOS::Ace