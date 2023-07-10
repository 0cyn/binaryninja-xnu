// -===-=
// Bootstrap for binaryninja-xnu when installed as a standalone plugin
// -===-=
#ifdef KSUITE
#error "This file should not be built as a part of parent projects! Please fix your build scripts!"
#endif

#ifndef NDEBUG
#warning "Debug Build"
#endif

#include <binaryninjaapi.h>

// Core
#include "Workflows/DarwinKernel.h"

#ifdef UI_BUILD
#include "uitypes.h"
#include "UI/TypeSetter.h"
#include "uicontext.h"

#endif

#ifdef UI_BUILD

#define ACTION_PREFIX "XNU Tools\\"

#define MAKE_ACTION_NAME(component, action) QString::fromStdString(std::string(ACTION_PREFIX) + #component + "\\" + #action)

namespace BinaryNinjaXNU {

    class Notifications : public UIContextNotification {
        inline static Notifications* m_instance;

    public:
        virtual void OnContextOpen(UIContext* context) override
        {
            context->globalActions()->bindAction(MAKE_ACTION_NAME(Types, External Method), UIAction([](const UIActionContext& ctx){
                auto type = TypeSetter::ClassTypeForContext(ctx);
                if (type)
                {
                    TypeSetter::TypeWithExternalMethod(ctx.binaryView, ctx.function, type);
                }
            }));
            context->globalActions()->bindAction(MAKE_ACTION_NAME(Types, Set this), UIAction([](const UIActionContext& ctx){
                auto type = TypeSetter::ClassTypeForContext(ctx);
                if (type)
                {
                    TypeSetter::SetThisArgType(ctx.binaryView, ctx.function, type);
                }
            }));
        }
        static void init()
        {
            m_instance = new Notifications;
            UIContext::registerNotification(m_instance);
        }
    };
}


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

    BinaryNinjaXNU::Notifications::init();

    return true;
}
#endif

}