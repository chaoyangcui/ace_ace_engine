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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_ANIMATION_CURVE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_ANIMATION_CURVE_H

#include "base/log/log.h"
#include "base/memory/ace_type.h"
#include "base/utils/macros.h"
#include "base/utils/utils.h"

namespace OHOS::Ace {

constexpr double SQUARE = 2.0;

// The running time of the curve needs to be normalized to the interval of 0.0 to 1.0;
// Relatively, the corresponding value of the curve also needs to be normalized to the interval of 0.0 to 1.0;
class Curve : public AceType {
    DECLARE_ACE_TYPE(Curve, AceType);

public:
    Curve() = default;
    ~Curve() override = default;

    // Returns the value at specific time.
    // Notice: The value of time must be between 0.0 and 1.0 since the curve uses normalized timestamp.
    float Move(float time)
    {
        // time always between 0.0 and 1.0
        return MoveInternal(time);
    }

    // Each subclass needs to override this method to implement motion in the 0.0 to 1.0 time range.
    virtual float MoveInternal(float time) = 0;
};

// The reverse curve is used to convert the direction of motion.
// It uses the 1.0->0.0 motion of the child curve to achieve the reverse purpose.
class ReverseCurve final : public Curve {
public:
    explicit ReverseCurve(const RefPtr<Curve>& curve) : curve_(curve) {}
    ~ReverseCurve() override = default;

    float MoveInternal(float time) final
    {
        if (!curve_) {
            LOGE("Curve is null, illegal constructor input parameter");
            return 0.0f;
        }

        if (time > 1.0f || time < 0.0f) {
            return curve_->MoveInternal(0.0f);
        }
        return curve_->MoveInternal(1.0f - time);
    }

private:
    const RefPtr<Curve> curve_;
};

class ComplementaryCurve final : public Curve {
public:
    explicit ComplementaryCurve(const RefPtr<Curve>& curve) : curve_(curve) {}
    ~ComplementaryCurve() override = default;

    float MoveInternal(float time) final
    {
        if (!curve_) {
            LOGE("Curve is null, illegal constructor input parameter");
            return 0.0f;
        }

        if (time > 1.0f || time < 0.0f) {
            return curve_->MoveInternal(0.0f);
        }
        return 1.0f - curve_->MoveInternal(time);
    }

private:
    const RefPtr<Curve> curve_;
};

class DecelerationCurve final : public Curve {
public:
    float MoveInternal(float time) override
    {
        return 1.0f - std::pow(1.0f - time, SQUARE);
    }
};

class LinearCurve final : public Curve {
public:
    float MoveInternal(float time) override
    {
        return time;
    }
};

class SineCurve final : public Curve {
public:
    float MoveInternal(float time) override
    {
        static constexpr float PI = 3.14f;
        return std::sin(PI * time / 2.0f); // half period
    }
};

class ElasticsCurve final : public Curve {
public:
    explicit ElasticsCurve(float tension) : tension_(tension) {};
    ~ElasticsCurve() override = default;
    float MoveInternal(float time) override
    {
        float para = time - 1.0f;
        return para * para * ((tension_ + 1.0f) * para + tension_) + 1.0f;
    }

private:
    float tension_ = 2.0f; // Default Elastics tension.
};

enum class StepsCurvePosition {
    START,
    END,
};

class StepsCurve : public Curve {
public:
    explicit StepsCurve(int32_t steps, StepsCurvePosition position = StepsCurvePosition::START)
        : steps_(steps <= 0 ? 1 : steps), position_(position) {};
    ~StepsCurve() override = default;

    float MoveInternal(float time) override
    {
        auto currentStep = static_cast<int32_t>(time * steps_);
        if (position_ == StepsCurvePosition::START) {
            currentStep++;
        }
        return static_cast<float>(currentStep) / steps_;
    }

private:
    int32_t steps_;
    const StepsCurvePosition position_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_ANIMATION_CURVE_H
