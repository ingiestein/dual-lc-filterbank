#include "amplifiers.h"
#include "grid.h"

#include <algorithm>
#include <cmath>

namespace duallc {

void AnalogAmp::prepare (double sampleRate, float smoothSeconds)
{
    const double fs = std::max (sampleRate, 8000.0);
    const float seconds = std::max (smoothSeconds, 0.001f);
    smoothCoeff_ = 1.f - std::exp (-1.f / (seconds * static_cast<float> (fs)));
    reset ();
}

void AnalogAmp::reset () noexcept
{
    gainSmoothed_ = gainTarget_;
}

void AnalogAmp::setGainDb (float db) noexcept
{
    gainDb_ = db;
    gainTarget_ = dbToLin (db);
}

void AnalogAmp::smoothParams () noexcept
{
    gainSmoothed_ += (gainTarget_ - gainSmoothed_) * smoothCoeff_;
}

float AnalogAmp::process (float x) noexcept
{
    const float lin = x * gainSmoothed_;
    constexpr float kDrive = 0.45f;
    return std::tanh (kDrive * lin) / kDrive;
}

} // namespace duallc
