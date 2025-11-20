#include "TimelineComponent.h"

using namespace juce;

static constexpr int kTickBottom    = 22;
static constexpr int kMajorTickLen  = 10;
static constexpr int kMinorTickLen  = 5;

namespace te = tracktion::engine;
namespace t  = tracktion;

//==============================================================================
// Public API
//==============================================================================

/**
 * @brief Set the horizontal zoom level in pixels per beat.
 *
 * Ensures the zoom never goes below a minimum (10.0) so the view is usable.
 */
void ui::TimelineComponent::setPixelsPerBeat (double ppb)
{
    // Ensure the zoom level never gets ridiculously small or negative
    pixelsPerBeat = juce::jmax (10.0, ppb);

    // Trigger a repaint so the timeline ticks update visually
    repaint();
}

/**
 * @brief Set the beat at which the visible region starts.
 */
void ui::TimelineComponent::setViewStartBeat (t::BeatPosition b)
{
    // Update where the visible region of the timeline starts (in beats)
    viewStartBeat = b;

    // Trigger a repaint to reflect the new offset
    repaint();
}

//==============================================================================
// Painting
//==============================================================================

void ui::TimelineComponent::paint (Graphics& g)
{
    auto r = getLocalBounds();

    // Background
    g.setColour (Colour (0xFF2B2F33));
    g.fillRect (r);

    // Bottom border line
    g.setColour (Colours::black.withAlpha (0.4f));
    g.drawLine (0.0f,
                (float) r.getBottom() - 0.5f,
                (float) r.getRight(),
                (float) r.getBottom() - 0.5f);

    // Beat-based timeline: major ticks every 4 beats (1 bar), minor ticks every beat
    const double beatsPerBar   = 4.0; // Assume 4/4 time signature
    const double beatsPerMinor = 1.0; // One beat per minor tick

    const double startBeat = std::floor (viewStartBeat.inBeats() / beatsPerMinor) * beatsPerMinor;
    const double endBeat   = viewStartBeat.inBeats() + (double) getWidth() / pixelsPerBeat;

    for (double beat = startBeat; beat <= endBeat + 1e-6; beat += beatsPerMinor)
    {
        const int x = int ((beat - viewStartBeat.inBeats()) * pixelsPerBeat + 0.5);
        const bool isMajor = std::fmod (beat, beatsPerBar) < 1e-9;

        g.setColour (Colours::white.withAlpha (isMajor ? 0.55f : 0.25f));
        const int tickLen = isMajor ? kMajorTickLen : kMinorTickLen;

        g.drawLine ((float) x + 0.5f,
                    (float) r.getBottom() - (float) tickLen,
                    (float) x + 0.5f,
                    (float) r.getBottom());

        // Label major ticks with bar numbers (beat / 4 + 1)
        if (isMajor && x >= 0 && x <= r.getRight())
        {
            g.setColour (Colours::white.withAlpha (0.7f));
            g.setFont (Font (FontOptions (11.0f)));

            const int barNumber = static_cast<int> (beat / beatsPerBar) + 1;
            String label (barNumber);

            g.drawFittedText (label,
                              Rectangle<int> (x + 3, 2, 60, 14),
                              Justification::left,
                              1);
        }
    }

    // Loop range visualization (if present)
    if (hasLoop)
    {
        // Convert loop range time positions to beat positions
        const auto startBeatPos = edit.tempoSequence.toBeats (loopRange.getStart());
        const auto endBeatPos   = edit.tempoSequence.toBeats (loopRange.getEnd());

        const int x1 = beatsToX (startBeatPos.inBeats());
        const int x2 = beatsToX (endBeatPos.inBeats());
        const int y  = 0;
        const int h  = getHeight();

        juce::Rectangle<int> rr (juce::jmin (x1, x2), y, std::abs (x2 - x1), h);

        // Filled area
        g.setColour (juce::Colours::darkorange.withAlpha (0.25f));
        g.fillRect (rr);

        // Outline
        g.setColour (juce::Colours::darkorange);
        g.drawRect (rr, 2);

        // Handles at left/right edges
        g.fillRect (juce::Rectangle<int> (rr.getX() - handleWidthPx / 2, y, handleWidthPx, h));
        g.fillRect (juce::Rectangle<int> (rr.getRight() - handleWidthPx / 2, y, handleWidthPx, h));
    }
}

//==============================================================================
// Transport / Scrub
//==============================================================================

/**
 * @brief Update Edit transport position based on a pixel x-coordinate.
 *
 * Optionally invokes onScrub callback. The `dragging` flag is currently
 * unused but kept for future integration (e.g., with EditViewState).
 */
void ui::TimelineComponent::setTransportPositionFromX (int x, bool dragging)
{
    const auto time = xToTime (x);
    auto& tc = edit.getTransport();
    tc.setPosition (time);

    if (onScrub)
        onScrub (time);

    ignoreUnused (dragging); // no EditViewState dependency anymore
}

//==============================================================================
// Mouse handling
//==============================================================================

/** Small helper for hit-testing near loop edges. */
static bool near (int px, int target, int slop)
{
    return std::abs (px - target) <= slop;
}

/**
 * @brief Handles mouse-down events for playhead positioning, loop range editing, and panning.
 *
 * Mouse interaction modes:
 * - **Alt/Option + Click**: Instantly position playhead at clicked beat (added Nov 19, 2025)
 * - **Shift + Left Click**: Begin horizontal panning of timeline view
 * - **Middle Mouse Button**: Begin horizontal panning of timeline view
 * - **Normal Click**: Create new loop range or resize existing loop boundaries
 *
 * Loop range editing supports dragging start/end boundaries and snaps to whole beats.
 *
 * @param e The mouse event containing position and modifiers.
 */
void ui::TimelineComponent::mouseDown (const juce::MouseEvent& e)
{
    // Alt/Option + Click: Position playhead at clicked location (Nov 19, 2025)
    // Provides quick navigation following standard DAW conventions
    if (e.mods.isAltDown())
    {
        setTransportPositionFromX (e.x, false);
        return;
    }

    // Middle mouse or Shift+Left drag: start horizontal panning
    if (e.mods.isMiddleButtonDown() || (e.mods.isShiftDown() && e.mods.isLeftButtonDown()))
    {
        if (auto* vp = findParentComponentOfClass<juce::Viewport>())
        {
            panning      = true;
            panStartView = vp->getViewPosition();
            panStartX    = e.getPosition().x;
            return;
        }
    }

    const int mx = e.x;

    // If no loop exists yet, create a new default-length loop where clicked
    if (! hasLoop)
    {
        const double startBeats   = xToBeats (mx);
        const double defLenBeats  = 4.0; // seed length: 1 bar (4 beats in 4/4)

        // Convert beat positions to time positions for loopRange
        const auto startTime = edit.tempoSequence.toTime (t::BeatPosition::fromBeats (startBeats));
        const auto endTime   = edit.tempoSequence.toTime (t::BeatPosition::fromBeats (startBeats + defLenBeats));

        loopRange        = t::TimeRange (startTime, endTime);
        hasLoop          = true;
        dragMode         = DragMode::dragEnd;
        originalStartSec = startTime.inSeconds();
        originalEndSec   = endTime.inSeconds();

        if (onLoopRangeChanged)
            onLoopRangeChanged (loopRange);

        repaint();
        return;
    }

    // Convert existing loop range to beat positions for hit testing
    const auto startBeatPos = edit.tempoSequence.toBeats (loopRange.getStart());
    const auto endBeatPos   = edit.tempoSequence.toBeats (loopRange.getEnd());

    const int x1 = beatsToX (startBeatPos.inBeats());
    const int x2 = beatsToX (endBeatPos.inBeats());

    const juce::Rectangle<int> rr (juce::jmin (x1, x2),
                                   0,
                                   std::abs (x2 - x1),
                                   getHeight());

    // Hit test handles/body
    if (near (mx, rr.getX(), hitSlopPx))
    {
        dragMode = DragMode::dragStart;
    }
    else if (near (mx, rr.getRight(), hitSlopPx))
    {
        dragMode = DragMode::dragEnd;
    }
    else if (rr.contains (mx, e.y))
    {
        // Drag the loop body
        dragMode         = DragMode::dragBody;
        dragAnchorSec    = loopRange.getStart().inSeconds();
        originalStartSec = loopRange.getStart().inSeconds();
        originalEndSec   = loopRange.getEnd().inSeconds();
    }
    else
    {
        // Click outside: start a new region at cursor
        const double startBeats   = xToBeats (mx);
        const double defLenBeats  = 4.0;

        const auto startTime = edit.tempoSequence.toTime (t::BeatPosition::fromBeats (startBeats));
        const auto endTime   = edit.tempoSequence.toTime (t::BeatPosition::fromBeats (startBeats + defLenBeats));

        loopRange        = t::TimeRange (startTime, endTime);
        hasLoop          = true;
        dragMode         = DragMode::dragEnd;
        originalStartSec = startTime.inSeconds();
        originalEndSec   = endTime.inSeconds();

        if (onLoopRangeChanged)
            onLoopRangeChanged (loopRange);
    }

    repaint();
}

void ui::TimelineComponent::mouseDrag (const juce::MouseEvent& e)
{
    // Panning mode
    if (panning)
    {
        if (auto* vp = findParentComponentOfClass<juce::Viewport>())
        {
            const int dx = e.getPosition().x - panStartX;
            vp->setViewPosition (juce::jmax (0, panStartView.x - dx), panStartView.y);
        }
        return;
    }

    if (dragMode == DragMode::none || ! hasLoop)
        return;

    // Work in beat space for dragging
    const double mouseBeats = xToBeats (e.x);
    double startBeats       = edit.tempoSequence.toBeats (loopRange.getStart()).inBeats();
    double endBeats         = edit.tempoSequence.toBeats (loopRange.getEnd()).inBeats();

    switch (dragMode)
    {
        case DragMode::dragStart:
            startBeats = juce::jlimit (0.0, endBeats, mouseBeats);
            break;

        case DragMode::dragEnd:
            endBeats = juce::jmax (startBeats, mouseBeats);
            break;

        case DragMode::dragBody:
        {
            const double anchorBeats = edit.tempoSequence
                                           .toBeats (t::TimePosition::fromSeconds (dragAnchorSec))
                                           .inBeats();

            const double delta = mouseBeats - anchorBeats;

            const double originalStartBeats = edit.tempoSequence
                                                  .toBeats (t::TimePosition::fromSeconds (originalStartSec))
                                                  .inBeats();

            const double originalEndBeats = edit.tempoSequence
                                                .toBeats (t::TimePosition::fromSeconds (originalEndSec))
                                                .inBeats();

            startBeats = juce::jmax (0.0, originalStartBeats + delta);
            endBeats   = originalEndBeats + delta;
            break;
        }

        default:
            break;
    }

    // Snap to beats (already in beat space)
    if (snapToBeats)
    {
        startBeats = std::round (startBeats);
        endBeats   = std::round (endBeats);
    }

    // Convert back to time positions
    const auto startTime = edit.tempoSequence.toTime (t::BeatPosition::fromBeats (startBeats));
    const auto endTime   = edit.tempoSequence.toTime (t::BeatPosition::fromBeats (endBeats));

    loopRange = t::TimeRange (startTime, endTime);

    if (onLoopRangeChanged)
        onLoopRangeChanged (loopRange);

    repaint();
}

void ui::TimelineComponent::mouseUp (const juce::MouseEvent&)
{
    panning  = false;
    dragMode = DragMode::none;
}

//==============================================================================
// Right-click / double-click behavior
//==============================================================================

/**
 * @brief Right-click or modifier double-click clears the loop.
 *
 * - Right mouse button
 * - Or any modifier key held on double-click
 */
void ui::TimelineComponent::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown() || e.mods.isAnyModifierKeyDown())
    {
        hasLoop = false;
        loopRange = t::TimeRange (t::TimePosition::fromSeconds (0.0),
                                  t::TimePosition::fromSeconds (0.0));

        if (onLoopRangeChanged)
            onLoopRangeChanged (loopRange);

        repaint();
    }
}

//==============================================================================
// Loop + snapping helpers
//==============================================================================

void ui::TimelineComponent::setLoopRange (t::TimeRange r)
{
    loopRange = r;
    hasLoop   = loopRange.getLength().inSeconds() > 0.0;
    repaint();
}

/**
 * @brief Snap a time in seconds to the nearest beat in editForSnap.
 */
void ui::TimelineComponent::snapSecondsToBeats (double& seconds) const
{
    if (! snapToBeats || editForSnap == nullptr)
        return;

    auto& ts = editForSnap->tempoSequence;
    const auto beat        = ts.toBeats (t::TimePosition::fromSeconds (seconds)).inBeats();
    const auto snappedBeat = std::round (beat);

    seconds = ts.toTime (t::BeatPosition::fromBeats (snappedBeat)).inSeconds();
}