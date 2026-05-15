#include "ScriptableRecordManager.hpp"

#include "App/Tweaks/TweakService.hpp"
#include "App/Tweaks/TweakTypeSpec.hpp"
#include "ScriptablePropertyManager.hpp"
#include "ScriptableRecordClass.hpp"
#include "ScriptableTweakDBRecord.hpp"

namespace App
{
ScriptablePropertySpecPtr ScriptableRecordSpec::FindPropertyByFunctionName(const std::string& aFunctionName) const
{
    for (const auto& prop : props | std::views::values)
    {
        if (prop->functionName == aFunctionName)
            return prop;
    }

    return nullptr;
}

ScriptablePropertySpecPtr ScriptableRecordSpec::FindPropertyByName(const std::string& aName) const
{
    const auto cn = Red::CName(aName.c_str());

    if (!props.contains(cn))
        return nullptr;
    return props.at(cn);
}

ScriptableRecordManager::ScriptableRecordManager(const Core::DeferredPtr<Red::TweakDBManager>& aManager,
                                                 const Core::SharedPtr<ScriptablePropertyManager>& aPropertyHandler)
    : m_rtti(Red::CRTTISystem::Get())
    , m_tweakManager(aManager)
    , m_propertyHandler(aPropertyHandler)
{
}

std::vector<Red::CName> ScriptableRecordManager::GetRecordSpecNames() const
{
    std::vector<Red::CName> vector;

    std::shared_lock lock(m_specsMutex);
    vector.reserve(m_specs.size());

    for (const auto& spec : m_specs | std::views::values)
        vector.emplace_back(spec->cname);

    return vector;
}

Core::Vector<ScriptableRecordSpecPtr> ScriptableRecordManager::GetRecordSpecs() const
{
    Core::Vector<ScriptableRecordSpecPtr> vector;

    std::shared_lock lock(m_specsMutex);
    vector.reserve(m_specs.size());

    for (const auto& spec : m_specs | std::views::values)
        vector.emplace_back(spec);

    return vector;
}

ScriptableRecordSpecPtr ScriptableRecordManager::GetRecordSpec(Red::CName aName) const
{
    std::shared_lock lockR(m_specsMutex);

    if (const auto it = m_specs.find(aName); it != m_specs.end())
        return it->second;

    return nullptr;
}

ScriptablePropertySpecPtr ScriptableRecordManager::GetPropertySpec(Red::CName aRecordName,
                                                                   Red::CName aPropertyName) const
{
    if (const auto recordSpec = GetRecordSpec(aRecordName))
    {
        std::shared_lock lockR(m_specsMutex);
        if (const auto it = recordSpec->props.find(aPropertyName); it != recordSpec->props.end())
            return it->second;
    }

    return nullptr;
}

bool ScriptableRecordManager::CreateScriptableRecord(Red::TweakDB* aTweakDB, const uint32_t aHash,
                                                     const Red::TweakDBID aRecordId) const
{
    if (const auto cls = GetRecordClass(aHash))
        return CreateScriptableRecord(aTweakDB, cls, aRecordId);

    return false;
}

bool ScriptableRecordManager::CreateScriptableRecord(Red::TweakDB* aTweakDB, ScriptableRecordClass* aClass,
                                                     const Red::TweakDBID aRecordId)
{
    if (!aClass || !aTweakDB)
    {
        LogError("Failed to create scriptable record because the provided class or TweakDB instance is null.");
        return false;
    }

    if (const auto instance = Red::MakeScriptedHandle<ScriptableTweakDBRecord>(aClass))
    {
        instance->recordID = aRecordId;
        instance->nativeType = aClass;

        Raw::InsertRecord(aTweakDB, aRecordId, aClass, instance);

        return true;
    }

    LogError(R"(Failed to create an instance of scriptable record type "{}".)", aClass->GetName().ToString());
    return false;
}

ScriptableRecordSpecPtr ScriptableRecordManager::RegisterScriptableRecordType(
    const std::string& aName, const std::optional<std::string>& aParentName)
{
    const auto cname = Red::CName(aName.c_str());
    const auto aliasName = Red::GetRecordAliasName<std::string>(aName);

    if (GetRecordSpec(cname))
    {
        LogError(R"(Registration of record type "{}" failed because another with the same name is already registered.)",
                 aliasName);
        return nullptr;
    }

    if (m_rtti->GetClass(cname))
    {
        LogError(R"(Registration of record type "{}" failed because a class with the same name already exists.)",
                 aliasName);
        return nullptr;
    }

    const auto spec = Core::MakeShared<ScriptableRecordSpec>();

    spec->name = aName;
    spec->aliasName = Red::GetRecordAliasName<std::string>(aName);
    spec->shortName = Red::GetRecordShortName<std::string>(aName);

    spec->cname = Red::CNamePool::Add(spec->name.c_str());
    spec->aliasCName = Red::CNamePool::Add(spec->aliasName.c_str());
    spec->shortCName = Red::CName(spec->shortName.c_str());

    spec->hash = Red::GetRecordTypeHash(spec->shortName);
    spec->parent = aParentName;

    {
        std::unique_lock lockRW(m_specsMutex);
        m_specs[spec->cname] = spec;
    }

    return spec;
}

void ScriptableRecordManager::RegisterScriptableProperty(const ScriptableRecordSpecPtr& aRecordSpec,
                                                         const std::string& aPropertyName,
                                                         const TweakTypeSpecPtr& aTypeSpec,
                                                         const Red::InstancePtr<>& aDefaultValue) const
{
    if (aRecordSpec->props.contains(aPropertyName.c_str()))
    {
        LogError(R"(Registration of property "{}" for record type "{}" failed because another with the same name is )"
                 "already registered.",
                 aPropertyName, aRecordSpec->aliasName);
        return;
    }

    const auto propertyInfo = Core::MakeShared<ScriptablePropertySpec>();
    propertyInfo->name = aPropertyName;
    propertyInfo->functionName = Red::Capitalize(aPropertyName);
    propertyInfo->appendix = "." + aPropertyName;
    propertyInfo->typeSpec = aTypeSpec;
    propertyInfo->cname = Red::CName{aPropertyName.c_str()};
    propertyInfo->defaultValue = aDefaultValue;

    aRecordSpec->props[propertyInfo->cname] = propertyInfo;
}

void ScriptableRecordManager::UnregisterScriptableRecordType(Red::CName aRecordName) const
{
    const auto spec = GetRecordSpec(aRecordName);

    if (!spec || !spec->isDescribed || spec->isDeleted)
        return;

    spec->isDeleted = true;

    for (const auto& prop : spec->props | std::views::values)
        UnregisterScriptableProperty(spec, prop);
}

void ScriptableRecordManager::RegisterRTTITypes()
{
    for (auto& spec : m_specs | std::views::values)
        RegisterRTTIType(spec);

    for (auto& spec : m_specs | std::views::values)
        (void)DescribeRTTIType(spec);
}

bool ScriptableRecordManager::RegisterRTTIType(const ScriptableRecordSpecPtr& aSpec)
{
    if (aSpec->isRegistered)
        return true;

    LogDebug(R"(Creating RTTI type "{}" for scriptable record "{}"...)", aSpec->name, aSpec->aliasName);

    if (auto* cls = CreateRecordClass(aSpec))
    {
        aSpec->type = cls;
        aSpec->isRegistered = true;
        return true;
    }

    return false;
}

bool ScriptableRecordManager::DescribeRTTIType(const ScriptableRecordSpecPtr& aSpec) const
{
    if (aSpec->isDescribed)
        return true;

    if (!aSpec->isRegistered || !aSpec->type)
    {
        LogDebug("Record type \"{}\" is not registered, skipping description.", aSpec->aliasName);
        return false;
    }

    if (aSpec->parent.has_value())
    {
        auto* parentCls = m_rtti->GetClass(Red::GetRecordFullName<Red::CName>(aSpec->parent.value()));

        if (!parentCls || !Red::IsRecordType(parentCls))
        {
            LogError("Failed to describe record type \"{}\" because the specified parent type \"{}\" does not exist or "
                     "is not a valid record type.",
                     aSpec->aliasName, aSpec->parent.value());
            return false;
        }

        aSpec->type->parent = parentCls;
    }
    else
    {
        aSpec->type->parent = ScriptableTweakDBRecord::TYPE::GetClass();
    }

    Red::CNamePool::Add(Red::GetHandleTypeName<std::string>(aSpec->type).c_str());
    Red::CNamePool::Add(Red::GetWHandleTypeName<std::string>(aSpec->type).c_str());
    Red::CNamePool::Add(Red::GetWHandleArrayTypeName<std::string>(aSpec->type).c_str());

    aSpec->isDescribed = true;

    return true;
}

void ScriptableRecordManager::CreatePropertyFunctions(const ScriptableRecordSpecPtr& aRecordSpec,
                                                      const ScriptablePropertySpecPtr& aPropSpec) const
{
    if (!aRecordSpec->isDescribed)
    {
        LogDebug(R"(Record type "{}" is not described, skipping function creation.)", aRecordSpec->aliasName);
        return;
    }

    if (aPropSpec->isCreated)
        return;

    if (const auto& typeSpec = aPropSpec->typeSpec; typeSpec->isForeignKey)
    {
        if (!typeSpec->foreignType)
        {
            if (const auto* type = m_rtti->GetClass(typeSpec->foreignTypeName))
            {
                typeSpec->foreignType = type;
            }
            else
            {
                LogError(
                    R"(Failed to describe property "{}" of record type "{}", the foreign type "{}" does not exist.)",
                    aPropSpec->name, aRecordSpec->aliasName, typeSpec->foreignName);
                return;
            }
        }

        Red::CNamePool::Add(Red::GetHandleTypeName<std::string>(typeSpec->foreignType).c_str());
        Red::CNamePool::Add(Red::GetWHandleTypeName<std::string>(typeSpec->foreignType).c_str());
        Red::CNamePool::Add(Red::GetWHandleArrayTypeName<std::string>(typeSpec->foreignType).c_str());
    }

    m_propertyHandler->CreateFunctions(aRecordSpec, aPropSpec);

    aPropSpec->isCreated = true;
}

void ScriptableRecordManager::InsertDefaultValues()
{
    for (const auto& spec : m_specs | std::views::values)
        InsertDefaultValues(spec);
}

void ScriptableRecordManager::AdaptScriptClasses(const Red::DynArray<Red::ScriptClass*>& aClasses) const
{
    for (const auto& classDef : aClasses)
        AdaptScriptClass(classDef);
}

bool ScriptableRecordManager::IsTweakDBReady() const
{
    return m_tweakDBReady;
}

void ScriptableRecordManager::SetTweakDBReady(const bool aTweakDBReady)
{
    m_tweakDBReady = aTweakDBReady;
}

#ifndef NDEBUG

bool ScriptableRecordManager::SetupTestRecord(const ScriptableRecordSpecPtr& aSpec)
{
    if (aSpec->isRegistered)
        return false;

    if (!RegisterRTTIType(aSpec))
        return false;

    if (!DescribeRTTIType(aSpec))
        return false;

    for (auto& propSpec : aSpec->props | std::views::values)
        CreatePropertyFunctions(aSpec, propSpec);

    InsertDefaultValues(aSpec);

    return true;
}

#endif

void ScriptableRecordManager::UnregisterScriptableProperty(const ScriptableRecordSpecPtr& aRecordSpec,
                                                           const std::string& aPropName) const
{
    if (const auto spec = aRecordSpec->FindPropertyByName(aPropName))
        UnregisterScriptableProperty(aRecordSpec, spec);
}

void ScriptableRecordManager::UpdateScriptableProperty(const ScriptableRecordSpecPtr& aRecordSpec,
                                                       const std::string& aPropName,
                                                       const Red::InstancePtr<>& aDefaultValue) const
{
    if (const auto spec = aRecordSpec->FindPropertyByName(aPropName))
    {
        LogDebug(R"(Updating default value for property "{}" of record type "{}"...)", spec->name,
                 aRecordSpec->aliasName);

        spec->isInserted = false;
        spec->defaultValue = aDefaultValue;
    }
}

void ScriptableRecordManager::UnregisterScriptableProperty(const ScriptableRecordSpecPtr& aRecordSpec,
                                                           const ScriptablePropertySpecPtr& aPropSpec) const
{
    aPropSpec->isDeleted = true;
    m_propertyHandler->DeleteFunctions(aRecordSpec, aPropSpec);
}

void ScriptableRecordManager::InsertDefaultValues(const ScriptableRecordSpecPtr& aSpec)
{
    if (!aSpec->isDescribed)
        return;

    const auto recordID = Red::GetRTDBRecordID(aSpec->shortName);

    for (const auto& prop : aSpec->props | std::views::values)
    {
        if (!prop->isCreated || prop->isInserted)
            continue;

        const auto flatID = recordID + std::string_view(".") + prop->name;
        auto instance = prop->defaultValue;

        if (!instance)
            instance = Red::ConstructFlatInstance(prop->typeSpec->flatType);

        const auto value = Red::MakeValue(prop->typeSpec->flatType, instance.get());

        if (!value || !m_tweakManager->SetFlat(flatID, *value))
            LogError(R"(Failed to insert default value for property "{}" of record type "{}" into TweakDB.)",
                     prop->name, aSpec->aliasName);

        prop->isInserted = true;
    }

    if (aSpec->type->parent)
        InsertDefaultValues(aSpec->type->parent);
}

void ScriptableRecordManager::InsertDefaultValues(const Red::CClass* aClass)
{
    if (!aClass)
        return;

    if (const auto spec = GetRecordSpec(aClass->GetName()))
        InsertDefaultValues(spec);
}

ScriptableRecordClass* ScriptableRecordManager::GetRecordClass(const uint32_t aHash) const
{
    std::shared_lock lockR(m_classesMutex);

    if (const auto it = m_classes.find(aHash); it != m_classes.end())
        return it->second;

    return nullptr;
}

ScriptableRecordClass* ScriptableRecordManager::CreateRecordClass(const ScriptableRecordSpecPtr& aSpec)
{
    if (GetRecordClass(aSpec->hash))
    {
        LogError(R"(Failed to create RTTI class for record type "{}" because its specification was not found.)",
                 aSpec->name);
        return nullptr;
    }

    if (m_rtti->GetClass(aSpec->cname))
    {
        LogError(
            R"(Failed to create RTTI class for record type "{}" because a class with the same name already exists.)",
            aSpec->name);
        return nullptr;
    }

    const auto cls = Red::Memory::New<ScriptableRecordClass>(aSpec->cname, aSpec->hash);

    m_rtti->RegisterType(cls);
    m_rtti->RegisterScriptName(aSpec->cname, aSpec->aliasCName);

    {
        std::unique_lock lockRW(m_classesMutex);
        m_classes[cls->tweakBaseHash] = cls;
    }

    return cls;
}

void ScriptableRecordManager::AdaptScriptClass(const Red::ScriptClass* aClassDef) const
{
    static const auto* ScriptableRecordType = ScriptableTweakDBRecord::TYPE::GetClass();

    if (!aClassDef->rttiClass)
        return;

    auto* cls = aClassDef->rttiClass;

    if (!cls->flags.isNative || !cls->parent || !cls->parent->IsA(ScriptableRecordType))
        return;

    const auto recordSpec = GetRecordSpec(cls->GetName());

    if (!recordSpec)
        return;

    for (const auto& func : cls->funcs)
        m_propertyHandler->AdaptFunction(recordSpec, func);
}

} // namespace App
