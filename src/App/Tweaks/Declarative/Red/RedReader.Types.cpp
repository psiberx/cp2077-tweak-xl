#include "RedReader.hpp"

Red::CName App::RedReader::GetFlatTypeName(const Core::SharedPtr<Red::TweakFlat>& aFlat)
{
    if (aFlat->isArray)
    {
        switch (aFlat->type)
        {
            case Red::ETweakFlatType::Int: return Red::ERTDBFlatType::IntArray;
            case Red::ETweakFlatType::Float: return Red::ERTDBFlatType::FloatArray;
            case Red::ETweakFlatType::Bool: return Red::ERTDBFlatType::BoolArray;
            case Red::ETweakFlatType::String: return Red::ERTDBFlatType::StringArray;
            case Red::ETweakFlatType::CName: return Red::ERTDBFlatType::CNameArray;
            case Red::ETweakFlatType::ResRef: return Red::ERTDBFlatType::ResRefArray;
            case Red::ETweakFlatType::LocKey: return Red::ERTDBFlatType::LocKeyArray;
            case Red::ETweakFlatType::ForeignKey: return Red::ERTDBFlatType::TweakDBIDArray;
            case Red::ETweakFlatType::Quaternion: return Red::ERTDBFlatType::QuaternionArray;
            case Red::ETweakFlatType::EulerAngles: return Red::ERTDBFlatType::EulerAnglesArray;
            case Red::ETweakFlatType::Vector3: return Red::ERTDBFlatType::Vector3Array;
            case Red::ETweakFlatType::Vector2: return Red::ERTDBFlatType::Vector2Array;
            case Red::ETweakFlatType::Color: return Red::ERTDBFlatType::ColorArray;
            case Red::ETweakFlatType::Undefined: break;
        }
    }
    else
    {
        switch (aFlat->type)
        {
            case Red::ETweakFlatType::Int: return Red::ERTDBFlatType::Int;
            case Red::ETweakFlatType::Float: return Red::ERTDBFlatType::Float;
            case Red::ETweakFlatType::Bool: return Red::ERTDBFlatType::Bool;
            case Red::ETweakFlatType::String: return Red::ERTDBFlatType::String;
            case Red::ETweakFlatType::CName: return Red::ERTDBFlatType::CName;
            case Red::ETweakFlatType::ResRef: return Red::ERTDBFlatType::ResRef;
            case Red::ETweakFlatType::LocKey: return Red::ERTDBFlatType::LocKey;
            case Red::ETweakFlatType::ForeignKey: return Red::ERTDBFlatType::TweakDBID;
            case Red::ETweakFlatType::Quaternion: return Red::ERTDBFlatType::Quaternion;
            case Red::ETweakFlatType::EulerAngles: return Red::ERTDBFlatType::EulerAngles;
            case Red::ETweakFlatType::Vector3: return Red::ERTDBFlatType::Vector3;
            case Red::ETweakFlatType::Vector2: return Red::ERTDBFlatType::Vector2;
            case Red::ETweakFlatType::Color: return Red::ERTDBFlatType::Color;
            case Red::ETweakFlatType::Undefined: break;
        }
    }

    return {};
}
