#include "TimelineComponent.h"

using namespace juce;

static constexpr int kTickBottom    = 22;
static constexpr int kMajorTickLen  = 10;
static constexpr int kMinorTickLen  = 5;

void ui::TimelineComponent::setPixelsPerSecond(double pps)
{
    // Ensure the zoom level never gets ridiculously small or negative
    pixelsPerSecond = juce::jmax(10.0, pps);

    // Trigger a repaint so the timeline ticks update visually
    repaint();
}

void ui::TimelineComponent::setViewStart(te::TimePosition t)
{
    // Update where the visible region of the timeline starts (in seconds)
    viewStart = t;

    // Trigger a repaint to reflect the new offset
    repaint();
}

void ui::TimelineComponent::paint (Graphics& g)
{
    auto r = getLocalBounds();

    g.setColour (Colour (0xFF2B2F33));
    g.fillRect (r);

    g.setColour (Colours::black.withAlpha (0.4f));
    g.drawLine (0.0f, (float) r.getBottom() - 0.5f, (float) r.getRight(), (float) r.getBottom() - 0.5f);

    const double targetPx = 100.0;
    const double secondsPerMajor = std::max (0.5, std::pow (2.0, std::floor (std::log2 (targetPx / pixelsPerSecond))));
    const double secondsPerMinor = secondsPerMajor / 4.0;

    const double startSec = std::floor (viewStart.inSeconds() / secondsPerMinor) * secondsPerMinor;
    const double endSec   = viewStart.inSeconds() + (double) getWidth() / pixelsPerSecond;

    for (double t = startSec; t <= endSec + 1e-6; t += secondsPerMinor)
    {
        const int x = int ((t - viewStart.inSeconds()) * pixelsPerSecond + 0.5);
        const bool isMajor = std::fmod (t, secondsPerMajor) < 1e-9;

        g.setColour (Colours::white.withAlpha (isMajor ? 0.55f : 0.25f));
        const int tickLen = isMajor ? kMajorTickLen : kMinorTickLen;
        g.drawLine ((float) x + 0.5f, (float) r.getBottom() - (float) tickLen, (float) x + 0.5f, (float) r.getBottom());

        if (isMajor && x >= 0 && x <= r.getRight())
        {
            g.setColour (Colours::white.withAlpha (0.7f));
            g.setFont (Font (FontOptions (11.0f)));
            String label (t, 2);
            g.drawFittedText (label, Rectangle<int> (x + 3, 2, 60, 14), Justification::left, 1);
        }
    }

    const double playPos = edit.getTransport().getPosition().inSeconds();
    const int playX = int ((playPos - viewStart.inSeconds()) * pixelsPerSecond + 0.5);

    if (playX >= 0 && playX < getWidth())
    {
        g.setColour (juce::Colours::aqua.withAlpha (0.8f));
        g.drawLine ((float) playX + 0.5f, 0.0f, (float) playX + 0.5f, (float) r.getBottom());
    }

}

void ui::TimelineComponent::setTransportPositionFromX (int x, bool dragging)
{
    const auto time = xToTime (x);
    auto& tc = edit.getTransport();
    tc.setPosition (time);

    if (onScrub)
        onScrub (time);

    ignoreUnused (dragging); // no EditViewState dependency anymore
}

void ui::TimelineComponent::mouseDown (const MouseEvent& e)
{
    setTransportPositionFromX (e.x, true);
    repaint();
}

void ui::TimelineComponent::mouseDrag (const MouseEvent& e)
{
    setTransportPositionFromX (e.x, true);
    repaint();
}
