#pragma once

#include "engine.h"

#include "public.sdk/source/vst/vstaudioeffect.h"

namespace Steinberg {
namespace DualLc {

class Processor : public Vst::AudioEffect
{
public:
    Processor ();
    ~Processor () SMTG_OVERRIDE = default;

    static FUnknown* createInstance (void* /*context*/)
    {
        return static_cast<Vst::IAudioProcessor*> (new Processor);
    }

    tresult PLUGIN_API initialize (FUnknown* context) SMTG_OVERRIDE;
    tresult PLUGIN_API terminate () SMTG_OVERRIDE;
    tresult PLUGIN_API setActive (TBool state) SMTG_OVERRIDE;
    tresult PLUGIN_API setProcessing (TBool state) SMTG_OVERRIDE;
    tresult PLUGIN_API setupProcessing (Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;
    tresult PLUGIN_API canProcessSampleSize (int32 symbolicSampleSize) SMTG_OVERRIDE;
    tresult PLUGIN_API setBusArrangements (Vst::SpeakerArrangement* inputs, int32 numIns,
                                           Vst::SpeakerArrangement* outputs, int32 numOuts) SMTG_OVERRIDE;
    tresult PLUGIN_API process (Vst::ProcessData& data) SMTG_OVERRIDE;
    tresult PLUGIN_API setState (IBStream* state) SMTG_OVERRIDE;
    tresult PLUGIN_API getState (IBStream* state) SMTG_OVERRIDE;
    uint32 PLUGIN_API getTailSamples () SMTG_OVERRIDE;

private:
    void applyNormalized (Vst::ParamID id, Vst::ParamValue value) noexcept;
    void enableDenormFlush (bool on) noexcept;

    duallc::Engine engine_;
    double sampleRate_ = 44100.0;
};

} // namespace DualLc
} // namespace Steinberg
