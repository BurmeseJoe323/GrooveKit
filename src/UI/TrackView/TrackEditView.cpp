// Note: Junie (JetBrains AI) contributed code to this file on 2025-09-24.
#include "TrackEditView.h"
#include "../TransportBar/TransportBar.h"
#include "../MenuBar/GrooveKitMenuBar.h"
#include "MidiListener.h"
#include "../../AppEngine/AppEngine.h"
#include "../../AppEngine/ValidationUtils.h"
#include <regex>

//==============================================================================
// Helper Functions

// Helper for styling the menu buttons
void styleMenuButton (juce::TextButton& button)
{
    button.setColour (juce::TextButton::buttonColourId, juce::Colour (0x00000000));
    button.setColour (juce::TextButton::textColourOffId, juce::Colours::lightgrey);
    button.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
}

//==============================================================================
// Construction / Destruction

TrackEditView::TrackEditView (AppEngine& engine, TransportBar& transport, GrooveKitMenuBar& menu)
    : transportBar(&transport), menuBar(&menu)
{
    appEngine = std::shared_ptr<AppEngine> (&engine,
        [](AppEngine*) {
        });

    // Add menu bar and transport bar (menu above transport on non-Mac)
    addAndMakeVisible (menuBar);
    addAndMakeVisible (transportBar);

    trackList = std::make_unique<TrackListComponent> (appEngine);

    trackList->setPixelsPerBeat (pixelsPerBeat);
    trackList->setViewStartBeat (viewStartBeat);

    viewport.setScrollBarsShown (true, true);
    viewport.setViewedComponent (trackList.get(), false);

    trackList->rebuildFromEngine();

    // Set up menu bar callbacks to refresh UI when tracks are created (Written by Claude Code)
    menuBar->onNewInstrumentTrack = [this] {
        const int index = appEngine->addInstrumentTrack();
        trackList->addNewTrack(index);
        trackList->setPixelsPerBeat(pixelsPerBeat);
        trackList->setViewStartBeat(viewStartBeat);
        trackList->resized(); // Trigger layout update to position new track
    };
    menuBar->onNewDrumTrack = [this] {
        const int index = appEngine->addDrumTrack();
        trackList->addNewTrack(index);
        trackList->setPixelsPerBeat(pixelsPerBeat);
        trackList->setViewStartBeat(viewStartBeat);
        trackList->resized(); // Trigger layout update to position new track
    };

    appEngine->onEditLoaded = [this] {
        trackList = std::make_unique<TrackListComponent> (appEngine);
        trackList->setPixelsPerBeat (pixelsPerBeat);
        trackList->setViewStartBeat (viewStartBeat);

        viewport.setViewedComponent (trackList.get(), false);
        trackList->rebuildFromEngine();

        hidePianoRoll();

        transportBar->updateBpmDisplay();

        repaint();
    };

    // Initialize and hide the piano roll editor
    pianoRoll = std::make_unique<PianoRollEditor> (*appEngine, -1);
    addAndMakeVisible (pianoRoll.get());
    pianoRoll->setVisible (false);
    pianoRoll->onClose = [this] { hidePianoRoll(); };

    // Split the view vertically
    verticalLayout.setItemLayout (0, -0.45, -0.85, -0.6); // Track list takes 70%
    verticalLayout.setItemLayout (1, 5, 5, 5); // 5-pixel splitter
    verticalLayout.setItemLayout (2, -0.15, -0.55, -0.4); // Piano roll takes 30%

    // Create and add resizer bar (index 1 in components array)
    resizerBar = std::make_unique<PianoRollResizerBar> (&verticalLayout, 1, false);
    addAndMakeVisible (resizerBar.get());

    addAndMakeVisible (viewport);

    setWantsKeyboardFocus (true);
}

TrackEditView::~TrackEditView() = default;

//==============================================================================
// Component Overrides

void TrackEditView::paint (juce::Graphics& g)
{
    // TransportBar now handles its own painting (Written by Claude Code)
    // Set all arm record buttons to be enabled or disabled depending on state of AppEngine
    trackList->setAllArmButtonsEnabled (!appEngine->isRecording());
}

void TrackEditView::resized ()
{
    auto r = getLocalBounds();

    // Position menu bar at top on non-Mac platforms
    #if !JUCE_MAC
    constexpr int menuHeight = 24;
    menuBar->setBounds (r.removeFromTop (menuHeight));
    #endif

    // Position transport bar below menu (or at top on Mac)
    constexpr int transportHeight = 40;
    transportBar->setBounds (r.removeFromTop (transportHeight));

    // Content area below menu and transport
    // If the piano roll is hidden, just fill with the viewport and hide the resizer
    if (!pianoRoll || !pianoRoll->isVisible())
    {
        viewport.setBounds (r);
        if (resizerBar)
            resizerBar->setVisible (false);
        return;
    }

    // Piano roll is visible: use the stretchable layout to split vertically
    if (resizerBar)
        resizerBar->setVisible (true);

    juce::Component* comps[] = { &viewport, resizerBar.get(), pianoRoll.get() };
    verticalLayout.layOutComponents (comps, (int) std::size (comps), r.getX(), r.getY(), r.getWidth(), r.getHeight(), true, true);

    // Ensure the resizer and piano roll are on top of the viewport
    resizerBar->toFront (false);
    pianoRoll->toFront (false);
}

bool TrackEditView::keyPressed (const juce::KeyPress& key_press)
{
    // The note keys are being handled by keyStateChanged, so we'll just say that the event is consumed
    if (appEngine->getMidiListener().getNoteKeys().contains (key_press.getKeyCode()))
        return true;

    if (key_press == juce::KeyPress::spaceKey)
    {
        // Spacebar toggles transport
        if (appEngine->isPlaying())
        {
            appEngine->stop();
        }
        else
        {
            appEngine->play();
        }
        return true;
    }

    // Let MidiListener handle octave changes (Z/X keys)
    if (appEngine->getMidiListener().handleKeyPress(key_press))
        return true;

    // This is the top level of our application, so if the key press has not been consumed,
    // it is not an implemented key command in GrooveKit
    return true;
}

bool TrackEditView::keyStateChanged (bool isKeyDown)
{
    return appEngine->getMidiListener().handleKeyStateChanged(isKeyDown);
}

void TrackEditView::parentHierarchyChanged()
{
    juce::MessageManager::callAsync(
        [safe = juce::Component::SafePointer<TrackEditView>(this)]
        {
            if (safe != nullptr && safe->isShowing())
                safe->grabKeyboardFocus();
        });
}

void TrackEditView::mouseDown (const juce::MouseEvent& e)
{
    grabKeyboardFocus();
    juce::Component::mouseDown(e);
}

//==============================================================================
// UI Setup

void TrackEditView::showPianoRoll (te::MidiClip* clip)
{
    if (pianoRoll == nullptr)
    {
        pianoRoll = std::make_unique<PianoRollEditor> (*appEngine, clip);
        addAndMakeVisible (pianoRoll.get());
    }
    else
    {
        pianoRoll->setClip (clip);
    }

    pianoRollClip = clip;
    pianoRoll->setVisible (true);
    resized();

    // Notify clip UI that it's being edited - Written by Claude Code
    if (clip && trackList)
    {
        const int n = appEngine->getNumTracks();

        // First, clear all clip highlights across all tracks (Written by Claude Code)
        for (int i = 0; i < n; ++i)
            trackList->updateClipEditState (i, nullptr);

        // Then, find and highlight the new clip
        for (int i = 0; i < n; ++i)
        {
            auto clips = appEngine->getMidiClipsFromTrack (i);
            for (auto* mc : clips)
            {
                if (mc == clip)
                {
                    trackList->updateClipEditState (i, clip);
                    return;
                }
            }
        }
    }
}

void TrackEditView::hidePianoRoll ()
{
    // Clear clip editing state before hiding - Written by Claude Code
    if (pianoRollClip && trackList)
    {
        // Find track index for this clip
        const int n = appEngine->getNumTracks();
        for (int i = 0; i < n; ++i)
        {
            auto clips = appEngine->getMidiClipsFromTrack (i);
            for (auto* mc : clips)
            {
                if (mc == pianoRollClip)
                {
                    trackList->updateClipEditState (i, nullptr);
                    break;
                }
            }
        }
    }

    pianoRollVisible = false;
    if (pianoRoll) pianoRoll->setVisible(false);
    pianoRollClip = nullptr;
    resized();
}

int TrackEditView::getPianoRollIndex () const
{
    if (pianoRollClip == nullptr || !appEngine)
        return -1;

    const int n = appEngine->getNumTracks();
    for (int i = 0; i < n; ++i)
    {
        auto clips = appEngine->getMidiClipsFromTrack (i);
        for (auto* mc : clips)
            if (mc == pianoRollClip)
                return i;
    }
    return -1;
}

// Written by Claude Code
void TrackEditView::refreshClipEditState()
{
    if (pianoRollClip && trackList && appEngine)
    {
        // Find track index for the currently edited clip
        const int n = appEngine->getNumTracks();
        for (int i = 0; i < n; ++i)
        {
            auto clips = appEngine->getMidiClipsFromTrack (i);
            for (auto* mc : clips)
            {
                if (mc == pianoRollClip)
                {
                    trackList->updateClipEditState (i, pianoRollClip);
                    return;
                }
            }
        }
    }
}


void TrackEditView::PianoRollResizerBar::hasBeenMoved ()
{
    // DBG("X: " << this->getX() << " Y: " << this->getY());
    resized();
}

void TrackEditView::PianoRollResizerBar::mouseDrag (const juce::MouseEvent& event)
{
    // DBG("X: " << this->getX() << " Y: " << this->getY());
    // this->setTopLeftPosition (this->getX(), event.getPosition().getY());
    hasBeenMoved();
}

TrackEditView::PianoRollResizerBar::PianoRollResizerBar (juce::StretchableLayoutManager* layoutToUse, int itemIndexInLayout, bool isBarVertical)
    : StretchableLayoutResizerBar (layoutToUse, itemIndexInLayout, isBarVertical)
{
}

TrackEditView::PianoRollResizerBar::~PianoRollResizerBar ()
= default;

//==============================================================================
// Timer Callback (Recording Visual Feedback)

void TrackEditView::timerCallback()
{
    const bool isRecording = appEngine->isRecording();
    const int armedTrackIndex = appEngine->getArmedTrackIndex();

    // Repaint the armed track while recording to show red tint
    if (isRecording && armedTrackIndex >= 0 && trackList)
    {
        trackList->repaintTrack(armedTrackIndex);
    }

    // If recording just stopped, repaint one more time to clear the red tint
    if (wasRecording && !isRecording && armedTrackIndex >= 0 && trackList)
    {
        trackList->repaintTrack(armedTrackIndex);
    }

    wasRecording = isRecording;
}