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

#include "core/components/track/render_track.h"

#include "base/log/dump_log.h"
#include "core/components/theme/theme_constants.h"

namespace OHOS::Ace {
namespace {

constexpr double MILLISECOND_PER_PERCENT = 20.0;
constexpr double MAX_TRANSITION_TIME = 5000.0;
constexpr double MIN_TRANSITION_TIME = 200.0;
constexpr double SECOND_TO_MILLISECOND = 1000.0;

} // namespace

void RenderTrack::Update(const RefPtr<Component>& component)
{
    LOGD("Slider/Progress::RenderTrack Update");
    const RefPtr<TrackComponent> track = AceType::DynamicCast<TrackComponent>(component);
    if (track == nullptr) {
        LOGE("Slider/Progress::RenderTrack track updata with nullptr");
        return;
    }

    theme_ = GetTheme<ProgressTheme>();
    selectColor_ = track->GetSelectColor();
    selectGradient_ = track->GetSelectGradient();
    backgroundColor_ = track->GetBackgroundColor();
    cachedColor_ = track->GetCachedColor();
    colors_ = track->GetSectionsColors();
    weights_ = track->GetSectionsWeights();
    showIndicator_ = track->GetIndicatorFlag();

    leftToRight_ = track->GetTextDirection() == TextDirection::LTR;

    paintData_.startDegree = track->GetStartDegree();
    paintData_.sweepDegree = track->GetSweepDegree();
    thickness_ = track->GetTrackThickness();
    paintData_.clockwise = track->GetClockwiseValue();
    paintData_.totalScaleNumber = track->GetScaleNumber();
    scaleStrokeWidth_ = track->GetScaleWidth();
    paintData_.radius = track->GetRadius();
    paintData_.center.SetY(track->GetCenterY());
    paintData_.center.SetX(track->GetCenterX());

    auto pipelineContext = GetContext().Upgrade();
    if (!pipelineContext) {
        return;
    }
    if (!progressTransitionController_) {
        progressTransitionController_ = AceType::MakeRefPtr<Animator>(pipelineContext);
    }
    if (!scanHaloController_) {
        scanHaloController_ = AceType::MakeRefPtr<Animator>(pipelineContext);
    }
    MarkNeedLayout();
}

void RenderTrack::PerformLayout()
{
    paintData_.thickness = NormalizeToPx(thickness_);
    paintData_.scaleStrokeWidth = NormalizeToPx(scaleStrokeWidth_);
    Size size = Measure();
    SetLayoutSize(size);
    UpdateAnimation();
}

void RenderTrack::UpdateAnimation()
{
    if (!playAnimation_) {
        return;
    }
    if (needUpdateAnimation_) {
        auto transistionAnimation =
            AceType::MakeRefPtr<CurveAnimation<double>>(totalRatio_, prevousPercentValue_, Curves::EASE_OUT);
        transistionAnimation->AddListener([this](const double& value) {
            totalRatio_ = value;
            MarkNeedRender();
        });
        if (progressTransitionController_) {
            double animationTime = 0.0;
            if (std::abs(percentChange_ * MILLISECOND_PER_PERCENT) > animationDuring_.count() * SECOND_TO_MILLISECOND) {
                animationTime = std::abs(percentChange_ * MILLISECOND_PER_PERCENT);
            } else {
                animationTime = animationDuring_.count() * SECOND_TO_MILLISECOND;
            }

            if (animationTime < MIN_TRANSITION_TIME || prevousPercentValue_ == 100.0) {
                animationTime = MIN_TRANSITION_TIME;
            } else if (animationTime > MAX_TRANSITION_TIME) {
                animationTime = MAX_TRANSITION_TIME;
            }
            progressTransitionController_->ClearInterpolators();
            progressTransitionController_->AddInterpolator(transistionAnimation);
            progressTransitionController_->SetDuration(animationTime);
            progressTransitionController_->SetIteration(1);
            progressTransitionController_->Stop();
            progressTransitionController_->Play();
        }

        auto scanAnimation = AceType::MakeRefPtr<CurveAnimation<double>>(0, prevousPercentValue_, Curves::LINEAR);
        scanAnimation->AddListener([this](const double& value) {
            scanHighLightValue_ = value;
            MarkNeedRender();
        });
        if (scanHaloController_) {
            scanHaloController_->ClearInterpolators();
            scanHaloController_->AddInterpolator(scanAnimation);
            scanHaloController_->SetDuration(1500);
            scanHaloController_->SetIteration(ANIMATION_REPEAT_INFINITE);
            scanHaloController_->Play();
        }
    }
}

Size RenderTrack::Measure()
{
    if (GetLayoutParam().GetMaxSize().IsInfinite()) {
        auto defaultWidth = theme_ != nullptr ? NormalizeToPx(theme_->GetTrackWidth()) : 0.0;
        return Size(defaultWidth, paintData_.thickness);
    }
    Size layoutSize;
    layoutSize.SetHeight(paintData_.thickness);
    layoutSize.SetWidth(GetLayoutParam().GetMaxSize().Width());
    return layoutSize;
}

void RenderTrack::Dump()
{
    DumpLog::GetInstance().AddDesc(std::string("RenderTrack: ").append(GetLayoutSize().ToString()));
}

Size RenderCircleTrack::Measure()
{
    if (GetLayoutParam().GetMaxSize().IsInfinite()) {
        double diameter = theme_ != nullptr ? NormalizeToPx(theme_->GetRingDiameter()) : 0.0;
        diameter =
            std::min(diameter, std::min(GetLayoutParam().GetMaxSize().Width(), GetLayoutParam().GetMaxSize().Height()));
        return Size(diameter, diameter);
    }
    double diameter = std::min(GetLayoutParam().GetMaxSize().Height(), GetLayoutParam().GetMaxSize().Width());
    return Size(diameter, diameter);
}

void RenderCircleTrack::Dump()
{
    DumpLog::GetInstance().AddDesc(std::string("RenderCircleTrack: ").append(GetLayoutSize().ToString()));
}

Size RenderScaleRingTrack::Measure()
{
    if (GetLayoutParam().GetMaxSize().IsInfinite()) {
        double diameter = theme_ != nullptr ? NormalizeToPx(theme_->GetScaleRingDiameter()) : 0.0;
        diameter =
            std::min(diameter, std::min(GetLayoutParam().GetMaxSize().Width(), GetLayoutParam().GetMaxSize().Height()));
        return Size(diameter, diameter);
    }
    double diameter = std::min(GetLayoutParam().GetMaxSize().Height(), GetLayoutParam().GetMaxSize().Width());
    return Size(diameter, diameter);
}

void RenderScaleRingTrack::Dump()
{
    DumpLog::GetInstance().AddDesc(std::string("RenderScaleRingTrack: ").append(GetLayoutSize().ToString()));
}

Size RenderArcTrack::Measure()
{
    if (GetLayoutParam().GetMaxSize().IsInfinite()) {
        double diameter = theme_ != nullptr ? NormalizeToPx(theme_->GetRingDiameter()) : 0.0;
        diameter =
            std::min(diameter, std::min(GetLayoutParam().GetMaxSize().Width(), GetLayoutParam().GetMaxSize().Height()));
        return Size(diameter, diameter);
    }
    double diameter = std::min(GetLayoutParam().GetMaxSize().Height(), GetLayoutParam().GetMaxSize().Width());
    return Size(diameter, diameter);
}

void RenderArcTrack::Dump()
{
    DumpLog::GetInstance().AddDesc(std::string("RenderArcTrack: ").append(GetLayoutSize().ToString()));
}

Size RenderMoonTrack::Measure()
{
    if (GetLayoutParam().GetMaxSize().IsInfinite()) {
        double diameter = theme_ != nullptr ? NormalizeToPx(theme_->GetRingDiameter()) : 0.0;
        diameter =
            std::min(diameter, std::min(GetLayoutParam().GetMaxSize().Width(), GetLayoutParam().GetMaxSize().Height()));
        return Size(diameter, diameter);
    }
    double diameter = std::min(GetLayoutParam().GetMaxSize().Height(), GetLayoutParam().GetMaxSize().Width());
    return Size(diameter, diameter);
}

void RenderMoonTrack::Dump()
{
    DumpLog::GetInstance().AddDesc(std::string("RenderMoonTrack: ").append(GetLayoutSize().ToString()));
}

} // namespace OHOS::Ace
