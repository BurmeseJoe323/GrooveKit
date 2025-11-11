// Written by Claude Code
#include "GhostClipComponent.h"

GhostClipComponent::GhostClipComponent()
{
    setInterceptsMouseClicks (false, false); // Ghost shouldn't intercept mouse events
    setVisible (false); // Hidden by default
}

void GhostClipComponent::paint (juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat();
    constexpr float radius = 8.0f;

    // Color-code based on validity: green for valid, red for invalid
    const auto ghostColor = isValidDrop
        ? juce::Colours::green.withAlpha (0.3f)
        : juce::Colours::red.withAlpha (0.3f);

    // Fill
    g.setColour (ghostColor);
    g.fillRoundedRectangle (r, radius);

    // Border - more opaque
    const auto borderColor = isValidDrop
        ? juce::Colours::green.withAlpha (0.6f)
        : juce::Colours::red.withAlpha (0.6f);

    g.setColour (borderColor);
    g.drawRoundedRectangle (r.reduced (0.5f), radius, 2.0f);
}

void GhostClipComponent::setDropLocation (int trackIndex, t::TimePosition time,
                                          t::TimeDuration length, bool isValid)
{
    isValidDrop = isValid;
    repaint();
}

void GhostClipComponent::show()
{
    setVisible (true);
    setAlwaysOnTop (true); // Render above other clips
}

void GhostClipComponent::hide()
{
    setVisible (false);
}
