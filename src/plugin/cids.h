#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"
#include "grid.h"

namespace Steinberg {
namespace DualLc {

static const FUID kProcessorUID (0xB48F7880, 0x26A44C3D, 0xAC806EF2, 0x80802D49);
static const FUID kControllerUID (0x2DAA44CA, 0xF3884D81, 0xB4454590, 0x32061199);

inline constexpr Vst::ParamID kParamCell0 = 0;
inline constexpr Vst::ParamID kParamPreamp = static_cast<Vst::ParamID> (duallc::kNumCells);
inline constexpr Vst::ParamID kParamOutput = static_cast<Vst::ParamID> (duallc::kNumCells + 1);

#define DualLcVST3Category "Fx|EQ|Filter"

} // namespace DualLc
} // namespace Steinberg
