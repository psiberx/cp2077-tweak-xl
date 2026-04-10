#pragma once

#ifndef NDEBUG

#include "App/Tweaks/TweakService.hpp"

namespace App::Tests
{
/**
 * @brief A test runner for validating the functionality of scriptable records within the context of the application's
 * tweak system. This class provides methods to set up the necessary environment for testing scriptable records,
 * including creating a scriptable record type with various property types and an instance of that type with predefined
 * values.
 *
 * The tests focus on verifying the correct behavior of property getters and the ability to create scriptable
 * record instances, ensuring that the scriptable record system functions as intended within the application's
 * architecture.
 */
class ScriptableRecordTestRunner
{
public:
    /**
     * @brief Gets the TweakService instance.
     *
     * @return A shared pointer to the TweakService instance.
     */
    static Core::SharedPtr<TweakService> GetTweakService();

    /**
     * @brief Gets the TweakDBManager instance.
     *
     * @return A deferred pointer to the TweakDBManager instance.
     */
    static Core::DeferredPtr<Red::TweakDBManager> GetTweakManager();

    /**
     * @brief Gets the ScriptableRecordManager instance.
     *
     * @return A shared pointer to the ScriptableRecordManager instance.
     */
    static Core::SharedPtr<ScriptableRecordManager> GetRecordManager();

    /**
     * @brief Gets the ScriptablePropertyHandler instance.
     *
     * @return A shared pointer to the ScriptablePropertyHandler instance.
     */
    static Core::SharedPtr<ScriptablePropertyManager> GetPropertyHandler();

    /**
     * @brief Runs the scriptable record test suite and outputs results to a log located in the specified directory.
     *
     * @param aDir The directory where the test log should be saved.
     * @return An integer representing the number of failed tests. A return value of 0 indicates that all tests passed
     * successfully.
     */
    static int Run(const std::filesystem::path& aDir);

private:
    /**
     * @brief Hidden constructor so that it cannot be externally instantiated.
     */
    ScriptableRecordTestRunner() = default;

    /**
     * @brief Creates and sets up a scriptable record class containing all supported property types and creates an
     * instance of it with a set of values. Once complete, the test suite has the state required to validate property
     * getter functionality and the ability to create scriptable record instances.
     */
    static void Setup();

    /**
     * @brief A boolean flag indicating whether the test environment has been set up. This ensures that the setup
     * process is only performed once, even if the Run method is called multiple times.
     */
    static inline bool s_isSetup = false;
};
} // namespace App::Tests

#endif
