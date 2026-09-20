#include "amplifiers.h"
#include "dual_lc_cell.h"
#include "filter_bank.h"
#include "grid.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

constexpr float kPi = 3.14159265358979323846f;

float rms (const std::vector<float>& x, int skip)
{
    double acc = 0;
    int n = 0;
    for (int i = skip; i < static_cast<int> (x.size ()); ++i)
    {
        acc += static_cast<double> (x[static_cast<size_t> (i)]) * x[static_cast<size_t> (i)];
        ++n;
    }
    return n > 0 ? static_cast<float> (std::sqrt (acc / n)) : 0.f;
}

std::vector<float> sine (float freq, float fs, int n, float amp = 1.f)
{
    std::vector<float> y (static_cast<size_t> (n));
    for (int i = 0; i < n; ++i)
        y[static_cast<size_t> (i)] = amp * std::sin (2.f * kPi * freq * static_cast<float> (i) / fs);
    return y;
}

float cellGainAt (duallc::DualLcCell cell, float freq, float fs)
{
    cell.prepare (fs);
    const int n = 16384;
    const int skip = 4096;
    auto in = sine (freq, fs, n, 0.5f);
    std::vector<float> out (static_cast<size_t> (n));
    for (int i = 0; i < n; ++i)
        out[static_cast<size_t> (i)] = cell.process (in[static_cast<size_t> (i)]);
    const float den = rms (in, skip);
    const float num = rms (out, skip);
    return den > 0.f ? num / den : 0.f;
}

float bankGainAt (duallc::FilterBank& bank, int ch, float freq, float fs, float amp = 0.05f)
{
    const int n = 16384;
    const int skip = 4096;
    auto in = sine (freq, fs, n, amp);
    std::vector<float> out (static_cast<size_t> (n));
    for (int i = 0; i < n; ++i)
    {
        bank.smoothParams ();
        out[static_cast<size_t> (i)] = bank.processChannel (ch, in[static_cast<size_t> (i)]);
    }
    return rms (out, skip) / rms (in, skip);
}

} // namespace

TEST_CASE ("grid has 14 half-octave cells")
{
    REQUIRE (duallc::kNumCells == 14);
    REQUIRE (duallc::kCellGrid[0].kind == duallc::CellKind::Lowpass);
    REQUIRE (duallc::kCellGrid[0].freqHz == Catch::Approx (60.f));
    REQUIRE (duallc::kCellGrid[13].kind == duallc::CellKind::Highpass);
    REQUIRE (duallc::kCellGrid[13].freqHz == Catch::Approx (7500.f));
    REQUIRE (duallc::kCellQ == Catch::Approx (0.8f));
}

TEST_CASE ("0 dB cell mix is exactly zero")
{
    REQUIRE (duallc::mixFromGainDb (0.f) == Catch::Approx (0.f).margin (1.0e-6f));
}

TEST_CASE ("filter bank is unity at 0 dB")
{
    for (float fs : duallc::kSupportedSampleRates)
    {
        duallc::FilterBank bank;
        bank.prepare (fs);
        for (int i = 0; i < duallc::kNumCells; ++i)
            bank.setCellGainDb (i, 0.f);
        bank.reset ();

        const float g1k = bankGainAt (bank, 0, 1000.f, fs);
        REQUIRE (g1k == Catch::Approx (1.f).margin (0.02f));

        // Impulse through the dry path is unchanged when mixes are 0.
        bank.reset ();
        REQUIRE (bank.processChannel (0, 1.f) == Catch::Approx (1.f).margin (1.0e-5f));
        REQUIRE (bank.processChannel (0, 0.f) == Catch::Approx (0.f).margin (1.0e-5f));
    }
}

TEST_CASE ("bandpass cells peak near fc with Q near 0.8")
{
    const float fs = 48000.f;
    for (int i = 1; i < duallc::kNumCells - 1; ++i)
    {
        const auto spec = duallc::kCellGrid[static_cast<size_t> (i)];
        duallc::DualLcCell cell;
        cell.setKind (duallc::CellKind::Bandpass);
        cell.setCutoff (spec.freqHz);

        const float peak = cellGainAt (cell, spec.freqHz, fs);
        REQUIRE (peak == Catch::Approx (1.f).margin (0.12f));

        const float low = spec.freqHz * 0.25f;
        const float high = std::min (spec.freqHz * 4.f, fs * 0.45f);
        REQUIRE (cellGainAt (cell, low, fs) < peak * 0.7f);
        REQUIRE (cellGainAt (cell, high, fs) < peak * 0.7f);

        // -3 dB bandwidth search around fc.
        auto mag = [&] (float f) { return cellGainAt (cell, f, fs); };
        const float target = peak / std::sqrt (2.f);
        float lo = spec.freqHz * 0.2f, hi = spec.freqHz;
        for (int n = 0; n < 24; ++n)
        {
            const float mid = 0.5f * (lo + hi);
            if (mag (mid) < target)
                lo = mid;
            else
                hi = mid;
        }
        const float fLo = 0.5f * (lo + hi);
        lo = spec.freqHz;
        hi = spec.freqHz * 3.5f;
        for (int n = 0; n < 24; ++n)
        {
            const float mid = 0.5f * (lo + hi);
            if (mag (mid) < target)
                hi = mid;
            else
                lo = mid;
        }
        const float fHi = 0.5f * (lo + hi);
        const float q = spec.freqHz / (fHi - fLo);
        REQUIRE (q == Catch::Approx (0.8f).margin (0.25f));
    }
}

TEST_CASE ("LP and HP cells sit on the locked 60 / 7500 Hz edges")
{
    const float fs = 48000.f;
    duallc::DualLcCell lp;
    lp.setKind (duallc::CellKind::Lowpass);
    lp.setCutoff (60.f);
    REQUIRE (cellGainAt (lp, 5.f, fs) == Catch::Approx (1.f).margin (0.08f));
    REQUIRE (cellGainAt (lp, 60.f, fs) > 0.65f);
    REQUIRE (cellGainAt (lp, 400.f, fs) < 0.25f);

    duallc::DualLcCell hp;
    hp.setKind (duallc::CellKind::Highpass);
    hp.setCutoff (7500.f);
    REQUIRE (cellGainAt (hp, 200.f, fs) < 0.15f);
    REQUIRE (cellGainAt (hp, 7500.f, fs) > 0.65f);
    REQUIRE (cellGainAt (hp, 16000.f, fs) == Catch::Approx (1.f).margin (0.12f));
}

TEST_CASE ("boost and cut mix around the tank")
{
    const float fs = 48000.f;
    duallc::FilterBank bank;
    bank.prepare (fs);
    const int band = 7; // 680 Hz
    const float fc = duallc::kCellGrid[static_cast<size_t> (band)].freqHz;

    bank.setCellGainDb (band, 12.f);
    bank.reset ();
    const float boost = bankGainAt (bank, 0, fc, fs);
    REQUIRE (duallc::linToDb (boost) == Catch::Approx (12.f).margin (1.5f));

    bank.setCellGainDb (band, -12.f);
    bank.reset ();
    const float cut = bankGainAt (bank, 0, fc, fs);
    REQUIRE (duallc::linToDb (cut) == Catch::Approx (-12.f).margin (1.5f));
}

TEST_CASE ("preamp and output amps are linear at modest levels")
{
    duallc::AnalogAmp amp;
    amp.prepare (48000.0);
    amp.setGainDb (0.f);
    amp.reset ();
    amp.smoothParams ();
    REQUIRE (amp.process (0.05f) == Catch::Approx (0.05f).margin (0.002f));

    amp.setGainDb (6.f);
    amp.reset ();
    amp.smoothParams ();
    REQUIRE (amp.process (0.05f) == Catch::Approx (0.05f * duallc::dbToLin (6.f)).margin (0.004f));
}

TEST_CASE ("hot preamp saturates instead of passing a brickwall peak")
{
    duallc::AnalogAmp amp;
    amp.prepare (48000.0);
    amp.setGainDb (24.f);
    amp.reset ();
    amp.smoothParams ();
    const float y = amp.process (1.f);
    REQUIRE (std::abs (y) < 8.f);
    REQUIRE (std::abs (y) < duallc::dbToLin (24.f) * 0.5f);
}

TEST_CASE ("coefficients rebuild at 44.1 / 48 / 88.2 / 96 kHz")
{
    for (float fs : duallc::kSupportedSampleRates)
    {
        duallc::DualLcCell cell;
        cell.setKind (duallc::CellKind::Bandpass);
        cell.setCutoff (960.f);
        cell.prepare (fs);
        float acc = 0.f;
        for (int i = 0; i < 64; ++i)
            acc += cell.process ((i == 0) ? 1.f : 0.f);
        REQUIRE (std::isfinite (acc));
        REQUIRE (std::abs (cell.b ()[0]) + std::abs (cell.a ()[1]) > 0.f);
    }
}

TEST_CASE ("nonlinear inductor hook is identity in v1")
{
    duallc::DualLcCell cell;
    cell.setKind (duallc::CellKind::Bandpass);
    cell.setCutoff (680.f);
    cell.prepare (48000.0);
    REQUIRE (cell.processNonlinear (0.37f) == Catch::Approx (0.37f));
}
