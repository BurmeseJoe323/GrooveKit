#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion::engine;
#include "tracktion_graph/tracktion_graph.h"
#include <functional>

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
    std::function<void(int targetTrack, te::TimePosition time, te::TimeDuration length, bool isValid)> onDragUpdate;
    std::function<void(te::MidiClip*, int targetTrack, te::TimePosition newStart)> onDragComplete;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;

private:
    void updateSizeFromClip();
    void onResizeEnd();

    // Drag helper methods - Written by Claude Code
    te::TimePosition mouseToTime (const juce::MouseEvent& e);
    int mouseToTrackIndex (const juce::MouseEvent& e);
    te::TimePosition quantizeToGrid (te::TimePosition time, double gridSize = 0.25);

    // Custom constrainer that notifies us when resizing completes
    class ResizeConstrainer : public juce::ComponentBoundsConstrainer
    {
    public:
        explicit ResizeConstrainer (TrackClip& owner) : trackClip (owner) {}

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
    te::TimePosition originalStartTime;
    int originalTrackIndex = -1;
    te::TimeDuration clickOffsetFromStart; // Offset from clip start to where user clicked (Written by Claude Code)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackClip)
};
