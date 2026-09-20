#pragma once

#include "grid.h"

namespace duallc {

// Constant-K dual-LC cell: LP/HP are terminated L-sections (2nd order);
// bandpass is the series-LC arm plus its dual shunt-LC arm (two cascaded
// biquads, one per resonator).
class DualLcCell
{
public:
    DualLcCell () = default;

    void setKind (CellKind kind) noexcept { kind_ = kind; }
    void setCutoff (float freqHz) noexcept { freqHz_ = freqHz; }

    CellKind kind () const noexcept { return kind_; }
    float cutoff () const noexcept { return freqHz_; }
    int order () const noexcept { return order_; }

    void prepare (double sampleRate);
    void reset () noexcept;

    // Linear tank response. Peak of a bandpass cell is unity at fc.
    float processLinear (float x) noexcept;

    // v1 identity. Replace this to add inductor saturation without touching VST3.
    float processNonlinear (float y) noexcept;

    float process (float x) noexcept
    {
        return processNonlinear (processLinear (x));
    }

    const float* b () const noexcept { return sos_[0].b; }
    const float* a () const noexcept { return sos_[0].a; }

private:
    struct Sos
    {
        float b[3] {};
        float a[3] {};
        float z[2] {};

        void reset () noexcept { z[0] = z[1] = 0.f; }

        float process (float x) noexcept
        {
            const float y = b[0] * x + z[0];
            z[0] = b[1] * x - a[1] * y + z[1];
            z[1] = b[2] * x - a[2] * y;
            return y;
        }
    };

    void rebuildCoeffs ();
    void setSosFromAnalog (int index, const double* bS, const double* aS, double c);

    CellKind kind_ = CellKind::Bandpass;
    float freqHz_ = 1000.f;
    double sampleRate_ = 44100.0;
    int order_ = 2;
    Sos sos_[2] {};
};

} // namespace duallc
