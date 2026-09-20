#include "processor.h"

#include "cids.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"

#include <algorithm>
#include <cmath>

#if defined(__SSE__)
#include <xmmintrin.h>
#endif

using namespace Steinberg;

namespace Steinberg {
namespace DualLc {

namespace {
constexpr int32 kStateMagic = 0x444C4346; // 'DLCF'
constexpr int32 kStateVersion = 1;
} // namespace

Processor::Processor ()
{
    setControllerClass (kControllerUID);
}

tresult PLUGIN_API Processor::initialize (FUnknown* context)
{
    tresult result = AudioEffect::initialize (context);
    if (result != kResultOk)
        return result;

    addAudioInput (STR16 ("Stereo In"), Vst::SpeakerArr::kStereo);
    addAudioOutput (STR16 ("Stereo Out"), Vst::SpeakerArr::kStereo);
    return kResultOk;
}

tresult PLUGIN_API Processor::terminate ()
{
    return AudioEffect::terminate ();
}

tresult PLUGIN_API Processor::setActive (TBool state)
{
    if (state)
        engine_.reset ();
    return AudioEffect::setActive (state);
}

tresult PLUGIN_API Processor::setProcessing (TBool state)
{
    if (state)
        engine_.reset ();
    return AudioEffect::setProcessing (state);
}

tresult PLUGIN_API Processor::setupProcessing (Vst::ProcessSetup& newSetup)
{
    sampleRate_ = newSetup.sampleRate;
    engine_.prepare (sampleRate_);
    return AudioEffect::setupProcessing (newSetup);
}

tresult PLUGIN_API Processor::canProcessSampleSize (int32 symbolicSampleSize)
{
    if (symbolicSampleSize == Vst::kSample32)
        return kResultTrue;
    return kResultFalse;
}

tresult PLUGIN_API Processor::setBusArrangements (Vst::SpeakerArrangement* inputs, int32 numIns,
                                                  Vst::SpeakerArrangement* outputs, int32 numOuts)
{
    if (numIns == 1 && numOuts == 1 && inputs && outputs
        && inputs[0] == Vst::SpeakerArr::kStereo && outputs[0] == Vst::SpeakerArr::kStereo)
    {
        return AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
    }
    return kResultFalse;
}

void Processor::applyNormalized (Vst::ParamID id, Vst::ParamValue value) noexcept
{
    const float n = static_cast<float> (value);
    if (id < static_cast<Vst::ParamID> (duallc::kNumCells))
        engine_.setCellGainDb (static_cast<int> (id), duallc::cellDbFromNorm (n));
    else if (id == kParamPreamp)
        engine_.setPreampDb (duallc::preampDbFromNorm (n));
    else if (id == kParamOutput)
        engine_.setOutputDb (duallc::outputDbFromNorm (n));
}

void Processor::enableDenormFlush (bool on) noexcept
{
#if defined(__SSE__)
    static thread_local unsigned savedCsr = 0;
    if (on)
    {
        savedCsr = _mm_getcsr ();
        _mm_setcsr (savedCsr | 0x8040); // FTZ | DAZ
    }
    else
    {
        _mm_setcsr (savedCsr);
    }
#else
    (void) on;
#endif
}

uint32 PLUGIN_API Processor::getTailSamples ()
{
    return static_cast<uint32> (0.25 * sampleRate_);
}

tresult PLUGIN_API Processor::process (Vst::ProcessData& data)
{
    struct Point
    {
        int32 sampleOffset = 0;
        Vst::ParamID id = 0;
        Vst::ParamValue value = 0;
    };

    constexpr int kMaxPoints = 1024;
    Point points[kMaxPoints];
    int nPoints = 0;

    if (data.inputParameterChanges)
    {
        const int32 numParamsChanged = data.inputParameterChanges->getParameterCount ();
        for (int32 i = 0; i < numParamsChanged; ++i)
        {
            if (auto* queue = data.inputParameterChanges->getParameterData (i))
            {
                const int32 numPts = queue->getPointCount ();
                const auto id = queue->getParameterId ();
                for (int32 p = 0; p < numPts && nPoints < kMaxPoints; ++p)
                {
                    Point pt;
                    pt.id = id;
                    if (queue->getPoint (p, pt.sampleOffset, pt.value) == kResultTrue)
                        points[nPoints++] = pt;
                }
            }
        }
        std::stable_sort (points, points + nPoints,
                          [] (const Point& a, const Point& b) { return a.sampleOffset < b.sampleOffset; });
    }

    auto applyAll = [this, &points, nPoints]() {
        for (int i = 0; i < nPoints; ++i)
            applyNormalized (points[i].id, points[i].value);
    };

    if (data.numInputs < 1 || data.numOutputs < 1 || data.numSamples <= 0)
    {
        applyAll ();
        return kResultOk;
    }

    if (data.symbolicSampleSize != Vst::kSample32)
        return kResultFalse;

    auto& in = data.inputs[0];
    auto& out = data.outputs[0];
    if (in.numChannels < 2 || out.numChannels < 2)
        return kResultFalse;

    float* inL = in.channelBuffers32[0];
    float* inR = in.channelBuffers32[1];
    float* outL = out.channelBuffers32[0];
    float* outR = out.channelBuffers32[1];
    if (!inL || !inR || !outL || !outR)
        return kResultFalse;

    enableDenormFlush (true);
    int ci = 0;
    for (int32 n = 0; n < data.numSamples; ++n)
    {
        while (ci < nPoints && points[ci].sampleOffset <= n)
        {
            applyNormalized (points[ci].id, points[ci].value);
            ++ci;
        }
        engine_.processStereoSample (inL[n], inR[n], outL[n], outR[n]);
    }
    while (ci < nPoints)
    {
        applyNormalized (points[ci].id, points[ci].value);
        ++ci;
    }
    enableDenormFlush (false);

    uint64 silence = 0;
    auto channelSilent = [n = data.numSamples] (const float* buf) {
        for (int32 i = 0; i < n; ++i)
        {
            if (std::abs (buf[i]) > 1.0e-12f)
                return false;
        }
        return true;
    };
    if (channelSilent (outL))
        silence |= 1ull;
    if (channelSilent (outR))
        silence |= 2ull;
    out.silenceFlags = silence;
    return kResultOk;
}

tresult PLUGIN_API Processor::setState (IBStream* state)
{
    if (!state)
        return kResultFalse;

    IBStreamer streamer (state, kLittleEndian);
    int32 magic = 0;
    int32 version = 0;
    if (!streamer.readInt32 (magic) || magic != kStateMagic)
        return kResultFalse;
    if (!streamer.readInt32 (version) || version != kStateVersion)
        return kResultFalse;

    for (int i = 0; i < duallc::kNumCells; ++i)
    {
        float n = 0.5f;
        if (!streamer.readFloat (n))
            return kResultFalse;
        applyNormalized (static_cast<Vst::ParamID> (i), n);
    }
    float pre = 0.f;
    float out = 0.5f;
    if (!streamer.readFloat (pre) || !streamer.readFloat (out))
        return kResultFalse;
    applyNormalized (kParamPreamp, pre);
    applyNormalized (kParamOutput, out);
    engine_.reset ();
    return kResultOk;
}

tresult PLUGIN_API Processor::getState (IBStream* state)
{
    if (!state)
        return kResultFalse;

    IBStreamer streamer (state, kLittleEndian);
    streamer.writeInt32 (kStateMagic);
    streamer.writeInt32 (kStateVersion);
    for (int i = 0; i < duallc::kNumCells; ++i)
        streamer.writeFloat (duallc::normFromCellDb (engine_.bank ().cellGainDb (i)));
    streamer.writeFloat (duallc::normFromPreampDb (engine_.preamp ().gainDb ()));
    streamer.writeFloat (duallc::normFromOutputDb (engine_.output ().gainDb ()));
    return kResultOk;
}

} // namespace DualLc
} // namespace Steinberg
