//
// Created by kat on 7/9/23.
//

#include <binaryninjaapi.h>

// Core
#include "Workflows/DarwinKernel.h"

#ifdef UI_BUILD
#include "uitypes.h"
#include "UI/TypeSetter.h"
#endif


extern "C" {

BN_DECLARE_CORE_ABI_VERSION

#ifdef UI_BUILD
BN_DECLARE_UI_ABI_VERSION
#endif

BINARYNINJAPLUGIN bool CorePluginInit() {

    DarwinKernelWorkflow::Register();

    return true;
}

#ifdef UI_BUILD
BINARYNINJAPLUGIN bool UIPluginInit() {

    return true;
}
#endif

}