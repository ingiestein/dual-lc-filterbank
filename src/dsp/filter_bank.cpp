#include "filter_bank.h"

#include <algorithm>
#include <cmath>

namespace duallc {

FilterBank::FilterBank ()
{
    for (int i = 0; i < kNumCells; ++i)
    {
        for (int ch = 0; ch < kNumChannels; ++ch)
        {
            cells_[static_cast<size_t> (i)][static_cast<size_t> (ch)].setKind (kCellGrid[static_cast<size_t> (i)].kind);
            cells_[static_cast<size_t> (i)][static_cast<size_t> (ch)].setCutoff (kCellGrid[static_cast<size_t> (i)].freqHz);
        }
    }
}

void FilterBank::prepare (double sampleRate, float smoothSeconds)
{
    const double fs = std::max (sampleRate, 8000.0);
    const float seconds = std::max (smoothSeconds, 0.001f);
    smoothCoeff_ = 1.f - std::exp (-1.f / (seconds * static_cast<float> (fs)));

    for (int i = 0; i < kNumCells; ++i)
        for (int ch = 0; ch < kNumChannels; ++ch)
            cells_[static_cast<size_t> (i)][static_cast<size_t> (ch)].prepare (fs);

    reset ();
}

void FilterBank::reset () noexcept
{
    for (int i = 0; i < kNumCells; ++i)
    {
        mixSmoothed_[static_cast<size_t> (i)] = mixTarget_[static_cast<size_t> (i)];
        for (int ch = 0; ch < kNumChannels; ++ch)
            cells_[static_cast<size_t> (i)][static_cast<size_t> (ch)].reset ();
    }
}

void FilterBank::setCellGainDb (int index, float db) noexcept
{
    if (index < 0 || index >= kNumCells)
        return;
    db = std::clamp (db, kCellGainMinDb, kCellGainMaxDb);
    gainDb_[static_cast<size_t> (index)] = db;
    mixTarget_[static_cast<size_t> (index)] = mixFromGainDb (db);
}

float FilterBank::cellGainDb (int index) const noexcept
{
    if (index < 0 || index >= kNumCells)
        return 0.f;
    return gainDb_[static_cast<size_t> (index)];
}

void FilterBank::smoothParams () noexcept
{
    for (int i = 0; i < kNumCells; ++i)
    {
        const float t = mixTarget_[static_cast<size_t> (i)];
        mixSmoothed_[static_cast<size_t> (i)] += (t - mixSmoothed_[static_cast<size_t> (i)]) * smoothCoeff_;
    }
}

float FilterBank::processChannel (int channel, float x) noexcept
{
    const int ch = std::clamp (channel, 0, kNumChannels - 1);
    float sum = x; // dry path: 0 dB cell gains cancel the mix contribution
    for (int i = 0; i < kNumCells; ++i)
    {
        const float tank = cells_[static_cast<size_t> (i)][static_cast<size_t> (ch)].process (x);
        sum += mixSmoothed_[static_cast<size_t> (i)] * tank;
    }
    return sum;
}

} // namespace duallc
