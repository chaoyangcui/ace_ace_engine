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

#include "frameworks/bridge/card_frontend/js_card_parser.h"

#include <array>

#include "base/i18n/localization.h"
#include "core/common/ace_application_info.h"
#include "frameworks/base/log/event_report.h"
#include "frameworks/bridge/common/utils/utils.h"

namespace OHOS::Ace::Framework {
namespace {

const char I18N_FOLDER[] = "i18n/";
const char FILE_TYPE_JSON[] = ".json";
const char RESOURCES_FOLDER[] = "resources/";
const char DEFAULTS_RESOURCES_JSON_FILE[] = "res-defaults.json";
const char BLOCK_VALUE[] = "blockValue";

const std::string REPEAT_INDEX = "$idx";
const std::string REPEAT_ITEM = "$item";
const std::string TRUE_STR = "true";
const std::string FALSE_STR = "false";

std::unique_ptr<JsonValue> GetJsonValue(
    const std::vector<std::string>& keys, const std::unique_ptr<JsonValue>& fileData)
{
    auto it = keys.begin();
    if (!fileData || !fileData->IsValid() || !fileData->Contains(*it)) {
        return nullptr;
    }
    auto data = fileData->GetValue(*it);
    for (++it; it != keys.end(); ++it) {
        if (data->IsValid() && data->Contains(*it)) {
            data = data->GetValue(*it);
        } else {
            return nullptr;
        }
    }

    return data;
}

std::string GetDeviceDpi(double dpi)
{
    static const LinearMapNode<bool (*)(double)> dpiMap[] = {
        { "ldpi", [](double dpi) { return GreatNotEqual(dpi, 0.0) && LessNotEqual(dpi, 0.875); } },
        { "mdpi", [](double dpi) { return GreatOrEqual(dpi, 0.875) && LessNotEqual(dpi, 1.25); } },
        { "hdpi", [](double dpi) { return GreatOrEqual(dpi, 1.25) && LessNotEqual(dpi, 1.75); } },
        { "xhdpi", [](double dpi) { return GreatOrEqual(dpi, 1.75) && LessNotEqual(dpi, 2.5); } },
        { "xxhdpi", [](double dpi) { return GreatOrEqual(dpi, 2.5) && LessNotEqual(dpi, 3.5); } },
        { "xxxhdpi", [](double dpi) { return GreatOrEqual(dpi, 3.5); } },
    };
    for (const auto& idx : dpiMap) {
        if (idx.value(dpi)) {
            return idx.key;
        }
    }
    return "mdpi";
}

void GetAttrOptionsSeriesPoint(const std::unique_ptr<JsonValue>& jsonPoint, PointInfo& pointInfo)
{
    if (!jsonPoint || !jsonPoint->IsValid() || !jsonPoint->IsObject()) {
        return;
    }

    pointInfo.SetDisplay(true);
    auto child = jsonPoint->GetChild();
    while (child && child->IsValid()) {
        static const LinearMapNode<void (*)(std::unique_ptr<JsonValue>&, PointInfo&)> chartOptionsPointMap[] = {
            { "display",
                [](std::unique_ptr<JsonValue>& child, PointInfo& pointInfo) {
                    if (child->IsBool()) {
                        pointInfo.SetDisplay(child->GetBool());
                    }
                } },
            { "fillColor",
                [](std::unique_ptr<JsonValue>& child, PointInfo& pointInfo) {
                    const auto& valStr = child->GetString();
                    pointInfo.SetFillColor(Color::FromString(valStr));
                } },
            { "shape",
                [](std::unique_ptr<JsonValue>& child, PointInfo& pointInfo) {
                    const auto& valStr = child->GetString();
                    PointShape shape = (valStr == "circle")
                                           ? PointShape::CIRCLE
                                           : (valStr == "square") ? PointShape::SQUARE : PointShape::TRIANGLE;
                    pointInfo.SetPointShape(shape);
                } },
            { "size",
                [](std::unique_ptr<JsonValue>& child, PointInfo& pointInfo) {
                    auto valStr = child->IsString() ? child->GetString() : child->ToString();
                    pointInfo.SetPointSize(StringToDimension(valStr));
                } },
            { "strokeColor",
                [](std::unique_ptr<JsonValue>& child, PointInfo& pointInfo) {
                    auto valStr = child->GetString();
                    pointInfo.SetStrokeColor(Color::FromString(valStr));
                } },
            { "strokeWidth",
                [](std::unique_ptr<JsonValue>& child, PointInfo& pointInfo) {
                    auto valStr = child->IsString() ? child->GetString() : child->ToString();
                    pointInfo.SetPointStrokeWidth(StringToDimension(valStr));
                } },
        };
        auto key = child->GetKey();
        auto iter = BinarySearchFindIndex(chartOptionsPointMap, ArraySize(chartOptionsPointMap), key.c_str());
        if (iter != -1) {
            chartOptionsPointMap[iter].value(child, pointInfo);
        } else {
            LOGD("key : %{public}s unsupported. Ignoring!", key.c_str());
        }
        child = child->GetNext();
    }
}

void GetChartAttrOptionsSeriesLineStyle(const std::unique_ptr<JsonValue>& jsonLineStyle, ChartOptions& options)
{
    if (!jsonLineStyle || !jsonLineStyle->IsValid() || !jsonLineStyle->IsObject()) {
        return;
    }

    auto child = jsonLineStyle->GetChild();
    while (child && child->IsValid()) {
        auto key = child->GetKey();
        if (key == "smooth" && child->IsBool()) {
            options.SetSmoothFlag(child->GetBool());
        } else if (key == "width") {
            auto valStr = child->IsString() ? child->GetString() : child->ToString();
            options.SetLineWidth(StringToDouble(valStr));
        }
        child = child->GetNext();
    }
}

void GetChartAttrOptionsSeries(const std::unique_ptr<JsonValue>& jsonSeries, ChartOptions& options)
{
    if (!jsonSeries || !jsonSeries->IsValid()) {
        return;
    }
    auto child = jsonSeries->GetChild();
    while (child && child->IsValid()) {
        if (child->IsObject()) {
            static const LinearMapNode<void (*)(const std::unique_ptr<JsonValue>&, ChartOptions&)>
                chartOptionsSeriesMap[] = {
                    { "bottomPoint",
                        [](const std::unique_ptr<JsonValue>& jsonVal, ChartOptions& chartOptions) {
                            PointInfo pointInfo;
                            GetAttrOptionsSeriesPoint(jsonVal, pointInfo);
                            chartOptions.SetBottomPoint(pointInfo);
                        } },
                    { "headPoint",
                        [](const std::unique_ptr<JsonValue>& jsonVal, ChartOptions& chartOptions) {
                            PointInfo pointInfo;
                            GetAttrOptionsSeriesPoint(jsonVal, pointInfo);
                            chartOptions.SetHeadPoint(pointInfo);
                        } },
                    { "lineStyle",
                        [](const std::unique_ptr<JsonValue>& jsonVal, ChartOptions& chartOptions) {
                            GetChartAttrOptionsSeriesLineStyle(jsonVal, chartOptions);
                        } },
                    { "topPoint",
                        [](const std::unique_ptr<JsonValue>& jsonVal, ChartOptions& chartOptions) {
                            PointInfo pointInfo;
                            GetAttrOptionsSeriesPoint(jsonVal, pointInfo);
                            chartOptions.SetTopPoint(pointInfo);
                        } },
                };
            auto key = child->GetKey();
            auto iter = BinarySearchFindIndex(chartOptionsSeriesMap, ArraySize(chartOptionsSeriesMap), key.c_str());
            if (iter != -1) {
                chartOptionsSeriesMap[iter].value(child, options);
            } else {
                LOGD("key : %{public}s unsupported. Ignoring!", key.c_str());
            }
        }
        child = child->GetNext();
    }
}

void GetAttrOptionsAxis(const std::unique_ptr<JsonValue>& jsonAxis, AxisOption& axisOption)
{
    if (!jsonAxis || !jsonAxis->IsValid() || !jsonAxis->IsObject()) {
        return;
    }

    auto child = jsonAxis->GetChild();
    while (child && child->IsValid()) {
        static const LinearMapNode<void (*)(const std::unique_ptr<JsonValue>&, AxisOption&)> chartOptionsAxisMap[] = {
            { "axisTick",
                [](const std::unique_ptr<JsonValue>& jsonAxis, AxisOption& axis) {
                    if (jsonAxis->IsNumber()) {
                        axis.tickNumber = jsonAxis->GetInt();
                    }
                } },
            { "color",
                [](const std::unique_ptr<JsonValue>& jsonAxis, AxisOption& axis) {
                    const auto& valStr = jsonAxis->GetString();
                    axis.color = Color::FromString(valStr);
                } },
            { "display",
                [](const std::unique_ptr<JsonValue>& jsonAxis, AxisOption& axis) {
                    if (jsonAxis->IsBool()) {
                        axis.display = jsonAxis->GetBool();
                    }
                } },
            { "max",
                [](const std::unique_ptr<JsonValue>& jsonAxis, AxisOption& axis) {
                    if (jsonAxis->IsNumber()) {
                        axis.max = jsonAxis->GetDouble();
                    }
                } },
            { "min",
                [](const std::unique_ptr<JsonValue>& jsonAxis, AxisOption& axis) {
                    if (jsonAxis->IsNumber()) {
                        axis.min = jsonAxis->GetDouble();
                    }
                } },
        };
        auto key = child->GetKey();
        auto iter = BinarySearchFindIndex(chartOptionsAxisMap, ArraySize(chartOptionsAxisMap), key.c_str());
        if (iter != -1) {
            chartOptionsAxisMap[iter].value(child, axisOption);
        } else {
            LOGD("key : %{public}s unsupported. Ignoring!", key.c_str());
        }
        child = child->GetNext();
    }
}

void GetAttrOptions(const std::unique_ptr<JsonValue>& jsonOption, ChartOptions& options)
{
    if (!jsonOption || !jsonOption->IsValid() || !jsonOption->IsObject()) {
        return;
    }
    auto child = jsonOption->GetChild();
    while (child && child->IsValid()) {
        static const LinearMapNode<void (*)(const std::unique_ptr<JsonValue>&, ChartOptions&)> chartOptionsMap[] = {
            { "series", [](const std::unique_ptr<JsonValue>& jsonSeries,
                            ChartOptions& chartOptions) { GetChartAttrOptionsSeries(jsonSeries, chartOptions); } },
            { "xAxis",
                [](const std::unique_ptr<JsonValue>& jsonOption, ChartOptions& chartOptions) {
                    AxisOption xAxis;
                    GetAttrOptionsAxis(jsonOption, xAxis);
                    chartOptions.SetXAxis(xAxis);
                } },
            { "yAxis",
                [](const std::unique_ptr<JsonValue>& jsonOption, ChartOptions& chartOptions) {
                    AxisOption yAxis;
                    GetAttrOptionsAxis(jsonOption, yAxis);
                    chartOptions.SetYAxis(yAxis);
                } },
        };
        auto key = child->GetKey();
        auto iter = BinarySearchFindIndex(chartOptionsMap, ArraySize(chartOptionsMap), key.c_str());
        if (iter != -1) {
            chartOptionsMap[iter].value(child, options);
        } else {
            LOGD("key : %{public}s unsupported. Ignoring!", key.c_str());
        }
        child = child->GetNext();
    }
}

void ParseLineDash(const std::string val, SegmentInfo& segmentInfo)
{
    std::vector<std::string> dash;
    StringUtils::StringSpliter(val, ',', dash);
    if (!dash.empty()) {
        segmentInfo.SetLineType(dash[0] == "dashed" ? LineType::DASHED : LineType::SOLID);
    }
    if (dash.size() > 1) {
        segmentInfo.SetSolidWidth(Framework::StringToDouble(dash[1]));
    }
    if (dash.size() > 2) {
        segmentInfo.SetSpaceWidth(Framework::StringToDouble(dash[2]));
    }
}

void ParseTextPlacement(const std::string val, TextInfo& textInfo)
{
    if (val == "top") {
        textInfo.SetPlacement(Placement::TOP);
    } else if (val == "bottom") {
        textInfo.SetPlacement(Placement::BOTTOM);
    } else if (val == "none") {
        textInfo.SetPlacement(Placement::NONE);
    }
}

void GetAttrDataSetData(const std::unique_ptr<JsonValue>& jsonData, MainChart& dataset)
{
    if (!jsonData || !jsonData->IsValid() || !jsonData->IsArray()) {
        return;
    }
    std::vector<LineInfo> line;
    for (int32_t i = 0; i < jsonData->GetArraySize(); ++i) {
        PointInfo pointInfo;
        TextInfo textInfo;
        SegmentInfo segmentInfo;
        if (jsonData->GetArrayItem(i)->IsNumber()) {
            pointInfo.SetX(static_cast<double>(i));
            pointInfo.SetY(jsonData->GetArrayItem(i)->GetDouble());
        } else if (jsonData->GetArrayItem(i)->IsObject()) {
            auto data = jsonData->GetArrayItem(i)->GetChild();
            while (data && data->IsValid()) {
                auto key = data->GetKey();
                auto val = data->GetString();
                if (key == "description") {
                    textInfo.SetTextValue(val);
                } else if (key == "textLocation") {
                    ParseTextPlacement(val, textInfo);
                } else if (key == "lineDash") {
                    ParseLineDash(val, segmentInfo);
                } else if (key == "lineColor") {
                    segmentInfo.SetSegmentColor(Color::FromString(val));
                } else if (key == "textColor") {
                    textInfo.SetColor(Color::FromString(val));
                } else if (key == "value" && data->IsNumber()) {
                    pointInfo.SetX(static_cast<double>(i));
                    pointInfo.SetY(data->GetDouble());
                } else if (key == "pointStyle" && data->IsObject()) {
                    GetAttrOptionsSeriesPoint(data, pointInfo);
                } else {
                    LOGW("key is %{public}s. Ignoring!", key.c_str());
                }
                data = data->GetNext();
            }
        } else {
            continue;
        }
        LineInfo lineInfo;
        lineInfo.SetPointInfo(pointInfo);
        lineInfo.SetTextInfo(textInfo);
        lineInfo.SetSegmentInfo(segmentInfo);
        line.emplace_back(lineInfo);
    }
    dataset.SetData(line);
}

void GetAttrDataset(const std::unique_ptr<JsonValue>& jsonDataSet, MainChart& dataset)
{
    if (!jsonDataSet || !jsonDataSet->IsValid()) {
        return;
    }
    auto data = jsonDataSet->GetChild();
    while (data && data->IsValid()) {
        auto key = data->GetKey();
        if (key == "gradient") {
            dataset.SetGradient(data->GetBool());
        } else if (key == "strokeColor") {
            dataset.SetStrokeColor(Color::FromString(data->GetString()));
        } else if (key == "fillColor") {
            dataset.SetFillColor(Color::FromString(data->GetString()));
        } else if (key == "data" && data->IsArray()) {
            GetAttrDataSetData(data, dataset);
        }
        data = data->GetNext();
    }
}

void GetAttrDatasets(const std::unique_ptr<JsonValue>& jsonDataSets, std::vector<MainChart>& datasets)
{
    if (!jsonDataSets || !jsonDataSets->IsValid() || !jsonDataSets->IsArray()) {
        return;
    }
    for (int32_t i = 0; i < jsonDataSets->GetArraySize(); ++i) {
        auto item = jsonDataSets->GetArrayItem(i);
        if (item->IsObject()) {
            MainChart chart;
            GetAttrDataset(item, chart);
            datasets.emplace_back(chart);
        }
    }
}

void ParseSegmentObject(const std::unique_ptr<JsonValue>& jsonDataSet, Segment& segment)
{
    if (!jsonDataSet || !jsonDataSet->IsValid()) {
        return;
    }
    auto data = jsonDataSet->GetChild();
    while (data && data->IsValid()) {
        auto key = data->GetKey();
        if (key == "startColor") {
            segment.SetStartColor(Color::FromString(data->GetString()));
            segment.SetColorType(SegmentStyleType::USE_COLOR);
        } else if (key == "endColor") {
            segment.SetEndColor(Color::FromString(data->GetString()));
            segment.SetColorType(SegmentStyleType::USE_COLOR);
        } else if (key == "value") {
            segment.SetValue(data->GetDouble());
        } else if (key == "name") {
            segment.SetSegmentName(data->GetString());
        } else {
            LOGD("json parser %{public}s fail", key.c_str());
        }
        data = data->GetNext();
    }
}

void ParseSegments(const std::unique_ptr<JsonValue>& jsonDataSets, std::vector<Segment>& datasets)
{
    if (!jsonDataSets || !jsonDataSets->IsValid()) {
        return;
    }
    if (jsonDataSets->IsObject()) {
        Segment segment;
        ParseSegmentObject(jsonDataSets, segment);
        datasets.emplace_back(segment);
    } else if (jsonDataSets->IsArray()) {
        for (int32_t i = 0; i < jsonDataSets->GetArraySize(); ++i) {
            auto item = jsonDataSets->GetArrayItem(i);
            if (item && item->IsObject()) {
                Segment segment;
                ParseSegmentObject(item, segment);
                datasets.emplace_back(segment);
            }
        }
    }
}

void GetBadgeConfig(const std::unique_ptr<JsonValue>& jsonDataSets, BadgeConfig& badgeConfig)
{
    if (!jsonDataSets || !jsonDataSets->IsValid()) {
        return;
    }
    auto data = jsonDataSets->GetChild();
    while (data && data->IsValid()) {
        auto key = data->GetKey();
        auto valStr = data->GetString();
        if (valStr.empty()) {
            valStr = data->ToString();
        }
        static const LinearMapNode<void (*)(const std::string&, BadgeConfig&)> badgeConfigOperators[] = {
            { DOM_BADGE_COLOR,
                [](const std::string& valStr, BadgeConfig& badgeConfig) {
                    badgeConfig.badgeColor = { Color::FromString(valStr), true };
                } },
            { DOM_BADGE_CIRCLE_SIZE,
                [](const std::string& valStr, BadgeConfig& badgeConfig) {
                    badgeConfig.badgeSize = { StringToDimension(valStr), true };
                } },
            { DOM_BADGE_TEXT_COLOR,
                [](const std::string& valStr, BadgeConfig& badgeConfig) {
                    badgeConfig.textColor = { Color::FromString(valStr), true };
                } },
            { DOM_BADGE_TEXT_FONT_SIZE,
                [](const std::string& valStr, BadgeConfig& badgeConfig) {
                    badgeConfig.textSize = { StringToDimension(valStr), true };
                } },
        };
        auto operatorIter = BinarySearchFindIndex(badgeConfigOperators, ArraySize(badgeConfigOperators), key.c_str());
        if (operatorIter != -1) {
            badgeConfigOperators[operatorIter].value(valStr, badgeConfig);
        } else {
            LOGD("key : %{public}s unsupported. Ignoring!", key.c_str());
        }
        data = data->GetNext();
    }
}

bool GetStrValue(const std::string& key, std::stack<std::string>& keyStack)
{
    auto topKey = keyStack.top();
    auto data = JsonUtil::ParseJsonString(topKey);
    if (!data || !data->IsValid() || !data->Contains(key)) {
        return false;
    }
    keyStack.pop();
    auto dataValue = data->GetValue(key);
    auto dataStr = dataValue->IsString() ? dataValue->GetString() : dataValue->ToString();
    keyStack.emplace(dataStr);
    return true;
}

bool GetIndexValue(const std::string& key, std::stack<std::string>& keyStack)
{
    auto topValue = keyStack.top();
    auto data = JsonUtil::ParseJsonString(topValue);
    auto index = StringToInt(key);
    if (!data || !data->IsValid() || !data->IsArray() || (data->IsArray() && index >= data->GetArraySize())) {
        return false;
    }
    keyStack.pop();
    auto dataValue = data->GetArrayItem(index);
    auto dataStr = dataValue->IsString() ? dataValue->GetString() : dataValue->ToString();
    keyStack.emplace(dataStr);
    return true;
}

bool IsVariable(const std::string& value)
{
    return StartWith(value, "{{") && EndWith(value, "}}");
}

bool IsMultiVariable(const std::string& value)
{
    return StartWith(value, "$f(") && EndWith(value, ")");
}

} // namespace

void JsCardParser::ParseAttributes(const std::unique_ptr<JsonValue>& rootJson, int32_t nodeId,
    std::vector<std::pair<std::string, std::string>>& attrs, JsCommandDomElementOperator* command)
{
    auto attrList = rootJson->GetValue("attr");
    if (!attrList || !attrList->IsValid()) {
        return;
    }
    auto attr = attrList->GetChild();
    while (attr && attr->IsValid()) {
        auto key = attr->GetKey();
        auto value = attr->IsString() ? attr->GetString() : attr->ToString();
        static const LinearMapNode<void (*)(std::string&, JsCommandDomElementOperator*, JsCardParser&)>
            attrOperators[] = {
                { "clockconfig",
                    [](std::string& value, JsCommandDomElementOperator* command, JsCardParser& jsCardParser) {
                        ClockConfig clockConfig;
                        jsCardParser.ParseSpecialAttr<ClockConfig>(
                            std::bind(&JsCardParser::GetClockConfig, &jsCardParser, std::placeholders::_1,
                                std::placeholders::_2),
                            value, clockConfig);
                        command->SetClockConfig(clockConfig);
                    } },
                { "config",
                    [](std::string& value, JsCommandDomElementOperator* command, JsCardParser& jsCardParser) {
                        BadgeConfig badgeConfig;
                        jsCardParser.ParseSpecialAttr<BadgeConfig>(GetBadgeConfig, value, badgeConfig);
                        command->SetBadgeConfig(badgeConfig);
                    } },
                { "datasets",
                    [](std::string& value, JsCommandDomElementOperator* command, JsCardParser& jsCardParser) {
                        std::vector<MainChart> datasets;
                        jsCardParser.ParseSpecialAttr<MainChart>(GetAttrDatasets, value, datasets);
                        command->SetDatasets(datasets);
                    } },
                { "options",
                    [](std::string& value, JsCommandDomElementOperator* command, JsCardParser& jsCardParser) {
                        ChartOptions options;
                        jsCardParser.ParseSpecialAttr<ChartOptions>(GetAttrOptions, value, options);
                        command->SetOptions(options);
                    } },
                { "segments",
                    [](std::string& value, JsCommandDomElementOperator* command, JsCardParser& jsCardParser) {
                        std::vector<Segment> segments;
                        jsCardParser.ParseSpecialAttr<Segment>(ParseSegments, value, segments);
                        command->SetSegments(segments);
                    } },
            };
        auto operatorIter = BinarySearchFindIndex(attrOperators, ArraySize(attrOperators), key.c_str());
        if (operatorIter != -1) {
            attrOperators[operatorIter].value(value, command, *this);
            attr = attr->GetNext();
            continue;
        }
        if (IsVariable(value)) {
            ParseVariable(value);
        } else if (IsMultiVariable(value)) {
            ParseMultiVariable(value);
        }
        attrs.emplace_back(std::make_pair(key, value));
        attr = attr->GetNext();
    }
}

bool JsCardParser::GetShownValue(std::string& value)
{
    std::vector<std::string> splitResult;
    StringUtils::SplitStr(value, "&&", splitResult);
    bool showValue = true;
    std::vector<std::pair<std::string, bool>> shownVariable;
    for (const auto& splitStr : splitResult) {
        if (IsVariable(splitStr)) {
            auto key = splitStr;
            ParseVariable(key);
            showValue &= StringToBool(key);
        } else if (StartWith(splitStr, "!{{") && EndWith(splitStr, "}}")) {
            // !{{value}} --> {{value}}
            auto key = splitStr.substr(1, splitStr.size() - 1);
            ParseVariable(key);
            showValue &= !StringToBool(key);
        } else {
            return false;
        }
    }
    value = showValue ? TRUE_STR : FALSE_STR;
    return true;
}

bool JsCardParser::GetRepeatData(std::unique_ptr<JsonValue>& repeatValue, std::string& key)
{
    if (!repeatValue || !repeatValue->IsValid()) {
        LOGE("GetRepeatData failed, repeat value is invalid.");
        return false;
    }
    auto dataValue = repeatValue->IsString() ? repeatValue->GetString() : repeatValue->ToString();
    if (IsVariable(dataValue)) {
        // {{value}} --> value
        key = dataValue.substr(2, dataValue.size() - 4);
        if (!dataJson_->Contains(key)) {
            return false;
        }
        repeatValue = dataJson_->GetValue(key);
    }
    return true;
}

void JsCardParser::SetRepeatItemValue(uint32_t index, const std::unique_ptr<JsonValue>& repeatValue, bool hasKeyValue)
{
    if (!repeatValue || !repeatValue->IsValid()) {
        LOGE("SetRepeatItemValue failed, repeat value is invalid.");
        return;
    }
    auto idx = std::to_string(index);
    auto itemValue = repeatValue->GetArrayItem(index);

    // update $idx and $item value.
    repeatJson_->Contains(REPEAT_INDEX) ? repeatJson_->Replace(REPEAT_INDEX.c_str(), idx.c_str())
                                        : repeatJson_->Put(REPEAT_INDEX.c_str(), idx.c_str());
    repeatJson_->Contains(REPEAT_ITEM) ? repeatJson_->Replace(REPEAT_ITEM.c_str(), itemValue)
                                       : repeatJson_->Put(REPEAT_ITEM.c_str(), itemValue);
    if (!hasKeyValue) {
        return;
    }
    // update the user-defined index and item value.
    if (!repeatIndex_.empty()) {
        repeatJson_->Contains(repeatIndex_) ? repeatJson_->Replace(repeatIndex_.c_str(), idx.c_str())
                                            : repeatJson_->Put(repeatIndex_.c_str(), idx.c_str());
    }
    if (!repeatItem_.empty()) {
        repeatJson_->Contains(repeatItem_) ? repeatJson_->Replace(repeatItem_.c_str(), itemValue)
                                           : repeatJson_->Put(repeatItem_.c_str(), itemValue);
    }
}

void JsCardParser::ParseRepeatIndexItem(const std::unique_ptr<JsonValue>& repeatValue)
{
    if (!repeatValue || !repeatValue->IsValid()) {
        LOGE("ParseRepeatIndexItem failed, repeat value is invalid.");
        return;
    }
    if (repeatValue->Contains("key")) {
        repeatIndex_ = repeatValue->GetValue("key")->GetString();
    }
    if (repeatValue->Contains("value")) {
        repeatItem_ = repeatValue->GetValue("value")->GetString();
    }
}
void JsCardParser::ResetRepeatIndexItem()
{
    repeatIndex_.clear();
    repeatItem_.clear();
}

void JsCardParser::LoadResImageUrl(const std::string& jsonFile, const std::string& splitStr, std::string& value)
{
    auto content = resourceJson_->GetValue(jsonFile);
    if (!content || !content->IsValid()) {
        LOGE("LoadResImageUrl failed, content is invalid.");
        return;
    }
    std::vector<std::string> keys;
    StringUtils::StringSpliter(splitStr, '.', keys);
    auto result = GetJsonValue(keys, content);
    if (result && result->IsValid()) {
        value = result->GetString();
    }
}

void JsCardParser::GetResImageUrl(std::string& value)
{
    static std::array<std::string, 2> themeArray = { "", "dark" };
    auto mode = static_cast<int32_t>(colorMode_);
    // $r('value') --> value
    auto splitStr = value.substr(4, value.size() - 6);
    auto iter = imageUrlMap_.find(splitStr + themeArray[mode]);
    if (iter != imageUrlMap_.end()) {
        value = iter->second;
        return;
    }
    auto assetManager = assetManager_.Upgrade();
    std::string imagePath;
    auto jsonFile = std::string(RESOURCES_FOLDER) + "res-" + themeArray[mode] + (themeArray[mode].empty() ? "" : "-") +
                    GetDeviceDpi(density_) + std::string(FILE_TYPE_JSON);
    LoadResImageUrl(jsonFile, splitStr, imagePath);

    if (!themeArray[mode].empty() && imagePath.empty()) {
        jsonFile =
            std::string(RESOURCES_FOLDER) + "res-" + themeArray[mode] + "-" + "defaults" + std::string(FILE_TYPE_JSON);
        LoadResImageUrl(jsonFile, splitStr, imagePath);
    }

    if (imagePath.empty()) {
        jsonFile = std::string(RESOURCES_FOLDER) + std::string(DEFAULTS_RESOURCES_JSON_FILE);
        LoadResImageUrl(jsonFile, splitStr, imagePath);
    }
    value = imagePath;
    imageUrlMap_.emplace(splitStr + themeArray[mode], imagePath);
}

bool JsCardParser::GetI18nData(std::string& value)
{
    std::vector<std::string> keys;
    StringUtils::StringSpliter(value, '.', keys);
    std::vector<std::string> files;
    auto context = context_.Upgrade();
    if (!context) {
        return false;
    }

    if (!AceApplicationInfo::GetInstance().GetFiles(context->GetInstanceId(), I18N_FOLDER, files)) {
        LOGE("Get i18n files fail!");
        return false;
    }

    std::vector<std::string> fileNameList;
    for (const auto& file : files) {
        if (EndWith(file, FILE_TYPE_JSON)) {
            fileNameList.emplace_back(file.substr(0, file.size() - (sizeof(FILE_TYPE_JSON) - 1)));
        }
    }
    auto priorityFileName = AceApplicationInfo::GetInstance().GetLocaleFallback(fileNameList);
    for (const auto& fileName : priorityFileName) {
        auto filePath = std::string(I18N_FOLDER) + fileName + std::string(FILE_TYPE_JSON);
        std::string content;
        auto assetManager = assetManager_.Upgrade();
        if (GetAssetContentImpl(assetManager, filePath, content)) {
            auto fileData = ParseFileData(content);
            auto result = GetJsonValue(keys, fileData);
            if (result && result->IsValid()) {
                value = result->IsString() ? result->GetString() : result->ToString();
                return true;
            }
        }
    }
    return false;
}

void JsCardParser::GetPlurals(std::string& value)
{
    // $tc('strings.plurals', 2) --> 'strings.plurals', 2
    auto variable = value.substr(4, value.size() - 5);
    std::vector<std::string> splitStr;
    StringUtils::SplitStr(variable, ",", splitStr);
    if (splitStr.size() != 2) {
        return;
    }
    if (!StartWith(splitStr[0], "'") || !EndWith(splitStr[0], "'")) {
        return;
    }
    // 'strings.plurals' --> strings.plurals
    auto tempStr = splitStr[0].substr(1, splitStr[0].size() - 2);
    GetI18nData(tempStr);
    auto plurals = ParseFileData(tempStr);
    auto key = Localization::GetInstance()->PluralRulesFormat(StringUtils::StringToDouble(splitStr[1]));
    if (plurals->IsValid() && plurals->Contains(key)) {
        value = plurals->GetValue(key)->GetString();
    }
}

void JsCardParser::ParseStyles(const std::unique_ptr<JsonValue>& rootJson, int32_t nodeId,
    std::vector<std::pair<std::string, std::string>>& styles)
{
    // parse class style
    auto classList = rootJson->GetValue("classList");
    if (classList && classList->IsValid()) {
        auto styleList = classList->GetChild();
        while (styleList && styleList->IsValid()) {
            auto value = styleList->GetString();
            if (IsVariable(value)) {
                ParseVariable(value);
            }
            std::string styleClass("." + value);
            SelectStyle(styleClass, styleJson_, styles);
            SelectMediaQueryStyle(styleClass, styles);
            styleList = styleList->GetNext();
        }
    }

    // parse id style
    if (rootJson->Contains("id")) {
        auto value = rootJson->GetValue("id")->GetString();
        if (IsVariable(value)) {
            ParseVariable(value);
        }
        std::string idStyle("#" + value);
        SelectStyle(idStyle, styleJson_, styles);
        SelectMediaQueryStyle(idStyle, styles);
    }

    // parse inline style
    ParseInlineStyles(rootJson, styles);
}

void JsCardParser::ParseInlineStyles(
    const std::unique_ptr<JsonValue>& rootJson, std::vector<std::pair<std::string, std::string>>& styles)
{
    auto styleList = rootJson->GetValue("style");
    if (!styleList || !styleList->IsValid()) {
        return;
    }
    auto style = styleList->GetChild();
    while (style && style->IsValid()) {
        auto key = style->GetKey();
        auto value = style->IsString() ? style->GetString() : style->ToString();
        if (IsVariable(value)) {
            ParseVariable(value);
        }
        if (key == "fontFamily") {
            RegisterFont(value);
        }
        styles.emplace_back(std::make_pair(key, value));
        style = style->GetNext();
    }
}

void JsCardParser::SelectStyle(const std::string& className, const std::unique_ptr<JsonValue>& styleClass,
    std::vector<std::pair<std::string, std::string>>& styles)
{
    auto styleDetail = styleClass->GetValue(className);
    if (!styleDetail || !styleDetail->IsValid()) {
        return;
    }
    auto style = styleDetail->GetChild();
    while (style && style->IsValid()) {
        auto key = style->GetKey();
        auto styleValue = style->IsString() ? style->GetString() : style->ToString();
        if (key == "fontFamily") {
            RegisterFont(styleValue);
        }
        styles.emplace_back(std::make_pair(key, styleValue));
        style = style->GetNext();
    }
}

void JsCardParser::SelectMediaQueryStyle(
    const std::string& styleClass, std::vector<std::pair<std::string, std::string>>& styles)
{
    const auto& mediaFeature = mediaQueryer_.GetMediaFeature();
    for (const auto& iter : mediaQueryStyles_) {
        if (mediaQueryer_.MatchCondition(iter.first, mediaFeature)) {
            auto mediaIter = mediaQueryStyles_.find(iter.first);
            if (mediaIter != mediaQueryStyles_.end()) {
                SelectStyle(styleClass, mediaIter->second, styles);
            }
        }
    }
}

void JsCardParser::RegisterFont(const std::string& fontFamily)
{
    auto fontFaceValue = styleJson_->GetValue("@FONT-FACE");
    if (!fontFaceValue || !fontFaceValue->IsValid()) {
        return;
    }
    auto fontFace = fontFaceValue->GetChild();
    while (fontFace && fontFace->IsValid()) {
        if (fontFace->GetValue("fontFamily")->GetString() == fontFamily) {
            auto fontSrc = fontFace->GetValue("src")->GetString();
            if (fontSrc.size() > 7) {
                // url("src") --> src
                fontSrc = fontSrc.substr(5, fontSrc.size() - 7);
                auto context = context_.Upgrade();
                if (context) {
                    context->RegisterFont(fontFamily, fontSrc);
                    return;
                }
            }
        }
        fontFace = fontFace->GetNext();
    }
}

void JsCardParser::ParseEvents(const std::unique_ptr<JsonValue>& rootJson, std::vector<std::string>& events,
    const RefPtr<Framework::JsAcePage>& page, int32_t nodeId)
{
    auto eventList = rootJson->GetValue("events");
    if (!eventList || !eventList->IsValid()) {
        return;
    }
    auto event = eventList->GetChild();
    while (event && event->IsValid()) {
        auto key = event->GetKey();
        auto value = event->GetString();
        events.emplace_back(key);
        auto action = eventJson_->GetValue(value)->ToString();
        auto eventAction = GetEventAction(action, key, nodeId);
        page->AddNodeEvent(nodeId, key, eventAction);
        event = event->GetNext();
    }
}

std::string JsCardParser::GetEventAction(const std::string& action, const std::string& actionType, int32_t nodeId)
{
    auto actionDetail = JsonUtil::ParseJsonString(action);
    if (!actionDetail || !actionDetail->IsValid()) {
        LOGE("GetEventAction: action detail is invalid");
        return "";
    }
    auto child = actionDetail->GetChild();
    while (child && child->IsValid()) {
        auto key = child->GetKey();
        auto value = child->IsString() ? child->GetString() : child->ToString();
        child = child->GetNext();
        if (key != "params") {
            auto originValue = value;
            if (IsVariable(value)) {
                ParseVariable(value);
                actionDetail->Replace(key.c_str(), value.c_str());
            }
        } else {
            auto jsonParamDetail = actionDetail->GetValue("params");
            if (jsonParamDetail->IsValid()) {
                auto pChild = jsonParamDetail->GetChild();
                while (pChild->IsValid()) {
                    auto paramKey = pChild->GetKey();
                    auto paramValue = pChild->IsString() ? pChild->GetString() : child->ToString();
                    pChild = pChild->GetNext();
                    auto originParamValue = paramValue;
                    if (IsVariable(paramValue)) {
                        ParseVariable(paramValue);
                        jsonParamDetail->Replace(paramKey.c_str(), paramValue.c_str());
                    }
                }
            }
            actionDetail->Replace("params", jsonParamDetail);
        }
    }
    return actionDetail->ToString();
}

void JsCardParser::LoadMediaQueryStyle()
{
    auto media = styleJson_->GetValue("@MEDIA");
    if (!media || !media->IsValid()) {
        return;
    }
    auto mediaChild = media->GetChild();
    while (mediaChild && mediaChild->IsValid()) {
        auto condition = mediaChild->GetString("condition", "");
        if (condition.empty()) {
            mediaChild = mediaChild->GetNext();
            continue;
        }

        // record media query style
        auto mediaQueryStyle = JsonUtil::Create(true);
        auto child = mediaChild->GetChild();
        while (child && child->IsValid()) {
            mediaQueryStyle->Put(child->GetKey().c_str(), child);
            child = child->GetNext();
        }

        mediaQueryStyles_[condition] = std::move(mediaQueryStyle);
        mediaChild = mediaChild->GetNext();
    }
}

void JsCardParser::GetResourceValue(const std::string& path)
{
    auto assetManager = assetManager_.Upgrade();
    if (!assetManager) {
        return;
    }
    std::string content;
    if (GetAssetContentImpl(assetManager, path, content) && !content.empty()) {
        auto jsonBody = ParseFileData(content);
        resourceJson_->Put(path.c_str(), jsonBody);
    }
}

void JsCardParser::LoadImageInfo()
{
    resourceJson_ = JsonUtil::Create(true);

    std::string path = std::string(RESOURCES_FOLDER) + std::string(DEFAULTS_RESOURCES_JSON_FILE);
    GetResourceValue(path);
    path = std::string(RESOURCES_FOLDER) + "res-" + GetDeviceDpi(density_) + std::string(FILE_TYPE_JSON);
    GetResourceValue(path);
    path = std::string(RESOURCES_FOLDER) + "res-" + "dark-defaults" + std::string(FILE_TYPE_JSON);
    GetResourceValue(path);
    path = std::string(RESOURCES_FOLDER) + "res-" + "dark-" + GetDeviceDpi(density_) + std::string(FILE_TYPE_JSON);
    GetResourceValue(path);
}

void JsCardParser::UpdatePageData(const std::string& dataList, const RefPtr<JsAcePage>& page)
{
    LOGI("update data is %{private}s", dataList.c_str());
    if (!page || dataList.empty()) {
        LOGE("update data is null");
        return;
    }
    const auto& rootData = JsonUtil::ParseJsonString(dataList);
    if (!rootData || !rootData->IsValid()) {
        return;
    }
    auto data = rootData->GetChild();
    if (!data || !data->IsValid()) {
        LOGE("update card data error");
        return;
    }
    while (data && data->IsValid()) {
        auto key = data->GetKey();
        dataJson_->Replace(key.c_str(), data);
        repeatJson_->Replace(key.c_str(), data);
        data = data->GetNext();
    }
    SetUpdateStatus(page);
}

void JsCardParser::UpdateStyle(const RefPtr<JsAcePage>& page)
{
    if (!page) {
        LOGE("update color mode error");
        return;
    }
    SetUpdateStatus(page);
}

bool JsCardParser::ParseComplexExpression(std::string& value)
{
    if (value.find('[') == std::string::npos && value.find('.') == std::string::npos) {
        return false;
    }
    std::stack<std::string> keyStack;
    auto key = value;
    if (!ParseArrayExpression(key, keyStack) || keyStack.size() != 1) {
        return false;
    }
    auto result = keyStack.top();
    keyStack.pop();
    value = result;
    return true;
}

bool JsCardParser::ParseArrayExpression(const std::string& expression, std::stack<std::string>& keyStack)
{
    auto dataJson = isRepeat_ ? repeatJson_->ToString() : dataJson_->ToString();
    auto dataValue = JsonUtil::ParseJsonString(dataJson);
    std::string tmpKey;
    for (char i : expression) {
        switch (i) {
            case '[':
                if (tmpKey.empty()) {
                    return false;
                }
                if (!ParsePointOperator(tmpKey, keyStack, dataJson)) {
                    return false;
                }
                tmpKey.clear();
                break;
            case ']':
                if (StringUtils::IsNumber(tmpKey)) {
                    if (!GetIndexValue(tmpKey, keyStack)) {
                        return false;
                    }
                } else if (tmpKey.empty() && keyStack.size() >= 2) {
                    auto topValue = keyStack.top();
                    keyStack.pop();
                    if (!GetStrValue(topValue, keyStack) && !GetIndexValue(topValue, keyStack)) {
                        return false;
                    }
                } else if (!tmpKey.empty() && dataValue->Contains(tmpKey)) {
                    auto data = dataValue->GetValue(tmpKey);
                    auto dataStr = data->IsString() ? data->GetString() : data->ToString();
                    if (!GetStrValue(dataStr, keyStack) && !GetIndexValue(dataStr, keyStack)) {
                        return false;
                    }
                } else if (tmpKey.find('.') != std::string::npos) {
                    std::vector<std::string> keys;
                    StringUtils::StringSpliter(tmpKey, '.', keys);
                    auto jsonValue = GetJsonValue(keys, dataValue);
                    if (jsonValue && jsonValue->IsValid()) {
                        auto result = jsonValue->IsString() ? jsonValue->GetString() : jsonValue->ToString();
                        if (!GetStrValue(result, keyStack) && !GetIndexValue(result, keyStack)) {
                            return false;
                        }
                    }
                } else {
                    return false;
                }
                tmpKey.clear();
                break;
            default: {
                tmpKey += i;
                break;
            }
        }
    }
    if (!tmpKey.empty()) {
        return ParsePointOperator(tmpKey, keyStack, dataJson);
    }
    return true;
}

void JsCardParser::UpdateDomNode(const RefPtr<Framework::JsAcePage>& page, const std::unique_ptr<JsonValue>& rootJson,
    int32_t parentId, const std::vector<int>& idArray)
{
    if (!page || !rootJson->IsValid()) {
        LOGE("fail to CreateDomNode due to page or root is invalid");
        return;
    }
    if (rootJson->Contains("repeat") && !isRepeat_) {
        CreateRepeatDomNode(page, rootJson, parentId);
        return;
    }
    auto type = rootJson->GetString("type");
    if (type == "block") {
        CreateBlockNode(page, rootJson, parentId);
        return;
    }
    int32_t selfId = 0;
    if (!isRepeat_) {
        selfId = parentId < 0 ? DOM_ROOT_NODE_ID_BASE : nodeId_;
        ++nodeId_;
    } else {
        if (listNodeIndex_ < static_cast<int32_t>(idArray.size())) {
            selfId = idArray[listNodeIndex_];
            ++listNodeIndex_;
        }
    }
    bool shouldShow = true;
    bool hasShownAttr = false;
    GetShownAttr(rootJson, shouldShow, hasShownAttr);

    std::vector<std::pair<std::string, std::string>> attrs;
    std::vector<std::pair<std::string, std::string>> styles;
    std::vector<std::string> events;

    auto styleCommand = Referenced::MakeRefPtr<JsCommandUpdateDomElementStyles>(selfId);
    auto attrCommand = Referenced::MakeRefPtr<JsCommandUpdateDomElementAttrs>(selfId);
    auto ptr = Referenced::RawPtr(attrCommand);
    if (shouldShow && hasShownAttr) {
        attrs.emplace_back(std::make_pair("show", TRUE_STR));
    }
    ParseAttributes(rootJson, selfId, attrs, (JsCommandDomElementOperator*)ptr);
    if (!shouldShow && hasShownAttr) {
        attrs.emplace_back(std::make_pair("show", FALSE_STR));
    }
    ParseStyles(rootJson, selfId, styles);
    ParseEvents(rootJson, events, page, selfId);
    attrCommand->SetAttributes(std::move(attrs));
    styleCommand->SetStyles(std::move(styles));
    page->PushCommand(attrCommand);
    page->PushCommand(styleCommand);

    auto childList = rootJson->GetValue("children");
    if (childList && childList->IsValid()) {
        auto child = childList->GetChild();
        while (child && child->IsValid()) {
            UpdateDomNode(page, child, selfId, idArray);
            child = child->GetNext();
        }
    }
}

void JsCardParser::ParseVariable(std::string& value)
{
    // {{value}} --> value
    auto variable = value.substr(2, value.size() - 4);
    if (ParseComplexExpression(variable) || GetVariable(variable) || ParseSpecialVariable(variable) ||
        ParseTernaryExpression(variable) || ParseLogicalExpression(variable)) {
        value = variable;
    }
}

void JsCardParser::ParseMultiVariable(std::string& value)
{
    if (value.size() < 4) {
        return;
    }
    // $f({{key1}} {{key2}}) --> {{key1}} {{key2}}
    auto variable = value.substr(3, value.size() - 4);
    std::vector<std::string> splitStr;
    StringUtils::SplitStr(variable, ".", splitStr);
    std::string resultStr;
    for (auto& split : splitStr) {
        if (IsVariable(split)) {
            ParseVariable(split);
        }
        resultStr += (split + '.');
    }
    value = resultStr.substr(0, resultStr.size() - 1);
}

bool JsCardParser::ParseTernaryExpression(std::string& value)
{
    if (value.find('?') == std::string::npos || value.find(':') == std::string::npos) {
        return false;
    }
    // eg:{{flag ? key1 : key2}}, flagStr[0] = "flag", flagStr[1] = "key1 : key2".
    std::vector<std::string> flagStr;
    StringUtils::SplitStr(value, "?", flagStr);
    if (flagStr.size() != 2) {
        return false;
    }
    GetVariable(flagStr[0]);
    auto flag = flagStr[0] == TRUE_STR;

    std::vector<std::string> keyStr;
    StringUtils::SplitStr(flagStr[1], ":", keyStr);
    if (keyStr.size() != 2) {
        return false;
    }
    // parse key1 and key2.
    GetVariable(keyStr[0]);
    GetVariable(keyStr[1]);
    value = flag ? keyStr[0] : keyStr[1];
    return true;
}

bool JsCardParser::ParseLogicalExpression(std::string& value)
{
    std::vector<std::string> splitStr;
    if (value.find("&&") != std::string::npos) {
        // for {{flag1 && flag2}}.
        StringUtils::SplitStr(value, "&&", splitStr);
        if (splitStr.size() != 2) {
            return false;
        }
        GetVariable(splitStr[0]);
        GetVariable(splitStr[1]);
        value = (splitStr[0] == TRUE_STR && splitStr[1] == TRUE_STR) ? TRUE_STR : FALSE_STR;
        return true;
    } else if (value.find("||") != std::string::npos) {
        // for {{flag1 || flag2}}.
        StringUtils::SplitStr(value, "||", splitStr);
        if (splitStr.size() != 2) {
            return false;
        }
        GetVariable(splitStr[0]);
        GetVariable(splitStr[1]);
        value = (splitStr[0] == TRUE_STR || splitStr[1] == TRUE_STR) ? TRUE_STR : FALSE_STR;
        return true;
    } else if (value.find('!') != std::string::npos) {
        // for {{!flag1}}.
        StringUtils::SplitStr(value, "!", splitStr);
        if (splitStr.size() != 1) {
            return false;
        }
        GetVariable(splitStr[0]);
        value = splitStr[0] == TRUE_STR ? FALSE_STR : TRUE_STR;
        return true;
    }
    return false;
}

bool JsCardParser::ParseSpecialVariable(std::string& value)
{
    if (StartWith(value, "$r('") && EndWith(value, "')")) {
        GetResImageUrl(value);
        return true;
    } else if (StartWith(value, "$t('") && EndWith(value, "')")) {
        // $t('value') --> value
        value = value.substr(4, value.size() - 6);
        return GetI18nData(value);
    } else if (StartWith(value, "$tc(") && EndWith(value, ")")) {
        GetPlurals(value);
        return true;
    }
    return false;
}

bool JsCardParser::GetVariable(std::string& value)
{
    auto key = value;
    if (!repeatJson_->Contains(key)) {
        return false;
    }
    auto dataValue = dataJson_->GetValue(key);
    if (isRepeat_) {
        dataValue = repeatJson_->GetValue(key);
    }
    if (!dataValue || !dataValue->IsValid()) {
        return false;
    }
    value = dataValue->IsString() ? dataValue->GetString() : dataValue->ToString();
    if (IsVariable(value)) {
        ParseVariable(value);
    }
    return true;
}

template<typename T>
void JsCardParser::ParseSpecialAttr(
    const std::function<void(const std::unique_ptr<JsonValue>&, std::vector<T>&)>& function, std::string& value,
    std::vector<T>& vector)
{
    if (IsVariable(value)) {
        ParseVariable(value);
        auto jsonValue = JsonUtil::ParseJsonString(value);
        function(jsonValue, vector);
    } else {
        auto jsonValue = JsonUtil::ParseJsonString(value);
        function(jsonValue, vector);
    }
}

template<typename T>
void JsCardParser::ParseSpecialAttr(
    const std::function<void(const std::unique_ptr<JsonValue>&, T&)>& function, std::string& variable, T& value)
{
    if (IsVariable(variable)) {
        ParseVariable(variable);
        auto jsonValue = JsonUtil::ParseJsonString(variable);
        function(jsonValue, value);
    } else {
        auto jsonValue = JsonUtil::ParseJsonString(variable);
        function(jsonValue, value);
    }
}

bool JsCardParser::ParsePointOperator(
    const std::string& tmpKey, std::stack<std::string>& keyStack, const std::string& dataJson)
{
    std::unique_ptr<JsonValue> data;
    if (tmpKey[0] == '.') {
        if (keyStack.empty()) {
            return false;
        }
        data = JsonUtil::ParseJsonString(keyStack.top());
        keyStack.pop();
    } else {
        data = JsonUtil::ParseJsonString(dataJson);
    }
    std::vector<std::string> keys;
    StringUtils::StringSpliter(tmpKey, '.', keys);
    auto result = GetJsonValue(keys, data);
    if (result && result->IsValid()) {
        auto dataStr = result->IsString() ? result->GetString() : result->ToString();
        keyStack.emplace(dataStr);
        return true;
    }
    return false;
}

void JsCardParser::CreateDomNode(const RefPtr<Framework::JsAcePage>& page, const std::unique_ptr<JsonValue>& rootJson,
    int32_t parentId, bool isNewNode)
{
    if (!page || !rootJson->IsValid()) {
        LOGE("fail to CreateDomNode due to page or root is invalid");
        EventReport::SendFormException(FormExcepType::CREATE_NODE_ERR);
        return;
    }
    if (rootJson->Contains("repeat") && !isRepeat_) {
        CreateRepeatDomNode(page, rootJson, parentId);
        return;
    }
    auto type = rootJson->GetString("type");
    if (type == "block") {
        CreateBlockNode(page, rootJson, parentId);
        return;
    }
    int32_t nodeId = 0;
    if (!isNewNode) {
        nodeId = parentId < 0 ? DOM_ROOT_NODE_ID_BASE : nodeId_;
        ++nodeId_;
    } else {
        nodeId = maxNodeId_++;
    }
    if (isRepeat_) {
        idArray_.emplace_back(nodeId);
    }
    bool shouldShow = true;
    bool hasShownAttr = false;
    GetShownAttr(rootJson, shouldShow, hasShownAttr);

    std::vector<std::pair<std::string, std::string>> attrs;
    std::vector<std::pair<std::string, std::string>> styles;
    std::vector<std::string> events;

    RefPtr<Framework::JsCommandDomElementCreator> command;
    if (parentId < 0) {
        command = Referenced::MakeRefPtr<Framework::JsCommandCreateDomBody>(type, nodeId);
    } else {
        command = Referenced::MakeRefPtr<Framework::JsCommandAddDomElement>(type, nodeId, parentId);
    }
    command->SetPipelineContext(context_);
    auto ptr = Referenced::RawPtr(command);
    if (shouldShow && hasShownAttr) {
        attrs.emplace_back(std::make_pair("show", TRUE_STR));
    }
    ParseAttributes(rootJson, nodeId, attrs, (Framework::JsCommandDomElementOperator*)ptr);
    if (!shouldShow && hasShownAttr) {
        attrs.emplace_back(std::make_pair("show", FALSE_STR));
    }
    LOGD("card nodeId: %d, parentId: %d, type: %s", nodeId, parentId, type.c_str());
    ParseStyles(rootJson, nodeId, styles);
    ParseEvents(rootJson, events, page, nodeId);
    command->SetAttributes(std::move(attrs));
    command->SetStyles(std::move(styles));
    command->AddEvents(std::move(events));
    page->PushCommand(command);

    auto childList = rootJson->GetValue("children");
    if (childList && childList->IsValid()) {
        auto child = childList->GetChild();
        while (child && child->IsValid()) {
            CreateDomNode(page, child, nodeId, isNewNode);
            child = child->GetNext();
        }
    }
}

void JsCardParser::CreateRepeatDomNode(
    const RefPtr<Framework::JsAcePage>& page, const std::unique_ptr<JsonValue>& rootJson, int32_t parentId)
{
    auto repeatValue = rootJson->GetValue("repeat");
    if (!repeatValue || !repeatValue->IsValid()) {
        LOGE("CreateRepeatDomNode failed, repeat value is invalid.");
        return;
    }
    isRepeat_ = true;
    std::string key;
    if (repeatValue->IsString()) {
        // eg: repeatValue = {{list}}.
        if (!GetRepeatData(repeatValue, key)) {
            LOGE("CreateRepeatDomNode failed, root data does not contains repeat value.");
            isRepeat_ = false;
            return;
        }
        ProcessRepeatNode(page, rootJson, key, parentId, false, repeatValue);
    } else {
        // eg: repeatValue = {"exp": {{list}}, "key":"index", "value": "item"}.
        if (!repeatValue->Contains("exp")) {
            LOGE("CreateRepeatDomNode failed, repeat value does not contains exp.");
            isRepeat_ = false;
            return;
        }
        auto expValue = repeatValue->GetValue("exp");
        if (!GetRepeatData(expValue, key)) {
            LOGE("CreateRepeatDomNode failed, root data does not contains exp value.");
            isRepeat_ = false;
            return;
        }
        ParseRepeatIndexItem(repeatValue);
        ProcessRepeatNode(page, rootJson, key, parentId, true, repeatValue);
        ResetRepeatIndexItem();
    }
    isRepeat_ = false;
}

void JsCardParser::GetClockConfig(const std::unique_ptr<JsonValue>& jsonDataSets, ClockConfig& clockConfig)
{
    if (!jsonDataSets || !jsonDataSets->IsValid()) {
        return;
    }
    auto data = jsonDataSets->GetChild();
    while (data && data->IsValid()) {
        auto key = data->GetKey();
        auto valStr = data->IsString() ? data->GetString() : data->ToString();
        static const LinearMapNode<void (*)(std::string&, ClockConfig&, JsCardParser&)> clockConfigOperators[] = {
            { DOM_DIGIT_COLOR, [](std::string& valStr, ClockConfig& clockConfig,
                               JsCardParser& jsCardParser) { clockConfig.digitColor_ = Color::FromString(valStr); } },
            { DOM_DIGIT_COLOR_NIGHT,
                [](std::string& valStr, ClockConfig& clockConfig, JsCardParser& jsCardParser) {
                    clockConfig.digitColorNight_ = Color::FromString(valStr);
                } },
            { DOM_DIGIT_RADIUS_RATIO,
                [](std::string& valStr, ClockConfig& clockConfig, JsCardParser& jsCardParser) {
                    clockConfig.digitRadiusRatio_ = StringToDouble(valStr);
                } },
            { DOM_DIGIT_SIZE_RATIO,
                [](std::string& valStr, ClockConfig& clockConfig, JsCardParser& jsCardParser) {
                    clockConfig.digitSizeRatio_ = StringToDouble(valStr);
                } },
            { DOM_CLOCK_FACE_SOURCE,
                [](std::string& valStr, ClockConfig& clockConfig, JsCardParser& jsCardParser) {
                    if (IsVariable(valStr)) {
                        jsCardParser.ParseVariable(valStr);
                    }
                    clockConfig.clockFaceSrc_ = valStr;
                } },
            { DOM_CLOCK_FACE_SOURCE_NIGHT,
                [](std::string& valStr, ClockConfig& clockConfig, JsCardParser& jsCardParser) {
                    if (IsVariable(valStr)) {
                        jsCardParser.ParseVariable(valStr);
                    }
                    clockConfig.clockFaceNightSrc_ = valStr;
                } },
            { DOM_HOUR_HAND_SOURCE,
                [](std::string& valStr, ClockConfig& clockConfig, JsCardParser& jsCardParser) {
                    if (IsVariable(valStr)) {
                        jsCardParser.ParseVariable(valStr);
                    }
                    clockConfig.hourHandSrc_ = valStr;
                } },
            { DOM_HOUR_HAND_SOURCE_NIGHT,
                [](std::string& valStr, ClockConfig& clockConfig, JsCardParser& jsCardParser) {
                    if (IsVariable(valStr)) {
                        jsCardParser.ParseVariable(valStr);
                    }
                    clockConfig.hourHandNightSrc_ = valStr;
                } },
            { DOM_MINUTE_HAND_SOURCE,
                [](std::string& valStr, ClockConfig& clockConfig, JsCardParser& jsCardParser) {
                    if (IsVariable(valStr)) {
                        jsCardParser.ParseVariable(valStr);
                    }
                    clockConfig.minuteHandSrc_ = valStr;
                } },
            { DOM_MINUTE_HAND_SOURCE_NIGHT,
                [](std::string& valStr, ClockConfig& clockConfig, JsCardParser& jsCardParser) {
                    if (IsVariable(valStr)) {
                        jsCardParser.ParseVariable(valStr);
                    }
                    clockConfig.minuteHandNightSrc_ = valStr;
                } },
            { DOM_SECOND_HAND_SOURCE,
                [](std::string& valStr, ClockConfig& clockConfig, JsCardParser& jsCardParser) {
                    if (IsVariable(valStr)) {
                        jsCardParser.ParseVariable(valStr);
                    }
                    clockConfig.secondHandSrc_ = valStr;
                } },
            { DOM_SECOND_HAND_SOURCE_NIGHT,
                [](std::string& valStr, ClockConfig& clockConfig, JsCardParser& jsCardParser) {
                    if (IsVariable(valStr)) {
                        jsCardParser.ParseVariable(valStr);
                    }
                    clockConfig.secondHandNightSrc_ = valStr;
                } },
        };
        auto operatorIter = BinarySearchFindIndex(clockConfigOperators, ArraySize(clockConfigOperators), key.c_str());
        if (operatorIter != -1) {
            clockConfigOperators[operatorIter].value(valStr, clockConfig, *this);
        } else {
            LOGD("key : %{public}s unsupported. Ignoring!", key.c_str());
        }
        data = data->GetNext();
    }
}

void JsCardParser::ProcessRepeatNode(const RefPtr<Framework::JsAcePage>& page,
    const std::unique_ptr<JsonValue>& rootJson, const std::string& key, int32_t parentId, bool hasKeyValue,
    std::unique_ptr<JsonValue>& repeatValue)
{
    ++numberOfForNode_;
    if (parsingStatus_ == ParsingStatus::CREATE) {
        if (repeatValue->GetArraySize() == 0) {
            listIdMap_.emplace(numberOfForNode_, std::make_pair(idArray_, 0));
            singleLoopMap_.emplace(key, 0);
            return;
        }
        for (auto i = 0; i < repeatValue->GetArraySize(); i++) {
            SetRepeatItemValue(i, repeatValue, hasKeyValue);
            CreateDomNode(page, rootJson, parentId);
        }
        // Number of nodes in a single loop
        auto factor = idArray_.size() / repeatValue->GetArraySize();
        listIdMap_.emplace(numberOfForNode_, std::make_pair(idArray_, static_cast<int32_t>(idArray_.size())));
        singleLoopMap_.emplace(key, factor);
    } else if (parsingStatus_ == ParsingStatus::UPDATE) {
        auto iter = listIdMap_.find(numberOfForNode_);
        auto loopIter = singleLoopMap_.find(key);
        if (iter == listIdMap_.end() || loopIter == singleLoopMap_.end()) {
            return;
        }
        auto array = iter->second.first;
        auto factor = loopIter->second;
        int32_t lastSize = factor > 0 ? array.size() / factor : 0;
        auto updateSize = std::min(static_cast<int32_t>(lastSize), repeatValue->GetArraySize());
        for (auto i = 0; i < updateSize; ++i) {
            SetRepeatItemValue(i, repeatValue, hasKeyValue);
            UpdateDomNode(page, rootJson, parentId, array);
        }

        if (repeatValue->GetArraySize() > lastSize) {
            for (auto i = lastSize; i < repeatValue->GetArraySize(); ++i) {
                SetRepeatItemValue(i, repeatValue, hasKeyValue);
                CreateDomNode(page, rootJson, parentId, true);
            }
            if (factor == 0) {
                factor = static_cast<int32_t>(idArray_.size()) / repeatValue->GetArraySize();
                loopIter->second = factor;
            }
            iter->second.first.insert(iter->second.first.end(), idArray_.begin(), idArray_.end());
        } else if (lastSize > repeatValue->GetArraySize()) {
            for (int32_t i = array.size() - factor; i >= repeatValue->GetArraySize() * factor; i = i - factor) {
                auto nodeId = iter->second.first[i];
                page->PushCommand(Referenced::MakeRefPtr<JsCommandRemoveDomElement>(nodeId));
            }
            iter->second.first.erase(iter->second.first.end() - (array.size() - repeatValue->GetArraySize() * factor),
                iter->second.first.end());
        }
        nodeId_ += iter->second.second;
    }
    idArray_.clear();
    listNodeIndex_ = 0;
}

void JsCardParser::SetUpdateStatus(const RefPtr<Framework::JsAcePage>& page)
{
    parsingStatus_ = ParsingStatus::UPDATE;
    UpdateDomNode(page, rootJson_, -1);
    nodeId_ = 0;
    numberOfForNode_ = 0;
    page->FlushCommands();
}

void JsCardParser::GetShownAttr(const std::unique_ptr<JsonValue>& rootJson, bool& shouldShow, bool& hasShownAttr)
{
    GetBoolValue(rootJson, "shown", shouldShow, hasShownAttr);
    bool blockValue = false;
    bool hasBlock = false;
    GetBoolValue(rootJson, BLOCK_VALUE, blockValue, hasBlock);
    if (hasBlock) {
        shouldShow &= blockValue;
        hasShownAttr = true;
    }
}

bool JsCardParser::Initialize()
{
    rootJson_ = rootBody_->GetValue("template");
    styleJson_ = rootBody_->GetValue("styles");
    eventJson_ = rootBody_->GetValue("actions");
    dataJson_ = rootBody_->GetValue("data");

    if (!rootJson_ || !styleJson_ || !eventJson_ || !dataJson_) {
        LOGE("the json template is nullptr");
        return false;
    }
    if (!rootJson_->IsValid() || !styleJson_->IsValid() || !eventJson_->IsValid() || !dataJson_->IsValid()) {
        LOGE("the json template is error");
        return false;
    }
    // repeatJson contains dataJson.
    repeatJson_ = JsonUtil::ParseJsonString(dataJson_->ToString());
    LoadMediaQueryStyle();
    return true;
}

void JsCardParser::OnSurfaceChanged(int32_t width, int32_t height)
{
    mediaQueryer_.SetSurfaceSize(width, height);
}

void JsCardParser::CreateBlockNode(const OHOS::Ace::RefPtr<OHOS::Ace::Framework::JsAcePage>& page,
    const std::unique_ptr<JsonValue>& rootJson, int32_t parentId)
{
    auto blockChild = rootJson->GetValue("children");
    bool shouldShow = true;
    bool hasShownAttr = false;
    GetBoolValue(rootJson, "shown", shouldShow, hasShownAttr);
    if (blockChild && blockChild->IsValid()) {
        auto child = blockChild->GetChild();
        while (child && child->IsValid()) {
            if (child->Contains(BLOCK_VALUE)) {
                shouldShow ? child->Replace(BLOCK_VALUE, "true") : child->Replace(BLOCK_VALUE, "false");
            } else {
                shouldShow ? child->Put(BLOCK_VALUE, "true") : child->Put(BLOCK_VALUE, "false");
            }
            parsingStatus_ == ParsingStatus::UPDATE ? UpdateDomNode(page, child, parentId)
                                                    : CreateDomNode(page, child, parentId);
            child = child->GetNext();
        }
    }
}

void JsCardParser::GetBoolValue(
    const std::unique_ptr<JsonValue>& rootJson, const std::string& key, bool& shouldShow, bool& hasShownAttr)
{
    auto shown = rootJson->GetValue(key);
    if (shown && shown->IsValid()) {
        if (shown->IsString()) {
            auto shownValue = shown->GetString();
            GetShownValue(shownValue);
            shouldShow = shownValue == "true";
            hasShownAttr = true;
        } else if (shown->IsBool()) {
            shouldShow = shown->GetBool();
            hasShownAttr = true;
        }
    }
}

} // namespace OHOS::Ace::Framework
