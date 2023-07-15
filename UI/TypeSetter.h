//
// Created by kat on 7/7/23.
//

#include "XNU/CPP/Types.h"
#include "binaryninja-api/ui/action.h"
#include "libkbinja/MetadataSerializable.hpp"

#ifndef KSUITE_TYPESETTER_H
#define KSUITE_TYPESETTER_H


class TypeSetter
{
public:
    static Ref<Type> ClassTypeForContext(UIActionContext ctx);
    static bool TypeWithExternalMethod(Ref<BinaryView> data, Ref<Function> func, Ref<Type> type);
    static bool SetThisArgType(Ref<BinaryView> data, Ref<Function> func, Ref<Type> type);

};


#endif //KSUITE_TYPESETTER_H
