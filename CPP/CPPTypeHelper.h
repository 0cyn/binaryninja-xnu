//
// Created by serket on 7/14/23.
//

#ifndef KSUITE_CPPTYPEHELPER_H
#define KSUITE_CPPTYPEHELPER_H

#include "libkbinja/MetadataSerializable.hpp"

const std::string TypeSetterViewMetadataKey = "KSUITE-TSVM";

struct TypeSetterViewMetadata : public MetadataSerializable
{
    std::vector<std::string> classes;
    std::unordered_map<std::string, std::string> classQualNames;
    std::unordered_map<std::string, std::string> classVtableQualNames;
    std::unordered_map<std::string, std::unordered_map<uint64_t, uint64_t>> vtables;

    void Store() override
    {
        MSS(classes);
        MSS(classQualNames);
        MSS(classVtableQualNames);
        MSS(vtables);
    }
    void Load() override
    {
        MSL(classes);
        MSL(classQualNames);
        MSS(classVtableQualNames);
        MSL(vtables);
    }
};

class CPPTypeHelper {
    Ref<BinaryView> m_data;
    TypeSetterViewMetadata* m_typeMetadata;

public:

    struct CPPClass {
        std::string name;
        uint64_t vtableStart;
        std::vector<std::string> superclasses;

        std::vector<uint64_t> constructors;
        std::vector<uint64_t> destructors;

        std::unordered_map<uint64_t, uint64_t> vtable;
    };

    CPPTypeHelper(Ref<BinaryView> data);
    std::vector<CPPClass> FetchClasses();
protected:
    std::vector<CPPClass> FetchClassesUsingVTableDestructorStrategy(std::set<std::string>& names, std::unordered_map<std::string, Ref<Symbol>>& vtables);

public:

    Ref<Type> CreateTypeForClass(const CPPClass& cppClass);

    Ref<Type> GetClassTypeForFunction(Ref<Function> func);
    bool SetThisArgType(Ref<Function> func, Ref<Type> type);
    bool SetExternalMethodType(Ref<Function> func, Ref<Type> type);
};


#endif //KSUITE_CPPTYPEHELPER_H
