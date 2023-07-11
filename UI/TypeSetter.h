//
// Created by kat on 7/7/23.
//

#include "Types.h"
#include "binaryninja-api/ui/action.h"
#include "libkbinja/MetadataSerializable.hpp"

#ifndef KSUITE_TYPESETTER_H
#define KSUITE_TYPESETTER_H

const std::string TypeSetterViewMetadataKey = "KSUITE-TypeSetterViewMetadata";

struct TypeSetterViewMetadata : public MetadataSerializable
{
    std::vector<std::string> classes;
    std::unordered_map<std::string, std::string> classQualNames;

    void Store() override
    {
        MSS(classes);
        MSS(classQualNames);
    }
    void Load() override
    {
        MSL(classes);
        MSL(classQualNames);
    }
};


class TypeSetter
{
public:
    static Ref<Type> ClassTypeForContext(UIActionContext ctx);
    static bool TypeWithExternalMethod(Ref<BinaryView> data, Ref<Function> func, Ref<Type> type);
    static bool SetThisArgType(Ref<BinaryView> data, Ref<Function> func, Ref<Type> type);

};


#endif //KSUITE_TYPESETTER_H
