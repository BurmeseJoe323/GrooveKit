#include "PlayheadComponent.h"

PlayheadComponent::PlayheadComponent (te::Edit& e, EditViewState& evs) : edit (e), editViewState (evs)
{
    startTimerHz (30);
}

void PlayheadComponent::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::yellow);
    g.drawRect (xPosition, 0, 2, getHeight());
}

bool PlayheadComponent::hitTest (int x, int)
{
    if (std::abs (x - xPosition) <= 3)
        return true;

    return false;
}

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
    auto t = editViewState.xToTime (e.x, getParentComponent()->getWidth());
    edit.getTransport().setPosition (t);
    timerCallback();
}

void PlayheadComponent::timerCallback()
{
    const double tSec = edit.getTransport().getPosition().inSeconds();
    const int newX = (int) juce::roundToIntAccurate((tSec - viewStart.inSeconds()) * pixelsPerSecond);

    if (newX != xPosition)
    {
        repaint (juce::jmin(newX, xPosition) - 2, 0,
                 std::abs(newX - xPosition) + 4, getHeight());
        xPosition = newX;
    }
}
