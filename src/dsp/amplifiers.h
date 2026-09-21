#pragma once

namespace duallc {

// Analog-style gain stage: linear gain plus a gentle tanh residual so hot
// preamp settings round over like a line amp instead of a digital brickwall.
class AnalogAmp
{
public:
    void prepare (double sampleRate, float smoothSeconds = 0.015f);
    void reset () noexcept;

    void setGainDb (float db) noexcept;
    float gainDb () const noexcept { return gainDb_; }

    void smoothParams () noexcept;
    float process (float x) noexcept;

private:
    float gainDb_ = 0.f;
    float gainTarget_ = 1.f;
    float gainSmoothed_ = 1.f;
    float smoothCoeff_ = 0.f;
};

} // namespace duallc
