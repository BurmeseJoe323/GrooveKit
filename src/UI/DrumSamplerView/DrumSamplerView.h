#pragma once

#include "DrumSamplerComponent.h"

/**
 * @brief Alias for the current drum sampler UI implementation.
 *
 * This allows the rest of the codebase to refer to `DrumSamplerView` instead
 * of the concrete class name `DrumSamplerComponent`. If we ever decide to
 * swap in a different implementation, we can update this alias instead of
 * changing all call sites.
 */
using DrumSamplerView = DrumSamplerComponent;