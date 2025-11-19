#include "LoopRangeComponent.h"

/**
 * @brief Constructor.
 *
 * The component does not accept mouse input; it simply renders loop markers
 * on top of the track view without interfering with user interaction.
 */
LoopRangeComponent::LoopRangeComponent (te::Edit& e)
    : edit (e)
{
    // Allow mouse events to pass through to underlying components.
    setInterceptsMouseClicks (false, false);
}

/**
 * @brief Draw two vertical lines at the loop start and loop end.
 *
 * Lines are only drawn if:
 *   - looping is true
 *   - loopRange has non-zero length
 *
 * The loop range is converted from seconds → beats → x-pixel positions.
 */
void LoopRangeComponent::paint (juce::Graphics& g)
{
    if (! looping || loopRange.getLength().inSeconds() <= 0.0)
        return;

    // Convert loop boundaries to beat positions
    const auto startBeatPos = edit.tempoSequence.toBeats (loopRange.getStart());
    const auto endBeatPos   = edit.tempoSequence.toBeats (loopRange.getEnd());

    // Convert beat → x pixel based on zoom & scroll
    const double startX = (startBeatPos.inBeats() - viewStartBeat.inBeats()) * pixelsPerBeat;
    const double endX   = (endBeatPos.inBeats()   - viewStartBeat.inBeats()) * pixelsPerBeat;

    // Draw vertical loop marker lines
    g.setColour (juce::Colours::darkorange);

    g.drawLine ((float) startX,
                0.0f,
                (float) startX,
                (float) getHeight(),
                2.0f);

    g.drawLine ((float) endX,
                0.0f,
                (float) endX,
                (float) getHeight(),
                2.0f);
}

//==============================================================================
// Setters (each triggers repaint only when value changes)
//==============================================================================

void LoopRangeComponent::setPixelsPerBeat (double ppb)
{
    if (pixelsPerBeat != ppb)
    {
        pixelsPerBeat = ppb;
        repaint();
    }
}

void LoopRangeComponent::setViewStartBeat (t::BeatPosition b)
{
    if (viewStartBeat != b)
    {
        viewStartBeat = b;
        repaint();
    }
}

void LoopRangeComponent::setLoopRange (t::TimeRange range)
{
    if (loopRange != range)
    {
        loopRange = range;
        repaint();
    }
}

void LoopRangeComponent::setLooping (bool shouldLoop)
{
    if (looping != shouldLoop)
    {
        looping = shouldLoop;
        repaint();
    }
}