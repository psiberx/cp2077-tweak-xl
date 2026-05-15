#ifndef NDEBUG

#include "ScriptableRecordTests.hpp"

#include "Core/Facades/Container.hpp"
#include "RED4ext/Scripting/Natives/Generated/game/data/Vehicle_Record.hpp"
#include "ScriptableTweakDBRecord.hpp"

#include <catch2/catch_all.hpp>

namespace
{
using RecordType = Red::TypeLocator<"gamedataTweakXLTest_Record">;
const auto RecordName = Red::NormalizeRecordName("TweakXLTest");
constexpr auto RecordID = Red::TweakDBID("test.tweakxl.scriptable");

const auto IntID = Red::TweakDBID(RecordID, ".Int32Prop");
const auto FloatID = Red::TweakDBID(RecordID, ".FloatProp");
const auto BoolID = Red::TweakDBID(RecordID, ".BoolProp");
const auto StringID = Red::TweakDBID(RecordID, ".StringProp");
const auto CNameID = Red::TweakDBID(RecordID, ".CNameProp");
const auto LocKeyID = Red::TweakDBID(RecordID, ".LocKeyProp");
const auto ResRefID = Red::TweakDBID(RecordID, ".ResRefProp");
const auto QuaternionID = Red::TweakDBID(RecordID, ".QuaternionProp");
const auto EulerAnglesID = Red::TweakDBID(RecordID, ".EulerAnglesProp");
const auto Vector3ID = Red::TweakDBID(RecordID, ".Vector3Prop");
const auto Vector2ID = Red::TweakDBID(RecordID, ".Vector2Prop");
const auto ColorID = Red::TweakDBID(RecordID, ".ColorProp");
const auto IntArrayID = Red::TweakDBID(RecordID, ".Int32ArrayProp");
const auto FloatArrayID = Red::TweakDBID(RecordID, ".FloatArrayProp");
const auto BoolArrayID = Red::TweakDBID(RecordID, ".BoolArrayProp");
const auto StringArrayID = Red::TweakDBID(RecordID, ".StringArrayProp");
const auto CNameArrayID = Red::TweakDBID(RecordID, ".CNameArrayProp");
const auto LocKeyArrayID = Red::TweakDBID(RecordID, ".LocKeyArrayProp");
const auto ResRefArrayID = Red::TweakDBID(RecordID, ".ResRefArrayProp");
const auto QuaternionArrayID = Red::TweakDBID(RecordID, ".QuaternionArrayProp");
const auto EulerAnglesArrayID = Red::TweakDBID(RecordID, ".EulerAnglesArrayProp");
const auto Vector3ArrayID = Red::TweakDBID(RecordID, ".Vector3ArrayProp");
const auto Vector2ArrayID = Red::TweakDBID(RecordID, ".Vector2ArrayProp");
const auto ColorArrayID = Red::TweakDBID(RecordID, ".ColorArrayProp");
const auto TweakDBIDID = Red::TweakDBID(RecordID, ".TweakDBIDProp");
const auto TweakDBIDArrayID = Red::TweakDBID(RecordID, ".TweakDBIDArrayProp");

using IntType = Red::TypeLocator<Red::ERTDBFlatType::Int>;
using FloatType = Red::TypeLocator<Red::ERTDBFlatType::Float>;
using BoolType = Red::TypeLocator<Red::ERTDBFlatType::Bool>;
using StringType = Red::TypeLocator<Red::ERTDBFlatType::String>;
using CNameType = Red::TypeLocator<Red::ERTDBFlatType::CName>;
using LocKeyType = Red::TypeLocator<Red::ERTDBFlatType::LocKey>;
using ResRefType = Red::TypeLocator<Red::ERTDBFlatType::ResRef>;
using QuaternionType = Red::TypeLocator<Red::ERTDBFlatType::Quaternion>;
using EulerAnglesType = Red::TypeLocator<Red::ERTDBFlatType::EulerAngles>;
using Vector3Type = Red::TypeLocator<Red::ERTDBFlatType::Vector3>;
using Vector2Type = Red::TypeLocator<Red::ERTDBFlatType::Vector2>;
using ColorType = Red::TypeLocator<Red::ERTDBFlatType::Color>;
using IntArrayType = Red::TypeLocator<Red::ERTDBFlatType::IntArray>;
using FloatArrayType = Red::TypeLocator<Red::ERTDBFlatType::FloatArray>;
using BoolArrayType = Red::TypeLocator<Red::ERTDBFlatType::BoolArray>;
using StringArrayType = Red::TypeLocator<Red::ERTDBFlatType::StringArray>;
using CNameArrayType = Red::TypeLocator<Red::ERTDBFlatType::CNameArray>;
using LocKeyArrayType = Red::TypeLocator<Red::ERTDBFlatType::LocKeyArray>;
using ResRefArrayType = Red::TypeLocator<Red::ERTDBFlatType::ResRefArray>;
using QuaternionArrayType = Red::TypeLocator<Red::ERTDBFlatType::QuaternionArray>;
using EulerAnglesArrayType = Red::TypeLocator<Red::ERTDBFlatType::EulerAnglesArray>;
using Vector3ArrayType = Red::TypeLocator<Red::ERTDBFlatType::Vector3Array>;
using Vector2ArrayType = Red::TypeLocator<Red::ERTDBFlatType::Vector2Array>;
using ColorArrayType = Red::TypeLocator<Red::ERTDBFlatType::ColorArray>;
using TweakDBIDType = Red::TypeLocator<Red::ERTDBFlatType::TweakDBID>;
using TweakDBIDArrayType = Red::TypeLocator<Red::ERTDBFlatType::TweakDBIDArray>;
} // namespace

namespace App
{
Core::SharedPtr<TweakService> Tests::ScriptableRecordTestRunner::GetTweakService()
{
    return Core::Resolve<TweakService>();
}

Core::DeferredPtr<Red::TweakDBManager> Tests::ScriptableRecordTestRunner::GetTweakManager()
{
    return GetTweakService()->GetManager();
}

Core::SharedPtr<ScriptableRecordManager> Tests::ScriptableRecordTestRunner::GetRecordManager()
{
    return GetTweakService()->GetRecordManager();
}

Core::SharedPtr<ScriptablePropertyManager> Tests::ScriptableRecordTestRunner::GetPropertyHandler()
{
    return GetTweakService()->GetPropertyHandler();
}

int Tests::ScriptableRecordTestRunner::Run(const std::filesystem::path& aDir)
{
    if (!s_isSetup)
    {
        Setup();
        s_isSetup = true;
    }

    const char* argv[] = {"TweakXL.dll"};
    auto session = Catch::Session();
    session.configData().defaultOutputFilename = (aDir / "test_results.txt").string();
    return session.run(1, argv);
}

void Tests::ScriptableRecordTestRunner::Setup()
{
    using namespace Red::ERTDBFlatType;

    const auto recordManager = GetRecordManager();
    const auto tweakManager = GetTweakManager();

    const auto recordSpec = recordManager->RegisterScriptableRecordType(RecordName);

    recordManager->RegisterScriptableProperty(recordSpec, "Int32Prop", GetTweakTypeSpec<Int>());
    recordManager->RegisterScriptableProperty(recordSpec, "FloatProp", GetTweakTypeSpec<Float>());
    recordManager->RegisterScriptableProperty(recordSpec, "BoolProp", GetTweakTypeSpec<Bool>());
    recordManager->RegisterScriptableProperty(recordSpec, "StringProp", GetTweakTypeSpec<String>());
    recordManager->RegisterScriptableProperty(recordSpec, "CNameProp", GetTweakTypeSpec<CName>());
    recordManager->RegisterScriptableProperty(recordSpec, "LocKeyProp", GetTweakTypeSpec<LocKey>());
    recordManager->RegisterScriptableProperty(recordSpec, "ResRefProp", GetTweakTypeSpec<ResRef>());
    recordManager->RegisterScriptableProperty(recordSpec, "QuaternionProp", GetTweakTypeSpec<Quaternion>());
    recordManager->RegisterScriptableProperty(recordSpec, "EulerAnglesProp", GetTweakTypeSpec<EulerAngles>());
    recordManager->RegisterScriptableProperty(recordSpec, "Vector3Prop", GetTweakTypeSpec<Vector3>());
    recordManager->RegisterScriptableProperty(recordSpec, "Vector2Prop", GetTweakTypeSpec<Vector2>());
    recordManager->RegisterScriptableProperty(recordSpec, "ColorProp", GetTweakTypeSpec<Color>());
    recordManager->RegisterScriptableProperty(recordSpec, "Int32ArrayProp", GetTweakTypeSpec<IntArray>());
    recordManager->RegisterScriptableProperty(recordSpec, "FloatArrayProp", GetTweakTypeSpec<FloatArray>());
    recordManager->RegisterScriptableProperty(recordSpec, "BoolArrayProp", GetTweakTypeSpec<BoolArray>());
    recordManager->RegisterScriptableProperty(recordSpec, "StringArrayProp", GetTweakTypeSpec<StringArray>());
    recordManager->RegisterScriptableProperty(recordSpec, "CNameArrayProp", GetTweakTypeSpec<CNameArray>());
    recordManager->RegisterScriptableProperty(recordSpec, "LocKeyArrayProp", GetTweakTypeSpec<LocKeyArray>());
    recordManager->RegisterScriptableProperty(recordSpec, "ResRefArrayProp", GetTweakTypeSpec<ResRefArray>());
    recordManager->RegisterScriptableProperty(recordSpec, "QuaternionArrayProp", GetTweakTypeSpec<QuaternionArray>());
    recordManager->RegisterScriptableProperty(recordSpec, "EulerAnglesArrayProp", GetTweakTypeSpec<EulerAnglesArray>());
    recordManager->RegisterScriptableProperty(recordSpec, "Vector3ArrayProp", GetTweakTypeSpec<Vector3Array>());
    recordManager->RegisterScriptableProperty(recordSpec, "Vector2ArrayProp", GetTweakTypeSpec<Vector2Array>());
    recordManager->RegisterScriptableProperty(recordSpec, "ColorArrayProp", GetTweakTypeSpec<ColorArray>());

    recordManager->RegisterScriptableProperty(recordSpec, "TweakDBIDProp", GetTweakTypeSpec<TweakDBID>("Vehicle"));
    recordManager->RegisterScriptableProperty(recordSpec, "TweakDBIDArrayProp",
                                              GetTweakTypeSpec<TweakDBIDArray>("Vehicle"));

    recordManager->SetupTestRecord(recordSpec);

    auto intVal = 42;
    auto floatVal = 3.14f;
    auto boolVal = true;
    auto stringVal = Red::CString("test");
    auto cnameVal = Red::CName("test");
    auto locKeyVal = Red::LocKeyWrapper("test");
    auto resRefVal = Red::RaRef("base\\gameplay\\vehicles\\visual_customization\\decals\\rayfield_caliburn\\vcc_"
                                "rayfield_caliburn_bumper_f_numbers_01_yellow.ent");
    auto quatVal = Red::Quaternion(1.0, 2.0, 3.0, 4.0);
    auto eulerVal = Red::EulerAngles(1.0, 2.0, 3.0);
    auto vec3Val = Red::Vector3(1.0, 2.0, 3.0);
    auto vec2Val = Red::Vector2(1.0, 2.0);
    auto colorVal = Red::Color(1, 2, 3, 4);
    auto intArrayVal = Red::DynArray({1, 2, 3});
    auto floatArrayVal = Red::DynArray({1.0f, 2.0f, 3.0f});
    auto boolArrayVal = Red::DynArray({true, false, true});
    auto stringArrayVal = Red::DynArray({Red::CString("one"), Red::CString("two"), Red::CString("three")});
    auto cnameArrayVal = Red::DynArray({Red::CName("one"), Red::CName("two"), Red::CName("three")});
    auto locKeyArrayVal =
        Red::DynArray({Red::LocKeyWrapper("one"), Red::LocKeyWrapper("two"), Red::LocKeyWrapper("three")});
    auto resRefArrayVal = Red::DynArray({Red::RaRef("base\\gameplay\\vehicles\\visual_customization\\decals\\rayfield_"
                                                    "caliburn\\vcc_rayfield_caliburn_bumper_f_numbers_01_yellow.ent")});
    auto quatArrayVal = Red::DynArray({Red::Quaternion(1.0, 2.0, 3.0, 4.0), Red::Quaternion(5.0, 6.0, 7.0, 8.0)});
    auto eulerArrayVal = Red::DynArray({Red::EulerAngles(1.0, 2.0, 3.0), Red::EulerAngles(4.0, 5.0, 6.0)});
    auto vec3ArrayVal = Red::DynArray({Red::Vector3(1.0, 2.0, 3.0), Red::Vector3(4.0, 5.0, 6.0)});
    auto vec2ArrayVal = Red::DynArray({Red::Vector2(1.0, 2.0), Red::Vector2(3.0, 4.0)});
    auto colorArrayVal = Red::DynArray({Red::Color(1, 2, 3, 4), Red::Color(4, 5, 6, 7)});
    auto tweakDBIDVal = Red::TweakDBID("Vehicle.v_sport1_herrera_outlaw");
    auto tweakDBIDArrayVal = Red::DynArray({Red::TweakDBID("Vehicle.v_sport1_herrera_outlaw"),
                                            Red::TweakDBID("Vehicle.v_sport1_quadra_sport_r7"),
                                            Red::TweakDBID("Vehicle.v_sport1_rayfield_caliburn")});

    tweakManager->SetFlat(IntID, IntType::Get(), &intVal);
    tweakManager->SetFlat(FloatID, FloatType::Get(), &floatVal);
    tweakManager->SetFlat(BoolID, BoolType::Get(), &boolVal);
    tweakManager->SetFlat(StringID, StringType::Get(), &stringVal);
    tweakManager->SetFlat(CNameID, CNameType::Get(), &cnameVal);
    tweakManager->SetFlat(LocKeyID, LocKeyType::Get(), &locKeyVal);
    tweakManager->SetFlat(ResRefID, ResRefType::Get(), &resRefVal);
    tweakManager->SetFlat(QuaternionID, QuaternionType::Get(), &quatVal);
    tweakManager->SetFlat(EulerAnglesID, EulerAnglesType::Get(), &eulerVal);
    tweakManager->SetFlat(Vector3ID, Vector3Type::Get(), &vec3Val);
    tweakManager->SetFlat(Vector2ID, Vector2Type::Get(), &vec2Val);
    tweakManager->SetFlat(ColorID, ColorType::Get(), &colorVal);
    tweakManager->SetFlat(IntArrayID, IntArrayType::Get(), &intArrayVal);
    tweakManager->SetFlat(FloatArrayID, FloatArrayType::Get(), &floatArrayVal);
    tweakManager->SetFlat(BoolArrayID, BoolArrayType::Get(), &boolArrayVal);
    tweakManager->SetFlat(StringArrayID, StringArrayType::Get(), &stringArrayVal);
    tweakManager->SetFlat(CNameArrayID, CNameArrayType::Get(), &cnameArrayVal);
    tweakManager->SetFlat(LocKeyArrayID, LocKeyArrayType::Get(), &locKeyArrayVal);
    tweakManager->SetFlat(ResRefArrayID, ResRefArrayType::Get(), &resRefArrayVal);
    tweakManager->SetFlat(QuaternionArrayID, QuaternionArrayType::Get(), &quatArrayVal);
    tweakManager->SetFlat(EulerAnglesArrayID, EulerAnglesArrayType::Get(), &eulerArrayVal);
    tweakManager->SetFlat(Vector3ArrayID, Vector3ArrayType::Get(), &vec3ArrayVal);
    tweakManager->SetFlat(Vector2ArrayID, Vector2ArrayType::Get(), &vec2ArrayVal);
    tweakManager->SetFlat(ColorArrayID, ColorArrayType::Get(), &colorArrayVal);
    tweakManager->SetFlat(TweakDBIDID, TweakDBIDType::Get(), &tweakDBIDVal);
    tweakManager->SetFlat(TweakDBIDArrayID, TweakDBIDArrayType::Get(), &tweakDBIDArrayVal);

    tweakManager->CreateRecord(RecordID, RecordType::GetClass());
}

TEST_CASE("When an existing scriptable record is queried, TweakDB returns it")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    REQUIRE(record != nullptr);
    REQUIRE(record->recordID == RecordID);
}

TEST_CASE("When a property is defined as Int32, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("Int32Prop");
    REQUIRE(func != nullptr);

    int result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == 42);
}

TEST_CASE("When a property is defined as Float, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("FloatProp");
    REQUIRE(func != nullptr);

    float result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == 3.14f);
}

TEST_CASE("When a property is defined as Bool, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("BoolProp");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == true);
}

TEST_CASE("When a property is defined as String, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("StringProp");
    REQUIRE(func != nullptr);

    Red::CString result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == "test");
}

TEST_CASE("When a property is defined as CName, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("CNameProp");
    REQUIRE(func != nullptr);

    Red::CName result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == "test");
}

TEST_CASE("When a property is defined as LocKey, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("LocKeyProp");
    REQUIRE(func != nullptr);

    Red::CName result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == Red::CName("test"));
}

TEST_CASE("When a property is defined as ResRef, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("ResRefProp");
    REQUIRE(func != nullptr);

    Red::ResRef result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.resource.path ==
            "base\\gameplay\\vehicles\\visual_customization\\decals\\rayfield_caliburn\\vcc_rayfield_caliburn_"
            "bumper_f_numbers_01_yellow.ent");
}

TEST_CASE("When a property is defined as Quaternion, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("QuaternionProp");
    REQUIRE(func != nullptr);

    Red::Quaternion result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.i == 1.0);
    REQUIRE(result.j == 2.0);
    REQUIRE(result.k == 3.0);
    REQUIRE(result.r == 4.0);
}

TEST_CASE("When a property is defined as EulerAngles, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("EulerAnglesProp");
    REQUIRE(func != nullptr);

    Red::EulerAngles result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.Roll == 1.0);
    REQUIRE(result.Pitch == 2.0);
    REQUIRE(result.Yaw == 3.0);
}

TEST_CASE("When a property is defined as Vector3, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("Vector3Prop");
    REQUIRE(func != nullptr);

    Red::Vector3 result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.X == 1.0);
    REQUIRE(result.Y == 2.0);
    REQUIRE(result.Z == 3.0);
}

TEST_CASE("When a property is defined as Vector2, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("Vector2Prop");
    REQUIRE(func != nullptr);

    Red::Vector2 result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.X == 1.0);
    REQUIRE(result.Y == 2.0);
}

TEST_CASE("When a property is defined as Color, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("ColorProp");
    REQUIRE(func != nullptr);

    Red::Color result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.Red == 1);
    REQUIRE(result.Green == 2);
    REQUIRE(result.Blue == 3);
    REQUIRE(result.Alpha == 4);
}

TEST_CASE("When a property is defined as Int32Array, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("Int32ArrayProp");
    REQUIRE(func != nullptr);

    Red::DynArray<int> result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.size() == 3);
    REQUIRE(result.At(0) == 1);
    REQUIRE(result.At(1) == 2);
    REQUIRE(result.At(2) == 3);
}

TEST_CASE("When a property is defined as Int32Array, its count function exists and returns the correct size")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetInt32ArrayPropCount");
    REQUIRE(func != nullptr);

    int result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == 3);
}

TEST_CASE("When a property is defined as Int32Array, its item getter function exists and returns values when the index "
          "is not out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetInt32ArrayPropItem");
    REQUIRE(func != nullptr);

    {
        int result;
        REQUIRE(Red::CallFunction(record, func, result, 0) == true);
        REQUIRE(result == 1);
    }
    {
        int result;
        REQUIRE(Red::CallFunction(record, func, result, 1) == true);
        REQUIRE(result == 2);
    }
    {
        int result;
        REQUIRE(Red::CallFunction(record, func, result, 2) == true);
        REQUIRE(result == 3);
    }
}

TEST_CASE("When a property is defined as Int32Array, its item getter function does not modify the output if the index "
          "is out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetInt32ArrayPropItem");
    REQUIRE(func != nullptr);

    {
        int result = -1234;
        REQUIRE(Red::CallFunction(record, func, result, -1) == true);
        REQUIRE(result == -1234);
    }
    {
        int result = -6789;
        REQUIRE(Red::CallFunction(record, func, result, 3) == true);
        REQUIRE(result == -6789);
    }
}

TEST_CASE(
    "When a property is defined as Int32Array, its contains function returns true if the item is present in the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("Int32ArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, 2) == true);
    REQUIRE(result == true);
}

TEST_CASE("When a property is defined as Int32Array, its contains function returns false if the item is not present in "
          "the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("Int32ArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, 5) == true);
    REQUIRE(result == false);
}

TEST_CASE("When a property is defined as FloatArray, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("FloatArrayProp");
    REQUIRE(func != nullptr);

    Red::DynArray<float> result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.size() == 3);
    REQUIRE(result.At(0) == 1.0f);
    REQUIRE(result.At(1) == 2.0f);
    REQUIRE(result.At(2) == 3.0f);
}

TEST_CASE("When a property is defined as FloatArray, its count function exists and returns the correct size")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetFloatArrayPropCount");
    REQUIRE(func != nullptr);

    int result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == 3);
}

TEST_CASE("When a property is defined as FloatArray, its item getter function exists and returns values when the index "
          "is not out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetFloatArrayPropItem");
    REQUIRE(func != nullptr);

    {
        float result;
        REQUIRE(Red::CallFunction(record, func, result, 0) == true);
        REQUIRE(result == 1.0f);
    }
    {
        float result;
        REQUIRE(Red::CallFunction(record, func, result, 1) == true);
        REQUIRE(result == 2.0f);
    }
    {
        float result;
        REQUIRE(Red::CallFunction(record, func, result, 2) == true);
        REQUIRE(result == 3.0f);
    }
}

TEST_CASE("When a property is defined as FloatArray, its item getter function does not modify the output if the index "
          "is out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetFloatArrayPropItem");
    REQUIRE(func != nullptr);

    {
        float result = -1234.0f;
        REQUIRE(Red::CallFunction(record, func, result, -1) == true);
        REQUIRE(result == -1234.0f);
    }
    {
        float result = -6789.0f;
        REQUIRE(Red::CallFunction(record, func, result, 3) == true);
        REQUIRE(result == -6789.0f);
    }
}

TEST_CASE(
    "When a property is defined as FloatArray, its contains function returns true if the item is present in the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("FloatArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, 2.0f) == true);
    REQUIRE(result == true);
}

TEST_CASE("When a property is defined as FloatArray, its contains function returns false if the item is not present in "
          "the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("FloatArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, 5.0f) == true);
    REQUIRE(result == false);
}

TEST_CASE("When a property is defined as BoolArray, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("BoolArrayProp");
    REQUIRE(func != nullptr);

    Red::DynArray<bool> result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.size() == 3);
    REQUIRE(result.At(0) == true);
    REQUIRE(result.At(1) == false);
    REQUIRE(result.At(2) == true);
}

TEST_CASE("When a property is defined as BoolArray, its count function exists and returns the correct size")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetBoolArrayPropCount");
    REQUIRE(func != nullptr);

    int result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == 3);
}

TEST_CASE("When a property is defined as BoolArray, its item getter function exists and returns values when the index "
          "is not out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetBoolArrayPropItem");
    REQUIRE(func != nullptr);

    {
        bool result;
        REQUIRE(Red::CallFunction(record, func, result, 0) == true);
        REQUIRE(result == true);
    }
    {
        bool result;
        REQUIRE(Red::CallFunction(record, func, result, 1) == true);
        REQUIRE(result == false);
    }
    {
        bool result;
        REQUIRE(Red::CallFunction(record, func, result, 2) == true);
        REQUIRE(result == true);
    }
}

TEST_CASE("When a property is defined as BoolArray, its item getter function does not modify the output if the index "
          "is out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetBoolArrayPropItem");
    REQUIRE(func != nullptr);

    {
        bool result = false;
        REQUIRE(Red::CallFunction(record, func, result, -1) == true);
        REQUIRE(result == false);
    }
    {
        bool result = true;
        REQUIRE(Red::CallFunction(record, func, result, 3) == true);
        REQUIRE(result == true);
    }
}

TEST_CASE("When a property is defined as BoolArray, its contains function returns true if the item is present in the "
          "array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("BoolArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, true) == true);
    REQUIRE(result == true);
}

TEST_CASE("When a property is defined as StringArray, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("StringArrayProp");
    REQUIRE(func != nullptr);

    Red::DynArray<Red::CString> result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.size() == 3);
    REQUIRE(result.At(0) == "one");
    REQUIRE(result.At(1) == "two");
    REQUIRE(result.At(2) == "three");
}

TEST_CASE("When a property is defined as StringArray, its count function exists and returns the correct size")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetStringArrayPropCount");
    REQUIRE(func != nullptr);

    int result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == 3);
}

TEST_CASE("When a property is defined as StringArray, its item getter function exists and returns values when the "
          "index is not out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetStringArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::CString result;
        REQUIRE(Red::CallFunction(record, func, result, 0) == true);
        REQUIRE(result == "one");
    }
    {
        Red::CString result;
        REQUIRE(Red::CallFunction(record, func, result, 1) == true);
        REQUIRE(result == "two");
    }
    {
        Red::CString result;
        REQUIRE(Red::CallFunction(record, func, result, 2) == true);
        REQUIRE(result == "three");
    }
}

TEST_CASE("When a property is defined as StringArray, its item getter function does not modify the output if the "
          "index is out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetStringArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::CString result = "asdf";
        REQUIRE(Red::CallFunction(record, func, result, -1) == true);
        REQUIRE(result == "asdf");
    }
    {
        Red::CString result = "qwer";
        REQUIRE(Red::CallFunction(record, func, result, 3) == true);
        REQUIRE(result == "qwer");
    }
}

TEST_CASE("When a property is defined as StringArray, its contains function returns true if the item is present in the "
          "array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("StringArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::CString("two")) == true);
    REQUIRE(result == true);
}

TEST_CASE("When a property is defined as StringArray, its contains function returns false if the item is not present "
          "in the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("StringArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::CString("four")) == true);
    REQUIRE(result == false);
}

TEST_CASE("When a property is defined as CNameArray, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("CNameArrayProp");
    REQUIRE(func != nullptr);

    Red::DynArray<Red::CName> result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.size() == 3);
    REQUIRE(result.At(0) == "one");
    REQUIRE(result.At(1) == "two");
    REQUIRE(result.At(2) == "three");
}

TEST_CASE("When a property is defined as CNameArray, its count function exists and returns the correct size")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetCNameArrayPropCount");
    REQUIRE(func != nullptr);

    int result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == 3);
}

TEST_CASE("When a property is defined as CNameArray, its item getter function exists and returns values when the index "
          "is not out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetCNameArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::CName result;
        REQUIRE(Red::CallFunction(record, func, result, 0) == true);
        REQUIRE(result == "one");
    }
    {
        Red::CName result;
        REQUIRE(Red::CallFunction(record, func, result, 1) == true);
        REQUIRE(result == "two");
    }
    {
        Red::CName result;
        REQUIRE(Red::CallFunction(record, func, result, 2) == true);
        REQUIRE(result == "three");
    }
}

TEST_CASE("When a property is defined as CNameArray, its item getter function does not modify the output if the index "
          "is out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetCNameArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::CName result = "asdf";
        REQUIRE(Red::CallFunction(record, func, result, -1) == true);
        REQUIRE(result == "asdf");
    }
    {
        Red::CName result = "qwer";
        REQUIRE(Red::CallFunction(record, func, result, 3) == true);
        REQUIRE(result == "qwer");
    }
}

TEST_CASE("When a property is defined as CNameArray, its contains function returns true if the item is present in the "
          "array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("CNameArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::CName("two")) == true);
    REQUIRE(result == true);
}

TEST_CASE("When a property is defined as CNameArray, its contains function returns false if the item is not present "
          "in the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("CNameArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::CName("four")) == true);
    REQUIRE(result == false);
}

TEST_CASE("When a property is defined as LocKeyArray, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("LocKeyArrayProp");
    REQUIRE(func != nullptr);

    Red::DynArray<Red::CName> result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.size() == 3);
    REQUIRE(result.At(0) == Red::CName("one"));
    REQUIRE(result.At(1) == Red::CName("two"));
    REQUIRE(result.At(2) == Red::CName("three"));
}

TEST_CASE("When a property is defined as LocKeyArray, its count function exists and returns the correct size")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetLocKeyArrayPropCount");
    REQUIRE(func != nullptr);

    int result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == 3);
}

TEST_CASE("When a property is defined as LocKeyArray, its item getter function exists and returns values when the "
          "index is not out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetLocKeyArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::CName result;
        REQUIRE(Red::CallFunction(record, func, result, 0) == true);
        REQUIRE(result == Red::CName("one"));
    }
    {
        Red::CName result;
        REQUIRE(Red::CallFunction(record, func, result, 1) == true);
        REQUIRE(result == Red::CName("two"));
    }
    {
        Red::CName result;
        REQUIRE(Red::CallFunction(record, func, result, 2) == true);
        REQUIRE(result == Red::CName("three"));
    }
}

TEST_CASE("When a property is defined as LocKeyArray, its item getter function does not modify the output if the "
          "index is out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetLocKeyArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::CName result("asdf");
        REQUIRE(Red::CallFunction(record, func, result, -1) == true);
        REQUIRE(result == Red::CName("asdf"));
    }
    {
        Red::CName result("qwer");
        REQUIRE(Red::CallFunction(record, func, result, 3) == true);
        REQUIRE(result == Red::CName("qwer"));
    }
}

TEST_CASE("When a property is defined as LocKeyArray, its contains function returns true if the item is present in the "
          "array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("LocKeyArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::CName("two")) == true);
    REQUIRE(result == true);
}

TEST_CASE("When a property is defined as LocKeyArray, its contains function returns false if the item is not present "
          "in the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("LocKeyArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::CName("four")) == true);
    REQUIRE(result == false);
}

TEST_CASE("When a property is defined as ResRefArray, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("ResRefArrayProp");
    REQUIRE(func != nullptr);

    Red::DynArray<Red::ResRef> result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.size() == 1);
    REQUIRE(result.At(0).resource.path ==
            "base\\gameplay\\vehicles\\visual_customization\\decals\\rayfield_caliburn\\vcc_rayfield_caliburn_"
            "bumper_f_numbers_01_yellow.ent");
}

TEST_CASE("When a property is defined as ResRefArray, its count function exists and returns the correct size")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetResRefArrayPropCount");
    REQUIRE(func != nullptr);

    int result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == 1);
}

TEST_CASE("When a property is defined as ResRefArray, its item getter function exists and returns values when the "
          "index is not out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetResRefArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::ResRef result;
        REQUIRE(Red::CallFunction(record, func, result, 0) == true);
        REQUIRE(result.resource.path ==
                "base\\gameplay\\vehicles\\visual_customization\\decals\\rayfield_caliburn\\vcc_rayfield_caliburn_"
                "bumper_f_numbers_01_yellow.ent");
    }
}

TEST_CASE("When a property is defined as ResRefArray, its item getter function does not modify the output if the "
          "index is out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetResRefArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::ResRef result;
        result.resource.path = "asdf";
        REQUIRE(Red::CallFunction(record, func, result, -1) == true);
        REQUIRE(result.resource.path == "asdf");
    }
    {
        Red::ResRef result;
        result.resource.path = "qwer";
        REQUIRE(Red::CallFunction(record, func, result, 1) == true);
        REQUIRE(result.resource.path == "qwer");
    }
}

TEST_CASE("When a property is defined as QuaternionArray, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("QuaternionArrayProp");
    REQUIRE(func != nullptr);

    Red::DynArray<Red::Quaternion> result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.size() == 2);
    REQUIRE(result.At(0).i == 1.0);
    REQUIRE(result.At(0).j == 2.0);
    REQUIRE(result.At(0).k == 3.0);
    REQUIRE(result.At(0).r == 4.0);
    REQUIRE(result.At(1).i == 5.0);
    REQUIRE(result.At(1).j == 6.0);
    REQUIRE(result.At(1).k == 7.0);
    REQUIRE(result.At(1).r == 8.0);
}

TEST_CASE("When a property is defined as QuaternionArray, its count function exists and returns the correct size")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetQuaternionArrayPropCount");
    REQUIRE(func != nullptr);

    int result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == 2);
}

TEST_CASE("When a property is defined as QuaternionArray, its item getter function exists and returns values when the "
          "index is not out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetQuaternionArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::Quaternion result;
        REQUIRE(Red::CallFunction(record, func, result, 0) == true);
        REQUIRE(result.i == 1.0);
        REQUIRE(result.j == 2.0);
        REQUIRE(result.k == 3.0);
        REQUIRE(result.r == 4.0);
    }
    {
        Red::Quaternion result;
        REQUIRE(Red::CallFunction(record, func, result, 1) == true);
        REQUIRE(result.i == 5.0);
        REQUIRE(result.j == 6.0);
        REQUIRE(result.k == 7.0);
        REQUIRE(result.r == 8.0);
    }
}

TEST_CASE("When a property is defined as QuaternionArray, its item getter function does not modify the output if the "
          "index is out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetQuaternionArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::Quaternion result;
        result.i = -1.0;
        result.j = -2.0;
        result.k = -3.0;
        result.r = -4.0;
        REQUIRE(Red::CallFunction(record, func, result, -1) == true);
        REQUIRE(result.i == -1.0);
        REQUIRE(result.j == -2.0);
        REQUIRE(result.k == -3.0);
        REQUIRE(result.r == -4.0);
    }
    {
        Red::Quaternion result;
        result.i = -5.0;
        result.j = -6.0;
        result.k = -7.0;
        result.r = -8.0;
        REQUIRE(Red::CallFunction(record, func, result, 2) == true);
        REQUIRE(result.i == -5.0);
        REQUIRE(result.j == -6.0);
        REQUIRE(result.k == -7.0);
        REQUIRE(result.r == -8.0);
    }
}

TEST_CASE("When a property is defined as QuaternionArray, its contains function returns true if the item is present in "
          "the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("QuaternionArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::Quaternion{1.0, 2.0, 3.0, 4.0}) == true);
    REQUIRE(result == true);
}

TEST_CASE("When a property is defined as QuaternionArray, its contains function returns false if the item is not "
          "present in the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("QuaternionArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::Quaternion{9.0, 8.0, 7.0, 6.0}) == true);
    REQUIRE(result == false);
}

TEST_CASE("When a property is defined as EulerAnglesArray, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("EulerAnglesArrayProp");
    REQUIRE(func != nullptr);

    Red::DynArray<Red::EulerAngles> result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.size() == 2);
    REQUIRE(result.At(0).Roll == 1.0);
    REQUIRE(result.At(0).Pitch == 2.0);
    REQUIRE(result.At(0).Yaw == 3.0);
    REQUIRE(result.At(1).Roll == 4.0);
    REQUIRE(result.At(1).Pitch == 5.0);
    REQUIRE(result.At(1).Yaw == 6.0);
}

TEST_CASE("When a property is defined as EulerAnglesArray, its count function exists and returns the correct size")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetEulerAnglesArrayPropCount");
    REQUIRE(func != nullptr);

    int result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == 2);
}

TEST_CASE("When a property is defined as EulerAnglesArray, its item getter function exists and returns values when the "
          "index is not out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetEulerAnglesArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::EulerAngles result;
        REQUIRE(Red::CallFunction(record, func, result, 0) == true);
        REQUIRE(result.Roll == 1.0);
        REQUIRE(result.Pitch == 2.0);
        REQUIRE(result.Yaw == 3.0);
    }
    {
        Red::EulerAngles result;
        REQUIRE(Red::CallFunction(record, func, result, 1) == true);
        REQUIRE(result.Roll == 4.0);
        REQUIRE(result.Pitch == 5.0);
        REQUIRE(result.Yaw == 6.0);
    }
}

TEST_CASE("When a property is defined as EulerAnglesArray, its item getter function does not modify the output if the "
          "index is out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetEulerAnglesArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::EulerAngles result;
        result.Roll = -1.0;
        result.Pitch = -2.0;
        result.Yaw = -3.0;
        REQUIRE(Red::CallFunction(record, func, result, -1) == true);
        REQUIRE(result.Roll == -1.0);
        REQUIRE(result.Pitch == -2.0);
        REQUIRE(result.Yaw == -3.0);
    }
    {
        Red::EulerAngles result;
        result.Roll = -4.0;
        result.Pitch = -5.0;
        result.Yaw = -6.0;
        REQUIRE(Red::CallFunction(record, func, result, 2) == true);
        REQUIRE(result.Roll == -4.0);
        REQUIRE(result.Pitch == -5.0);
        REQUIRE(result.Yaw == -6.0);
    }
}

TEST_CASE("When a property is defined as EulerAnglesArray, its contains function returns true if the item is present "
          "in the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("EulerAnglesArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::EulerAngles{1.0, 2.0, 3.0}) == true);
    REQUIRE(result == true);
}

TEST_CASE("When a property is defined as EulerAnglesArray, its contains function returns false if the item is not "
          "present in the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("EulerAnglesArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::EulerAngles{7.0, 8.0, 9.0}) == true);
    REQUIRE(result == false);
}

TEST_CASE("When a property is defined as Vector3Array, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("Vector3ArrayProp");
    REQUIRE(func != nullptr);

    Red::DynArray<Red::Vector3> result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.size() == 2);
    REQUIRE(result.At(0).X == 1.0);
    REQUIRE(result.At(0).Y == 2.0);
    REQUIRE(result.At(0).Z == 3.0);
    REQUIRE(result.At(1).X == 4.0);
    REQUIRE(result.At(1).Y == 5.0);
    REQUIRE(result.At(1).Z == 6.0);
}

TEST_CASE("When a property is defined as Vector3Array, its count function exists and returns the correct size")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetVector3ArrayPropCount");
    REQUIRE(func != nullptr);

    int result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == 2);
}

TEST_CASE("When a property is defined as Vector3Array, its item getter function exists and returns values when the "
          "index is not out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetVector3ArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::Vector3 result;
        REQUIRE(Red::CallFunction(record, func, result, 0) == true);
        REQUIRE(result.X == 1.0);
        REQUIRE(result.Y == 2.0);
        REQUIRE(result.Z == 3.0);
    }
    {
        Red::Vector3 result;
        REQUIRE(Red::CallFunction(record, func, result, 1) == true);
        REQUIRE(result.X == 4.0);
        REQUIRE(result.Y == 5.0);
        REQUIRE(result.Z == 6.0);
    }
}

TEST_CASE("When a property is defined as Vector3Array, its item getter function does not modify the output if the "
          "index is out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetVector3ArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::Vector3 result;
        result.X = -1.0;
        result.Y = -2.0;
        result.Z = -3.0;
        REQUIRE(Red::CallFunction(record, func, result, -1) == true);
        REQUIRE(result.X == -1.0);
        REQUIRE(result.Y == -2.0);
        REQUIRE(result.Z == -3.0);
    }
    {
        Red::Vector3 result;
        result.X = -4.0;
        result.Y = -5.0;
        result.Z = -6.0;
        REQUIRE(Red::CallFunction(record, func, result, 2) == true);
        REQUIRE(result.X == -4.0);
        REQUIRE(result.Y == -5.0);
        REQUIRE(result.Z == -6.0);
    }
}

TEST_CASE("When a property is defined as Vector3Array, its contains function returns true if the item is present in "
          "the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("Vector3ArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::Vector3{1.0, 2.0, 3.0}) == true);
    REQUIRE(result == true);
}

TEST_CASE("When a property is defined as Vector3Array, its contains function returns false if the item is not present "
          "in the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("Vector3ArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::Vector3{7.0, 8.0, 9.0}) == true);
    REQUIRE(result == false);
}

TEST_CASE("When a property is defined as Vector2Array, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("Vector2ArrayProp");
    REQUIRE(func != nullptr);

    Red::DynArray<Red::Vector2> result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.size() == 2);
    REQUIRE(result.At(0).X == 1.0);
    REQUIRE(result.At(0).Y == 2.0);
    REQUIRE(result.At(1).X == 3.0);
    REQUIRE(result.At(1).Y == 4.0);
}

TEST_CASE("When a property is defined as Vector2Array, its count function exists and returns the correct size")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetVector2ArrayPropCount");
    REQUIRE(func != nullptr);

    int result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == 2);
}

TEST_CASE("When a property is defined as Vector2Array, its item getter function exists and returns values when the "
          "index is not out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetVector2ArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::Vector2 result;
        REQUIRE(Red::CallFunction(record, func, result, 0) == true);
        REQUIRE(result.X == 1.0);
        REQUIRE(result.Y == 2.0);
    }
    {
        Red::Vector2 result;
        REQUIRE(Red::CallFunction(record, func, result, 1) == true);
        REQUIRE(result.X == 3.0);
        REQUIRE(result.Y == 4.0);
    }
}

TEST_CASE("When a property is defined as Vector2Array, its item getter function does not modify the output if the "
          "index is out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetVector2ArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::Vector2 result;
        result.X = -1.0;
        result.Y = -2.0;
        REQUIRE(Red::CallFunction(record, func, result, -1) == true);
        REQUIRE(result.X == -1.0);
        REQUIRE(result.Y == -2.0);
    }
    {
        Red::Vector2 result;
        result.X = -3.0;
        result.Y = -4.0;
        REQUIRE(Red::CallFunction(record, func, result, 2) == true);
        REQUIRE(result.X == -3.0);
        REQUIRE(result.Y == -4.0);
    }
}

TEST_CASE("When a property is defined as Vector2Array, its contains function returns true if the item is present in "
          "the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("Vector2ArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::Vector2{1.0, 2.0}) == true);
    REQUIRE(result == true);
}

TEST_CASE("When a property is defined as Vector2Array, its contains function returns false if the item is not present "
          "in the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("Vector2ArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::Vector2{5.0, 6.0}) == true);
    REQUIRE(result == false);
}

TEST_CASE("When a property is defined as ColorArray, its getter function exists and returns a valid value")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("ColorArrayProp");
    REQUIRE(func != nullptr);

    Red::DynArray<Red::Color> result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.size() == 2);
    REQUIRE(result.At(0).Red == 1);
    REQUIRE(result.At(0).Green == 2);
    REQUIRE(result.At(0).Blue == 3);
    REQUIRE(result.At(0).Alpha == 4);
    REQUIRE(result.At(1).Red == 4);
    REQUIRE(result.At(1).Green == 5);
    REQUIRE(result.At(1).Blue == 6);
    REQUIRE(result.At(1).Alpha == 7);
}

TEST_CASE("When a property is defined as ColorArray, its count function exists and returns the correct size")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetColorArrayPropCount");
    REQUIRE(func != nullptr);

    int result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == 2);
}

TEST_CASE("When a property is defined as ColorArray, its item getter function exists and returns values when the index "
          "is not out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetColorArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::Color result;
        REQUIRE(Red::CallFunction(record, func, result, 0) == true);
        REQUIRE(result.Red == 1);
        REQUIRE(result.Green == 2);
        REQUIRE(result.Blue == 3);
        REQUIRE(result.Alpha == 4);
    }
    {
        Red::Color result;
        REQUIRE(Red::CallFunction(record, func, result, 1) == true);
        REQUIRE(result.Red == 4);
        REQUIRE(result.Green == 5);
        REQUIRE(result.Blue == 6);
        REQUIRE(result.Alpha == 7);
    }
}

TEST_CASE("When a property is defined as ColorArray, its item getter function does not modify the output if the index "
          "is out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetColorArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::Color result;
        result.Red = 10;
        result.Green = 20;
        result.Blue = 30;
        result.Alpha = 40;
        REQUIRE(Red::CallFunction(record, func, result, -1) == true);
        REQUIRE(result.Red == 10);
        REQUIRE(result.Green == 20);
        REQUIRE(result.Blue == 30);
        REQUIRE(result.Alpha == 40);
    }
    {
        Red::Color result;
        result.Red = 50;
        result.Green = 60;
        result.Blue = 70;
        result.Alpha = 80;
        REQUIRE(Red::CallFunction(record, func, result, 2) == true);
        REQUIRE(result.Red == 50);
        REQUIRE(result.Green == 60);
        REQUIRE(result.Blue == 70);
        REQUIRE(result.Alpha == 80);
    }
}

TEST_CASE("When a property is defined as ColorArray, its contains function returns true if the item is present in the "
          "array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("ColorArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::Color{1, 2, 3, 4}) == true);
    REQUIRE(result == true);
}

TEST_CASE("When a property is defined as ColorArray, its contains function returns false if the item is not present "
          "in the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("ColorArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    REQUIRE(Red::CallFunction(record, func, result, Red::Color{0, 0, 0, 1}) == true);
    REQUIRE(result == false);
}

TEST_CASE("When a property is defined as a TweakDBID, its getter function exists and returns a weak handle to a "
          "TweakDB record with that ID")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("TweakDBIDProp");
    REQUIRE(func != nullptr);

    Red::WeakHandle<Red::TweakDBRecord> result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.instance->recordID == Red::TweakDBID("Vehicle.v_sport1_herrera_outlaw"));
}

TEST_CASE("When a property is defined as a TweakDBID, its handle getter function returns a strong handle to a TweakDB "
          "record with that ID")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("TweakDBIDPropHandle");
    REQUIRE(func != nullptr);

    Red::Handle<Red::TweakDBRecord> result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.instance->recordID == Red::TweakDBID("Vehicle.v_sport1_herrera_outlaw"));
}

TEST_CASE("When a property is defined as a TweakDBIDArray, its getter function exists and returns an array of weak "
          "handles to TweakDB records with the correct IDs")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("TweakDBIDArrayProp");
    REQUIRE(func != nullptr);

    Red::DynArray<Red::WeakHandle<Red::Vehicle_Record>> result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result.size() == 3);
    REQUIRE(result.At(0).instance->recordID == Red::TweakDBID("Vehicle.v_sport1_herrera_outlaw"));
    REQUIRE(result.At(1).instance->recordID == Red::TweakDBID("Vehicle.v_sport1_quadra_sport_r7"));
    REQUIRE(result.At(2).instance->recordID == Red::TweakDBID("Vehicle.v_sport1_rayfield_caliburn"));
}

TEST_CASE("When a property is defined as a TweakDBIDArray, its count function exists and returns the correct size")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetTweakDBIDArrayPropCount");
    REQUIRE(func != nullptr);

    int result;
    REQUIRE(Red::CallFunction(record, func, result) == true);
    REQUIRE(result == 3);
}

TEST_CASE("When a property is defined as a TweakDBIDArray, its item getter function exists and returns values when "
          "the index is not out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetTweakDBIDArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::WeakHandle<Red::TweakDBRecord> result;
        REQUIRE(Red::CallFunction(record, func, result, 0) == true);
        REQUIRE(result.instance->recordID == Red::TweakDBID("Vehicle.v_sport1_herrera_outlaw"));
    }
    {
        Red::WeakHandle<Red::TweakDBRecord> result;
        REQUIRE(Red::CallFunction(record, func, result, 1) == true);
        REQUIRE(result.instance->recordID == Red::TweakDBID("Vehicle.v_sport1_quadra_sport_r7"));
    }
    {
        Red::WeakHandle<Red::TweakDBRecord> result;
        REQUIRE(Red::CallFunction(record, func, result, 2) == true);
        REQUIRE(result.instance->recordID == Red::TweakDBID("Vehicle.v_sport1_rayfield_caliburn"));
    }
}

TEST_CASE("When a property is defined as a TweakDBIDArray, its item getter function does not modify the output if "
          "the index is out of bounds")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("GetTweakDBIDArrayPropItem");
    REQUIRE(func != nullptr);

    {
        Red::WeakHandle<Red::TweakDBRecord> result;
        REQUIRE(Red::CallFunction(record, func, result, -1) == true);
        REQUIRE(result.instance == nullptr);
    }
    {
        Red::WeakHandle<Red::TweakDBRecord> result;
        REQUIRE(Red::CallFunction(record, func, result, 3) == true);
        REQUIRE(result.instance == nullptr);
    }
}

TEST_CASE("When a property is defined as a TweakDBIDArray, its contains function returns true if the item is "
          "present in the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("TweakDBIDArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    const Red::WeakHandle item = tweakManager->GetRecord(Red::TweakDBID("Vehicle.v_sport1_herrera_outlaw"));
    REQUIRE(item.instance != nullptr);
    REQUIRE(Red::CallFunction(record, func, result, item) == true);
    REQUIRE(result == true);
}

TEST_CASE("When a property is defined as a TweakDBIDArray, its contains function returns false if the item is not "
          "present in the array")
{
    const auto tweakManager = Tests::ScriptableRecordTestRunner::GetTweakManager();

    const auto record =
        reinterpret_cast<ScriptableTweakDBRecord*>(tweakManager->GetTweakDB()->GetRecord(RecordID).instance);

    auto* func = RecordType::GetClass()->GetFunction("TweakDBIDArrayPropContains");
    REQUIRE(func != nullptr);

    bool result;
    const Red::WeakHandle item = tweakManager->GetRecord(Red::TweakDBID("Vehicle.v_standard25_villefort_columbus"));
    REQUIRE(item.instance != nullptr);
    REQUIRE(Red::CallFunction(record, func, result, item) == true);
    REQUIRE(result == false);
}
} // namespace App

#endif
