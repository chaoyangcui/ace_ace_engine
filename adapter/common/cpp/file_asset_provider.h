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

#ifndef FOUNDATION_ACE_ADAPTER_COMMON_CPP_FILE_ASSET_PROVIDER_H
#define FOUNDATION_ACE_ADAPTER_COMMON_CPP_FILE_ASSET_PROVIDER_H

#include <string>
#include <unordered_map>
#include <vector>

#include "contrib/minizip/unzip.h"
#include "flutter/assets/asset_resolver.h"
#include "flutter/fml/mapping.h"

#include "base/utils/macros.h"

namespace OHOS::Ace {

class ACE_EXPORT FileAssetProvider : public flutter::AssetResolver {
public:
    FileAssetProvider() = default;
    ~FileAssetProvider() override;

    bool Initialize(const std::string& packagePath, const std::vector<std::string>& assetBasePaths);

    bool IsValid() const override;

    std::unique_ptr<fml::Mapping> GetAsMapping(const std::string& assetName) const override;

    std::string GetAssetPath(const std::string& assetName);

private:
    std::unordered_map<std::string, uint64_t> fileMap_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::string> filePathMap_;
    std::string packagePath_;
    std::vector<std::string> assetBasePaths_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_ADAPTER_COMMON_CPP_FILE_ASSET_PROVIDER_H
