#include "LoopRangeComponent.h"

LoopRangeComponent::LoopRangeComponent(te::Edit& e)
    : edit(e)
{
    setInterceptsMouseClicks(false, false); // Don't block mouse events
}

void LoopRangeComponent::paint(juce::Graphics& g)
{
    // Only draw if looping is enabled and we have a valid loop range
    if (!looping || loopRange.getLength().inSeconds() <= 0.0)
        return;

    // Convert loop range time positions to beat positions
    const auto startBeatPos = edit.tempoSequence.toBeats(loopRange.getStart());
    const auto endBeatPos = edit.tempoSequence.toBeats(loopRange.getEnd());

    // Calculate x positions in beat space
    const double startX = (startBeatPos.inBeats() - viewStartBeat.inBeats()) * pixelsPerBeat;
    const double endX = (endBeatPos.inBeats() - viewStartBeat.inBeats()) * pixelsPerBeat;

    // Draw the loop range lines in dark orange
    g.setColour(juce::Colours::darkorange);
    g.drawLine(static_cast<float>(startX), 0.0f, static_cast<float>(startX), static_cast<float>(getHeight()), 2.0f);
    g.drawLine(static_cast<float>(endX), 0.0f, static_cast<float>(endX), static_cast<float>(getHeight()), 2.0f);
}

void LoopRangeComponent::setPixelsPerBeat(double ppb)
{
    if (pixelsPerBeat != ppb)
    {
        pixelsPerBeat = ppb;
        repaint();
    }
}

void LoopRangeComponent::setViewStartBeat(t::BeatPosition b)
{
    if (viewStartBeat != b)
    {
        viewStartBeat = b;
        repaint();
    }
}

void LoopRangeComponent::setLoopRange(t::TimeRange range)
{
    if (loopRange != range)
    {
        loopRange = range;
        repaint();
    }
}

void LoopRangeComponent::setLooping(bool shouldLoop)
{
    if (looping != shouldLoop)
    {
        looping = shouldLoop;
        repaint();
    }
}
