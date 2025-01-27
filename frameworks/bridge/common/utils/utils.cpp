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

#include "frameworks/bridge/common/utils/utils.h"

#include <unordered_set>

#include "base/utils/string_utils.h"

namespace OHOS::Ace::Framework {

namespace {

using CustomCurveCreator = RefPtr<Curve> (*)(const std::vector<std::string>&);

const size_t STEPS_PARAMS_SIZE = 2;
const size_t CUBIC_PARAMS_SIZE = 4;

static const std::unordered_set<std::string> HORIZON_SET = {
    DOM_BACKGROUND_IMAGE_POSITION_LEFT,
    DOM_BACKGROUND_IMAGE_POSITION_RIGHT,
};
static const std::unordered_set<std::string> VERTICAL_SET = {
    DOM_BACKGROUND_IMAGE_POSITION_TOP,
    DOM_BACKGROUND_IMAGE_POSITION_BOTTOM,
};

RefPtr<Curve> StepsCurveCreator(const std::vector<std::string>& params)
{
    if (params.empty() || params.size() > STEPS_PARAMS_SIZE) {
        LOGE("steps curve accept 1 or 2 parameter(s), current size is %zu.", params.size());
        return nullptr;
    }
    auto step = StringUtils::StringToInt(params.front());
    if (step <= 0) {
        LOGE("step number is illegal: %{public}d", step);
        return nullptr;
    }
    StepsCurvePosition position = StepsCurvePosition::END;
    if (params.size() > 1) {
        if (params.back() == "start") {
            position = StepsCurvePosition::START;
        } else if (params.back() == "end") {
            position = StepsCurvePosition::END;
        } else {
            LOGE("step position is illegal: %{public}s", params.back().c_str());
            return nullptr;
        }
    }
    return AceType::MakeRefPtr<StepsCurve>(step, position);
}

RefPtr<Curve> CubicCurveCreator(const std::vector<std::string>& params)
{
    if (params.size() != CUBIC_PARAMS_SIZE) {
        LOGE("cubic curve accept 4 parameters");
        return nullptr;
    }
    double x1 = StringToDouble(params.at(0));
    double y1 = StringToDouble(params.at(1));
    double x2 = StringToDouble(params.at(2));
    double y2 = StringToDouble(params.at(3));
    return AceType::MakeRefPtr<CubicCurve>(x1, y1, x2, y2);
}

void SetBgImgPositionX(
    const BackgroundImagePositionType type, const double value, BackgroundImagePosition& bgImgPosition)
{
    bgImgPosition.SetSizeTypeX(type);
    bgImgPosition.SetSizeValueX(value);
}

void SetBgImgPositionY(
    const BackgroundImagePositionType type, const double value, BackgroundImagePosition& bgImgPosition)
{
    bgImgPosition.SetSizeTypeY(type);
    bgImgPosition.SetSizeValueY(value);
}

void SetBgImgPosition(
    const BackgroundImagePositionType type, const double value, BackgroundImagePosition& bgImgPosition)
{
    SetBgImgPositionX(type, value, bgImgPosition);
    SetBgImgPositionY(type, value, bgImgPosition);
}

void GetOffsetValue(std::vector<std::string> offsets, std::string& posX, std::string& posY)
{
    if (offsets.size() == 1) {
        posX = offsets.front();
        if (VERTICAL_SET.find(posX) != VERTICAL_SET.end()) {
            posY = offsets.front();
            posX = DOM_BACKGROUND_IMAGE_POSITION_CENTER;
        } else {
            posY = DOM_BACKGROUND_IMAGE_POSITION_CENTER;
        }
    } else {
        posX = offsets.front();
        posY = offsets.back();
        if (VERTICAL_SET.find(posX) != VERTICAL_SET.end() && HORIZON_SET.find(posY) != HORIZON_SET.end()) {
            posY = offsets.front();
            posX = offsets.back();
        }
    }
}

bool BgImgPositionIsValid(const std::string& posX, const std::string& posY)
{
    if ((posX == DOM_BACKGROUND_IMAGE_POSITION_CENTER) || (posY == DOM_BACKGROUND_IMAGE_POSITION_CENTER)) {
        return true;
    }
    // posX and posY are not strictly corresponding to horizontal or vertical, but they must not conflict,
    // for example both of them are "top" is invalid.
    if (posX.find("px") != std::string::npos || posX.find('%') != std::string::npos ||
        HORIZON_SET.find(posX) != HORIZON_SET.end()) {
        if (posY.find("px") != std::string::npos || posY.find('%') != std::string::npos ||
            VERTICAL_SET.find(posY) != VERTICAL_SET.end()) {
            return true;
        }
    }
    return VERTICAL_SET.find(posX) != VERTICAL_SET.end() && HORIZON_SET.find(posY) != HORIZON_SET.end();
}

} // namespace

RefPtr<Curve> CreateBuiltinCurve(const std::string& aniTimFunc)
{
    // this map should be sorted by key
    static const LinearMapNode<RefPtr<Curve>> aniTimFuncMap[] = {
        { DOM_ANIMATION_TIMING_FUNCTION_EASE, Curves::EASE },
        { DOM_ANIMATION_TIMING_FUNCTION_EASE_IN, Curves::EASE_IN },
        { DOM_ANIMATION_TIMING_FUNCTION_EASE_IN_OUT, Curves::EASE_IN_OUT },
        { DOM_ANIMATION_TIMING_FUNCTION_EASE_OUT, Curves::EASE_OUT },
        { DOM_ANIMATION_TIMING_FUNCTION_EXTREME_DECELERATION, Curves::EXTREME_DECELERATION },
        { DOM_ANIMATION_TIMING_FUNCTION_FAST_OUT_LINEAR_IN, Curves::FAST_OUT_LINEAR_IN },
        { DOM_ANIMATION_TIMING_FUNCTION_FAST_OUT_SLOW_IN, Curves::FAST_OUT_SLOW_IN },
        { DOM_ANIMATION_TIMING_FUNCTION_FRICTION, Curves::FRICTION },
        { DOM_ANIMATION_TIMING_FUNCTION_LINEAR, Curves::LINEAR },
        { DOM_ANIMATION_TIMING_FUNCTION_LINEAR_OUT_SLOW_IN, Curves::LINEAR_OUT_SLOW_IN },
        { DOM_ANIMATION_TIMING_FUNCTION_RHYTHM, Curves::RHYTHM },
        { DOM_ANIMATION_TIMING_FUNCTION_SHARP, Curves::SHARP },
        { DOM_ANIMATION_TIMING_FUNCTION_SMOOTH, Curves::SMOOTH },
    };
    auto index = BinarySearchFindIndex(aniTimFuncMap, ArraySize(aniTimFuncMap), aniTimFunc.c_str());
    return index < 0 ? nullptr : aniTimFuncMap[index].value;
}

RefPtr<Curve> CreateCustomCurve(const std::string& aniTimFunc)
{
    if (aniTimFunc.back() != ')') {
        LOGE("last character must be right embrace.");
        return nullptr;
    }
    std::string::size_type leftEmbracePosition = aniTimFunc.find_last_of('(');
    if (leftEmbracePosition == std::string::npos) {
        LOGE("left embrace not found.");
        return nullptr;
    }
    auto aniTimFuncName = aniTimFunc.substr(0, leftEmbracePosition);
    auto params = aniTimFunc.substr(leftEmbracePosition + 1, aniTimFunc.length() - leftEmbracePosition - 2);
    if (aniTimFuncName.empty() || params.empty()) {
        LOGE("no easing function name, or no parameters.");
        return nullptr;
    }
    std::vector<std::string> paramsVector;
    StringUtils::StringSpliter(params, ',', paramsVector);
    for (auto& param : paramsVector) {
        RemoveHeadTailSpace(param);
    }
    static const LinearMapNode<CustomCurveCreator> customCurveMap[] = {
        { DOM_ANIMATION_TIMING_FUNCTION_CUBIC_BEZIER, CubicCurveCreator },
        { DOM_ANIMATION_TIMING_FUNCTION_STEPS, StepsCurveCreator },
    };
    int64_t index = BinarySearchFindIndex(customCurveMap, ArraySize(customCurveMap), aniTimFuncName.c_str());
    if (index < 0) {
        LOGE("no valid creator found for easing function: %{public}s", aniTimFuncName.c_str());
        return nullptr;
    }
    return customCurveMap[index].value(paramsVector);
}

RefPtr<Curve> CreateCurve(const std::string& aniTimFunc)
{
    auto curve = CreateBuiltinCurve(aniTimFunc);
    if (curve) {
        return curve;
    }
    curve = CreateCustomCurve(aniTimFunc);
    if (curve) {
        return curve;
    }
    return Curves::EASE;
}

GradientDirection StrToGradientDirectionCorner(const std::string& horizontal, const std::string& vertical)
{
    static const std::vector<std::tuple<std::string, std::string, GradientDirection>> directionCornerVec = {
        std::make_tuple(DOM_GRADIENT_DIRECTION_TOP, DOM_GRADIENT_DIRECTION_LEFT, GradientDirection::LEFT_TOP),
        std::make_tuple(DOM_GRADIENT_DIRECTION_LEFT, DOM_GRADIENT_DIRECTION_TOP, GradientDirection::LEFT_TOP),
        std::make_tuple(DOM_GRADIENT_DIRECTION_TOP, DOM_GRADIENT_DIRECTION_RIGHT, GradientDirection::RIGHT_TOP),
        std::make_tuple(DOM_GRADIENT_DIRECTION_RIGHT, DOM_GRADIENT_DIRECTION_TOP, GradientDirection::RIGHT_TOP),
        std::make_tuple(DOM_GRADIENT_DIRECTION_BOTTOM, DOM_GRADIENT_DIRECTION_LEFT, GradientDirection::LEFT_BOTTOM),
        std::make_tuple(DOM_GRADIENT_DIRECTION_LEFT, DOM_GRADIENT_DIRECTION_BOTTOM, GradientDirection::LEFT_BOTTOM),
        std::make_tuple(DOM_GRADIENT_DIRECTION_BOTTOM, DOM_GRADIENT_DIRECTION_RIGHT, GradientDirection::RIGHT_BOTTOM),
        std::make_tuple(DOM_GRADIENT_DIRECTION_RIGHT, DOM_GRADIENT_DIRECTION_BOTTOM, GradientDirection::RIGHT_BOTTOM),
    };

    auto iter =
        find_if(directionCornerVec.begin(), directionCornerVec.end(), [&horizontal, &vertical](const auto& item) {
            return std::get<0>(item) == horizontal && std::get<1>(item) == vertical;
        });
    return iter != directionCornerVec.end() ? std::get<GradientDirection>(*iter) : GradientDirection::BOTTOM;
}

// Support keyword values / percentage/px values. Do not support multiple images and edge offsets values.
bool ParseBackgroundImagePosition(const std::string& value, BackgroundImagePosition& backgroundImagePosition)
{
    static const LinearMapNode<void (*)(BackgroundImagePosition&)> backGroundPositionOperators[] = {
        { DOM_BACKGROUND_IMAGE_POSITION_BOTTOM,
            [](BackgroundImagePosition& backgroundImagePosition) {
                SetBgImgPositionY(BackgroundImagePositionType::PERCENT, 100.0, backgroundImagePosition);
            } },
        { DOM_BACKGROUND_IMAGE_POSITION_LEFT,
            [](BackgroundImagePosition& backgroundImagePosition) {
                SetBgImgPositionX(BackgroundImagePositionType::PERCENT, 0.0, backgroundImagePosition);
            } },
        { DOM_BACKGROUND_IMAGE_POSITION_RIGHT,
            [](BackgroundImagePosition& backgroundImagePosition) {
                SetBgImgPositionX(BackgroundImagePositionType::PERCENT, 100.0, backgroundImagePosition);
            } },
        { DOM_BACKGROUND_IMAGE_POSITION_TOP,
            [](BackgroundImagePosition& backgroundImagePosition) {
                SetBgImgPositionY(BackgroundImagePositionType::PERCENT, 0.0, backgroundImagePosition);
            } },
    };

    std::vector<std::string> offsets;
    StringUtils::StringSpliter(value, ' ', offsets);
    if (!offsets.empty()) {
        std::string valueX = "";
        std::string valueY = "";
        GetOffsetValue(offsets, valueX, valueY);
        if (!BgImgPositionIsValid(valueX, valueY)) {
            return false;
        }
        // The input is valid,so set the default is (center,center),
        // if the value is different, the default value is overwritten.
        // the center value is 50%.
        SetBgImgPosition(BackgroundImagePositionType::PERCENT, 50.0, backgroundImagePosition);
        if (valueX.find("px") != std::string::npos) {
            SetBgImgPositionX(BackgroundImagePositionType::PX, StringToDouble(valueX), backgroundImagePosition);
        } else if (valueX.find('%') != std::string::npos) {
            SetBgImgPositionX(BackgroundImagePositionType::PERCENT, StringToDouble(valueX), backgroundImagePosition);
        } else {
            auto operatorIterX = BinarySearchFindIndex(
                backGroundPositionOperators, ArraySize(backGroundPositionOperators), valueX.c_str());
            if (operatorIterX != -1) {
                backGroundPositionOperators[operatorIterX].value(backgroundImagePosition);
            }
        }
        if (valueY.find("px") != std::string::npos) {
            SetBgImgPositionY(BackgroundImagePositionType::PX, StringToDouble(valueY), backgroundImagePosition);
        } else if (valueY.find('%') != std::string::npos) {
            SetBgImgPositionY(BackgroundImagePositionType::PERCENT, StringToDouble(valueY), backgroundImagePosition);
        } else {
            auto operatorIterY = BinarySearchFindIndex(
                backGroundPositionOperators, ArraySize(backGroundPositionOperators), valueY.c_str());
            if (operatorIterY != -1) {
                backGroundPositionOperators[operatorIterY].value(backgroundImagePosition);
            }
        }
    } else {
        SetBgImgPosition(BackgroundImagePositionType::PERCENT, 50.0, backgroundImagePosition);
        if (value.find("px") != std::string::npos) {
            SetBgImgPositionX(BackgroundImagePositionType::PX, StringToDouble(value), backgroundImagePosition);
        } else if (value.find('%') != std::string::npos) {
            SetBgImgPositionX(BackgroundImagePositionType::PERCENT, StringToDouble(value), backgroundImagePosition);
        } else {
            auto operatorIter = BinarySearchFindIndex(
                backGroundPositionOperators, ArraySize(backGroundPositionOperators), value.c_str());
            if (operatorIter != -1) {
                backGroundPositionOperators[operatorIter].value(backgroundImagePosition);
            }
        }
    }
    return true;
}

} // namespace OHOS::Ace::Framework