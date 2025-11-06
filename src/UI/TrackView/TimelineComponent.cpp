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
            String label (t + 1);
            g.drawFittedText (label, Rectangle<int> (x + 3, 2, 60, 14), Justification::left, 1);
        }
    }

    const double playPos = edit.getTransport().getPosition().inSeconds();
    const double playX = ((playPos - viewStart.inSeconds()) * pixelsPerSecond);

    if (playX >= 0 && playX < getWidth())
    {
        g.setColour (juce::Colours::aqua);
        g.drawRect (playX + 0.5, 0.0, 2.0, getHeight());
    }

    if (hasLoop)
    {
        const int x1 = timeSecToX(loopRange.getStart().inSeconds());
        const int x2 = timeSecToX(loopRange.getEnd().inSeconds());
        const int y  = 0, h = getHeight();

        juce::Rectangle<int> rr (juce::jmin(x1, x2), y, std::abs(x2 - x1), h);

        g.setColour (juce::Colours::darkorange.withAlpha (0.25f));
        g.fillRect (rr);

        g.setColour (juce::Colours::darkorange);
        g.drawRect (rr, 2);

        g.fillRect (juce::Rectangle<int>(rr.getX() - handleWidthPx/2, y, handleWidthPx, h));
        g.fillRect (juce::Rectangle<int>(rr.getRight() - handleWidthPx/2, y, handleWidthPx, h));
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

static bool near (int px, int target, int slop) { return std::abs(px - target) <= slop; }

void ui::TimelineComponent::mouseDown (const juce::MouseEvent& e)
{

    if (e.mods.isMiddleButtonDown() || (e.mods.isShiftDown() && e.mods.isLeftButtonDown()))
    {
        if (auto* vp = findParentComponentOfClass<juce::Viewport>())
        {
            panning = true;
            panStartView = vp->getViewPosition();
            panStartX = e.getPosition().x;
            return;
        }
    }

    const int mx = e.x;

    if (!hasLoop)
    {
        const double start = xToTimeSec(mx);
        const double defLenSec = 2.0; // seed length; change to a bar if you like

        loopRange = te::TimeRange (te::TimePosition::fromSeconds(start),
                                   te::TimePosition::fromSeconds(start + defLenSec));
        hasLoop   = true;
        dragMode  = DragMode::dragEnd;
        originalStartSec = start;
        originalEndSec   = start + defLenSec;

        if (onLoopRangeChanged) onLoopRangeChanged(loopRange);
        repaint();
        return;
    }

    const int x1 = timeSecToX(loopRange.getStart().inSeconds());
    const int x2 = timeSecToX(loopRange.getEnd().inSeconds());
    const juce::Rectangle<int> rr (juce::jmin(x1, x2), 0, std::abs(x2 - x1), getHeight());

    if (near(mx, rr.getX(), hitSlopPx))          dragMode = DragMode::dragStart;
    else if (near(mx, rr.getRight(), hitSlopPx)) dragMode = DragMode::dragEnd;
    else if (rr.contains (mx, e.y))
    {
        dragMode = DragMode::dragBody;
        dragAnchorSec   = xToTimeSec(mx);
        originalStartSec = loopRange.getStart().inSeconds();
        originalEndSec   = loopRange.getEnd().inSeconds();
    }
    else
    {
        // click outside: start a new region at cursor
        const double start = xToTimeSec(mx);
        const double defLenSec = 2.0;

        loopRange = te::TimeRange (te::TimePosition::fromSeconds(start),
                                   te::TimePosition::fromSeconds(start + defLenSec));
        hasLoop   = true;
        dragMode  = DragMode::dragEnd;
        originalStartSec = start;
        originalEndSec   = start + defLenSec;

        if (onLoopRangeChanged) onLoopRangeChanged(loopRange);
    }

    repaint();
}

void ui::TimelineComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (panning)
    {
        if (auto* vp = findParentComponentOfClass<juce::Viewport>())
        {
            const int dx = e.getPosition().x - panStartX;
            vp->setViewPosition(juce::jmax(0, panStartView.x - dx), panStartView.y);
        }
        return;
    }

    if (dragMode == DragMode::none || !hasLoop) return;

    const double t = xToTimeSec(e.x);
    double s = loopRange.getStart().inSeconds();
    double d = loopRange.getEnd().inSeconds();

    switch (dragMode)
    {
        case DragMode::dragStart: s = juce::jlimit(-1e9, d, t); break;
        case DragMode::dragEnd:   d = juce::jmax(s, t);         break;
        case DragMode::dragBody:
        {
            const double delta = t - dragAnchorSec;
            s = originalStartSec + delta;
            d = originalEndSec   + delta;
            break;
        }
        default: break;
    }

    snapSecondsToBeats(s);
    snapSecondsToBeats(d);

    loopRange = te::TimeRange (te::TimePosition::fromSeconds(s),
                               te::TimePosition::fromSeconds(d));

    if (onLoopRangeChanged) onLoopRangeChanged(loopRange);
    repaint();
}

void ui::TimelineComponent::mouseUp (const juce::MouseEvent&)
{

    panning = false;
    dragMode = DragMode::none;

}

// right-click (or cmd/ctrl double-click) clears loop
void ui::TimelineComponent::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown() || e.mods.isAnyModifierKeyDown())
    {
        hasLoop   = false;
        loopRange = te::TimeRange (te::TimePosition::fromSeconds(0.0),
                                   te::TimePosition::fromSeconds(0.0));
        if (onLoopRangeChanged) onLoopRangeChanged(loopRange);
        repaint();
    }
}


void ui::TimelineComponent::setLoopRange (te::TimeRange r)
{
    loopRange = r;
    hasLoop = loopRange.getLength().inSeconds() > 0.0;
    repaint();
}

void ui::TimelineComponent::snapSecondsToBeats (double& seconds) const
{
    if (!snapToBeats || editForSnap == nullptr) return;

    auto& ts = editForSnap->tempoSequence;
    const auto beat   = ts.toBeats (te::TimePosition::fromSeconds(seconds)).inBeats();
    const auto snappedBeat = std::round(beat);
    seconds = ts.toTime (te::BeatPosition::fromBeats(snappedBeat)).inSeconds();
}

