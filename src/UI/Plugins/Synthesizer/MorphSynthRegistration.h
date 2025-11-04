#pragma once

// ==============================================================================
// MorphSynthRegistration.h
// ------------------------------------------------------------------------------
// Registers the Morph Synth as a Tracktion Engine built-in plugin type.
// - Provides a BuiltInType subclass that constructs MorphSynthPlugin instances.
// - Exposes a single entry point to register the type with an Engine.
//
// Notes:
//   * The type string passed to BuiltInType must match MorphSynthPlugin::pluginType.
//   * Tracktion will call BuiltInType::create with PluginCreationInfo BY VALUE.
// ==============================================================================

#include <tracktion_engine/tracktion_engine.h>
#include "MorphSynthPlugin.h"

namespace te = tracktion::engine;

//==============================================================================
// Built-in type registration
//==============================================================================

/**
 * @brief Tracktion built-in type for the Morph Synth.
 *
 * The PluginManager uses this to instantiate MorphSynthPlugin when a plugin
 * with the matching type id is requested (e.g., when loading edits or creating
 * new instrument tracks).
 */
struct MorphSynthBuiltIn : public te::PluginManager::BuiltInType
{
    /**
     * @brief Construct with the Morph Synth's type identifier.
     *
     * The base class requires a stable type string; this must match the
     * MorphSynthPlugin::pluginType value.
     */
    MorphSynthBuiltIn()
        : te::PluginManager::BuiltInType (MorphSynthPlugin::pluginType) {}

    /**
     * @brief Create a new MorphSynthPlugin instance.
     * @param info Plugin creation context (passed by value by Tracktion).
     * @return New plugin pointer managed by Tracktion.
     */
    te::Plugin::Ptr create (te::PluginCreationInfo info) override
    {
        return new MorphSynthPlugin (info);
    }
};

//==============================================================================
// Public entry point
//==============================================================================

/**
 * @brief Register the Morph Synth built-in plugin with a Tracktion Engine.
 *
 * Call this once during app initialisation (after Engine is constructed)
 * so the Morph Synth appears in the host’s plugin lists and can be created
 * from code or restored from saved edits.
 */
inline void registerMorphSynthCompat (te::Engine& engine)
{
    auto& pm = engine.getPluginManager();
    pm.registerBuiltInType (std::make_unique<MorphSynthBuiltIn>());
}