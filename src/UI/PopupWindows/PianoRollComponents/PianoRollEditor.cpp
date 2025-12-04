//
// Created by Joseph Rockwell on 4/8/25.
//

#include "PianoRollEditor.h"

//==============================================================================
// Construction / Destruction
//==============================================================================

PianoRollEditor::PianoRollEditor (AppEngine& engine, te::MidiClip* clip)
    : noteGrid (gridStyleSheet, engine, clip),
      controlPanel (noteGrid, gridStyleSheet)
{
    // Setup note grid
    addAndMakeVisible (gridView);
    gridView.setViewedComponent (&noteGrid, false);
    gridView.setScrollBarsShown (true, true);
    gridView.setScrollBarThickness (10);

    // Setup timeline
    addAndMakeVisible (timelineView);
    timelineView.setViewedComponent (&timeline, false);
    timelineView.setScrollBarsShown (false, false);

    // Setup keyboard
    addAndMakeVisible (keyboardView);
    keyboardView.setViewedComponent (&keyboard, false);
    keyboardView.setScrollBarsShown (false, false);

    // Scroll the other components when the grid is scrolled
    gridView.positionMoved = [this] (int x, int y) {
        timelineView.setViewPosition (x, y);
        keyboardView.setViewPosition (x, y);
    };

    // Setup control panel
    addAndMakeVisible (controlPanel);
    controlPanel.configureGrid = [this] (int pixelsPerBar, int noteHeight) {
        setup (10, pixelsPerBar, noteHeight);
    };

    noteGrid.onEdit = [this]() //pass up the chain.
    {
        if (this->onEdit != nullptr)
        {
            this->onEdit();
        }
    };
    noteGrid.sendChange = [this] (int note, int vel) {
        if (this->sendChange != nullptr)
        {
            this->sendChange (note, vel);
        }
    };

    playbackTicks = 0;
    showPlaybackMarker = false;

    addAndMakeVisible(closeButton);
    closeButton.setTooltip("Close piano roll");
    closeButton.onClick = [this]
    {
        if (onClose) onClose();
    };

    setWantsKeyboardFocus(true);
    setFocusContainerType (juce::Component::FocusContainerType::focusContainer);

}
PianoRollEditor::PianoRollEditor (AppEngine& engine, int trackIndex)
    : PianoRollEditor (engine, [&]() -> te::MidiClip*
{
    // resolve the first MIDI clip on that track, or nullptr
    auto& edit = engine.getEdit();
    auto audioTracks = te::getAudioTracks (edit);
    if (! juce::isPositiveAndBelow (trackIndex, audioTracks.size()))
        return nullptr;

    if (auto* at = audioTracks.getUnchecked (trackIndex))
        for (auto* c : at->getClips())
            if (auto* mc = dynamic_cast<te::MidiClip*> (c))
                return mc;

    return nullptr;
}())
{}

//==============================================================================
// Component Overrides
//==============================================================================

void PianoRollEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey.darker());
}

void PianoRollEditor::paintOverChildren (juce::Graphics& g)
{
    const int x = noteGrid.getPixelsPerBar() * (playbackTicks / (4.0 * PRE::defaultResolution));
    const int xAbsolute = gridView.getViewPosition().getX();

    g.setColour (juce::Colours::greenyellow);
    g.drawLine (x - xAbsolute, 0, x - xAbsolute, getHeight(), 5.0);
}

//==============================================================================
// UI Control
//==============================================================================

void PianoRollEditor::showControlPanel (bool state)
{
    controlPanel.setVisible (state);
}

void PianoRollEditor::resized()
{
    auto r = getLocalBounds();

    // Header area for controls
    auto header = r.removeFromTop(32);
    closeButton.setBounds(header.removeFromRight(28).reduced(2));
    gridView.setBounds (80, 50, getWidth() - 90, getHeight() - 100);
    timelineView.setBounds (gridView.getX(), 5, gridView.getWidth() - 10, gridView.getY() - 5);
    keyboardView.setBounds (5, gridView.getY(), 70, gridView.getHeight() - 10);

    noteGrid.setupGrid (pixelsPerBar, noteHeight, numBars);
    timeline.setBounds (0, 0, noteGrid.getWidth(), timelineView.getHeight());
    timeline.setup (numBars, pixelsPerBar);
    keyboard.setNoteHeight (noteHeight);  // Sync keyboard note height with grid
    keyboard.setBounds (0, 0, keyboardView.getWidth(), noteGrid.getHeight());

    controlPanel.setBounds (5, gridView.getBottom() + 5, getWidth() - 10, 80);
}

//==============================================================================
// Setup and Configuration
//==============================================================================

void PianoRollEditor::setStyleSheet (GridStyleSheet style)
{
    gridStyleSheet = style;
}

void PianoRollEditor::setup (const int bars, const int pixelsPerBar, const int noteHeight)
{
    // NOTE: there's probably a better way to do this. Depending on how we implement bars, we may not
    // need to do this check at all
    if (bars > 1 && bars < 1000)
    {
        noteGrid.setupGrid (pixelsPerBar, noteHeight, bars);
        timeline.setup (bars, noteGrid.getPixelsPerBar());
        timeline.setSize (noteGrid.getWidth(), timeline.getHeight());
        keyboard.setSize (keyboardView.getWidth(), noteGrid.getHeight());
    }
    else
    {
        jassertfalse;
    }
}

void PianoRollEditor::updateBars (const int newNumberOfBars)
{
    if (newNumberOfBars > 1 && newNumberOfBars < 1000)
    {
        // sensible limits..
        const float pixelsPerBar = noteGrid.getPixelsPerBar();
        const float noteHeight = noteGrid.getNoteCompHeight();

        noteGrid.setupGrid (pixelsPerBar, noteHeight, newNumberOfBars);
        timeline.setup (newNumberOfBars, pixelsPerBar);
        keyboard.setSize (keyboardView.getWidth(), noteGrid.getHeight());
    }
    else
    {
        jassertfalse;
    }
}

// void PianoRollEditor::loadSequence() {
//     noteGrid.loadSequence();
//
//
//     // TODO: fix me, this automatically scrolls the grid
//     //    const int middleNote = ((sequence.highNote - sequence.lowNote) * 0.5) + sequence.lowNote;
//     //    const float scrollRatio = middleNote / 127.0;
//     //    setScroll(0.0, scrollRatio);
// }

//==============================================================================
// Data Access
//==============================================================================

const te::MidiList& PianoRollEditor::getSequence()
{
    return noteGrid.getSequence();
}

GridControlPanel& PianoRollEditor::getControlPanel()
{
    return controlPanel;
}

//==============================================================================
// Scrolling and View Control
//==============================================================================

void PianoRollEditor::setScroll (double x, double y)
{
    gridView.setViewPositionProportionately (x, y);
}

//==============================================================================
// Playback Marker
//==============================================================================

void PianoRollEditor::setPlaybackMarkerPosition (const st_int ticks, bool isVisible)
{
    showPlaybackMarker = isVisible;
    playbackTicks = ticks;

    const float qnTicks       = (float) PRE::defaultResolution;      // ticks per quarter note
    const float beatsNow      = playbackTicks / qnTicks;             // current beat position
    const float beatsPerBar   = noteGrid.getBeatsPerBar();           // expose getter (see below)
    const float pixelsPerBeat = noteGrid.getPixelsPerBar() / beatsPerBar;

    const int x = (int) std::round(beatsNow * pixelsPerBeat);

    const int xAbsolute = gridView.getViewPosition().getX();

    repaint();
}

bool PianoRollEditor::keyPressed (const juce::KeyPress& k)
{
    if (k == juce::KeyPress(juce::KeyPress::escapeKey))
    {
        if (onClose) onClose();
        return true;
    }
    return false;
}

void PianoRollEditor::setClip (te::MidiClip* c)
{
    if (c == nullptr) return;

    // Hand off to the note grid to swap its data model.
    noteGrid.setClip(c);

    // Update keyboard dimming for drum tracks (Written by Claude Code)
    keyboard.setIsDrumTrack (noteGrid.getIsDrumTrack());

    // Trigger resized() to update grid bounds for drum vs instrument tracks (Written by Claude Code)
    resized();
    repaint();
}