#pragma once

#include "dual_lc_cell.h"

#include <array>

namespace duallc {

class FilterBank
{
public:
    FilterBank ();

    void prepare (double sampleRate, float smoothSeconds = kSmoothTimeSeconds);
    void reset () noexcept;

    void setCellGainDb (int index, float db) noexcept;
    float cellGainDb (int index) const noexcept;

    // Advance parameter smoothers once per sample, then process a channel.
    void smoothParams () noexcept;
    float processChannel (int channel, float x) noexcept;

    DualLcCell& cell (int index, int channel) noexcept { return cells_[static_cast<size_t> (index)][static_cast<size_t> (channel)]; }
    const DualLcCell& cell (int index, int channel) const noexcept { return cells_[static_cast<size_t> (index)][static_cast<size_t> (channel)]; }

private:
    std::array<std::array<DualLcCell, kNumChannels>, kNumCells> cells_ {};
    std::array<float, kNumCells> mixTarget_ {};
    std::array<float, kNumCells> mixSmoothed_ {};
    std::array<float, kNumCells> gainDb_ {};
    float smoothCoeff_ = 0.f;
};

} // namespace duallc
