#include "Locator.hpp"

Red::Rtti::Locator::Locator(RED4ext::CName aName) noexcept
    : m_name(aName)
    , m_type(nullptr)
{
}

void Red::Rtti::Locator::Resolve()
{
    m_type = RED4ext::CRTTISystem::Get()->GetType(m_name);
}

Red::Rtti::Locator::operator const RED4ext::CBaseRTTIType*()
{
    if (!m_type)
    {
        Resolve();
    }

    return m_type;
}

Red::Rtti::Locator::operator const RED4ext::CClass*()
{
    if (!m_type)
    {
        Resolve();
    }

    if (m_type->GetType() != RED4ext::ERTTIType::Class)
    {
        return nullptr;
    }

    return reinterpret_cast<RED4ext::CClass*>(m_type);
}

Red::Rtti::Locator::operator bool()
{
    return m_type;
}
