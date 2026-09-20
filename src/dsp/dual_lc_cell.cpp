#include "dual_lc_cell.h"
#include "bilinear.h"

#include <algorithm>
#include <cmath>

namespace duallc {

namespace {

// Normalized prototype (ω0 = 1, R0 = 1) series inductance that yields Q ≈ 0.8
// for the terminated constant-K dual-LC bandpass L-section.
constexpr double kBandpassProtoL = 1.02;

// Quadratic factors of the palindromic analog denominator at ω0 = 1:
//   (s^2 + 0.68598 s + 2.33003)(s^2 + 0.29441 s + 0.42918)
// These are the series-LC arm and its dual shunt-LC arm.
constexpr double kBpSosA1[2] = {0.6859827481893406, 0.2944094086734050};
constexpr double kBpSosA0[2] = {2.3300299785945930, 0.4291790273888120};
constexpr double kBpGain = 1.0 / (kBandpassProtoL * kBandpassProtoL);

} // namespace

void DualLcCell::prepare (double sampleRate)
{
    sampleRate_ = std::max (sampleRate, 8000.0);
    rebuildCoeffs ();
    reset ();
}

void DualLcCell::reset () noexcept
{
    sos_[0].reset ();
    sos_[1].reset ();
}

void DualLcCell::setSosFromAnalog (int index, const double* bS, const double* aS, double c)
{
    double bZ[5] {};
    double aZ[5] {};
    bilinearTransform (bS, aS, 2, c, bZ, aZ);
    auto& s = sos_[static_cast<size_t> (index)];
    s.b[0] = static_cast<float> (bZ[0]);
    s.b[1] = static_cast<float> (bZ[1]);
    s.b[2] = static_cast<float> (bZ[2]);
    s.a[0] = 1.f;
    s.a[1] = static_cast<float> (aZ[1]);
    s.a[2] = static_cast<float> (aZ[2]);
}

void DualLcCell::rebuildCoeffs ()
{
    const double fs = sampleRate_;
    const double w = prewarpedOmega (static_cast<double> (freqHz_), fs);
    const double c = 2.0 * fs;
    const double q = static_cast<double> (kCellQ);

    sos_[1] = {};

    if (kind_ == CellKind::Bandpass)
    {
        order_ = 4;
        // Two cascaded resonators: H = g * Π_i  (w s) / (s^2 + α_i w s + β_i w^2)
        for (int i = 0; i < 2; ++i)
        {
            const double gain = (i == 0) ? (kBpGain * w) : w;
            double bS[5] {};
            double aS[5] {};
            bS[1] = gain;
            aS[2] = 1.0;
            aS[1] = kBpSosA1[i] * w;
            aS[0] = kBpSosA0[i] * w * w;
            setSosFromAnalog (i, bS, aS, c);
        }
    }
    else
    {
        order_ = 2;
        double bS[5] {};
        double aS[5] {};
        const double wn2 = w * w;
        const double a1 = w / q;
        aS[2] = 1.0;
        aS[1] = a1;
        aS[0] = wn2;
        if (kind_ == CellKind::Lowpass)
            bS[0] = wn2;
        else
            bS[2] = 1.0;
        setSosFromAnalog (0, bS, aS, c);
    }
}

float DualLcCell::processLinear (float x) noexcept
{
    float y = sos_[0].process (x);
    if (order_ >= 4)
        y = sos_[1].process (y);
    return y;
}

float DualLcCell::processNonlinear (float y) noexcept
{
    // Linear L/C in v1. Future inductor saturation can warp `y` or SOS state here.
    return y;
}

} // namespace duallc
