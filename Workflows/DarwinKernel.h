//
// Created by vr1s on 12/17/22.
//

#ifndef KSUITE_DARWINKERNEL_H
#define KSUITE_DARWINKERNEL_H

#include <binaryninjaapi.h>

using namespace BinaryNinja;



enum Arm64Intrinsic : uint32_t
{
    ARM64_INTRIN_AUTDA,
    ARM64_INTRIN_AUTDB,
    ARM64_INTRIN_AUTIA,
    ARM64_INTRIN_AUTIB,
    ARM64_INTRIN_DC,
    ARM64_INTRIN_DMB,
    ARM64_INTRIN_DSB,
    ARM64_INTRIN_ESB,
    ARM64_INTRIN_HINT_BTI,
    ARM64_INTRIN_HINT_CSDB,
    ARM64_INTRIN_HINT_DGH,
    ARM64_INTRIN_HINT_TSB,
    ARM64_INTRIN_ISB,
    ARM64_INTRIN_MRS,
    ARM64_INTRIN_MSR,
    ARM64_INTRIN_PACDA,
    ARM64_INTRIN_PACDB,
    ARM64_INTRIN_PACGA,
    ARM64_INTRIN_PACIA,
    ARM64_INTRIN_PACIB,
    ARM64_INTRIN_PRFM,
    ARM64_INTRIN_PSBCSYNC,
    ARM64_INTRIN_SEV,
    ARM64_INTRIN_SEVL,
    ARM64_INTRIN_WFE,
    ARM64_INTRIN_WFI,
    ARM64_INTRIN_XPACD,
    ARM64_INTRIN_XPACI,
    ARM64_INTRIN_YIELD,
    ARM64_INTRIN_ERET,
    ARM64_INTRIN_CLZ,
    ARM64_INTRIN_CLREX,
    ARM64_INTRIN_REV,
    ARM64_INTRIN_RBIT,
    ARM64_INTRIN_AESD,
    ARM64_INTRIN_AESE,
    ARM64_INTRIN_NORMAL_END, /* needed so intrinsics can be extended by other lists, like neon
	                            intrinsics */
    ARM64_INTRIN_INVALID = 0xFFFFFFFF,
};


enum Arm64Regs {
    x16 = 50,
    x17 = 51
};

class DarwinKernelWorkflow {
public:
    static void RewritePacInstructions(Ref<AnalysisContext> ctx);
    static void FixBrokenSIMD(Ref<AnalysisContext> ctx);

    static void Register();
};


#endif //KSUITE_DARWINKERNEL_H
