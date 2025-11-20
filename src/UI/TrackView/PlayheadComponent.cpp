#include "PlayheadComponent.h"

//==============================================================================
// Construction
//==============================================================================

PlayheadComponent::PlayheadComponent (te::Edit& e, EditViewState& evs, AppEngine& ae)
    : edit (e),
      editViewState (evs),
      appEngine (ae)
{
    startTimerHz (30);
}

//==============================================================================
// Component Overrides
//==============================================================================

void PlayheadComponent::paint (juce::Graphics& g)
{
    // Change playhead color to red during recording (Nov 19, 2025)
    if (appEngine.isRecording())
        g.setColour (juce::Colours::red);
    else
        g.setColour (juce::Colours::aqua);

    g.drawRect (xPosition, 0, 2, getHeight());
}

bool PlayheadComponent::hitTest (int x, int)
{
    if (std::abs (x - xPosition) <= 3)
        return true;

    return false;
}

//==============================================================================
// Mouse Event Handling
//==============================================================================

void PlayheadComponent::mouseDown (const juce::MouseEvent&)
{
    edit.getTransport().setUserDragging (true);
}

void PlayheadComponent::mouseUp (const juce::MouseEvent&)
{
    edit.getTransport().setUserDragging (false);
}

void PlayheadComponent::mouseDrag (const juce::MouseEvent& e)
{
    // Convert mouse x to beat position, then to time using tempo sequence
    const double beats = e.x / pixelsPerBeat + viewStartBeat.inBeats();
    const auto beatPos = t::BeatPosition::fromBeats(juce::jmax(0.0, beats));
    const auto timePos = edit.tempoSequence.toTime(beatPos);
    edit.getTransport().setPosition(timePos);
    timerCallback();
}

//==============================================================================
// Timer Callback
//==============================================================================

void PlayheadComponent::timerCallback ()
{
    // Convert transport position (time) to beat position, then to x coordinate
    const auto timePos = edit.getTransport().getPosition();
    const auto beatPos = edit.tempoSequence.toBeats(timePos);
    const double beats = beatPos.inBeats();

    if (const int newX = (beats - viewStartBeat.inBeats()) * pixelsPerBeat; newX != xPosition)
    {
        repaint (juce::jmin (newX, xPosition) - 2,
            0,
            std::abs (newX - xPosition) + 4,
            getHeight());
        xPosition = newX;
    }
}
