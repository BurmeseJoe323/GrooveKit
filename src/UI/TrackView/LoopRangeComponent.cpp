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

    // Calculate positions of the start and end lines
    const double startX = (loopRange.getStart().inSeconds() - viewStart.inSeconds()) * pixelsPerSecond;
    const double endX = (loopRange.getEnd().inSeconds() - viewStart.inSeconds()) * pixelsPerSecond;

    // Draw the loop range lines in dark orange
    g.setColour(juce::Colours::darkorange);
    g.drawLine(static_cast<float>(startX), 0.0f, static_cast<float>(startX), static_cast<float>(getHeight()), 2.0f);
    g.drawLine(static_cast<float>(endX), 0.0f, static_cast<float>(endX), static_cast<float>(getHeight()), 2.0f);
}

void LoopRangeComponent::setPixelsPerSecond(double pps)
{
    if (pixelsPerSecond != pps)
    {
        pixelsPerSecond = pps;
        repaint();
    }
}

void LoopRangeComponent::setViewStart(t::TimePosition t)
{
    if (viewStart != t)
    {
        viewStart = t;
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
