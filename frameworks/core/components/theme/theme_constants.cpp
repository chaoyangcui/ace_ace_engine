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

#include "core/components/theme/theme_constants.h"

#include <cstdint>

#include "base/json/json_util.h"
#include "base/utils/device_type.h"
#include "base/utils/string_utils.h"
#include "base/utils/system_properties.h"
#include "core/components/theme/theme_utils.h"

namespace OHOS::Ace {
namespace {

const ResValueWrapper ERROR_VALUE = { .type = ThemeConstantsType::ERROR };
// Don't use Color::BLACK in case Color haven't been initialized.
const Color ERROR_VALUE_COLOR = Color(0xff000000);
constexpr Dimension ERROR_VALUE_DIMENSION = 0.0_vp;
constexpr int32_t ERROR_VALUE_INT = 0;
constexpr double ERROR_VALUE_DOUBLE = 0.0;
constexpr double BLEND_ALPHA_MAX = 1.0;
constexpr InternalResource::ResourceId ERROR_VALUE_RESOURCE_ID = InternalResource::ResourceId::NO_ID;
const char STYLE_FILE_PATH[] = "resources/styles/default.json";
const char PHONE_STYLE_FILE_PATH[] = "resources/styles/phone.json";
const char WEARABLE_STYLE_FILE_PATH[] = "resources/styles/wearable.json";
const char TV_STYLE_FILE_PATH[] = "resources/styles/tv.json";
const char CUSTOM_STYLE_ROOT_NAME[] = "style";
const Color TRANSPARENT_BG_COLOR = Color::FromRGBO(0, 0, 0, 0.2);
// Same with global resource define.
constexpr uint32_t SYSTEM_RES_ID_START = 0x7000000;
constexpr uint32_t SYSTEM_RES_ID_END = 0x7ffffff;

DeviceType g_deviceType = DeviceType::PHONE;

// Check whether value is match with expected type
bool ValueTypeMatch(const ResValueWrapper& valueWrapper, uint32_t key, const ThemeConstantsType& expectType)
{
    if (valueWrapper.type == ThemeConstantsType::ERROR) {
        LOGE("ThemeConstants value not found: %{public}u", key);
        return false;
    }
    if (valueWrapper.type != expectType) {
        LOGE("ThemeConstants value type error: %{public}u, expectType: %{public}u", key, expectType);
        return false;
    }
    return true;
}

bool IsSystemResource(uint32_t resId)
{
    return resId >= SYSTEM_RES_ID_START && resId <= SYSTEM_RES_ID_END;
}

} // namespace

void ThemeConstants::InitDeviceType()
{
    g_deviceType = SystemProperties::GetDeviceType();
    LOGD("InitDeviceType deviceType=%{public}d.", g_deviceType);
}

const ResValueWrapper* ThemeConstants::GetPlatformConstants(uint32_t key)
{
#ifdef WEARABLE_PRODUCT
    if (g_deviceType == DeviceType::WATCH && key < ThemeConstants::WatchMapCount &&
        ThemeConstants::styleMapWatch[key] != nullptr) {
        return ThemeConstants::styleMapWatch[key];
    }
#else
    if (g_deviceType == DeviceType::TV && key < ThemeConstants::TvMapCount &&
        ThemeConstants::styleMapTv[key] != nullptr) {
        return ThemeConstants::styleMapTv[key];
    }
#endif
    if (key < ThemeConstants::DefaultMapCount) {
        return ThemeConstants::styleMapDefault[key];
    }
    return nullptr;
}

Color ThemeConstants::GetColor(uint32_t key) const
{
    if (IsSystemResource(key)) {
        if (!resAdapter_) {
            return ERROR_VALUE_COLOR;
        }
        return resAdapter_->GetColor(key);
    }
    const auto& valueWrapper = GetValue(key);
    if (!ValueTypeMatch(valueWrapper, key, ThemeConstantsType::COLOR)) {
        return ERROR_VALUE_COLOR;
    }
    auto colorPair = valueWrapper.GetValue<Color>(ERROR_VALUE_COLOR);
    if (!colorPair.first) {
        LOGE("GetColor error: %{public}u, type: %{public}u", key, valueWrapper.type);
    }
    return colorPair.second;
}

Dimension ThemeConstants::GetDimension(uint32_t key) const
{
    if (IsSystemResource(key)) {
        if (!resAdapter_) {
            return ERROR_VALUE_DIMENSION;
        }
        return resAdapter_->GetDimension(key);
    }
    const auto& valueWrapper = GetValue(key);
    if (!ValueTypeMatch(valueWrapper, key, ThemeConstantsType::DIMENSION)) {
        return ERROR_VALUE_DIMENSION;
    }
    auto dimensionPair = valueWrapper.GetValue<Dimension>(ERROR_VALUE_DIMENSION);
    if (!dimensionPair.first) {
        LOGE("GetDimension error: %{public}u, type: %{public}u", key, valueWrapper.type);
    }
    return dimensionPair.second;
}

int32_t ThemeConstants::GetInt(uint32_t key) const
{
    if (IsSystemResource(key)) {
        if (!resAdapter_) {
            return ERROR_VALUE_INT;
        }
        return resAdapter_->GetInt(key);
    }
    const auto& valueWrapper = GetValue(key);
    if (!ValueTypeMatch(valueWrapper, key, ThemeConstantsType::INT)) {
        return ERROR_VALUE_INT;
    }
    auto intPair = valueWrapper.GetValue<int32_t>(ERROR_VALUE_INT);
    if (!intPair.first) {
        LOGE("GetInt error: %{public}u, type: %{public}u", key, valueWrapper.type);
    }
    return intPair.second;
}

double ThemeConstants::GetDouble(uint32_t key) const
{
    if (IsSystemResource(key)) {
        if (!resAdapter_) {
            return ERROR_VALUE_DOUBLE;
        }
        return resAdapter_->GetDouble(key);
    }
    const auto& valueWrapper = GetValue(key);
    if (!ValueTypeMatch(valueWrapper, key, ThemeConstantsType::DOUBLE)) {
        return ERROR_VALUE_DOUBLE;
    }
    auto doublePair = valueWrapper.GetValue<double>(ERROR_VALUE_DOUBLE);
    if (!doublePair.first) {
        LOGE("GetDouble error: %{public}u, type: %{public}u", key, valueWrapper.type);
    }
    return doublePair.second;
}

std::string ThemeConstants::GetString(uint32_t key) const
{
    if (IsSystemResource(key)) {
        if (!resAdapter_) {
            return "";
        }
        return resAdapter_->GetString(key);
    }
    const auto& valueWrapper = GetValue(key);
    if (!ValueTypeMatch(valueWrapper, key, ThemeConstantsType::STRING)) {
        return "";
    }
    auto stringPair = valueWrapper.GetValue<std::string>("");
    if (!stringPair.first) {
        LOGE("GetString error: %{public}u, type: %{public}u", key, valueWrapper.type);
    }
    return stringPair.second;
}

InternalResource::ResourceId ThemeConstants::GetResourceId(uint32_t key) const
{
    const auto& valueWrapper = GetValue(key);
    if (!ValueTypeMatch(valueWrapper, key, ThemeConstantsType::RESOURCE_ID)) {
        return ERROR_VALUE_RESOURCE_ID;
    }
    auto resPair = valueWrapper.GetValue<InternalResource::ResourceId>(ERROR_VALUE_RESOURCE_ID);
    if (!resPair.first) {
        LOGE("GetResourceId error: %{public}u, type: %{public}u", key, valueWrapper.type);
    }
    return resPair.second;
}

ResValueWrapper ThemeConstants::GetValue(uint32_t key) const
{
    // Find resource at custom styles.
    auto customIter = customStyleMap_.find(key);
    if (customIter != customStyleMap_.end()) {
        return customIter->second;
    }
    // Find resource at prebuilt maps.
    const auto platformConstants = ThemeConstants::GetPlatformConstants(key);

    if (platformConstants == nullptr) {
        return ERROR_VALUE;
    }
    if (platformConstants->type != ThemeConstantsType::REFERENCE) {
        return *platformConstants;
    }
    // This value point to another style, recursively find target.
    auto uintPtr = std::get_if<uint32_t>(&(platformConstants->value));
    if (!uintPtr) {
        return ERROR_VALUE;
    }
    // Copy reference value, blend alpha if need(reference color and current blendAlpha < 1.0).
    auto refValue = GetValue(*uintPtr);
    refValue.isPublic = platformConstants->isPublic;
    auto blendAlpha = GetBlendAlpha(platformConstants->blendAlpha);
    if ((refValue.type == ThemeConstantsType::COLOR) && (blendAlpha < BLEND_ALPHA_MAX)) {
        auto colorPtr = std::get_if<Color>(&refValue.value);
        if (!colorPtr) {
            return ERROR_VALUE;
        }
        refValue.value = colorPtr->BlendOpacity(blendAlpha);
    }
    return refValue;
}

double ThemeConstants::GetBlendAlpha(const BlendAlpha& blendAlpha) const
{
    auto doublePtr = std::get_if<double>(&blendAlpha);
    if (doublePtr) {
        return *doublePtr;
    }
    auto idPtr = std::get_if<uint32_t>(&blendAlpha);
    if (idPtr) {
        return ThemeConstants::GetDouble(*idPtr);
    }
    return BLEND_ALPHA_MAX;
}

void ThemeConstants::LoadTheme(int32_t themeId)
{
    if (!resAdapter_) {
        LOGE("resAdapter_ is null, load theme resource failed!");
        return;
    }
    currentThemeStyle_ = resAdapter_->GetTheme(themeId);
    if (currentThemeStyle_) {
        currentThemeStyle_->SetName(std::to_string(themeId));
    }
}

void ThemeConstants::ParseTheme()
{
    if (currentThemeStyle_) {
        currentThemeStyle_->ParseContent();
    }
}

void ThemeConstants::LoadCustomStyle(const RefPtr<AssetManager>& assetManager)
{
    if (!assetManager) {
        LOGE("AssetManager is null, load custom style failed!");
        return;
    }
    auto asset = assetManager->GetAsset(STYLE_FILE_PATH);
    ThemeConstants::LoadFile(asset);
    RefPtr<Asset> deviceAsset;
    switch (g_deviceType) {
        case DeviceType::PHONE:
            deviceAsset = assetManager->GetAsset(PHONE_STYLE_FILE_PATH);
            break;
        case DeviceType::TV:
            deviceAsset = assetManager->GetAsset(TV_STYLE_FILE_PATH);
            break;
        case DeviceType::WATCH:
            deviceAsset = assetManager->GetAsset(WEARABLE_STYLE_FILE_PATH);
            break;
        default:
            return;
    }
    ThemeConstants::LoadFile(deviceAsset);
}

void ThemeConstants::ParseCustomStyle(const std::string& content)
{
    auto rootJson = JsonUtil::ParseJsonString(content);
    auto rootNode = rootJson->GetObject(CUSTOM_STYLE_ROOT_NAME);
    if (rootNode->IsNull()) {
        LOGE("Load custom style, root node 'style' not found.");
        return;
    }
    auto child = rootNode->GetChild();
    while (child && !child->IsNull()) {
        const auto& key = child->GetKey();
        const auto& value = child->GetString();
        child = child->GetNext();
        uint32_t styleId = StringUtils::StringToUint(key, UINT32_MAX);
        if (styleId == UINT32_MAX) {
            // Id format error.
            continue;
        }
        const auto& oldValue = ThemeConstants::GetValue(styleId);
        if (oldValue.type == ThemeConstantsType::ERROR) {
            // Id not found.
            continue;
        }
        if (!oldValue.isPublic) {
            // Id is not public.
            continue;
        }
        const auto& newValue = ThemeUtils::ParseStyleValue(styleId, oldValue, value);
        // Replace default style with user custom style, use type to check parse success.
        if (newValue.type == oldValue.type) {
            customStyleMap_[styleId] = newValue;
        }
    }
}

void ThemeConstants::LoadFile(const RefPtr<Asset>& asset)
{
    if (!asset) {
        LOGD("No custom style found.");
        return;
    }

    auto fileSize = asset->GetSize();
    if (fileSize <= 0) {
        LOGD("Load custom style, file is empty.");
        return;
    }
    const auto& fileData = asset->GetData();
    if (!fileData) {
        LOGD("Load custom style, file data is null.");
        return;
    }
    std::string styleContent;
    styleContent.assign(fileData, fileData + fileSize);
    if (styleContent.empty()) {
        return;
    }
    ParseCustomStyle(styleContent);
}

void ThemeConstants::SetColorScheme(ColorScheme colorScheme)
{
    if (!currentThemeStyle_) {
        return;
    }
    if (colorScheme == ColorScheme::SCHEME_TRANSPARENT) {
        currentThemeStyle_->SetAttr(
            THEME_ATTR_BG_COLOR, { .type = ThemeConstantsType::COLOR, .value = TRANSPARENT_BG_COLOR });
    }
}

} // namespace OHOS::Ace
