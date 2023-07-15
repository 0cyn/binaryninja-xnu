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
#include "CPP/CPPTypeHelper.h"

#ifdef UI_BUILD
#include "binaryninja-api/ui/uitypes.h"
#include "UI/TypeSetter.h"
#include "binaryninja-api/ui/uicontext.h"

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
                auto helper = new CPPTypeHelper(ctx.binaryView);
                if (auto type = helper->GetClassTypeForFunction(ctx.function))
                {
                    helper->SetExternalMethodType(ctx.function, type);
                }
            }));
            context->globalActions()->bindAction(MAKE_ACTION_NAME(Types, Set this), UIAction([](const UIActionContext& ctx){
                auto helper = new CPPTypeHelper(ctx.binaryView);
                if (auto type = helper->GetClassTypeForFunction(ctx.function))
                {
                    helper->SetThisArgType(ctx.function, type);
                }
            }));
            context->globalActions()->bindAction(MAKE_ACTION_NAME(Types, Analyze), UIAction([](const UIActionContext& ctx){
                if (ctx.binaryView)
                {
                    auto helper = new CPPTypeHelper(ctx.binaryView);
                    auto classes = helper->FetchClasses();
                    for (auto& c : classes)
                    {
                        /*
                        BNLogInfo("Class: %s", c.name.c_str());
                        for (auto& s : c.superclasses)
                            BNLogInfo("  Superclass: %s", s.c_str());
                        for (auto& it : c.vtable)
                        {
                            std::string name;
                            if (auto sym = ctx.binaryView->GetSymbolByAddress(it.second))
                                name = sym->GetShortName();
                            if (name.empty())
                                if (auto sym = ctx.binaryView->GetSymbolByAddress(c.vtableStart + 0x10 + it.first))
                                    name = sym->GetShortName();
                            BNLogInfo("  0x%llx = %s", it.first, name.c_str());
                        }
                         */
                        helper->CreateTypeForClass(c);
                    }
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