#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"

namespace Steinberg {
namespace DualLc {

class Controller : public Vst::EditControllerEx1
{
public:
    Controller () = default;
    ~Controller () SMTG_OVERRIDE = default;

    static FUnknown* createInstance (void* /*context*/)
    {
        return static_cast<Vst::IEditController*> (new Controller);
    }

    tresult PLUGIN_API initialize (FUnknown* context) SMTG_OVERRIDE;
    tresult PLUGIN_API terminate () SMTG_OVERRIDE;
    tresult PLUGIN_API setComponentState (IBStream* state) SMTG_OVERRIDE;
    IPlugView* PLUGIN_API createView (FIDString name) SMTG_OVERRIDE;
    tresult PLUGIN_API setState (IBStream* state) SMTG_OVERRIDE;
    tresult PLUGIN_API getState (IBStream* state) SMTG_OVERRIDE;

    DEFINE_INTERFACES
    END_DEFINE_INTERFACES (EditController)
    DELEGATE_REFCOUNT (EditController)
};

} // namespace DualLc
} // namespace Steinberg
