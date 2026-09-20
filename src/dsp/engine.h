#pragma once

#include "amplifiers.h"
#include "filter_bank.h"

namespace duallc {

class Engine
{
public:
    void prepare (double sampleRate);
    void reset () noexcept;

    void setCellGainDb (int index, float db) noexcept { bank_.setCellGainDb (index, db); }
    void setPreampDb (float db) noexcept { preamp_.setGainDb (db); }
    void setOutputDb (float db) noexcept { output_.setGainDb (db); }

    float process (int channel, float x) noexcept
    {
        const float driven = preamp_.process (x);
        const float summed = bank_.processChannel (channel, driven);
        return output_.process (summed);
    }

    void processStereoSample (float inL, float inR, float& outL, float& outR) noexcept
    {
        preamp_.smoothParams ();
        output_.smoothParams ();
        bank_.smoothParams ();
        outL = process (0, inL);
        outR = process (1, inR);
    }

    void processStereo (const float* inL, const float* inR, float* outL, float* outR, int numSamples) noexcept
    {
        for (int n = 0; n < numSamples; ++n)
            processStereoSample (inL[n], inR[n], outL[n], outR[n]);
    }

    FilterBank& bank () noexcept { return bank_; }
    AnalogAmp& preamp () noexcept { return preamp_; }
    AnalogAmp& output () noexcept { return output_; }

private:
    FilterBank bank_;
    AnalogAmp preamp_;
    AnalogAmp output_;
};

inline void Engine::prepare (double sampleRate)
{
    bank_.prepare (sampleRate, kSmoothTimeSeconds);
    preamp_.prepare (sampleRate, kSmoothTimeSeconds);
    output_.prepare (sampleRate, kSmoothTimeSeconds);
}

inline void Engine::reset () noexcept
{
    bank_.reset ();
    preamp_.reset ();
    output_.reset ();
}

} // namespace duallc
