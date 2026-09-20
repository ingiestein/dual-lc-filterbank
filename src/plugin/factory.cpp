#include "processor.h"
#include "controller.h"
#include "cids.h"
#include "version.h"

#include "public.sdk/source/main/pluginfactory.h"

#define stringPluginName "Dual-LC Filterbank"

using namespace Steinberg::Vst;
using namespace Steinberg;

BEGIN_FACTORY_DEF (stringCompanyName, stringCompanyWeb, stringCompanyEmail)

DEF_CLASS2 (INLINE_UID_FROM_FUID (DualLc::kProcessorUID),
            PClassInfo::kManyInstances,
            kVstAudioEffectClass,
            stringPluginName,
            Vst::kDistributable,
            DualLcVST3Category,
            FULL_VERSION_STR,
            kVstVersionString,
            DualLc::Processor::createInstance)

DEF_CLASS2 (INLINE_UID_FROM_FUID (DualLc::kControllerUID),
            PClassInfo::kManyInstances,
            kVstComponentControllerClass,
            stringPluginName "Controller",
            0,
            "",
            FULL_VERSION_STR,
            kVstVersionString,
            DualLc::Controller::createInstance)

END_FACTORY
