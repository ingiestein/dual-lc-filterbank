#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace duallc {

inline constexpr int kNumCells = 14;
inline constexpr int kNumChannels = 2;
inline constexpr float kCellQ = 0.8f;

inline constexpr float kCellGainMinDb = -12.f;
inline constexpr float kCellGainMaxDb = 12.f;
inline constexpr float kPreampMinDb = 0.f;
inline constexpr float kPreampMaxDb = 24.f;
inline constexpr float kOutputMinDb = -12.f;
inline constexpr float kOutputMaxDb = 12.f;

inline constexpr float kSmoothTimeSeconds = 0.015f;

enum class CellKind
{
    Lowpass,
    Bandpass,
    Highpass
};

struct CellSpec
{
    CellKind kind;
    float freqHz;
    const char* label;
    const char* title;
};

// Half-octave graphic from 60 Hz LP through 7500 Hz HP.
inline constexpr std::array<CellSpec, kNumCells> kCellGrid {{
    {CellKind::Lowpass,    60.f,   "60",    "60 Hz LP"},
    {CellKind::Bandpass,   85.f,   "85",    "85 Hz"},
    {CellKind::Bandpass,  120.f,   "120",   "120 Hz"},
    {CellKind::Bandpass,  170.f,   "170",   "170 Hz"},
    {CellKind::Bandpass,  240.f,   "240",   "240 Hz"},
    {CellKind::Bandpass,  340.f,   "340",   "340 Hz"},
    {CellKind::Bandpass,  480.f,   "480",   "480 Hz"},
    {CellKind::Bandpass,  680.f,   "680",   "680 Hz"},
    {CellKind::Bandpass,  960.f,   "960",   "960 Hz"},
    {CellKind::Bandpass, 1360.f,   "1.36k", "1360 Hz"},
    {CellKind::Bandpass, 1920.f,   "1.92k", "1920 Hz"},
    {CellKind::Bandpass, 2720.f,   "2.72k", "2720 Hz"},
    {CellKind::Bandpass, 3840.f,   "3.84k", "3840 Hz"},
    {CellKind::Highpass, 7500.f,   "7.5k",  "7500 Hz HP"},
}};

inline constexpr float kSupportedSampleRates[] = {44100.f, 48000.f, 88200.f, 96000.f};

enum class ParamId : int
{
    CellGain0 = 0,
    PreampGain = kNumCells,
    OutputGain = kNumCells + 1,
    NumParams = kNumCells + 2
};

inline float dbToLin (float db) noexcept
{
    return std::pow (10.f, db / 20.f);
}

inline float linToDb (float lin) noexcept
{
    const float absv = std::max (lin, 1.0e-12f);
    return 20.f * std::log10 (absv);
}

inline float mixFromGainDb (float db) noexcept
{
    return dbToLin (db) - 1.f;
}

inline float cellDbFromNorm (float n) noexcept
{
    return kCellGainMinDb + n * (kCellGainMaxDb - kCellGainMinDb);
}

inline float preampDbFromNorm (float n) noexcept
{
    return kPreampMinDb + n * (kPreampMaxDb - kPreampMinDb);
}

inline float outputDbFromNorm (float n) noexcept
{
    return kOutputMinDb + n * (kOutputMaxDb - kOutputMinDb);
}

inline float normFromCellDb (float db) noexcept
{
    return (db - kCellGainMinDb) / (kCellGainMaxDb - kCellGainMinDb);
}

inline float normFromPreampDb (float db) noexcept
{
    return (db - kPreampMinDb) / (kPreampMaxDb - kPreampMinDb);
}

inline float normFromOutputDb (float db) noexcept
{
    return (db - kOutputMinDb) / (kOutputMaxDb - kOutputMinDb);
}

} // namespace duallc
