#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>
#include "tracktion_graph/tracktion_graph.h"
#include <functional>

namespace te = tracktion::engine;
namespace t = tracktion;

class TrackClip final : public juce::Component, private juce::ValueTree::Listener
{
public:
    explicit TrackClip(te::MidiClip* clip, float pixelsPerBeat);
    ~TrackClip() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setColor (juce::Colour newColor);
    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;
    void setPixelsPerBeat (float ppb);
    void setBeingEdited (bool edited); // Highlight clip when being edited in piano roll (Written by Claude Code)

    te::MidiClip* getMidiClip() const noexcept { return clip; }
    juce::ValueTree clipState;

    std::function<void(te::MidiClip*)> onClicked;
    std::function<void(te::MidiClip*)> onCopyRequested;
    std::function<void(te::MidiClip*)> onDuplicateRequested;
    std::function<void(te::MidiClip*, double pasteAtBeats)> onPasteRequested; // paste location in beats
    std::function<void(te::MidiClip*)> onDeleteRequested;
    // Request the parent TrackComponent to show a context menu for this clip at the given beat position
    std::function<void(te::MidiClip*)> onContextMenuRequested;

    // Drag callbacks - Written by Claude Code
    std::function<void(int targetTrack, t::TimePosition time, t::TimeDuration length, bool isValid)> onDragUpdate;
    std::function<void(te::MidiClip*, int targetTrack, t::TimePosition newStart)> onDragComplete;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;

private:
    void updateSizeFromClip();
    void onResizeEnd();
    void quantizeWidth (juce::Rectangle<int>& bounds); // Live quantization during resize (Written by Claude Code)

    // Drag helper methods - Written by Claude Code
    t::TimePosition mouseToTime (const juce::MouseEvent& e);
    int mouseToTrackIndex (const juce::MouseEvent& e);
    t::TimePosition quantizeToGrid (t::TimePosition time, double gridSize = 0.25);

    // Custom constrainer that notifies us when resizing completes
    // and provides live quantization during drag (Written by Claude Code)
    class ResizeConstrainer : public juce::ComponentBoundsConstrainer
    {
    public:
        explicit ResizeConstrainer (TrackClip& owner) : trackClip (owner) {}

        void checkBounds (juce::Rectangle<int>& bounds,
                         const juce::Rectangle<int>& previousBounds,
                         const juce::Rectangle<int>& limits,
                         bool isStretchingTop,
                         bool isStretchingLeft,
                         bool isStretchingBottom,
                         bool isStretchingRight) override
        {
            // Call base class first to apply min/max constraints
            juce::ComponentBoundsConstrainer::checkBounds (bounds, previousBounds, limits,
                                                           isStretchingTop, isStretchingLeft,
                                                           isStretchingBottom, isStretchingRight);

            // Apply live quantization when resizing right edge (Written by Claude Code)
            if (isStretchingRight)
            {
                trackClip.quantizeWidth (bounds);
            }
        }

        void resizeEnd() override
        {
            trackClip.onResizeEnd();
        }

    private:
        TrackClip& trackClip;
    };

    te::MidiClip* clip = nullptr; // not owned
    float pixelsPerBeat = 100.0f;
    juce::Colour clipColor { juce::Colours::blueviolet };

    ResizeConstrainer resizeConstrainer;
    juce::ResizableEdgeComponent edgeResizer;

    // Drag state - Written by Claude Code
    bool isDragging = false;
    bool dragThresholdExceeded = false; // Track if drag threshold passed (Written by Claude Code)
    float dragAlpha = 1.0f; // Transparency during drag
    juce::Point<int> dragStartMousePos;
    t::TimePosition originalStartTime;
    int originalTrackIndex = -1;
    t::TimeDuration clickOffsetFromStart; // Offset from clip start to where user clicked (Written by Claude Code)

    // Piano roll editing state - Written by Claude Code
    bool isBeingEdited = false; // True when this clip is open in piano roll

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackClip)
};
