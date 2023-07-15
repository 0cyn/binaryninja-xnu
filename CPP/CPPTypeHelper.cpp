//
// Created by serket on 7/14/23.
//

#include "CPPTypeHelper.h"
#include "Types.h"

std::vector<std::string> splitt(const std::string& s, char seperator)
{
    std::vector<std::string> output;

    std::string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );

        output.push_back(substring);

        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

    return output;
}

CPPTypeHelper::CPPTypeHelper(Ref<BinaryView> data)
    : m_data(data)
{

}

std::vector<CPPTypeHelper::CPPClass> CPPTypeHelper::FetchClassesUsingVTableDestructorStrategy(
        std::set<std::string>& names, std::unordered_map<std::string, Ref<Symbol>>& vtables)
{
    std::vector<CPPTypeHelper::CPPClass> classes;
    std::unordered_map<uint64_t, std::string> destructorToClassMap;

    BinaryReader rdr(m_data);

    for (auto& name : names)
    {
        if (const auto& it = vtables.find(name); it != vtables.end())
        {
            CPPTypeHelper::CPPClass cppClass;
            cppClass.name = name;
            cppClass.vtableStart = it->second->GetAddress();

            for (auto& symbol : m_data->GetSymbols())
            {
                if (symbol->GetShortName().find(name + "::" + name) != std::string::npos)
                {
                    cppClass.constructors.push_back(symbol->GetAddress());
                }
                if (symbol->GetShortName().find(name + "::~" + name) != std::string::npos)
                {
                    cppClass.destructors.push_back(symbol->GetAddress());
                }
            }

            classes.push_back(cppClass);
        }
        for (auto& symbol : m_data->GetSymbols())
        {
            if (symbol->GetShortName().find(name + "::~" + name) != std::string::npos)
            {
                destructorToClassMap[symbol->GetAddress()] = name;
            }
        }
    }

    for (auto& cppClass : classes)
    {
        for (const auto& dAddr : cppClass.destructors)
        {
            auto func = m_data->GetRecentAnalysisFunctionForAddress(dAddr);
            if (!func)
                continue;
            for (const auto& callee : m_data->GetCallees({func, func->GetArchitecture(), func->GetStart()}))
            {
                if (const auto& it = destructorToClassMap.find(callee); it != destructorToClassMap.end())
                {
                    if (it->second != cppClass.name)
                        cppClass.superclasses.push_back(it->second);
                }
            }
        }

        if (cppClass.vtableStart)
        {
            uint64_t off = 0;
            bool done = false;
            rdr.Seek(cppClass.vtableStart + 0x10);
            //BNLogInfo("cn: %s", cppClass.name.c_str());
            while (!done)
            {
                if (uint64_t addr = rdr.Read64()) {
                    // BNLogInfo("  - Addr based 0x%llx", addr);
                    cppClass.vtable[off] = addr;
                }
                else
                {
                    if (m_data->GetSymbolByAddress(rdr.GetOffset()-8) && m_data->GetSymbolByAddress(rdr.GetOffset()-8)->GetType() == ImportedDataSymbol)
                    {
                        cppClass.vtable[off] = UINT64_MAX;
                        //BNLogInfo("  - ImportDataSymbol based 0x%llx", rdr.GetOffset()-8);
                    }
                    else
                        done = true;
                }
                off += 8;
            }
        }
    }

    return classes;
}

std::vector<CPPTypeHelper::CPPClass> CPPTypeHelper::FetchClasses()
{
    std::set<std::string> classNames;
    std::unordered_map<std::string, Ref<Symbol>> vtables;

    for (auto& symbol : m_data->GetSymbols())
    {
        if (symbol->GetShortName().find("::") != std::string::npos)
        {
            auto name = splitt(symbol->GetShortName(), ':')[0];
            if (!name.empty())
                classNames.insert(name);
        }
    }

    for (auto& symbol : m_data->GetSymbols())
    {
        if (symbol->GetShortName().find("_vtable_for_") != std::string::npos)
        {
            // _vtable_for_
            auto name = symbol->GetShortName().erase(0, strlen("_vtable_for_"));
            vtables[name] = symbol;
        }
    }
    if (!vtables.empty())
        return FetchClassesUsingVTableDestructorStrategy(classNames, vtables);

    return {};
}


Ref<Type> CPPTypeHelper::CreateTypeForClass(const CPPClass& cppClass)
{

    auto storedData = new TypeSetterViewMetadata;
    if (auto metadata = m_data->QueryMetadata(TypeSetterViewMetadataKey))
        storedData->LoadFromMetadata(metadata);

    Ref<Type> classType;


#if (BN_CURRENT_CORE_ABI_VERSION >= 36)
    auto undoID = m_data->BeginUndoActions();
#else
    m_data->BeginUndoActions();
#endif

    Ref<Type> vtableType = Type::VoidType();

    if (!cppClass.vtable.empty())
    {
        auto vtableBuilder = StructureBuilder();
        for (auto it : cppClass.vtable)
        {
            size_t emptyI = 0;
            if (it.second)
            {
                std::string name = "pure_virtual_" + std::to_string(emptyI);
                Ref<Type> type = Type::VoidType();
                auto targetSym = m_data->GetSymbolByAddress(it.second);
                if (!targetSym)
                {
                    targetSym = m_data->GetSymbolByAddress(cppClass.vtableStart + 0x10 + it.first);
                }
                auto targetFunc = m_data->GetRecentAnalysisFunctionForAddress(it.second);
                if (targetSym)
                {
                    name = targetSym->GetShortName();
                    auto targetClassName = splitt(name, ':')[0];
                    name.erase(0, std::strlen((targetClassName + "::").c_str()));
                }
                if (targetFunc)
                {
                    type = targetFunc->GetType();
                }
                vtableBuilder.AddMemberAtOffset(Type::PointerType(8, type), name, it.first);
            }
        }

        auto vtableStruct = vtableBuilder.Finalize();
        QualifiedName struName = QualifiedName("_vtable_for_" + cppClass.name);
        Ref<Type> struType = Type::StructureType(vtableStruct);
        auto assignedName = m_data->DefineType(Type::GenerateAutoTypeId("ksuite", "_vtable_for_" + cppClass.name), struName, struType);
        vtableType = m_data->GetTypeByName(assignedName);
        storedData->classVtableQualNames[cppClass.name] = assignedName.GetString();
    }

    if (auto it = std::find(storedData->classes.begin(), storedData->classes.end(), cppClass.name); it != storedData->classes.end())
    {
        if (auto it2 = storedData->classQualNames.find(*it); it2 != storedData->classQualNames.end())
        {
            classType = m_data->GetTypeByName(it2->second);
        }
    }

    // i feel like this has to be a bit more complex than it should...
    if (!classType)
    {
        auto cTBuilder = StructureBuilder();
        cTBuilder.AddMember(Type::PointerType(8, vtableType), "vptr");
        auto stru = cTBuilder.Finalize();
        QualifiedName struName = QualifiedName(cppClass.name);
        Ref<Type> struType = Type::StructureType(stru);
        auto assignedName = m_data->DefineType(Type::GenerateAutoTypeId("ksuite", cppClass.name), struName, struType);
        classType = m_data->GetTypeByName(assignedName);
        storedData->classes.push_back(cppClass.name);
        storedData->classQualNames[cppClass.name] = assignedName.GetString();
    }

    if (!vtableType->IsVoid())
    {
        for (auto it : cppClass.vtable)
        {
            size_t emptyI = 0;
            if (it.second)
            {
                std::string name = std::to_string(emptyI);
                Ref<Type> type = Type::VoidType();
                auto targetSym = m_data->GetSymbolByAddress(it.second);
                if (!targetSym)
                {
                    targetSym = m_data->GetSymbolByAddress(cppClass.vtableStart + 0x10 + it.first);
                }
                auto targetFunc = m_data->GetRecentAnalysisFunctionForAddress(it.second);
                if (targetSym)
                {
                    name = targetSym->GetShortName();
                    auto targetClassName = splitt(name, ':')[0];
                    name.erase(0, std::strlen((targetClassName + "::").c_str()));
                }
                if (targetFunc)
                {
                    SetThisArgType(targetFunc, classType);
                }
            }
        }
    }

#if (BN_CURRENT_CORE_ABI_VERSION >= 36)
    m_data->CommitUndoActions(undoID);
#else
    m_data->CommitUndoActions();
#endif
    m_data->StoreMetadata(TypeSetterViewMetadataKey, storedData->AsMetadata());

    return classType;
}

Ref<Type> CPPTypeHelper::GetClassTypeForFunction(Ref<Function> func)
{
    auto storedData = new TypeSetterViewMetadata;
    if (auto metadata = m_data->QueryMetadata(TypeSetterViewMetadataKey))
        storedData->LoadFromMetadata(metadata);

    Ref<Symbol> sym = func->GetSymbol();
    std::string className;
    if (sym)
    {
        auto parts = splitt(sym->GetShortName(), ':');
        if (parts.size() > 1 && parts[0].size() > 1)
            className = parts[0];
    }
    if (className.empty())
    {
        if (!BinaryNinja::GetTextLineInput(className, "Class Name", "Class Name"))
            return nullptr;
    }

#if (BN_CURRENT_CORE_ABI_VERSION >= 36)
    auto undoID = m_data->BeginUndoActions();
#else
    m_data->BeginUndoActions();
#endif

    Ref<Type> classType = nullptr;

    if (auto it = std::find(storedData->classes.begin(), storedData->classes.end(), className); it != storedData->classes.end())
    {
        if (auto it2 = storedData->classQualNames.find(*it); it2 != storedData->classQualNames.end())
        {
            classType = m_data->GetTypeByName(it2->second);
        }
    }

    // i feel like this has to be a bit more complex than it should...
    if (!classType)
    {
        auto cTBuilder = StructureBuilder();
        cTBuilder.AddMember(Type::PointerType(8, Type::VoidType()), "vptr");
        auto stru = cTBuilder.Finalize();
        QualifiedName struName = QualifiedName(className);
        Ref<Type> struType = Type::StructureType(stru);
        auto assignedName = m_data->DefineType(Type::GenerateAutoTypeId("ksuite", className), struName, struType);
        classType = m_data->GetTypeByName(assignedName);
        storedData->classes.push_back(className);
        storedData->classQualNames[className] = assignedName.GetString();
    }
#if (BN_CURRENT_CORE_ABI_VERSION >= 36)
    m_data->CommitUndoActions(undoID);
#else
    m_data->CommitUndoActions();
#endif
    m_data->StoreMetadata(TypeSetterViewMetadataKey, storedData->AsMetadata());

    return classType;
}


bool CPPTypeHelper::SetThisArgType(Ref<Function> func, Ref<Type> type)
{
    if (!func || !type)
        return false;
#if (BN_CURRENT_CORE_ABI_VERSION >= 36)
    auto undoID = m_data->BeginUndoActions();
#else
    m_data->BeginUndoActions();
#endif

    auto params = func->GetParameterVariables();
    if (params->empty())
    {
        func->GetReturnType()->GetTypeName();
        func->GetCallingConvention()->GetName();
        auto funcTypeBuilder = TypeBuilder::FunctionType(func->GetReturnType(), func->GetCallingConvention(),
                                                         {FunctionParameter("this", Type::PointerType(8, type))});

        auto funcType = funcTypeBuilder.Finalize();

        if (funcType)
        {
            func->SetUserType(funcType);
        }
        func->Reanalyze();

#if (BN_CURRENT_CORE_ABI_VERSION >= 36)
        m_data->CommitUndoActions(undoID);
#else
        m_data->CommitUndoActions();
#endif
        return true;
    }

    func->CreateUserVariable(params->at(0), Type::PointerType(8, type), "this");

    func->Reanalyze();
#if (BN_CURRENT_CORE_ABI_VERSION >= 36)
    m_data->CommitUndoActions(undoID);
#else
    m_data->CommitUndoActions();
#endif

    return true;
}


bool CPPTypeHelper::SetExternalMethodType(Ref<Function> func, Ref<Type> type)
{
    if (!func || !type)
        return false;
#if (BN_CURRENT_CORE_ABI_VERSION >= 36)
    auto undoID = m_data->BeginUndoActions();
#else
    m_data->BeginUndoActions();
#endif

    Ref<Type> ioReturnType = m_data->GetTypeByName(QualifiedName("IOReturn"));
    if (!ioReturnType)
    {
        auto krtName = m_data->DefineType(Type::GenerateAutoTypeId("ksuite", QualifiedName("kern_return_t")), QualifiedName("kern_return_t"), Type::IntegerType(4, false));
        auto iorName = m_data->DefineType(Type::GenerateAutoTypeId("ksuite", QualifiedName("IOReturn")), QualifiedName("IOReturn"), m_data->GetTypeByName(krtName));
        ioReturnType = m_data->GetTypeByName(iorName);
    }

    Ref<Type> argsType = m_data->GetTypeByName(QualifiedName("IOExternalMethodArguments"));
    if (!argsType)
    {
        std::string errors;
        TypeParserResult tpResult;
        auto ok = m_data->ParseTypesFromSource(extArgs, {}, {}, tpResult, errors);
        if (ok && !tpResult.types.empty())
        {
            m_data->DefineType(Type::GenerateAutoTypeId("ksuite", QualifiedName("IOExternalMethodArguments")), QualifiedName("IOExternalMethodArguments"), tpResult.types[0].type);
        }
        argsType = m_data->GetTypeByName(QualifiedName("IOExternalMethodArguments"));
    }

    auto funcTypeBuilder = TypeBuilder::FunctionType(ioReturnType, func->GetCallingConvention(),
                                                     {FunctionParameter("target", Type::PointerType(8, type)),
                                                      FunctionParameter("reference", Type::PointerType(8, Type::VoidType())),
                                                      FunctionParameter("args", Type::PointerType(8, argsType))});

    auto funcType = funcTypeBuilder.Finalize();

    if (funcType)
    {
        func->SetUserType(funcType);
    }

    func->Reanalyze();

#if (BN_CURRENT_CORE_ABI_VERSION >= 36)
    m_data->CommitUndoActions(undoID);
#else
    m_data->CommitUndoActions();
#endif

    return true;
}
