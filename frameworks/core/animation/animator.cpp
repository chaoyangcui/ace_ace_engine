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

#include "core/animation/animator.h"

#include "base/utils/utils.h"

namespace OHOS::Ace {
namespace {

int32_t g_controllerId = 0;
int32_t AllocControllerId()
{
    return ++g_controllerId;
}

} // namespace

// Static Functions.
float Animator::scale_ = 1.0f;

void Animator::SetDurationScale(float scale)
{
    if (scale < 0.0f) {
        LOGE("Invalid scale value: %{public}f", scale);
        return;
    }
    scale_ = scale;
}

float Animator::GetDurationScale()
{
    return scale_;
}

// Public Functions.
Animator::Animator()
{
    controllerId_ = AllocControllerId();
}

Animator::Animator(const WeakPtr<PipelineContext>& context)
{
    controllerId_ = AllocControllerId();
    AttachScheduler(context);
}

Animator::~Animator()
{
    LOGD("Animator destructor. id:%{public}d", controllerId_);
    // Clear all listeners first to make its destruction silently.
    ClearAllListeners();
    ClearInterpolators();
    if (!IsStopped()) {
        Stop();
    }
}

void Animator::AttachScheduler(const WeakPtr<PipelineContext>& context)
{
    auto&& callback = [weak = AceType::WeakClaim(this)](uint64_t duration) {
        auto controller = weak.Upgrade();
        if (!controller) {
            LOGE("controller is nullptr, skip frame callback.");
            return;
        }
        controller->OnFrame(duration);
    };
    scheduler_ = SchedulerBuilder::Build(callback, context);
}

bool Animator::HasScheduler() const
{
    return scheduler_ != nullptr;
}

void Animator::AddInterpolator(const RefPtr<Interpolator>& animation)
{
    if (animation) {
        interpolators_.emplace_back(animation);
    }
}

void Animator::RemoveInterpolator(const RefPtr<Interpolator>& animation)
{
    interpolators_.remove(animation);
}

void Animator::ClearInterpolators()
{
    interpolators_.clear();
}

void Animator::AddProxyController(const RefPtr<Animator>& proxy)
{
    if (!proxy) {
        LOGE("Add Proxy Controller failed. controller is null.");
        return;
    }
    if (RawPtr(proxy) != this) {
        proxyControllers_.emplace_back(proxy);
        proxy->Copy(Claim(this));
        proxy->scheduler_.Reset();
    }
}

void Animator::RemoveProxyController(const RefPtr<Animator>& proxy)
{
    proxyControllers_.remove(proxy);
}

void Animator::ClearProxyControllers()
{
    proxyControllers_.clear();
}

Animator::Status Animator::GetStatus() const
{
    return status_;
}

bool Animator::IsStopped() const
{
    return status_ == Status::STOPPED;
}

bool Animator::IsRunning() const
{
    return status_ == Status::RUNNING;
}

// When the animation is in the delayed start phase.
bool Animator::IsPending() const
{
    if ((status_ == Status::RUNNING) || (status_ == Status::PAUSED)) {
        return elapsedTime_ < startDelay_;
    } else {
        return false;
    }
}

int32_t Animator::GetDuration() const
{
    return duration_;
}

void Animator::SetDuration(int32_t duration)
{
    if (duration < 0) {
        LOGE("invalid duration time, keep the old. id: %{public}d", controllerId_);
        return;
    }
    LOGD("set duration: %{public}d. id: %{public}d", duration, controllerId_);
    if ((status_ == Status::RUNNING || status_ == Status::PAUSED) && duration != 0) {
        // Need to update elapsedTime when animation running or paused.
        elapsedTime_ = (duration_ / duration) * elapsedTime_;
    }

    if (isDeclarativeAnimator) {
        // if status is not idle, finish current animation and init animation
        if (isLastAnimationInterrupted_) {
            duration = elapsedTime_ - startDelay_;
            isLastAnimationInterrupted_ = false;
        }
        elapsedTime_ = 0;
    }

    duration_ = duration;
    for (auto& controller : proxyControllers_) {
        controller->SetDuration(duration);
    }
}

bool Animator::SetIteration(int32_t iteration)
{
    if (iteration_ == iteration) {
        return true;
    }
    if ((iteration < 0) && (iteration != ANIMATION_REPEAT_INFINITE)) {
        LOGE("invalid iteration: %{public}d. id: %{public}d", iteration, controllerId_);
        return false;
    }
    // if status is not idle, finish current animation and init animation
    if (status_ != Status::IDLE) {
        Finish();
    }
    LOGD("set iteration: %{public}d. id: %{public}d", iteration, controllerId_);
    UpdateIteration(iteration);
    repeatTimesLeft_ = repeatTimes_;
    for (auto& controller : proxyControllers_) {
        controller->SetIteration(iteration);
    }
    return true;
}

void Animator::SetStartDelay(int32_t startDelay)
{
    if (startDelay < 0) {
        LOGE("invalid startDelay time, keep the old. id: %{public}d", controllerId_);
        return;
    }
    LOGD("set start delay: %{public}d. id: %{public}d", startDelay, controllerId_);
    startDelay_ = startDelay;
    for (auto& controller : proxyControllers_) {
        controller->SetStartDelay(startDelay);
    }
}

void Animator::SetFillMode(FillMode fillMode)
{
    fillMode_ = fillMode;
    for (auto& controller : proxyControllers_) {
        controller->SetFillMode(fillMode);
    }
}

void Animator::SetAnimationDirection(AnimationDirection direction)
{
    direction_ = direction;
    for (auto& controller : proxyControllers_) {
        controller->SetAnimationDirection(direction);
    }
}

// return true, the animation is played backward
// return false, the animation is played forward
bool Animator::GetInitAnimationDirection()
{
    if (repeatTimes_ == ANIMATION_REPEAT_INFINITE) {
        return false;
    }
    if (direction_ == AnimationDirection::ALTERNATE_REVERSE || direction_ == AnimationDirection::REVERSE) {
        isReverse_ = true;
    }
    if (direction_ != AnimationDirection::ALTERNATE) {
        return isReverse_;
    }
    return isReverse_ ? (repeatTimesLeft_ % 2) == 0 : false;
}

void Animator::UpdatePlayedTime(int32_t playedTime)
{
    if (playedTime < 0 || playedTime > duration_) {
        LOGE("UpdatePlayedTime failed. Invalid playedTime:%{public}d", playedTime);
        return;
    }
    if (startDelay_ != 0 || motion_) {
        LOGE("Unsupported UpdatePlayedTime when startDelay or motion");
        return;
    }
    // only support go forward.
    isReverse_ = false;
    isCurDirection_ = false;
    float scale = GetDurationScale();
    int32_t scaledPlayedTime = playedTime * scale;
    elapsedTime_ = scaledPlayedTime;
}

void Animator::TriggerFrame(int32_t playedTime)
{
    if (playedTime < 0 || playedTime > duration_) {
        LOGE("TriggerFrame failed. Invalid playedTime:%{public}d", playedTime);
        return;
    }
    if (startDelay_ != 0 || motion_) {
        LOGE("Unsupported TriggerFrame when startDelay or motion");
        return;
    }
    UpdatePlayedTime(playedTime);
    UpdateScaledTime();
    NotifyPrepareListener();
    NotifyInterpolator(elapsedTime_);
}

void Animator::PlayMotion(const RefPtr<Motion>& motion)
{
    if (!motion) {
        LOGE("PlayMotion failed, motion is null.");
        return;
    }
    interpolators_.clear();
    isReverse_ = false;
    motion_ = motion;
    StartInner(false);
}

void Animator::Play()
{
    if (iteration_ == 0) {
        LOGD("no need to play animation, iteration is zero.");
        return;
    }
    LOGD("Play animation in current direction, id: %{public}d", controllerId_);
    motion_ = nullptr;
    StartInner(false);
}

void Animator::Reverse()
{
    if (iteration_ == 0) {
        LOGD("no need to reverse animation, iteration is zero.");
        return;
    }
    LOGD("Play animation in opposite direction, id: %{public}d", controllerId_);
    motion_ = nullptr;
    ToggleDirection();
    StartInner(true);
}

void Animator::Forward()
{
    if (iteration_ == 0) {
        LOGD("no need to play animation forward, iteration is zero.");
        return;
    }
    if (isReverse_) {
        ToggleDirection();
    }
    Play();
}

void Animator::Backward()
{
    if (iteration_ == 0) {
        LOGD("no need to play animation backward, iteration is zero.");
        return;
    }
    if (isReverse_) {
        ToggleDirection();
    }
    Reverse();
}

void Animator::Pause()
{
    if (iteration_ == 0) {
        LOGD("no need to pause animation, iteration is zero.");
        return;
    }
    if (status_ == Status::PAUSED) {
        LOGW("Already paused, do not need pause again. id: %{public}d", controllerId_);
        return;
    }
    if (status_ == Status::IDLE) {
        Play();
    }
    LOGD("animation pause. id: %{public}d", controllerId_);
    if (scheduler_ && scheduler_->IsActive()) {
        scheduler_->Stop();
    }
    status_ = Status::PAUSED;
    StatusListenable::NotifyPauseListener();
    for (auto& controller : proxyControllers_) {
        controller->Pause();
    }
}

void Animator::Resume()
{
    if (iteration_ == 0) {
        LOGD("no need to resume animation, iteration is zero.");
        return;
    }
    if (status_ == Status::RUNNING) {
        LOGW("Already running, do not need resume again. id: %{public}d", controllerId_);
        return;
    }
    LOGD("animation resume. id: %{public}d", controllerId_);
    if (scheduler_ && !scheduler_->IsActive()) {
        scheduler_->Start();
    }
    status_ = Status::RUNNING;
    isResume_ = true;
    StatusListenable::NotifyResumeListener();
    for (auto& controller : proxyControllers_) {
        controller->Resume();
    }
}

void Animator::Stop()
{
    if (iteration_ == 0) {
        LOGD("no need to stop animation, iteration is zero.");
        return;
    }
    if (status_ == Status::STOPPED) {
        LOGD("Already stop, do not need stop again. id: %{public}d", controllerId_);
        return;
    }
    LOGD("animation stop. id: %{public}d", controllerId_);
    if (!isDeclarativeAnimator) {
        elapsedTime_ = 0;
    }

    repeatTimesLeft_ = repeatTimes_;
    isBothBackwards = false;
    UpdateScaledTime();
    if (scheduler_ && scheduler_->IsActive()) {
        scheduler_->Stop();
    }
    status_ = Status::STOPPED;
    StatusListenable::NotifyStopListener();
    for (auto& controller : proxyControllers_) {
        controller->Stop();
    }
}

void Animator::Finish()
{
    if (iteration_ == 0) {
        LOGD("no need to finish animation, iteration is zero.");
        return;
    }
    if (status_ == Status::STOPPED) {
        LOGW("Already stop, do not need finish again. id: %{public}d", controllerId_);
        return;
    }
    LOGD("animation finish. id: %{public}d", controllerId_);
    if (motion_) {
        LOGD("end action not supports for motion. just stop it. id: %{public}d", controllerId_);
        Stop();
        return;
    }
    repeatTimesLeft_ = 0;
    UpdateScaledTime();
    if (isDeclarativeAnimator) {
        Stop();
        repeatTimesLeft_ = 0;
        isLastAnimationInterrupted_ = true;
    } else {
        OnFrame(((int64_t)scaledStartDelay_) + scaledDuration_);
    }
}

void Animator::Cancel()
{
    if (iteration_ == 0) {
        LOGD("no need to cancel animation, iteration is zero.");
        return;
    }
    if (status_ == Status::IDLE) {
        LOGW("Already in idle, do not need cancel again. id: %{public}d", controllerId_);
        return;
    }
    status_ = Status::IDLE;
    LOGE("animation cancel. id: %{public}d", controllerId_);
    elapsedTime_ = 0;
    repeatTimesLeft_ = repeatTimes_;
    UpdateScaledTime();
    NotifyPrepareListener();
    float normalizedTime = GetNormalizedTime(0.0f, true);
    for (auto& interpolator : interpolators_) {
        interpolator->OnInitNotify(normalizedTime, isReverse_);
    }
    if (scheduler_ && scheduler_->IsActive()) {
        scheduler_->Stop();
    }
    NotifyIdleListener();
}

// Private Functions.
void Animator::OnFrame(int64_t duration)
{
    // notify child first
    for (auto& controller : proxyControllers_) {
        controller->OnFrame(duration);
    }
    if (duration > INT64_MAX - elapsedTime_) {
        LOGW("duration is too big, skip it. id:%{public}d", controllerId_);
        return;
    }
    elapsedTime_ += duration;
    UpdateScaledTime();
    // skip delay time
    if (elapsedTime_ < scaledStartDelay_) {
        if ((fillMode_ == FillMode::BACKWARDS || fillMode_ == FillMode::BOTH) && !isBothBackwards) {
            for (const auto& interpolator : interpolators_) {
                interpolator->OnNormalizedTimestampChanged(0.0f, isReverse_);
            }
            isBothBackwards = true;
        }
        return;
    }
    NotifyPrepareListener();
    // get playedTime
    auto playedTime = elapsedTime_ - scaledStartDelay_;
    // make playedTime in range 0 ~ INT32_MAX(max duration value)
    playedTime = std::clamp<int64_t>(playedTime, 0L, (int64_t)INT32_MAX);
    if (!motion_) {
        NotifyInterpolator(playedTime);
    } else {
        NotifyMotion(playedTime);
    }
}

void Animator::NotifyInterpolator(int32_t playedTime)
{
    bool needStop = false;
    bool notifyRepeat = false;
    // do not notify user when handling playedTime, because users may be change duration or repeat times in callback.
    // if they change this parameter, it will take effect next tick.
    if (playedTime >= scaledDuration_) {
        notifyRepeat = true;
        auto isAlternateDirection =
            (direction_ == AnimationDirection::ALTERNATE) || (direction_ == AnimationDirection::ALTERNATE_REVERSE);
        if ((scaledDuration_ == 0) && isAlternateDirection && (iteration_ % 2 == 0)) {
            isCurDirection_ = !isCurDirection_;
        }
        // make playedTime in range 0 ~ INTERPOLATE_DURATION_MAX
        if (repeatTimesLeft_ == 0 || scaledDuration_ == 0) {
            LOGD("animation stop, repeatTimesLeft: %{public}d, duration: %{public}d, id: %{public}d", repeatTimesLeft_,
                scaledDuration_, controllerId_);
            repeatTimesLeft_ = 0;
            needStop = true;
        } else {
            auto playedLoops = GetPlayedLoopsAndRemaining(playedTime);
            if (repeatTimesLeft_ != ANIMATION_REPEAT_INFINITE) {
                needStop = UpdateRepeatTimesLeftAndCheckFinished(playedLoops);
            }
            if (isAlternateDirection) {
                isCurDirection_ = !isCurDirection_;
            }
            LOGD("animation next loop. repeat times remaining: %{public}d. id: %{public}d", repeatTimesLeft_,
                controllerId_);
        }

        // after the above branches, playedTime in range 0 ~ INTERPOLATE_DURATION_MAX
        // make elapsedTime_ in range 0 ~ scaledStartDelay_ + INTERPOLATE_DURATION_MAX
        elapsedTime_ = playedTime + scaledStartDelay_;
    }

    float normalizedTime = GetNormalizedTime(playedTime, needStop);
    // all calculation above is done, we can notify user from now.
    // users can change controller's configuration, and it will take effect next tick.
    if (notifyRepeat && !needStop) {
        // notify repeat before on timestamp changed.
        StatusListenable::NotifyRepeatListener();
    }
    for (const auto& interpolator : interpolators_) {
        if (needStop && (fillMode_ == FillMode::NONE || fillMode_ == FillMode::BACKWARDS) && !isDeclarativeAnimator) {
            // notify init value set by user.
            interpolator->OnInitNotify(normalizedTime, isReverse_);
        } else {
            // interpolate animation accept normalized time.
            interpolator->OnNormalizedTimestampChanged(normalizedTime, isReverse_);
        }
    }
    if (needStop && (!IsStopped())) {
        // notify stop after on timestamp changed.
        LOGD("animation stop in tick callback.");
        Stop();
    }
}

void Animator::NotifyMotion(int32_t playedTime)
{
    // motion do not have normalized time, because no exact duration in motion.
    // just pass actual time to motion, normalized time always zero.
    motion_->OnTimestampChanged(playedTime, 0.0f, false);
    if (motion_->IsCompleted()) {
        Stop();
    }
}

void Animator::StartInner(bool alwaysNotify)
{
    if (status_ == Status::RUNNING) {
        LOGW("controller already running, do not need start again. skip it. id: %{public}d", controllerId_);
        if (directionChange_) {
            directionChange_ = !directionChange_;
            isCurDirection_ = !isCurDirection_;
        }

        if (alwaysNotify) {
            StatusListenable::NotifyStartListener();
            for (auto& controller : proxyControllers_) {
                controller->StartInner(alwaysNotify);
            }
        }
        return;
    }
    if (scheduler_ && !scheduler_->IsActive()) {
        scheduler_->Start();
    }
    StatusListenable::NotifyStartListener();
    status_ = Status::RUNNING;
    isCurDirection_ = GetInitAnimationDirection();
    for (auto& controller : proxyControllers_) {
        controller->StartInner(alwaysNotify);
    }
}

int32_t Animator::GetPlayedLoopsAndRemaining(int32_t& playedTime)
{
    // when duration equals 0, played loop equals INT32_MAX, and playedTime remains unchanged.
    int32_t playedLoop = INT32_MAX;
    if (scaledDuration_ != 0) {
        // in order to make playedTime in range of 0 ~ INTERPOLATE_DURATION_MAX, calc elapsed loop between two vsyncs
        playedLoop = std::clamp(playedTime / scaledDuration_, 0, INT32_MAX);
        playedTime = playedTime % scaledDuration_;
    }
    return playedLoop;
}

bool Animator::UpdateRepeatTimesLeftAndCheckFinished(int32_t playedLoops)
{
    // get the remaining repeatTimesLeft_
    repeatTimesLeft_ -= playedLoops;
    if (playedLoops > 1) {
        LOGW("too long time between neighbor vsync, elapsed loop count: %{public}d. id: %{public}d", playedLoops,
            controllerId_);
    }
    if (repeatTimesLeft_ < 0) {
        LOGD("elapse too long, no repeat remaining. Loop: %{public}d. id: %{public}d", playedLoops, controllerId_);
        return true;
    }
    return false;
}

void Animator::ToggleDirection()
{
    isReverse_ = !isReverse_;
    LOGD("change direction. isReverse change to: %{public}d, id: %{public}d", isReverse_, controllerId_);
    directionChange_ = !directionChange_;
    if (status_ == Status::IDLE || status_ == Status::STOPPED) {
        LOGD("toggle an idle controller, do not change time related params. id: %{public}d", controllerId_);
        return;
    }
    if (repeatTimes_ == ANIMATION_REPEAT_INFINITE) {
        elapsedTime_ = scaledDuration_ - elapsedTime_;
        LOGI("duration is infinite, can not reverse time related params. id: %{public}d", controllerId_);
        return;
    }
    repeatTimesLeft_ = repeatTimes_ - repeatTimesLeft_;
    LOGD("change left repeat times: %{public}d. id: %{public}d", repeatTimesLeft_, controllerId_);
    elapsedTime_ = scaledDuration_ - elapsedTime_;
}

float Animator::GetNormalizedTime(float playedTime, bool needStop) const
{
    float normalizedTime = 0.0f;
    if (needStop) {
        LOGD("animation stop. set played time to the end value according to fill mode. id: %{public}d", controllerId_);
        if (!isDeclarativeAnimator) {
            switch (fillMode_) {
                case FillMode::FORWARDS:
                    // Fall through.
                case FillMode::BOTH:
                    LOGD("fill mode is FORWARDS or BOTH. id: %{public}d", controllerId_);
                    normalizedTime = 1.0f;
                    break;
                case FillMode::NONE:
                    // Fall through.
                case FillMode::BACKWARDS:
                    LOGD("fill mode is NONE or BACKWARDS. id: %{public}d", controllerId_);
                    normalizedTime = 0.0f;
                    break;
                default:
                    LOGD("unknown fill mode, use default mode. id: %{public}d", controllerId_);
                    normalizedTime = 1.0f;
                    break;
            }
        } else {
            normalizedTime = scaledDuration_ == 0 ? 1.0f : std::min(1.0f, (1.0f * playedTime) / scaledDuration_);
        }
    } else {
        normalizedTime = scaledDuration_ == 0 ? 1.0f : (1.0f * playedTime) / scaledDuration_;
    }
    return isCurDirection_ ? 1.0f - normalizedTime : normalizedTime;
}

void Animator::UpdateScaledTime()
{
    float scale = GetDurationScale();
    scaledDuration_ = duration_ * scale;
    scaledStartDelay_ = startDelay_ * scale;
}

void Animator::UpdateIteration(int32_t iteration)
{
    iteration_ = iteration;
    if (iteration <= 0 && iteration != ANIMATION_REPEAT_INFINITE) {
        repeatTimes_ = 0;
    } else if (iteration == ANIMATION_REPEAT_INFINITE) {
        repeatTimes_ = ANIMATION_REPEAT_INFINITE;
    } else {
        repeatTimes_ = iteration - 1;
    }
}

void Animator::Copy(const RefPtr<Animator>& controller)
{
    if (!controller) {
        return;
    }
    fillMode_ = controller->fillMode_;
    direction_ = controller->direction_;
    isCurDirection_ = controller->isCurDirection_;
    directionChange_ = controller->directionChange_;
    duration_ = controller->duration_;
    elapsedTime_ = controller->elapsedTime_;
    startDelay_ = controller->startDelay_;
    repeatTimes_ = controller->repeatTimes_;
    iteration_ = controller->iteration_;
    repeatTimesLeft_ = controller->repeatTimesLeft_;
    isReverse_ = controller->isReverse_;
    isResume_ = controller->isResume_;
    status_ = controller->status_;
    scaledDuration_ = controller->scaledDuration_;
    scaledStartDelay_ = controller->scaledStartDelay_;
}

} // namespace OHOS::Ace
