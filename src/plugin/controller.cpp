#include "controller.h"

#include "cids.h"
#include "grid.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "vstgui/plugin-bindings/vst3editor.h"

using namespace Steinberg;

namespace Steinberg {
namespace DualLc {

namespace {
constexpr int32 kStateMagic = 0x444C4346;
constexpr int32 kStateVersion = 1;
} // namespace

tresult PLUGIN_API Controller::initialize (FUnknown* context)
{
    tresult result = EditControllerEx1::initialize (context);
    if (result != kResultOk)
        return result;

    for (int i = 0; i < duallc::kNumCells; ++i)
    {
        const auto& spec = duallc::kCellGrid[static_cast<size_t> (i)];
        auto* param = new Vst::RangeParameter (
            USTRING (spec.title), static_cast<Vst::ParamID> (i), USTRING ("dB"),
            duallc::kCellGainMinDb, duallc::kCellGainMaxDb, 0.0, 0,
            Vst::ParameterInfo::kCanAutomate, 0, USTRING (spec.label));
        param->setPrecision (1);
        parameters.addParameter (param);
    }

    auto* pre = new Vst::RangeParameter (
        USTRING ("Preamp"), kParamPreamp, USTRING ("dB"), duallc::kPreampMinDb,
        duallc::kPreampMaxDb, 0.0, 0, Vst::ParameterInfo::kCanAutomate, 0, USTRING ("Pre"));
    pre->setPrecision (1);
    parameters.addParameter (pre);

    auto* out = new Vst::RangeParameter (
        USTRING ("Output"), kParamOutput, USTRING ("dB"), duallc::kOutputMinDb,
        duallc::kOutputMaxDb, 0.0, 0, Vst::ParameterInfo::kCanAutomate, 0, USTRING ("Out"));
    out->setPrecision (1);
    parameters.addParameter (out);

    return kResultOk;
}

tresult PLUGIN_API Controller::terminate ()
{
    return EditControllerEx1::terminate ();
}

tresult PLUGIN_API Controller::setComponentState (IBStream* state)
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
        setParamNormalized (static_cast<Vst::ParamID> (i), n);
    }
    float pre = 0.f;
    float out = 0.5f;
    if (!streamer.readFloat (pre) || !streamer.readFloat (out))
        return kResultFalse;
    setParamNormalized (kParamPreamp, pre);
    setParamNormalized (kParamOutput, out);
    return kResultOk;
}

tresult PLUGIN_API Controller::setState (IBStream* /*state*/)
{
    return kResultTrue;
}

tresult PLUGIN_API Controller::getState (IBStream* /*state*/)
{
    return kResultTrue;
}

IPlugView* PLUGIN_API Controller::createView (FIDString name)
{
    if (FIDStringsEqual (name, Vst::ViewType::kEditor))
    {
        auto* view = new VSTGUI::VST3Editor (this, "view", "editor.uidesc");
        return view;
    }
    return nullptr;
}

} // namespace DualLc
} // namespace Steinberg
